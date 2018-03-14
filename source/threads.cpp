////////////////////////////////////////////////////////////////////////////////
// entry points for secondary threads of RECONSTRUCT application
//
//    Copyright (C) 1999-2007  John Fiala (fiala@bu.edu)
//
//    This is free software created with funding from the NIH. You may
//    redistribute it and/or modify it under the terms of the GNU General
//    Public License published by the Free Software Foundation (www.gnu.org).
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License version 2 for more details.
//
// 
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change: Modified ThumbsThread to use new ImageSize() and ContourSize() methods.
// -+- change: Added Reset when scene empty after adding objects.
// modified 7/14/05 by JCF (fiala@bu.edu)
// -+- change: Modified RenderThread to detect GDI failure when attempting to copy from canvas to view.
// modified 10/3/05 by JCF (fiala@bu.edu)
// -+- change: Added splash error when thunbnail bitmap memory runs out
// modified 11/17/05 by JCF (fiala@bu.edu)
// -+- change:  Added progress to ObjectList thread; removed MAX_INT32 section range.
// modified 6/14/06 by JCF (fiala@bu.edu)
// -+- change: Added check for vertex-less object with Adding to Scene to avoid crash when scene is empty.
// modified 2/5/07 by JCF (fiala@bu.edu)
// -+- change: Added SetForegroundWindow() to end of ObjectListThread(), plus some debugging stuff.
// modified 4/5/07 by JCF (fiala@bu.edu)
// -+- change: Corrected initial domain boundary extent in RenderThread()
// modified 5/8/07 by JCF (fiala@bu.edu)
// -+- change: Modified ObjectList and DistanceList message passing to use appWnd to avoid hanging.
//

#include "reconstruct.h"


BOOL CALLBACK RenderProgressProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char dlgItemTxt[64];

	switch (message)					// dialog procedure for showing tread progress and allowing user abort
		{
		case WM_INITDIALOG:
			SetWindowText( hwndDlg, "Exporting Image...");
			break;
										// use UM message to allow PostMessage to update text
		case UM_UPDATETEXT:
			SetDlgItemText( hwndDlg, ID_MSGTXT, (LPCTSTR)lParam );
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case ID_DONE:
					if ( AbortRender )						// user aborted, so report this
						sprintf(dlgItemTxt,"Operation aborted.");
					else sprintf(dlgItemTxt,"Operation completed.");
					SetDlgItemText( hwndDlg, ID_MSGTXT, dlgItemTxt );
					AbortRender = true;						// use abort to flag for end dialog
					SetDlgItemText( hwndDlg, IDOK, "OK" );		// switch button and icon
					SendDlgItemMessage( hwndDlg, ID_MSGICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon( appInstance, "ALargeIcon") );
					return TRUE;

				case IDOK:								// button press after end or abort terminates dialog
					if ( AbortRender )
						{
						AbortRender = false;
						DestroyWindow(hwndDlg);
						}
					else {								// button press during operation indicates abort request
						AbortRender = true;
						sprintf(dlgItemTxt,"Aborting...");
						SetDlgItemText( hwndDlg, ID_MSGTXT, dlgItemTxt );
						}
					return TRUE;
				}
		}
	return FALSE;
}

DWORD WINAPI RenderThread( LPVOID )					// Render all images in range...
{
	char titletxt[80], msgtxt[80];
	int sectnum, index, width, height, x, y, first, last;
	char filename[MAX_PATH], filepath[MAX_PATH];
	ADib *adib;
	ViewPort *rendering;
	Series *series;
	Section *section, *nsection;
	Transform *transform;
	Mvmt mvmt;								
	Point *p;
	RECT region;
	double pixel_size, offset_x, offset_y;

	first = FirstSection;								// make local copy of global values
	last = LastSection;
	pixel_size = UsePixelSize;							// set units to pixelsize value from dialog
	if ( RenderWindow )									// to use current view set size and offset
		{
		GetClientRect( appWnd, &region );
		x = region.right - region.left;					// to that of the main window
		y = region.bottom - region.top;
		offset_x = CurrSeries->offset_x;
		offset_y = CurrSeries->offset_y;
		}
	else {												// otherwise use maximum offset for whole series
		offset_x = XOffset;
		offset_y = YOffset;
		x = (int)(XLength/UsePixelSize);				// calculate final bitmap size using values from
		y = (int)(YLength/UsePixelSize);				// dialog and earlier calculations from series
		}
														// create series file for new series
	series = new Series( *CurrSeries );
	if ( (series->index < first) || (series->index > last) ) series->index = first;
	sprintf(filename,"%s%s.ser",WorkingPath,SeriesName);
	series->Save( filename );
	delete series;

	width = x + 1;
	height = y + 1;
	rendering = new ViewPort( width, height, appWnd );	// renderable viewport for creating output bitmap
	region.left = 0;
	region.right = rendering->width - 1;
	region.top = 0;
	region.bottom = rendering->height - 1;
														// Now render each section onto this bitmap...
	sectnum = first - 1;								// use local copy of sections loaded from disk, not memory ones
	while ( !AbortRender && FindSection( sectnum, 1, filepath ) )
		{
		if ( sectnum > last ) break;						// report progress
		sprintf(msgtxt,"section %d",sectnum);
		PostMessage( renderProgressDlg, UM_UPDATETEXT, 0, (LPARAM)msgtxt );
		
		section = new Section( filepath );					// read section data
		if ( section->index >= 0 )							// if valid, use section for rendering
			{											
			rendering->section = section;					// render images at full selected resolution
			if ( !AbortRender )
				rendering->RenderImages( region, pixel_size, offset_x, offset_y, false );

			if ( RenderTraces && !AbortRender )				// now draw contours if desired
				{
				rendering->ImageToViewDC();					// put image into canvas
				rendering->DrawContours( pixel_size, offset_x, offset_y );	// draw using GDI
				if ( !rendering->ViewDCToImage() )			// if fail to retrieve image, inform user
					ErrMsgSplash( ERRMSG_GDIFAILED, "Try using Fill option for Traces." );
				}

			if ( RenderFill && !AbortRender )				// colorize contour regions
				rendering->FillContours( region, pixel_size, offset_x, offset_y );

			if ( !AbortRender )
				{
				adib = new ADib( width, height, 24 );		// create adib with 24-bit output format
				rendering->view->CopyBits( adib );			// and copy pixel data into this format
				if ( RenderJPEG )							// now, write out the image file
					{
					sprintf(filename,"%s_%s.%d.jpg",WorkingPath,SeriesName,sectnum);
					adib->SaveADibAsJPEG( filename, JPEGquality );
					}
				else {
					sprintf(filename,"%s_%s.%d.bmp",WorkingPath,SeriesName,sectnum);
					adib->SaveADibAsBMP( filename );
					}
				delete adib;								// discard adib
															// create a new section for this image		
				nsection = new Section();
				nsection->index = section->index;
				nsection->thickness = section->thickness;
				nsection->transforms = new Transforms();
				transform = new Transform();
				nsection->transforms->Add( transform );
				mvmt.transX = offset_x;
				mvmt.transY = offset_y;
				transform->nform->PostApply( mvmt );
				transform->image = new Image();
				transform->image->mag = pixel_size;			// reformat filename w/o full path
				if ( RenderJPEG ) sprintf(filename,"_%s.%d.jpg",SeriesName,sectnum);
				else sprintf(filename,"_%s.%d.bmp",SeriesName,sectnum);
				strcpy(transform->image->src,filename);
				transform->domain = new Contour();			// create domain contour for image
				strcpy(transform->domain->name,"domain0");
				transform->domain->mode = R2_NOP;			// don't fill a domain
				transform->domain->closed = true;			// but use a closed boundary
				transform->domain->points = new Points(); 
				p = new Point(0.0,0.0,0.0);	
				transform->domain->points->Add(p);
				p = new Point((double)(width-1),0.0,0.0);
				transform->domain->points->Add(p);
				p = new Point((double)(width-1),(double)(height-1),0.0);
				transform->domain->points->Add(p);
				p = new Point(0.0,(double)(height-1),0.0);
				transform->domain->points->Add(p);
															// now save as new section file
				sprintf(filename,"%s%s.%d",WorkingPath,SeriesName,sectnum);
				nsection->Save( filename );
				delete nsection;
				}
			}
		delete section;

		}		// end while

	delete rendering;									// done with viewport, delete it
	
														// notify progress dialog of termination
	PostMessage( renderProgressDlg, WM_COMMAND, (WPARAM)ID_DONE, 0 );
	PostMessage( appWnd, UM_RENDERDONE, 0, 0 );			// notify window so it can close thread handle
	return 0;											// ExitThread
}

void RenderThreadBegin( void )			// invoke background thread for rendering of sections
{
	HMENU hMenu;
	DWORD dwThreadID;							// also open a progress window to allow user to abort
	AbortRender = false;
	hMenu = GetMenu( appWnd );					// disable render command during thread
	EnableMenuItem( hMenu, CM_RENDERSECTIONS, MF_BYCOMMAND | MF_GRAYED );
	renderProgressDlg = CreateDialog( appInstance, "Progress", appWnd, (DLGPROC)RenderProgressProc );
	hRenderThread = CreateThread( NULL, 0, RenderThread, NULL, 0, &dwThreadID);
	if ( hRenderThread == NULL ) ErrMsgOK( ERRMSG_RUN_FAILED, NULL );
	else SetThreadPriority( hRenderThread, THREAD_PRIORITY_BELOW_NORMAL );
}

void RenderThreadEnd( void )				// release window menu, etc. and close thread handle
{
	HMENU hMenu;
	hMenu = GetMenu( appWnd );					// disable render command during thread
	EnableMenuItem( hMenu, CM_RENDERSECTIONS, MF_BYCOMMAND | MF_ENABLED );
	if ( hRenderThread )
		{
   		CloseHandle( hRenderThread );			// when thread completes, close handle
   		hRenderThread = NULL;
		}										// progress dialog is destroyed at end of thread proc
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ThumbsThread( LPVOID )					// render sections as thumbnails on buttons
{
	Button *button;
	Buttons *buttlist;
	int sectnum;
	Section *section;
	ViewPort *rendered;
	Point min, max;
	RECT wrect, region;
	double w, h, width, height, s, x, y, pixel_size;
	char filename[MAX_PATH];

	if ( ThumbButtons )											// in case window is destroyed before thread starts!
		buttlist = new Buttons( *ThumbButtons );				// make local copy for thread use
	else buttlist = new Buttons();
	w = (double)CurrSeries->thumbWidth;
	h = (double)CurrSeries->thumbHeight;						// copy dimensions of thumbnails
	ClientRectLessStatusBar( &wrect );
	height = (double)(wrect.bottom - wrect.top);				// use window dimensions for view params
	width = (double)(wrect.right - wrect.left);

	button = buttlist->first;
	while ( button && !AbortThumbs )
		{
		sectnum = button->number;
		if ( FindSection( sectnum, 0, filename ) )				// from all sections calculate bitmap size
			{
			section = new Section( filename );					// read section data
			if ( section->index >= 0 )							// create bitmap if valid...
				{
				if ( CurrSeries->fitThumbSections )
					{
					if ( section->HasImage() )					// get extremes of the section
						 section->ImageSize( min, max );
					else section->ContourSize( min, max );
					x = (max.x - min.x);						// determine size of section		
					y = (max.y - min.y);
					if ( x/w > y/h ) pixel_size = x/w;			// determine corresponding pixel size
					else pixel_size = y/h;
					x = min.x;
					y = min.y;									// set offsets to edge of section
					}
				else {
					s = w/width;
					if ( s > (h/height) ) s = h/height;
					pixel_size = CurrSeries->pixel_size/s;		// scale pixel_size up to fit in thumb window
					x = CurrSeries->offset_x;
					y = CurrSeries->offset_y;
					}

				if ( !AbortThumbs )								// check for abort before rendering section
					{
					rendered = new ViewPort( button->hwnd );		// create a button-sized rendering
					region.left = 0; region.right = rendered->width - 1;
					region.top = 0; region.bottom = rendered->height - 1;									
					rendered->section = section;					// display images, always use proxies if available
					rendered->RenderImages( region, pixel_size, x, y, true );
					rendered->ImageToViewDC();						// put on canvas bitmap
					if ( CurrSeries->displayThumbContours )
						rendered->DrawContours( pixel_size, x, y );	// add contours;					
					button->canvas = rendered->ReleaseCanvas();
					if ( !button->canvas )
						ErrMsgSplash( ERRMSG_GDIFAILED, "Try reducing thumbnail size or number." );
					delete rendered;								// ask button to update its canvas
						
					PostMessage( button->hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(button->canvas) );
					}
				}
			delete section;
			}
		button = button->next;
		}

	PassbackButtons = buttlist;							// passback modified global list to main thread
	
	PostMessage( appWnd, UM_THUMBSDONE, 0, 0 );			// notify main window to close thread handle

	return 0;											// terminate the thread with exit code 0
}

void ThumbsThreadBegin( void )				// invoke background thread for rendering thumbnails
{
	DWORD dwThreadID;
startTime2 = GetTickCount();
	AbortThumbs = false;
	hThumbsThread = CreateThread( NULL, 0, ThumbsThread, NULL, 0, &dwThreadID);
	if ( hThumbsThread == NULL ) ErrMsgOK( ERRMSG_RUN_FAILED, NULL );
	else SetThreadPriority( hThumbsThread, THREAD_PRIORITY_BELOW_NORMAL );
}

void ThumbsThreadEnd( void )				// release window menu, etc. and close thread handle
{
	if ( hThumbsThread )						// when thread completes, close handle
		{
   		CloseHandle( hThumbsThread );
   		hThumbsThread = NULL;					// if abortion ended thread, then clean up everything
		if ( PassbackButtons )
			if ( AbortThumbs )
			delete PassbackButtons;
			else {
				delete( ThumbButtons );			// this routine is called from main thread so no conflicts
				ThumbButtons = PassbackButtons;	// when switching to modified list with canvas handles
				}
		PassbackButtons = NULL;
		AbortThumbs = false;					// clear abort flag if set
		}
totalTime2 += GetTickCount() - startTime2;	// DEBUGGING
nTime2++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ObjectListThread( LPVOID )				// fill interobjects distances list
{
	VRMLObject *object;
	Section *section;
	Transform *transform;
	Contour *contour, *c;
	LV_ITEM lvi;
	double x, y, length, area;
	int sectnum, last_sectnum;
	bool listObjectSurfarea, listObjectFlatarea, listObjectVolume;

	if ( CurrSeries )									// make a local copy of CurrSeries options
		{												// in case user exits series while thread is running
		listObjectSurfarea = CurrSeries->listObjectSurfarea;
		listObjectFlatarea = CurrSeries->listObjectFlatarea;
		listObjectVolume = CurrSeries->listObjectVolume;
		}
	last_sectnum = MAX_INT32;							// get index of last section from info list
	if ( CurrSectionsInfo )
		if ( CurrSectionsInfo->last )
			last_sectnum = CurrSectionsInfo->last->index;

	if ( CurrObjects ) delete CurrObjects;
	CurrObjects = new VRMLObjects();					// create object list
	
	sectnum = 0;										// don't start at -1, so exclude section 0 traces
	while ( !AbortObject )
		{
		section = GetNextSectionBetween( sectnum, last_sectnum );
		if ( section )
		  {														// thread only communicates with main window
		  PostMessage( appWnd, UM_OBJECTPROGRESS, 0, (LPARAM)sectnum );	// which passes msg on to list window
		  if ( section->transforms )
			{
			transform = section->transforms->first;
			while ( transform )									// check each transform for contours
			  {
			  if ( transform->contours )
				{												// for all contours, add object to list
				contour = transform->contours->first;
				while ( contour )
				  {												// only include contour names that match limit
				  if ( MatchLimit( contour->name, limitObjectList ) )
					{											// or update contours if already there
					object = CurrObjects->Match( contour->name );
					if ( object )
						{
						object->contour_count++;
						if ( object->lastSection < sectnum ) object->lastSection = sectnum;
						}
					else {										// object not found...
						object = new VRMLObject();				// create new object list entry
						strcpy(object->name,contour->name);
						object->diffuseColor = contour->border;	// fill-in info
						object->firstSection = sectnum;
						object->lastSection = sectnum;
						object->contour_count = 1;				// this is first contour of object
						CurrObjects->Add( object );				// add to this list
						}
					if ( listObjectSurfarea || listObjectFlatarea || listObjectVolume ) 
						{
						c = new Contour( *contour );		// make sure don't edit curr section contour
						c->InvNform( transform->nform );	// do computations for list data columns
						length = 0.0; area = 0.0;
						if ( listObjectSurfarea || listObjectFlatarea ) length = c->Length();
						if ( listObjectFlatarea || listObjectVolume ) c->GreensCentroidArea( x, y, area );
						object->surface_area += length*section->thickness;
						if ( c->closed ) object->flat_area += area;
						else object->flat_area += length*section->thickness;
						object->volume += area*section->thickness;
						delete c;
						}
					}
				  contour = contour->next;					// do next contour in section
				  }
				}
			  transform = transform->next;
			  } // end while transform
			}
		  PutSection( section, false, false, false );		// free section memory
		  }
		else break;											// if no more sections, quit loop
		}													// otherwise end while and do next section

	PostMessage( appWnd, UM_OBJECTDONE, 0, 0 );			// PostMessage to objectWindow directly won't always work
														// apparently because only main window has msg loop
	return 0;												
}

void ObjectListThreadBegin( void )	// invoke background thread for filling ditance list
{
	DWORD dwThreadID;
	AbortObject = false;
	hObjectListThread = CreateThread( NULL, 0, ObjectListThread, NULL, 0, &dwThreadID);
	if ( hObjectListThread == NULL ) ErrMsgOK( ERRMSG_RUN_FAILED, NULL );
	else SetThreadPriority( hObjectListThread, THREAD_PRIORITY_BELOW_NORMAL );
}

void ObjectListThreadEnd( void )				// release window menu, etc. and close thread handle
{
	if ( hObjectListThread )								// when thread completes, close handle and
		{
   		CloseHandle( hObjectListThread );
   		hObjectListThread = NULL;
		AbortObject = false;								// OK to post to objectWindow directly here
		PostMessage( objectWindow, UM_OBJECTDONE, 0, 0 );	// because this is not part of thread procedure!
		}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK GenerateProgressProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char dlgItemTxt[64];

	switch (message)					// dialog procedure for showing tread progress and allowing user abort
		{
		case WM_INITDIALOG:
			SetWindowText( hwndDlg, "Generating 3D...");
			SetFocus( hwndDlg );
			break;

		case UM_UPDATETEXT:
			SetDlgItemText( hwndDlg, ID_MSGTXT, (LPCTSTR)lParam );
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case IDOK:								// button press after end or abort terminates dialog
					Abort3D = true;
					sprintf(dlgItemTxt,"Aborting...");
					SetDlgItemText( hwndDlg, ID_MSGTXT, dlgItemTxt );
					return TRUE;
				}
		}
	return FALSE;
}

DWORD WINAPI Generate3DThread( LPVOID )				// generate 3D representations for selected objects
{
	VRMLObject *object, *sceneObject;

	object = selectedObjects->first;
	while ( object && !Abort3D )					// inform user of progress but use Post to avoid deadlock
		{
		PostMessage( generateProgressDlg, UM_UPDATETEXT, 0, (LPARAM)object->name );

		switch ( CurrSeries->type3Dobject )					// generate 3D representation
			{
			case TRACES_OBJECT:
				object->CreateContours();
				break;
			case SURFACE_OBJECT:
				object->CreateBoissonnat();
				break;
			case AREAS_OBJECT:
				object->CreateAreas();
				break;
			case BOX_OBJECT:
				object->CreateBox();
				break;
			case POINTSET_OBJECT:
				object->CreatePointSet();
				break;
			case CYLINDER_OBJECT:
				object->CreateCylinder();
				break;
			case ELLIPSOID_OBJECT:
				object->CreateEllipsoid();
				break;
			case SPHERE_OBJECT:
				object->CreateSphere();
				break;
			}
	
		object = object->next;						// since extracted first, this is now next object
		}

	PostMessage( appWnd, UM_GENERATE3DDONE, 0, 0 );	// notify window so it can close thread handle
	return 0;										// ExitThread
}

void Generate3DThreadBegin( void )	// invoke background thread for filling ditance list
{
	DWORD dwThreadID;		// SHOULD CreateDialog OCCUR INSIDE THREAD PROCESS TO PREVENT HANGING??
	Abort3D = false;
	generateProgressDlg = CreateDialog( appInstance, "Progress", appWnd, (DLGPROC)GenerateProgressProc );
	hGenerate3DThread = CreateThread( NULL, 0, Generate3DThread, NULL, 0, &dwThreadID);
	if ( hGenerate3DThread == NULL ) ErrMsgOK( ERRMSG_RUN_FAILED, NULL );
	else SetThreadPriority( hGenerate3DThread, THREAD_PRIORITY_BELOW_NORMAL );
}

void Generate3DThreadEnd( void )				// close thread handle and finished with 3D object generation
{
	VRMLObject *object, *sceneObject;
	bool reset = true;

	if ( hGenerate3DThread )									// close handle of completed thread
		{
   		CloseHandle( hGenerate3DThread );
   		hGenerate3DThread = NULL;
		}

	if ( !Abort3D && selectedObjects->Number() )
		{
		if ( !CurrScene ) CurrScene = new VRMLScene();			// create new scene if necessary
		if ( CurrScene->objects )								// reset scene only if scene is empty
			if ( CurrScene->objects->Number() ) reset = false;

		object = selectedObjects->first;
		while ( object )
			{
			selectedObjects->Extract( object );					// remove object from selected list
			if ( CurrScene->objects )
				{
				sceneObject = CurrScene->objects->Match( object->name );
				if ( sceneObject )								// object already exists in scene!
					{
					CurrScene->objects->Extract( sceneObject );	// remove and delete it
					delete sceneObject;
					}
				}

			if ( object->vertices->total ) CurrScene->Add( object );// add to scene list
			else delete object;									// but only if not empty

			object = selectedObjects->first;					// continue with remainder of list
			}

		if ( IsWindow( openGLWindow ) )							// show 3D Scene window
			{
			CurrScene->Compile();								// recompile into openGL
			if ( reset ) PostMessage( openGLWindow, WM_COMMAND, CM_RESETSCENE, 0 );
			else InvalidateRect(openGLWindow,NULL,FALSE);		// update display of scene
			}
		else openGLWindow = MakeOpenGLWindow();					// if necessary open scene window
	
		if ( IsWindow( objectWindow ) ) ResetSceneIcons( objectList );	// update icons of new scene items
		}

	DestroyWindow( generateProgressDlg );						// terminate progress dialog box
	delete selectedObjects;										// clean up memory
	selectedObjects = NULL;
	Abort3D = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double Distance( VRMLObject *object1, VRMLObject *object2 )		// compute the minimum distance between 2 objects
{																// as part of DistanceList thread
	Contour *c1, *c2;
	Point *p1, *p2, *q1, *q2;
	double d, distance, dxy, dz;
																// ONLY THE DISTANCE BETWEEN CONTOURS IS COMPUTED
	distance = MAX_FLOAT;										// NO ALLOWANCE IS MADE FOR INSIDE OR OUTSIDE!
	if ( object1->contours && object2->contours )
		{
		c1 = object1->contours->first;
		while ( c1 && (distance > 0.0) )						// quit if intersection found
			{
			if ( c1->points )
				{
				p1 = c1->points->first;
				c2 = object2->contours->first;
				while ( c2 && (distance > 0.0) )				// quit if intersection found
					{
					if ( c2->points )
						{
						p2 = c2->points->first;
						dz = p2->z - p1->z;						// ASSUMES: contour segments lie in parallel planes
						d = dz*dz;
						if ( d < distance )						// only need to compute distance if close
							{
							dxy = c1->MinSqrdDistance( c2 );
							d = d + dxy;
							if ( d < distance ) distance = d;	// remember minimum squared distance
							}
						}	// end if ( c2->points
					c2 = c2->next;
					}
				}	// end if ( c1->points
			c1 = c1->next;
			}
		}

	return( sqrt(fabs(distance)) );								// save sqrt for end to save time
}

DWORD WINAPI DistanceListThread( LPVOID )				// fill interobjects distances list
{
	InterObject *interobject;
	VRMLObjects *series_objects;
	VRMLObject *object, *object1, *object2;
	Section *section;
	Transform *transform;
	Contour *contour, *ocontour;
	Point *p;
	int sectnum, i;
	double d, z;										// NOTE: can't post messages directly to list window
	bool report;										// so progress is reported by messaging through main window

	series_objects = new VRMLObjects();					// create object list
	sectnum = 0;
	while ( !AbortDistances )
		{
		section = GetNextSectionBetween( sectnum, MAX_INT32 );
		if ( section )
		  {
		  if ( section->transforms )
			{
			z = CurrSectionsInfo->ZDistance( section->index, CurrSeries->zMidSection );
			transform = section->transforms->first;
			while ( transform )									// check each transform for contours
			  {
			  if ( transform->contours )
				{												// for all contours, add object to list
				contour = transform->contours->first;
				while ( contour )
					{													// only include object that match row limits
					if ( MatchLimit( contour->name, limitLeftDistanceList )
							|| MatchLimit( contour->name, limitRightDistanceList ) )
						{
						object = series_objects->Match( contour->name );// update contours if already there
						if ( object == NULL )
							{											// object not found...
							object = new VRMLObject();					// create new object list entry
							strcpy(object->name,contour->name);
							series_objects->Add( object );				// add to object list
							}
						ocontour = new Contour( *contour );				// copy contour so can remember it
						ocontour->InvNform( transform->nform );			// transform into section
						if ( ocontour->points )
							{
							p = ocontour->points->first;				// store z-distance of points
							p->z = z;
							}											// add contour to object
						if ( !object->contours ) object->contours = new Contours();
						object->contours->Add( ocontour );
						}
					contour = contour->next;					// do next contour in section
					}
				}
			  transform = transform->next;
			  } // end while transform
			}
		  PutSection( section, false, false, false );			// free section memory
		  }
		else break;												// if no more sections, quit loop
		}														// otherwise end while and do next section

/*	contour = CurrSeries->zcontours->first;							
	while ( contour )											// Include z-traces to distance list??
		{
		if ( MatchLimit( contour->name, limitLeftDistanceList )
				|| MatchLimit( contour->name, limitRightDistanceList ) )
			{
			ocontour = new Contour( *contour );
			if ( ocontour->points )
				{
				p = ocontour->points->first;						// sum length contribution from all segments
				while ( p )
					{
					z = CurrSectionsInfo->ZDistance( (int)p->z, CurrSeries->zMidSection );
					p->z = z;
					p = p->next;
					}
				:								// CONFLICT WITH MATCHING OBJECT NAMES!
*/
												
	if ( InterObjectDistances ) delete InterObjectDistances;
	InterObjectDistances = new InterObjects();
	i = 0;
	if ( series_objects->Number() > 1 )			// USE OF C runtime "strcpy" and "strcmp" COULD LEAD TO MEMORY LEAK?
		{										// MAY NEED TO CREATE THREAD WITH C runtime "beginthread"
		object1 = series_objects->first;
		while ( object1 && !AbortDistances )				// put each distance into list...
			{
			if ( MatchLimit( object1->name, limitLeftDistanceList ) )	// object belongs in left column
				{
				object2 = series_objects->first;
				while ( object2 && !AbortDistances )
					{
					if ( strcmp( object1->name, object2->name )		// object is different and belongs
						&& MatchLimit( object2->name, limitRightDistanceList ) )// in right column
						{
						interobject = new InterObject();			// create list entry
						strcpy(interobject->object1, object1->name);
						strcpy(interobject->object2, object2->name);
																// tell appWnd to tell list window to display progress
						PostMessage( appWnd, UM_DISTANCEPROGRESS, 0, (LPARAM)i++ );
						interobject->distance = Distance( object1, object2 );
						InterObjectDistances->Add( interobject );	// add to distances list
						}
					object2 = object2->next;
					}
				}											// and do next pair of objects
			object1 = object1->next;
			}
		}
	
	delete series_objects;									// cleanup list of objects and contours

	PostMessage( appWnd, UM_DISTANCEDONE, 0, 0 );			// notify window so it can close thread handle
	return 0;												// ExitThread
}

void DistanceListThreadBegin( void )	// invoke background thread for filling ditance list
{
	DWORD dwThreadID;
	AbortDistances = false;
	hDistanceListThread = CreateThread( NULL, 0, DistanceListThread, NULL, 0, &dwThreadID);
	if ( hDistanceListThread == NULL ) ErrMsgOK( ERRMSG_RUN_FAILED, NULL );
	else SetThreadPriority( hDistanceListThread, THREAD_PRIORITY_BELOW_NORMAL );
}

void DistanceListThreadEnd( void )				// clean up thread handle and update listview
{
	if ( hDistanceListThread )								// when thread completes, close handle
		{
   		CloseHandle( hDistanceListThread );
   		hDistanceListThread = NULL;
		PostMessage( distanceWindow, UM_DISTANCEDONE, 0, 0 );	// now tell list window to fill list
		}
}

