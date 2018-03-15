/////////////////////////////////////////////////////////////////////////
//	This file contains code for the palette toolbar and palette selection
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
// 
// modified 2/16/05 by JCF (fiala@bu.edu)
// -+- change: Modified palette window title text to retain state after button press.
// modified 9/13/05 by JCF (fiala@bu.edu)
// -+- change: Added UnPushPaletteButtons() to remove all button presses.
// modified 5/18/06 by JCF (fiala@bu.edu)
// -+- change: Removed unmodifiable PointContours from palette.
//

#include "reconstruct.h"

Contour * IndexedContour( int index )				// return contour that has index in defaults
{
	Contour *contour;
	int i;

	contour = NULL;
	i = 0;
	if ( CurrSeries )
	  if ( CurrSeries->contours )					// search in CurrSeries default contours
		{											// find contour in index position in list
		contour = CurrSeries->contours->first;
		while ( contour )
			{
			if ( i == index ) break;				// found it!
			i++;
			contour = contour->next;
			}
		}

	return( contour );									// return ptr or NULL if not found
}

void PushPaletteButton( int index )			// push the indexed palette contour button
{
	Contour *contour;
	Button *button;
	int i;

	if ( PaletteButtons )
	  {
	  i = 0;
	  button = PaletteButtons->first;		// push the indexed one, and unpush all others
	  while ( button )
		{
		if ( i == index )					// if index, then depress button and update window text
			{
			SendMessage( button->hwnd, BM_SETCHECK, (WPARAM)BST_CHECKED, 0 );
			contour = IndexedContour( index );
			SetWindowText( paletteWindow, contour->name );
			}
		else SendMessage( button->hwnd, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0 );
		button = button->next;
		i++;
		}
	}
}

void UnPushPaletteButtons( void )			// unpush all palette contour button
{
	Button *button;

	if ( PaletteButtons )
	  {
	  button = PaletteButtons->first;		// window handles stored in button list
	  while ( button )
		{
		SendMessage( button->hwnd, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0 );
		button = button->next;
		}
	  }
}

void UpdatePaletteButton( int index )				// update the image of the palette contour button
{
	HBITMAP canvas;
	Button *button;
	Contour *contour;
	int i;

	if ( PaletteButtons )
	  {
	  i = 0;
	  button = PaletteButtons->first;		// figure out which button has the default contour
	  contour = CurrSeries->contours->first;
	  while ( contour )
		if ( i == index )							// find contour + button with index
			{
			canvas = button->canvas;							// update the palette image
			button->canvas = MakeContourBitmap( button->hwnd, contour );
			SendMessage( button->hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(button->canvas) );
			DeleteObject( canvas );			
			contour = NULL;
			}
		else {
			button = button->next;
			contour = contour->next;
			i++;
			}
		}
}

BOOL CALLBACK MovePaletteButton( HWND hWnd, LPARAM lParam )
{
	RECT *pos;
	char msg[80];

	pos = (RECT*)lParam;
	MoveWindow( hWnd, pos->left, pos->top, ButtonSize, ButtonSize, TRUE );
	pos->left += ButtonSize+1;
	if ( pos->left+((ButtonSize+1)/2) > pos->right ) {
		pos->top += ButtonSize+1;
		pos->left = 0;
		if ( pos->top > pos->bottom ) return FALSE;
		}
	return TRUE;
}

HBITMAP MakeContourBitmap( HWND hWnd, Contour *contour )	// Convert pixel offset contour into a bitmap for button
{
	Contour *c;
	Point *p, min, max;
	double fx, fy, size;
	int i, numpts;
	POINT *lpPoints;
	HPEN pen;
	HBRUSH brush;
	HBITMAP canvas, stipple;
	RECT wrect;
	HDC winDC, canvasDC;

	winDC = GetDC( hWnd );							// create bitmap for contour image					
	canvas = CreateCompatibleBitmap( winDC, ButtonSize, ButtonSize );
	canvasDC = CreateCompatibleDC( winDC );			// create DC for drawing
	SelectObject( canvasDC, canvas );				// select bitmap in DC for drawing
	ReleaseDC( hWnd, winDC );						// done with display DC for now

	GetClientRect( hWnd, &wrect );					// paint background
	stipple = LoadBitmap(appInstance, "StippleBitmap");
	brush = CreatePatternBrush( stipple );
	FillRect( canvasDC, &wrect, brush );
	DeleteObject( brush );
	DeleteObject( stipple );
	
	c = new Contour( *contour );
	c->YInvert( 0.0 );								// flip y coord. offsets
	c->Extent( &min, &max );						// how big is it?
	fx = max.x - min.x;
	fy = max.y - min.y;								// if it's bigger than the bitmap
	if ( fy > fx ) fx = fy;							// squeeze it down into bitmap size
	size = ButtonSize - 4;
	if ( fx > size ) c->Scale( (double)size/(double)fx );
	c->Shift( ButtonSize/2, ButtonSize/2 );	// center on bitmap

	numpts = c->points->Number();
	lpPoints = new POINT[ numpts ];					// create Window POINT array for drawing
	i = 0;
	p = c->points->first;							// translate shrunken contour into POINTS
	while ( p != NULL )
		{
		lpPoints[i].x = (int)floor(p->x);
		lpPoints[i].y = (int)floor(p->y);
		i++;
		p = p->next;
		}
													// create pen for border of object
	pen = CreatePen( PS_SOLID, 1, c->border.ref() );
	SelectObject( canvasDC, pen );					// set pen into device context
	brush = CreateSolidBrush( c->fill.ref() );		// interior will be filled
	SelectObject( canvasDC, brush );
	SetROP2( canvasDC, abs(c->mode) );				// using contour fill mode and color
	Polygon( canvasDC, lpPoints, numpts );

	SelectObject( canvasDC, (HBRUSH)GetStockObject(NULL_BRUSH) );
	DeleteObject(brush);							// clean up fill brush

	SetROP2( canvasDC, R2_COPYPEN );				// draw contour border with pen only
	Polygon( canvasDC, lpPoints, numpts );

	DeleteObject(pen);								// clean up pen
	delete[] lpPoints;								// and dynamic memory
	DeleteDC( canvasDC );

	return( canvas );
}

void CreatePaletteButtons( HWND hWnd, int x, int y, Contours *contours, Buttons *buttons )
{
	Button *button;
	Contour *contour;
	int i, xx, yy;
	
	xx = x; yy = y;
	i = buttons->Number();			// this procedure is also used in options dialog!
	
	contour = contours->first;			// create a list of contour buttons from contours
	while ( contour )
		{
		button = new Button();				// create new button
		buttons->Add( button );				// add button to button list

		button->number = i;					// remember index of buttons

		button->hwnd = CreateWindow( "BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE | BS_BITMAP,
											xx, yy, ButtonSize, ButtonSize, hWnd, NULL, appInstance, NULL );

		button->canvas = MakeContourBitmap( button->hwnd, contour );								// draw contour on bitmap
		SendMessage( button->hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(button->canvas) );	// and put bitmap on button
		i++;
		if ( i > 20 ) return;				// cap number of palette entries at 20
		xx = xx + ButtonSize;				// do next contour button
		if ( xx > (9*ButtonSize + x) )
			{								// after 10 buttons in a row, skip to next row
			xx = x;
			yy = yy + ButtonSize;
			}
		contour = contour->next;
		}
}
											// Windows procedure for a doubleing toolbar...

LRESULT APIENTRY PaletteProc(	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	HWND chWnd;
	Contour *contour;
	int i;
	HCURSOR cur;
	RECT wrect;
	POINT mpoint;
	Button *button;
	char txt[MAX_CONTOUR_NAME];

	switch(msg)
		{
		case WM_CREATE:								// setup buttons
			PaletteButtons = new Buttons();
			CreatePaletteButtons( hWnd, 0, 0, CurrSeries->contours, PaletteButtons );
			SetWindowText( hWnd, "Trace Palette" );			
			// WM_SIZE will now be issued by Windows so layout buttons there
			break;

		case WM_DESTROY:						// when window is destroyed...
			if ( PaletteButtons ) delete PaletteButtons;
			PaletteButtons = NULL;				// destroy buttons
			hMenu = GetMenu( appWnd );			// uncheck menu item
			CheckMenuItem( hMenu, CM_TRACEPALETTE, MF_BYCOMMAND | MF_UNCHECKED );
			paletteWindow = NULL;				// clear global handle pointer
			SetFocus(appWnd);					// return to main window
			break;

		case WM_KILLFOCUS:							// remove highlight
			if ( highlightedPaletteButton ) highlightedPaletteButton->HighlightButton();
			highlightedPaletteButton = NULL;
			break;

		case WM_SIZE:
			GetClientRect( hWnd, &wrect );
			EnumChildWindows( hWnd, MovePaletteButton, (LPARAM)&wrect );
			InvalidateRect( hWnd, NULL, TRUE );
			break;

		case WM_KEYDOWN:
			switch ( wParam )					// allow control+TAB to switch active window
				{
				case VK_TAB: 
					if ( GetKeyState(VK_CONTROL) & 0x8000 ) CmNextWindow( paletteWindow );
					else {
						if ( highlightedPaletteButton )		// TAB goes through window button
							{
							highlightedPaletteButton->HighlightButton();	// clear highlight
							highlightedPaletteButton = highlightedPaletteButton->next;
							}
						if ( !highlightedPaletteButton ) highlightedPaletteButton = PaletteButtons->first;
						highlightedPaletteButton->HighlightButton();
						}
					break;

				case VK_RETURN:					// push the button
					if ( highlightedPaletteButton ) 
						PostMessage( highlightedPaletteButton->hwnd, BM_CLICK, 0, 0 );
					break;
				}
				
		case WM_COMMAND:						// relay button responses to parent window
			switch (LOWORD(wParam))
				{
				case BN_CLICKED:
					chWnd = (HWND)lParam;				// lParam contains handle to button window
					button = PaletteButtons->first;	// figure out which button was pressed
					while ( button )					// and send command represented by button
						if ( button->hwnd == chWnd )
							{					
							CmUseDefaultContour( button->number );
							button = NULL;
							}
						else button = button->next;
					SetFocus( appWnd );
					break;
				}

		}

	return( DefWindowProc(hWnd, msg, wParam, lParam) );
}


HWND MakePaletteWindow( void )
{
	HWND palWnd;
	HMENU hMenu;
	RECT r;
	int x, y, w, h;
	WNDCLASS wndclass;
											// create the toolbar window...
	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)PaletteProc;
	wndclass.cbClsExtra  = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance = appInstance;
	wndclass.hCursor   = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon     = NULL;	// LoadIcon(appInstance,"ASmallIcon"); // not used for WS_EX_TOOLWINDOW
	wndclass.lpszMenuName = NULL;
	wndclass.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wndclass.lpszClassName = "ReconstructPaletteClass";
	RegisterClass(&wndclass);

	GetWindowRect(appWnd,&r);
	x = r.left + (r.right-r.left)/3;				// put partway along window top
	y = r.top;
	r.left = 0; r.right = 10*(ButtonSize+1);		// size to exactly hold 20 buttons
	r.top = 0; r.bottom = 2*(ButtonSize+1);
	AdjustWindowRectEx( &r, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_TOOLWINDOW );
	w = r.right-r.left;
	h = r.bottom-r.top;
											// NOTE: There is no way in Win32 to create a minimizable window
											// without a minimize box and limited resize, so just forget it!

    palWnd = CreateWindowEx( WS_EX_TOOLWINDOW, "ReconstructPaletteClass", "Trace Palette",
								WS_OVERLAPPEDWINDOW, x, y, w, h, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

	if ( palWnd )			// if window created, display it and check corresponding menu item in main window
		{
		ShowWindow(palWnd, SW_SHOW);
		hMenu = GetMenu( appWnd );
		CheckMenuItem( hMenu, CM_TRACEPALETTE, MF_BYCOMMAND | MF_CHECKED );
		}
	return palWnd;
}


void CmUseDefaultContour( int index )					// switch default to indexed default
{
	Contour *contour;
	HCURSOR cur;

	contour = IndexedContour( index );
	if ( contour )										// have a default contour for index
		{
		if ( StampContour ) delete StampContour;			// delete existing point contour
		StampContour = new Contour( *contour );				// make copy of default to use as point contour
		SetCursor( LoadCursor( 0, IDC_WAIT ) );				// in case StampCursor is in use
		cur = StampCursor;									// remember handle for destroy
		StampCursor = MakeStampCursor( StampContour );		// create new cursor from contour
		DestroyCursor( cur );								// destroy old cursor
		SetToolCursor( appWnd );							// restore original cursor
		CurrSeries->defaultBorder = StampContour->border;	// set default attributes for all tools
		CurrSeries->defaultFill = StampContour->fill;
		CurrSeries->defaultMode = StampContour->mode;
		strcpy(CurrSeries->defaultName,StampContour->name);
		strcpy(CurrSeries->defaultComment,StampContour->comment);
		PushPaletteButton( index );
		}
}


void CmSetDefaultContour( int index )					// set contour[index] default attributes
{
	Point *p, *q;
	Contour *contour;

	contour = IndexedContour( index );
	if ( contour )									// have a default contour for index
		{
		if ( StampContour )							// replace point array with that of stamp contour
			{
			if ( contour->points ) delete contour->points;
			contour->points = new Points();
			p = StampContour->points->first;
			while ( p )
				{
				q = new Point( *p );
				contour->points->Add(q);
				p = p->next;
				}
			}										// set default attributes to this default contour
		contour->border = CurrSeries->defaultBorder;	
		contour->fill = CurrSeries->defaultFill;
		contour->mode = CurrSeries->defaultMode;
		strcpy(contour->name,CurrSeries->defaultName);
		UpdatePaletteButton( index );
		PushPaletteButton( index );
		}
}
