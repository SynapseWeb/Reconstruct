/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Mouse and cursor operations
//
//    Copyright (C) 1999-2006  John Fiala (fiala@bu.edu)
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
// modified 10/29/04 by JCF (fiala@bu.edu)
// -+- change: Added Wildfire region growing mouse behaviors.
// modified 2/03/05 by JCF (fiala@bu.edu)
// -+- change: Added Scalpel tool mouse behaviors.
// modified 2/22/05 by JCF (fiala@bu.edu)
// -+- change: Added right mouse button trace selection when using scalpel tool.
// modified 3/07/05 by JCF (fiala@bu.edu)
// -+- change: Added InvalidateAllViews() to handle rerendering cases when have a domain selected.
// modified 3/10/05 by JCF (fiala@bu.edu)
// -+- change: Made XOR traces (in Scalpel LButtonUp) retain simplify flag of parent trace.
// -+- change: Added GRID_ELLIPSE element type to Grid Tool.
// modified 4/??/05 by Ju Lu
// modified 4/29/05 by JCF (fiala@bu.edu)
// -+- change: Added CurrSeries parameters to Wildfire() call.
// modified 5/21/05 by JCF (fiala@bu.edu)
// -+- change: Removed pencil-initiate wildfire autotrace
// modified 6/8/05 by JCF (fiala@bu.edu)
// -+- change: Added Wildfire option to trace all regions within rectangle defined by left mouse drag.
// modified 6/17/05 by JCF (fiala@bu.edu)
// -+- change: Added option to WildfireRegion() so can ignore small regions.
// modified 10/3/05 by JCF (fiala@bu.edu)
// -+- change: Added MButtonUp for select with any tool.
// -+- change: Modified scapel right mouse to scroll instead of select trace.
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Modified SetDefaultName params.
// modified 5/11/06 by JCF (fiala@bu.edu)
// -+- change: Moved CreateGridTraces to the Section methods.
// modified 6/22/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified WILDFIRE_TOOL's behavior in LButtonUp to accomodate adaptive threshold
// modified 7/2/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified WILDFIRE_TOOL's behavior in LButtonUp to accomodate multiple autotrace
//             Moved the job done after creation of EditContour in wildfire to a separate function 
//             AddContour in tools.cpp file
// // modified 11/14/06 by JCF (fiala@bu.edu)
// -+- change: Modified name of AddContour to avoid confusion with Ju Lu's routine of same name.
// -+- change: Modified WILDFIRE_TOOL LButtonUp behavior to reflect new Wildfire() function

#include "reconstruct.h"


void BeginDrag( void )							// capture cursor and prepare for mouse move
{
	RECT r;
	
	ClientRectLessStatusBar( &r );				// get client area for clipping cursor
	ClientRectToScreen( appWnd, &r );
	ClipCursor( &r );				
	appDC = GetDC( appWnd );					// set color pen but will use inverse of screen
	if ( CurrSeries )							// by setting R2_NOT so no color in most cases
		{
		dragPen = CreatePen( PS_SOLID, 1, CurrSeries->defaultBorder.ref() );
		SelectObject( appDC, dragPen );				
		}
	else SelectObject( appDC, GetStockObject(BLACK_PEN) );
	SelectObject( appDC, GetStockObject(NULL_BRUSH) );
	SetROP2(appDC, R2_NOT );
}

void ActivateLTool( int x, int y )
{
	LToolRect.left = x;				// remember starting point for subsequent mouse operations
	LToolRect.top = y;				// e.g. drag lines
	LToolRect.right = x;
	LToolRect.bottom = y;
	LToolActive = true;
}

void ActivateRTool( int x, int y )
{
	RToolRect.left = x;				// remember starting point for subsequent mouse operations
	RToolRect.top = y;				// e.g. drag lines
	RToolRect.right = x;
	RToolRect.bottom = y;
	RToolActive = true;
}

void EndDrag( void )							// release device context and cursor from window...
{
	ReleaseDC( appWnd, appDC );
	ClipCursor( NULL );
	if ( dragPen )								// clean up pen created in BeginDrag
		{
		SelectObject( appDC, GetStockObject(BLACK_PEN) );
		DeleteObject(dragPen);
		dragPen = NULL;
		}
}												// ...but use CmEscapeCurrentTool() to cleanup drag's R2_NOT drawing!

void ScrollSection( int x, int y )		// when tool is captured by window client region
{										// touching the border triggers a scroll of the section
	int incx, incy;
	RECT wrect;

	incx = 0; incy = 0;
	ClientRectLessStatusBar( &wrect );	// set scroll size to 2% of visible client area

	if ( x <= wrect.left+5 ) incx = (wrect.right-wrect.left)/50;
	if ( x >= wrect.right-6 ) incx = -(wrect.right-wrect.left)/50;
	if ( y <= wrect.top+5 ) incy = (wrect.bottom-wrect.top)/50;
	if ( y >= wrect.bottom-6 ) incy = -(wrect.bottom-wrect.top)/50;

	if ( incx || incy )						// activate timer to do actual scrolling
		{
		if ( !Scrolling )
			{
			ScrollX = incx;
			ScrollY = incy;
			Scrolling = true;
			SetTimer( appWnd, SCROLL_TIMER, 101, NULL);
			}
		}
	else									// not near edge so disable scroll interrupts
		{
		ScrollX = 0;
		ScrollY = 0;
		KillTimer( appWnd, SCROLL_TIMER );
		Scrolling = false;
		}
}

void YInvertRECT( int h, RECT *r )				// for cases where the user draws a rectangle then a
{												// contour is drawn inside it, make sure we get a clockwise result
	int tmp;									// by flipping the sign of the y coord. first
	tmp = h - r->top;
	r->top = h - r->bottom;
	r->bottom = tmp;
}

void AddContourToSection( int sectnum, Contour *contour )
{													 // use this for point-by-point tracing because   
	Section *section;								 // user can page away from section while tracing
	int s;
	char filename[MAX_PATH];

	if ( CurrSection )								// if this section is in memory...
		if ( CurrSection->index == sectnum ) 
			{
			CurrSection->AddNewContour(contour);
			if ( FrontView != DomainView )
				FrontView->needsDrawing = true;
			return;
			}										// just modify it and flag for redrawing
	if ( PrevSection )						
		if ( PrevSection->index == sectnum )
			{
			PrevSection->AddNewContour(contour);
			if ( BackView != DomainView )
				BackView->needsDrawing = true;
			return;
			}
	s = sectnum;
	if ( FindSection( s, 0, filename ) )			// load section from file if not in memory
		{
		section = new Section( filename );
		section->AddNewContour(contour);			// change it and save it back to disk
		section->Save( filename );
		delete section;								// and delete memory structure
		}
}


void LButtonDown( HWND hWnd, WPARAM, LPARAM lParam)
{
	int x,y;
	double fx,fy,px,py;
	Point *p, *q, min, max;
	Contour *contour;
	Transform *transform;
	HCURSOR cur;

	if ( !CurrSeries || !FrontView ) return;// do nothing if nothing in view

	x = LOWORD(lParam);						// get input pt in client coordinates
	y = HIWORD(lParam);

	switch ( CurrentTool )
		{
		case ARROW_TOOL:					
		case RECTANGLE_TOOL:				// these tools look the same initially
		case MAGNIFY_TOOL:
			if ( !LToolActive )
				{
				BeginDrag();				// capture cursor and prepare for mouse move
				ActivateLTool( x, y );
				}
			break;

		case ZOOM_TOOL:
		case WILDFIRE_TOOL:
			if ( !LToolActive )
				{
				if ( !RToolActive ) BeginDrag();			// dragging may already be active
				ActivateLTool( x, y );						// start LTool
				}
			break;

		case DOMAIN_TOOL:
			if ( FrontView == DomainView ) 				// if have domain, prepare to drag it
				{
				if ( !LToolActive )
					{
					BeginDrag();
					ActivateLTool( x, y );
					}
				}
			else							// select nearest domain to lift from section
				{							// SelectDomain.. does nothing if DomainSection already exists
				transform = FrontView->DomainFromPixel( x, FrontView->height-y );
				SelectDomainFromSection( FrontView->section, transform );
				}
			break;

		case POINT_TOOL:
			if ( GetKeyState(VK_CONTROL) & 0x8000 )			// if control key, pickup nearest contour shape
				{
				fx = CurrSeries->offset_x + ((double)x)*CurrSeries->pixel_size;
				fy = CurrSeries->offset_y + ((double)FrontView->height-(double)y)*CurrSeries->pixel_size;
				CurrContour = FrontView->section->FindClosestContour( fx, fy );
				if ( CurrContour )
					{
					contour = new Contour( *CurrContour );				// copy nearest contour
					contour->Extent( &min, &max );							// find maximum extent in units
					contour->Shift( -(max.x+min.x)/2.0, -(max.y+min.y)/2.0 );// shift contour to origin
					contour->Scale( 1.0/CurrSeries->pixel_size );		// convert to screen pixels
					if ( StampContour ) delete StampContour;
					StampContour = new Contour( *contour );				// now have pixel offset contour
					SetCursor( LoadCursor( 0, IDC_WAIT ) );				// in case StampCursor is in use
					cur = StampCursor;									// remember handle for destroy
					StampCursor = MakeStampCursor( contour );			// create new cursor from contour
					DestroyCursor( cur );								// destroy old cursor
					SetToolCursor( appWnd );							// restore original cursor
					delete contour;
					}
				}
			  else											// no control key => draw replica of point contour
				{
				CurrContour = new Contour( *StampContour );			// copy shape of stamp contour
				CurrSeries->SetDefaultAttributes( CurrContour );	// use current defaults
				FrontView->section->SetDefaultName(CurrContour->name,CurrSeries->defaultName,FrontView==DomainView);
				fx = (double)x; fy = (double)(FrontView->height-y);
				CurrContour->Shift(fx,fy);								// shift (in pixels) to cursor
				CurrContour->Scale( CurrSeries->pixel_size );			// put into section untis
				CurrContour->Shift( CurrSeries->offset_x, CurrSeries->offset_y );
				FrontView->section->AddNewContour( CurrContour );
				if ( FrontView == DomainView ) FrontView->needsRendering = true;
				else FrontView->needsDrawing = true;
				InvalidateRect( hWnd, NULL, FALSE );
				}
			break;


		case PENCIL_TOOL:
			if ( !RToolActive && !LToolActive )
				{
				BeginDrag();							// capture cursor and prepare for mouse move
				SetROP2(appDC, R2_COPYPEN );			// turn on pencil color
				ActivateLTool( x, y );					// signal that drawing in progress
				ToolContour = new Contour();			// create escapeable buffer for pen drawing
				ToolContour->points = new Points();		// (i.e. don't add to section yet)
				MoveToEx( appDC, x, y, NULL );			// prepare to draw from first point
				p = new Point((double)x,(double)y,0.0);	// add first point to buffer
				ToolContour->points->Add(p);
				}
			break;

		case LINE_TOOL:
			if ( !LToolActive )						// start line drawing
				{
				BeginDrag();						// capture cursor and prepare for mouse move
				ActivateLTool( x, y );
				MoveToEx( appDC, LToolRect.left, LToolRect.top, NULL );
				LineTo( appDC, LToolRect.right, LToolRect.bottom );
				}
			else									// with line, done after second LButtonDown
				{
				CmEscapeCurrentTool();
				if ( (FrontView != DomainView)		// prevent contour creation on DomainView
				  && ((LToolRect.left != LToolRect.right) || (LToolRect.top != LToolRect.bottom)) )
					{
					CurrContour = new Contour();				// create a new contour
					CurrSeries->SetDefaultAttributes( CurrContour );	// use current defaults
					FrontView->section->SetDefaultName(CurrContour->name,CurrSeries->defaultName,false);
					CurrContour->closed = false;
					CurrContour->points = new Points();	
					q = new Point( (double)LToolRect.left, (double)LToolRect.top, 0.0 );
					CurrContour->points->Add(q);
					q = new Point( (double)LToolRect.right, (double)LToolRect.bottom, 0.0 );
					CurrContour->points->Add(q);
					CurrContour->YInvert( (double)FrontView->height );
					CurrContour->Scale( CurrSeries->pixel_size );
					CurrContour->Shift( CurrSeries->offset_x, CurrSeries->offset_y );
					CurrContour->simplified = true;
					FrontView->section->AddNewContour( CurrContour );
					FrontView->needsDrawing = true;
					InvalidateRect( hWnd, NULL, FALSE );
					}
				}
			break;

		case ZLINE_TOOL:
		case MULTILINE_TOOL:						
		case CULTILINE_TOOL:						// add point to contour being edited (or start new one)
			if ( !LToolActive )						// first point entered, start edit contour
				{
				BeginDrag();						// capture cursor and prepare for mouse move
				ActivateLTool( x, y );
				EditContour = new Contour();		// create a new contour and points list
				CurrSeries->SetDefaultAttributes( EditContour );	// use current defaults
				FrontView->section->SetDefaultName(EditContour->name,CurrSeries->defaultName,FrontView==DomainView);
				EditContour->closed = (CurrentTool == CULTILINE_TOOL);
				EditContour->points = new Points();					// multiline is open, cultiline closed
				EditContourSection = FrontView->section->index;		// flag start on domain by negative
				if ( FrontView == DomainView ) EditContourSection = -EditContourSection;
				}
			else {
				MoveToEx( appDC, LToolRect.left, LToolRect.top, NULL );	// erase previous drag line
				LineTo( appDC, LToolRect.right, LToolRect.bottom );		// don't end drag (wait for right button)
				ActivateLTool( x, y );									// so restart LToolRect drag rectangle
				}
																		// convert point at cursor to section units
			px = CurrSeries->pixel_size*((double)x) + CurrSeries->offset_x;
			py = CurrSeries->pixel_size*((double)(FrontView->height - y)) + CurrSeries->offset_y;
			q = new Point( px, py, (double)(FrontView->section->index) );
			EditContour->points->AddFirst(q);							// add segment to EditContour
			FrontView->needsDrawing = true;
			if ( BackView )
				BackView->needsDrawing = true;			// update back trace of EditContour
			InvalidateRect( hWnd, NULL, FALSE );		// redraw contours on section
			break;

		case ELLIPSE_TOOL:
			if ( !LToolActive )				// allow for case of button up on child window!
				{
				BeginDrag();				// capture cursor and prepare for mouse move
				ActivateLTool( x, y );
				}
			break;

		case SCISSOR_TOOL:								// edit nearest contour(s) in section
			px = CurrSeries->offset_x + ((double)x)*CurrSeries->pixel_size;;
			py = CurrSeries->offset_y + ((double)(FrontView->height-y))*CurrSeries->pixel_size;
			if ( FrontView == DomainView )
				{
				EditContour = new Contour( *DomainSection->active->domain );// ASSUMES DomainSection has active domain
				EditContour->Scale( DomainSection->active->image->mag );	// convert from pixels and
				EditContour->InvNform( DomainSection->active->nform );		// transform out of domain tform
				}
			else {
				FrontView->section->PushUndoState();	// remember uncut contour state
				EditContour = FrontView->section->ExtractClosestContour( px, py );
				}
			if ( EditContour )							// found trace to edit
				{
				BeginDrag();							// capture cursor and prepare for mouse move
				ActivateLTool( x, y );
				EditContourSection = FrontView->section->index;
				if ( FrontView == DomainView ) EditContourSection = -EditContourSection;
				EditContour->CutAtNearest( px, py );
				EditContour->simplified = false;
				if ( EditContour->closed )	SwitchTool( CULTILINE_TOOL );
				else						SwitchTool( MULTILINE_TOOL );
				CmBackspace();							// with new tool active, backup one point
				}
			break;

		case GRID_TOOL:								// place repeating pattern of traces on section
			if ( FrontView != DomainView )			// don't allow grid for domain boundary
				{
				px = CurrSeries->offset_x + ((double)x)*CurrSeries->pixel_size;;
				py = CurrSeries->offset_y + ((double)(FrontView->height-y))*CurrSeries->pixel_size;
				FrontView->section->CreateGridTraces( px, py );
				FrontView->needsDrawing = true;		// add grid traces to section and redraw
				InvalidateRect( hWnd, NULL, FALSE );
				}
			break;

		case SCALPEL_TOOL:								// create a break line for traces
			if ( !RToolActive && !LToolActive )			// also find nearest trace at start of cut
				{
				BeginDrag();							// capture cursor and prepare for mouse move
				SetROP2(appDC, R2_NOT );				// turn on reversible line
				ActivateLTool( x, y );					// signal that drawing in progress
				ToolContour = new Contour();			// create escapeable buffer for pen drawing
				ToolContour->points = new Points();		// (i.e. don't add to section yet)
				MoveToEx( appDC, x, y, NULL );			// prepare to draw from first point
				p = new Point((double)x,(double)y,0.0);	// add first point to buffer
				ToolContour->points->Add(p);
				}
			break;
		}
}

void RButtonDown( HWND, WPARAM, LPARAM lParam)
{
	int x,y,sectnum;
	double fx,fy,area;
	Point *p, min, max;
	Contour *contour;
	Section *section;
	HCURSOR cur;

	if ( !CurrSeries || !FrontView ) return;	// do nothing if nothing in view

	x = LOWORD(lParam);				// get input pt in client coordinates
	y = HIWORD(lParam);

	switch ( CurrentTool )
		{
		case ZOOM_TOOL:
			if ( !RToolActive )
				{
				if ( !LToolActive ) BeginDrag();			// capture cursor in client area
				SetCursor( LoadCursor( appInstance, MAKEINTRESOURCE(ZOOMCUR) ) );
				ActivateRTool( x, y );
				}
			break;

		case DOMAIN_TOOL:				// if there's a domain section, right mouse click merges it back
			CmEscapeCurrentTool();		// but first abort any active LButton dragging
			if ( DomainSection ) CmMergeFront();
			break;

		case GRID_TOOL:
		case POINT_TOOL:
			if ( !RToolActive )
				{
				if ( !LToolActive ) BeginDrag();			// capture cursor in client area
				ActivateRTool( x, y );
				}
			break;

		case PENCIL_TOOL:
			if ( !RToolActive )
				{
				if ( !LToolActive ) BeginDrag();			// capture cursor in client area
				ActivateRTool( x, y );
				}
			break;

		case MAGNIFY_TOOL:				// go back to lastZoomRect
			CmEscapeCurrentTool();
			CmLastZoom();
			break;

		case LINE_TOOL:
			CmEscapeCurrentTool();		// treat line like end of multiline for consistency
			break;

		case MULTILINE_TOOL:			// user is ending the EditContour, add it to section
		case CULTILINE_TOOL:
			if ( LToolActive && EditContour )	
				{
				cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass becuz exterior can be slow
				EditContour->Extent(&min,&max);
				if ( (max.x>min.x) || (max.y>min.y) )			// EditContour must not be same point
					{
					if ( AutoSimplify && (CurrentTool==CULTILINE_TOOL) )// take outer boundary to remove loops
						EditContour->Simplify( CurrSeries->pixel_size, EditContour->closed );
					if ( EditContourSection < 0 )
						{											// if this started in a Domain								
						DomainSection->AddNewContour(EditContour);	// then add it there and flag
						DomainView->needsRendering = true;			// Domain for reredenring of image
						}
					else											// if not domain boundary, add to orig section
						AddContourToSection(EditContourSection,EditContour);  // which may not be in memory by now
					EditContour = NULL;							// prevent deletion of EditContour in CmEscape...
					}
				SetCursor( cur );								// restore cursor
				CmEscapeCurrentTool();							// end dragging, clean drawings, and repaint
				}
			break;

		case ZLINE_TOOL:			// user is ending the EditContour, add it to Z-Contours of series
			if ( LToolActive && EditContour )	
				{
				EditContour->Extent(&min,&max);
				if ( (max.x>min.x) || (max.y>min.y) || (max.z>min.z))	// EditContour must not be same point
					{
					CurrSeries->AddZContour(EditContour);		// add contour to series z-list
					EditContour = NULL;							// prevent deletion of EditContour in CmEscape...
					}
				CmEscapeCurrentTool();							// end dragging, clean drawings, and repaint
				}
			break;

		case WILDFIRE_TOOL:				// flip cursor while RButtonDown
			if ( !RToolActive )
				{
				if ( !LToolActive ) BeginDrag();		// capture cursor in client area
				SetCursor( LoadCursor( appInstance, MAKEINTRESOURCE(OUTFIRECUR) ) );
				ActivateRTool( x, y );
				SetROP2(appDC, R2_BLACK );				// set pencil color to black
				ToolContour = new Contour();			// create escapeable buffer for pen drawing
				ToolContour->points = new Points();		// (i.e. don't add to section yet)
				MoveToEx( appDC, x, y, NULL );			// prepare to draw from first point
				p = new Point((double)x,(double)y,0.0);	// add first point to buffer
				ToolContour->points->Add(p);
				}
			break;

		case SCALPEL_TOOL:				// allow selection of trace on button up
			if ( !LToolActive )
				{
				if ( !RToolActive ) BeginDrag();			// capture cursor in client area
				ActivateRTool( x, y );
				}
			break;
		}
}

void MouseMove(HWND, WPARAM, LPARAM lParam)
{
	int x, y, nx, ny;
	double ns;
	POINT cp;
	Point *p;

	if ( !CurrSeries || !FrontView ) return;			// do nothing if nothing in view

	x = LOWORD(lParam);									// get mouse pt in client coordinates
	y = HIWORD(lParam);

	switch ( CurrentTool )
		{
		case ARROW_TOOL:								// these 3 tools all use a drag rectangle
		case RECTANGLE_TOOL:
		case MAGNIFY_TOOL:
			if ( LToolActive )							// using R2_NOT, erase then draw in new location
				{
				ScrollSection( x, y );			// allow view to be scrolled near edge
				if ( !ScrollOccurred )			// if scroll occurred line was cleared
					Rectangle( appDC, LToolRect.left, LToolRect.top, LToolRect.right, LToolRect.bottom );
				Rectangle( appDC, LToolRect.left, LToolRect.top, x, y );
				}
			break;

		case ZOOM_TOOL:
			if ( RToolActive || LToolActive )
				{
				if ( RToolActive )						// zoom view
					{
					ns = 1.0+(double)(y-RToolRect.top)/100.0;
					if ( ns < 0.1 ) ns = 0.1;
					if ( BlendView ) BlendView->Zoom( appDC, ns, RToolRect.left, RToolRect.top );
					else FrontView->Zoom( appDC, ns, RToolRect.left, RToolRect.top );
					}
				else {									// LToolActive, so pan view
					nx = x - LToolRect.left;
					ny = y - LToolRect.top;
					if ( BlendView ) BlendView->Pan( appDC, nx, ny );
					else FrontView->Pan( appDC, nx, ny );
					}
				}
			break;

		case DOMAIN_TOOL:
			if ( LToolActive ) {						// drag domain around the section
				nx = x - LToolRect.left;				// by skipping on screen shift when domain is in back
				ny = y - LToolRect.top;					// can see where cursor moves to on rest of section
				if ( DomainView == FrontView ) DomainView->Pan( appDC, nx, ny );
				}
			break;

		case PENCIL_TOOL:
			if ( LToolActive && ToolContour )			// while pencil is down add points to buffer
				{
				LineTo( appDC, x, y );
				p = new Point((double)x,(double)y,0.0);
				ToolContour->points->Add(p);
				}
			else if ( RToolActive )							// when drag with right mouse button
				ScrollSection( x, y );					// allow view to be scrolled near edge
			break;

		case GRID_TOOL:
		case POINT_TOOL:
			if ( RToolActive )						// when drag with right mouse button
				ScrollSection( x, y );					// allow view to be scrolled near edge
			break;

		case ELLIPSE_TOOL:
			if ( LToolActive )							// using R2_NOT, erase then draw in new location
				{
				ScrollSection( x, y );			// allow view to be scrolled near edge
				if ( !ScrollOccurred )			// if scroll occurred line was cleared
					Ellipse( appDC, LToolRect.left, LToolRect.top, LToolRect.right, LToolRect.bottom );
				Ellipse( appDC, LToolRect.left, LToolRect.top, x, y );
				}
			break;

		case LINE_TOOL:									// these tools use same drag line technique
		case ZLINE_TOOL:
		case MULTILINE_TOOL:
		case CULTILINE_TOOL:
			if ( LToolActive )
				{
				ScrollSection( x, y );			// allow view to be scrolled near edge
				if ( !ScrollOccurred )			// if scroll occurred line was cleared
					{
					MoveToEx( appDC, LToolRect.left, LToolRect.top, NULL );	// erase
					LineTo( appDC, LToolRect.right, LToolRect.bottom );
					}
				MoveToEx( appDC, LToolRect.left, LToolRect.top, NULL );	// draw
				LineTo( appDC, x, y );
				}
			break;

		case WILDFIRE_TOOL:
			if ( RToolActive || LToolActive )
				if ( RToolActive )					// drag fire break line
					{
					LineTo( appDC, x, y );
					p = new Point((double)x,(double)y,0.0);
					ToolContour->points->Add(p);
					}
				else								// update drag rectangle
					{
					Rectangle( appDC, LToolRect.left, LToolRect.top, LToolRect.right, LToolRect.bottom );
					Rectangle( appDC, LToolRect.left, LToolRect.top, x, y );
					}
			break;

		case SCALPEL_TOOL:
			if ( LToolActive && ToolContour )			// while pencil is down add points to buffer
				{
				LineTo( appDC, x, y );
				p = new Point((double)x,(double)y,0.0);
				ToolContour->points->Add(p);
				}
			else if ( RToolActive )						// when drag with right mouse button
				ScrollSection( x, y );					// allow view to be scrolled near edge
			break;
		}

	LToolRect.right = x; LToolRect.bottom = y;			// update tool regions
	RToolRect.right = x; RToolRect.bottom = y;
	ScrollOccurred = false;								// clear scroll event flag for next draw
}

void LButtonUp( HWND hWnd, WPARAM, LPARAM lParam)
{
	int x, y, nx, ny;
	double fx, fy, r, l, t, b;
	ADib *region;
	Transform *transform;
	Contour *contour;
	Contours *contours;
	RGBQUAD origin_color;
	Mvmt mvmt;
	Point *p, *q, mn, mx;
	Points *pts;
	RECT bx, by;
	HCURSOR cur;

	if ( !CurrSeries || !FrontView ) return;			// do nothing if nothing in view

	x = LOWORD(lParam);									// get mouse pt in client coordinates
	y = HIWORD(lParam);

	switch ( CurrentTool )
		{
		case ARROW_TOOL:								// select nearest contour(s) in section
			if ( LToolActive )
				{
				CmEscapeCurrentTool();
				if ( (abs(x - LToolRect.left)<=3) || (abs(y - LToolRect.top)<=3) )	// no rectangle, select closest
					{
					fx = CurrSeries->offset_x + ((double)x)*CurrSeries->pixel_size;;
					fy = CurrSeries->offset_y + ((double)FrontView->height-(double)y)*CurrSeries->pixel_size;
					if ( FrontView->section->SelectClosestContour( fx, fy ) )
						{
						FrontView->needsDrawing = true;
						InvalidateRect( hWnd, NULL, FALSE );
						}
					}
				else {															// select all in rectangle
					nx = LToolRect.left; if ( nx > x ) { nx = x; x = LToolRect.left; }
					ny = LToolRect.top; if ( ny > y ) { ny = y; y = LToolRect.top; }										
					l = CurrSeries->offset_x + ((double)nx)*CurrSeries->pixel_size;
					r = CurrSeries->offset_x + ((double)x)*CurrSeries->pixel_size;
					t = CurrSeries->offset_y + ((double)FrontView->height-(double)ny)*CurrSeries->pixel_size;
					b = CurrSeries->offset_y + ((double)FrontView->height-(double)y)*CurrSeries->pixel_size;
					if ( FrontView->section->SelectContoursInRegion( l, t, r, b ) )
						{
						FrontView->needsDrawing = true;
						InvalidateRect( hWnd, NULL, FALSE );
						}
					}
				}
			break;

		case ZOOM_TOOL:
			if ( LToolActive )							// set section to final position
				{
				if ( !RToolActive ) EndDrag();			// end drag only if not dragging with right button
				LToolActive = false;
				nx = x - LToolRect.left;
				ny = y - LToolRect.top;
				if ( FrontView && CurrSeries )
					{
					CurrSeries->offset_x -= CurrSeries->pixel_size*(double)nx;
					CurrSeries->offset_y += CurrSeries->pixel_size*(double)ny;
					InvalidateAllViews();
					InvalidateRect( hWnd, NULL, FALSE );
					}		  
				}
			break;

		case DOMAIN_TOOL:
			if ( LToolActive )							// adjust domain tform after drag domain with mouse!
				{
				EndDrag();								// Note: there are 2 different behaviors depending on
				LToolActive = false;					// whether domain is in front or back, 
				nx = x - LToolRect.left;				// but this part works for both
				ny = y - LToolRect.top;
				if ( DomainSection )
					{
					DomainSection->PushUndoState();					// remember undo state
					mvmt.transX = CurrSeries->pixel_size*(double)nx;
					mvmt.transY = -CurrSeries->pixel_size*(double)ny;
					DomainSection->active->nform->PostApply( mvmt );// apply mvmt to domain
					if ( LastAdjustment ) delete LastAdjustment;
					LastAdjustment = new Nform();
					LastAdjustment->PostApply( mvmt );				// remember as last adjustment
					if ( Recording ) Recording->PostApply( mvmt );	// and add to recording
					DomainView->needsRendering = true;
					InvalidateRect( hWnd, NULL, FALSE );			// repaint window
					} 
				}
			break;

		case PENCIL_TOOL:
			if ( LToolActive && ToolContour )				// complete contour drawn with pencil drag
				{
				ToolContour->Extent(&mn,&mx);				// ignore pencil marks without area
				if ( (mx.x>mn.x) || (mx.y>mn.y) )
					{					
					cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass becuz exterior can be slow
					CurrContour = new Contour(*ToolContour);			// create new contour from drawing
					CurrSeries->SetDefaultAttributes( CurrContour );	// use current defaults
					FrontView->section->SetDefaultName(CurrContour->name,CurrSeries->defaultName,FrontView==DomainView);
					CurrContour->closed = true;							// complete contour
					CurrContour->YInvert( (double)FrontView->height );	// flip y coord
					CurrContour->Scale( CurrSeries->pixel_size );		// put into units and view position
					CurrContour->Shift( CurrSeries->offset_x, CurrSeries->offset_y );
					if ( AutoSimplify )									// take outer boundary of points
						CurrContour->Simplify( CurrSeries->pixel_size, true );
					FrontView->section->AddNewContour( CurrContour );	// add trace or domain
					if ( FrontView == DomainView ) FrontView->needsRendering = true;
					else FrontView->needsDrawing = true;
					SetCursor( cur );
					}
				CmEscapeCurrentTool();								// release DC and cursor, delete ToolContour,
				}													// and Invalidate Window (in Escape for Pencil only!)
			break;

		case ELLIPSE_TOOL:
			if ( LToolActive )
				{
				CmEscapeCurrentTool();								// release DC and cursor		
				if ( (LToolRect.left != LToolRect.right) && (LToolRect.top != LToolRect.bottom) )
					{														// if non-zero area...
					CurrContour = new Contour();							// create a new contour
					CurrSeries->SetDefaultAttributes( CurrContour );		// use current defaults
					FrontView->section->SetDefaultName(CurrContour->name,CurrSeries->defaultName,FrontView==DomainView);
					CurrContour->points = new Points();						// compute parameters of ellipse
					YInvertRECT( FrontView->height, &LToolRect );
					double a = ((double)abs(LToolRect.right-LToolRect.left))/2.0;			
					double b = ((double)abs(LToolRect.bottom-LToolRect.top))/2.0;
					double h = ((double)min(LToolRect.left,LToolRect.right))+a;
					double k = ((double)min(LToolRect.top,LToolRect.bottom))+b;
					double step = 2.0*PI/min(a,b);
					if ( step > PI/10.0 ) step = PI/10.0;
					for (double p=-PI; p<PI-step; p=p+step)
						{													// create ellipse
						fx = a*cos(p) + h;
						fy = b*sin(p) + k;
						q = new Point( fx, fy, 0.0 );
						CurrContour->points->Add(q);
						}
					CurrContour->Scale( CurrSeries->pixel_size );			// convert pixels into section units
					CurrContour->Shift( CurrSeries->offset_x, CurrSeries->offset_y );
					CurrContour->simplified = true;
					FrontView->section->AddNewContour( CurrContour );		// add trace or domain and redraw
					if ( FrontView == DomainView ) FrontView->needsRendering = true;
					else FrontView->needsDrawing = true;					// rerender if domain, else draw only
					InvalidateRect( hWnd, NULL, FALSE );
					}
				}
			break;

		case RECTANGLE_TOOL:
			if ( LToolActive )
				{
				CmEscapeCurrentTool();								// release DC and cursor
				if ( (LToolRect.left != LToolRect.right) && (LToolRect.top != LToolRect.bottom) )
					{														// if non-zero area...
					CurrContour = new Contour();							// create a new contour
					CurrSeries->SetDefaultAttributes( CurrContour );		// use current defaults
					FrontView->section->SetDefaultName(CurrContour->name,CurrSeries->defaultName,FrontView==DomainView);
					CurrContour->points = new Points();						// add points of rectangle
					YInvertRECT( FrontView->height, &LToolRect );			// 
					if ( LToolRect.right < LToolRect.left )
						 { l = (double)LToolRect.right; r = (double)LToolRect.left; }
					else { r = (double)LToolRect.right; l = (double)LToolRect.left; }
					if ( LToolRect.bottom < LToolRect.top )
						 { t = (double)LToolRect.bottom; b = (double)LToolRect.top; }
					else { b = (double)LToolRect.bottom; t = (double)LToolRect.top; }
					q = new Point( l, t, 0.0 );
					CurrContour->points->Add(q);		
					q = new Point( r, t, 0.0 );
					CurrContour->points->Add(q);
					q = new Point( r, b, 0.0 );
					CurrContour->points->Add(q);
					q = new Point( l, b, 0.0 );
					CurrContour->points->Add(q);
					//CurrContour->Shift( 0.5, 0.5 );			// should this be added to all tools?? JCF 6/17/05
					CurrContour->Scale( CurrSeries->pixel_size );			// convert pixels into section units
					CurrContour->Shift( CurrSeries->offset_x, CurrSeries->offset_y );
					CurrContour->simplified = true;
					FrontView->section->AddNewContour( CurrContour );		// redraw domain with new boundary
					if ( FrontView == DomainView ) FrontView->needsRendering = true;
					else FrontView->needsDrawing = true;					// or just redraw traces
					InvalidateRect( hWnd, NULL, FALSE );
					}
				}
			break;

		case MAGNIFY_TOOL:
			if ( LToolActive )
				{
				CmEscapeCurrentTool();
				if ( (LToolRect.left != LToolRect.right) && (LToolRect.top != LToolRect.bottom) )	// if non-zero area...
					{
					LastZoom.pixel_size = CurrSeries->pixel_size;
					LastZoom.offset_x = CurrSeries->offset_x;
					LastZoom.offset_y = CurrSeries->offset_y;						// get lower-left corner of rect
					x = LToolRect.left; if ( x > LToolRect.right ) x = LToolRect.right;
					y = LToolRect.bottom; if ( y < LToolRect.top ) y = LToolRect.top;
					y = FrontView->height - y;										// invert y-coordinate
					CurrSeries->offset_x += ((double)x)*CurrSeries->pixel_size;		// compute new offsets
					CurrSeries->offset_y += ((double)y)*CurrSeries->pixel_size;
					nx = abs(LToolRect.right - LToolRect.left);
					ny = abs(LToolRect.bottom - LToolRect.top);			// from size of rect get largest new mag
					fx = ((double)nx)*CurrSeries->pixel_size/((double)FrontView->width);
					fy = ((double)ny)*CurrSeries->pixel_size/((double)FrontView->height);
					if ( fx > fy ) CurrSeries->pixel_size = fx;
					else CurrSeries->pixel_size = fy;					// use this as new pixel size
					InvalidateAllViews();								// update views
					InvalidateRect( hWnd, NULL, FALSE );
					}
				}
			break;

		case WILDFIRE_TOOL:
			if ( LToolActive )
				{											
				cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// Wildfire entire rectangular region...
				
				if ( (abs(LToolRect.left-LToolRect.right)>6) && (abs(LToolRect.top-LToolRect.bottom)>6) )
					{
					YInvertRECT( FrontView->height, &LToolRect );	// put rectangle in valid view coord.
					if ( LToolRect.right < LToolRect.left )
						 { nx = LToolRect.right; LToolRect.right = LToolRect.left; LToolRect.left = nx; }
					if ( LToolRect.top < LToolRect.bottom )
						 { ny = LToolRect.bottom; LToolRect.bottom = LToolRect.top; LToolRect.top = ny; }

					region = FrontView->CopyView( CurrSeries->tracesStopWhen );	// get bitmap for wildfires

					for (y=LToolRect.bottom; y<LToolRect.top; y++)	// search bitmap for wildifeable areas	
						for (x=LToolRect.left; x<LToolRect.right; x++)
							{													// test if pixel ignites
							contour = region->Wildfire( x, y, CurrSeries->areaStopSize ); 
							if ( contour )
								{									// reduce contour pts and store in section
								contour->Reduce( 1.0 );
								FrontView->section->AddViewPortContour(contour);
								if ( FrontView == DomainView ) FrontView->needsRendering = true;
								else FrontView->needsDrawing = true;
								}									// redrawing will occur in CmEscapeCurrentTool	
							}
					delete region;									// clean up bitmap memory			
					}
				else											//...else ignite single Wildfire at cursor point
					{	
					region = FrontView->CopyView( CurrSeries->tracesStopWhen );
					contour = region->Wildfire( x, FrontView->height-y, CurrSeries->areaStopSize );
					if ( contour )
						{											// reduce contour pts and store in section
						contour->Reduce( 1.0 );
						FrontView->section->AddViewPortContour(contour);
						if ( FrontView == DomainView ) FrontView->needsRendering = true;
						else FrontView->needsDrawing = true;		// redrawing will occur in CmEscapeCurrentTool
						}						
					delete region;									// clean up bitmap memory					
					}

				SetCursor( cur );
				CmEscapeCurrentTool();						// release DC and cursor, update view if needed
				}
			break;

		case SCALPEL_TOOL:
			if ( LToolActive && ToolContour )				// after ToolContour drag line, locate EditContour
			  {												// and split it into pieces along drag outline
			  if ( FrontView != DomainView )				// not applicable to DomainView since only one domain
				{
				FrontView->section->PushUndoState();
				x = ToolContour->points->first->x;			// get contour nearest LButtonDown
				y = ToolContour->points->first->y;
				fx = CurrSeries->offset_x + ((double)x)*CurrSeries->pixel_size;;
				fy = CurrSeries->offset_y + ((double)(FrontView->height-y))*CurrSeries->pixel_size;
				contour = FrontView->section->ExtractClosestContour(fx,fy);
				if ( contour )
					{
					cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass becuz this can be slow
					EditContour = new Contour( *contour );
					transform = new Transform();					// create identity transform for results
					transform->contours = new Contours();			// will be creating multiple pieces
					ToolContour->YInvert( (double)FrontView->height );	// flip y coord
					ToolContour->closed = false;
					ToolContour->ChainCode( 1.0 );						// complete chain code on pixels
					ToolContour->Exterior();							// go around outside of it
					EditContour->Shift( -CurrSeries->offset_x, -CurrSeries->offset_y );
					EditContour->ChainCode( CurrSeries->pixel_size );	// convert section units to pixels
					pts = EditContour->XOR( ToolContour );
					if ( pts )									// if have intersections, generate pieces
						{
						delete contour;							// forget about original contour
						while ( pts )
							{									// just keep adding pieces to transform...
							contour = new Contour();
							strcpy(contour->name,EditContour->name);
							strcpy(contour->comment,EditContour->comment);
							contour->closed = EditContour->closed;
							contour->simplified = EditContour->simplified;	// ASSUMES: XOR never adds loops!
							contour->border = EditContour->border;
							contour->fill = EditContour->fill;
							contour->mode = EditContour->mode;
							contour->points = pts;
							contour->Reduce( 1.0 );					// make some reduction in number of pts
							contour->Scale( CurrSeries->pixel_size );
							contour->Shift( CurrSeries->offset_x, CurrSeries->offset_y );
							transform->contours->Add( contour );
							FrontView->section->hasChanged = true;
							pts = EditContour->XOR( ToolContour );	// look for next piece
							}
						}
					else transform->contours->Add( contour );	// put orig contour back into transform
					FrontView->section->transforms->Add( transform );	// add transform to section
					delete EditContour;									// delete the copy of original
					EditContour = NULL;
					FrontView->needsDrawing = true;
					SetCursor( cur );
					}
				}
			  CmEscapeCurrentTool();						// release DC and cursor, delete ToolContour,
			  }												// and Invalidate Window in Escape
			break;
		}
}

void MButtonUp( HWND hWnd, WPARAM, LPARAM lParam)
{
	int x, y;
	double fx, fy;

	if ( !CurrSeries || !FrontView ) return;			// do nothing if nothing in view

	x = LOWORD(lParam);									// get mouse pt in client coordinates
	y = HIWORD(lParam);

	fx = CurrSeries->offset_x + ((double)x)*CurrSeries->pixel_size;;
	fy = CurrSeries->offset_y + ((double)FrontView->height-(double)y)*CurrSeries->pixel_size;
	if ( FrontView->section->SelectClosestContour( fx, fy ) )
		{
		FrontView->needsDrawing = true;
		InvalidateRect( hWnd, NULL, FALSE );
		}
}

void RButtonUp( HWND hWnd, WPARAM, LPARAM lParam )
{
	double ns, fx, fy;
	int x, y, nx, ny;
	RECT r;
	Point *q;

	if ( !CurrSeries || !FrontView ) return;	// do nothing if nothing in view

	x = LOWORD(lParam);							// get mouse pt in client coordinates
	y = HIWORD(lParam);

	switch ( CurrentTool )
		{
		case ARROW_TOOL:						// unselect nearest selected contour
			fx = CurrSeries->offset_x + ((double)x)*CurrSeries->pixel_size;
			fy = CurrSeries->offset_y + ((double)FrontView->height-(double)y)*CurrSeries->pixel_size;
			FrontView->section->UnSelectClosestActive( fx, fy );
			FrontView->needsDrawing = true;
			InvalidateRect( hWnd, NULL, FALSE );
			break;

		case ZOOM_TOOL:							// complete zoom drag, update view parameters
			if ( RToolActive )
				{
				SetCursor( LoadCursor( appInstance, MAKEINTRESOURCE(XYCUR) ) );
				if ( !LToolActive ) EndDrag();
				RToolActive = false;
				ns = 1.0+(double)(y-RToolRect.top)/100.0;			// compute final scale factor
				if ( ns < 0.1 ) ns = 0.1;
				if ( FrontView )
					{
					LastZoom.pixel_size = CurrSeries->pixel_size;	// remember where we were
					LastZoom.offset_x = CurrSeries->offset_x;		// and update view parameters
					LastZoom.offset_y = CurrSeries->offset_y;
					CurrSeries->offset_x += RToolRect.left*CurrSeries->pixel_size*(ns-1.0)/ns;
					CurrSeries->offset_y += (FrontView->height-RToolRect.top)*CurrSeries->pixel_size*(ns-1.0)/ns;
					CurrSeries->pixel_size = CurrSeries->pixel_size/ns;
					InvalidateAllViews();
					InvalidateRect( hWnd, NULL, FALSE );
					}		  
				}
			break;

		case GRID_TOOL:
		case POINT_TOOL:						// stop scrolling
		case PENCIL_TOOL:
		case SCALPEL_TOOL:
			if ( RToolActive ) CmEscapeCurrentTool();
			break;

		case WILDFIRE_TOOL:
			if ( RToolActive && ToolContour )					// release outfire marking tool
				{
				SetCursor( LoadCursor( appInstance, MAKEINTRESOURCE(WILDFIRECUR) ) );
				ToolContour->closed = false;						// don't complete contour
				ToolContour->YInvert( (double)FrontView->height );	// flip y coord
				ToolContour->ChainCode( 1.0 );						// fill in gaps
				q = ToolContour->points->first;
				while ( q )
					{
					r.left = (int)q->x - 2;
					r.right = (int)q->x + 2;
					r.top = (int)q->y - 2;
					r.bottom = (int)q->y + 2;
					FrontView->view->Clear( r );				// clear bitmap around pixel (including index!)
					q = q->next;
					}											// redrawing will place modified bitmap on display
				FrontView->needsDrawing = true;
				CmEscapeCurrentTool();							// release DC and cursor, delete ToolContour,
				}												// and Invalidate Window (in Escape)
			break;
/*
		case SCALPEL_TOOL:								// select nearest contour(s) in section
			if ( RToolActive )
				{
				CmEscapeCurrentTool();
				fx = CurrSeries->offset_x + ((double)x)*CurrSeries->pixel_size;;
				fy = CurrSeries->offset_y + ((double)FrontView->height-(double)y)*CurrSeries->pixel_size;
				if ( FrontView->section->SelectClosestContour( fx, fy ) )
					{
					FrontView->needsDrawing = true;
					InvalidateRect( hWnd, NULL, FALSE );
					}
				}
			break;
*/
		}
}
