/////////////////////////////////////////////////////////////////////////////
//	This file contains menu help items
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
// modified 11/01/04 by JCF (fiala@bu.edu)
// -+- change: Modified Debug procedure for timing wildfires.
// modified 3/07/05 by JCF (fiala@bu.edu)
// -+- change: Updated License copyright to 2005. Modified Debug() to test some timing issues.
// modified 6/07/05 by JCF (fiala@bu.edu)
// -+- change: Updated License to reflect credit to additional authors.
// modified 11/17/05 by JCF (fiala@bu.edu)
// -+- change:  Modified CmDebug to debug Object List hanging problem.
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Updated the copyright/license information.
// modified 6/16/06 by JCF (fiala@bu.edu)
// -+- change: Moved debugging menu item to Program menu.
// modified 11/08/06 by JCF (fiala@bu.edu)
// -+- change: Converted old WinHelp files to HtmlHelp system
// modified 7/20/07 by JCF (fiala@bu.edu)
// -+- change: Fixed typo in license text.
//

#include "reconstruct.h"
#include <htmlhelp.h>	// not needed for WinHelp so added 11/08/06 for html help system

void SpawnProcessSimple ( char *cmd ) {
    // LPSTR szCmd[] = _tcsdup(TEXT("\"hh manual.chm\""));
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory ( &si, sizeof(si) );
    ZeroMemory ( &pi, sizeof(pi) );
    si.cb = sizeof(si);
    // Get the path of this executable
    HMODULE hModule = GetModuleHandle(NULL);
    char exe[MAX_PATH+1];
    GetModuleFileName ( hModule, exe, MAX_PATH );

    //printf ( "Cmd = %s\n", cmd );
    //printf ( "Exe = %s\n", exe );
    char path[MAX_PATH+1];
    strcpy ( path, exe );
    // Chop off the "/Reconstruct.exe" part (hard coded for now!!)
    path[strlen(path)-16] = '\0';
    //printf ( "Path = %s\n", path );

    //              App   Cmd  Psec  Tsec  InhH   F  Env   Dir    SInf Pinf
    CreateProcess ( NULL, cmd, NULL, NULL, FALSE, 0, NULL, path, &si, &pi );
}

void CmHelpReconstruct( void )		// open WinHelp for User's Manual
{
	char mpath[MAX_PATH], *c, *pathend;

	GetModuleFileName( GetModuleHandle(NULL), mpath, MAX_PATH );

	c = mpath;									// mpath is location of application .exe
	while ( *c != '\0' ) {
		if ( *c == '\\' ) pathend = c;			// find last directory
		c++;
		}

	strcpy(pathend,"\\manual.chm\0");		// specify path to help file

	/*
	if ( !HtmlHelp(appWnd, mpath, HH_DISPLAY_TOPIC, NULL ) )
		ErrMsgOK( ERRMSG_RUN_FAILED, mpath );
	*/
	// Since HtmlHelp fails the linking step, try another way (may require Reconstruct to be started in .exe folder)
    SpawnProcessSimple ( "hh manual.chm" );
}

void CmHelpKeyTable( void )			// open Key Command quick reference guide
{
	char mpath[MAX_PATH], *c, *pathend;

	GetModuleFileName( GetModuleHandle(NULL), mpath, MAX_PATH );

	c = mpath;									// mpath is location of application .exe
	while ( *c != '\0' ) {
		if ( *c == '\\' ) pathend = c;			// find last directory
		c++;
		}

	strcpy(pathend,"\\keycmds.chm\0");			// specify path to help file

	/*
	if ( !HtmlHelp(appWnd, mpath, HH_DISPLAY_TOPIC, NULL ) )
		ErrMsgOK( ERRMSG_RUN_FAILED, mpath );
	*/
	// Since HtmlHelp fails the linking step, try another way (may require Reconstruct to be started in .exe folder)
    SpawnProcessSimple ( "hh keycmds.chm" );
}

void CmHelpMouse( void )			// open mouse command quick reference guide
{
	char mpath[MAX_PATH], *c, *pathend;

	GetModuleFileName( GetModuleHandle(NULL), mpath, MAX_PATH );

	c = mpath;									// mpath is location of application .exe
	while ( *c != '\0' ) {
		if ( *c == '\\' ) pathend = c;			// find last directory
		c++;
		}

	strcpy(pathend,"\\mousecmds.chm\0");		// specify path to help file

	/* if ( !HtmlHelp(appWnd, mpath, HH_DISPLAY_TOPIC, NULL ) )
		ErrMsgOK( ERRMSG_RUN_FAILED, mpath );
	*/
	// Since HtmlHelp fails the linking step, try another way (may require Reconstruct to be started in .exe folder)
    SpawnProcessSimple ( "hh mousecmds.chm" );
}

void CmHelpLicense( void )			// display info about GNU General Public License
{
	HWND hwndDlg;								// display message box for version info
	char txt[] = { " Reconstruct copyright \xA9 1996-2007 John C. Fiala.\r\n\
 Additional credits and copyrights: Andy Key, Thomas G. Lane, Ju Lu.\r\n\r\n\
 Development of Reconstruct was funded, in part, by the Human\r\n\
 Brain Project and the National Institutes of Health under grants\r\n\
 P30 HD18655, R01 MH/DA57351, R01 EB002170, R01 NS024760, R01\r\n\
 MH057414. Permission to use, copy, and redistribute Reconstruct is\r\n\
 granted without fee under the terms of the GNU General Public\r\n\
 License as published by the Free Software Foundation (www.gnu.org).\r\n\r\n\
 This program is distributed in the hope that it will be useful,\r\n\
 but WITHOUT ANY WARRANTY, including the implied warranty of\r\n\
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." };

	hwndDlg = CreateDialog( appInstance, "License", appWnd, (DLGPROC)WaitUntilOK  );

	SetDlgItemText( hwndDlg, ID_LICENSE, txt );
}

void CmHelpAbout( void )			// display about dialog box with version information			
{
	char FileName[MAX_PATH], versionText[MAX_PATH], *description, *copyright;
	DWORD dwVersionInfoSize, dwZero;
	UINT nBytesReturned;
	void *pVersionInfo;
	VS_FIXEDFILEINFO* pFixedFileInfo;
	HWND hwndDlg;														// display message box for version info

	hwndDlg = CreateDialogParam( appInstance, "About", appWnd, (DLGPROC)WaitTimeout, 15000  );

	GetModuleFileName( GetModuleHandle(NULL), FileName, MAX_PATH );		// get the application file name

	dwVersionInfoSize = GetFileVersionInfoSize( FileName, &dwZero );	// how big is the version info?
	if ( dwVersionInfoSize )
		{
		pVersionInfo = malloc(dwVersionInfoSize);						// allocate space to store version info

		if ( GetFileVersionInfo( FileName, 0, dwVersionInfoSize, pVersionInfo) )	// get version info from file
			{
			if ( VerQueryValue(pVersionInfo, "\\", (LPVOID *)&pFixedFileInfo, &nBytesReturned) )	// version #
				{
				sprintf(versionText, "version %d.%d.%d.%d", 
							HIWORD(pFixedFileInfo->dwFileVersionMS), LOWORD(pFixedFileInfo->dwFileVersionMS),
							HIWORD(pFixedFileInfo->dwFileVersionLS), LOWORD(pFixedFileInfo->dwFileVersionLS) );
				SendDlgItemMessage(hwndDlg, ID_VERSION, WM_SETTEXT, 0, (LPARAM)versionText);
				}
			if ( VerQueryValue(pVersionInfo, TEXT("\\StringFileInfo\\040904E4\\ProductName"),	// description
															(LPVOID *)&description, &nBytesReturned ) )   
				SendDlgItemMessage(hwndDlg, ID_DESCRIPTION, WM_SETTEXT, 0, (LPARAM)description);

			if ( VerQueryValue(pVersionInfo, TEXT("\\StringFileInfo\\040904E4\\LegalCopyright"),	// copyright
															(LPVOID *)&copyright, &nBytesReturned ) )   
				SendDlgItemMessage(hwndDlg, ID_COPYRIGHT, WM_SETTEXT, 0, (LPARAM)copyright);
			}
		free(pVersionInfo);												// free memory used for version info
		}
}
