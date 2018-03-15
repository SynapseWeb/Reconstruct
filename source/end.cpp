/////////////////////////////////////////////////////////////////////////////
//	This file contains the routines for closing the application
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
// modified 6/14/06 by JCF (fiala@bu.edu)
// -+- change: moved child window destructions into a separate routine so can call elsewhere.
//

#include "reconstruct.h"

bool SaveSeriesObjects( void )
{															// make sure everything is saved	
	if ( PrevSection )
		if ( !PrevSection->SaveIfNeeded() ) return false;
	if ( CurrSection )				
		if ( !CurrSection->SaveIfNeeded() ) return false;
	if ( CurrSeries )										// save series before deleting
		CurrSeries->SaveIfNeeded();
	return true;
}

void CloseAllChildWindows( void )					// destroy all subordinate windows
{
	if ( IsWindow( distanceWindow ) ) DestroyWindow( distanceWindow );
	distanceWindow = NULL;
	if ( IsWindow( thumbnailWindow ) ) DestroyWindow( thumbnailWindow );
	thumbnailWindow = NULL;
	if ( IsWindow( objectWindow ) ) DestroyWindow( objectWindow );
	objectWindow = NULL;
	if ( IsWindow( openGLWindow ) ) DestroyWindow( openGLWindow );
	openGLWindow = NULL;
	if ( IsWindow( sectionWindow ) ) DestroyWindow( sectionWindow );
	sectionWindow = NULL;
	if ( IsWindow( domainWindow ) ) DestroyWindow( domainWindow );
	domainWindow = NULL;
	if ( IsWindow( traceWindow ) ) DestroyWindow( traceWindow );
	traceWindow = NULL;
	if ( IsWindow( paletteWindow ) ) DestroyWindow( paletteWindow );
	paletteWindow = NULL;
	if ( IsWindow( zTraceWindow ) ) DestroyWindow( zTraceWindow );
	zTraceWindow = NULL;
}

void FreeSeriesObjects( void )						// clean up when closing series	
{
	DWORD status;

	if ( hRenderThread ) AbortRender = true;				// abort threads if active
	if ( hDistanceListThread ) AbortDistances = true;

	CloseAllChildWindows();									// close windows that contain series data

															// free viewports and NULL viewport ptrs
	if ( PrevView ) delete PrevView;
	PrevView = NULL;
	if ( CurrView ) delete CurrView;
	CurrView = NULL;
	if ( BlendView ) CmBlend();
	BlendView = NULL;
	if ( DomainView ) delete DomainView;
	DomainView = NULL;
	FrontView = NULL;
	BackView = NULL;
															// free section data and NULL section ptrs
	if ( DomainSection ) delete DomainSection;
	DomainSection = NULL;		
	if ( PrevSection ) delete PrevSection;
	PrevSection = NULL;
	if ( CurrSection ) delete CurrSection;
	CurrSection = NULL;
	
	CurrContour = NULL;										// clear other ptrs to deleted objects

	if ( CurrSectionsInfo )									// delete list of sections of series
		{
		delete CurrSectionsInfo;
		CurrSectionsInfo = NULL;
		}
	if ( CurrSeries )										// finally delete and clear series
		{
		delete CurrSeries;
		CurrSeries = NULL;
		}
	if ( CurrObjects )										// clear all objects
		{
		delete CurrObjects;
		CurrObjects = NULL;
		}
}

void FreeProgramObjects( void )				// free memory objects on program exit
{
	if ( debugLogFile != INVALID_HANDLE_VALUE ) CloseHandle( debugLogFile );
		
	DestroyCursor( StampCursor );			// destroy dynamically created cursor
	DeleteObject( lockBitmap );				// delete loaded bitmaps
	DeleteObject( unlockBitmap );
											// close any help files
	WinHelp(appWnd,NULL,HELP_QUIT,0L);
}
