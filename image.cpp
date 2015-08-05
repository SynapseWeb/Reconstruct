////////////////////////////////////////////////////////////////////////
//	This file contains the methods for the Image class
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
// modified 11/21/06 by JCF (fiala@bu.edu)
// -+- change: Modified LoadAdib to turn off Windows error dialogs while attempting to load.
// modified 3/15/07 by JCF (fiala@bu.edu)
// -+- change: Added color channel mask initializations.
// modified 4/2/07 by JCF (fiala@bu.edu)
// -+- change: Modified TestGBMfile to also return the number of images in an image stack
//

#include "reconstruct.h"

Image::Image()										// constructor
{
	mag = 1.0;
	contrast = 1.0;
	brightness = 0.0;
	red = true;
	green = true;
	blue = true;
	src[0]='\0';
	proxySrc[0]='\0';
	proxyScale = 1.0;
	loaded = NONE;
	adib = NULL;
}

Image::Image ( Image &copyfrom )					// copy constructor
{
	mag = copyfrom.mag;
	contrast = copyfrom.contrast;
	brightness = copyfrom.brightness;
	red = copyfrom.red;
	green = copyfrom.green;
	blue = copyfrom.blue;
	strcpy( src, copyfrom.src );
	strcpy( proxySrc, copyfrom.proxySrc );
	proxyScale = copyfrom.proxyScale;
	loaded = NONE;
	adib = NULL;									// don't copy actual adib memory!
}

Image::~Image()										// destructor
{
	if ( adib )
	 delete adib;						// delete the adib memory
}

BOOL CALLBACK MissingDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	OPENFILENAME  ofn;		  							// common dialog box structure
	char filename[MAX_PATH];

	switch (message)		// before calling dialog, InputDlgString was initialized to filename
		{
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg, ID_MISSINGMSG, WM_SETTEXT, 0, (LPARAM)InputDlgString);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case ID_BROWSESRC:						// initialize OPENFILENAME data
					strcpy( filename, InputDlgString );
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = appWnd;
					ofn.hInstance = appInstance;
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "All Files\0*.*\0Windows Bitmaps\0*.bmp\0TIFF Images\0*.tif\0GIF Images\0*.gif\0JPEG Images\0*.jpg\0\0";
					ofn.nFilterIndex = 0;
					ofn.lpstrFileTitle = NULL;
					ofn.lpstrTitle = "Find Missing File\0";
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

					if ( GetOpenFileName(&ofn) )					// get filename from dialog box
						{
						strcpy( InputDlgString, filename );
						EndDialog(hwndDlg, ID_BROWSESRC);
						}
					return TRUE;
				case ID_SKIPFILE:
					EndDialog(hwndDlg, ID_SKIPFILE);
					return TRUE;
				case ID_SKIPALL:
					EndDialog(hwndDlg, ID_SKIPALL);
					return TRUE;
				}
		}
	return FALSE;
}

bool Image::LoadAdib( bool useProxy, double pixel_size )	// load Adib from file
{														// base proxy load on flag and pixel_size
	int	id;
	UINT lastErrorMode;
	double psize;
	char basename[MAX_BASE], suffix[MAX_PATH], dir[MAX_PATH], filename[MAX_PATH];
// begin debug logging...
	DWORD byteswritten;
	char debugline[MAX_PATH];
	if ( debugLogFile != INVALID_HANDLE_VALUE )
		{
		sprintf(debugline,"Entered Image::LoadAdib\r\n");
		WriteFile(debugLogFile, debugline, strlen(debugline), &byteswritten, NULL);
		}
// ...end debug logging	
	lastErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);	// turn off Windows error dialogs
	switch ( loaded )									// determine if adib already loaded correctly
		{
		case SOURCE:
			if ( adib == NULL ) loaded = NONE;			// need to do full source load
			break;

		case SKIPPED:									// user has already said to skip loading
			break;

		case PROXY:										// is proxy appropriate at this resolution?
			psize = mag/proxyScale;						// compute (bigger) pixel_size of proxy
			if ( useProxy && (strlen(proxySrc)>0) && (proxyScale<1.0)
					&& ( psize < (pixel_size - mag) ) )	// if less than one full pixel difference
				if ( adib ) break;						// then OK to use it the loaded proxy image
			// else fall through to retry proxy load in case (adib==NULL) 

		default:
			loaded = NONE;								// reset to NONE here, may need full src load
			SetCurrentDirectory(ImagesPath);			// source path may be relative to images folder!
			psize = mag/proxyScale;						// compute (bigger) pixel_size of proxy
			if ( useProxy && (strlen(proxySrc)>0) && (proxyScale<1.0)
					&& ( psize < (pixel_size - mag) ) )	// if less than one full pixel difference
				{
				if ( adib ) delete adib;				// attempt proxy load
				adib = new ADib( proxySrc );
				if ( adib->bits ) loaded = PROXY;		// successful proxy loaded
				}
		   // if reach this point, will need to try full src load...
		}

	while ( loaded == NONE )								// keep trying until found or skipped
		{
		if ( adib ) delete adib;							// throw out existing proxy adib
		adib = new ADib( src );								// try to load full sized image
		if ( adib->bits ) loaded = SOURCE;
		else												// else failed, show missing file dialog
			{
			if ( adib ) delete adib;						// throw out existing adib from above failed attempt
			adib = NULL;									// set to NULL so will correctly fail in ADib
			if ( SkipAllMissing ) loaded = SKIPPED;			// if user never wants to find it, then done
			else
				{											// ask user to find file
				strcpy( InputDlgString, src );
				id = DialogBox( appInstance, "MissingDlg", appWnd, (DLGPROC)MissingDlgProc );
				if ( id == ID_SKIPALL ) SkipAllMissing = true;
				if ( (id == ID_SKIPALL) || (id == ID_SKIPFILE) ) loaded = SKIPPED;
				}
			if ( loaded != SKIPPED )						// if user chooses skip, then return w/o loading
				{
				SplitPath(InputDlgString,dir,filename,suffix);// user found a new filepath! parse it
				if ( SetCurrentDirectory(dir) )
					{										// try new path as default images path
					strcpy(ImagesPath,dir);
					MakeLocalPath(ImagesPath,InputDlgString,src);
					}
				else strcpy(src,InputDlgString);			// otherwise, just use user's filepath
				}
			}
		}

	SetErrorMode( lastErrorMode );							// restore Windows error dialogs
	if ( (loaded == SOURCE) || (loaded == PROXY) ) return true;
	else return false;										// failure if no image is available
}

bool Image::HasProxy( void )							// return true if image has proxy
{
	int ft, w, h, bpp, n;
	if ( strlen(proxySrc) && (proxyScale < 1.0) )		// there is a reference to a proxy file
		{
		ft = TestGBMfile( proxySrc, &w, &h, &bpp, &n );
		if ( ft ) return true;							// but file must really exist and be known format!
		}
	return false;
}

void Image::MakeProxy( double scale )					// from full adib, make scaled version and save to file
{
	ADib *proxy;
	char suffix[MAX_PATH], dir[MAX_PATH], file[MAX_PATH];

	if ( LoadAdib( false, 0.0 ) )						// pixel_size is irrelevant becuz no proxy read allowed
		{
		proxy = new ADib( *adib, scale );				// create scaled replica
		proxy->ConvertUpTo24bit();						// JPEG saves only in 24-bit color
		proxyScale = scale;
		MakeLocalPath(ImagesPath,src,proxySrc);			// get path relative to ImagesPath
		strcat( proxySrc, "_proxy.jpg" );				// tack on proxy suffix to make unique name	
		proxy->SaveADibAsJPEG( proxySrc, 70 );			// save with moderate compression
		delete proxy;
		}
}

void Image::DeleteProxy( void )							// remove proxy reference and file
{
	SetCurrentDirectory(ImagesPath);					// proxy path is relative to images folder!
	DeleteFile( proxySrc );	
	proxySrc[0] = '\0';
	proxyScale = 1.0;
}
