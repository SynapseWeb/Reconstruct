/////////////////////////////////////////////////////////////////////////
//	This file contains code for the doubleing thumbnails window
//
//    Copyright (C) 2003-2007  John Fiala (fiala@bu.edu)
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
// modified 11/22/06 by JCF (fiala@bu.edu)
// -+- change: Switched CreateThumbButtons() from reading section files to using CurrSectionInfo data.
// modified 7/16/07 by JCF (fiala@bu.edu)
// -+- change: Added enable of Maximize sys menu cmd in WM_CREATE of Thumbnails Window.
//

#include "reconstruct.h"

BOOL CALLBACK MoveThumbButtons( HWND hWnd, LPARAM lParam )	// reposition buttons
{
	RECT *pos;								// pos is in parent's client coordinates
	int part;
	pos = (RECT*)lParam;					// place window based on previous values

	MoveWindow( hWnd, pos->left, pos->top, CurrSeries->thumbWidth, CurrSeries->thumbHeight, FALSE );

	part = pos->bottom - pos->top;			// remaining part of client area
	if ( part < CurrSeries->thumbHeight ) yThumbsOver = CurrSeries->thumbHeight - part;

	pos->left += CurrSeries->thumbWidth;				// set where next button will go
	if ( pos->left >= pos->right )			// row full, instead start next row
		{
		xThumbsOver = pos->left - pos->right;	// part of thumb right of client area
		pos->top += CurrSeries->thumbHeight;
		pos->left = xThumbsPos;				// start to left of window if necessary
		}
	return TRUE;
}

void CreateThumbButtons( HWND hWnd, HINSTANCE hInstance )	// create buttons for thumbnails
{
	Button *button;
	int x, y, w, h, lastSection;
	SectionInfo *info;

	ThumbButtons = new Buttons();							// create list for buttons
	w = CurrSeries->thumbWidth; h = CurrSeries->thumbHeight;
	x = 0; y = 0;	
	lastSection = CurrSeries->firstThumbSection - CurrSeries->skipSections;
	info = CurrSectionsInfo->first;
	while ( info )											// find next section in series
		{
		if ( info->index > CurrSeries->lastThumbSection ) break;		// quit if done all sections in range
		if ( info->index > lastSection+CurrSeries->skipSections-1 )	// use only if sectnum has skipped enough
			{
			button = new Button();										// section is valid, so save info
			button->number = info->index;
			lastSection = info->index;
			button->hwnd = CreateWindow( "BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE | BS_BITMAP,
															x, y, w, h, hWnd, NULL, hInstance, NULL );
			if ( CurrSeries->useFlipbookStyle ) ShowWindow( button->hwnd, SW_HIDE );
			ThumbButtons->Add( button );
			}
		info = info->next;
		}
}

										// Windows procedure for flipbook style thumbnails

LRESULT APIENTRY FlipbookProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int p, i, min, max;
	HDC  hdc, lhdc;
	PAINTSTRUCT ps;
	HWND chWnd;
	HMENU hMenu;
	HCURSOR cur;
	DWORD status;
	Button *button;
	char section[MAX_PATH];

	switch(msg)
		{
		case WM_DESTROY:
			if ( hThumbsThread ) AbortThumbs = true;	// signal thread to terminate if still active
			KillTimer( hWnd, THUMBS_TIMER );			// halt timer interrupts

			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// don't destroy windows until thread is done
			GetExitCodeThread( hThumbsThread, &status );
			while ( status == STILL_ACTIVE )				// wait for thread to finish aborting
				{
				Sleep(33);									// release control to thread for 33ms slice
				GetExitCodeThread( hThumbsThread, &status );// see if thread is done yet
				}
			SetCursor( cur );								// depending on section data, might be awhile

			flippage = 0;
			if ( ThumbButtons ) delete ThumbButtons;	// delete button memory
			ThumbButtons = NULL;
			hMenu = GetMenu( appWnd );					// uncheck menu item
			CheckMenuItem( hMenu, CM_THUMBNAILS, MF_BYCOMMAND | MF_UNCHECKED );
			thumbnailWindow = NULL;						// clear global handle pointer
			SetFocus(appWnd);							// return to main window
			break;

		case WM_CREATE:									// setup thumbnails
			CurrButtonWnd = NULL;
			CreateThumbButtons( hWnd, appInstance );
			ThumbsThreadBegin();						// background thread with render bitmaps
			hMenu = GetSystemMenu( hWnd, FALSE );		// allow Maximize on titlebar menu
			EnableMenuItem( hMenu, SC_MAXIMIZE, MF_ENABLED );
			// Windows will issue WM_SIZE immediately so complete setup there
			break;

		case WM_PAINT:									// use parent window paint to draw button canvas directly
			if ( GetUpdateRect(hWnd,NULL,FALSE) && CurrButtonWnd )
				{
				button = ThumbButtons->first;			// find current button window in list
				while ( button )
					if ( button->hwnd == CurrButtonWnd ) break;
					else button = button->next;
				if ( button )							// use canvas of button to paint window client area
					{
					hdc = BeginPaint( hWnd, &ps );
					lhdc = CreateCompatibleDC(hdc);		// create device context for canvas and blt it to window
					SelectObject(lhdc, button->canvas);
					BitBlt( hdc, 0, 0, CurrSeries->thumbWidth, CurrSeries->thumbHeight, lhdc, 0, 0, SRCCOPY);
					DeleteDC(lhdc);
					EndPaint( hWnd, &ps );
					}
				}
			break;

		case WM_LBUTTONUP:								// when all buttons are hidden, this will receive
			button = ThumbButtons->first;				// mouse click -- go to corresponding section
			while ( button )
				if ( button->hwnd == CurrButtonWnd ) break;
				else button = button->next;
			if ( button )
				{
				i = button->number;
				if ( FindSection( i, 0, section ) )	
						GotoSection( i, section );
				SetFocus( appWnd );						// switch focus to main window
				}
			break;

		case WM_MOUSEWHEEL:								// use scroll processing also for mousewheel
			flippage = 0;
			KillTimer( hWnd, THUMBS_TIMER );					
		case WM_HSCROLL:								// flipping of buttons
			p = GetScrollPos( hWnd, SB_HORZ );
			GetScrollRange( hWnd, SB_HORZ, &min, &max );
			if ( msg == WM_HSCROLL )					// for scrollbar mvmt, get section increment
			  switch ( LOWORD(wParam) )
				{
         		case SB_LINEDOWN:						// up one section in list
					i = 1;
					break;
         		case SB_LINEUP:							// down one section in list
					i = -1;
					break;
				case SB_PAGEDOWN:						// do nothing now, but start timer for shuttle
					i = 0;
					if ( flippage ) { flippage = 0; KillTimer( hWnd, THUMBS_TIMER ); }
					else {
						flippage = 1;
						PostMessage(hWnd, WM_HSCROLL, MAKELONG(SB_LINEDOWN, 0), 0L);
						SetTimer(hWnd, THUMBS_TIMER, 1000/CurrSeries->flipRate, NULL);
						} 
					break;
				case SB_PAGEUP:							// user clicked scrollbar, start/stop shuttle
					i = 0;
					if ( flippage ) { flippage = 0; KillTimer( hWnd, THUMBS_TIMER ); }
					else {
						flippage = -1;
						PostMessage(hWnd, WM_HSCROLL, MAKELONG(SB_LINEUP, 0), 0L);
						SetTimer(hWnd, THUMBS_TIMER, 1000/CurrSeries->flipRate, NULL);
						}
					break;
				case SB_THUMBTRACK:
					i = HIWORD(wParam)-p;				// get movement of scroll tab
					KillTimer( hWnd, THUMBS_TIMER );
					break;
				default: i = 0;
				}
			else										// MOUSEWHEEL movements are one button only
				if ( (int)wParam > 0 ) i = 1;
				else i = -1;
			if ( (p+i) > max ) i = max-p;				// limit thumb mvmt to button range
			if ( (p+i) < min ) i = min-p;
			p += i;										// compute new thumb position
			if ( i )
			  {
			  button = ThumbButtons->first;			// find current button window in list
			  while ( button )
				if ( button->hwnd == CurrButtonWnd ) break;
				else button = button->next; 
			  while ( i && button )						// move relative to button in list
				{
				if ( i < 0 ) { button = button->prev; i++; } // backup through list
				if ( i > 0 ) { button = button->next; i--; } // forward through list
				}
			  if ( button )								// found new button to display
				{										// display section # represented by button
				ShowWindow( CurrButtonWnd, SW_HIDE );
				CurrButtonWnd = button->hwnd;
				sprintf(section,"Section %d",button->number);
				SetWindowText( hWnd, section );
				if ( hThumbsThread )					// if all thumbs have not been rendered
					ShowWindow( CurrButtonWnd, SW_SHOW );// then keep displaying buttons, otherwise
				else InvalidateRect( hWnd, NULL, FALSE );// switch to direct drawing in parent window
				}										// to provide smoother flipbook scrolling
			  }
			SetScrollPos( hWnd, SB_HORZ, p, TRUE );		// update thumb pos
      		break;

		case WM_TIMER:									// timeouts cause scrolling of one section
			p = GetScrollPos( hWnd, SB_HORZ );
			GetScrollRange( hWnd, SB_HORZ, &min, &max );
			if ( (p <= min ) || (p >= max) ) flippage = -flippage;
			if ( flippage > 0 ) PostMessage(hWnd, WM_HSCROLL, MAKELONG(SB_LINEDOWN, 0), 0L);
			else if ( flippage < 0 ) PostMessage(hWnd, WM_HSCROLL, MAKELONG(SB_LINEUP, 0), 0L);
      		break;

		case WM_SIZE:									// at start up or resize, restart section list
			KillTimer( hWnd, THUMBS_TIMER );
			SetScrollRange( hWnd, SB_HORZ, 1, ThumbButtons->Number(), FALSE );
			SetScrollRange( hWnd, SB_VERT, 0, 0, FALSE );
			SetScrollPos( hWnd, SB_HORZ, 1, TRUE );		// update thumb pos to first section
			button = ThumbButtons->first;				// display first button's section#
			sprintf(section,"Section %d",button->number);
			SetWindowText( hWnd, section );
			CurrButtonWnd = button->hwnd;				// and show its window
			ShowWindow( CurrButtonWnd, SW_SHOW );
			break;

		case WM_KEYDOWN:
			switch ( wParam )
				{										// allow control+TAB to switch active window
				case VK_TAB:
					if ( GetKeyState(VK_CONTROL) & 0x8000 ) CmNextWindow( thumbnailWindow );
					else PostMessage(hWnd, WM_HSCROLL, MAKELONG(SB_PAGEDOWN, 0), 0L);	// TAB to start shuttle
					break;
				case VK_RETURN:							// make like user clicked button
					PostMessage( CurrButtonWnd, BM_CLICK, 0, 0 );
					break;
				}
			break;

		case WM_COMMAND:								// if user presses button before they are hidden...
			if ( (LOWORD(wParam) == BN_CLICKED) && (ThumbButtons != NULL) )
				{
				chWnd = (HWND)lParam;					// lParam contain handle to button window
				button = ThumbButtons->first;			// figure out which button was pressed
				while ( button ) 
					if ( button->hwnd == chWnd )
						{								// go to the section represented by button
						i = button->number;
						if ( FindSection( i, 0, section ) )
							GotoSection( i, section );
						button = NULL;
						}
					else button = button->next;
				SetFocus( appWnd );						// switch focus to main window
				}
			break;

		default:
			return( DefWindowProc(hWnd, msg, wParam, lParam) );
		}

	return 0L;
}

											// Windows procedure for array of thumbnails...

LRESULT APIENTRY ThumbsProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int p, i, min, max;
	HWND chWnd;
	HMENU hMenu;
	HCURSOR cur;
	DWORD status;
	RECT wrect;
	POINT mpoint;
	Button *button;
	char sectionfile[MAX_PATH];

	switch(msg)
		{
		case WM_DESTROY:
			if ( hThumbsThread ) AbortThumbs = true;		// signal thread to terminate if still active
			KillTimer( hWnd, THUMBS_TIMER );				// halt timer interrupts

			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// don't destroy windows until thread is done
			GetExitCodeThread( hThumbsThread, &status );
			while ( status == STILL_ACTIVE )				// wait for thread to finish aborting
				{
				Sleep(33);									// release control to thread for 33ms slice
				GetExitCodeThread( hThumbsThread, &status );// see if thread is done yet
				}
			SetCursor( cur );								// depending on section data, might be awhile

			if ( ThumbButtons ) delete ThumbButtons;		// delete button memory
			ThumbButtons = NULL;
			hMenu = GetMenu( appWnd );						// uncheck menu item
			CheckMenuItem( hMenu, CM_THUMBNAILS, MF_BYCOMMAND | MF_UNCHECKED );
			thumbnailWindow = NULL;							// clear global handle pointer
			break;
		case WM_CREATE:										// setup thumbnails
			CreateThumbButtons( hWnd, appInstance );
			ThumbsThreadBegin();							// background thread with render bitmaps
			SetTimer(hWnd, THUMBS_TIMER, 1000, NULL);		// setup timer for mouse detection
			hMenu = GetSystemMenu( hWnd, FALSE );			// allow Maximize on titlebar menu
			EnableMenuItem( hMenu, SC_MAXIMIZE, MF_ENABLED );
			// Windows will issue WM_SIZE immediately so can complete setup there
			break;

		case WM_HSCROLL:						// horizontal scroll bar movement of buttons
			i = 0; p = GetScrollPos( hWnd, SB_HORZ );
			GetScrollRange( hWnd, SB_HORZ, &min, &max );
			switch ( LOWORD(wParam) )
				{
				case SB_LINEDOWN: i = max/10;
					break;
				case SB_LINEUP: i = -max/10;
					break;
				case SB_PAGEDOWN: i = CurrSeries->thumbWidth;
					break;
				case SB_PAGEUP: i = -CurrSeries->thumbWidth;
					break;
				case SB_THUMBTRACK: i = HIWORD(wParam)-p;	// get movement of scroll tab
				}
			if ( (p+i) > max ) i = max-p;
			if ( (p+i) < 0 ) i = -p;
			p += i; xThumbsPos -= i;				// compute move, then move buttons
			GetClientRect( hWnd, &wrect );
			wrect.left += xThumbsPos; wrect.top += yThumbsPos;
			EnumChildWindows( hWnd, MoveThumbButtons, (LPARAM)&wrect );
			SetScrollPos( hWnd, SB_HORZ, p, TRUE );	// update thumb pos
			InvalidateRect( hWnd, NULL, TRUE );		// clear old button images
      		break;

		case WM_MOUSEWHEEL:						
			i = (int)wParam;					// mouse wheel movements are the same as page up/down
			if ( i > 0 ) i = -CurrSeries->thumbHeight;
			else i = CurrSeries->thumbHeight;
		case WM_VSCROLL:						// vertical scroll bar movement of buttons
			p = GetScrollPos( hWnd, SB_VERT );
			GetScrollRange( hWnd, SB_VERT, &min, &max );
			if ( msg == WM_VSCROLL )
			  switch ( LOWORD(wParam) )
				{
         		case SB_LINEDOWN: i = max/10;
					break;
         		case SB_LINEUP: i = -max/10;
					break;
				case SB_PAGEDOWN: i = CurrSeries->thumbHeight;
					break;
				case SB_PAGEUP: i = -CurrSeries->thumbHeight;
					break;
				case SB_THUMBTRACK: i = HIWORD(wParam)-p;	// get movement of scroll tab
					break;
				default: i = 0;
				}
			if ( (p+i) > max ) i = max-p;
			if ( (p+i) < 0 ) i = -p;
			p += i; yThumbsPos -= i;				// compute move and move buttons
			GetClientRect( hWnd, &wrect );
			wrect.left += xThumbsPos; wrect.top += yThumbsPos;
			EnumChildWindows( hWnd, MoveThumbButtons, (LPARAM)&wrect );
			InvalidateRect( hWnd, NULL, TRUE );		// clear old button images
			SetScrollPos( hWnd, SB_VERT, p, TRUE );	// update thumb pos
      		break;

		case WM_TIMER:								// display on title bar section # pointed to by mouse
			GetCursorPos(&mpoint);					// get mouse pt in screen coordinates
			chWnd = WindowFromPoint(mpoint);
			button = ThumbButtons->first;			// figure out which button is there
			while ( button ) 
				if ( button->hwnd == chWnd )
					{								// display section # represented by button
					sprintf(sectionfile,"Section %d",button->number);
					SetWindowText( hWnd, sectionfile );
					button = NULL;
					}
				else button = button->next;			// continue until found or end of list
			SetTimer( hWnd, THUMBS_TIMER, 305, NULL);	// decrease interval for monitoring mouse
      		break;

		case WM_KILLFOCUS:							// stop and start timer as window is inactivated
			KillTimer( hWnd, THUMBS_TIMER );
			SetWindowText( hWnd, "Thumbnails" );
			break;
		case WM_SETFOCUS:							// and reactivated
			SetTimer( hWnd, THUMBS_TIMER, 305, NULL);
      		break;

		case WM_SIZE:
			xThumbsPos = 0;	yThumbsPos = 0;		// start buttons in upper left corner of parentWnd
			xThumbsOver = 0; yThumbsOver = 0;	// if these stay zero then all fit in window
			GetClientRect( hWnd, &wrect );
			EnumChildWindows( hWnd, MoveThumbButtons, (LPARAM)&wrect );
			SetScrollRange( hWnd, SB_HORZ, 0, xThumbsOver, FALSE );
			SetScrollPos( hWnd, SB_HORZ, 0, TRUE );
			SetScrollRange( hWnd, SB_VERT, 0, yThumbsOver, FALSE );	
			SetScrollPos( hWnd, SB_VERT, 0, TRUE );
			InvalidateRect( hWnd, NULL, TRUE );	// repaint all button images when done
			break;

		case WM_KEYDOWN:						// allow control+TAB to switch active window
			if ( (wParam==VK_TAB) && (GetKeyState(VK_CONTROL) & 0x8000) ) CmNextWindow( thumbnailWindow );
			break;

		case WM_COMMAND:						// if user presses button...
			if ( (LOWORD(wParam) == BN_CLICKED) && (ThumbButtons != NULL) )
				{
				chWnd = (HWND)lParam;			// lParam contain handle to button window
				button = ThumbButtons->first;	// figure out which button was pressed
				while ( button ) 
					if ( button->hwnd == chWnd )
						{						// go to the section represented by button
						i = button->number;
						if ( FindSection( i, 0, sectionfile ) )
							GotoSection( i, sectionfile );
						button = NULL;
						}
					else button = button->next;
				SetFocus( appWnd );				// switch focus to main window
				}
			break;
		}

	return( DefWindowProc(hWnd, msg, wParam, lParam) );
}


HWND MakeThumbsWindow( void )				// size and create thumbnail view window
{
	HWND thumbsWnd;
	HMENU hMenu;
	WNDCLASS wndclass;
	RECT wrect;
	int x, y, w, h, hs;
													// recreate the thumbnail window class, might be different
	UnregisterClass("ReconstructThumbsClass",appInstance);
	wndclass.style = CS_OWNDC;
	if ( CurrSeries->useFlipbookStyle )				// flipbook style needs different actions to user input
		wndclass.lpfnWndProc = (WNDPROC)FlipbookProc;
	else wndclass.lpfnWndProc = (WNDPROC)ThumbsProc;
	wndclass.cbClsExtra  = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance = appInstance;
	wndclass.hCursor   = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon     = NULL;						// not used by WS_EX_TOOLWINDOW
	wndclass.lpszMenuName = NULL;
	wndclass.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wndclass.lpszClassName = "ReconstructThumbsClass";
	RegisterClass(&wndclass);
	
	ClientRectLessStatusBar( &wrect );				// determine window size
	ClientRectToScreen(appWnd, &wrect );
	hs = GetSystemMetrics(SM_CYHSCROLL);			// get scrollbar height in pixels
	h = wrect.bottom - wrect.top;
	x = wrect.right;								// put window at right of client window
	y = wrect.top;
	if ( CurrSeries->useFlipbookStyle )				// if flipbook, size window to one button
		{
		wrect.left = 0; wrect.right = CurrSeries->thumbWidth;
		wrect.top = 0; wrect.bottom = CurrSeries->thumbHeight + hs;	// allow for scrollbar, frame and titlebar
		AdjustWindowRectEx( &wrect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_TOOLWINDOW );
		w = wrect.right-wrect.left;
		h = wrect.bottom-wrect.top;
		}
	else
		{
		wrect.left = 0; wrect.right = CurrSeries->thumbWidth;
		wrect.top = 0; wrect.bottom = h;						// allow for scrollbar, frame and titlebar
		AdjustWindowRectEx( &wrect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_TOOLWINDOW );
		w = wrect.right-wrect.left;
		}		
	x = x - w;										// allow for final width of window
	
    thumbsWnd = CreateWindowEx(WS_EX_TOOLWINDOW, "ReconstructThumbsClass", "Thumbnails", WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
									x, y, w, h, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

	if ( thumbsWnd )			// if window created, display it and check corresponding menu item in main window
		{
		ShowWindow(thumbsWnd, SW_SHOW);
		hMenu = GetMenu( appWnd );
		CheckMenuItem( hMenu, CM_THUMBNAILS, MF_BYCOMMAND | MF_CHECKED );
		}
	return thumbsWnd;
}

