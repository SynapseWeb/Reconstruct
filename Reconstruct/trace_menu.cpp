/////////////////////////////////////////////////////////////////////////////////////
//	This file contains the routines to perform the menu functions in the Trace menu.
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
// modified 3/07/05 by JCF (fiala@bu.edu)
// -+- change: Added InvalidateAllViews() to handle rerendering cases when have a domain selected.
// modified 3/10/05 by JCF (fiala@bu.edu)
// -+- change: Modified SimplifySelected to report when nothing changes.
// modified 4/27/05 by JCF (fiala@bu.edu)
// -+- change: Modified CmPasteSelected() to allow paste of domain boundary from the clipboard.
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change:  traceList col widths set by user are remember when refill list. 
// modified 11/1/05 by JCF (fiala@bu.edu)
// -+- change:  modified CmZoomSelected to only fill 24% of window instead of 96%. 
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Modified SetDefaultName params.
// modified 3/15/06 by JCF (fiala@bu.edu)
// -+- change: Fixed mode of dialog boxes evoked from Trace List.
// modified 4/20/06 by JCF (fiala@bu.edu)
// -+- change: Added check for mismatched traces when determining correspondences.
// modified 4/25/06 by JCF (fiala@bu.edu)
// -+- change: Added CM_REFRESHLIST and CM_INFOLIST to TraceList.
// modified 5/3/06 by JCF (fiala@bu.edu)
// -+- change: Added CmSmoothSelected() for smoothing traces.
// modified 6/22/06 by JCF (fiala@bu.edu)
// -+- change: Fixed bug in calibrate associated with updating the domain list.
// -+- change: Fixed CmZoomSelected when contour is a single point.
// modified 11/16/06 by JCF (fiala@bu.edu)
// -+- change: Added new CmFindTrace() function for trace menu.
// modified 4/23/07 by JCF (fiala@bu.edu)
// -+- change: Modified CmFindTrace() to split out ShowSectionTrace() to be also called from Object List.
// modified 7/16/07 by JCF (fiala@bu.edu)
// -+- change: Added enable of Maximize sys menu cmd in WM_CREATE of Trace List Window.
//

#include "reconstruct.h"


void CmSelectAll( void )					// select all the contours on the front displayed section
{												
	if ( CurrSeries )
	  if ( FrontView != DomainView )			// do only if FrontView is not NULL or DomainView
		{
		FrontView->section->SelectAll();
		FrontView->needsDrawing = true;
		InvalidateRect( appWnd, NULL, FALSE );
		}
}

void CmUnSelectAll( void )					// unselect all contours, 
{												
	if ( CurrSeries )		
	  if ( FrontView != DomainView )			// but don't mess with DomainSection's active transform!
		{
		FrontView->section->UnSelectAll();
		FrontView->needsDrawing = true;
		InvalidateRect( appWnd, NULL, FALSE );
		}
}

void CmZoomSelected( void )								//zoom into the selected contours
{
	Point min, max, allmin, allmax;
	Contour *contour, *c;
	RECT r;
	int sh;
	double x, y, w, h, fill;

	if ( FrontView )
	  if ( FrontView->section)
		if ( FrontView->section->active )			// do only if there are active contours
		  if ( FrontView->section->active->contours )
			{
			allmin.x = MAX_FLOAT;						// allmin, allmax will hold final extents
			allmin.y = MAX_FLOAT;
			allmax.x = -MAX_FLOAT;
			allmax.y = -MAX_FLOAT;
			contour = FrontView->section->active->contours->first;
			while ( contour )
				{
				c = new Contour( *contour );					// create a copy of contour
				c->InvNform( FrontView->section->active->nform );// transform into section
				c->Extent( &min, &max );						//  find extremes...
				delete c;										// delete transformed contour
				if ( min.x < allmin.x ) allmin.x = min.x;		// accummulate min, max
				if ( min.y < allmin.y ) allmin.y = min.y;
				if ( max.x > allmax.x ) allmax.x = max.x;
				if ( max.y > allmax.y ) allmax.y = max.y;
				contour = contour->next;						// use all active contours
				}

			if ( IsWindow( statusbarWindow ) )		// excepting the space taken up by the status bar
				{
				GetWindowRect( statusbarWindow, &r );
				sh = r.bottom - r.top;
				}
			else sh = 0;							// zoom only if feasible
			
			if ( ((allmin.x < allmax.x) || (allmin.y < allmax.y)) && (FrontView->height > sh) )
				{													// save current zoom
				LastZoom.pixel_size = CurrSeries->pixel_size;
				LastZoom.offset_x = CurrSeries->offset_x;
				LastZoom.offset_y = CurrSeries->offset_y;

				w = (double)FrontView->width;					// compute new zoom to fill 24% of viewport
				h = (double)FrontView->height;
				fill = 0.24; 									// it is not possible for both x and y to be zero!
				x = (allmax.x - allmin.x)/(fill*w);
				y = (allmax.y - allmin.y)/(fill*(h-sh));
				if ( x > y )									// use larger of x or y to resize view
						{										// and leave (1-fill)/2 border at edges
						CurrSeries->pixel_size = x;
						CurrSeries->offset_x = allmin.x - ((1.0-fill)/2)*x*w;
						y = x*(h-sh) - (allmax.y-allmin.y);
						CurrSeries->offset_y = allmin.y - y/2.0;	// also center y direction		
						}
					else {
						CurrSeries->pixel_size = y;
						CurrSeries->offset_y = allmin.y - ((1.0-fill)/2)*y*(h-sh) - y*sh;
						x = y*w - (allmax.x-allmin.x);	
						CurrSeries->offset_x = allmin.x - x/2.0;
						}
				InvalidateAllViews();								// update views
				InvalidateRect( appWnd, NULL, FALSE );
				}
			}
}

void FillTraceList( HWND list, Section *section )		// fill listview with contours from section
{
	Transform *transform;
	Contour *contour, *xycontour;
	Point min, max;
	LV_ITEM lvi;
	LV_COLUMN column;
	int i, colwidth[MAX_TRACELISTCOLS];
	double x, y, z, area, length, cx, cy;
	char txt[MAX_COMMENT];

	SendMessage(list, LVM_DELETEALLITEMS, 0, 0 );	// clear the list view

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;	// setup columns masks 
	column.fmt = LVCFMT_LEFT; column.pszText = txt;	column.cchTextMax = MAX_COMMENT;

	for (i=MAX_TRACELISTCOLS-1; i>0; i--)				// clear all columns
		{
		colwidth[i] = 100;								// default column width
		if ( SendMessage(list, LVM_GETCOLUMN, i, (LPARAM)(&column) ) )
			{
			colwidth[i] = column.cx;					// if exists, remember old width
			SendMessage(list, LVM_DELETECOLUMN, i, 0);	// delete column from listview
			}
		}

	i = 1;
	if ( CurrSeries->listTraceComment )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Comment");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listTraceLength )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Length");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listTraceArea )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Area");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listTraceCentroid )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Centroid X");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Centroid Y");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listTraceExtent )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Min X");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Min Y");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Max X");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Max Y");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listTraceZ )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Z-Distance");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listTraceThickness )
		{
		column.cx = colwidth[i]; column.iSubItem = i; strcpy(txt,"Thickness");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}

	z = 0.0;
	if ( CurrSectionsInfo ) z = CurrSectionsInfo->ZDistance( section->index, CurrSeries->zMidSection );

	lvi.iItem = 0;
	if ( section->transforms )							// find and list every contour in section
		{
		transform = section->transforms->first;
		while ( transform )
			{
			if ( transform->contours )
				{
				contour = transform->contours->first;
				while ( contour )
				  {
				  if ( MatchLimit( contour->name, limitTraceList ) )							
					{											// use lParam value to store contourId for ability
					lvi.lParam = contour->Id;					// to reference specific contour in the section
					lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
					lvi.state = 0; lvi.stateMask = 0; lvi.iImage = 0;
					if ( !contour->simplified ) lvi.iImage = 2;
					if ( !contour->closed ) lvi.iImage += 1;
					if ( contour->hidden ) lvi.iImage = 4;
					lvi.iSubItem = 0; lvi.pszText = contour->name;
					lvi.iItem = SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)(&lvi));

					lvi.mask = LVIF_TEXT;						// now set text of other columns
					i = 1;
					if ( CurrSeries->listTraceComment )
						{
						lvi.iSubItem = i++; lvi.pszText = contour->comment;	// display comment
						SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
						}
					if ( CurrSeries->listTraceLength || CurrSeries->listTraceArea || CurrSeries->listTraceCentroid
							|| CurrSeries->listTraceExtent )
						{
						xycontour = new Contour( *contour );	// transform contour into section coord.
						xycontour->InvNform( transform->nform );// to correctly calculate centroid + length
						if ( CurrSeries->listTraceLength )
							{
							length = xycontour->Length();
							lvi.iSubItem = i++; lvi.pszText = txt;	// display length
							sprintf(txt,"%1.*g",Precision, length);
							SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
							}
						if ( CurrSeries->listTraceArea || CurrSeries->listTraceCentroid )
						  {
						  xycontour->GreensCentroidArea( x, y, area );
						  if ( CurrSeries->listTraceArea )
							{
							lvi.iSubItem = i++; lvi.pszText = txt;	// display area
							sprintf(txt,"%1.*g",Precision, area);
							SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
							}
						  if ( CurrSeries->listTraceCentroid )
							{
							lvi.iSubItem = i++; lvi.pszText = txt;	// display centroid x
							sprintf(txt,"%1.*g",Precision, x);
							SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));						
							lvi.iSubItem = i++; lvi.pszText = txt;	// display centroid y
							sprintf(txt,"%1.*g",Precision, y);
							SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
							}
						  }
						if ( CurrSeries->listTraceExtent )
							{
							xycontour->Extent( &min, &max );
							lvi.iSubItem = i++; lvi.pszText = txt;	// display min x
							sprintf(txt,"%1.*g",Precision, min.x);
							SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
							lvi.iSubItem = i++; lvi.pszText = txt;	// display min y
							sprintf(txt,"%1.*g",Precision, min.y);
							SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
							lvi.iSubItem = i++; lvi.pszText = txt;	// display max x
							sprintf(txt,"%1.*g",Precision, max.x);
							SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
							lvi.iSubItem = i++; lvi.pszText = txt;	// display max y
							sprintf(txt,"%1.*g",Precision, max.y);
							SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
							}
						delete xycontour;							// free contour memory
						}

					if ( CurrSeries->listTraceZ )
						{
						lvi.iSubItem = i++; lvi.pszText = txt;		// display z position
						sprintf(txt,"%1.*g",Precision, z);
						SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
						}
					if ( CurrSeries->listTraceThickness )
						{
						lvi.iSubItem = i; lvi.pszText = txt;		// display thickness
						sprintf(txt,"%1.*g",Precision, section->thickness);
						SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
						}
					}				// end if MatchLimit
				  contour = contour->next;					// do next contour
				  }
				}
			transform = transform->next;			// do next transform
			}
		  }
}

void SelectTraces( HWND list )		// select listview selections in Frontview
{
	Contour *contour;
	bool found = false;
	LV_ITEM lvi;									// will extract lParam of selected items

	lvi.mask = LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	while ( lvi.iItem >= 0 )
		{														// an item was selected, retrieve contour
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));		// get the lParam (ContourId) of iItem
		contour = FrontView->section->SelectContourId( lvi.lParam );	// and select contour in section with matching Id
		if ( contour )											// make sure contour exists!
			{
			lvi.iImage = 0; lvi.iSubItem = 0;
			if ( !contour->simplified ) lvi.iImage = 2;
			if ( !contour->closed ) lvi.iImage += 1;
			if ( contour->hidden )
				{
				if ( !found ) FrontView->section->PushUndoState(); // save undo state only once at beginning
				FrontView->section->hasChanged = true;			 // making some hidden contours visible!
				contour->hidden = false;
				}
			found = true;
			FrontView->needsDrawing = true;
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
			}
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		}
}

void HideListTraces( HWND list, bool hideit )		// hide/unhide listview selection
{
	Contour *contour;
	bool found = false;
	LV_ITEM lvi;									// will extract lParam of selected items

	lvi.mask = LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	while ( lvi.iItem >= 0 )
		{														// an item was selected, retrieve contour
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));		// get the lParam (ContourId) of iItem
		contour = FrontView->section->FindContourId( lvi.lParam );	// and select contour in section with matching Id
		if ( contour )											// make sure contour exists!
			{
			lvi.iImage = 0; lvi.iSubItem = 0;
			if ( !contour->simplified ) lvi.iImage = 2;
			if ( !contour->closed ) lvi.iImage += 1;
			if ( contour->hidden != hideit )
				{
				if ( !found ) FrontView->section->PushUndoState();	// save undo only once
				found = true;
				FrontView->section->hasChanged = true;			// some contours changed hidden status!
				contour->hidden = hideit;
				if ( contour->hidden ) lvi.iImage = 4;
				FrontView->needsDrawing = true;
				SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
				}
			}
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		}

	if ( found )
		{
		FrontView->section->UnselectHidden();
		InvalidateRect( appWnd, NULL, FALSE );
		}
}

void EditTraces( HWND list )		// edit attributes of listview selections
{
	Contour *contour;
	int num_selected;
	LV_ITEM lvi;
	char name[MAX_CONTOUR_NAME];

	DlgContour = new Contour();						// use as global value holder when call dialog

	lvi.mask = LVIF_PARAM;								// go through list of selections
	num_selected = 0;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	while ( lvi.iItem >= 0 )
		{														// an item was selected, retrieve contour
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));		// get the lParam (ContourId) of iItem
		contour = FrontView->section->FindContourId( lvi.lParam );	// and find in section with matching Id
		if ( contour )
		  {
		  if ( num_selected == 0 )		// use first selection to fill DlgContour
			{
			strcpy(DlgContour->name,contour->name);
			strcpy(DlgContour->comment,contour->comment);
			DlgContour->border = contour->border;					// set attributes accordingly for dialog
			DlgContour->fill = contour->fill;
			DlgContour->mode =	contour->mode;
			DlgContour->closed = contour->closed;
			DlgContour->hidden = contour->hidden;
			DlgContour->simplified = contour->simplified;
			ChangeClosed = true;									// flags used to signal don't change
			ChangeSelected = true;									// ...initially assume will change
			ChangeSimplified = true;
			}
		else							// found first one already, see if others have differences...
			{
			if ( strcmp(DlgContour->name,contour->name) )		// contour names differ
				strcpy(DlgContour->name,"*");					// so use wildcard to match all names
			if ( strcmp(DlgContour->comment,contour->comment) )	// comments differ
				strcpy(DlgContour->comment,"*");				// so use wildcard to match all comments
			if ( DlgContour->closed != contour->closed )
				ChangeClosed = false;							// dont change closed state
			if ( DlgContour->hidden != contour->hidden )
				ChangeHidden = false;							// dont change closed state
			if ( DlgContour->simplified != contour->simplified )
				ChangeSimplified = false;						// dont change simplified state
			if ( DlgContour->border == contour->border )
				;
			else DlgContour->border.negate();					// dont change border color
			if ( DlgContour->fill == contour->fill )
				;
			else DlgContour->fill.negate();						// dont change fill color
			if ( DlgContour->mode*contour->mode < 0 )
				ChangeSelected = false;							// dont change fill selected
			if ( abs(DlgContour->mode) != abs(contour->mode) )
				DlgContour->mode =  R2_BLACK;					// dont change fill mode
			}
		  num_selected++;				// count number of contours that will be edited
		  }
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		}	// end while items selected
	
	sprintf(InputDlgName,"Changing %d traces to...",num_selected);
	if ( num_selected )
	  if ( DialogBox( appInstance, "AttributesDlg", list, (DLGPROC)AttributesDlgProc ) == IDOK )
		{
		FrontView->section->PushUndoState();				// go back through selected items in trace List
		FrontView->section->hasChanged = true;				// and modify selected contours using DlgContour
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		while ( lvi.iItem >= 0 )
			{													// get the lParam (ContourId) of iItem
			SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));	// find it in section with matching Id
			contour = FrontView->section->FindContourId( lvi.lParam );	
			if ( contour )
				{												// parse any special characters in name
				FrontView->section->SetDefaultName(name,DlgContour->name,false);
				if ( name[0] == '*' )							// concatenate if wild card was used
					strncat(contour->name,name+1,MAX_CONTOUR_NAME-strlen(contour->name));
				else strcpy(contour->name,name);					// otherwise just copy contour name
				if ( DlgContour->comment[0] == '*' )			// concatenate if wild card was used
					strncat(contour->comment,DlgContour->comment+1,MAX_COMMENT-strlen(contour->comment));
				else strcpy(contour->comment,DlgContour->comment);	// otherwise just copy comment
				if ( ChangeClosed )
					contour->closed = DlgContour->closed;		// change open/close state
				if ( ChangeHidden )
					contour->hidden = DlgContour->hidden;		// change hide state
				if ( ChangeSimplified )
					contour->simplified = DlgContour->simplified;// change simplified state
				if ( !DlgContour->border.invalid() ) 
					contour->border = DlgContour->border;		// change border color
				if ( !DlgContour->fill.invalid() )
					contour->fill = DlgContour->fill;			// change fill color
				if ( abs(DlgContour->mode) != R2_BLACK )
					contour->mode = abs(DlgContour->mode);		// change fill mode
				if ( ChangeSelected )
					if ( DlgContour->mode < 0 )					// change fill selected to match DlgContour
						contour->mode = -abs(contour->mode);
					else contour->mode = abs(contour->mode);
				}
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			}	// end while items selected

		FrontView->needsDrawing = true;					// finally display changes on section
		FrontView->section->UnselectHidden();			// but hidden traces should no longer be selected
		FillTraceList( list, FrontView->section );
		InvalidateRect( appWnd, NULL, FALSE );
		}

	delete DlgContour;		// done with this data structure
}


LRESULT APIENTRY ListTracesProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pnmh;
	LV_COLUMN column;
	LV_ITEM lvi;
	LV_KEYDOWN *key;
	RECT r;
	HMENU hMenu;
	HIMAGELIST himl;			
	int i, w, h, sectnum;
	OPENFILENAME ofn;								// common dialog box structure
	char txt[MAX_PATH], filename[MAX_PATH];

	switch (message)
		{											// initialize the list view section info
		case WM_CREATE:
			GetClientRect( hWnd, &r );
			w = r.right-r.left-10; h = r.bottom-r.top-10,
			traceList = CreateWindow( WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER | LVS_SORTASCENDING,
														5, 5, w, h, hWnd, NULL, appInstance, NULL);
													//use (GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), TRUE, 4, 1) to auto resize icons
			himl = ImageList_Create(16,16,TRUE,5,1);
			ImageList_AddIcon(himl, LoadIcon(appInstance, "TraceIcon"));
			ImageList_AddIcon(himl, LoadIcon(appInstance, "OpenIcon"));
			ImageList_AddIcon(himl, LoadIcon(appInstance, "LoopIcon"));
			ImageList_AddIcon(himl, LoadIcon(appInstance, "OLoopIcon"));
			ImageList_AddIcon(himl, LoadIcon(appInstance, "HiddenIcon"));
			SendMessage(traceList, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)(himl));	// setup icons
			column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; 
			column.fmt = LVCFMT_LEFT; column.pszText = txt;							// setup columns
			strcpy(txt,"Trace"); column.cx = 100;
			for ( column.iSubItem = 0; column.iSubItem < MAX_TRACELISTCOLS; column.iSubItem++ )
				SendMessage(traceList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			FillTraceList( traceList, FrontView->section );
			sprintf(txt,"Section %d traces %s",FrontView->section->index,limitTraceList);
			SetWindowText(hWnd,txt);
			SetFocus( traceList );
			hMenu = GetSystemMenu( hWnd, FALSE );						// allow Maximize on titlebar menu
			EnableMenuItem( hMenu, SC_MAXIMIZE, MF_ENABLED );
			break;

		case WM_SIZE:								// resize list view control to match dialog box
			GetClientRect( hWnd, &r );
			SetWindowPos( traceList, 0, 0, 0, r.right-r.left-10, r.bottom-r.top-10, SWP_NOMOVE | SWP_NOOWNERZORDER);
			break;
			
		case WM_DESTROY:
			traceWindow = NULL;						// clear global handle pointer
			delete CurrContours;
			CurrContours = NULL;
			hMenu = GetMenu( appWnd );
			CheckMenuItem( hMenu, CM_LISTTRACES, MF_BYCOMMAND | MF_UNCHECKED );	// uncheck menu item
			SetFocus(appWnd);						// return to main window
			break;

		case WM_NOTIFY:								// process listview notification messages
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case LVN_KEYDOWN:
					key = (LV_KEYDOWN *)lParam;		// allow control+TAB to switch active window
					if ( (key->wVKey==VK_TAB) && (GetKeyState(VK_CONTROL) & 0x8000) ) CmNextWindow( traceWindow );
					if ( key->wVKey==VK_DELETE )
						{
						FrontView->section->UnSelectAll();	// same deletion process as WM_COMMAND below
						SelectTraces( traceList );
						CmDeleteSelected();
						}
					if ( key->wVKey !=  VK_RETURN )	// fall through to double-click if Enter key pressed
						break;

				case NM_DBLCLK:						// user double-clicks or presses enter, select them
					FrontView->section->UnSelectAll();
					SelectTraces( traceList );
					InvalidateRect( appWnd, NULL, FALSE );
					SetFocus(appWnd);				// and switch to main window
					return TRUE;
				}
			break;

		case WM_SETFOCUS:
			SetFocus( traceList );					// activate list view if window activated
			break;

		case UM_UPDATESECTION:						// the front section has changed!
			FillTraceList( traceList, FrontView->section );
			sprintf(txt,"Section %d traces %s",FrontView->section->index,limitTraceList);
			SetWindowText(hWnd,txt);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{												// default pushbutton: select traces
				case CM_SELECTTRACES:
					FrontView->section->UnSelectAll();
					SelectTraces( traceList );
					InvalidateRect( appWnd, NULL, FALSE );
					SetFocus(appWnd);							// and switch to main window
					break;

				case CM_REFRESHLIST:							// regenerate the list
					FillTraceList( traceList, FrontView->section );
					break;

				case CM_INFOLIST:								// display count of items
					DisplayListInfo( traceList );
					break;

				case CM_SAVELIST:
					sprintf(filename,"traces%d.csv",FrontView->section->index);
					ZeroMemory(&ofn, sizeof(ofn));						// clear the OPENFILENAME fields
					ofn.lStructSize = sizeof(ofn);						// set only the necessary values
					ofn.hwndOwner = appWnd;
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "All Files\0*.*\0Comma Separated Value file\0*.csv\0\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrTitle = "Save Trace List As...";
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY; // ask user if overwrite desired
					if ( GetSaveFileName(&ofn) == TRUE )				// display the Open dialog box
						{
						SaveList( traceList, filename );
						}
					break;

				case CM_EDITTRACES:					// change attributes of selected traces
					EditTraces( traceList );
					break;

				case CM_HIDETRACES:					// hide selected traces from view
					HideListTraces( traceList, true );
					break;

				case CM_UNHIDETRACES:				// unhide selected traces from view
					HideListTraces( traceList, false );
					break;

				case CM_DELETETRACES:				// delete selected traces from section
					FrontView->section->UnSelectAll();
					SelectTraces( traceList );
					CmDeleteSelected();
					break;
				}
		}
	return( DefWindowProc(hWnd, message, wParam, lParam) );
}


HWND MakeTraceListWindow( void )			// display list for user selection of objects to view
{
	WNDCLASS wndclass;
	HWND sWnd;
	HMENU hMenu;
	RECT r;
	int width;
														// create the object list window...
	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)ListTracesProc;
	wndclass.cbClsExtra  = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance = appInstance;
	wndclass.hCursor   = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon     = NULL;							// not used for WS_EX_TOOLWINDOW
	wndclass.lpszMenuName = "TraceListMenu";
	wndclass.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wndclass.lpszClassName = "TraceListClass";
	RegisterClass(&wndclass);

	ClientRectLessStatusBar( &r );						// position window in upper left corner of client
	ClientRectToScreen( appWnd, &r );
	width = 140;										// set width based on how many data fields present
	if ( CurrSeries->listTraceComment ) width += 100;	
	if ( CurrSeries->listTraceLength ) width += 100;
	if ( CurrSeries->listTraceArea ) width += 100;
	if ( CurrSeries->listTraceCentroid ) width += 200;
	if ( CurrSeries->listTraceExtent ) width += 400;
	if ( CurrSeries->listTraceZ ) width += 100;
	if ( CurrSeries->listTraceThickness ) width += 100;
	sWnd = CreateWindowEx( WS_EX_TOOLWINDOW, "TraceListClass", "Traces", WS_OVERLAPPEDWINDOW,
								r.left+40, r.top+40, width, r.bottom-r.top-40, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

	if ( sWnd )
		{											// if window created, display it
		ShowWindow(sWnd, SW_SHOW);			// and check corresponding menu item in main window	
		hMenu = GetMenu( appWnd );
		CheckMenuItem( hMenu, CM_LISTTRACES, MF_BYCOMMAND | MF_CHECKED );
		}

	return( sWnd );
}

void CmListTraces( void )									// create or destroy trace list window
{
	HCURSOR cur;

	if ( IsWindow( traceWindow ) ) DestroyWindow( traceWindow );						
	else if ( FrontView )
		  if ( FrontView->section )			
			traceWindow = MakeTraceListWindow();			// create list window
		 // else ErrMsgOK( ERRMSG_NOOBJECTS, "" );			// otherwise report error
}

void SetFillMode( HWND hwndDlg, int nofill, int maskfill, int copyfill, int mergefill )
{
	SendDlgItemMessage(hwndDlg, ID_NOFILL, BM_SETCHECK, (WPARAM)nofill, 0);
	SendDlgItemMessage(hwndDlg, ID_MASKFILL, BM_SETCHECK, (WPARAM)maskfill, 0);
	SendDlgItemMessage(hwndDlg, ID_COPYFILL, BM_SETCHECK, (WPARAM)copyfill, 0);			
	SendDlgItemMessage(hwndDlg, ID_MERGEFILL, BM_SETCHECK, (WPARAM)mergefill, 0);
}

void FillFromDlgContour( HWND hwndDlg )
{
	SendDlgItemMessage(hwndDlg, ID_CONTOURNAME, WM_SETTEXT, 0, (LPARAM)DlgContour->name);
	SendDlgItemMessage(hwndDlg, ID_COMMENT, WM_SETTEXT, 0, (LPARAM)DlgContour->comment);
	ColorButton( GetDlgItem( hwndDlg, ID_BORDERCOLOR ), DlgContour->border );
	ColorButton( GetDlgItem( hwndDlg, ID_FILLCOLOR ), DlgContour->fill );
	if ( !ChangeClosed )									// gray fill selected state
		SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
	else if ( DlgContour->closed )						// or match DlgContour state
			SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		 else SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	if ( !ChangeHidden )									// gray fill selected state
		SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
	else if ( DlgContour->hidden )						// or match DlgContour state
			SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		 else SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	if ( !ChangeSimplified )									// gray fill selected state
		SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
	else if ( DlgContour->simplified )						// or match DlgContour state
			SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		 else SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	if ( !ChangeSelected )									// gray fill selected state
		SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
	else if ( DlgContour->mode > 0 )						// or match DlgContour state
			SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
		 else SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	if ( abs(DlgContour->mode) == R2_NOP )
		SetFillMode( hwndDlg, BST_CHECKED, BST_UNCHECKED, BST_UNCHECKED, BST_UNCHECKED );
	else if ( abs(DlgContour->mode) == R2_MASKPEN )
		SetFillMode( hwndDlg, BST_UNCHECKED, BST_CHECKED, BST_UNCHECKED, BST_UNCHECKED );
	else if ( abs(DlgContour->mode) == R2_COPYPEN )
		SetFillMode( hwndDlg, BST_UNCHECKED, BST_UNCHECKED, BST_CHECKED, BST_UNCHECKED );
	else if ( abs(DlgContour->mode) == R2_MERGEPEN )
		SetFillMode( hwndDlg, BST_UNCHECKED, BST_UNCHECKED, BST_UNCHECKED, BST_CHECKED );
	else SetFillMode( hwndDlg, BST_INDETERMINATE, BST_INDETERMINATE, BST_INDETERMINATE, BST_INDETERMINATE );
}

BOOL CALLBACK AttributesDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	CHOOSECOLOR cc;
	Contour *c;
	int i;
	char txt[64];

	switch (message)					// enter with CurrContour set from first active contour so can set values after OK
		{
		case WM_INITDIALOG:
			SetWindowText( hwndDlg, InputDlgName );					// fill in the values that will result from user OK
			FillFromDlgContour( hwndDlg );
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case ID_BORDERCOLOR:
					ZeroMemory(&cc, sizeof(CHOOSECOLOR));			// setup color dialog structure
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					cc.rgbResult = DlgContour->border.ref();		// set current color
					for (i=0; i<16; i++) CustomColors[i] = CurrSeries->borderColors[i].ref();
					cc.lpCustColors = CustomColors;					// set custom colors
					if ( ChooseColor(&cc) )							// open color dialog
						{
						DlgContour->border = Color( cc.rgbResult );// reset default color
						ColorButton( GetDlgItem( hwndDlg, ID_BORDERCOLOR ), DlgContour->border );
						for (i=0; i<16; i++) CurrSeries->borderColors[i] = Color( CustomColors[i] );
						}
					return TRUE;

				case ID_FILLCOLOR:
					ZeroMemory(&cc, sizeof(CHOOSECOLOR));			// setup color dialog structure
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					cc.rgbResult = DlgContour->fill.ref();			// set current color
					for (i=0; i<16; i++) CustomColors[i] = CurrSeries->fillColors[i].ref();
					cc.lpCustColors = CustomColors;					// set custom colors
					if ( ChooseColor(&cc) )							// open color dialog
						{
						DlgContour->fill = Color( cc.rgbResult );	// reset default color
						ColorButton( GetDlgItem( hwndDlg, ID_FILLCOLOR ), DlgContour->fill );
						for (i=0; i<16; i++) CurrSeries->fillColors[i] = Color( CustomColors[i] );
						}
					return TRUE;

				case ID_CLOSED:		// if user clicks on INDETERMINATE state then must change it
					i = SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_GETCHECK, 0, 0);
					if ( i == BST_INDETERMINATE ) ChangeClosed = true;
					if ( i == BST_CHECKED )
						{
						SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						DlgContour->closed = false;
						}
					else {
						SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						DlgContour->closed = true;
						}
					return TRUE;

				case ID_HIDDEN:		// if user clicks on INDETERMINATE state then must change it
					i = SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_GETCHECK, 0, 0);
					if ( i == BST_INDETERMINATE ) ChangeHidden = true;
					if ( i == BST_CHECKED )
						{
						SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						DlgContour->hidden = false;
						}
					else {
						SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						DlgContour->hidden = true;
						}
					return TRUE;

				case ID_SIMPLIFIED:		// if user clicks on INDETERMINATE state then must change it
					i = SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_GETCHECK, 0, 0);
					if ( i == BST_INDETERMINATE ) ChangeSimplified = true;
					if ( i == BST_CHECKED )
						{
						SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						DlgContour->simplified = false;
						}
					else {
						SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						DlgContour->simplified = true;
						}
					return TRUE;

				case ID_FILLSELECTED:
					i = SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_GETCHECK, 0, 0);
					if ( i == BST_INDETERMINATE ) ChangeSelected = true;
					if ( i == BST_CHECKED )
						SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
					else SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
					return TRUE;

				case ID_NOFILL:
					DlgContour->mode = R2_NOP;
					SetFillMode( hwndDlg, BST_CHECKED, BST_UNCHECKED, BST_UNCHECKED, BST_UNCHECKED );
					return TRUE;

				case ID_MASKFILL:
					DlgContour->mode = R2_MASKPEN;
					SetFillMode( hwndDlg, BST_UNCHECKED, BST_CHECKED, BST_UNCHECKED, BST_UNCHECKED );
					return TRUE;

				case ID_COPYFILL:
					DlgContour->mode = R2_COPYPEN;
					SetFillMode( hwndDlg, BST_UNCHECKED, BST_UNCHECKED, BST_CHECKED, BST_UNCHECKED );
					return TRUE;

				case ID_MERGEFILL:
					DlgContour->mode = R2_MERGEPEN;
					SetFillMode( hwndDlg, BST_UNCHECKED, BST_UNCHECKED, BST_UNCHECKED, BST_CHECKED );
					return TRUE;

				case ID_GETDEFAULTS:			// copy default attributes into dialog
					strcpy(DlgContour->name,CurrSeries->defaultName);
					DlgContour->border = CurrSeries->defaultBorder;
					DlgContour->fill = CurrSeries->defaultFill;
					DlgContour->mode =	CurrSeries->defaultMode;
					ChangeSelected = true;
					FillFromDlgContour( hwndDlg );
					return TRUE;

				case ID_GETCLIPBOARD:			// copy clipboard attributes into dialog
					if ( ClipboardTransform )
					  if ( ClipboardTransform->contours )
						{
						c = ClipboardTransform->contours->first;
						strcpy(DlgContour->name,c->name);
						DlgContour->border = c->border;
						DlgContour->fill = c->fill;
						DlgContour->mode =	c->mode;
						ChangeSelected = true;
						FillFromDlgContour( hwndDlg );
						}
					return TRUE;
																	// user says OK, so store values from dialog in DlgContour
				case IDOK:
					SendDlgItemMessage(hwndDlg, ID_CONTOURNAME, WM_GETTEXT, (WPARAM)MAX_CONTOUR_NAME, (LPARAM)DlgContour->name);
					RemoveIllegalChars( DlgContour->name );
					SendDlgItemMessage(hwndDlg, ID_COMMENT, WM_GETTEXT, (WPARAM)MAX_COMMENT, (LPARAM)DlgContour->comment);
					RemoveIllegalChars( DlgContour->comment );
					if ( ChangeSelected )
					  if ( SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						DlgContour->mode = -abs(DlgContour->mode);
					  else DlgContour->mode = abs(DlgContour->mode);	// change this here so can add it to mode
					EndDialog(hwndDlg, 1);
					return TRUE;

				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				}
		}
	return FALSE;
}

void CmTraceAttributes( void )				// change default color of contour interiors
{
	Contour *contour, *c;
	CHOOSECOLOR cc;
	char name[MAX_CONTOUR_NAME];
	int i;
										// do only if one or more contours have been selected
	if ( FrontView )
	  if ( FrontView->section && (FrontView->section != DomainSection) )
		if ( FrontView->section->active )
		  if ( FrontView->section->active->contours )
			{
			i = FrontView->section->active->contours->Number();			// will display how many contours are selected
			if ( i > 0 )
				{
				DlgContour = new Contour();								// use it here as global value holder
				contour = FrontView->section->active->contours->first;	// 1st selected contour is current one
				strcpy(DlgContour->name,contour->name);
				strcpy(DlgContour->comment,contour->comment);
				DlgContour->border = contour->border;					// set attributes accordingly for dialog
				DlgContour->fill = contour->fill;
				DlgContour->mode =	contour->mode;
				DlgContour->closed = contour->closed;
				DlgContour->hidden = contour->hidden;
				DlgContour->simplified = contour->simplified;
				ChangeClosed = true;									// flags used to signal don't change
				ChangeSelected = true;									// ...initially assume will change
				ChangeSimplified = true;
				contour = contour->next;
				while ( contour )										// check for more than one contour name
					{
					if ( strcmp(DlgContour->name,contour->name) )		// contour names differ
						strcpy(DlgContour->name,"*");					// so use wildcard to match all names
					if ( strcmp(DlgContour->comment,contour->comment) )	// comments differ
						strcpy(DlgContour->comment,"*");				// so use wildcard to match all comments
					if ( DlgContour->closed != contour->closed )
						ChangeClosed = false;							// dont change closed state
					if ( DlgContour->hidden != contour->hidden )
						ChangeHidden = false;							// dont change closed state
					if ( DlgContour->simplified != contour->simplified )
						ChangeSimplified = false;						// dont change simplified state
					if ( DlgContour->border == contour->border )
						;
					else DlgContour->border.negate();					// dont change border color
					if ( DlgContour->fill == contour->fill )
						;
					else DlgContour->fill.negate();						// dont change fill color
					if ( DlgContour->mode*contour->mode < 0 )
						ChangeSelected = false;							// dont change fill selected
					if ( abs(DlgContour->mode) != abs(contour->mode) )
						DlgContour->mode =  R2_BLACK;					// dont change fill mode
					contour = contour->next;
					}
				sprintf(InputDlgName,"Changing %d traces to...",i);
				if ( DialogBox( appInstance, "AttributesDlg", appWnd, (DLGPROC)AttributesDlgProc ) == IDOK )
					{
					FrontView->section->PushUndoState();
					FrontView->section->hasChanged = true;				// will modify selected contours
					contour = FrontView->section->active->contours->first;
					while ( contour )
						{												// parse any special characters in name
						FrontView->section->SetDefaultName(name,DlgContour->name,false);
						if ( name[0] == '*' )							// concatenate if wild card was used
							strncat(contour->name,name+1,MAX_CONTOUR_NAME-strlen(contour->name));
						else strcpy(contour->name,name);					// otherwise just copy contour name
						if ( DlgContour->comment[0] == '*' )			// concatenate if wild card was used
							strncat(contour->comment,DlgContour->comment+1,MAX_COMMENT-strlen(contour->comment));
						else strcpy(contour->comment,DlgContour->comment);	// otherwise just copy comment
						if ( ChangeClosed )
							contour->closed = DlgContour->closed;		// change open/close state
						if ( ChangeHidden )
							contour->hidden = DlgContour->hidden;		// change hide state
						if ( ChangeSimplified )
							contour->simplified = DlgContour->simplified;// change simplified state
						if ( !DlgContour->border.invalid() ) 
							contour->border = DlgContour->border;		// change border color
						if ( !DlgContour->fill.invalid() )
							contour->fill = DlgContour->fill;			// change fill color
						if ( abs(DlgContour->mode) != R2_BLACK )
							contour->mode = abs(DlgContour->mode);		// change fill mode
						if ( ChangeSelected )
							if ( DlgContour->mode < 0 )					// change fill selected to match DlgContour
								contour->mode = -abs(contour->mode);
							else contour->mode = abs(contour->mode);
						contour = contour->next;						// do next contour in list
						}
					FrontView->needsDrawing = true;
					FrontView->section->UnselectHidden();				// hidden traces should no longer be selected
					if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
					InvalidateRect( appWnd, NULL, FALSE );
					}
				delete DlgContour;
				}
			}
}

void CmTracePalette( void )
{
	if ( IsWindow( paletteWindow ) ) DestroyWindow( paletteWindow );
	else {
		paletteWindow = MakePaletteWindow();
		}
}

void CmCopySelected( void )				// copy active transform to clipboard transform
{												
	if ( FrontView )
	  if ( FrontView->section && (FrontView != DomainView) )
		if ( FrontView->section->active )
			{
			if ( ClipboardTransform ) delete ClipboardTransform;
			ClipboardTransform = new Transform( *(FrontView->section->active) );
			UpdateMenus();				// make sure Paste is now enabled
			}
}

void CmCutSelected( void )				// delete active transform from section, and put in clipboard
{												
	if ( FrontView )
	  if ( FrontView->section && (FrontView != DomainView) )
		if ( FrontView->section->active )
			{
			FrontView->section->PushUndoState();			// save for undo
			if ( ClipboardTransform ) delete ClipboardTransform;
			ClipboardTransform = FrontView->section->ExtractActiveTransform();
			FrontView->needsDrawing = true;
			if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
			InvalidateRect(appWnd,NULL,FALSE);			// invoke paint to redraw
			}
}

void CmPasteSelected( void )			// paste clipboard traces into section		
{
	Contour *contour;
											
	if ( FrontView && ClipboardTransform && FrontView->section )
		if ( FrontView == DomainView )				// this is a domain, so replace domain boundary
			{
			if ( FrontView->section->active && ClipboardTransform->contours )
			  if ( 	ClipboardTransform->contours->first )
				{
				FrontView->section->PushUndoState();				// save current state
				contour = new Contour( *ClipboardTransform->contours->first );	// copy first contour
				contour->FwdNform(  FrontView->section->active->nform );		// put into domain coordinates
				contour->Scale( 1.0/FrontView->section->active->image->mag );	// scale to image pixels
				delete FrontView->section->active->domain;
				FrontView->section->active->domain = contour;		// set as domain
				contour->closed = true;								// a domain contour is always closed
				contour->mode = R2_NOP;								// a domain is never filled!
				FrontView->section->hasChanged = true;
				FrontView->needsRendering = true;
				InvalidateRect(appWnd,NULL,FALSE);	// repaint domain
				}
			}
		else
			{
			FrontView->section->PushUndoState();			// save current state
			if ( FrontView->section->active )			// first reset active transform
					FrontView->section->active->isActive = false;
			FrontView->section->active = new Transform( *ClipboardTransform );
			if ( !FrontView->section->transforms ) FrontView->section->transforms = new Transforms();
			FrontView->section->transforms->Add( FrontView->section->active );
			FrontView->section->active->isActive = true;
			FrontView->section->hasChanged = true;
			FrontView->needsDrawing = true;
			if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
			InvalidateRect(appWnd,NULL,FALSE);				// use paint to redraw contours
			}
}

void CmDeleteSelected( void )			// delete active transform from section
{
	Transform *transform;
										
	if ( FrontView )
	  if ( FrontView->section && (FrontView->section != DomainSection) )
		if ( FrontView->section->active )
			{
			FrontView->section->PushUndoState();			// remember contours
			transform = FrontView->section->ExtractActiveTransform();
			delete transform;
			if ( CurrSeries->beepDeleting ) MessageBeep(0xFFFFFFFF);
			FrontView->needsDrawing = true;
			if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
			InvalidateRect(appWnd,NULL,FALSE);				// redraw without them
			}
}

void Correspondences( Points *CurrCentroids, Points *PrevCentroids )
{
	Contour *contour, *cc, *pcontour, *pc;
	Point *point;
	bool correct = true;
	double a, x, y;

	if ( CurrSection && PrevSection )							// find mathcing selected contours
		if ( CurrSection->active && PrevSection->active )
			if ( CurrSection->active->contours && PrevSection->active->contours  )
				{
				contour = CurrSection->active->contours->first;
				while ( contour )
					{
					cc = new Contour( *contour );				// transform contour into section coord.
					cc->InvNform( CurrSection->active->nform );	// to correctly calculate centroid
					pcontour = PrevSection->active->contours->first;
					pc = NULL;
					while ( pcontour )							// look for matching contour
						{
						if ( !strcmp( contour->name, pcontour->name ) )
							if ( pc == NULL ) pc = pcontour;
							else correct = false;				// if duplicate, flag error
						pcontour = pcontour->next;
						}
					if ( pc )									// found matching names
						{
						pc = new Contour( *pc );				// transform pcontour into section
						pc->InvNform( PrevSection->active->nform );
						pc->GreensCentroidArea( x, y, a );		// compute centroids and save pts
						point = new Point( x, y, 0.0 );
						PrevCentroids->Add( point );
						cc->GreensCentroidArea( x, y, a );			
						point = new Point( x, y, 0.0 );
						CurrCentroids->Add( point );
						delete pc;
						}
					else correct = false;						// didn't find match, flag error
					delete cc;
					contour = contour->next;
					}
				}

	if ( !correct ) ErrMsgSplash( ERRMSG_MISMATCHEDTRACES, "" ); // tell user s/he screwed up!
}

void CmAlignRigid( void )	// solve Nform to align translation and rotation only
{
	double *cx, *cy, *px, *py;
	int i, min_count;
	Point *cp, *pp;
	Points *CurrPoints, *PrevPoints;
	Nform *adjustment;
	Transform *transform;
	
	CurrPoints = new Points();			// keep track of correspondence points
	PrevPoints = new Points();
	Correspondences( CurrPoints, PrevPoints );	// find correspondences between selected contours

	min_count = CurrPoints->Number();				// how many correspondence points?
	if ( min_count > 1 )							// align curr to prev using these points
		{
		cx = new double[min_count];
		cy = new double[min_count];
		px = new double[min_count];
		py = new double[min_count];
		cp = CurrPoints->first;
		pp = PrevPoints->first;
		for ( i=0; i<min_count; i++ )				// fill with correspondences
			{
      		cx[i] = cp->x;  cy[i] = cp->y;
			px[i] = pp->x;  py[i] = pp->y;
			cp = cp->next;
			pp = pp->next;
			}										// compute the adjustment to put these pts in alignment

		adjustment = new Nform();
		adjustment->ComputeRigid( px, py, cx, cy, min_count );

		CurrSection->PushUndoState();
		if ( !CurrSection->alignLocked )			// move everything if section mvmt enabled
			{
			transform = CurrSection->transforms->first;
			while ( transform )						// by moving each transform
				{
				CurrSection->hasChanged = true;
				transform->nform->ApplyLinearOf( adjustment );
				transform = transform->next;
				}
			CurrView->needsRendering = true;
			}					
		else										// if section locked, just move selected contours!
			{
			CurrSection->hasChanged = true;
			CurrSection->active->nform->ApplyLinearOf( adjustment );
			CurrView->needsDrawing = true;
			}

		if ( LastAdjustment ) delete LastAdjustment;		// clear last adjustment
		LastAdjustment = adjustment;
		delete[] cx;
		delete[] cy;
		delete[] px;
		delete[] py;
	
   		InvalidateRect( appWnd, NULL, FALSE );
		}
	else ErrMsgOK( ERRMSG_NEEDTOALIGN, "" );

	delete CurrPoints;								// free memory
	delete PrevPoints;
}

void CmAlignLinear( void )			// use centroids of selected contours to align two sections
{
	double *cx, *cy, *px, *py;
	int i, min_count;
	Point *cp, *pp;
	Points *CurrPoints, *PrevPoints;
	Nform *adjustment;
	Transform *transform;
	
	CurrPoints = new Points();			// keep track of correspondence points
	PrevPoints = new Points();
	Correspondences( CurrPoints, PrevPoints );	// find correspondences between selected contours

	min_count = CurrPoints->Number();				// how many correspondence points?
	if ( min_count > 2 )							// align curr to prev using these points
		{
		cx = new double[min_count];
		cy = new double[min_count];
		px = new double[min_count];
		py = new double[min_count];
		cp = CurrPoints->first;
		pp = PrevPoints->first;
		for ( i=0; i<min_count; i++ )				// fill with correspondences
			{
      		cx[i] = cp->x;  cy[i] = cp->y;
			px[i] = pp->x;  py[i] = pp->y;
			cp = cp->next;
			pp = pp->next;
			}										// compute the adjustment to put these pts in alignment

		adjustment = new Nform();
		adjustment->ComputeMapping( px, py, cx, cy, min_count, 3 );

		CurrSection->PushUndoState();
		if ( !CurrSection->alignLocked )							// move everything is section mvmt enabled
			{
			transform = CurrSection->transforms->first;
			while ( transform )						// by moving each transform
				{
				CurrSection->hasChanged = true;
				transform->nform->ApplyLinearOf( adjustment );
				transform = transform->next;
				}
			CurrView->needsRendering = true;
			}					
		else										// if section locked, just move selected contours!
			{
			CurrSection->hasChanged = true;
			CurrSection->active->nform->ApplyLinearOf( adjustment );
			CurrView->needsDrawing = true;
			}

		if ( LastAdjustment ) delete LastAdjustment;		// clear last adjustment
		LastAdjustment = adjustment;
		delete[] cx;
		delete[] cy;
		delete[] px;
		delete[] py;
	
   		InvalidateRect( appWnd, NULL, FALSE );
		}
	else ErrMsgOK( ERRMSG_NEEDTOALIGN, "" );

	delete CurrPoints;								// free memory
	delete PrevPoints;
}

void CmAlignDeform( void )	// solve Nform to align 4 pairs of points
{
	double *cx, *cy, *px, *py;
	int i, min_count;
	Point *cp, *pp;
	Points *CurrPoints, *PrevPoints;
	Nform *adjustment;
	Transform *transform;
	
	CurrPoints = new Points();			// keep track of correspondence points
	PrevPoints = new Points();
	Correspondences( CurrPoints, PrevPoints );	// find correspondences between selected contours

	min_count = CurrPoints->Number();				// how many correspondence points?
	if ( min_count > 3 )							// align curr to prev using these points
		{
		cx = new double[min_count];
		cy = new double[min_count];
		px = new double[min_count];
		py = new double[min_count];
		cp = CurrPoints->first;
		pp = PrevPoints->first;
		for ( i=0; i<min_count; i++ )				// fill with correspondences
			{
      		cx[i] = cp->x;  cy[i] = cp->y;
			px[i] = pp->x;  py[i] = pp->y;
			cp = cp->next;
			pp = pp->next;
			}										// compute the adjustment to put these pts in alignment

		adjustment = new Nform();
		adjustment->ComputeMapping( px, py, cx, cy, min_count, 4 );

		CurrSection->PushUndoState();
		if ( !CurrSection->alignLocked )							// move everything is section mvmt enabled
			{
			transform = CurrSection->transforms->first;
			while ( transform )						// by moving each transform
				{
				CurrSection->hasChanged = true;
				transform->nform->Compose( adjustment, transform->nform, 1.0, 1.0 );
				transform = transform->next;
				}
			CurrView->needsRendering = true;
			}					
		else										// if section locked, just move selected contours!
			{
			CurrSection->hasChanged = true;
			CurrSection->active->nform->Compose( adjustment, CurrSection->active->nform, 1.0, 1.0 );
			CurrView->needsDrawing = true;
			}

		if ( LastAdjustment ) delete LastAdjustment;		// clear last adjustment
		LastAdjustment = adjustment;
		delete[] cx;
		delete[] cy;
		delete[] px;
		delete[] py;
	
   		InvalidateRect( appWnd, NULL, FALSE );
		}
	else ErrMsgOK( ERRMSG_NEEDTOALIGN, "" );

	delete CurrPoints;								// free memory
	delete PrevPoints;
}

void CmAlignQuadratic( void )	// solve Nform to align 6 pairs of points
{
	double *cx, *cy, *px, *py;
	int i, min_count;
	Point *cp, *pp;
	Points *CurrPoints, *PrevPoints;
	Nform *adjustment;
	Transform *transform;
	
	CurrPoints = new Points();			// keep track of correspondence points
	PrevPoints = new Points();
	Correspondences( CurrPoints, PrevPoints );	// find correspondences between selected contours

	min_count = CurrPoints->Number();				// how many correspondence points?
	if ( min_count > 5 )							// align curr to prev using these points
		{
		cx = new double[min_count];
		cy = new double[min_count];
		px = new double[min_count];
		py = new double[min_count];
		cp = CurrPoints->first;
		pp = PrevPoints->first;
		for ( i=0; i<min_count; i++ )				// fill with correspondences
			{
      		cx[i] = cp->x;  cy[i] = cp->y;
			px[i] = pp->x;  py[i] = pp->y;
			cp = cp->next;
			pp = pp->next;
			}										// compute the adjustment to put these pts in alignment

		adjustment = new Nform();
		adjustment->ComputeMapping( px, py, cx, cy, min_count, 6 );

		CurrSection->PushUndoState();
		if ( !CurrSection->alignLocked )							// move everything is section mvmt enabled
			{
			transform = CurrSection->transforms->first;
			while ( transform )						// by moving each transform
				{
				CurrSection->hasChanged = true;
				transform->nform->Compose( adjustment, transform->nform, 1.0, 1.0 );
				transform = transform->next;
				}
			CurrView->needsRendering = true;
			}					
		else										// if section locked, just move selected contours!
			{
			CurrSection->hasChanged = true;
			CurrSection->active->nform->Compose( adjustment, CurrSection->active->nform, 1.0, 1.0 );
			CurrView->needsDrawing = true;
			}

		if ( LastAdjustment ) delete LastAdjustment;		// clear last adjustment
		LastAdjustment = adjustment;
		delete[] cx;
		delete[] cy;
		delete[] px;
		delete[] py;
	
   		InvalidateRect( appWnd, NULL, FALSE );
		}
	else ErrMsgOK( ERRMSG_NEEDTOALIGN, "" );

	delete CurrPoints;								// free memory
	delete PrevPoints;
}


BOOL CALLBACK CalibrateDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	switch (message)
		{
		case WM_INITDIALOG:
			if ( strlen( InputDlgString ) )
   				SetDlgItemText( hwndDlg, ID_APPLYDOMAIN, InputDlgString );
			else {											// if no domain disable this item
				EnableWindow( GetDlgItem( hwndDlg, ID_APPLYDOMAIN ), FALSE );
				EnableWindow( GetDlgItem( hwndDlg, ID_SETPIXELSIZE ), FALSE );
				}
			SetDlgItemText( hwndDlg, ID_APPLYSECTION, InputDlgValue );
			SetDlgItemText( hwndDlg, ID_DOMAINNAME, InputDlgName );
			SendDlgItemMessage(hwndDlg, ID_APPLYTRACES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case IDOK:
					if ( SendDlgItemMessage(hwndDlg, ID_APPLYTRACES, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CalibrationScope = APPLY_TRACES;
					if ( SendDlgItemMessage(hwndDlg, ID_APPLYDOMAIN, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CalibrationScope = APPLY_DOMAIN;
					if ( SendDlgItemMessage(hwndDlg, ID_APPLYSECTION, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CalibrationScope = APPLY_SECTION;
					EndDialog(hwndDlg, 1);
					return TRUE;
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				}
		}
	return FALSE;
}

double InducedPixelSize( Transform *transform, double *calLengths, Contours *calContours )
{
	Contour *contour, *cc;
	double pixel_length, actual_length;
	int i;
	
	pixel_length = 0.0;
	actual_length = 0.0;
	if ( transform )									// found image, calc pixel_size
	  if ( transform->image )
		{
		contour = calContours->first;
		i = 0;
		while ( contour )
			{
			cc = new Contour( *contour );
			cc->FwdNform( transform->nform );		// put copy into image coordinates
			cc->Scale( 1.0/transform->image->mag );	// then into pixels using mag as displayed on screen
			actual_length += calLengths[i];			// each contour contributes to actual length
			pixel_length += cc->Length();			// and to pixel length
			delete cc;
			i++;
			contour = contour->next;
			}
		}

	if ( (pixel_length > 0.0) && (actual_length > 0.0) ) return( actual_length/pixel_length );

	return( 0.0 );			// pixel size cannot be calculated!
}


void CmCalibrateTraces( void )		// let user specify length of selected traces and adjust sections
{
	Section *section;
	Transform *transform;
	Contour *contour, *cc;
	Point allmin, allmax, min, max;
	Mvmt mvmt;
	Contours *calContours;
	double *calLengths;
	double length, cal, rescale, pixelsize;
	bool changed;
	int calCount, i, x, y, sectnum;
	
	calContours = NULL;
	length = 0.0;
	cal = 0.0;
	rescale = 0.0;
	allmin.x = MAX_FLOAT; allmin.y = MAX_FLOAT;
	allmax.x = -MAX_FLOAT; allmax.y = -MAX_FLOAT;

	if ( FrontView )										// find selected contours if any
	  if ( FrontView->section )
		if ( FrontView->section->active )
			if ( FrontView->section->active->contours )
				{
				contour = FrontView->section->active->contours->first;
				while ( contour )
					{
					if ( contour->points )
					  if ( contour->points->Number() > 1 )					// need at least 2 points for length
						{
						if ( !calContours ) calContours = new Contours();
						cc = new Contour( *contour );						// transform contour into section coord.
						cc->InvNform( FrontView->section->active->nform );	// to correctly calculate length
						length += cc->Length();								// total all length to get rescale
						cc->Extent( &min, &max );
						if (min.x < allmin.x) allmin.x = min.x;				// get extent of contours so can
						if (max.x > allmax.x) allmax.x = max.x;				// calculate midpt for domain query
						if (min.y < allmin.y) allmin.y = min.y;
						if (max.y > allmax.y) allmax.y = max.y;
						calContours->Add( cc );
						}
					contour = contour->next;
					}
				}

	if ( !calContours ) ErrMsgOK( ERRMSG_NEEDTRACE, "" );	// none selected, advise user of problem
	else {
		calCount = calContours->Number();
		calLengths = new double[calCount];
		i = 0;
		contour = calContours->first;						// get user sizes for these contours
		while ( contour )
			{
			sprintf(InputDlgString,"%1.*g",Precision,contour->Length());
			sprintf(InputDlgName,"Enter New Length");
			sprintf(InputDlgValue,"Length of %s in %s:", contour->name, CurrSeries->units);
			if ( DialogBox( appInstance, "InputDlg", appWnd, (DLGPROC)InputDlgProc ) )
				{
				calLengths[i] = atof(InputDlgString);		// remember calibrated length
				cal += calLengths[i];
				i++;
				contour = contour->next;
				}
			else contour = NULL;							// user cancel, abort length input
			}
															// compute rescale value
		if ( (length > 0.0) && (cal > 0.0) ) rescale = cal/length;
															// convert to pixels
		x = (int)floor(((allmin.x+allmax.x)/2.0 - CurrSeries->offset_x)/CurrSeries->pixel_size);
		y = (int)floor(((allmin.y+allmax.y)/2.0 - CurrSeries->offset_y)/CurrSeries->pixel_size);
		transform = FrontView->DomainFromPixel( x, y );
		pixelsize = InducedPixelSize( transform, calLengths, calContours );

		if ( rescale > 0.0 )								// have a valid rescale calculation
			{
			sprintf(InputDlgValue,"Scale entire section by %g",rescale);
			strcpy(InputDlgString,"");
			strcpy(InputDlgName,"");
			if ( transform && (pixelsize > 0.0) )			// if also have domain and pixelsize
				{
				sprintf(InputDlgString,"Set pixel size to %g %s/pixel", pixelsize, CurrSeries->units );
				sprintf(InputDlgName,"for domain: %s", transform->domain->name);
				}
															// invoke dialog for calbration scope

			if ( DialogBox( appInstance, "CalibrateDlg", appWnd, (DLGPROC)CalibrateDlgProc ) )
				{
				FrontView->section->PushUndoState();
				switch ( CalibrationScope )
					{
					case APPLY_TRACES:				// scale each selected contour as user specified
						i = 0;
						contour = FrontView->section->active->contours->first;
						while ( contour )
							{
							if ( contour->points )
							  if ( contour->points->Number() > 1 )
								{
								contour->Scale( calLengths[i]/contour->Length() );
								FrontView->section->hasChanged = true;
								i++;
								}
							contour = contour->next;
							}
						FrontView->needsDrawing = true;
						if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
						InvalidateRect( appWnd, NULL, FALSE );
						break;

					case APPLY_DOMAIN:							// apply to domain mag only
						ImagePixelSize = pixelsize;				// remember pixel size for dialog
						transform->image->mag = pixelsize;
						FrontView->section->hasChanged = true;
						FrontView->needsRendering = true;		// redraw section and update domain list if open
						sectnum = FrontView->section->index;
						if ( IsWindow(domainWindow) ) PostMessage( domainWindow, UM_UPDATESECTION, 0, (LPARAM)sectnum );
						InvalidateRect( appWnd, NULL, FALSE );
						PostMessage( appWnd, WM_COMMAND, CM_CALIBRATESECTIONS, 0 );
						break;

					case APPLY_SECTION:				// scale section, remember last adjustment
						mvmt.scaleX = rescale;
						mvmt.scaleY = rescale;
						if ( LastAdjustment ) delete LastAdjustment;
						LastAdjustment = new Nform();
						LastAdjustment->PostApply( mvmt );
						transform = FrontView->section->transforms->first;
						while ( transform )
							{
							
							transform->nform->PostApply( mvmt );
							FrontView->section->hasChanged = true;
							transform = transform->next;		// do all transforms in section
							}
						FrontView->needsRendering = true;
						if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
						InvalidateRect( appWnd, NULL, FALSE );
						break;
					}
				}
			}

		delete calContours;
		delete[] calLengths;
		}
}

void CmMergeSelected( void )									// merge the selected traces
{
	Contour *contour, *c, *merged;
	Point min, max;
	bool closeit;

	if ( FrontView )
	  if ( FrontView->section)
		if ( FrontView->section->active )						// do only if there are active contours
		  if ( FrontView->section->active->contours )
			{
			FrontView->section->PushUndoState();
			FrontView->section->active->contours->Merge( CurrSeries->pixel_size );
			FrontView->section->hasChanged = true;			// flag change for save
			FrontView->needsDrawing = true;
			if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
			InvalidateRect( appWnd, NULL, FALSE );			// display result
			}
}

void CmReverseSelected( void )									// reverse orientation of selected traces
{
	Contour *contour;

	if ( FrontView )
	  if ( FrontView->section)
		if ( FrontView->section->active )						// do only if there are active contours
		  if ( FrontView->section->active->contours )
			{
			FrontView->section->PushUndoState();
			contour = FrontView->section->active->contours->first;
			while ( contour )
				{
				contour->Reverse();
				FrontView->section->hasChanged = true;			// flag change for save
				FrontView->needsDrawing = true;
				contour = contour->next;						// check all active contours
				}
			if ( FrontView->needsDrawing )
				{
				if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
				InvalidateRect( appWnd, NULL, FALSE );
				}
			}
}

void CmSimplifySelected( void )									// simplify the selected traces
{
	Contour *contour;
	bool simplified = false;

	if ( FrontView )
	  if ( FrontView->section)
		if ( FrontView->section->active )						// do only if there are active contours
		  if ( FrontView->section->active->contours )
			{
			FrontView->section->PushUndoState();
			contour = FrontView->section->active->contours->first;
			while ( contour )
				{
				if ( !contour->simplified )						// reduce number of contour points
					{
					contour->Simplify( SimplifyResolution, contour->closed  );
					simplified = true;
					}
				contour = contour->next;						// check all active contours
				}
			if ( simplified )
				{
				FrontView->section->hasChanged = true;			// flag change for save
				FrontView->needsDrawing = true;
				if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
				InvalidateRect( appWnd, NULL, FALSE );
				}
			else ErrMsgOK( ERRMSG_NOTSIMPLIFIED, "" );			// tell user why nothing happended
			}
}

void CmSmoothSelected( void )								// smooth selected traces using MA filter
{
	Contour *contour;

	if ( FrontView )
	  if ( FrontView->section)
		if ( FrontView->section->active )						// do only if there are active contours
		  if ( FrontView->section->active->contours )
			{
			FrontView->section->PushUndoState();				// remember undo state
			contour = FrontView->section->active->contours->first;
			while ( contour )
				{
				contour->Smooth( CurrSeries->smoothingLength );	// smooth it
				contour->simplified = false;					// and mark it unsimplified
				FrontView->section->hasChanged = true;			// flag change for save
				FrontView->needsDrawing = true;
				contour = contour->next;						// check all active contours
				}
			if ( FrontView->needsDrawing )						// update window -- length/area has changed
				{
				if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
				InvalidateRect( appWnd, NULL, FALSE );
				}
			}
}


void CmPasteAttributes( void )					// give selected trace same attrib. as first clipboard trace
{
	Contour *contour, *clipcontour;
											
	if ( FrontView && ClipboardTransform )		// do only if there are clipboard and active contours
	  if ( FrontView->section )
		if ( FrontView->section->active )
		  if ( FrontView->section->active->contours && ClipboardTransform->contours )
			{
			FrontView->section->PushUndoState();
			clipcontour = ClipboardTransform->contours->first;
			contour = FrontView->section->active->contours->first;
			while ( contour && clipcontour )
				{									// set attributes from clipcontour
				strcpy(contour->name,clipcontour->name);
				contour->border = clipcontour->border;
				contour->fill = clipcontour->fill;
				contour->mode =	clipcontour->mode;
				FrontView->section->hasChanged = true;			// flag change for save
				FrontView->needsDrawing = true;
				contour = contour->next;
				}
			FrontView->section->hasChanged = true;
			FrontView->needsDrawing = true;
			if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
			InvalidateRect(appWnd,NULL,FALSE);			// use paint to redraw contours
			}
}

void ShowSectionTrace( int sectnum, char *tracename )	// goto section and center tracename in main window
{
	Section *section;
	Transform *transform;
	Contour *contour;
	char filename[MAX_PATH];
	bool found;

	found = false;
	if ( FrontView )										// if correct section is already displayed
	  if ( FrontView->section )								// no need to load it again from memory
		if ( FrontView->section->index == sectnum ) found = true;

	if ( !found )											// otherwise, attempt to load and display it
	  if ( FindSection( sectnum, 0, filename ) )
		if ( GotoSection( sectnum, filename ) )	found = true;

	if ( found )									
		{
		found = false;
		section = FrontView->section;				// desired section will now be the FrontView section

		if ( section )								// find first instance of contour on section
			{
			section->UnSelectAll();					// unselect any active contours so can select instance
			if ( section->transforms )
				{										// check every transform...
				transform = section->transforms->first;
				while ( transform && !found )							
					{
					if ( transform->contours )				// ...and every contour therein
						{
						contour = transform->contours->first;
						while ( contour && !found )
							{			
							if ( MatchLimit(contour->name,tracename) )	// found it!
								{
								if ( !section->active )
									{										// if not active transform, create it
									section->active = new Transform();
									section->transforms->Add( section->active );
									}										// create contours list if doesn't exist
								if ( !section->active->contours ) section->active->contours = new Contours();
								section->active->isActive = true;
								transform->contours->Extract( contour );	// extract contour
								contour->InvNform( transform->nform );		// transform out of transform
								contour->FwdNform( section->active->nform );// and in to active transform
								section->active->contours->Add( contour );	// then add to active list
								found = true;								// stop searching
								}
							else contour = contour->next;
							}								// end while contour
						}
					transform = transform->next;
					}								// end while transform
				}
			}
		
		if ( found ) CmZoomSelected();					// finally, display the selected contour on screen
		else ErrMsgOK( ERRMSG_TRACENOTFOUND, tracename  );
		}
}


void CmFindTrace( void )						// find a trace in sections and zoom to it
{
	Section *section;
	Transform *transform;
	Contour *contour;
	int sectnum, last_sectnum;
	bool found;

	sectnum = -1;									// include section 0 in search
	strcpy(InputDlgName,"Find trace...");		    // set up dialog params
	strcpy(InputDlgValue,"Trace Name:");
	if ( FrontView->section )
	  if ( FrontView->section->active )
		 if ( FrontView->section->active->contours )// if selected trace
			{
			contour = FrontView->section->active->contours->first;
			if ( contour )
				{									// then use this name in dialog
				strcpy(FindString,contour->name);
				sectnum = FrontView->section->index;// and start search at next section
				}
			}
	strcpy(InputDlgString,FindString);				// or just use last search string
	
	if ( DialogBox( appInstance, "InputDlg", appWnd, (DLGPROC)InputDlgProc ) )
		{

		if ( strcmp(InputDlgString,FindString) )	// if user changed trace name
			sectnum = -1;							// go back to search all sections
		strcpy(FindString,InputDlgString);			// remember search string
		last_sectnum = MAX_INT32;
		if ( CurrSectionsInfo )						// get index of last section from info list
			if ( CurrSectionsInfo->last )
				last_sectnum = CurrSectionsInfo->last->index;
		found = false;

		while ( !found )							// search until found or section is invalid
			{										// sectnum will be incremented by the call!
			section = GetNextSectionBetween( sectnum, last_sectnum );
			if ( section )
				{
				if ( section->transforms )				// check every section
					{
					transform = section->transforms->first;
					while ( transform && !found )							
						{									// check every transform
						if ( transform->contours )
							{
							contour = transform->contours->first;
							while ( contour && !found )			// check every contour
								{			
								if ( MatchLimit(contour->name,FindString) )
									found = true;				// found name
								else contour = contour->next;
								}							// end while contour
							}
						transform = transform->next;
						}							// end while transform
					}
				PutSection( section, false, false, false );	// free section memory
				}
			else break;		// if no section found then abort
			}				// otherwise try next section

		if ( found ) ShowSectionTrace( sectnum, FindString );	// display trace in main window
		else ErrMsgOK( ERRMSG_TRACENOTFOUND, FindString  );		// or error if trace not found
		}
}
