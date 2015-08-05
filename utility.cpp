////////////////////////////////////////////////////////////////////////
//	This file contains misc. utilities used in other components of the
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
// modified 4/27/05 by JCF (fiala@bu.edu)
// -+- change: Fixed bad contour name when editing with multiline tools.
// modified 6/23/05 by JCF (fiala@bu.edu)
// -+- change: Modified UpdateMenus to reflect changes in Movement menu.
// modified 6/29/05 by JCF (fiala@bu.edu)
// -+- change: Added WILDFIRE_TOOL left mouse drag to UpdateStatusBar().
// modified 11/9/05 by JCF (fiala@bu.edu)
// -+- change: Fixed Fatal error in UpdateStatusBar with pass pencil over bottom edge of window.
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Modified SetDefaultName params.
// modified 4/26/06 by JCF (fiala@bu.edu)
// -+- change: Added DisplayListInfo utility.
// modified 5/4/06 by JCF (fiala@bu.edu)
// -+- change: Added CM_SMOOTHSELECTED to trace menu enable/disable.
// modified 5/24/06 by JCF (fiala@bu.edu)
// -+- change: Moved SectionRangeDlgProc() here from section_menu.cpp becuz used in multiple places now.
// modified 6/26/06 by JCF (fiala@bu.edu)
// -+- change: Cleaned up some comments in ErrMsgSplash().
// modified 6/28/06 by JCF (fiala@bu.edu)
// -+- change: Added trace comment to status bar display.
// modified 11/22/06 by JCF (fiala@bu.edu)
// -+- change: Fixed ErrMsgSplash() to return focus to appWnd after timeout.
// -+- change: Added MakeAbsolutePath() to combine relative and absolute directory references to make one path.
// modified 4/3/07 by JCF (fiala@bu.edu)
// -+- change: Modified TestGBMfile to also return the number of images in a GIF or TIFF image stack
// modified 4/30/07 by JCF (fiala@bu.edu)
// -+- change: Modified UpdateStatusBar to report HSB for Wildfire Tool. Brought RGBtoHSB routine into this file.
// modified 7/18/07 by JCF (fiala@bu.edu)
// -+- change: Moved RGB2HSB back into ADib and replaced use in UpdateStatusBar with new GetHSBPixel().
//

#include "reconstruct.h"

void CenterDialog( HWND hwndOwner, HWND hwndDlg )	// position of child relative to parent's client area!
{
	RECT rcOwner, rcDlg;
	int ow, oh, dw, dh, x, y;
        											// if no owner window, center on desktop
	if ( hwndOwner == NULL )
		hwndOwner = GetDesktopWindow();

	GetWindowRect(hwndOwner, &rcOwner);				// how big is main window?
	ow = rcOwner.right - rcOwner.left;
	oh = rcOwner.bottom - rcOwner.top;
	GetWindowRect(hwndDlg, &rcDlg);					// how big is dialog?
	dw = rcDlg.right - rcDlg.left;
	dh = rcDlg.bottom - rcDlg.top;
	x = y = 0;										// to center use half the difference in size
	if ( ow > dw ) x = (ow - dw)/2;
	if ( oh > dh ) y = (oh - dh)/2;
	x += rcOwner.left;								// offset from upper left corner of owner
	y += rcOwner.top;
	SetWindowPos(hwndDlg, HWND_TOP, x, y, 0, 0, SWP_NOSIZE); // move dialog, but don't change size     		
}

void SizeViewPorts( void )							// on window resize event, adjust viewport bitmap sizes
{
	RECT wrect;
	int w, h, oh;

	GetClientRect( appWnd, &wrect );
	w = wrect.right - wrect.left;
	h = wrect.bottom - wrect.top;
	if ( PrevView ) PrevView->Resize( w, h );
	if ( CurrView )
		{
		oh = CurrView->height;									// use old height of viewport to determine scaling
		CurrView->Resize( w, h );
		}
	if ( DomainView ) DomainView->Resize( w, h );
	if ( BlendView ) BlendView->Resize( w, h );
	if ( CurrSeries && CurrView )
		CurrSeries->pixel_size = oh*CurrSeries->pixel_size/h;	// scale pixel_size to view same relative to window
		
	InvalidateRect(appWnd,NULL,FALSE);
}


void ErrMsgOK( int msgno, const char *msgtxt )		// a Win32 error message routine that
{													// is callable from non-windows function
	char errmsgtxt[80+MAX_PATH];					// allow 78 for msgno and MAX_PATH for msgtxt

	LoadString( appInstance, msgno, errmsgtxt, 80 );
	if ( msgtxt )
		{
		strcat(errmsgtxt,"\n\n");
		strcat(errmsgtxt,msgtxt);
		}
	MessageBox( appWnd, errmsgtxt, "Reconstruct", MB_OK | MB_ICONEXCLAMATION );
}

BOOL CALLBACK WaitUntilOK(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	switch (message)									// callback procedure for dialog just waits...
		{												// (e.g. the License dialog)
		case WM_INITDIALOG:
      		SetFocus( GetDlgItem(hwndDlg, IDOK) );
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
				{										// ...for OK button, then ends
				case IDOK:
					DestroyWindow(hwndDlg);
					return TRUE;
				}
		}
	return FALSE;
}

BOOL CALLBACK WaitTimeout(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{							// ASSUMES ONLY CALLED BY A SINGLE THREAD=> FIX MULTITHREADING OF ERROR MSGS!
	switch (message)		// callback procedure for dialog just waits for a few seconds
		{
		case WM_INITDIALOG:
			SetTimer(hwndDlg, SPLASH_TIMER, lParam, NULL);	// start timer for lParam milliseconds
			return TRUE;
		case WM_DESTROY:
			SetFocus(appWnd);								// return to main window
			return FALSE;
		case WM_CLOSE:
		case WM_TIMER:
			KillTimer(hwndDlg, SPLASH_TIMER );				// timeout, end dialog
			DestroyWindow(hwndDlg);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case IDOK: DestroyWindow(hwndDlg);
					return TRUE;
				}
		}
	return FALSE;
}

void ErrMsgSplash( int msgno, const char *msgtxt )	// display error message transiently
{
	char errmsgtxt[80];	
	HWND hwndDlg;

	LoadString( appInstance, msgno, errmsgtxt, 80 );	// s = LoadString for length of string
	hwndDlg = CreateDialogParam( appInstance, "Splash", appWnd, (DLGPROC)WaitTimeout, 5000  );
	
	SendDlgItemMessage(hwndDlg, ID_ERRMSG, WM_SETTEXT, 0, (LPARAM)errmsgtxt);
	SendDlgItemMessage(hwndDlg, ID_MSGTXT, WM_SETTEXT, 0, (LPARAM)msgtxt);
}


void * GlobalAllocOrDie( DWORD num_bytes )		// allocate memory from heap, if fail exit
{
	EXCEPTION e;								// NOTE: Borland malloc uses Win32 GlobalAlloc
	void *ptr;									// but has heap manager that will suballocate
	ptr = malloc((size_t)num_bytes);			// smaller blocks and save memory
	if ( ptr == NULL ) {
		MessageBox(0,"Insufficient memory to complete operation.","Error", MB_OK | MB_ICONSTOP );
		throw e;
		}										// throw exception to allow program to exit gracefully
	return ptr;
}

bool CanOverwrite( char *filename )				// see if file exists and whether can overwrite
{
	WIN32_FIND_DATA	f;
	HANDLE	search;
	char msgtxt[MAX_PATH+100];

	search = FindFirstFile( filename, &f );				// find existing file
	if ( search == INVALID_HANDLE_VALUE ) return true;	// none, therefore allow write
	else {												// if file is read only, fail
		FindClose( search );
		if ( f.dwFileAttributes & FILE_ATTRIBUTE_READONLY ) return false;
		else {
			sprintf(msgtxt,"%s\nexists. Over write file?",filename);
			if ( MessageBox(appWnd,msgtxt,"Warning", MB_YESNO ) == IDYES ) return true;
			else return false;
			}
		}
}
		
void SplitPath( char *src, char *dir, char *file, char *ext )
{															// take src="D:\myfiles\myseries\aSeries.ser"
	int j, k;												// and return dir="D:\myfiles\myseries\"
															// file="aSeries" and ext=".ser"
	char txt[MAX_PATH];

	k = strlen(src);
	while ( k >= 0 )								// find last directory separator in src 
		if ( (src[k] == '\\') || (src[k] == ':') ) break;
		else k--;

	j = strlen(src);
	while ( j >= 0 )								// find last period in src 
		if ( src[j] == '.' ) break;
		else j--;

	ext[0] = '\0';									// clear the output string
	if ( j > k ) strcpy(ext,src+j);					// there is an extension, so copy it
	k++;											// point to first char in file string
	if ( k > 0 ) strncpy(dir,src,k);				// dir is preceding part if present
	dir[k] = '\0';									// make sure dir is null terminated
	if ( j < 0 ) j = strlen(src);					// no extension, so use whole string
	j = j-k;
	if ( j > 0 ) strncpy(file,src+k,j);				// stuff in middle is file
	else j = 0;
	file[j] = '\0';									// null terminate
}
											
void MakeLocalPath( char *wpath, char *ipath, char *localpath )
{														// take ipath and make it local relative to wpath
	int subdirs, k;										// e.g. wpath="D:\dir\", ipath="D:\images\file.bmp"
	char *w, *i, *f;									// then localpath="..\images\file.bmp"
	
	w = wpath;
	i = ipath;
	if ( *i != *w ) strcpy( localpath, ipath );			// if drive is different, use full path
	else												// otherwise look for difference in paths
		{
		k = strlen(ipath);
		while ( k > 0 )									// find last path indicator in ipath 
			{
			if ( ipath[k-1] == '\\' ) break;
			k--;
			}
		f = ipath + k;
		while ( (*i == *w) && (i != f) && (*w != '\0') )// search for character mismatch in paths
			{ w++; i++; }
		subdirs = 0;
		while (*w != '\0')								// from mismatch, find number of '\\'
			{
			if ( *w == '\\' ) subdirs++;
			w++;
			}
		*localpath = '\0';								// start with empty string
		while ( subdirs )								// add directory recursions
			{
			subdirs--;
			strcat(localpath,"..\\");
			}
		strcat(localpath,i);							// add relative path to file
		}
	// ASSUMES: Any network paths are from same machine!
}

void MakeAbsolutePath( char *adir, char *rdir, char *fulldir )  // NOTE: adir cannot include a filename!
{														// combine relative or absolute path rdir with an absolute directory
	int subdirs, r, a;									// e.g. rdir="..\mydir\image.bmp" and adir="C:\ourdir\yourdir\"
	char *f;											// then fulldir = "C:\ourdir\mydir\image.bmp"
	bool skipit;
	f = fulldir;										// form output string in reverse
	r = strlen(rdir) - 1;
	a = strlen(adir) - 1;								// start at back of input strings
	if ( rdir[1] == ':' ) subdirs = 0;					// have absolute path
	else subdirs = 1;									// number of subdirs to backup + 1
	while ( r >= 0 ) 
		{												// move subdir to fulldir string
		if ( rdir[r] != '\\' ) *f++ = rdir[r--];	
		else {											// hit new subdir delimiter
			skipit = false;
			if ( r > 1 )								// check for ..
				if ( (rdir[r-1] == '.') && (rdir[r-2] == '.') )
					skipit = true;						// if found skip over it
			if ( skipit ) 								// skip .. and don't copy backslash
				{ subdirs++; r = r - 3; }
			else *f++ = rdir[r--];						// otherwise copy backslash and continue
			}
		}
	if ( subdirs )										// rdir was relative not absolute
		{
		subdirs--;											// will need to copy rest of adir
		while ( subdirs )									// peel off subdirs of adir
			{
			a--;											// skip over last backslash
			while ( (a>=0) && (adir[a] != '\\') ) a--;		// skip over subdir
			subdirs--;										// do next subdir
			}
		while ( a >= 0 ) *f++ = adir[a--];				// copy rest of adir to complete path
		}
	*f = '\0';
	strrev(fulldir);									// reverse string to correct order	
}
	
												
void UpdateTitles( void )					// update the caption bar
{
	char titletxt[MAX_BASE+32], txt[32];

	if ( CurrSeries ) sprintf(titletxt,"%s",BaseName);				// put series name first
	else LoadString( appInstance, MSG_TITLE, titletxt, MAX_BASE );	// or program title

	if ( FrontView )												// if a section is visible...
		if ( FrontView->section )
			{
			sprintf(txt,": %d",FrontView->section->index);	// display section number
			strcat(titletxt,txt);
			}

	SetWindowText( appWnd, titletxt );								// display it
}

void UpdateMenus( void )						// set the menus according to the front section
{
	char	menutxt[32];
	UINT	state, alignmenu, mvmtmenu;
	HMENU	hMenu, hTraceMenu, hMvmtMenu;		

	alignmenu = 15;								// position of subsubmenu in Trace submenu
	mvmtmenu = 12;								// CHANGE when add/remove items from menu

	if ( FrontView )
	  if ( FrontView->section )
		{
		hMenu = GetMenu( appWnd );				// get menu handle to sub Menus
		hTraceMenu = GetSubMenu( hMenu, 4 );	// CHANGE these numbers when add new menus
		hMvmtMenu = GetSubMenu( GetSubMenu( hMenu, 2 ), mvmtmenu );

		if ( LastAdjustment ) EnableMenuItem( hMenu, CM_PROPAGATEADJUST, MF_ENABLED );
		else EnableMenuItem( hMenu, CM_PROPAGATEADJUST, MF_GRAYED );

		if ( FrontView->section->active || !FrontView->section->alignLocked )	// mvmts are possible
			{
			EnableMenuItem( hMvmtMenu, 1, MF_BYPOSITION | MF_ENABLED );			// allow flips
			EnableMenuItem( hMvmtMenu, 2, MF_BYPOSITION | MF_ENABLED );			// allow rotates
			EnableMenuItem( hMvmtMenu, 3, MF_BYPOSITION | MF_ENABLED );			// allow type in
			if ( LastAdjustment ) EnableMenuItem( hMenu, CM_REPEATADJUSTMENT, MF_ENABLED );
			else EnableMenuItem( hMenu, CM_REPEATADJUSTMENT, MF_GRAYED );
			}
		else {																// there is nothing to move
			EnableMenuItem( hMvmtMenu, 1, MF_BYPOSITION | MF_GRAYED );			// disable flips
			EnableMenuItem( hMvmtMenu, 2, MF_BYPOSITION | MF_GRAYED );			// disable rotates
			EnableMenuItem( hMvmtMenu, 3, MF_BYPOSITION | MF_GRAYED );			// disable type in
			EnableMenuItem( hMenu, CM_REPEATADJUSTMENT, MF_GRAYED );
			}

		if ( FrontView->section->alignLocked )					// setup alignment lock menu items
			{
			EnableMenuItem( hMenu, CM_BYCORRELATION, MF_GRAYED );
			strcpy(menutxt,"&Unlock\tCtrl+L");					// switch lock/unlock menu text
			ModifyMenu( hMenu, CM_LOCKSECTION, MF_BYCOMMAND | MF_STRING | MF_ENABLED, CM_LOCKSECTION, menutxt );
			strcpy(menutxt,"Align &traces");					// switch Trace->Align menu text
			ModifyMenu( hTraceMenu, alignmenu, MF_BYPOSITION | MF_STRING, alignmenu, menutxt );
			}
		else {
			strcpy(menutxt,"&Lock\tCtrl+L");					// switch lock/unlock menu text
			ModifyMenu( hMenu, CM_LOCKSECTION, MF_BYCOMMAND | MF_STRING | MF_ENABLED, CM_LOCKSECTION, menutxt );
			strcpy(menutxt,"Align sectio&n");					// switch Trace->Align menu text
			ModifyMenu( hTraceMenu, alignmenu, MF_BYPOSITION | MF_STRING, alignmenu, menutxt );
			if ( BackView ) EnableMenuItem( hMenu, CM_BYCORRELATION, MF_ENABLED );
			}

		state = MF_GRAYED;										// by default disable undo + reset
		if ( FrontView->section->HasUndos() )					// but if undos are present
			state = MF_ENABLED;									// enable undo menuitems
		EnableMenuItem( hMenu, CM_UNDOSECTION, state );
		EnableMenuItem( hMenu, CM_RESETSECTION, state );

//		if ( FrontView->section->HasUndos() && DomainSection )
//			EnableMenuItem( hMenu, CM_GETFROMRESET, MF_ENABLED );
//		else EnableMenuItem( hMenu, CM_GETFROMRESET, MF_GRAYED );

		state = MF_GRAYED;
		if ( FrontView->section->HasRedo() )					// similarly enable redo when present
			state = MF_ENABLED;
		EnableMenuItem( hMenu, CM_REDOSECTION, state );

		state = MF_GRAYED;								// disable most of trace menu when no contours selected
		if ( FrontView->section->active )				
		  if ( FrontView->section->active->contours )	// enable them otherwise
			state = MF_ENABLED;
		EnableMenuItem( hMenu, CM_UNSELECTALL, state );
		EnableMenuItem( hMenu, CM_ZOOMSELECTED, state );
		EnableMenuItem( hMenu, CM_TRACEATTRIBUTES, state );
		EnableMenuItem( hMenu, CM_CUTSELECTED, state );
		EnableMenuItem( hMenu, CM_COPYSELECTED, state );
		EnableMenuItem( hMenu, CM_DELETESELECTED, state );
		EnableMenuItem( hTraceMenu, alignmenu, MF_BYPOSITION | state );	// also align traces submenu
		EnableMenuItem( hMenu, CM_CALIBRATETRACES, state );
		EnableMenuItem( hMenu, CM_MERGESELECTED, state );
		EnableMenuItem( hMenu, CM_REVERSESELECTED, state );
		EnableMenuItem( hMenu, CM_SIMPLIFYSELECTED, state );
		EnableMenuItem( hMenu, CM_SMOOTHSELECTED, state );

		if ( FrontView->section->active )
			EnableMenuItem( hMenu, CM_COPYACTIVENFORM, MF_ENABLED );
		else EnableMenuItem( hMenu, CM_COPYACTIVENFORM, MF_GRAYED );

		if ( ClipboardTransform )						// if something to paste enable paste
			{
			EnableMenuItem( hMenu, CM_PASTESELECTED, MF_ENABLED );
			EnableMenuItem( hMenu, CM_PASTEATTRIBUTES, state );
			}
		else {
			EnableMenuItem( hMenu, CM_PASTESELECTED, MF_GRAYED );
			EnableMenuItem( hMenu, CM_PASTEATTRIBUTES, MF_GRAYED );
			}

		DrawMenuBar( appWnd );
		}
}

void UpdateBlendView( void )						// recompute blend view
{
	if ( BlendView && FrontView && BackView )
		{
		BlendView->view->Blend( FrontView->view, BackView->view );
		BlendView->ImageToViewDC();
		BlendView->section = BackView->section;
		BlendView->DrawContours( CurrSeries->pixel_size, CurrSeries->offset_x, CurrSeries->offset_y );
		BlendView->DrawActiveDomain( CurrSeries->pixel_size, CurrSeries->offset_x, CurrSeries->offset_y );
		BlendView->section = FrontView->section;
		BlendView->DrawContours( CurrSeries->pixel_size, CurrSeries->offset_x, CurrSeries->offset_y );
		BlendView->DrawActiveDomain( CurrSeries->pixel_size, CurrSeries->offset_x, CurrSeries->offset_y );
		}
}

void PaintViews( HDC hdc, RECT region )				// paint the screen from either front or blended views 
{
	bool frontUpdate, backUpdate;								// keep track of regeneration

	if ( FrontView )											// first regenerate the front view (if needed)
		{
		frontUpdate = FrontView->Regenerate( region, CurrSeries->pixel_size, CurrSeries->offset_x,
																	CurrSeries->offset_y, CurrSeries->useProxies );

		if ( BlendView && BackView )							// only have blend view when have back view
			{													// blend view depends also on back view so regenerate it also
			backUpdate = BackView->Regenerate( region, CurrSeries->pixel_size, CurrSeries->offset_x,
																	CurrSeries->offset_y, CurrSeries->useProxies );
			if ( frontUpdate || backUpdate ) UpdateBlendView();	// regenerate the blend of front and back views
			BlendView->Display( hdc, region );					// blt to screen dc (since BlendView exists its displayed)
			}
		else FrontView->Display( hdc, region );					// otherwise just blt front view to screen dc
		UpdateMenus();
		UpdateTitles();
		}
}

void InvalidateAllViews( void )						// after a view adjustment, e.g. a zoom, prepare everything to be redisplayed
{
	FrontView->needsRendering = true;
	FrontView->needsDrawing = true;						// if only have traces, must set needsDrawing to get update
	if ( BackView )
		{
		BackView->needsRendering = true;				// backview is other view (for flicker) not currently displayed
		BackView->needsDrawing = true;
		}
	if ( DomainView )									// if a domain is selected can also have views which are not in backview
		{
		if ( CurrView )
			{
			CurrView->needsRendering = true;
			CurrView->needsDrawing = true;
			}
		if ( PrevView )
			{
			PrevView->needsRendering = true;
			PrevView->needsDrawing = true;
			}
		}
}

void ScrollViews( void )								// shift the display by increment in x and y
{
	RECT region;

	if ( CurrSeries && (ScrollX || ScrollY) )
		{
		CurrSeries->offset_x -= CurrSeries->pixel_size*(double)ScrollX;
		CurrSeries->offset_y += CurrSeries->pixel_size*(double)ScrollY;

		if ( FrontView )				// move the front view
			{
			FrontView->ShiftView( ScrollX, ScrollY, CurrSeries->pixel_size,
									CurrSeries->offset_x, CurrSeries->offset_y, CurrSeries->useProxies);
			}
		if ( BackView )					// move the back view if needed for blend view
			if ( BlendView )
				{
				BackView->ShiftView( ScrollX, ScrollY, CurrSeries->pixel_size,
									CurrSeries->offset_x, CurrSeries->offset_y, CurrSeries->useProxies);
				UpdateBlendView();		// regenerate the blend of front and back views
				}
		InvalidateAllViews();			// flag for full rerendering later

		ClientRectLessStatusBar( &region );
		if ( BlendView ) BlendView->Display(appDC, region );
		else FrontView->Display(appDC, region );

		ScrollOccurred = true;
		if ( ScrollX ) LToolRect.left += ScrollX;
		if ( ScrollY ) LToolRect.top += ScrollY;
		}
}

void ClientRectLessStatusBar( RECT *r )
{											// return client rectangle of application window
	RECT s;
	GetClientRect( appWnd, r );
	if ( IsWindow( statusbarWindow ) )		// but subtract status bar if present in client area
		{
		GetClientRect( statusbarWindow, &s );
		r->bottom -= s.bottom - s.top;
		}
}

void ClientRectToScreen( HWND hWnd, RECT *r )		// convert r from clientRect to screen coordinates
{
	POINT p;
	p.x = r->left;
	p.y = r->top;
	ClientToScreen( hWnd, &p );
	r->left = p.x;
	r->top = p.y;
	p.x = r->right;
	p.y = r->bottom;
	ClientToScreen( hWnd, &p );
	r->right = p.x;
	r->bottom = p.y;
}
													// generic input dialog procedure

BOOL CALLBACK InputDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	switch (message)
		{
		case WM_INITDIALOG:
			SetWindowText( hwndDlg, InputDlgName );
			SetDlgItemText( hwndDlg, ID_INPUTVALUE, InputDlgValue );
   			SetDlgItemText( hwndDlg, ID_INPUTSTRING, InputDlgString );
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case IDOK:
   					GetDlgItemText( hwndDlg, ID_INPUTSTRING, InputDlgString, sizeof(InputDlgString) );
					EndDialog(hwndDlg, 1);
					return TRUE;
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				}
		}
	return FALSE;
}
													// ask for section range for operation

BOOL CALLBACK SectionRangeDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char dlgItemTxt[64];							// when init, display a msg pointed to by lParam

	switch (message)
		{
		case WM_INITDIALOG:							// FirstSection, LastSection are global variables
			sprintf(dlgItemTxt,"%d",FirstSection);
			SetDlgItemText( hwndDlg, ID_FIRSTSECTION, dlgItemTxt );
			sprintf(dlgItemTxt,"%d",LastSection);
   			SetDlgItemText( hwndDlg, ID_LASTSECTION, dlgItemTxt );
			SetDlgItemText( hwndDlg, ID_SECTIONRANGEMSG, (char *)lParam );	// lParam points to char string
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{	
				case IDOK:
   					GetDlgItemText( hwndDlg, ID_FIRSTSECTION, dlgItemTxt, sizeof(dlgItemTxt) );
					FirstSection = atoi(dlgItemTxt);
   					GetDlgItemText( hwndDlg, ID_LASTSECTION, dlgItemTxt, sizeof(dlgItemTxt) );
					LastSection = atoi(dlgItemTxt);
					EndDialog(hwndDlg, 1);			// return non-zero value if user selects OK
					return TRUE;
				case IDCANCEL:
					EndDialog(hwndDlg, 0);			// otherwise return zero
					return TRUE;
				}
		}
	return FALSE;
}

															// use GBM library to read image file params:
int TestGBMfile( const char *fn, int *w, int *h, int *bpp, int *n )
{															// set w=width, h=height, bpp=bits per pixel, n = images in stack
	int fd, ft, num;										// return 0 if format not recognized; filetype > 0 otherwise
	GBM gbm;
	GBMRGB *gbmrgb;
	GBM_ERR rc;
	char opt[64], buf[32], errmsg[80];
	HANDLE file;
	DWORD numbytesread;
													// first see what GBM makes of the filename
	if ( gbm_guess_filetype(fn, &ft) != GBM_ERR_OK )
		{											// need to check header, so open the file...
		file = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		ft = -1;
		if ( file )
			{												// if successful, read first characters
			strcpy(buf,"000000000000000000000");			// clear buffer
			ReadFile( file, buf, 20, &numbytesread, NULL );	// read from file
			CloseHandle( file );
			if ( !(strncmp(buf, "\x42\x4D", 2)) ) ft = 0;				// Windows Bitmap
			else if ( !(strncmp(buf, "\x47\x49\x46", 3)) ) ft = 1;		// GIF
			else if ( !(strncmp(buf, "\x49\x49\x2A\x00", 4)) ) ft = 3;	// TIFF
			else if ( !(strncmp(buf, "\x4D\x4D\x00\x2A", 4)) ) ft = 3;	// TIFF
			else if ( !(strncmp(buf, "\xff\xd8\xff", 3)) ) ft = 16;		// JPEG
			}
		}

	if ( ft >= 0 )									// if found a valid filetype
		{
		if ( (fd = gbm_io_open(fn, O_RDONLY|O_BINARY)) == -1 )	// open file
			{	
			sprintf(errmsg,"Can't open %s", fn);
			ErrMsgOK( ERRMSG_READ_FAILED, errmsg );
			return 0;
			}

		opt[0] = '\0';											// no option string used

		if ( (rc = gbm_read_header(fn, fd, ft, &gbm, opt)) != GBM_ERR_OK )	// read header
			{
			sprintf(errmsg,"Can't read header of %s.\nError is %d.", fn, rc);
			ErrMsgOK( ERRMSG_READ_FAILED, errmsg );
			gbm_io_close(fd);
			return 0;
			}

		*w = gbm.w;									// width in pixels
		*h = gbm.h;									// height in pixels
		*bpp = gbm.bpp;								// bits per pixel
		num = 1;									// there's always at least one image
		sprintf(opt,"index=%d",num);				// if GIF or TIFF there could be more images
		if ( (ft==1) || (ft==3) )
			while ( gbm_read_header(fn, fd, ft, &gbm, opt) == GBM_ERR_OK )
				{
				num++;								// count through rest of images in file
				sprintf(opt,"index=%d",num);		// final n is number of images in stack
				}
		*n = num;									// update passed variable (1 more than index of last image)
		gbm_io_close(fd);
		}
													
   return ft+1;							// offset GBM types by one so zero is "file not recognized"
}

void ColorButton( HWND hWnd, Color color )		// apply color to button whose handle is hWnd
{
	RECT wrect;
	HDC winDC, bitmapDC;
	HPEN pen;
	HBRUSH brush;
	HBITMAP bitmap, oldbitmap;

	winDC = GetDC( hWnd );									// create bitmap based on button window	
	GetClientRect( hWnd, &wrect );
	bitmap = CreateCompatibleBitmap( winDC, wrect.right-wrect.left, wrect.bottom-wrect.top );
	bitmapDC = CreateCompatibleDC( winDC );
	ReleaseDC( hWnd, winDC );								// done with button DC for now
	oldbitmap = (HBITMAP)SelectObject( bitmapDC, bitmap );	// use bitmapDC to color bitmap
	if ( color.invalid() )									// use don't care/change symbolism
		{
		pen = CreatePen( PS_SOLID, 1, RGB(0,0,0) );			// use black pen
		SelectObject( bitmapDC, pen );
		brush = CreateSolidBrush( RGB(128,128,128) );		// with gray brush to symbol no change
		SelectObject( bitmapDC, brush );
		Rectangle( bitmapDC, 0, 0, wrect.right-wrect.left, wrect.bottom-wrect.top );
		MoveToEx( bitmapDC, 0, 0, NULL );
		LineTo( bitmapDC, wrect.right, wrect.bottom );
		}
	else {													// use color to paint button
		pen = CreatePen( PS_SOLID, 1, color.ref() );
		SelectObject( bitmapDC, pen );
		brush = CreateSolidBrush( color.ref() );			// color bitmap
		SelectObject( bitmapDC, brush );
		Rectangle( bitmapDC, 0, 0, wrect.right-wrect.left, wrect.bottom-wrect.top );
		}
	SelectObject( bitmapDC, oldbitmap );					// release bitmap for use on button
	DeleteObject( brush );									// free pen, brush memory
	DeleteObject( pen );
	DeleteDC( bitmapDC );									// set colored bitmap on button
	oldbitmap = (HBITMAP)SendMessage( hWnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(bitmap) );
	if ( oldbitmap ) DeleteObject( oldbitmap );				// if there was already a bitmap, delete it
}




void UpdateStatusBar( void )		// called from WM_TIMER message in main window (reconstruct.cpp)
{									// only called when STATUS_TIMER is active (when status bar present)
	Transform *transform;
	Contour *contour;
	double fx, fy, v, z1, z2;
	int x, y, i;
	HDC dc, bdc;
	HBITMAP orig;
	RECT r;
	POINT mpoint;
	bool inWindow;
	char status[1024], txt[128+MAX_UNITS_STRING+MAX_CONTOUR_NAME+MAX_COMMENT];
	char defname[MAX_CONTOUR_NAME], name[MAX_CONTOUR_NAME];


	if ( IsWindow( statusbarWindow ) )
		{
		inWindow = true;												// figure out whether in main client region...
		GetCursorPos(&mpoint);											// get cursor position in screen coordinates
		if (WindowFromPoint(mpoint) != appWnd ) inWindow = false;		// make sure cursor not on a child window
		ScreenToClient(appWnd,&mpoint);									// convert to client coordinates
		GetClientRect(appWnd,&r);										// check client area
		if ( !PtInRect(&r,mpoint) ) inWindow = false;					// to make sure cursor not on menu or borders
	
		strcpy(status,"\0");
		if ( FrontView && CurrSeries )									// if a section is visible...
		  if ( FrontView->section )
			{															// ...display section number
			sprintf(status,"Section: %d", FrontView->section->index );	// and lock status
	
			SendMessage( statusbarWindow, SB_GETRECT, 0, (LPARAM)(&r) );
			x = (r.right + r.left)/2 - 8;								// determine where lock bitmap goes
			y = (r.bottom + r.top)/2 - 8;
			dc = GetDC( statusbarWindow );								// create memory DC so can display HBITMAP					
			bdc = CreateCompatibleDC( dc );								// select bitmap in DC to display it
			if ( FrontView->section->alignLocked ) orig = (HBITMAP)SelectObject( bdc, lockBitmap );
			else orig = (HBITMAP)SelectObject( bdc, unlockBitmap );
			BitBlt( dc, x, y, 16, 16, bdc, 0, 0, SRCCOPY );				// display bitmap
			SelectObject( bdc, orig );
			DeleteDC( bdc );
			ReleaseDC( statusbarWindow, dc );							// clean up gdi objects

			if ( inWindow )
			  if ( LToolActive )
				{														// convert tool delta to section coords.
				fx = ((double)abs(LToolRect.right-LToolRect.left))*CurrSeries->pixel_size;
				fy = ((double)abs(LToolRect.bottom-LToolRect.top))*CurrSeries->pixel_size;
				switch ( CurrentTool )
					{
					case ARROW_TOOL:
					case MAGNIFY_TOOL:
					case WILDFIRE_TOOL:
						v = fx*fy;
						sprintf(txt,"    Selecting area of %1.*g square %s", Precision, v, CurrSeries->units );
						strcat(status,txt);
						break;
					case ELLIPSE_TOOL: 
					case RECTANGLE_TOOL:
						v = fx*fy; if ( CurrentTool == ELLIPSE_TOOL ) v = PI*v/4.0;
						strcpy(defname,CurrSeries->defaultName);				// make local copy so won't modify defaultName
						FrontView->section->SetDefaultName(name,defname,FrontView==DomainView); // parse name string
						sprintf(txt,"    Drawing: %s    Area: %1.*g square %s", name, Precision, v, CurrSeries->units );
						strcat(status,txt);
						break;
					case ZOOM_TOOL:
						sprintf(txt,"    Panning by %1.*g, %1.*g %s", Precision, fx, Precision, fy, CurrSeries->units );
						strcat(status,txt);
						break;
					case DOMAIN_TOOL:
						sprintf(txt,"    Moving by %1.*g, %1.*g %s", Precision, fx, Precision, fy, CurrSeries->units );
						strcat(status,txt);
						break;
					case LINE_TOOL:
						v = sqrt(fx*fx+fy*fy);
						strcpy(defname,CurrSeries->defaultName);				// make local copy so won't modify defaultName
						FrontView->section->SetDefaultName(name,defname,false); // parse name string for next name
						sprintf(txt,"    Drawing: %s    Length: %1.*g %s", name, Precision, v, CurrSeries->units );
						strcat(status,txt);
						break;
					case MULTILINE_TOOL:
					case CULTILINE_TOOL:
						mpoint.y = FrontView->height - mpoint.y;			// convert cursor position to section coords.
						fx = CurrSeries->offset_x + ((double)mpoint.x)*CurrSeries->pixel_size;
						fy = CurrSeries->offset_y + ((double)mpoint.y)*CurrSeries->pixel_size;
						if ( EditContour ) strcpy(name,EditContour->name);
						else sprintf(name,"??");
						sprintf(txt,"    Drawing: %s    Position: %1.*g, %1.*g %s", name, Precision, fx, Precision, fy, CurrSeries->units );
						strcat(status,txt);
						break;
					case ZLINE_TOOL:
						v = fx*fx+fy*fy; 
						if ( EditContour )
						  if ( EditContour->points->first )
							{
							z1 = CurrSectionsInfo->ZDistance( (int)EditContour->points->first->z, CurrSeries->zMidSection );
							z2 = CurrSectionsInfo->ZDistance( FrontView->section->index, CurrSeries->zMidSection );
							v += (z2-z1)*(z2-z1);
							}
						v = sqrt(v);
						strcpy(defname,CurrSeries->defaultName);				// make local copy so won't modify defaultName
						FrontView->section->SetDefaultName(name,defname,false);		// parse name string for next contour name
						sprintf(txt,"    Drawing: %s    Length increment: %1.*g %s", name, Precision, v, CurrSeries->units );
						strcat(status,txt);
						break;
					case PENCIL_TOOL:
						strcpy(defname,CurrSeries->defaultName);				// make local copy so won't modify defaultName
						FrontView->section->SetDefaultName(name,defname,FrontView==DomainView); // parse name string
						sprintf(txt,"    Drawing: %s", name );
						strcat(status,txt);
						break;
					}
				}
			  else if ( FrontView == DomainView )						// if domain, show pixel position
				{
				mpoint.y = FrontView->height - mpoint.y;					// convert cursor position to section coords.
				fx = CurrSeries->offset_x + ((double)mpoint.x)*CurrSeries->pixel_size;
				fy = CurrSeries->offset_y + ((double)mpoint.y)*CurrSeries->pixel_size;
				transform = DomainView->DomainFromPixel( mpoint.x, mpoint.y );
				if ( transform )
					{													// put cursor pt into image coordinates
					x = (int)(transform->nform->X( fx, fy )/transform->image->mag);
					y = (int)(transform->nform->Y( fx, fy )/transform->image->mag);
					sprintf(txt,"    Domain: %s    Position: %d, %d pixels", transform->domain->name, x, y);
					}
				else sprintf(txt,"   Domain View");
				strcat(status,txt);
				}			
			  else														// if section, report what's under cursor
				{
				mpoint.y = FrontView->height - mpoint.y;				// convert cursor position to DIB coords.
				if ( CurrentTool == WILDFIRE_TOOL )						// for Wildfire report the HSB of the pixel
					{
					i = FrontView->view->GetHSBPixel( mpoint.x, mpoint.y );	// retrieve pixel in HSB format
					sprintf(txt,"    Hue: %03d, Sat: %03d, Bright: %03d", (i&0x00ff0000)>>16, (i&0x0000ff00)>>8, (i&0x000000ff) );
					}
				else													// for all other tools indicate contour and cursor position
					{
					fx = CurrSeries->offset_x + ((double)mpoint.x)*CurrSeries->pixel_size;
					fy = CurrSeries->offset_y + ((double)mpoint.y)*CurrSeries->pixel_size;
					contour = FrontView->section->FindClosestContour( fx, fy );
					if ( contour )										// display status AND contour comment field!
						{
						sprintf(txt,"    Trace: %s    Position: %1.*g, %1.*g %s", contour->name, Precision, fx, Precision, fy, CurrSeries->units );
						SendMessage( statusbarWindow, SB_SETTEXT, (WPARAM)2, (LPARAM)contour->comment );
						}
					else sprintf(txt,"    Position: %1.*g, %1.*g %s", Precision, fx, Precision, fy, CurrSeries->units );
					}
				strcat(status,txt);
				}

			}															// write out status text
		SendMessage( statusbarWindow, SB_SETTEXT, (WPARAM)1, (LPARAM)status );
		}
}

void SaveList( HWND list, char *filename )		// save data from listview to file
{
	LV_ITEM lvi;
	LV_COLUMN lvc;
 	HANDLE hFile;
	DWORD byteswritten;
	bool ErrorOccurred = false;
	int i;
	char txt[MAX_COMMENT];
	char *l, line[512], errmsg[1024];
																// first, check for list item
	
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL) );
	if ( lvi.iItem < 0 ) return;								// no traces in list so quit
																// otherwise attempt to open file

	hFile = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		{														// open failed, format error string
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							errmsg, 512, NULL );
		strcat(errmsg, filename );
		ErrMsgOK( ERRMSG_WRITE_FAILED, errmsg );
		return;
		}
															// retreive column header info from listview
	l = line;												// format into header with fixed width fields
	i = 0;
	lvc.mask = LVCF_TEXT; lvc.pszText = txt; lvc.cchTextMax = MAX_COMMENT;
	while ( SendMessage(list, LVM_GETCOLUMN, i, (LPARAM)(&lvc) ) )
		{
		l += sprintf(l,"%s,",txt);
		i++;
		}													// write header line
	sprintf(l,"\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	while ( lvi.iItem >= 0 )									// now do all items until done
		{
		l = line;												// format the line string with listview items
		lvi.mask = LVIF_TEXT; lvi.cchTextMax = MAX_COMMENT; lvi.pszText = txt;
		lvi.iSubItem = 0;
		while ( lvi.iSubItem < i )								// retrieve same i columns as in header
			{
			SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));	// get the field text info
			l += sprintf(l,"%s,",txt);							// add it to the line
			lvi.iSubItem++;
			}
		sprintf(l,"\r\n");										// write trace data line...

		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
		
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL) );
		}

	CloseHandle( hFile );

	if ( ErrorOccurred )
		ErrMsgOK( ERRMSG_WRITE_FAILED, filename );
}


void RemoveIllegalChars( char *s )				// remove characters not allowed in XML field
{
	int i = 0;
	while ( s[i] )
		{
		if ( (s[i]=='\\') || (s[i]=='\"') || (s[i]=='<') || (s[i]=='>') || (s[i]=='=') || (s[i]==',') )
			s[i] = '_';
		i++;
		}
}

bool MatchLimit( char *s, char *t )				// true if s matches t up to * character
{
	int i = 0;
	bool match = true;
	
	while ( match )								// done when mismatch found
		{
		if ( t[i] == '*' ) break;				// also exit with true if * found before mismatched chars
		if ( s[i] != t[i] )
			{
			if ( (t[i] == '#') && isdigit(s[i]) ) ;	// '#' matches a single digit
			else if ( (t[i] == '?') && s[i] ) ;		// '?' matches anything but the end of string
			else match = false;
			}										// exit when limit string is done
		if ( (t[i] == '\0') || (s[i] == '\0') ) break;
		i++;
		}
	return match;
}

void DisplayListInfo( HWND list )		// display statistics of listview in dialog box
{
	char txt[128];
	int item_count;
	int selected_count;
	//int page_count;

	if ( list )
		{
		item_count = SendMessage(list, LVM_GETITEMCOUNT, 0, 0 );
		selected_count = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
		//page_count = SendMessage(list, LVM_GETCOUNTPERPAGE, 0, 0 );
		//if ( item_count < page_count ) page_count = item_count;
		sprintf(txt,"List contains: \n\n%d items\n\n%d highlighted", item_count, selected_count );
		MessageBox(list,txt,"List Info",MB_OK);
		}
}


															// sort in ascending order subarrays[begin,end] 
void Sort(int begin, int end, double arr[], double brr[])	// from two arrays {arr, brr} using arr as key
{															// (adapted from Quicksort algorithm of NRinC)
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;
	int i,ir=end,j,k,l=begin;							
	int istack[65],jstack=0;							// 2log(2)N = 64 => so will allow 32-bit integer index
	double a,b,temp;			

	for (;;)						// loop until all items sorted
		{
		if (ir-l < 7)
			{						// use insertion sort for subarrays of less than 7 elements
			for (j=l+1;j<=ir;j++)
				{
				a=arr[j];
				b=brr[j];
				for (i=j-1;i>=l;i--)
					{
					if (arr[i] <= a) break;
					arr[i+1]=arr[i];
					brr[i+1]=brr[i];
					}
				arr[i+1]=a;
				brr[i+1]=b;
				}
			if (!jstack) return;	// if stack empty, done
			ir=istack[jstack];
			l=istack[jstack-1];
			jstack -= 2;
			}
		else {
			k=(l+ir) >> 1;
			SWAP(arr[k],arr[l+1])
			SWAP(brr[k],brr[l+1])
			if (arr[l] > arr[ir])
				{
				SWAP(arr[l],arr[ir])
				SWAP(brr[l],brr[ir])
				}
			if (arr[l+1] > arr[ir])
				{
				SWAP(arr[l+1],arr[ir])
				SWAP(brr[l+1],brr[ir])
				}
			if (arr[l] > arr[l+1])
				{
				SWAP(arr[l],arr[l+1])
				SWAP(brr[l],brr[l+1])
				}
			i=l+1;
			j=ir;
			a=arr[l+1];
			b=brr[l+1];
			for (;;)
				{
				do i++; while (arr[i] < a);
				do j--; while (arr[j] > a);
				if (j < i) break;
				SWAP(arr[i],arr[j])
				SWAP(brr[i],brr[j])
				}
			arr[l+1]=arr[j];
			arr[j]=a;
			brr[l+1]=brr[j];
			brr[j]=b;
			jstack += 2;
			if (ir-i+1 >= j-l)
				{
				istack[jstack]=ir;
				istack[jstack-1]=i;
				ir=j-1;
				}
			else {
				istack[jstack]=j-1;
				istack[jstack-1]=l;
				l=i;
				}
			}
		}
#undef SWAP
}

double pythag(double a, double b)						// used only in svd() routine
{
	double absa, absb, r;
	absa = fabs(a);
	absb = fabs(b);
	if ( absa > absb )
		{
		r = absb/absa;
		if ( r == 0.0 ) return( absa );
		else return( absa*sqrt(1.0+r*r) );
		}
	else
		{
		r = absa/absb;
		if ( r == 0.0 ) return( absb );
		else return( absb*sqrt(1.0+r*r) );
		}
}

void svd(double **a, int m, int n, double *w, double **v)	// compute the singular value decomposition of A = U w V_transpose
{															// where A is m x n real matrix using Householder bidiagonalization
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))			// and a variant of the QR algorithm, on return A is overwritten by U
	int flag,i,its,j,jj,k,l,nm;								// to get A back compute R[i][j] += A[i][k]*V[j][k]*s[k] over k,j,i
	double anorm,c,f,g,h,s,scale,x,y,z,*rv1;				// based on Golub & Reinsch algorithm as written by Forsythe et al. (1977)
															// converted to C++ in the manner of NRinC but with zero-based indices
	rv1 = new double[n];
	g = 0.0; scale = 0.0; anorm = 0.0;
	for (i=0;i<n;i++)										// householder reduction to bidiagonal form...
		{
		l = i+1;
		rv1[i] = scale*g;
		g = 0.0; s = 0.0; scale = 0.0;
		if ( i < m )
			{
			for (k=i;k<m;k++) scale += fabs(a[k][i]);
			if ( scale )
				{
				for (k=i;k<m;k++)
					{
					a[k][i] /= scale;
					s += a[k][i]*a[k][i];
					}
				f = a[i][i];
				g = -SIGN(sqrt(s),f);
				h = f*g-s;
				a[i][i] = f-g;
				for (j=l;j<n;j++)
					{
					for (s=0.0,k=i;k<m;k++) s += a[k][i]*a[k][j];
					f = s/h;
					for (k=i;k<m;k++) a[k][j] += f*a[k][i];
					}
				for (k=i;k<m;k++) a[k][i] *= scale;
				}
			}
		w[i] = scale*g;
		g = 0.0;
		s = 0.0;
		scale = 0.0;
		if ( (i < m) && (i != n-1) )
			{
			for (k=l;k<n;k++) scale += fabs(a[i][k]);
			if ( scale )
				{
				for (k=l;k<n;k++)
					{
					a[i][k] /= scale;
					s += a[i][k]*a[i][k];
					}
				f = a[i][l];
				g = -SIGN(sqrt(s),f);
				h = f*g-s;
				a[i][l] = f-g;
				for (k=l;k<n;k++) rv1[k] = a[i][k]/h;
				for (j=l;j<m;j++)
					{
					for (s=0.0,k=l;k<n;k++) s += a[j][k]*a[i][k];
					for (k=l;k<n;k++) a[j][k] += s*rv1[k];
					}
				for (k=l;k<n;k++) a[i][k] *= scale;
				}
			}
		anorm = max(anorm,(fabs(w[i])+fabs(rv1[i])));
		}

	for (i=n-1;i>=0;i--)										// accumulation of right-hand transformations
		{
		if (i < n-1)
			{
			if ( g )
				{
				for (j=l;j<n;j++)
					v[j][i] = (a[i][j]/a[i][l])/g;				// double division avoids underflow
				for (j=l;j<n;j++)
					{
					for (s=0.0,k=l;k<n;k++) s += a[i][k]*v[k][j];
					for (k=l;k<n;k++) v[k][j] += s*v[k][i];
					}
				}
			for (j=l;j<n;j++) v[i][j]=v[j][i]=0.0;
			}
		v[i][i] = 1.0;
		g = rv1[i];
		l = i;
		}

	for (i=min(m-1,n-1);i>=0;i--)								// accumluation of left-hand transformations
		{
		l = i+1;
		g = w[i];
		for (j=l;j<n;j++) a[i][j] = 0.0;
		if ( g )
			{
			g = 1.0/g;											// invert to avoid repeated division
			for (j=l;j<n;j++)
				{
				for (s=0.0,k=l;k<m;k++) s += a[k][i]*a[k][j];
				f = (s/a[i][i])*g;								// double division avoids underflow
				for (k=i;k<m;k++) a[k][j] += f*a[k][i];
				}
			for (j=i;j<m;j++) a[j][i] *= g;
			}
		else for (j=i;j<m;j++) a[j][i] = 0.0;
		++a[i][i];
		}

	for (k=n-1;k>=0;k--)										// diagonalization of bidiagonal form
		{
		for (its=1;its<=30;its++)								// give up after 30 iterations if not converging
			{
			flag = 1;
			for (l=k;l>=0;l--)									// test for splitting
				{
				nm = l-1;
				if ((double)(fabs(rv1[l])+anorm) == anorm)
					{
					flag = 0;									// rv1(1) is always zero, so there is no exit
					break;
					}
				if ((double)(fabs(w[nm])+anorm) == anorm) break;
				}
			if ( flag )											// cancellation of rv1[l] if l > 0
				{
				c = 0.0;
				s = 1.0;
				for (i=l;i<=k;i++)
					{
					f = s*rv1[i];
					rv1[i] = c*rv1[i];
					if ((double)(fabs(f)+anorm) == anorm) break;
					g = w[i];
					h = pythag(f,g);
					w[i] = h;
					h = 1.0/h;
					c = g*h;
					s = -f*h;
					for (j=0;j<m;j++)
						{
						y = a[j][nm];
						z = a[j][i];
						a[j][nm] = y*c+z*s;
						a[j][i] = z*c-y*s;
						}
					}
				}												// test for convergence
			z = w[k];
			if ( l == k )										// shift from bottom 2x2 minor
				{
				if ( z < 0.0 )
					{
					w[k] = -z;
					for (j=0;j<n;j++) v[j][k] = -v[j][k];
					}
				break;
				}
			if ( its == 30 ) { ErrMsgOK(ERRMSG_CONVERGENCE,NULL); break; }
			x = w[l];
			nm = k-1;
			y = w[nm];
			g = rv1[nm];
			h = rv1[k];
			f = ((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g = pythag(f,1.0);
			f = ((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			c = 1.0; s = 1.0;									// next QR transformation
			for (j=l;j<=nm;j++)
				{
				i = j+1;
				g = rv1[i];
				y = w[i];
				h = s*g;
				g = c*g;
				z = pythag(f,h);
				rv1[j] = z;
				c = f/z;
				s = h/z;
				f = x*c+g*s;
				g = g*c-x*s;
				h = y*s;
				y *= c;
				for (jj=0;jj<n;jj++)
					{
					x = v[jj][j];
					z = v[jj][i];
					v[jj][j] = x*c+z*s;
					v[jj][i] = z*c-x*s;
					}
				z = pythag(f,h);
				w[j] = z;										// rotation can be arbitrary of z is zero
				if ( z )
					{
					z = 1.0/z;
					c = f*z;
					s = h*z;
					}
				f = c*g+s*y;
				x = c*y-s*g;
				for (jj=0;jj<m;jj++)
					{
					y = a[jj][j];
					z = a[jj][i];
					a[jj][j] = y*c+z*s;
					a[jj][i] = z*c-y*s;
					}
				}
			rv1[l] = 0.0;
			rv1[k] = f;
			w[k] = x;
			}
		}
	delete[] rv1;
#undef SIGN
}
