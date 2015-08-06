////////////////////////////////////////////////////////////////////////
//	Routines to perform the menu functions in the Program menu.
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
// modified 6/16/06 by JCF (fiala@bu.edu)
// -+- change: Moved debugging menu items here from help_menu.cpp. Added Debug Logging.
// modified 6/23/06 by JCF (fiala@bu.edu)
// -+- change: Modified CmDebug output.
// modified 6/28/06 by JCF (fiala@bu.edu)
// -+- change: Added trace comment part to statusbar.
// modified 2/5/07 by JCF (fiala@bu.edu)
// -+- change: Created separate CmDebugTimes() to allow more debug menu items.
//

#include "reconstruct.h"

void CmRestoreToolbar( void )						// create or destroy toolbar window
{
	if ( IsWindow( toolbarWindow ) ) DestroyWindow( toolbarWindow );
	else toolbarWindow = MakeToolWindow();
}													// menu check/uncheck is done in the code for the tool window
													// so that it occurs even if toolbar is closed by other means

void CmStatusBar( void )							// create or destroy status window
{
	HMENU hMenu;
	RECT r;
	int parts[3];									// statusbar will have three parts
	char txt[4];

	hMenu = GetMenu( appWnd );

	if ( IsWindow( statusbarWindow ) ) 
		{
		DestroyWindow( statusbarWindow );
		KillTimer( appWnd, STATUS_TIMER );
		CheckMenuItem( hMenu, CM_STATUSBAR, MF_BYCOMMAND | MF_UNCHECKED );
		}
	else {
		statusbarWindow = CreateStatusWindow( WS_CHILD | WS_VISIBLE, "Reconstruct Status", appWnd, ID_STATUSBAR );
		if ( statusbarWindow ) {
			GetClientRect(statusbarWindow, &r);		// first part for bitmap, second for status text, third for comment
			parts[0] = 32; parts[1] = 2*r.right/3; parts[2] = -1;
			SendMessage( statusbarWindow, SB_SETPARTS, (WPARAM)3, (LPARAM)(&parts) );
			strcpy(txt,"   ");						// clear background of lock bitmap area
			SendMessage( statusbarWindow, SB_SETTEXT, (WPARAM)0, (LPARAM)txt);
			CheckMenuItem( hMenu, CM_STATUSBAR, MF_BYCOMMAND | MF_CHECKED );
			SetTimer( appWnd, STATUS_TIMER, 100, NULL);
			}
		}
	DrawMenuBar( appWnd );
}

void CmDebugLog( void )								// turn on/off debug logging of subroutines
{
	HMENU hMenu;
	hMenu = GetMenu( appWnd );
													// use file handle to know whether to log debug info
	if ( debugLogFile == INVALID_HANDLE_VALUE )
		{
		debugLogFile = CreateFile( "debug.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if ( debugLogFile == INVALID_HANDLE_VALUE )
			ErrMsgOK( ERRMSG_WRITE_FAILED, "debug.txt" );
		else CheckMenuItem( hMenu, CM_DEBUGLOG, MF_BYCOMMAND | MF_CHECKED );
		}
	else {											// set file handle to invalid to turn off logging
		CloseHandle( debugLogFile );
		debugLogFile = INVALID_HANDLE_VALUE;
		CheckMenuItem( hMenu, CM_DEBUGLOG, MF_BYCOMMAND | MF_UNCHECKED );
		}
}

void CmDebugTimes( void )					// report execution times of debug timing variables				
{
	char txt[256], msg[512];

	if ( nTime1 )
		sprintf(msg,"%d calls to create object list used %dms\n\n",nTime1,totalTime1);
	else sprintf(msg,"No calls to create object list yet!\n\n");
	nTime1 = 0; totalTime1 = 0;
	MessageBox(appWnd,msg,"Debug",MB_OK);

	if ( nTime2 )
		sprintf(msg,"%d calls to create thumbnails used %dms\n\n",nTime2,totalTime2);
	else sprintf(msg,"No calls to create thumbnails yet!\n\n");
	nTime2 = 0; totalTime2 = 0;
	MessageBox(appWnd,msg,"Debug",MB_OK);

	if ( nTime3 )
		sprintf(msg,"%d calls to render images used %dms\n\n",nTime3,totalTime3);
	else sprintf(msg,"No calls to render images yet!\n\n");
	nTime3 = 0; totalTime3 = 0;
	MessageBox(appWnd,msg,"Debug",MB_OK);

}

void CmDebug( void )								// test something special here if needed, otherwise comment out				
{
	char msg[100];

//	strcpy(InputDlgName,"fromPath");		    // set up dialog params
//	strcpy(InputDlgValue,"fromPath");
//	strcpy(InputDlgString,"fromPath");			// or just use last search string
//	DialogBox( appInstance, "InputDlg", appWnd, (DLGPROC)InputDlgProc );
//	strcpy(msg,InputDlgString);

	if ( FrontView )
	  if ( FrontView->view )
        {
		sprintf(msg,"width=%d, height=%d",FrontView->view->width, FrontView->view->height);
		MessageBox(0,msg,"Debug",MB_OK);
		}
}
