/////////////////////////////////////////////////////////////////////////
//	This file contains code for the floating tool bars and tool selection
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
// modified 11/01/04 by JCF (fiala@bu.edu)
// -+- change: Added CmWildfire() and wildfire cursor options.
// modified 2/03/05 by JCF (fiala@bu.edu)
// -+- change: Added CmScalpel() and scalpel cursor options.
// -+- change: Modified tool button responses to set window text of tools window
//             and removed WM_TIMER in Tools Window Proc for setting this text.
// modified 2/22/05 by JCF (fiala@bu.edu)
// -+- change: Modified scalpel cursor to become arrow on right mouse button press.
// modified 8/12/05 by JCF (fiala@bu.edu)
// -+- change: Added precision cursor option to Magnify Tool
// modified 10/20/05 by JCF (fiala@bu.edu)
// -+- change: Removed Rt Button Arrow cursor from Scalpel Tool
// modified 7/2/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added AddContour function to do the job after creation of EditContour
//             so as to guarantee manual and automatic wildfire will have the same behavior.
// modified 11/13/06 by JCF (fiala@bu.edu)
// -+- change: Eliminated compiler WARNINGS/ERRORS so would compile under VC++.
// // modified 11/15/06 by JCF (fiala@bu.edu)
// -+- change: Removed StoreTracingInfo(): active traces in back section are used instead.

#include "reconstruct.h"

BOOL CALLBACK MoveToolButton( HWND hWnd, LPARAM lParam )
{
	RECT *pos;
	char msg[80];

	pos = (RECT*)lParam;
	MoveWindow( hWnd, pos->left, pos->top, BUTTON_SIZE, BUTTON_SIZE, TRUE );
	pos->left += BUTTON_SPACE;
	if ( pos->left+(BUTTON_SPACE/2) > pos->right ) {
		pos->top += BUTTON_SPACE;
		pos->left = 0;
		if ( pos->top > pos->bottom ) return FALSE;
		}
	return TRUE;
}

void CreateToolButtons( HWND hWnd )
{
	Button *button;
	int i, x, y;

	ToolButtons = new Buttons();						// create global list for buttons
	x = 0; y = 0;

	for ( i=0; i<NUM_TOOLS; i++)						// for each tool ID (numbered 0 to NUM_TOOLS-1)
		{
		button = new Button();
		button->number = 2100+i;						// see constants.h for explanation of 2100!

		button->hwnd = CreateWindow( "BUTTON", NULL, WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE | BS_BITMAP,
											x, y, BUTTON_SIZE, BUTTON_SIZE, hWnd, NULL, appInstance, NULL );
												
		button->canvas = LoadBitmap(appInstance, MAKEINTRESOURCE(button->number));				// get bitmap from resources
		SendMessage( button->hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(button->canvas) );// and put on button
		x = x + BUTTON_SIZE;
		ToolButtons->Add( button );					// add button to button list
		}
}
											// Windows procedure for a doubleing toolbar...

LRESULT APIENTRY ToolsProc(	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	HWND chWnd;
	RECT wrect;
	POINT mpoint;
	Button *button;

	switch(msg)
		{
		case WM_CREATE:								// setup toolbar
			CreateToolButtons( hWnd );
			SetWindowText( hWnd, "Tools" );
			// WM_SIZE will now be issued by Windows so layout buttons there
			break;

		case WM_DESTROY:						// when window is destroyed...
			if ( ToolButtons ) delete ToolButtons;
			ToolButtons = NULL;					// destroy tool buttons
			hMenu = GetMenu( appWnd );			// uncheck menu item
			CheckMenuItem( hMenu, CM_TOOLBAR, MF_BYCOMMAND | MF_UNCHECKED );
			toolbarWindow = NULL;				// clear global handle pointer
			SetFocus(appWnd);					// return to main window
			break;

		case WM_KILLFOCUS:
			if ( highlightedToolButton ) highlightedToolButton->HighlightButton();
			highlightedToolButton = NULL;
			break;

		case WM_SIZE:
			GetClientRect( hWnd, &wrect );
			EnumChildWindows( hWnd, MoveToolButton, (LPARAM)&wrect );
			InvalidateRect( hWnd, NULL, TRUE );
			break;

		case WM_KEYDOWN:
			switch ( wParam )					// allow control+TAB to switch active window
				{
				case VK_TAB: 
					if ( GetKeyState(VK_CONTROL) & 0x8000 ) CmNextWindow( toolbarWindow );
					else {
						if ( highlightedToolButton )
							{
							highlightedToolButton->HighlightButton();		// clear highlight
							highlightedToolButton = highlightedToolButton->next;// goto next button
							}
						if ( !highlightedToolButton ) highlightedToolButton = ToolButtons->first;
						highlightedToolButton->HighlightButton();			// highlight it
						}
					break;
				case VK_RETURN:					// push the button
					if ( highlightedToolButton ) 
						PostMessage( highlightedToolButton->hwnd, BM_CLICK, 0, 0 );
					break;
				}
				
		case WM_COMMAND:						// relay button responses to parent window
			switch (LOWORD(wParam))
				{
				case BN_CLICKED:
					chWnd = (HWND)lParam;			// lParam contains handle to button window
					button = ToolButtons->first;	// figure out which button was pressed
					while ( button )				// and send command represented by button
						if ( button->hwnd == chWnd )
							{						
							PostMessage( appWnd, WM_COMMAND, button->number, NULL );
							button = NULL;
							}
						else button = button->next;
					SetFocus( appWnd );
					break;
				}
		}

	return( DefWindowProc(hWnd, msg, wParam, lParam) );
}


HWND MakeToolWindow( void )
{
	HWND toolWnd;
	HMENU hMenu;
	RECT r;
	int x, y, w, h;
	WNDCLASS wndclass;
											// create the toolbar window...
	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)ToolsProc;
	wndclass.cbClsExtra  = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance = appInstance;
	wndclass.hCursor   = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon     = NULL;	// LoadIcon(appInstance,"ASmallIcon"); // not used for WS_EX_TOOLWINDOW
	wndclass.lpszMenuName = NULL;
	wndclass.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wndclass.lpszClassName = "ReconstructToolsClass";
	RegisterClass(&wndclass);

	GetWindowRect(appWnd,&r);
	x =r.right - 120;				// put toolbar at top, near right edge
	y = r.top;
	r.left = 0; r.right = (NUM_TOOLS/2)*BUTTON_SPACE;	// size to exactly hold buttons
	r.top = 0; r.bottom = 2*BUTTON_SIZE;
	AdjustWindowRectEx( &r, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_TOOLWINDOW );
	w = r.right-r.left;
	x = x - w;
	h = r.bottom-r.top;
											// NOTE: There is no way in Win32 to create a minimizable window
											// without a minimize box and limited resize, so just forget it!

    toolWnd = CreateWindowEx( WS_EX_TOOLWINDOW, "ReconstructToolsClass", "Tools",
								WS_OVERLAPPEDWINDOW, x, y, w, h, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

//	Alternative "docking" style toolbar inside the client area... requires WS_CLIPCHILDREN in main window 
//  toolWnd = CreateWindow( "ReconstructToolsClass", NULL, WS_CHILD | WS_BORDER,
//								0, 0, r.right-r.left, BUTTON_SIZE+4, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

	if ( toolWnd )			// if window created, display it and check corresponding menu item in main window
		{
		ShowWindow(toolWnd, SW_SHOW);
		hMenu = GetMenu( appWnd );
		CheckMenuItem( hMenu, CM_TOOLBAR, MF_BYCOMMAND | MF_CHECKED );
		}
	return toolWnd;
}



void SetToolCursor( HWND hWnd )
{
	HCURSOR cur;
	POINT position, lefttop, rightbottom;
	RECT clientarea, winarea;
	int border;

	GetCursorPos(&position);						// in screen coordinates
	GetClientRect(hWnd, &clientarea);				// client rect is in client coordinates
	lefttop.x = clientarea.left;
	lefttop.y = clientarea.top;
	rightbottom.x = clientarea.right;
	rightbottom.y = clientarea.bottom;
	ClientToScreen( hWnd, &lefttop );				// convert client rect to screen coordinates
	ClientToScreen( hWnd, &rightbottom );
	clientarea.left = lefttop.x;
	clientarea.top = lefttop.y;
	clientarea.right = rightbottom.x;
	clientarea.bottom = rightbottom.y;
	GetWindowRect( hWnd, &winarea );				// get window rect in screen coordinates
	border = winarea.left - clientarea.left;		// determine (negative) width of resizing border
	InflateRect( &winarea, border, border );		// shrink winarea by this amount

	if ( PtInRect( &clientarea, position ) )
		{
		switch ( CurrentTool )
			{
			case ZOOM_TOOL:
				if ( RToolActive ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(ZOOMCUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(XYCUR) );
				break;
			case MAGNIFY_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(MAGNIFYCUR) );
				break;
			case DOMAIN_TOOL:
				cur = LoadCursor( appInstance, MAKEINTRESOURCE(DOMAINCUR) );
				break;
			case POINT_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = StampCursor;
				break;
			case ELLIPSE_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(ELLIPSECUR) );
				break;
			case RECTANGLE_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(RECTCUR) );
				break;
			case LINE_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(LINECUR) );
				break;
			case MULTILINE_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(MULTILINECUR) );
				break;
			case PENCIL_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(PENCILCUR) );
				break;
			case CULTILINE_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(CULTILINECUR) );
				break;
			case SCISSOR_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(SCISSORCUR) );
				break;
			case ZLINE_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(ZLINECUR) );
				break;
			case GRID_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(GRIDCUR) );
				break;
			case WILDFIRE_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				else if ( RToolActive ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(OUTFIRECUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(WILDFIRECUR) );
				break;
			case SCALPEL_TOOL:
				if ( UsePrecisionCursor ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(PRECISECUR) );
				//else if ( RToolActive ) cur = LoadCursor( 0, IDC_ARROW );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(SCALPELCUR) );
				break;
			default: cur = LoadCursor( 0, IDC_ARROW );
			}
		}
	else 
		if ( PtInRect( &winarea, position ) ) cur = LoadCursor( 0, IDC_ARROW );
		else cur = LoadCursor( 0, IDC_SIZEALL );

	SetCursor( cur );
}
														// routines for switching tools
void CmArrow( void )
{
	char txt[64];
	if ( CurrentTool != ARROW_TOOL )
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = ARROW_TOOL;
	LoadString( appInstance, TB_ARROW, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmZoom( void )
{
	char txt[64];
	if ( CurrentTool != ZOOM_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = ZOOM_TOOL;
	LoadString( appInstance, TB_ZOOM, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmMagnify( void )
{
	char txt[64];
	if ( CurrentTool != MAGNIFY_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = MAGNIFY_TOOL;
	LoadString( appInstance, TB_MAGNIFY, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmDomain( void )
{
	char txt[64];
	if ( CurrentTool != DOMAIN_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = DOMAIN_TOOL;
	LoadString( appInstance, TB_DOMAIN, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmPoint( void )
{
	char txt[64];
	if ( CurrentTool != POINT_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = POINT_TOOL;
	LoadString( appInstance, TB_POINT, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmEllipse( void )
{
	char txt[64];
	if ( CurrentTool != ELLIPSE_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = ELLIPSE_TOOL;
	LoadString( appInstance, TB_ELLIPSE, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmRectangle( void )
{
	char txt[64];
	if ( CurrentTool != RECTANGLE_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = RECTANGLE_TOOL;
	LoadString( appInstance, TB_RECTANGLE, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmLine( void )
{
	char txt[64];
	if ( CurrentTool != LINE_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = LINE_TOOL;
	LoadString( appInstance, TB_LINE, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmMultiLine( void )
{
	char txt[64];
	if ( CurrentTool != MULTILINE_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = MULTILINE_TOOL;
	LoadString( appInstance, TB_MULTILINE, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmPencil( void )
{
	char txt[64];
	if ( CurrentTool != PENCIL_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = PENCIL_TOOL;
	LoadString( appInstance, TB_PENCIL, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmCultiLine( void )
{
	char txt[64];
	if ( CurrentTool != CULTILINE_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = CULTILINE_TOOL;
	LoadString( appInstance, TB_CULTILINE, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmScissor( void )
{
	char txt[64];
	if ( CurrentTool != SCISSOR_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = SCISSOR_TOOL;
	LoadString( appInstance, TB_SCISSOR, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmZLine( void )
{
	char txt[64];
	if ( CurrentTool != ZLINE_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = ZLINE_TOOL;
	LoadString( appInstance, TB_ZLINE, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmGrid( void )
{
	char txt[64];
	if ( CurrentTool != GRID_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = GRID_TOOL;
	LoadString( appInstance, TB_GRID, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmWildfire( void )
{
	char txt[64];
	if ( CurrentTool != WILDFIRE_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = WILDFIRE_TOOL;
	LoadString( appInstance, TB_WILDFIRE, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}

void CmScalpel( void )
{
	char txt[64];
	if ( CurrentTool != SCALPEL_TOOL ) 
		{
		CmEscapeCurrentTool();
		UsePrecisionCursor = false;
		}
	CurrentTool = SCALPEL_TOOL;
	LoadString( appInstance, TB_SCALPEL, txt, 64 );
	SetWindowText( toolbarWindow, txt );
}


void SwitchTool( int toolId )		// this assumes CurrentTool has already been escaped as needed
{
	Button *button;

	CurrentTool = toolId;						// switch tool id
	SetToolCursor( appWnd );					// and cursor
	if ( IsWindow( toolbarWindow ) )
		{
		toolId += 2100;							// see constants.h for explanation of 2100!
		button = ToolButtons->first;			// press appropriate button on toolbar
		while ( button )
			if ( button->number == toolId )
				{						
				PostMessage( button->hwnd, BM_CLICK, 0, 0 );
				button = NULL;
				}
			else button = button->next;
		}
}



HCURSOR MakeStampCursor( Contour *contour )	// Convert pixel offset contour into a cursor for stamp tool
{
	Contour *c;										// cursor is 32x32 with hot spot at 16,16
	Point *p, min, max;								// each row of bitmap is 32 bits => 4 bytes
	double fx, fy;									// AND XOR Result
	int hotx, hoty, i, j, k;						//  0   0  Black 
	HCURSOR cur;									//  0   1  White
	BYTE b;											//  1   0  Transparent
													//  1   1  Reverse
	BYTE ANDbitmap[128] = { 0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFF, 0xFF, 0xFF,
							0xFF, 0xFE, 0x7F, 0xFF,
							0xFF, 0xFE, 0x3F, 0xFF,
							0xFF, 0xFE, 0x1F, 0xFF,
							0xFF, 0xFE, 0x0F, 0xFF,
							0xFF, 0xFE, 0x07, 0xFF,
							0xFF, 0xFE, 0x03, 0xFF,
							0xFF, 0xFE, 0x01, 0xFF,
							0xFF, 0xFE, 0x00, 0xFF,
							0xFF, 0xFE, 0x00, 0x7F,
							0xFF, 0xFE, 0x00, 0x3F,
							0xFF, 0xFE, 0x00, 0xFF,
							0xFF, 0xFE, 0x00, 0xFF,
							0xFF, 0xFE, 0x00, 0xFF,
							0xFF, 0xFE, 0x40, 0x7F,
							0xFF, 0xFE, 0xE0, 0x7F,
							0xFF, 0xFF, 0xE0, 0x7F,
							0xFF, 0xFF, 0xF0, 0xFF };
	BYTE XORbitmap[128] = { 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x80, 0x00,
							0x00, 0x00, 0xC0, 0x00,
							0x00, 0x00, 0xE0, 0x00,
							0x00, 0x00, 0xF0, 0x00,
							0x00, 0x00, 0xF8, 0x00,
							0x00, 0x00, 0xFC, 0x00,
							0x00, 0x00, 0xFE, 0x00,
							0x00, 0x00, 0xF8, 0x00,
							0x00, 0x00, 0xDC, 0x00,
							0x00, 0x00, 0xAC, 0x00,
							0x00, 0x00, 0x0E, 0x00,
							0x00, 0x00, 0x0E, 0x00,
							0x00, 0x00, 0x07, 0x00,
							0x00, 0x00, 0x06, 0x00,
							0x00, 0x00, 0x00, 0x00 };

	hotx = 15; hoty = 15;

	c = new Contour( *contour );
	c->YInvert( 0.0 );			// flip y coord. offsets
	c->Extent( &min, &max );	// how big is it?
	fx = max.x - min.x;
	fy = max.y - min.y;			// if it's bigger than the cursor
	if ( fy > fx ) fx = fy;		// squeeze it down into cursor size
	if ( fx > 30.0 ) c->Scale( 30.0/fx );
	c->Shift( hotx, hoty );		// center on hot spot
	c->ChainCode( 1.0 );			// rasterize contour

	p = c->points->first;		// now draw contour onto cursor bitmap
	while ( p )
		{
		i = (int)floor(p->x);
		j = (int)floor(p->y);
		k = j*4 + i/8;
		b = 0x80>>(i%8);
		ANDbitmap[k] |= b;		/// set pixel to reverse
		XORbitmap[k] |= b;
		p = p->next;
		}
	
	cur = CreateCursor( appInstance, hotx, hoty, 32, 32, ANDbitmap, XORbitmap );
	return( cur );
}

// Ju: don't put automatic tracing code here
 