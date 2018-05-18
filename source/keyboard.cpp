/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Keyboard operations: transform movements, etc.
//
//    Copyright (C) 2003-2006  John Fiala (fiala@bu.edu)
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
// modified 11/01/04 by JCF (fiala@bu.edu)
// -+- change: Added Wildfire Tool to EscapeCurrentTool cases.
// modified 2/03/05 by JCF (fiala@bu.edu)
// -+- change: Added Scalpel tool to EscapeCurrentTool cases.
// modified 2/22/05 by JCF (fiala@bu.edu)
// -+- change: Added escape for RToolActive with SCALPEL_TOOL.
// modified 4/21/05 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added CmTraceForward, CmTraceBackward, CmStopTrace for autotrace
// modified 4/29/05 by JCF (fiala@bu.edu)
// -+- change: Moved Ju's DoAutoTrace() and supporting routines here from viewport methods.
// modified 6/8/05 by JCF (fiala@bu.edu)
// -+- change: Modified EscapeCurrentTool to reflect wildfire rectangle drag with left mouse.
// modified 6/17/05 by JCF (fiala@bu.edu)
// -+- change: Cleaned up more of Ju's code. Added areaStopPercent series option.
// modified 6/23/05 by JCF (fiala@bu.edu)
// -+- change: Moved Flip routine to section_menu.cpp.
// modified 7/7/05 by JCF (fiala@bu.edu)
// -+- change: Modified AdjustContrast() to do whole section if no domain selected.
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change: Modified CmMovement() to check for contours or images before moving section.
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Modified SetDefaultName params.
// modified 3/29/06 by JCF (fiala@bu.edu)
// -+- change: Rewrote Ju Lu's stuff so it would work properly.
// modified 6/19/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified DoAutoTrace to allow automatic adjustment of brightness threshold.
// modified 6/22/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified DoAutoTrace to utilize histogram for automatic adjustment of threshold.
// modified 6/30/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified DoAutoTrace to adapt threshold according to different stop criterion.
// modified 7/2/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified DoAutoTrace to utilize info stored in AutomaticTraces object for simultaneous multiple tracing
//             Created AdjustStopCriteria function
// modified 7/3/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified DoAutoTrace further.
//             Added AutoTraceOneContour function.
//             Added VerifyTraces function
// modified 11/13/06 by JCF (fiala@bu.edu)
// -+- change: Eliminated compiler WARNINGS/ERRORS so would compile under VC++.
// modified 11/15/06 by JCF (fiala@bu.edu)
// -+- change: Fixed DoAutoTrace to handle Active contours rather than separate AutomaticTraces.
// modified 11/16/06 by JCF (fiala@bu.edu)
// -+- change: Added CmGetClipboardAttributes() for Ctrl-G key access.
// modified 4/3/18 by BK (bobkuczewski@salk.edu)
// -+- change: Added CmHideAllTraces.

#include <iostream>
using namespace std;

#include "reconstruct.h"

void CmToggleViews( HWND hWnd )			// respond to '/' by switching views
{
	Section *tmpSection;
	ViewPort *tmpView;
	HMENU	hMenu = GetMenu( hWnd );

	if ( BlendView )					// if in blend mode, turn off blending
		{
		CheckMenuItem( hMenu, CM_BLEND, MF_BYCOMMAND | MF_UNCHECKED );
		delete BlendView;
		BlendView = NULL;
		}

	if ( DomainView )
		{
		tmpView = FrontView;			// swap view pointers only
		FrontView = BackView;
		BackView = tmpView;
		InvalidateRect( hWnd, NULL, FALSE );
		if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
		}
	else Previous();					// swap sections also so can page correctly
}

void CmAdjustContrast( HWND hWnd, double bfactor, double cfactor )
{
	bool done = false;
	Transform *transform;

	if ( FrontView )
	  if ( FrontView->section )
		{
		if ( FrontView->section->active )				// only mode active elements of FrontView Section
			if ( FrontView->section->active->image )
				{
				FrontView->section->PushUndoState();
				FrontView->section->active->image->brightness += bfactor;
				FrontView->section->active->image->contrast *= cfactor;
				done = true;
				FrontView->needsRendering = true;
				InvalidateRect( hWnd, NULL, FALSE );
				}
		if ( !done )									// if no domain selected, do whole seciton
			if ( FrontView->section->transforms )
				{
				FrontView->section->PushUndoState();
				transform = FrontView->section->transforms->first;
				while ( transform )
					{
					if ( transform->image )					// change contrast of every domain image
						{
						transform->image->brightness += bfactor;
						transform->image->contrast *= cfactor;
						}
					transform = transform->next;
					}
				FrontView->section->hasChanged = true;
				FrontView->needsRendering = true;		// flag view for regeneration
				InvalidateRect( hWnd, NULL, FALSE );	// repaint window
				}
		}
}

void CmMovement( HWND hWnd, WORD command )
{
	Transform *moveit;
	Mvmt mvmt;											// initialized for no movement in constructor
	Nform *adjustment;									// will also remember move so can repeat it

	mvmt = Movement;									// start from last type-in to get center values
	mvmt.Clear();										// clear everything in mvmt to no movement
	adjustment = NULL;
	if ( CurrSeries )
	  {
	  switch (command)									// set increment of commanded movement variable
		{
		case CM_RIGHT:			mvmt.transX = CurrSeries->Increment.transX; break;
		case CM_UP:				mvmt.transY = CurrSeries->Increment.transY; break;
		case CM_LEFT:			mvmt.transX = -CurrSeries->Increment.transX; break;
		case CM_DOWN:			mvmt.transY = -CurrSeries->Increment.transY; break;
		case CM_SLOWRIGHT:		mvmt.transX = CurrSeries->CtrlIncrement.transX; break;
		case CM_SLOWUP:			mvmt.transY = CurrSeries->CtrlIncrement.transY; break;
		case CM_SLOWLEFT:		mvmt.transX = -CurrSeries->CtrlIncrement.transX; break;
		case CM_SLOWDOWN:		mvmt.transY = -CurrSeries->CtrlIncrement.transY; break;
		case CM_FASTRIGHT:		mvmt.transX = CurrSeries->ShiftIncrement.transX; break;
		case CM_FASTUP:			mvmt.transY = CurrSeries->ShiftIncrement.transY; break;
		case CM_FASTLEFT:		mvmt.transX = -CurrSeries->ShiftIncrement.transX; break;
		case CM_FASTDOWN:		mvmt.transY = -CurrSeries->ShiftIncrement.transY; break;
		case CM_XSMALLER:		mvmt.scaleX = 1.0/CurrSeries->Increment.scaleX; break;
		case CM_XLARGER:		mvmt.scaleX = CurrSeries->Increment.scaleX; break;
		case CM_YSMALLER:		mvmt.scaleY = 1.0/CurrSeries->Increment.scaleY; break;
		case CM_YLARGER:		mvmt.scaleY = CurrSeries->Increment.scaleY; break;
		case CM_SLOWXSMALLER:	mvmt.scaleX = 1.0/CurrSeries->CtrlIncrement.scaleX; break;
		case CM_SLOWXLARGER:	mvmt.scaleX = CurrSeries->CtrlIncrement.scaleX; break;
		case CM_SLOWYSMALLER:	mvmt.scaleY = 1.0/CurrSeries->CtrlIncrement.scaleY; break;
		case CM_SLOWYLARGER:	mvmt.scaleY = CurrSeries->CtrlIncrement.scaleY; break;
		case CM_FASTXSMALLER:	mvmt.scaleX = 1.0/CurrSeries->ShiftIncrement.scaleX; break;
		case CM_FASTXLARGER:	mvmt.scaleX = CurrSeries->ShiftIncrement.scaleX; break;
		case CM_FASTYSMALLER:	mvmt.scaleY = 1.0/CurrSeries->ShiftIncrement.scaleY; break;
		case CM_FASTYLARGER:	mvmt.scaleY = CurrSeries->ShiftIncrement.scaleY; break;
		case CM_CLKWISE:		mvmt.theta = -CurrSeries->Increment.theta; break;
		case CM_COUNTERCLKWISE: mvmt.theta = CurrSeries->Increment.theta; break;
		case CM_SLOWCLKWISE:	mvmt.theta = -CurrSeries->CtrlIncrement.theta; break;
		case CM_SLOWCOUNTERCLKWISE: mvmt.theta = CurrSeries->CtrlIncrement.theta; break;
		case CM_FASTCLKWISE:	mvmt.theta = -CurrSeries->ShiftIncrement.theta; break;
		case CM_FASTCOUNTERCLKWISE: mvmt.theta = CurrSeries->ShiftIncrement.theta; break;
		}
	  if ( UseDeformKeys )
		switch ( command )
			{
			case CM_SLANTXLEFT:		mvmt.deformX = -CurrSeries->Increment.deformX; break;
			case CM_SLANTXRIGHT:	mvmt.deformX = CurrSeries->Increment.deformX; break;
			case CM_SLANTYUP:		mvmt.deformY = CurrSeries->Increment.deformY; break;
			case CM_SLANTYDOWN:		mvmt.deformY = -CurrSeries->Increment.deformY; break;
			case CM_SLOWSLANTXLEFT:		mvmt.deformX = -CurrSeries->CtrlIncrement.deformX; break;
			case CM_SLOWSLANTXRIGHT:	mvmt.deformX = CurrSeries->CtrlIncrement.deformX; break;
			case CM_SLOWSLANTYUP:		mvmt.deformY = CurrSeries->CtrlIncrement.deformY; break;
			case CM_SLOWSLANTYDOWN:		mvmt.deformY = -CurrSeries->CtrlIncrement.deformY; break;
			case CM_FASTSLANTXLEFT:		mvmt.deformX = -CurrSeries->ShiftIncrement.deformX; break;
			case CM_FASTSLANTXRIGHT:	mvmt.deformX = CurrSeries->ShiftIncrement.deformX; break;
			case CM_FASTSLANTYUP:		mvmt.deformY = CurrSeries->ShiftIncrement.deformY; break;
			case CM_FASTSLANTYDOWN:		mvmt.deformY = -CurrSeries->ShiftIncrement.deformY; break;
			}
	  else
		switch ( command )
			{
			case CM_SLANTXLEFT:		mvmt.slantX = -CurrSeries->Increment.slantX; break;
			case CM_SLANTXRIGHT:	mvmt.slantX = CurrSeries->Increment.slantX; break;
			case CM_SLANTYUP:		mvmt.slantY = CurrSeries->Increment.slantY; break;
			case CM_SLANTYDOWN:		mvmt.slantY = -CurrSeries->Increment.slantY; break;
			case CM_SLOWSLANTXLEFT:		mvmt.slantX = -CurrSeries->CtrlIncrement.slantX; break;
			case CM_SLOWSLANTXRIGHT:	mvmt.slantX = CurrSeries->CtrlIncrement.slantX; break;
			case CM_SLOWSLANTYUP:		mvmt.slantY = CurrSeries->CtrlIncrement.slantY; break;
			case CM_SLOWSLANTYDOWN:		mvmt.slantY = -CurrSeries->CtrlIncrement.slantY; break;
			case CM_FASTSLANTXLEFT:		mvmt.slantX = -CurrSeries->ShiftIncrement.slantX; break;
			case CM_FASTSLANTXRIGHT:	mvmt.slantX = CurrSeries->ShiftIncrement.slantX; break;
			case CM_FASTSLANTYUP:		mvmt.slantY = CurrSeries->ShiftIncrement.slantY; break;
			case CM_FASTSLANTYDOWN:		mvmt.slantY = -CurrSeries->ShiftIncrement.slantY; break;
			}

	  if ( FrontView )
		if ( FrontView->section )
		  if ( FrontView->section->active )			// move active elements of FrontView Section
			{
			FrontView->section->PushUndoState();
			FrontView->section->active->nform->PostApply( mvmt );	// move in section(screen) coords
			adjustment = new Nform();
			adjustment->PostApply( mvmt );
			FrontView->section->hasChanged = true;
			if ( FrontView->section->active->image ) FrontView->needsRendering = true;
			else FrontView->needsDrawing = true;		// only need to draw contours if not image
			InvalidateRect( hWnd, NULL, FALSE );
			}
		  else
			if ( !FrontView->section->alignLocked )// if no active elements, move entire section if unlocked
				 if ( FrontView->section->HasImage() || FrontView->section->HasContour() )
					{
					FrontView->section->PushUndoState();
					adjustment = new Nform();
					adjustment->PostApply( mvmt );
					moveit = FrontView->section->transforms->first;
					while ( moveit )
						{									// always move in section coordinates
						moveit->nform->PostApply( mvmt );
						moveit = moveit->next;
						}
					FrontView->section->hasChanged = true;
					FrontView->needsRendering = true;		// flag view for regeneration
					InvalidateRect( hWnd, NULL, FALSE );	// repaint window
					}

	  if ( adjustment )								// if movement was made, update LastAdjustment
		{
		if ( LastAdjustment ) delete LastAdjustment;
		LastAdjustment = adjustment;				// and add mvmt to recording
		if ( Recording ) Recording->PostApply( mvmt );
		}
	  }
}

void CmBackspace( void )		// only works with tools that create an EditContour while drawing
{
	Point *p;
	double fx, fy;
	RECT r;

	if ( CurrSeries && EditContour && FrontView )		// contour is being drawn in CurrSeries
	   if ( EditContour->points )
		  if ( EditContour->points->Number() > 1 )		// don't remove first contour point
			{
			EditContour->points->DeleteFirst();			// remove contour point at head of contour
			FrontView->needsDrawing = true;
			GetClientRect( appWnd, &r );
			PaintViews( appDC, r );						// redraw modified contour (and erase drag lines)
			if ( LToolActive )
				{										// if dragging...
				p = EditContour->points->first;				// adjust drag line start position to p
				fx = (p->x - CurrSeries->offset_x)/CurrSeries->pixel_size;	// convert to pixels
				fy = (p->y - CurrSeries->offset_y)/CurrSeries->pixel_size;
				LToolRect.left = (int)floor(fx);							// adjust LToolRect to
				LToolRect.top = FrontView->height - (int)floor(fy);			// screen coord. of p
				MoveToEx( appDC, LToolRect.left, LToolRect.top, NULL );
				LineTo( appDC, LToolRect.right, LToolRect.bottom );	// so can correctly draw new drag line
				}
			}
}

void CmEscapeCurrentTool( void )				// abort tool operation when press escape key
{
	switch ( CurrentTool )
		{
		case ZOOM_TOOL:							// return to view position at start of drag
			if ( LToolActive )
				if ( BlendView ) BlendView->Pan( appDC, 0, 0 );
				else FrontView->Pan( appDC, 0, 0 );
			if ( RToolActive )
				if ( BlendView ) BlendView->Zoom( appDC, 1.0, 0, 0 );
				else FrontView->Zoom( appDC, 1.0, 0, 0 );
			EndDrag();							// release DC and cursor captured by BeginDrag()
			LToolActive = false;
			RToolActive = false;
			break;

		case DOMAIN_TOOL:						// just undo shift, no R2_NOT lines on screen
			if ( LToolActive && DomainView ) DomainView->Pan( appDC, 0, 0 );
			LToolActive = false;
			EndDrag();							// release DC and cursor
			break;

		case PENCIL_TOOL:
			if ( LToolActive )
				{
				EndDrag();						// release DC and cursor
				LToolActive = false;
				if ( ToolContour ) delete ToolContour;
				ToolContour = NULL;
				InvalidateRect( appWnd, NULL, FALSE );	// cleanup ROP2_NOT drawing
				}
			if ( RToolActive )
				{
				EndDrag();
				RToolActive = false;
				}
			break;

		case GRID_TOOL:
		case POINT_TOOL:
			if ( RToolActive )
				{
				EndDrag();
				RToolActive = false;
				}
			break;

		case ELLIPSE_TOOL:
			if ( LToolActive )					// erase temporary ellipse drag drawing
				{
				if ( !ScrollOccurred )
					Ellipse( appDC, LToolRect.left, LToolRect.top, LToolRect.right, LToolRect.bottom );	// erase
				EndDrag();						// release DC and cursor
				LToolActive = false;
				}
			break;

		case LINE_TOOL:
			if ( LToolActive )					// erase temporary drag line drawing if active
				{
				if ( !ScrollOccurred )
					{
					MoveToEx( appDC, LToolRect.left, LToolRect.top, NULL );
					LineTo( appDC, LToolRect.right, LToolRect.bottom );
					}
				EndDrag();						// release DC and cursor
				LToolActive = false;
				}
			break;

		case ZLINE_TOOL:
		case MULTILINE_TOOL:
		case CULTILINE_TOOL:
			if ( LToolActive )					// erase temporary drawing if active
				{
				EndDrag();								// release DC and cursor
				if ( EditContour ) delete EditContour;
				EditContour = NULL;						// stop editing contour
				FrontView->needsDrawing = true;			// erase EditContour
				if ( BackView )
					BackView->needsDrawing = true;		// erase back trace which may exist
				InvalidateRect( appWnd, NULL, FALSE );	// cleanup drawing
				LToolActive = false;
				}
			break;

		case ARROW_TOOL:
		case MAGNIFY_TOOL:
		case RECTANGLE_TOOL:					// both tools need drag rectangle erase
			if ( LToolActive )
				{
				if ( !ScrollOccurred )
					Rectangle( appDC, LToolRect.left, LToolRect.top, LToolRect.right, LToolRect.bottom );
				EndDrag();						// release DC and cursor
				LToolActive = false;
				}
			break;

		case WILDFIRE_TOOL:
			if ( RToolActive )					// cleanup fire break contour
				{
				if ( ToolContour ) delete ToolContour;
				ToolContour = NULL;
				}
			EndDrag();								// release DC and cursor
			RToolActive = false;
			LToolActive = false;
			InvalidateRect( appWnd, NULL, FALSE );	// cleanup any ROP2_NOT drawing
			break;

		case SCALPEL_TOOL:
			if ( LToolActive )					// if using tool left button, abort drag+draw
				{
				EndDrag();						// release DC and cursor
				LToolActive = false;
				if ( ToolContour ) delete ToolContour;
				ToolContour = NULL;
				InvalidateRect( appWnd, NULL, FALSE );	// cleanup ROP2_NOT drawing
				}
			if ( RToolActive )
				{
				EndDrag();
				RToolActive = false;			// if right button, just inactivate
				}
			break;

		default:								// other tools have nothing to cleanup!
			break;
		}

	KillTimer( appWnd, SCROLL_TIMER );			// in case section is scrolling, stop it
	Scrolling = false;
}


void CmPrecisionCursor( void )
{
	UsePrecisionCursor = !UsePrecisionCursor;	// switch precision cursor state
	SetToolCursor( appWnd );					// and update cursor diasplay
}

void CmHideTraces( void )									// hide selected traces
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
				contour->hidden = true;
				FrontView->section->hasChanged = true;			// flag change for save
				FrontView->needsDrawing = true;
				contour = contour->next;						// check all active contours
				}
			FrontView->section->active->isActive = false;		// remove hidden contours from active list
			FrontView->section->active = NULL;
			if ( FrontView->needsDrawing )
				{
				InvalidateRect( appWnd, NULL, FALSE );
				if ( IsWindow(traceList) ) FillTraceList( traceList, FrontView->section );
				}
			}
}

void CmHideAllTraces( void )									// hide all traces
{
	Contour *contour;

	if ( (FrontView == NULL) || (FrontView->section == NULL) ) {
		cout << "FrontView Section is NULL" << endl;
	} else {
		// Find the first section
		// Section *sect = CurrSection;
		Section *sect = FrontView->section;
		Transform *trans;

		// First determine if any traces are currently visible (for toggling)
        bool any_visible = false;
		if (sect->transforms != NULL) {
			trans = sect->transforms->first;
			while (trans != NULL) {
				if (trans->contours != NULL) {
					Contour *c = trans->contours->first;
					while (c != NULL) {
						if (!(c->hidden)) {
							any_visible = true;
						}
						sect->hasChanged = true;
						c = c->next;
					}
				}
				trans = trans->next;
			}
			trans = sect->transforms->first;
			while (trans != NULL) {
				if (trans->contours != NULL) {
					Contour *c = trans->contours->first;
					while (c != NULL) {
						c->hidden = any_visible;
						sect->hasChanged = true;
						c = c->next;
					}
				}
				trans = trans->next;
			}
		}
		FrontView->needsDrawing = true;
	}
	if ( FrontView->needsDrawing ) {
		InvalidateRect( appWnd, NULL, FALSE );
		if ( IsWindow(traceList) ) {
			FillTraceList( traceList, FrontView->section );
		}
	}
}

void CmNextWindow( HWND currWnd )			// switch active foreground window in response to ctrl-Tab
{
	int i, j;
	HWND Wnds[NUM_WINDOWS];					// fill array with doubleing window handles

	Wnds[0] = appWnd;						// at least one of this is not NULL and equal to currWnd!
	Wnds[1] = toolbarWindow;
	Wnds[2] = sectionWindow;
	Wnds[3] = thumbnailWindow;
	Wnds[4] = traceWindow;
	Wnds[5] = domainWindow;
	Wnds[6] = paletteWindow;
	Wnds[7] = objectWindow;
	Wnds[8] = openGLWindow;
	Wnds[9] = zTraceWindow;
	Wnds[10] = distanceWindow;

	i=0;
	while ( currWnd != Wnds[i] ) i++;			// find which one (i) is currently active

	j = i+1; if ( j >= NUM_WINDOWS ) j = 0;		// go to next window in array

	while ( Wnds[j] == NULL )					// if it doesn't exist, keep looking
		{
		j++;
		if ( j >= NUM_WINDOWS ) j = 0;
		}										// appWnd must exist!

	SetFocus( Wnds[j] );						// give it the keyboard focus
}

void CmGetClipboardAttributes(void)				// put clipboard trace attributes into defaults
{
	Contour *contour;

	if ( ClipboardTransform )
		if ( ClipboardTransform->contours )
			{
			contour = ClipboardTransform->contours->first;
			strcpy(CurrSeries->defaultName,contour->name);
			strcpy(CurrSeries->defaultComment,contour->comment);
			CurrSeries->defaultBorder = contour->border;
			CurrSeries->defaultFill = contour->fill;
			CurrSeries->defaultMode = contour->mode;
			}
}

void CmTraceForward(void)						// initiate forward autotracing
{
	if ( CurrSeries )							// do only if series is open and
	  if ( FrontView != DomainView )			//  a non-domain section is in front
		{
		AutoTrace=1;							// timer interrupts do tracing
		SetTimer( appWnd, AUTOTRACE_TIMER, 250, NULL);
		}
}

void CmTraceBackward(void)						// initiate backward autotracing
{
	if ( CurrSeries )							// do only is series is open and
	  if ( FrontView != DomainView )			//  a non-domain section is visible
		{
		AutoTrace=-1;							// timer interrupts do tracing
		SetTimer( appWnd, AUTOTRACE_TIMER, 250, NULL);
		}
}

/*
void AdjustStopCriteria(int sh, int ss, int sv, int ch, int cs, int cv)
{
	// if stop criterion is
	// (1) less than or larger than, then shift the threshold according to the difference
	//		between previous and current contours average value
	// (2) "equal to", do not change it
	// (3) "differs by", do not change it

	switch (CurrSeries->hueStopWhen)					// test hue color
	{
		case 0:											// stop when less than
		case 2: 										// stop when greater than
			CurrSeries->hueStopValue = int((CurrSeries->hueStopValue + ch - sh));
			if (CurrSeries->hueStopValue < 0)
				CurrSeries->hueStopValue = CurrSeries->hueStopValue + HLSMAX;
			else if (CurrSeries->hueStopValue > HLSMAX)
				CurrSeries->hueStopValue = CurrSeries->hueStopValue - HLSMAX;
			break;
		case 1:											// stop when equal
		case 3:											// stop when differs by
			break;
		default:										// stop when less than
			CurrSeries->hueStopValue = int((CurrSeries->hueStopValue + ch - sh));
			if (CurrSeries->hueStopValue < 0)
				CurrSeries->hueStopValue = CurrSeries->hueStopValue + HLSMAX;
			else if (CurrSeries->hueStopValue > HLSMAX)
				CurrSeries->hueStopValue = CurrSeries->hueStopValue - HLSMAX;
			break;
	}

	switch (CurrSeries->satStopWhen)					// test sat color
	{
		case 0:											// stop when less than
		case 2: 										// stop when greater than
			CurrSeries->satStopValue = int((CurrSeries->satStopValue + cs - ss));
			if (CurrSeries->satStopValue<0)
				CurrSeries->satStopValue=0;
			else if (CurrSeries->satStopValue>HLSMAX)
				CurrSeries->satStopValue=HLSMAX;
			break;
		case 1:											// stop when equal
		case 3:											// stop when differs by
			break;
		default:										// stop when less than
			CurrSeries->satStopValue = int((CurrSeries->satStopValue + cs - ss));
			if (CurrSeries->satStopValue<0)
				CurrSeries->satStopValue=0;
			else if (CurrSeries->satStopValue>HLSMAX)
				CurrSeries->satStopValue=HLSMAX;
			break;
	}

	switch (CurrSeries->brightStopWhen)					// test bright color
	{
		case 0:											// stop when less than
		case 2: 										// stop when greater than
			CurrSeries->brightStopValue = int((CurrSeries->brightStopValue + cv - sv));
			if (CurrSeries->brightStopValue<0)
				CurrSeries->brightStopValue=0;
			else if (CurrSeries->brightStopValue>HLSMAX)
				CurrSeries->brightStopValue=HLSMAX;
			break;
		case 1:											// stop when equal
		case 3:											// stop when differs by
			break;
		default:										// stop when less than
			CurrSeries->brightStopValue = int((CurrSeries->brightStopValue + cv - sv));
			if (CurrSeries->brightStopValue<0)
				CurrSeries->brightStopValue=0;
			else if (CurrSeries->brightStopValue>HLSMAX)
				CurrSeries->brightStopValue=HLSMAX;
			break;
	}
}

void getHistogramStat( Histogram* &histogram, int& h, int& s, int& v)
{	// h, s, v stands for the corresponding stat for HSV color system components


	// my experience shows that the mean of pixels about certain percentile seems a better
	// indicator than the mean value
	double percentile = 0.5; // this value is empirical and may be changed.

	h = histogram->MeanValue('h');
	s = histogram->MeanValue('s');
	//v = histogram->MeanValue('v');
	v = histogram->PercentileValue('v', percentile);
	//printf("v=%d\n",v);
}


bool AutoTraceOneContour(AutomaticTraces* &traces, AutomaticTrace* const &pTrace, const double &AreaUpperLimit, const double &AreaLowerLimit, ADib* region)
{
	// trace on contour from pTrace, and add the resulted contour and associated info in traces

	// global variables:
	// CurrSeries, CurrHistogram, AutoAdjustThreshold


	Contour *editContour = NULL;
	double CentroidX, CentroidY, SrcContourArea, EditContourArea;
	int SeedX, SeedY;
	bool stop = false;

	// read out recorded trace info
	// get wildfire seed

	//////////////////////////////////////////////////////////////////////////////////////////////
	//previously working code:
	//Contour *srcContour = NULL;
	//pTrace->setContour(srcContour);						// set srcContour
	//srcContour->GreensCentroidArea(CentroidX, CentroidY, SrcContourArea );
	//SeedX = (int)floor(CentroidX);						// srcContour is in view bitmap coordinates!
	//SeedY = (int)floor(CentroidY);
	//delete srcContour;	srcContour = NULL;
	///////////////////////////////////////////////////////////////////////////////////////////////
    Contour* srcContour;
	srcContour = pTrace->accessContour();
	srcContour->GreensCentroidArea(CentroidX, CentroidY, SrcContourArea );
	SeedX = (int)floor(CentroidX);						// srcContour is in view bitmap coordinates!
	SeedY = (int)floor(CentroidY);
	//srcContour = 0;				                        // shouldn't delete it because it points to an object inside *pTrace!!!


	//////////////////////////////////////////////////////////////////////////////////////////////
	// previously working code:
	// get source histogram
	//Histogram* srcHistogram = NULL;
	//pTrace->setHistogram(srcHistogram);					// set srcHistogram
	//int sh, ss, sv;
	//getHistogramStat(srcHistogram, sh, ss, sv);
	//delete srcHistogram; srcHistogram = NULL;
	//////////////////////////////////////////////////////////////////////////////////////////////
	Histogram* srcHistogram;
	srcHistogram = pTrace->accessHistogram();
	//printf("src mean v = %d\n",srcHistogram->MeanValue('v'));
	int sh, ss, sv;
	getHistogramStat(srcHistogram, sh, ss, sv);
	//printf("sv=%d\n",sv);
	srcHistogram = 0;

	pTrace->setStopCriteria(CurrSeries);
	pTrace->setContourDefaults(CurrSeries);

	//editContour = region->Wildfire(SeedX,SeedY );

	if (!editContour)
		stop = true;
	else
	{
		if (AutoAdjustThreshold) // prediction -> adjustment -> generation of new contour
		{
			//printf("auto adjust threshold!\n");
			int ch, cs, cv;		// s/c = source/current, h/s/v = hue/sat/bright
			editContour->GreensCentroidArea(CentroidX, CentroidY, EditContourArea );
			getHistogramStat(CurrHistogram, ch, cs, cv);
			if (cv != 0) // trial does generate a contour
				AdjustStopCriteria(sh, ss, sv, ch, cs, cv);
			delete editContour;

			// now need to redraw a contour. because previously region's ADib has been modified during Wildfire
			// a new region is needed
			delete region;
			region = FrontView->CopyView( CurrSeries->tracesStopWhen );
//			editContour = region->Wildfire( SeedX, SeedY );
		}

		if ( !editContour )											// see if region is growing or shrinking too much
			stop = true;
		else
		{
			editContour->GreensCentroidArea(CentroidX, CentroidY, EditContourArea );
			if ( (EditContourArea > SrcContourArea*AreaUpperLimit) || (EditContourArea < SrcContourArea*AreaLowerLimit) )
			{
				delete editContour; 					// if so, give up on autotracing
				stop = true;
			}
			else
			{
				StoreTracingInfo(traces, CurrSeries, CurrHistogram, editContour);
				AddContour(editContour);
				FrontView->needsDrawing = true;					// redraw traces for user's pleasure

				editContour = NULL;								// editContour is added so don't delete it.
				//InvalidateRect( appWnd, NULL, FALSE );		// was working. now moved to outside the while loop
			}
		} // if !editContour
	}	// if !editContour
	return stop;

}
*/
								// this gets called repeatedly by WM_TIMER message in reconstruct.cpp
void DoAutoTrace(void)			// it generates a new wildfire based on the centroid of the last one
{								// PROBABLY SHOULD ALSO DEVISE NON_CENTROID METHOD FOR CONCAVE CASES
	Contour *contour, *c, *wc;
	int autotrace, SeedX, SeedY;
	double x, y, area, wc_area;
	ADib *region;
	const double AreaUpperLimit=1.0+CurrSeries->areaStopPercent/100.0;
	const double AreaLowerLimit=1.0-CurrSeries->areaStopPercent/100.0;

	if ( FrontView->needsRendering || FrontView->needsDrawing )	// wait until display completes
		return;

	KillTimer( appWnd, AUTOTRACE_TIMER );						// shut off timer interrupts until ready to continue

	switch (AutoTrace) {				// vairable indicates processing step and direction in autotracing
	case (1):
		if ( FrontView->section->index < CurrSectionsInfo->last->index )
			{
			CmSuccessor();				// page up
			AutoTrace = 2;				// go to second step (tracing)
			}
		else AutoTrace = 0;				// quit autotracing
		break;

	case (-1):
		if ( FrontView->section->index > CurrSectionsInfo->first->index )
			{
			CmPredecessor();			// page down
			AutoTrace = -2;				// go to second step (tracing)
			}
		else AutoTrace = 0;				// quit autotracing
		break;

	case (2):							// step two: generate new trace(s)
	case (-2):
		autotrace = 0;
												// must have front view and back view (previous section)
		if ( FrontView && BackView)
		  if ( FrontView->section && BackView->section )
			if ( BackView->section->active )		// must have active contours on back view to propagate
				if ( BackView->section->active->contours )
					{
					FrontView->section->UnSelectAll();	// don't merge result with existing selected contours
					region = FrontView->CopyView( CurrSeries->tracesStopWhen );
					contour = BackView->section->active->contours->first;
					while ( contour && region )
						{							// generate Wildfire from each contour centroid
						contour->GreensCentroidArea( x, y, area );
						BackView->section->active->nform->XYinverse( &x, &y ); // tform to view port
						SeedX = (int)floor( (x-CurrSeries->offset_x)/CurrSeries->pixel_size );
						SeedY = (int)floor( (y-CurrSeries->offset_y)/CurrSeries->pixel_size );
						wc = region->Wildfire( SeedX, SeedY, CurrSeries->areaStopSize ); // try wildfire
						if ( wc )					// add wildfire contour to FrontView->section
							{
							wc->Reduce( 1.0 );							// reduce # of contour points
							wc->GreensCentroidArea( x, y, wc_area );	// check change in area
							wc_area = wc_area*CurrSeries->pixel_size*CurrSeries->pixel_size;
							if ( (wc_area <= area*AreaUpperLimit) && (wc_area >= area*AreaLowerLimit) )
								{
								FrontView->section->AddViewPortContour(wc); // keep new trace, don't delete
								if ( FrontView == DomainView ) FrontView->needsRendering = true;
								else FrontView->needsDrawing = true;	// want domain image or to redraw traces
								autotrace = AutoTrace/2;				// will continue to next section
								}
							else delete wc;								// trash trace if area test fails
							}
						contour = contour->next;	// do next contour from back section
						}
					delete region;								// release bitmap memory
					if ( autotrace )
						InvalidateRect( appWnd, NULL, FALSE );	// display autotrace result on screen
					}
		AutoTrace = autotrace;			// continue to next section or abandon autotracing
		break;

	default:
		AutoTrace = 0;					// no more autotracing!
	}

	if ( AutoTrace != 0 ) SetTimer( appWnd, AUTOTRACE_TIMER, 250, NULL);		// restart timer interrupts to continue
}
