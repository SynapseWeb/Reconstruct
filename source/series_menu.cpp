/////////////////////////////////////////////////////////////////////////////
//	This file contains the routines for processing the series commands
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
// -+- change: Fixed MAX_CONTOUR_NAME string limit in Names/Colors series options dialog procedure.
// -+- change: Moved simplification parameters to new AutoTracing tab.
// -+- change: Added AutoTracing dialog procedure and tab entry in series options.
// modified 11/12/04 by JCF (fiala@bu.edu)
// -+- change: Switched wildfire stop criteria to HSB instead of RGB.
// modified 11/23/04 by JCF (fiala@bu.edu)
// -+- change: Fixed bug in Names/Colors options palette and pushed palette button in Palette Window when OK.
// modified 2/03/05 by JCF (fiala@bu.edu)
// -+- change: Added shrink back option to Autotracing tab.
// modified 2/15/05 by JCF (fiala@bu.edu)
// -+- change: Added section number field to Export Trace Lists
// modified 3/10/05 by JCF (fiala@bu.edu)
// -+- change: Added GRID_ELLIPSE element type to Grid Tool.
// modified 4/29/05 by JCF (fiala@bu.edu)
// -+- change: Added Import Lines... for DXF file import of section traces.
//             Added wildfire Stop At Traces option to AutoTracing tab.
// modified 6/17/05 by JCF (fiala@bu.edu)
// -+- change: Added series options stop params for wildfire autotracing and region wildfire.
//             Doubled size of import filename buffers in ImportImages and ImportLines.
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change: Modified RenderSections() to use new SectionInfo for initial image sizing.
// modified 9/13/05 by JCF (fiala@bu.edu)
// -+- change: Added UnPushPaletteButtons() when change default contour profile.
// modified 11/7/05 by JCF (fiala@bu.edu)
// -+- change: Make sure autonumbering (-1) in import images is non-negative.
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Check for saving memory sections before export images.
// modified 3/15/06 by JCF (fiala@bu.edu)
// -+- change: Made File Select dialog modal to Import Images dialog.
// modified 5/4/06 by JCF (fiala@bu.edu)
// -+- change: Added smoothingLength parameter to Autotracing tab of Series Options,
//      and OffsetZtrace's to 3D tab for section-by-section smoothing when reconstructing.
// modified 5/11/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added ContourMaskWidth option to mask out a strip along existing contours for autotracing. 
// modified 5/17/06 by JCF (fiala@bu.edu)
// -+- change: Fixed delay when open proxies tab. Modified Palette editing in Names/Colors tab.
// modified 5/25/06 by JCF (fiala@bu.edu)
// -+- change: Added DXF export command CmExportLines()
// modified 6/14/06 by JCF (fiala@bu.edu)
// -+- change: Fixed minor bugs in progress reports of Count/Create/Delete Proxies.
// -+- change: Added code to CloseAllChildWindows before import dialog boxes are opened -- needed?
// modified 6/19/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added automatic adjustment of threshold in AutoTracingDlgProc
// modified 7/3/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added multi-autotrace choice in AutoTracingDlgProc
// modified 11/15/06 by JCF (fiala@bu.edu)
// -+- change: Removed MultiAutotrace since now all autotrace is multi when multiple traces selected.
// modified 11/22/06 by JCF (fiala@bu.edu)
// -+- change: Limit CurrSeries->skipSections to be greater than 0.
// -+- change: Fixed Import Series to generate relative paths if requested.
// modified 11/24/06 by JCF (fiala@bu.edu)
// -+- change: Added proxySrc relative/absolute path to Import Series.
// modified 4/2/07 by JCF (fiala@bu.edu)
// -+- change: Modified TestGBMfile to also return the number of images in an image stack
// modified 4/3/07 by JCF (fiala@bu.edu)
// -+- change: Fixed bug in counting proxies.
// -+- change: Fixed errors trying to copy nonexistent proxy images and wrong paths when copying in Absolute path mode.
// -+- change: Added GIF and TIFF stack processing to Import Images, with modifications to listview fields.
// modified 4/4/07 by JCF (fiala@bu.edu)
// -+- change: Altered Import Images to PageTo first section of import automatically.
// modified 4/13/07 by JCF (fiala@bu.edu)
// -+- change: Put test for CurrSeries before open dialog in CmSetOptions()
// modified 7/27/07 by JCF (fiala@bu.edu)
// -+- change: Switched Import Images file selection dialog to start with All Files listing.

#include "reconstruct.h"

void EnableSeriesMenus( void )				// switch menu state when series is open
{
	HMENU hMenu, hSeriesMenu;
	int i;

	hMenu = GetMenu( appWnd );						// enable all menus
	i = GetMenuItemCount( hMenu );
	do { i--; EnableMenuItem( hMenu, i, MF_BYPOSITION | MF_ENABLED ); }
	while ( i > 0 );
	DrawMenuBar( appWnd );							// redraw so will be enabled
	hSeriesMenu = GetSubMenu( hMenu, 1 );			// now enable all of series menu
	i = GetMenuItemCount( hSeriesMenu );
	do { i--; EnableMenuItem( hSeriesMenu, i, MF_BYPOSITION | MF_ENABLED ); }
	while ( i > 0 );
}

void DisableSeriesMenus( void )				// switch menu state to series closed format
{
	HMENU hMenu, hSeriesMenu;
	int i;

	hMenu = GetMenu( appWnd );					// disable most menus
	for ( i=2; i<=5; i++ ) EnableMenuItem( hMenu, i, MF_BYPOSITION | MF_GRAYED );

	hSeriesMenu = GetSubMenu( hMenu, 1 );		// and most of series menu
	i = GetMenuItemCount( hSeriesMenu );
	do { i--; EnableMenuItem( hSeriesMenu, i, MF_BYPOSITION | MF_GRAYED ); }
	while ( i > 0 );
	DrawMenuBar( appWnd );						// flush this menu change
												// enable open or creating new series
	EnableMenuItem( hSeriesMenu, CM_OPEN, MF_BYCOMMAND | MF_ENABLED );
	EnableMenuItem( hSeriesMenu, CM_NEWSERIES, MF_BYCOMMAND | MF_ENABLED );								
}

void CmClose( void )						// Close current study
{
	if ( !CurrSeries ) return;					// do nothing if no series is open
	if ( !SaveSeriesObjects() ) return;			// try to save sections, user may cancel
	FreeSeriesObjects();						// free and NULL all memory objects
	UpdateTitles();								// clear title caption
	DisableSeriesMenus();						// disable menus
	InvalidateRect( appWnd, NULL, TRUE );		// clear screen
}

void ValidOpen( char * seriesPath )			// validate series file path and open
{
	Series *testSeries;
	int sectnum;
	char basename[MAX_BASE], suffix[MAX_PATH], dir[MAX_PATH], filename[MAX_PATH];

	testSeries = new Series( seriesPath );		// try reading file
	if ( testSeries->index < 0 )				// problem with Series data
		{
		delete testSeries;						// give up right away
		return;
		}
	SplitPath(seriesPath,dir,filename,suffix);	// try parsing filename
	strncpy(basename,filename,MAX_BASE-1);		// get basename, make sure it fits
	basename[MAX_BASE] = '\0';
	
	if ( !SaveSeriesObjects() )					// try to save old series stuff
		{
		delete testSeries;						// give up if something isn't saved
		return;
		}	
	if ( SetCurrentDirectory(dir) )				// must be able to set to this directory
		{
		FreeSeriesObjects();					// delete objects and reset object ptrs
		CurrSeries = testSeries;				// we're really gonna open now, so keep data
		strcpy(BaseName,basename);				// remember basename of series
		strcpy(WorkingPath,dir);				// remember path to series & section files
		strcpy(ImagesPath,dir);					// remember path to image source files
		InitCurrSeries();						// initialize stuff before using it
		EnableSeriesMenus();					// turn-on menu items
		InvalidateRect( appWnd, NULL, TRUE );	// clear screen

		sectnum =  CurrSeries->index;					// now goto the section in series
		if ( sectnum >= 0 )
			if ( FindSection( sectnum, 0, filename ) )	
				{
				if ( GotoSection( sectnum, filename ) )	// load it and display it
					return;
				}
		CurrSeries->index = 0;		// IF GOTO FAILS, THEN SEARCH UP/DOWN AND THEN PRINT ERROR?
		UpdateTitles();								// no sections or error reading first one
		}		
	else {
		delete testSeries;
		ErrMsgOK( ERRMSG_OPEN_SERIES, seriesPath );	// change to dir failed!
		}
}

void CmOpen( void )						// Open a series menu command
{
	OPENFILENAME	ofn;								// common dialog box structure
	char srcname[MAX_PATH];

	strcpy(srcname,"*.ser\0");							// the series file will be "seriesname.ser"

	ZeroMemory(&ofn, sizeof(ofn));						// clear the OPENFILENAME fields
	ofn.lStructSize = sizeof(ofn);						// set only the necessary values
	ofn.hwndOwner = appWnd;
	ofn.lpstrFile = srcname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = "All Files\0*.*\0Series files\0*.ser\0\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrTitle = "Open Series";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;	// allow only existing writeable files
	ofn.lpstrDefExt = "ser";							// append "ser" to the file name if not extension

	if ( GetOpenFileName(&ofn) == TRUE )				// display the Open dialog box.
		ValidOpen(srcname);								// open series if srcname is valid
}

void CmNewSeries( void )								// Create a series menu command
{
	OPENFILENAME	ofn;								// common dialog box structure
	char srcname[MAX_PATH];
	Series *newSeries;									// for empty series

	strcpy(srcname,"newSeries\0");						// the series file will be "newSeries.ser"

	ZeroMemory(&ofn, sizeof(ofn));						// clear the OPENFILENAME fields
	ofn.lStructSize = sizeof(ofn);						// set only the necessary values
	ofn.hwndOwner = appWnd;
	ofn.lpstrFile = srcname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = "All Files\0*.*\0Series files\0*.ser\0\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrTitle = "New Series";
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY; // ask user if overwrite desired
	ofn.lpstrDefExt = "ser";							// append "ser" to the file name if not extension

	if ( GetSaveFileName(&ofn) == TRUE )				// display the Open dialog box
		{
		newSeries = new Series();						// create empty newSeries
		newSeries->Save( srcname );						// attempt to write to file
		delete newSeries;								// discard
		ValidOpen(srcname);								// open series if srcname is valid
		}
}

void CmSaveSeries( void )			// Save series
{
	char filename[MAX_PATH];

	sprintf(filename,"%s%s.ser",WorkingPath,BaseName);
	CurrSeries->Save( filename );
}

void UpdateRenderDialog( HWND hwndDlg )			// local routine to simplify RenderSectionsProc
{
	char dlgItemTxt[128];
	RECT r;
	double bpp, megabytes, width, height, min_pixel_size;

	if ( IsDlgButtonChecked( hwndDlg, ID_WINDOW ) == BST_CHECKED )
		{
		RenderWindow = true;
		GetClientRect( appWnd, &r );
		width = r.right - r.left;
		height = r.bottom - r.top;
		UsePixelSize = CurrSeries->pixel_size;
		}											// width*height must fit in an int
	else {
		RenderWindow = false;
		GetDlgItemText( hwndDlg, ID_PIXELSIZE, dlgItemTxt, sizeof(dlgItemTxt) );
		UsePixelSize = atof(dlgItemTxt);
		min_pixel_size = sqrt(XLength*YLength/(double)MAX_INT32);
		if ( UsePixelSize <= min_pixel_size ) UsePixelSize = FullPixelSize;
		width = XLength/UsePixelSize;
		height = YLength/UsePixelSize;
		}
	bpp = 3.0;
	megabytes = width*height*bpp/1048576.0;
	sprintf(dlgItemTxt,"Image size: %d x %d pixels  %.2f Mb (uncompressed)",(int)width,(int)height,megabytes);
	SetDlgItemText( hwndDlg, ID_WIDTHHEIGHT, dlgItemTxt );
}

BOOL CALLBACK RenderSectionsProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	char dlgItemTxt[64];

	switch (message)
		{
		case WM_INITDIALOG:
			sprintf(dlgItemTxt,"%d",FirstSection);
			SetDlgItemText( hwndDlg, ID_FIRSTSECTION, dlgItemTxt );
			sprintf(dlgItemTxt,"%d",LastSection);
   			SetDlgItemText( hwndDlg, ID_LASTSECTION, dlgItemTxt );
			sprintf(dlgItemTxt,"%1.*g",Precision,FullPixelSize);
   			SetDlgItemText( hwndDlg, ID_PIXELSIZE, dlgItemTxt );
			if ( RenderWindow )
				CheckRadioButton( hwndDlg, ID_WINDOW, ID_FULL, ID_WINDOW );
			else CheckRadioButton( hwndDlg, ID_WINDOW, ID_FULL, ID_FULL );
			if ( RenderJPEG ) CheckDlgButton( hwndDlg, ID_JPEG, BST_CHECKED );
			else CheckDlgButton( hwndDlg, ID_BITMAP, BST_CHECKED );
			sprintf(dlgItemTxt,"%d",JPEGquality);
   			SetDlgItemText( hwndDlg, ID_JPEGQUALITY, dlgItemTxt );
			if ( RenderFill )
				CheckRadioButton( hwndDlg, ID_NOTRACES, ID_COLORIZE, ID_COLORIZE );
			else if ( RenderTraces )
				CheckRadioButton( hwndDlg, ID_NOTRACES, ID_COLORIZE, ID_TRACES );
			else CheckRadioButton( hwndDlg, ID_NOTRACES, ID_COLORIZE, ID_NOTRACES );
			SetDlgItemText( hwndDlg, ID_DESTNAME, SeriesName );
			UpdateRenderDialog( hwndDlg );
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case ID_FULL:
					sprintf(dlgItemTxt,"%1.*g",Precision,FullPixelSize);
   					SetDlgItemText( hwndDlg, ID_PIXELSIZE, dlgItemTxt );
					CheckRadioButton( hwndDlg, ID_WINDOW, ID_FULL, ID_FULL );
					UpdateRenderDialog( hwndDlg );
					break;
				case ID_WINDOW:
					sprintf(dlgItemTxt,"%1.*g",Precision,CurrSeries->pixel_size);
   					SetDlgItemText( hwndDlg, ID_PIXELSIZE, dlgItemTxt );
					CheckRadioButton( hwndDlg, ID_WINDOW, ID_FULL, ID_WINDOW );
					UpdateRenderDialog( hwndDlg );
					break;
				case ID_PIXELSIZE:
					switch (HIWORD(wParam)) {
					case EN_CHANGE:
						CheckRadioButton( hwndDlg, ID_WINDOW, ID_FULL, ID_OTHER );
						UpdateRenderDialog( hwndDlg );
						break;
					case EN_KILLFOCUS:	// in case need to reset UsePixelSize to reasonable value
						sprintf(dlgItemTxt,"%1.*g",Precision,UsePixelSize);
   						SetDlgItemText( hwndDlg, ID_PIXELSIZE, dlgItemTxt );
						UpdateRenderDialog( hwndDlg );
						}
					break;
				case ID_JPEGQUALITY:
					switch (HIWORD(wParam)) {
					case EN_KILLFOCUS:
						GetDlgItemText( hwndDlg, ID_JPEGQUALITY, dlgItemTxt, sizeof(dlgItemTxt) );
						JPEGquality = atoi(dlgItemTxt);
						if ( JPEGquality < 1 ) JPEGquality = 1;
						if ( JPEGquality > 100 ) JPEGquality = 100;
						sprintf(dlgItemTxt,"%d",JPEGquality);
   						SetDlgItemText( hwndDlg, ID_JPEGQUALITY, dlgItemTxt );
						}
					break;
				case IDOK:
					UpdateRenderDialog( hwndDlg );
					if ( IsDlgButtonChecked( hwndDlg, ID_JPEG ) == BST_CHECKED )
						RenderJPEG = true;
					else RenderJPEG = false;
					if ( IsDlgButtonChecked( hwndDlg, ID_TRACES ) == BST_CHECKED )
						RenderTraces = true;
					else RenderTraces = false;
					if ( IsDlgButtonChecked( hwndDlg, ID_COLORIZE ) == BST_CHECKED )
						RenderFill = true;
					else RenderFill = false;
   					GetDlgItemText( hwndDlg, ID_FIRSTSECTION, dlgItemTxt, sizeof(dlgItemTxt) );
					FirstSection = atoi(dlgItemTxt);
   					GetDlgItemText( hwndDlg, ID_LASTSECTION, dlgItemTxt, sizeof(dlgItemTxt) );
					LastSection = atoi(dlgItemTxt);
					GetDlgItemText( hwndDlg, ID_DESTNAME, SeriesName, sizeof(SeriesName) );
					EndDialog(hwndDlg, 1);
					return TRUE;
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				}
		}
	return FALSE;
}

void CmRenderSections( void )							// render sections onto bitmaps
{
	SectionInfo *sectioninfo;
	Section *section;
	int sectnum;								
	Point min, max;
	double min_mag_value;
	bool render;								// first, determine params for dialog

	min.x = MAX_FLOAT; min.y = MAX_FLOAT; max.x = -MAX_FLOAT; max.y = -MAX_FLOAT;
	min_mag_value = MAX_FLOAT;

	FirstSection = 0;
	LastSection = 0;
	sectioninfo = CurrSectionsInfo->first;
	if ( sectioninfo ) FirstSection = sectioninfo->index;
	while ( sectioninfo )									// look for exterme values across sections
		{
		LastSection = sectioninfo->index;
		if ( sectioninfo->min.x < min.x ) min.x = sectioninfo->min.x;	// accummulate min, max
		if ( sectioninfo->min.y < min.y ) min.y = sectioninfo->min.y;
		if ( sectioninfo->max.x > max.x ) max.x = sectioninfo->max.x;
		if ( sectioninfo->max.y > max.y ) max.y = sectioninfo->max.y;
		if ( sectioninfo->pixelsize < min_mag_value ) min_mag_value = sectioninfo->pixelsize;
		sectioninfo = sectioninfo->next;
		}

	if ( min.x > max.x )									// if no images, then abandon operation
		{
		ErrMsgOK( ERRMSG_NOSECTIONS, NULL );
		return;
		}

	XLength = (max.x - min.x);								// set globals for dialog and thread			
	YLength = (max.y - min.y);
	XOffset = min.x;
	YOffset = min.y;
	FullPixelSize = min_mag_value;							// this will render every pixel

	sprintf(SeriesName,"%s_aligned",BaseName);				// ask user for parameters of rendering
														
	if ( DialogBox( appInstance, "RenderSections", appWnd, (DLGPROC)RenderSectionsProc ) )
		{
		render = true;
		if ( strcmp(SeriesName,BaseName) == 0 )				// don't allow overwrite of series
			{
			ErrMsgOK( ERRMSG_NEW_SERIES, SeriesName );
			render = false;
			}												// save sections before exporting
		if ( PrevSection )
			if ( !PrevSection->SaveIfNeeded() ) render = false;	
		if ( CurrSection )				
			if ( !CurrSection->SaveIfNeeded() ) render = false;

		if ( render ) RenderThreadBegin();					// invoke bkgnd thread to do the rendering
		}
}


void CmExportTraceLists( void )				// generate and write out all trace lists into one file
{
	Section *section;								
	SectionInfo *sectioninfo;
	LV_ITEM lvi;
	LV_COLUMN lvc;
	HWND hList;
	OPENFILENAME ofn;							// common dialog box structure
 	HANDLE hFile = NULL;
	DWORD byteswritten;
	bool ErrorOccurred = false;
	bool firstOutput = true;
	int i, sectnum;
	char *l, line[11*MAX_CONTOUR_NAME+MAX_COMMENT], errmsg[1024];
	char txt[MAX_COMMENT], filename[MAX_PATH];

	sprintf(filename,"traceLists.csv");
	ZeroMemory(&ofn, sizeof(ofn));						// clear the OPENFILENAME fields
	ofn.lStructSize = sizeof(ofn);						// set only the necessary values
	ofn.hwndOwner = appWnd;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = "All Files\0*.*\0Comma Separated Value file\0*.csv\0\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrTitle = "Save As...";
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY; // ask user if overwrite desired
	if ( GetSaveFileName(&ofn) == TRUE )				// display the Open dialog box
	  {
	  sectioninfo = CurrSectionsInfo->first;			// fill trace lists from sectioninfo sections
	  while ( sectioninfo )
		{
		sectnum = sectioninfo->index-1;					// determine max section range
		section = GetNextSectionBetween( sectnum, MAX_INT32 );
		if ( section )
			{
			hList = CreateWindow( WC_LISTVIEW, "", WS_CHILD | LVS_REPORT | LVS_NOSORTHEADER | LVS_SORTASCENDING,
												5, 5, 100, 100, appWnd, NULL, appInstance, NULL);
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; 
			lvc.fmt = LVCFMT_LEFT; lvc.pszText = txt;						// setup columns
			lvc.iSubItem = 0; strcpy(txt,"Trace"); lvc.cx = 100;			// set the column sizes to fit
			SendMessage(hList, LVM_INSERTCOLUMN, lvc.iSubItem, (LPARAM)(&lvc));
			FillTraceList( hList, section );

			if ( firstOutput )										// first section, open the output file and
				{													// write a header line to indicate the columns
				firstOutput = false;
				hFile = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
				if ( hFile == INVALID_HANDLE_VALUE )
					{														// open failed, format error string
					FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
					NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					errmsg, 512, NULL );
					strcat(errmsg, filename );
					ErrMsgOK( ERRMSG_WRITE_FAILED, errmsg );
					DestroyWindow( hList );
					return;
					}
																		// retreive column header info from listview
				l = line;												// format into header with fixed width fields
				l += sprintf(l,"section,");							// first column is section number
				i = 0;
				lvc.mask = LVCF_TEXT; lvc.pszText = txt; lvc.cchTextMax = MAX_COMMENT;
				while ( SendMessage(hList, LVM_GETCOLUMN, i, (LPARAM)(&lvc) ) )
					{
					l += sprintf(l,"%s,",txt);
					i++;
					}													// write header line
				sprintf(l,"\r\n");
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
				}

			lvi.iItem = SendMessage(hList, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL) );
			while ( lvi.iItem >= 0 )									// now do all items until done
				{
				l = line;												// format the line string with listview items
				l += sprintf(l,"%d,",section->index);
				lvi.mask = LVIF_TEXT; lvi.cchTextMax = MAX_COMMENT; lvi.pszText = txt;
				lvi.iSubItem = 0;
				while ( lvi.iSubItem < i )								// retrieve same i columns as in header
					{
					SendMessage(hList, LVM_GETITEM, 0, (LPARAM)(&lvi));	// get the field text info
					l += sprintf(l,"%s,",txt);							// add it to the line
					lvi.iSubItem++;
					}
				sprintf(l,"\r\n");										// write trace data line...

				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

				lvi.iItem = SendMessage(hList, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL) );
				}

			DestroyWindow( hList );										// get rid of the hidden list window
			PutSection( section, false, false, false );					// and the section data
			}
		sectioninfo = sectioninfo->next;							// do next section in info list
		}

	  CloseHandle( hFile );						// done all section, close output file and report any error
	  if ( ErrorOccurred )
			ErrMsgOK( ERRMSG_WRITE_FAILED, filename );
	  else ErrMsgOK( ERRMSG_FILEWRITTEN, filename );
	  }		
}

void CmExportLines( void )				// generate and write out all traces from each section
{
	Section *section;								
	SectionInfo *sectioninfo;
	Transform *transform;
	Contour *contour, *ncontour;
	Point *p;
	HCURSOR cur;
 	HANDLE hFile;
	DWORD byteswritten;
	bool ErrorOccurred = false;
	int i, c, sectnum;
	double x, y, z;
	char line[MAX_CONTOUR_NAME+MAX_COMMENT], errmsg[1024], filename[MAX_PATH];
	char *msg = "Export lines creates a separate DXF file for each section.";

	FirstSection = CurrSectionsInfo->first->index;			// ask user for section range
	LastSection = CurrSectionsInfo->last->index;
	if ( DialogBoxParam( appInstance, "SectionRange", appWnd, (DLGPROC)SectionRangeDlgProc, (LPARAM)msg ) == IDOK )
		{
		cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow
		
		sectnum = FirstSection - 1;
		section = GetNextSectionBetween( sectnum, LastSection );	// fetch section
		while ( section )
			{
			sprintf(filename,"%s.%d.dxf",BaseName,sectnum);
																		// create file
			hFile = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
			if ( hFile == INVALID_HANDLE_VALUE )
				{														// open failed, format error string
				FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				errmsg, 512, NULL );
				strcat(errmsg, filename );
				ErrMsgOK( ERRMSG_WRITE_FAILED, errmsg );
				PutSection( section, false, false, false );				// free section data
				return;
				}
																		// write header line
			sprintf(line,"0\r\nSECTION\r\n2\r\nENTITIES\r\n");
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

			if ( section->transforms )								// now write every contour in section
				{
				z = CurrSectionsInfo->ZDistance( sectnum, CurrSeries->zMidSection );
				transform = section->transforms->first;
				while ( transform )									// for each transform do contours
					{
					if ( transform->contours )
						{
						contour = transform->contours->first;		// retrieve first contour
						while ( contour )
							{
							if ( !contour->hidden && contour->points ) 	// for each visible contour...
								{
								ncontour = new Contour( *contour );		// ...copy it
								ncontour->InvNform( transform->nform );	// ...transform into section coord.
								i = 0; if ( ncontour->closed ) i = 1;	// ...set open or closed
								c = (int)floor(7*ncontour->border.r);	// ...set ~color index
								c = (c << 3) + (int)floor(7*ncontour->border.g);
								c = (c << 2) + (int)floor(3*ncontour->border.b);
								sprintf(line,"0\r\nPOLYLINE\r\n62\r\n%d\r\n66\r\n1\r\n70\r\n%1d\r\n",c,i);
								if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
								sprintf(line,"3\r\n%s\r\n",ncontour->name);
								if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
								p = ncontour->points->first;
								while ( p )								// ...write it to file
									{				
									sprintf(line,"0\r\nVERTEX\r\n10\r\n%1.*g\r\n20\r\n%1.*g\r\n30\r\n%1.*g\r\n",
																			Precision,p->x,Precision,p->y,Precision,z);
									if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
									p = p->next;
									}
								sprintf(line,"0\r\nSEQEND\r\n");		// end polyline description
								if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
								delete ncontour;
								}
							contour = contour->next;
							}
						}
					transform = transform->next;
					}
				}

			sprintf(line,"0\r\nENDSEC\r\n0\r\nEOF\r\n");				// end of dxf file
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
			CloseHandle( hFile );										// close output file
			PutSection( section, false, false, false );					// free section data
			section = GetNextSectionBetween( sectnum, LastSection );	// fetch next section
			}

		SetCursor( cur );									// undo hour glass

		if ( ErrorOccurred )								// report any error
			ErrMsgOK( ERRMSG_WRITE_FAILED, "" );
		else ErrMsgOK( ERRMSG_FILEWRITTEN, "" );
		}
}


int CompareAsNumbers( char *s1, char *s2 )			// return 0 if same
{
	int i, j, k;
	char digits1[MAX_PATH], digits2[MAX_PATH];
	
	i = strlen(s1); j = 0;							// pull out digits in s1
	while ( i >= 0 )
		{
   		if ( isdigit(s1[i]) ) { digits1[j] = s1[i]; j++; }
		i--;
		}
	i = strlen(s2); k = 0;							// pull out digits in s1
	while ( i >= 0 )
		{
   		if ( isdigit(s2[i]) ) { digits2[k] = s2[i]; k++; }
		i--;
		}
													// pad shorter string with zeros
	if ( j < k )
		while ( j < k ) { digits1[j] = '0'; j++; }
	else if ( k < j )
		while ( k < j ) { digits2[k] = '0'; k++; }

	i = j-1;										// compare digits, starting with most sig.
	while ( i >= 0 )
		{
		if ( digits1[i] < digits2[i] ) return -1;	// string s2 is bigger integer
		if ( digits1[i] > digits2[i] ) return 1;	// string s1 is bigger integer
		i--;
		}
   return 0;										// strings are numerically identical
}
												// sort listview either alphabetically or numerically
void SortItems( HWND list, bool alphabetically )
{
	LV_ITEM lvi;
	int i, j, s, index, sindex, total;
	bool moved;
	char sortitem[MAX_PATH], sorttype[MAX_PATH], sortindex[MAX_PATH], listitem[MAX_PATH];

	lvi.mask = LVIF_TEXT; lvi.state = 0; lvi.stateMask = 0; lvi.cchTextMax = MAX_PATH;

	total = SendMessage(list, LVM_GETITEMCOUNT, 0, 0 );
	
	for (i=1; i<total; i++)						// get each item in list
		{
		lvi.iItem = i; lvi.iSubItem = 0; lvi.pszText = sortindex;
		SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
		sindex = atoi(sortindex);
		lvi.iItem = i; lvi.iSubItem = 1; lvi.pszText = sortitem;
		SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
		lvi.iItem = i; lvi.iSubItem = 2; lvi.pszText = sorttype;
		SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
		j = 0; moved = false;
		while ( (j<i) && !moved )				// compare with previous items in list
			{
			lvi.iItem = j; lvi.iSubItem = 0; lvi.pszText = listitem;
			SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
			index = atoi( listitem );
			lvi.iItem = j; lvi.iSubItem = 1; lvi.pszText = listitem;
			SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
			if ( alphabetically ) s = strcmp(sortitem,listitem);
			else s = CompareAsNumbers(sortitem,listitem);
			if ( (s < 0) ||								// it may belong here (at position j)
		        ((s==0) && (sindex < index)) ) // check the subimage index also
				{
				moved = true;
				SendMessage(list, LVM_DELETEITEM, i, 0 );
				lvi.iItem = j; lvi.iSubItem = 0; lvi.pszText = sortindex;
				SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)(&lvi));
				lvi.iSubItem = 1; lvi.pszText = sortitem;
				SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
				lvi.iSubItem = 2; lvi.pszText = sorttype;
				SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
				}   
			j++;
			}
		}
}

void Randomize( HWND list )
{
	LV_ITEM lvi;
	int i, j, total;
	char randitem[MAX_PATH], randtype[MAX_PATH], randindex[MAX_PATH];
	char listitem[MAX_PATH], listtype[MAX_PATH], listindex[MAX_PATH];

	lvi.mask = LVIF_TEXT; lvi.state = 0; lvi.stateMask = 0; lvi.cchTextMax = MAX_PATH;

	total = SendMessage(list, LVM_GETITEMCOUNT, 0, 0 );
	for (i=0; i<total; i++)
		{
		j = rand()%total;						// get random j in [0,total-1]
      											// swap item j with item i

		lvi.iItem = i; lvi.iSubItem = 0; lvi.pszText = listindex;
		SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
		lvi.iItem = i; lvi.iSubItem = 1; lvi.pszText = listitem;
		SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
		lvi.iItem = i; lvi.iSubItem = 2; lvi.pszText = listtype;
		SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
		lvi.iItem = j; lvi.iSubItem = 0; lvi.pszText = randindex;
		SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
		lvi.iItem = j; lvi.iSubItem = 1; lvi.pszText = randitem;
		SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
		lvi.iItem = j; lvi.iSubItem = 2; lvi.pszText = randtype;
		SendMessage(list, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
		lvi.iItem = i; lvi.iSubItem = 0; lvi.pszText = randindex;
		SendMessage(list, LVM_SETITEMTEXT, i, (LPARAM)(&lvi));
		lvi.iSubItem = 1; lvi.pszText = randitem;
		SendMessage(list, LVM_SETITEMTEXT, i, (LPARAM)(&lvi));
		lvi.iSubItem = 2; lvi.pszText = randtype;
		SendMessage(list, LVM_SETITEMTEXT, i, (LPARAM)(&lvi));
		lvi.iItem = j; lvi.iSubItem = 0; lvi.pszText = listindex;
		lvi.iItem = SendMessage(list, LVM_SETITEMTEXT, j, (LPARAM)(&lvi));
		lvi.iSubItem = 1; lvi.pszText = listitem;
		SendMessage(list, LVM_SETITEMTEXT, j, (LPARAM)(&lvi));
		lvi.iSubItem = 2; lvi.pszText = listtype;
		SendMessage(list, LVM_SETITEMTEXT, j, (LPARAM)(&lvi));
		}
}
													// adjust sectnum and placement fields
void CompleteImportList( HWND hwndDlg )				// based on the current dialog parameters
{
	LV_ITEM lvi;
	int i, j, sectnum, incr;
	unsigned int k;
	double x, y;
	char txt[MAX_PATH], suffix[MAX_PATH], dir[MAX_PATH], file[MAX_PATH];

	SendDlgItemMessage(hwndDlg, ID_XOFFSET, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
	x = atof(txt);
	SendDlgItemMessage(hwndDlg, ID_YOFFSET, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
	y = atof(txt);
	SendDlgItemMessage(hwndDlg, ID_FIRSTSECTION, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
	sectnum = atoi(txt);
	SendDlgItemMessage(hwndDlg, ID_INCREMENTBY, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
	incr = atoi(txt);
	lvi.mask = LVIF_TEXT; lvi.state = 0; lvi.stateMask = 0; lvi.cchTextMax = MAX_PATH;

	lvi.iItem = SendMessage(importList, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL) );
	i = sectnum;
	while ( lvi.iItem >= 0 )
		{														// for each item in list
		lvi.iSubItem = 1; lvi.pszText = txt;
		SendMessage(importList, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
		if ( sectnum < 0 )
			{													// try to get number from filename
			SplitPath(txt,dir,file,suffix);
			k = 0; j = 0;										// pull out digits in suffix
			while ( k < strlen(suffix) )
				{
   				if ( isdigit(suffix[k]) ) { txt[j] = suffix[k]; j++; }
				k++;
				}
			txt[j] = '\0';
			if ( strlen(txt) ) i = atoi(txt);					// use suffix if has digits
			else	
				{												// otherwise try to use file
				k = 0; j = 0;									// pull out digits in file
				while ( k < strlen(file) )
					{
   					if ( isdigit(file[k]) ) { txt[j] = file[k]; j++; }
					k++;
					}
				txt[j] = '\0';
				if ( strlen(txt) ) i = atoi(txt);				// use digits value of file name
				else i = 0;										// or give up completely
				}
			if ( i < 0 ) i = 0;									// don't allow negative integers
			}
		lvi.iSubItem = 3; lvi.pszText = txt;
		sprintf(txt,"%d",i);
		SendMessage(importList, LVM_SETITEM, 0, (LPARAM)(&lvi));
		sprintf(txt,"%1.*g %1.*g %s", Precision, x, Precision, y, CurrSeries->units);
		lvi.iSubItem = 4; lvi.pszText = txt;
		SendMessage(importList, LVM_SETITEM, 0, (LPARAM)(&lvi));		
		lvi.iItem = SendMessage(importList, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL) );
		i += incr;
		}
}

BOOL CALLBACK ImportImagesProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HCURSOR cur;
	HIMAGELIST himl;
	OPENFILENAME  ofn;		  							// common dialog box structure
	char srcfiles[MAX_PATH+256*200], *src;				// allow up to 200 256-char filenames
	char filename[MAX_PATH], folder[MAX_PATH], txt[MAX_PATH];
	char srcpath[MAX_PATH], localpath[MAX_PATH], destpath[MAX_PATH];
	Section *section;
	Transform *transform;
	Mvmt mvmt;
	ADib *adib;
	bool copyfiles;
	int i, j, ft, w, h, bpp, number_of_images, s, t, image_num;
	double x, y, pixelsize;
	NMHDR *pnmh;
	LV_KEYDOWN *key;
	LV_COLUMN column;
	LV_ITEM lvi;
	RECT dr, wr;

	switch (message)
		{
		case WM_INITDIALOG:
			GetClientRect( hwndDlg, &dr );			// position listview at ID_FILELIST in dialog
			ClientRectToScreen( hwndDlg, &dr );
			GetClientRect( GetDlgItem( hwndDlg, ID_FILELIST ), &wr );
			ClientRectToScreen( GetDlgItem( hwndDlg, ID_FILELIST ), &wr );
			i = wr.left - dr.left;
			j = wr.top - dr.top;
			w = wr.right-wr.left; h = wr.bottom-wr.top,
			importList = CreateWindow( WC_LISTVIEW, "",
											WS_TABSTOP | WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER,
														i, j, w, h, hwndDlg, NULL, appInstance, NULL);
			himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), TRUE, 1, 1);
			ImageList_AddIcon(himl, LoadIcon(appInstance, "DomainIcon"));
			SendMessage(importList, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)(himl));// setup icons
			column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; 
			column.fmt = LVCFMT_LEFT; column.pszText = txt;								// setup columns
			column.iSubItem = 0; strcpy(txt,"Image"); column.cx = w/12; 
			SendMessage(importList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			column.iSubItem = 1; strcpy(txt,"from file"); column.cx = w/3;			// set the column sizes to fit
			SendMessage(importList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			column.iSubItem = 2; strcpy(txt,"of type"); column.cx = w/12; 
			SendMessage(importList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			column.iSubItem = 3; strcpy(txt,"into section"); column.cx = w/6; 
			SendMessage(importList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			column.iSubItem = 4; strcpy(txt,"at position"); column.cx = w/3; 
			SendMessage(importList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));

			sprintf(txt,"%1.*g",Precision,ImagePixelSize);
			SendDlgItemMessage(hwndDlg, ID_PIXELSIZE, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"0.0");
			SendDlgItemMessage(hwndDlg, ID_XOFFSET, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_YOFFSET, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"1");
			SendDlgItemMessage(hwndDlg, ID_FIRSTSECTION, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_INCREMENTBY, WM_SETTEXT, 0, (LPARAM)txt);	
			if ( CopyFiles ) SendDlgItemMessage(hwndDlg, ID_COPYFILES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_COPYFILES, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			return TRUE;

		case WM_NOTIFY:							// process listview notification messages
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case LVN_KEYDOWN:				// process keystrokes on selected items in listview
					key = (LV_KEYDOWN *)lParam;
					switch (key->wVKey)
					  {										// i = item selected
					  case VK_DELETE:
						i = SendMessage(importList, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
						while ( i>=0 )
							{
							SendMessage(importList, LVM_DELETEITEM, i, 0);
							i = SendMessage(importList, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
							}
						break;
					  }
				break;
				}
			return TRUE;

		case UM_CONVERTNEXT:							// do next importation...							
			lvi.mask = LVIF_TEXT; lvi.state = 0; lvi.stateMask = 0; lvi.cchTextMax = MAX_PATH;													
			lvi.iItem = SendMessage(importList, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL) );
			if ( lvi.iItem >= 0 )
				{
				SendDlgItemMessage( hwndDlg, ID_SOURCE, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)folder );
				
				lvi.iSubItem = 1; lvi.pszText = filename;		// read image source filename
				SendMessage(importList, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
				strcpy(srcpath,folder);
				strcat(srcpath,filename);						// form path to image from source & listview text

				lvi.iSubItem = 0; lvi.pszText = txt;			// get number of images in file and
				SendMessage(importList, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
				image_num = atoi(txt) - 1;						// index to subimage in file
				
				lvi.iSubItem = 3; lvi.pszText = txt;
				SendMessage(importList, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
				t = atoi(txt);									// get destination section number
				
				SendDlgItemMessage(hwndDlg, ID_PIXELSIZE, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
				pixelsize = atof(txt);									// get desired pixel size
				SendDlgItemMessage(hwndDlg, ID_XOFFSET, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
				x = atof(txt);
				SendDlgItemMessage(hwndDlg, ID_YOFFSET, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
				y = atof(txt);									// get x and y offsets

				if ( SendDlgItemMessage(hwndDlg, ID_COPYFILES, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
					CopyFiles = true;
				else CopyFiles = false;							// determine whether files should be copied

				MakeLocalPath(WorkingPath,srcpath,localpath);	// make local path to source file
				sprintf(destpath,"%s%s",WorkingPath,filename);	// make path to folder of series

				cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// file access could be slow

				ft = TestGBMfile( srcpath, &w, &h, &bpp, &number_of_images );
				if ( number_of_images > 1 )						// image stack: need to break out subimage
					{
					adib = new ADib( w, h, bpp );				// read subimage from stack file
					adib->filetype = ft;						// will need filetype set for GBM read
					if ( !adib->readGBMtoADib( srcpath, image_num ) )
						ErrMsgSplash( ERRMSG_READ_FAILED, srcpath );
					else										// make new filename in series folder
						{										// but create strings for both local
						sprintf(localpath,"%s_%d.bmp",filename,image_num+1);
						strcpy(srcpath,WorkingPath);
						strcat(srcpath,localpath);				// or absolute paths (decided below)
						adib->SaveADibAsBMP( srcpath );			// write bitmap to new file
						}
					delete adib;								// clean up temp storage
					}
				else											// if single image file, copy only if requested
					if ( strcmp(filename,localpath) )			// if workingpath is not same as file path
					  if ( CopyFiles )							// try to copy file to workingpath
						if ( !CopyFile( srcpath, destpath, FALSE ) )	// if copy fails
							ErrMsgSplash( ERRMSG_COPY_FAILED, srcpath );// just splash becuz multiple files to do
						else
							{
							strcpy(localpath,filename);			// adjust paths to reflect copied locale
							strcpy(srcpath,destpath);			// in either local or absolute path mode
							}
				
				s = t - 1;
				section = GetNextSectionBetween( s, t );
				if ( !section )									// if section exists, add image to it
					{
					section = new Section();					// otherwise, create new section
					section->index = t;
					sprintf(filename,"%s%s.%d",WorkingPath,BaseName,section->index);
					CurrSectionsInfo->AddSection( section, filename );
					if ( sectionWindow ) FillSectionList( sectionList );
					}
																// add image using appropriate path
				if ( CurrSeries->useAbsolutePaths )
					transform = section->AddNewImage( srcpath, pixelsize );
				else transform = section->AddNewImage( localpath, pixelsize );

				if ( transform )								// finally shift image to position
					{
					mvmt.transX = x; mvmt.transY = y;
					transform->nform->PostApply( mvmt );
					PutSection( section, true, true, false );	// save now if not in memory
					}											// save file if successful
				else ErrMsgOK( ERRMSG_INVALID_FORMAT, srcpath );// otherwise, probably bad format
		
				SetCursor( cur );								// remove item from list

				SendMessage(importList, LVM_DELETEITEM, lvi.iItem, 0 );
				UpdateWindow(importList);						// redraw display
				PostMessage( hwndDlg, UM_CONVERTNEXT, 0, 0 );	// continue with next item
				}
			else
				{												// no more images to import so
				if ( FirstSection >= 0 )						// display first section imported
					if ( FindSection( FirstSection, 0, filename ) )	
						GotoSection( FirstSection, filename );	// by paging to it in main window
				}
			return TRUE;
        
		case WM_COMMAND:
		  switch (LOWORD(wParam))
			{									// if change parameter, uptdate list info
         	case ID_XOFFSET:
            case ID_YOFFSET:
            case ID_FIRSTSECTION:
			case ID_INCREMENTBY:
				switch ( HIWORD(wParam) )
				  {
                  case EN_CHANGE:
					CompleteImportList( hwndDlg );
					break;
				  }
				return TRUE;

			case ID_RANDOMFILES:
				Randomize( importList );
				CompleteImportList( hwndDlg );
				return TRUE;

			case ID_SORTFILES:
				if ( SendDlgItemMessage(hwndDlg, ID_SORTFILES, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
					SortItems( importList, false );
				else SortItems( importList, true );
				CompleteImportList( hwndDlg );
				return TRUE;

			case ID_IMPORTFILES:									// import files in order listed
				lvi.mask = LVIF_TEXT; lvi.state = 0; lvi.stateMask = 0; lvi.cchTextMax = MAX_PATH;													
				lvi.iItem = SendMessage(importList, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL) );
				if ( lvi.iItem >= 0 ) 
					{												// if there any items available
					lvi.iSubItem = 3; lvi.pszText = txt;			// remember first section number
					SendMessage(importList, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
					FirstSection = atoi(txt);						
					SendMessage(importList, LVM_ENSUREVISIBLE, 0,0);// scroll to top of list
					PostMessage( hwndDlg, UM_CONVERTNEXT, 0, 0 );	// and initiate import of first image
					}
				return TRUE;

			case ID_BROWSESRC:
				strcpy(srcfiles,"*.*\0");						// initialize OPENFILENAME data
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hwndDlg;
				ofn.hInstance = appInstance;
				ofn.lpstrFile = srcfiles;
				ofn.nMaxFile = MAX_PATH+256*200;
				ofn.lpstrFilter = "All Files\0*.*\0Windows Bitmaps\0*.bmp\0TIFF Images\0*.tif\0GIF Images\0*.gif\0JPEG Images\0*.jpg\0\0";
				ofn.nFilterIndex = 0;
				ofn.lpstrFileTitle = NULL;
				ofn.lpstrTitle = "Select Files\0";
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT
								| OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

				if ( GetOpenFileName(&ofn) )								// get filenames from dialog box
					{
					cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );			// show hourglass, file access could be slow
					j = ofn.nFileOffset;
					strncpy(folder,srcfiles,j);								// get directory of file(s)
					folder[j] = '\0';
					while ( (j>0) && (folder[j] == '\0') ) j--;				// make sure last character is backslash 
					if ( (j>0) && (folder[j] != '\\') ) folder[j+1] = '\\';
					SendDlgItemMessage(hwndDlg, ID_SOURCE, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
					if ( strcmp(txt,folder) )
						SendMessage(importList, LVM_DELETEALLITEMS, 0, 0 );	// clear the listview if new folder

					SendDlgItemMessage(hwndDlg, ID_SOURCE, WM_SETTEXT, 0, (LPARAM) folder);

					lvi.mask = LVIF_TEXT | LVIF_IMAGE; lvi.state = 0; lvi.stateMask = 0; lvi.iImage = 0;
					lvi.iItem = SendMessage(importList, LVM_GETITEMCOUNT, 0, 0 );
																// now fill listview with filenames and types
					i = ofn.nFileOffset;
					while ( srcfiles[i] != '\0' )				// list ends with double zeros when ALLOWMULTISELECT
						{
						src = srcfiles + i;						// compose next file path
						strcpy(filename,folder);
						strcat(filename,src);
						t = TestGBMfile( filename, &w, &h, &bpp, &number_of_images );// attempt read image file
						if ( t )								// if recognize file, then insert in lists
						  while ( number_of_images )
							{
							sprintf(txt,"%d",number_of_images);		// display subimage index
							lvi.iSubItem = 0; lvi.pszText = txt;
							lvi.iItem = SendMessage(importList, LVM_INSERTITEM, 0, (LPARAM)(&lvi));
							number_of_images--;						// display filename
							lvi.iSubItem = 1; lvi.pszText = src;
							SendMessage(importList, LVM_SETITEM, 0, (LPARAM)(&lvi));
			               	strncpy(txt,fileformats+4*t,4); txt[4] = '\0';
							lvi.iSubItem = 2; lvi.pszText = txt;	// display filetype
							SendMessage(importList, LVM_SETITEM, 0, (LPARAM)(&lvi));
							}
						while ( srcfiles[i] != '\0' ) i++;		// advance to end of filename
						i++;									// skip to next filename
						}
					SetCursor( cur );							// restore cursor and sort listing
					PostMessage( hwndDlg, WM_COMMAND, ID_SORTFILES, 0 );
					}
				return TRUE;

			case IDCANCEL:										// quit the import dialog
				EndDialog(hwndDlg, 0);
				return TRUE;
				}
			}

	return FALSE;
}


void CmImportImages( void )									// incorporate images files into sections
{
															// look for likely pixel_size in memory
	if ( PrevSection ) 
		if ( PrevSection->HasImage() ) ImagePixelSize = PrevSection->MinPixelsize();
	if ( CurrSection ) 
		if ( CurrSection->HasImage() ) ImagePixelSize = CurrSection->MinPixelsize();

//	CloseAllChildWindows();									// prevent data modification while importing

	DialogBox( appInstance, "ImportImages", appWnd, (DLGPROC)ImportImagesProc );
}


BOOL CALLBACK ImportLinesProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HCURSOR cur;
	HIMAGELIST himl;
	OPENFILENAME  ofn;		  							// common dialog box structure
	char srcfiles[MAX_PATH+256*200], *src;				// allow up to 200 256-char filenames
	char filename[MAX_PATH], folder[MAX_PATH], txt[MAX_PATH];
	char srcpath[MAX_PATH], localpath[MAX_PATH], destpath[MAX_PATH];
	Section *section;
	Transform *transform;
	Mvmt mvmt;
	bool copyfiles;
	int numFiles, i, j, w, h, s, t, first;
	double x, y, scale;
	NMHDR *pnmh;
	LV_KEYDOWN *key;
	LV_COLUMN column;
	LV_ITEM lvi;
	RECT dr, wr;

	switch (message)
		{
		case WM_INITDIALOG:
			GetClientRect( hwndDlg, &dr );			// position listview at ID_FILELIST in dialog
			ClientRectToScreen( hwndDlg, &dr );
			GetClientRect( GetDlgItem( hwndDlg, ID_FILELIST ), &wr );
			ClientRectToScreen( GetDlgItem( hwndDlg, ID_FILELIST ), &wr );
			i = wr.left - dr.left;
			j = wr.top - dr.top;
			w = wr.right-wr.left; h = wr.bottom-wr.top,
			importList = CreateWindow( WC_LISTVIEW, "",
											WS_TABSTOP | WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER,
														i, j, w, h, hwndDlg, NULL, appInstance, NULL);
			himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), TRUE, 1, 1);
			ImageList_AddIcon(himl, LoadIcon(appInstance, "DomainIcon"));
			SendMessage(importList, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)(himl));// setup icons
			column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; 
			column.fmt = LVCFMT_LEFT; column.pszText = txt;								// setup columns
			column.iSubItem = 0; strcpy(txt,"Place lines"); column.cx = w/3;			// set the column sizes to fit
			SendMessage(importList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			column.iSubItem = 1; strcpy(txt,"of Type"); column.cx = w/6; 
			SendMessage(importList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			column.iSubItem = 2; strcpy(txt,"into Section"); column.cx = w/6; 
			SendMessage(importList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			column.iSubItem = 3; strcpy(txt,"at Position"); column.cx = w/3; 
			SendMessage(importList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));

			sprintf(txt,"1.0");
			SendDlgItemMessage(hwndDlg, ID_SCALE, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"0.0");
			SendDlgItemMessage(hwndDlg, ID_XOFFSET, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_YOFFSET, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"-1");
			SendDlgItemMessage(hwndDlg, ID_FIRSTSECTION, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"1");
			SendDlgItemMessage(hwndDlg, ID_INCREMENTBY, WM_SETTEXT, 0, (LPARAM)txt);	
			return TRUE;

		case WM_NOTIFY:							// process listview notification messages
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case LVN_KEYDOWN:				// process keystrokes on selected items in listview
					key = (LV_KEYDOWN *)lParam;
					switch (key->wVKey)
					  {										// i = item selected
					  case VK_DELETE:
						i = SendMessage(importList, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
						while ( i>=0 )
							{
							SendMessage(importList, LVM_DELETEITEM, i, 0);
							i = SendMessage(importList, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
							}
						break;
					  }
				break;
				}
			return TRUE;

		case UM_CONVERTNEXT:							// do next importation...							
			lvi.mask = LVIF_TEXT; lvi.state = 0; lvi.stateMask = 0; lvi.cchTextMax = MAX_PATH;													
			lvi.iItem = SendMessage(importList, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL) );
			if ( lvi.iItem >= 0 )
				{
				SendDlgItemMessage( hwndDlg, ID_SOURCE, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)folder );
				lvi.iSubItem = 0; lvi.pszText = filename;
				SendMessage(importList, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
				strcpy(srcpath,folder);
				strcat(srcpath,filename);						// form path to image from source & listview text
				lvi.iSubItem = 2; lvi.pszText = txt;
				SendMessage(importList, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)(&lvi));
				t = atoi(txt);									// get destination section number
				
				SendDlgItemMessage(hwndDlg, ID_SCALE, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
				scale = atof(txt);									// get desired pixel size
				SendDlgItemMessage(hwndDlg, ID_XOFFSET, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
				x = atof(txt);
				SendDlgItemMessage(hwndDlg, ID_YOFFSET, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
				y = atof(txt);									// get x and y offsets
				
				cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// file access could be slow

				s = t - 1;
				section = GetNextSectionBetween( s, t );
				if ( !section )									// if section exists, add image to it
					{
					section = new Section();					// otherwise, create new section
					section->index = t;
					sprintf(filename,"%s%s.%d",WorkingPath,BaseName,section->index);
					CurrSectionsInfo->AddSection( section, filename );
					if ( sectionWindow ) FillSectionList( sectionList );
					}
																// add polylines from file
				section->AddDXFLines( srcpath );

				if ( section->active )							// finally shift image to position
					{
					mvmt.transX = x; mvmt.transY = y; mvmt.scaleX = scale; mvmt.scaleY = scale;
					section->active->nform->PostApply( mvmt );
					PutSection( section, true, true, false );	// save now if not in memory
					}											// save file if successful
				else ErrMsgOK( ERRMSG_INVALID_FORMAT, srcpath );// otherwise, probably bad format
		
				SetCursor( cur );								// remove item from list
				SendMessage(importList, LVM_DELETEITEM, lvi.iItem, 0 );
				UpdateWindow(importList);						// redraw display
				PostMessage( hwndDlg, UM_CONVERTNEXT, 0, 0 );	// continue with next item
				}
			return TRUE;
        
		case WM_COMMAND:
		  switch (LOWORD(wParam))
			{									// if change parameter, uptdate list info
         	case ID_XOFFSET:
            case ID_YOFFSET:
            case ID_FIRSTSECTION:
			case ID_INCREMENTBY:
				switch ( HIWORD(wParam) )
				  {
                  case EN_CHANGE:
					CompleteImportList( hwndDlg );
					break;
				  }
				return TRUE;

			case ID_RANDOMFILES:
				Randomize( importList );
				CompleteImportList( hwndDlg );
				return TRUE;

			case ID_SORTFILES:
				if ( SendDlgItemMessage(hwndDlg, ID_SORTFILES, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
					SortItems( importList, false );
				else SortItems( importList, true );
				CompleteImportList( hwndDlg );
				return TRUE;

			case ID_IMPORTFILES:									// import files in order listed
				numFiles = SendMessage(importList, LVM_GETITEMCOUNT, 0, 0 );
				if ( numFiles > 0 ) 
					{
					SendMessage(importList, LVM_ENSUREVISIBLE, 0, 0 );	// scroll to top of list
					PostMessage( hwndDlg, UM_CONVERTNEXT, 0, 0 );
					}
				return TRUE;

			case ID_BROWSESRC:
				strcpy(srcfiles,"*.dxf\0");						// initialize OPENFILENAME data
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = appWnd;
				ofn.hInstance = appInstance;
				ofn.lpstrFile = srcfiles;
				ofn.nMaxFile = MAX_PATH+256*200;
				ofn.lpstrFilter = "DXF files\0*.dxf\0\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.lpstrTitle = "Select DXF Files\0";
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT
								| OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

				if ( GetOpenFileName(&ofn) )								// get filenames from dialog box
					{
					cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );			// show hourglass, file access could be slow
					j = ofn.nFileOffset;
					strncpy(folder,srcfiles,j);								// get directory of file(s)
					folder[j] = '\0';
					while ( (j>0) && (folder[j] == '\0') ) j--;				// make sure last character is backslash 
					if ( (j>0) && (folder[j] != '\\') ) folder[j+1] = '\\';
					SendDlgItemMessage(hwndDlg, ID_SOURCE, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
					if ( strcmp(txt,folder) )
						SendMessage(importList, LVM_DELETEALLITEMS, 0, 0 );	// clear the listview if new folder

					SendDlgItemMessage(hwndDlg, ID_SOURCE, WM_SETTEXT, 0, (LPARAM) folder);

					lvi.mask = LVIF_TEXT | LVIF_IMAGE; lvi.state = 0; lvi.stateMask = 0; lvi.iImage = 0;
					lvi.iItem = SendMessage(importList, LVM_GETITEMCOUNT, 0, 0 );
																// now fill listview with filenames and types
					i = ofn.nFileOffset;
					strcpy(txt,"DXF\0");
					while ( srcfiles[i] != '\0' )				// list ends with double zeros when ALLOWMULTISELECT
						{
						src = srcfiles + i;						// compose next file path
						strcpy(filename,folder);
						strcat(filename,src);
						lvi.iSubItem = 0; lvi.pszText = src;
						lvi.iItem = SendMessage(importList, LVM_INSERTITEM, 0, (LPARAM)(&lvi));
						lvi.iSubItem = 1; lvi.pszText = txt;
						SendMessage(importList, LVM_SETITEM, 0, (LPARAM)(&lvi));
						while ( srcfiles[i] != '\0' ) i++;		// advance to end of filename
						i++;									// skip to next filename
						}
					SetCursor( cur );							// restore cursor and sort listing
					PostMessage( hwndDlg, WM_COMMAND, ID_SORTFILES, 0 );
					}
				return TRUE;

			case IDCANCEL:										// quit the import dialog
				EndDialog(hwndDlg, 0);
				return TRUE;
				}
			}

	return FALSE;
}


void CmImportLines( void )									// incorporate images files into sections
{
//	CloseAllChildWindows();									// prevent data modification while importing

	DialogBox( appInstance, "ImportLines", appWnd, (DLGPROC)ImportLinesProc );
}

BOOL CALLBACK ImportSeriesProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	HCURSOR cur;
	OPENFILENAME  ofn;		  							// common dialog box structure
	Series *fromSeries, *toSeries;
	Section *fromSection, *toSection;
	Transform *fromTransform, *toTransform;
	Image *fromImage, *toImage;
	Contour *fromContour, *toContour;
	int i, sectnum;
	WIN32_FIND_DATA	f;
	HANDLE search;
	char fromBasename[MAX_BASE], fromPath[MAX_PATH], srcpath[MAX_PATH], filename[MAX_PATH], destpath[MAX_PATH];
	char localpath[MAX_PATH], suffix[MAX_PATH], txt[MAX_PATH], traceLimit[MAX_CONTOUR_NAME], domainLimit[MAX_CONTOUR_NAME];
	bool changed, updatePalette, updateZlist, importDomains, importTraces;

	switch (message)
		{
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg, ID_IMPORTOPTIONS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			SendDlgItemMessage(hwndDlg, ID_IMPORTPALETTE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, ID_IMPORTZTRACES, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			sprintf(txt,"*");
			SendDlgItemMessage(hwndDlg, ID_ZTRACENAME, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_DOMAINNAME, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_CONTOURNAME, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_IMPORTSECTIONS, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, ID_IMPORTDOMAINS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			SendDlgItemMessage(hwndDlg, ID_IMPORTTRACES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			SendDlgItemMessage(hwndDlg, ID_COPYFILES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			FirstSection = 0;
			if (CurrSectionsInfo->first) FirstSection = CurrSectionsInfo->first->index;
			LastSection = 0;
			if (CurrSectionsInfo->last) LastSection = CurrSectionsInfo->last->index;	
			sprintf(txt,"%d",FirstSection);
			SendDlgItemMessage(hwndDlg, ID_FIRSTSECTION, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%d",LastSection);
			SendDlgItemMessage(hwndDlg, ID_LASTSECTION, WM_SETTEXT, 0, (LPARAM)txt);
			EnableWindow( GetDlgItem( hwndDlg, ID_IMPORTDOMAINS ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, ID_IMPORTTRACES ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, ID_FIRSTSECTION ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, ID_LASTSECTION ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, ID_DOMAINNAME ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, ID_CONTOURNAME ), FALSE );
			EnableWindow( GetDlgItem( hwndDlg, ID_COPYFILES ), FALSE );
			return TRUE;

		case WM_COMMAND:
		  switch (LOWORD(wParam))
			{									// if change parameter, uptdate list info
			case ID_BROWSE:
				strcpy(srcpath,"*.ser\0");							// the series file will be "seriesname.0"
				ZeroMemory(&ofn, sizeof(ofn));						// clear the OPENFILENAME fields
				ofn.lStructSize = sizeof(ofn);						// set only the necessary values
				ofn.hwndOwner = hwndDlg;
				ofn.lpstrFile = srcpath;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = "All Files\0*.*\0Series files\0*.ser\0\0";
				ofn.nFilterIndex = 2;
				ofn.lpstrTitle = "Select Series";
				ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
				
				if ( GetOpenFileName(&ofn) == TRUE )				// display the Open dialog box.
					SendDlgItemMessage(hwndDlg, ID_SOURCE, WM_SETTEXT, 0, (LPARAM)srcpath);
				return TRUE;

			case ID_IMPORTSECTIONS:
				if ( SendDlgItemMessage(hwndDlg, ID_IMPORTSECTIONS, BM_GETCHECK, 0, 0) == BST_CHECKED )
					{
					EnableWindow( GetDlgItem( hwndDlg, ID_IMPORTDOMAINS ), TRUE );
					EnableWindow( GetDlgItem( hwndDlg, ID_IMPORTTRACES ), TRUE );
					EnableWindow( GetDlgItem( hwndDlg, ID_FIRSTSECTION ), TRUE );
					EnableWindow( GetDlgItem( hwndDlg, ID_LASTSECTION ), TRUE );
					EnableWindow( GetDlgItem( hwndDlg, ID_DOMAINNAME ), TRUE );
					EnableWindow( GetDlgItem( hwndDlg, ID_CONTOURNAME ), TRUE );
					EnableWindow( GetDlgItem( hwndDlg, ID_COPYFILES ), TRUE );
					}
				else {
					EnableWindow( GetDlgItem( hwndDlg, ID_IMPORTDOMAINS ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, ID_IMPORTTRACES ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, ID_FIRSTSECTION ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, ID_LASTSECTION ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, ID_DOMAINNAME ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, ID_CONTOURNAME ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, ID_COPYFILES ), FALSE );
					}
				return TRUE;

			case IDOK:												// user pressed Import button!
				SendDlgItemMessage(hwndDlg, ID_SOURCE, WM_GETTEXT, MAX_PATH, (LPARAM)srcpath);

				cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow!

				fromSeries = new Series( srcpath );					// read series file, return if invalid
				if ( fromSeries->index < 0 ) { delete fromSeries; return TRUE; }

				SplitPath(srcpath,fromPath,txt,suffix);
				strncpy(fromBasename,txt,MAX_BASE-1);				// get basename, make sure it fits
				fromBasename[MAX_BASE] = '\0';

				if ( SendDlgItemMessage(hwndDlg, ID_COPYFILES, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
					CopyFiles = true;
				else CopyFiles = false;							// determine whether images should be copied
				
				if ( SendDlgItemMessage(hwndDlg, ID_IMPORTOPTIONS, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
					toSeries = new Series( *fromSeries );			// import options settings
				else toSeries = new Series( *CurrSeries );

				if ( SendDlgItemMessage(hwndDlg, ID_IMPORTPALETTE, BM_GETCHECK, 0, 0) == BST_CHECKED )
					{
					delete toSeries->contours;						// import palette
					toSeries->contours = new Contours( *(fromSeries->contours) );
					updatePalette = true;
					}
				else updatePalette = false;

				if ( (SendDlgItemMessage(hwndDlg, ID_IMPORTZTRACES, BM_GETCHECK, 0, 0) == BST_CHECKED)
				   && fromSeries->zcontours )						// only import them if they exist
					{
					updateZlist = false;							
					SendDlgItemMessage(hwndDlg, ID_ZTRACENAME, WM_GETTEXT, MAX_CONTOUR_NAME, (LPARAM)traceLimit);
					if ( !toSeries->zcontours ) toSeries->zcontours = new Contours();
					fromContour = fromSeries->zcontours->first;
					while ( fromContour )							// import matching z-traces
						{
						if ( MatchLimit( fromContour->name, traceLimit ) )
							{
							toContour = new Contour( *fromContour );
							toSeries->zcontours->Add( toContour );
							updateZlist = true;
							}
						fromContour = fromContour->next;
						}
					if ( IsWindow( zTraceWindow ) && updateZlist ) CmZList();	// blow away old z-list 
					}

				delete fromSeries;
				toSeries->index	= CurrSeries->index;			// keep current position in series
				toSeries->pixel_size = CurrSeries->pixel_size;	// becuz user will need to reset view
				toSeries->offset_x = CurrSeries->offset_x;
				toSeries->offset_y = CurrSeries->offset_y;		// replace CurrSeries with changes
				delete CurrSeries;
				CurrSeries = toSeries;							// now, import section data...

				if ( SendDlgItemMessage(hwndDlg, ID_IMPORTSECTIONS, BM_GETCHECK, 0, 0) == BST_CHECKED )
					{
					SendDlgItemMessage(hwndDlg, ID_FIRSTSECTION, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
					FirstSection = atoi( txt );
					SendDlgItemMessage(hwndDlg, ID_LASTSECTION, WM_GETTEXT, MAX_PATH, (LPARAM)txt);
					LastSection = atoi( txt );
					if ( SendDlgItemMessage(hwndDlg, ID_IMPORTDOMAINS, BM_GETCHECK, 0, 0) == BST_CHECKED )
						{
						importDomains = true;
						SendDlgItemMessage(hwndDlg, ID_DOMAINNAME, WM_GETTEXT, MAX_CONTOUR_NAME, (LPARAM)domainLimit);
						}
					else importDomains = false;
					if ( SendDlgItemMessage(hwndDlg, ID_IMPORTTRACES, BM_GETCHECK, 0, 0) == BST_CHECKED )
						{
						importTraces = true;
						SendDlgItemMessage(hwndDlg, ID_CONTOURNAME, WM_GETTEXT, MAX_CONTOUR_NAME, (LPARAM)traceLimit);
						}
					else importTraces = false;
					
					for (sectnum=FirstSection; sectnum<=LastSection; sectnum++)
						{
						sprintf(srcpath,"%s%s.%d",fromPath,fromBasename,sectnum);
						fromSection = NULL;
						search = FindFirstFile( srcpath, &f );			// find foreign section file
						if ( search != INVALID_HANDLE_VALUE ) fromSection = new Section( srcpath );
						FindClose( search );
						
						if ( fromSection )
						  if ( fromSection->index < 0 ) delete fromSection;
						  else 											// have foreign section data for this sectnum
							{
							i = sectnum-1;
							toSection = GetNextSectionBetween( i, sectnum );
							if ( !toSection )
								{										// create new section if none in CurrSeries
								toSection = new Section();
								toSection->index = fromSection->index;
								toSection->thickness = fromSection->thickness;
								toSection->alignLocked = fromSection->alignLocked;
								sprintf(filename,"%s%s.%d",WorkingPath,BaseName,toSection->index);
								CurrSectionsInfo->AddSection( toSection, filename );
								changed = true;
								}
							else changed = false;						// next copy data as desired
							
							if ( fromSection->transforms && (importDomains || importTraces)	)
								{
								if ( !toSection->transforms ) toSection->transforms = new Transforms();
								changed = true;
								fromTransform = fromSection->transforms->first;
								while ( fromTransform )
									{
									toTransform = new Transform();		// first copy transform and its parameters
									delete toTransform->nform;
									toTransform->nform = new Nform( *(fromTransform->nform) );
									toSection->transforms->Add( toTransform );	// add it to section

									if ( importTraces && fromTransform->contours )
										{										// copy contours
										toTransform->contours = new Contours();
										fromContour = fromTransform->contours->first;
										while ( fromContour )
											{							// only include contour names that match limit
											if ( MatchLimit( fromContour->name, traceLimit ) )
												{
												toContour = new Contour( *fromContour );
												toTransform->contours->Add( toContour );
												}
											fromContour = fromContour->next;
											}
										}
																				// copy image & domain
									if ( importDomains && fromTransform->domain )
									  if ( MatchLimit( fromTransform->domain->name, domainLimit ) )
										{										
										toTransform->image = new Image( *(fromTransform->image) ); // copy all paths
										toTransform->domain = new Contour( *(fromTransform->domain) );
																							// First do image src file...
										MakeAbsolutePath(fromPath,toTransform->image->src,srcpath);// absolute path to image
										SplitPath(srcpath,txt,filename,suffix);			// get filename of image file
										strcat(filename,suffix);
										MakeLocalPath(WorkingPath,srcpath,localpath);	// make local path to image file
										if ( strcmp(filename,localpath) )				// if workingpath is not same as file path
										  if ( CopyFiles )								// try to copy file to workingpath
											{
											sprintf(destpath,"%s%s",WorkingPath,filename);	// make path to folder of series
											if ( !CopyFile( srcpath, destpath, FALSE ) )	// if copy fails
												ErrMsgSplash( ERRMSG_COPY_FAILED, srcpath );// just splash becuz multiple files to do
											else
												{
												strcpy(localpath,filename);			// adjust paths to reflect copied locale
												strcpy(srcpath,destpath);			// in either local or absolute path mode
												}
											}
										if ( CurrSeries->useAbsolutePaths )
											strcpy(toTransform->image->src,srcpath);	// use series option to determine
										else strcpy(toTransform->image->src,localpath);	// which path type to use
										
										if ( strlen(toTransform->image->proxySrc) )			// Next, do image proxySrc file...
											{
											MakeAbsolutePath(fromPath,toTransform->image->proxySrc,srcpath);// absolute path to image
											SplitPath(srcpath,txt,filename,suffix);			// get filename of image file
											strcat(filename,suffix);
											MakeLocalPath(WorkingPath,srcpath,localpath);	// make local path to image file
											if ( strcmp(filename,localpath) )				// if workingpath is not same as file path
											  if ( CopyFiles )								// try to copy file to workingpath
												{
												sprintf(destpath,"%s%s",WorkingPath,filename);	// make path to folder of series
												if ( !CopyFile( srcpath, destpath, FALSE ) )	// if copy fails
													ErrMsgSplash( ERRMSG_COPY_FAILED, srcpath );// just splash becuz multiple files to do
												else 
													{
													strcpy(localpath,filename);			// adjust paths to reflect copied locale
													strcpy(srcpath,destpath);			// in either local or absolute path mode
													}
												}
											if ( CurrSeries->useAbsolutePaths )
												strcpy(toTransform->image->proxySrc,srcpath);	// use series option to determine
											else strcpy(toTransform->image->proxySrc,localpath);	// which path to use
											}
										}
												
									fromTransform = fromTransform->next;		// copy from next transform
									}
								}
							
							delete fromSection;								// free memory
							PutSection( toSection, changed, importDomains, importTraces );
							}
						}	// end for loop
					}	// end if

				SetCursor( cur );								// restore cursor and update windows
				if ( sectionWindow ) FillSectionList( sectionList );
				if ( IsWindow(paletteWindow) && updatePalette )
					for (i=0; i<CurrSeries->contours->Number(); i++) UpdatePaletteButton( i );

				// fall through to EndDialog
			case IDCANCEL:										
				EndDialog(hwndDlg, 0);							// quit the import dialog
				return TRUE;
				}
			}

	return FALSE;
}


void CmImportSeries( void )							// incorporate foreign series data into CurrSeries
{
//	CloseAllChildWindows();									// prevent data modification while importing

	DialogBox( appInstance, "ImportSeries", appWnd, (DLGPROC)ImportSeriesProc );
}

											// dialog procedure for options property sheets

BOOL CALLBACK GeneralDlgProc(HWND hwndDlg, UINT message, WPARAM, LPARAM lParam)
{
	NMHDR *pnmh;
	NM_UPDOWN *pnmud;
	int precision;
	char txt[MAX_DIGITS];

	switch (message)
		{
		case WM_INITDIALOG:
			if ( LastOptionTab != 0 ) LastOptionTab = 0;	// if just opened options, center window
			else CenterDialog( appWnd, GetParent(hwndDlg) );
			SendDlgItemMessage(hwndDlg, ID_UNITS, WM_SETTEXT, 0, (LPARAM)(CurrSeries->units));
			sprintf(txt,"%d",Precision);
			SendDlgItemMessage(hwndDlg, ID_SIGDIGITS, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->defaultThickness);
			SendDlgItemMessage(hwndDlg, ID_THICKNESS, WM_SETTEXT, 0, (LPARAM)txt);
			if ( CurrSeries->autoSaveSeries ) SendDlgItemMessage(hwndDlg, ID_AUTOSAVESERIES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_AUTOSAVESERIES, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->warnSaveSection ) SendDlgItemMessage(hwndDlg, ID_WARNSAVESECTION, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_WARNSAVESECTION, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->autoSaveSection ) SendDlgItemMessage(hwndDlg, ID_AUTOSAVESECTION, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_AUTOSAVESECTION, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->beepDeleting ) SendDlgItemMessage(hwndDlg, ID_BEEPDELETING, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_BEEPDELETING, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->beepPaging ) SendDlgItemMessage(hwndDlg, ID_BEEPPAGING, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_BEEPPAGING, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->hideTraces ) SendDlgItemMessage(hwndDlg, ID_HIDETRACES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_HIDETRACES, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->unhideTraces ) SendDlgItemMessage(hwndDlg, ID_UNHIDETRACES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_UNHIDETRACES, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->hideDomains ) SendDlgItemMessage(hwndDlg, ID_HIDEDOMAINS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_HIDEDOMAINS, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->unhideDomains ) SendDlgItemMessage(hwndDlg, ID_UNHIDEDOMAINS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_UNHIDEDOMAINS, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->useAbsolutePaths ) SendDlgItemMessage(hwndDlg, ID_USEABSOLUTEPATHS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_USEABSOLUTEPATHS, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->zMidSection ) SendDlgItemMessage(hwndDlg, ID_ZMIDSECTION, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_ZMIDSECTION, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			return TRUE;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{						// since only one up/down control don't check id
				case UDN_DELTAPOS:
					pnmud = (NM_UPDOWN *)lParam; 
					SendDlgItemMessage(hwndDlg, ID_SIGDIGITS, WM_GETTEXT, MAX_DIGITS, (LPARAM)txt);
					precision = atoi(txt);
					if ( pnmud->iDelta < 0 ) precision++; else precision--;
					if ( precision < 6 ) precision = 6;
					if ( precision > MAX_SIGDIGITS ) precision = MAX_SIGDIGITS;
					sprintf(txt,"%d",precision);
					SendDlgItemMessage(hwndDlg, ID_SIGDIGITS, WM_SETTEXT, 0, (LPARAM)txt);
					return TRUE;
				case PSN_APPLY:
					SendDlgItemMessage(hwndDlg, ID_SIGDIGITS, WM_GETTEXT, MAX_DIGITS, (LPARAM)txt);
					precision = atoi(txt);
					if ( precision < Precision ) ErrMsgOK( ERRMSG_PRECISION, "" );
					Precision = precision;
					CurrSeries->significantDigits = Precision;
					SendDlgItemMessage(hwndDlg, ID_UNITS, WM_GETTEXT, MAX_UNITS_STRING, (LPARAM)(CurrSeries->units));
					SendDlgItemMessage(hwndDlg, ID_THICKNESS, WM_GETTEXT, MAX_DIGITS, (LPARAM)txt);
					CurrSeries->defaultThickness = atof(txt);
					if ( SendDlgItemMessage(hwndDlg, ID_AUTOSAVESERIES, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->autoSaveSeries = true;
					else CurrSeries->autoSaveSeries = false;
					if ( SendDlgItemMessage(hwndDlg, ID_WARNSAVESECTION, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->warnSaveSection = true;
					else CurrSeries->warnSaveSection = false;
					if ( SendDlgItemMessage(hwndDlg, ID_AUTOSAVESECTION, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->autoSaveSection = true;
					else CurrSeries->autoSaveSection = false;
					if ( SendDlgItemMessage(hwndDlg, ID_BEEPDELETING, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->beepDeleting = true;
					else CurrSeries->beepDeleting = false;
					if ( SendDlgItemMessage(hwndDlg, ID_BEEPPAGING, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->beepPaging = true;
					else CurrSeries->beepPaging = false;
					if ( SendDlgItemMessage(hwndDlg, ID_HIDETRACES, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->hideTraces = true;
					else CurrSeries->hideTraces = false;
					if ( SendDlgItemMessage(hwndDlg, ID_UNHIDETRACES, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->unhideTraces = true;
					else CurrSeries->unhideTraces = false;
					if ( SendDlgItemMessage(hwndDlg, ID_HIDEDOMAINS, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->hideDomains = true;
					else CurrSeries->hideDomains = false;
					if ( SendDlgItemMessage(hwndDlg, ID_UNHIDEDOMAINS, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->unhideDomains = true;
					else CurrSeries->unhideDomains = false;
					if ( SendDlgItemMessage(hwndDlg, ID_USEABSOLUTEPATHS, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->useAbsolutePaths = true;
					else CurrSeries->useAbsolutePaths = false;
					if ( SendDlgItemMessage(hwndDlg, ID_ZMIDSECTION, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->zMidSection = true;
					else CurrSeries->zMidSection = false;
					return TRUE;
				case PSN_RESET:
					return TRUE;
				}
			break;
		}
	return FALSE;
}

																	// set the default attributes fields in the Traces tab
void SetDefaultValues( HWND hwndDlg, char *name, Color border, Color fill, int mode, char *comment )
{
	SendDlgItemMessage(hwndDlg, ID_CONTOURNAME, WM_SETTEXT, 0, (LPARAM)name);
	ColorButton( GetDlgItem( hwndDlg, ID_BORDERCOLOR ), border );
	ColorButton( GetDlgItem( hwndDlg, ID_FILLCOLOR ), fill );
	if ( mode < 0 ) SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	else SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	if ( abs(mode) == R2_NOP ) SendDlgItemMessage(hwndDlg, ID_NOFILL, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	else SendDlgItemMessage(hwndDlg, ID_NOFILL, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	if ( abs(mode) == R2_COPYPEN ) SendDlgItemMessage(hwndDlg, ID_COPYFILL, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);			
	else SendDlgItemMessage(hwndDlg, ID_COPYFILL, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	if ( abs(mode) == R2_MASKPEN ) SendDlgItemMessage(hwndDlg, ID_MASKFILL, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	else SendDlgItemMessage(hwndDlg, ID_MASKFILL, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	if ( abs(mode) == R2_MERGEPEN ) SendDlgItemMessage(hwndDlg, ID_MERGEFILL, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	else SendDlgItemMessage(hwndDlg, ID_MERGEFILL, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	SendDlgItemMessage(hwndDlg, ID_COMMENT, WM_SETTEXT, 0, (LPARAM)comment);
}

void SwapPaletteRows( void )					// swap rows of buttons
{
	HBITMAP canvas;
	Button *button;
	Contour *contour, *econtour;
	int i = 0;									// first swap the actual contours
	contour = CurrSeries->contours->last;
	while ( contour && (i<10) )					// go half-way through contour list
		{
		i++;									// moving from the end to front
		econtour = contour;
		contour = contour->prev;
		CurrSeries->contours->Extract( econtour );		// extract last one
		CurrSeries->contours->AddFirst( econtour );		// and add it to front of list
		}
	
	if ( DefaultPaletteButtons )				// now update button images in dialog
	  {
	  button = DefaultPaletteButtons->first;
	  contour = CurrSeries->contours->first;
	  while ( contour )
		{
		canvas = button->canvas;
		button->canvas = MakeContourBitmap( button->hwnd, contour );
		SendMessage( button->hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(button->canvas) );
		DeleteObject( canvas );
		SendMessage( button->hwnd, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0 );	// unpush buttons
		button = button->next;
		contour = contour->next;
		}
	  }

	if ( PaletteButtons )						// and update button images in palette window
	  {
	  button = PaletteButtons->first;
	  contour = CurrSeries->contours->first;
	  while ( contour )
		{
		canvas = button->canvas;							// update the palette image
		button->canvas = MakeContourBitmap( button->hwnd, contour );
		SendMessage( button->hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(button->canvas) );
		DeleteObject( canvas );
		SendMessage( button->hwnd, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0 );	// unpush buttons
		button = button->next;
		contour = contour->next;
		}
	  }
}


void UpdatePaletteButtons( void )					// update the image of the palette contour on buttons
{
	HBITMAP canvas;
	Button *button;
	Contour *contour;

	if ( DefaultPaletteButtons )
	  {
	  button = DefaultPaletteButtons->first;		// figure out which button has the default contour
	  contour = CurrSeries->contours->first;
	  while ( contour )
		if ( contour == DefaultPaletteContour )		// OK TO COMPARE LOCAL PTR TO GLOBAL ONE?
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
			}
		}

	if ( PaletteButtons )
	  {
	  button = PaletteButtons->first;		// figure out which button has the default contour
	  contour = CurrSeries->contours->first;
	  while ( contour )
		if ( contour == DefaultPaletteContour )		// OK TO COMPARE LOCAL PTR TO GLOBAL ONE?
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
			}
		}
}


int sign( int i )
{
	if ( i < 0 ) return -1; else return 1;
}

BOOL CALLBACK NamesColorsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pnmh;
	CHOOSECOLOR cc;
	Color c;
	Button *button;
	Contour *contour;
	HWND chWnd;
	RECT dr, wr;
	WORD cmd;
	int i, x, y, w, h;
	char txt[MAX_COMMENT];

	switch (message)
		{
		case WM_INITDIALOG:
			if ( LastOptionTab != 1 ) LastOptionTab = 1;	// if just opened options, center window
			else CenterDialog( appWnd, GetParent(hwndDlg) );
			SetDefaultValues( hwndDlg, CurrSeries->defaultName, CurrSeries->defaultBorder, CurrSeries->defaultFill,
									CurrSeries->defaultMode, CurrSeries->defaultComment );
			DefaultPaletteButtons = new Buttons();
			DefaultPaletteContour = NULL;
			GetWindowRect( hwndDlg, &dr );
			GetWindowRect( GetDlgItem( hwndDlg, ID_PALETTE ), &wr );		// position buttons at ID_PALETTE relative to dialog
			x = wr.left - dr.left;
			y = wr.top - dr.top;
			CreatePaletteButtons( hwndDlg, x, y, CurrSeries->contours, DefaultPaletteButtons );
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case ID_CONTOURNAME:
					SendDlgItemMessage(hwndDlg, ID_CONTOURNAME, WM_GETTEXT, (WPARAM)MAX_COMMENT, (LPARAM)txt);
					RemoveIllegalChars( txt );
					if ( DefaultPaletteContour ) strcpy(DefaultPaletteContour->name,txt);
					else strcpy(CurrSeries->defaultName,txt);
					return TRUE;

				case ID_BORDERCOLOR:
					ZeroMemory(&cc, sizeof(CHOOSECOLOR));		// setup color dialog structure
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;						// set current color for dialog
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					if ( DefaultPaletteContour ) cc.rgbResult = DefaultPaletteContour->border.ref();
					else cc.rgbResult = CurrSeries->defaultBorder.ref();
					for (i=0; i<16; i++) CustomColors[i] = CurrSeries->borderColors[i].ref();
					cc.lpCustColors = CustomColors;				// set custom colors
					if ( ChooseColor(&cc) )						// open color dialog
						{										// use return value to update button
						c = Color( cc.rgbResult );
						ColorButton( GetDlgItem( hwndDlg, ID_BORDERCOLOR ), c );
						for (i=0; i<16; i++) CurrSeries->borderColors[i] = Color( CustomColors[i] );
						if ( DefaultPaletteContour )
							{
							DefaultPaletteContour->border = c;	// and to update the default color
							UpdatePaletteButtons();
							}
						else CurrSeries->defaultBorder = c;
						}
					return TRUE;

				case ID_FILLCOLOR:
					ZeroMemory(&cc, sizeof(CHOOSECOLOR));		// setup color dialog structure
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;						// set current color for dialog
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					if ( DefaultPaletteContour ) cc.rgbResult = DefaultPaletteContour->fill.ref();
					else cc.rgbResult = CurrSeries->defaultFill.ref();
					for (i=0; i<16; i++) CustomColors[i] = CurrSeries->fillColors[i].ref();
					cc.lpCustColors = CustomColors;				// set custom colors
					if ( ChooseColor(&cc) )						// open color dialog
						{										// use Result to update button
						c = Color( cc.rgbResult );
						ColorButton( GetDlgItem( hwndDlg, ID_FILLCOLOR ), c );
						for (i=0; i<16; i++) CurrSeries->fillColors[i] = Color( CustomColors[i] );
						if ( DefaultPaletteContour )
							{
							DefaultPaletteContour->fill = c;	// and to update the default color
							UpdatePaletteButtons();
							}
						else CurrSeries->defaultFill = c;
						}
					return TRUE;

				case ID_FILLSELECTED:							// state change
					if ( SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						i = -1;
					else i = 1;
					if ( DefaultPaletteContour ) DefaultPaletteContour->mode = i*abs(DefaultPaletteContour->mode);
					else CurrSeries->defaultMode = i*abs(CurrSeries->defaultMode);
					return TRUE;

				case ID_NOFILL:									// no fill checked
					if ( DefaultPaletteContour )
						{
						DefaultPaletteContour->mode = sign(DefaultPaletteContour->mode)*R2_NOP;
						UpdatePaletteButtons();
						}
					else CurrSeries->defaultMode = sign(CurrSeries->defaultMode)*R2_NOP;
					return TRUE;

				case ID_MASKFILL:								// mask fill checked
					if ( DefaultPaletteContour )
						{
						DefaultPaletteContour->mode = sign(DefaultPaletteContour->mode)*R2_MASKPEN;
						UpdatePaletteButtons();
						}
					else CurrSeries->defaultMode = sign(CurrSeries->defaultMode)*R2_MASKPEN;
					return TRUE;

				case ID_COPYFILL:								// copy fill checked
					if ( DefaultPaletteContour )
						{
						DefaultPaletteContour->mode = sign(DefaultPaletteContour->mode)*R2_COPYPEN;
						UpdatePaletteButtons();
						}
					else CurrSeries->defaultMode = sign(CurrSeries->defaultMode)*R2_COPYPEN;
					return TRUE;

				case ID_MERGEFILL:								// merge fill checked
					if ( DefaultPaletteContour )
						{
						DefaultPaletteContour->mode = sign(DefaultPaletteContour->mode)*R2_MERGEPEN;
						UpdatePaletteButtons();
						}
					else CurrSeries->defaultMode = sign(CurrSeries->defaultMode)*R2_MERGEPEN;
					return TRUE;

				case ID_COMMENT:
					SendDlgItemMessage(hwndDlg, ID_COMMENT, WM_GETTEXT, (WPARAM)MAX_COMMENT, (LPARAM)txt);
					RemoveIllegalChars( txt );
					if ( DefaultPaletteContour ) strcpy(DefaultPaletteContour->comment,txt);
					else strcpy(CurrSeries->defaultComment,txt);
					return TRUE;

				case ID_GETCLIPBOARD:			// copy clipboard attributes into dialog
					if ( ClipboardTransform )
					  if ( ClipboardTransform->contours )
						{
						contour = ClipboardTransform->contours->first;
						SetDefaultValues( hwndDlg, contour->name, contour->border, contour->fill, contour->mode, contour->comment );
						if ( DefaultPaletteContour )
							{
							strcpy(DefaultPaletteContour->name,contour->name);
							strcpy(DefaultPaletteContour->comment,contour->comment);
							DefaultPaletteContour->border = contour->border;
							DefaultPaletteContour->fill = contour->fill;
							DefaultPaletteContour->mode =	contour->mode;
							}
						else {
							strcpy(CurrSeries->defaultName,contour->name);
							strcpy(CurrSeries->defaultComment,contour->comment);
							CurrSeries->defaultBorder = contour->border;
							CurrSeries->defaultFill = contour->fill;
							CurrSeries->defaultMode = contour->mode;
							}
						}
					return TRUE;

				case ID_SWAPPALETTEROWS:
					SwapPaletteRows();
					return TRUE;

				case BN_CLICKED:							// user clicked one of the palette buttons
					chWnd = (HWND)lParam;					// lParam contains handle to button window
					button = DefaultPaletteButtons->first;	// figure out which button was pressed
					contour = CurrSeries->contours->first;	// and which contour that is
					i = 0;
					while ( button )
						if ( button->hwnd == chWnd )
							{								// remember which default contour this is
							DefaultPaletteContour = contour;
							DefaultPaletteContour->Id = i;
							SetDefaultValues( hwndDlg, contour->name, contour->border, contour->fill, contour->mode, contour->comment );				
							button = NULL;
							}
						else {
							button = button->next;
							contour = contour->next;
							i++;
							}
					return TRUE;
				}
			break;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{										// user clicks OK, update changes
				case PSN_APPLY:
					if ( DefaultPaletteContour )		// user edited palette contour, use this as defaults
						{
						CurrSeries->defaultMode = DefaultPaletteContour->mode;
						CurrSeries->defaultFill = DefaultPaletteContour->fill;
						CurrSeries->defaultBorder = DefaultPaletteContour->border;
						strcpy(CurrSeries->defaultName,DefaultPaletteContour->name);
						strcpy(CurrSeries->defaultComment,DefaultPaletteContour->comment);
						PushPaletteButton( DefaultPaletteContour->Id );
						}
					else UnPushPaletteButtons();		// otherwise clear palette button presses
				case PSN_RESET:
					if ( DefaultPaletteButtons ) delete DefaultPaletteButtons;
					DefaultPaletteButtons = NULL;
					DefaultPaletteContour = NULL;
					return TRUE;
				}
			break;
		}
	return FALSE;
}

BOOL CALLBACK GridsDlgProc(HWND hwndDlg, UINT message, WPARAM, LPARAM lParam)
{
	NMHDR *pnmh;
	char txt[MAX_DIGITS];

	switch (message)
		{
		case WM_INITDIALOG:
			if ( LastOptionTab != 2 ) LastOptionTab = 2;	// if just opened options, center window
			else CenterDialog( appWnd, GetParent(hwndDlg) );
			SendDlgItemMessage(hwndDlg, ID_UNITS1, WM_SETTEXT, 0, (LPARAM)CurrSeries->units);
			SendDlgItemMessage(hwndDlg, ID_UNITS2, WM_SETTEXT, 0, (LPARAM)CurrSeries->units);
			switch (CurrSeries->gridType)
			  {									// Autoradiobutton will clear others so just set one
			  case (POINT_GRID):
				SendDlgItemMessage(hwndDlg, ID_GRIDPOINT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (FRAME_GRID):
				SendDlgItemMessage(hwndDlg, ID_GRIDFRAME, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (STAMP_GRID):
				SendDlgItemMessage(hwndDlg, ID_GRIDSTAMP, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (CLIPBOARD_GRID):
				SendDlgItemMessage(hwndDlg, ID_GRIDCLIPBOARD, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (CYCLOID_GRID):
				SendDlgItemMessage(hwndDlg, ID_GRIDCYCLOID, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (ELLIPSE_GRID):
				SendDlgItemMessage(hwndDlg, ID_GRIDELLIPSE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  default: // RECTANGLE_GRID
				SendDlgItemMessage(hwndDlg, ID_GRIDRECTANGLE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  }
			sprintf(txt,"%d",CurrSeries->gridXNumber);
			SendDlgItemMessage(hwndDlg, ID_NUMBERX, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%d",CurrSeries->gridYNumber);
			SendDlgItemMessage(hwndDlg, ID_NUMBERY, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->gridXSize);
			SendDlgItemMessage(hwndDlg, ID_SIZEX, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->gridYSize);
			SendDlgItemMessage(hwndDlg, ID_SIZEY, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->gridXDistance);
			SendDlgItemMessage(hwndDlg, ID_DISTANCEX, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->gridYDistance);
			SendDlgItemMessage(hwndDlg, ID_DISTANCEY, WM_SETTEXT, 0, (LPARAM)txt);
			return TRUE;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case PSN_APPLY:
					if ( SendDlgItemMessage(hwndDlg, ID_GRIDRECTANGLE, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->gridType = RECTANGLE_GRID;
					if ( SendDlgItemMessage(hwndDlg, ID_GRIDPOINT, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->gridType = POINT_GRID;
					if ( SendDlgItemMessage(hwndDlg, ID_GRIDFRAME, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->gridType = FRAME_GRID;
					if ( SendDlgItemMessage(hwndDlg, ID_GRIDSTAMP, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->gridType = STAMP_GRID;
					if ( SendDlgItemMessage(hwndDlg, ID_GRIDCLIPBOARD, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->gridType = CLIPBOARD_GRID;
					if ( SendDlgItemMessage(hwndDlg, ID_GRIDCYCLOID, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->gridType = CYCLOID_GRID;
					if ( SendDlgItemMessage(hwndDlg, ID_GRIDELLIPSE, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->gridType = ELLIPSE_GRID;
					SendDlgItemMessage(hwndDlg, ID_NUMBERX, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->gridXNumber= atoi(txt);
					SendDlgItemMessage(hwndDlg, ID_NUMBERY, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->gridYNumber = atoi(txt);
					SendDlgItemMessage(hwndDlg, ID_SIZEX, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->gridXSize = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_SIZEY, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->gridYSize = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_DISTANCEX, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->gridXDistance = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_DISTANCEY, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->gridYDistance = atof(txt);
					return TRUE;
				case PSN_RESET:
					return TRUE;
				}
			break;
		}
	return FALSE;
}


BOOL CALLBACK ThreedDlgProc(HWND hwndDlg, UINT message, WPARAM, LPARAM lParam)
{
	NMHDR *pnmh;
	Contour *zcontour;
	int i, j;
	char txt[MAX_DIGITS];

	switch (message)
		{
		case WM_INITDIALOG:
			if ( LastOptionTab != 3 ) LastOptionTab = 3;	// if just opened options, center window
			else CenterDialog( appWnd, GetParent(hwndDlg) );
			sprintf(txt,"%1.*g",Precision,CurrSeries->x3Doffset);
			SendDlgItemMessage(hwndDlg, ID_XOFFSET, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->y3Doffset);
			SendDlgItemMessage(hwndDlg, ID_YOFFSET, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->z3Doffset);
			SendDlgItemMessage(hwndDlg, ID_ZOFFSET, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_UNITS1, WM_SETTEXT, 0, (LPARAM)CurrSeries->units);
			SendDlgItemMessage(hwndDlg, ID_UNITS2, WM_SETTEXT, 0, (LPARAM)CurrSeries->units);
			SendDlgItemMessage(hwndDlg, ID_UNITS3, WM_SETTEXT, 0, (LPARAM)CurrSeries->units);
			switch (CurrSeries->type3Dobject)
			  {									// Autoradiobutton will clear others so just set one
			  case (SURFACE_OBJECT):
				SendDlgItemMessage(hwndDlg, ID_3DSURFACE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (POINTSET_OBJECT):
				SendDlgItemMessage(hwndDlg, ID_3DPOINTSET, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (AREAS_OBJECT):
				SendDlgItemMessage(hwndDlg, ID_3DAREAS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (BOX_OBJECT):
				SendDlgItemMessage(hwndDlg, ID_3DBOX, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (CYLINDER_OBJECT):
				SendDlgItemMessage(hwndDlg, ID_3DCYLINDER, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (ELLIPSOID_OBJECT):
				SendDlgItemMessage(hwndDlg, ID_3DELLIPSOID, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  case (SPHERE_OBJECT):
				SendDlgItemMessage(hwndDlg, ID_3DSPHERE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  default: // TRACES_OBJECT
				SendDlgItemMessage(hwndDlg, ID_3DTRACES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				break;
			  }
			sprintf(txt,"%d",CurrSeries->first3Dsection);
			SendDlgItemMessage(hwndDlg, ID_FIRSTSECTION, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%d",CurrSeries->last3Dsection);
			SendDlgItemMessage(hwndDlg, ID_LASTSECTION, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->max3Dconnection);
			SendDlgItemMessage(hwndDlg, ID_MAXCONNECTION, WM_SETTEXT, 0, (LPARAM)txt);
			if ( CurrSeries->upper3Dfaces ) SendDlgItemMessage(hwndDlg, ID_UPPERFACES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_UPPERFACES, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->lower3Dfaces ) SendDlgItemMessage(hwndDlg, ID_LOWERFACES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LOWERFACES, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->faceNormals )									// just set one becuz auto
				SendDlgItemMessage(hwndDlg, ID_FACENORMALS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else if ( CurrSeries->vertexNormals ) SendDlgItemMessage(hwndDlg, ID_VERTEXNORMALS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_NONORMALS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			if ( CurrSeries->facets3D < 6 )
				SendDlgItemMessage(hwndDlg, ID_4FACETS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else if (  CurrSeries->facets3D < 12 )
				SendDlgItemMessage(hwndDlg, ID_8FACETS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else if ( CurrSeries->facets3D < 24 )
				SendDlgItemMessage(hwndDlg, ID_16FACETS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_32FACETS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			sprintf(txt,"%1.*g",Precision,CurrSeries->dim3Da);
			SendDlgItemMessage(hwndDlg, ID_ASIZE, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->dim3Db);
			SendDlgItemMessage(hwndDlg, ID_BSIZE, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->dim3Dc);
			SendDlgItemMessage(hwndDlg, ID_CSIZE, WM_SETTEXT, 0, (LPARAM)txt);
			strcpy(txt,"<this z-trace>");
			SendDlgItemMessage(hwndDlg, ID_ZTRACEOFFSET2, CB_ADDSTRING, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_ZTRACEOFFSET2, CB_SELECTSTRING, -1, (LPARAM)txt);
			strcpy(txt,"<minus this>");
			SendDlgItemMessage(hwndDlg, ID_ZTRACEOFFSET1, CB_ADDSTRING, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_ZTRACEOFFSET1, CB_SELECTSTRING, -1, (LPARAM)txt);
			if ( CurrSeries->zcontours )
				{
				zcontour = CurrSeries->zcontours->first;			// fill z-trace list boxes
				while ( zcontour )
					{
					SendDlgItemMessage(hwndDlg, ID_ZTRACEOFFSET1, CB_ADDSTRING, 0, (LPARAM)zcontour->name);
					if ( zcontour == OffsetZTrace1 )
						SendDlgItemMessage(hwndDlg, ID_ZTRACEOFFSET1, CB_SELECTSTRING, -1, (LPARAM)zcontour->name);
					SendDlgItemMessage(hwndDlg, ID_ZTRACEOFFSET2, CB_ADDSTRING, 0, (LPARAM)zcontour->name);
					if ( zcontour == OffsetZTrace2 )
						SendDlgItemMessage(hwndDlg, ID_ZTRACEOFFSET2, CB_SELECTSTRING, -1, (LPARAM)zcontour->name);
					zcontour = zcontour->next;
					}
				}
			return TRUE;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case PSN_APPLY:
					SendDlgItemMessage(hwndDlg, ID_XOFFSET, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->x3Doffset = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_YOFFSET, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->y3Doffset = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_ZOFFSET, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->z3Doffset = atof(txt);
					if ( SendDlgItemMessage(hwndDlg, ID_3DTRACES, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->type3Dobject = TRACES_OBJECT;
					if ( SendDlgItemMessage(hwndDlg, ID_3DSURFACE, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->type3Dobject = SURFACE_OBJECT;
					if ( SendDlgItemMessage(hwndDlg, ID_3DPOINTSET, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->type3Dobject = POINTSET_OBJECT;
					if ( SendDlgItemMessage(hwndDlg, ID_3DAREAS, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->type3Dobject = AREAS_OBJECT;
					if ( SendDlgItemMessage(hwndDlg, ID_3DBOX, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->type3Dobject = BOX_OBJECT;
					if ( SendDlgItemMessage(hwndDlg, ID_3DCYLINDER, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->type3Dobject = CYLINDER_OBJECT;
					if ( SendDlgItemMessage(hwndDlg, ID_3DELLIPSOID, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->type3Dobject = ELLIPSOID_OBJECT;
					if ( SendDlgItemMessage(hwndDlg, ID_3DSPHERE, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->type3Dobject = SPHERE_OBJECT;
					SendDlgItemMessage(hwndDlg, ID_FIRSTSECTION, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->first3Dsection = atoi(txt);
					SendDlgItemMessage(hwndDlg, ID_LASTSECTION, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->last3Dsection = atoi(txt);
					SendDlgItemMessage(hwndDlg, ID_MAXCONNECTION, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->max3Dconnection = atof(txt);
					if ( SendDlgItemMessage(hwndDlg, ID_UPPERFACES, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CurrSeries->upper3Dfaces = true;
					else CurrSeries->upper3Dfaces = false;
					if ( SendDlgItemMessage(hwndDlg, ID_LOWERFACES, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CurrSeries->lower3Dfaces = true;
					else CurrSeries->lower3Dfaces = false;
					CurrSeries->faceNormals = false;
					CurrSeries->vertexNormals = false;
					if ( SendDlgItemMessage(hwndDlg, ID_FACENORMALS, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CurrSeries->faceNormals = true;
					if ( SendDlgItemMessage(hwndDlg, ID_VERTEXNORMALS, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CurrSeries->vertexNormals = true;
					if ( SendDlgItemMessage(hwndDlg, ID_4FACETS, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CurrSeries->facets3D = 4;
					if ( SendDlgItemMessage(hwndDlg, ID_8FACETS, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CurrSeries->facets3D = 8;
					if ( SendDlgItemMessage(hwndDlg, ID_16FACETS, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CurrSeries->facets3D = 16;
					if ( SendDlgItemMessage(hwndDlg, ID_32FACETS, BM_GETCHECK, 0, 0) == BST_CHECKED )
						CurrSeries->facets3D = 32;
					SendDlgItemMessage(hwndDlg, ID_ASIZE, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->dim3Da = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_BSIZE, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->dim3Db = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_CSIZE, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->dim3Dc = atof(txt);
					ApplyZOffset3D = false;
					i = SendDlgItemMessage(hwndDlg, ID_ZTRACEOFFSET1, CB_GETCURSEL, 0, 0);
					OffsetZTrace1 = NULL;
					if ( CurrSeries->zcontours )
						{
						j = 1;
						zcontour = CurrSeries->zcontours->first;		// find zcontour based on index
						while ( zcontour && (i != j) )
							{ j++; zcontour = zcontour->next; }
						OffsetZTrace1 = zcontour;
						}
					i = SendDlgItemMessage(hwndDlg, ID_ZTRACEOFFSET2, CB_GETCURSEL, 0, 0);
					OffsetZTrace2 = NULL;
					if ( CurrSeries->zcontours )
						{
						j = 1;
						zcontour = CurrSeries->zcontours->first;		// find zcontour based on index
						while ( zcontour && (i != j) )
							{ j++; zcontour = zcontour->next; }
						OffsetZTrace2 = zcontour;
						}
					if ( OffsetZTrace1 && OffsetZTrace2 ) ApplyZOffset3D = true;
					return TRUE;
				case PSN_RESET:
					return TRUE;
				}
			break;
		}
	return FALSE;
}

BOOL CALLBACK ListsDlgProc(HWND hwndDlg, UINT message, WPARAM, LPARAM lParam)
{
	NMHDR *pnmh;

	switch (message)
		{
		case WM_INITDIALOG:
			if ( LastOptionTab != 4 ) LastOptionTab = 4;	// if just opened options, center window
			else CenterDialog( appWnd, GetParent(hwndDlg) );
			SendDlgItemMessage(hwndDlg, ID_LIMITSECTIONLIST, WM_SETTEXT, 0, (LPARAM)limitSectionList);
			SendDlgItemMessage(hwndDlg, ID_LIMITDOMAINLIST, WM_SETTEXT, 0, (LPARAM)limitDomainList);
			SendDlgItemMessage(hwndDlg, ID_LIMITTRACELIST, WM_SETTEXT, 0, (LPARAM)limitTraceList);
			SendDlgItemMessage(hwndDlg, ID_LIMITOBJECTLIST, WM_SETTEXT, 0, (LPARAM)limitObjectList);
			SendDlgItemMessage(hwndDlg, ID_LEFTDISTANCELIST, WM_SETTEXT, 0, (LPARAM)limitLeftDistanceList);
			SendDlgItemMessage(hwndDlg, ID_RIGHTDISTANCELIST, WM_SETTEXT, 0, (LPARAM)limitRightDistanceList);
			SendDlgItemMessage(hwndDlg, ID_LIMITZTRACELIST, WM_SETTEXT, 0, (LPARAM)limitZTraceList);
			if ( CurrSeries->listSectionThickness ) SendDlgItemMessage(hwndDlg, ID_LISTSECTIONTHICKNESS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTSECTIONTHICKNESS, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listDomainSource ) SendDlgItemMessage(hwndDlg, ID_LISTDOMAINSOURCE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTDOMAINSOURCE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listDomainPixelsize ) SendDlgItemMessage(hwndDlg, ID_LISTDOMAINPIXELSIZE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTDOMAINPIXELSIZE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listDomainLength ) SendDlgItemMessage(hwndDlg, ID_LISTDOMAINLENGTH, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTDOMAINLENGTH, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listDomainArea ) SendDlgItemMessage(hwndDlg, ID_LISTDOMAINAREA, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTDOMAINAREA, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listDomainMidpoint ) SendDlgItemMessage(hwndDlg, ID_LISTDOMAINMIDPOINT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTDOMAINMIDPOINT, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listTraceComment ) SendDlgItemMessage(hwndDlg, ID_LISTTRACECOMMENT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTTRACECOMMENT, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listTraceLength ) SendDlgItemMessage(hwndDlg, ID_LISTTRACELENGTH, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTTRACELENGTH, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listTraceArea ) SendDlgItemMessage(hwndDlg, ID_LISTTRACEAREA, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTTRACEAREA, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listTraceCentroid ) SendDlgItemMessage(hwndDlg, ID_LISTTRACECENTROID, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTTRACECENTROID, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listTraceExtent ) SendDlgItemMessage(hwndDlg, ID_LISTTRACEEXTENT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTTRACEEXTENT, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listTraceZ ) SendDlgItemMessage(hwndDlg, ID_LISTTRACEZ, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTTRACEZ, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listTraceThickness ) SendDlgItemMessage(hwndDlg, ID_LISTTRACETHICKNESS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTTRACETHICKNESS, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listObjectRange ) SendDlgItemMessage(hwndDlg, ID_LISTOBJECTRANGE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTOBJECTRANGE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listObjectCount ) SendDlgItemMessage(hwndDlg, ID_LISTOBJECTCOUNT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTOBJECTCOUNT, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listObjectSurfarea ) SendDlgItemMessage(hwndDlg, ID_LISTOBJECTSURFACEAREA, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTOBJECTSURFACEAREA, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listObjectFlatarea ) SendDlgItemMessage(hwndDlg, ID_LISTOBJECTFLATAREA, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTOBJECTFLATAREA, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listObjectVolume ) SendDlgItemMessage(hwndDlg, ID_LISTOBJECTVOLUME, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTOBJECTVOLUME, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listZTraceNote ) SendDlgItemMessage(hwndDlg, ID_LISTZTRACENOTE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTZTRACENOTE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listZTraceRange ) SendDlgItemMessage(hwndDlg, ID_LISTZTRACERANGE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTZTRACERANGE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->listZTraceLength ) SendDlgItemMessage(hwndDlg, ID_LISTZTRACELENGTH, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_LISTZTRACELENGTH, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			return TRUE;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{										// user clicks OK, update changes
				case PSN_APPLY:
					SendDlgItemMessage(hwndDlg, ID_LIMITSECTIONLIST, WM_GETTEXT, (WPARAM)MAX_CONTOUR_NAME, (LPARAM)limitSectionList);
					SendDlgItemMessage(hwndDlg, ID_LIMITDOMAINLIST, WM_GETTEXT, (WPARAM)MAX_CONTOUR_NAME, (LPARAM)limitDomainList);
					SendDlgItemMessage(hwndDlg, ID_LIMITTRACELIST, WM_GETTEXT, (WPARAM)MAX_CONTOUR_NAME, (LPARAM)limitTraceList);
					SendDlgItemMessage(hwndDlg, ID_LIMITOBJECTLIST, WM_GETTEXT, (WPARAM)MAX_CONTOUR_NAME, (LPARAM)limitObjectList);
					SendDlgItemMessage(hwndDlg, ID_LEFTDISTANCELIST, WM_GETTEXT, (WPARAM)MAX_CONTOUR_NAME, (LPARAM)limitLeftDistanceList);
					SendDlgItemMessage(hwndDlg, ID_RIGHTDISTANCELIST, WM_GETTEXT, (WPARAM)MAX_CONTOUR_NAME, (LPARAM)limitRightDistanceList);
					SendDlgItemMessage(hwndDlg, ID_LIMITZTRACELIST, WM_GETTEXT, (WPARAM)MAX_CONTOUR_NAME, (LPARAM)limitZTraceList);
					CurrSeries->listSectionThickness = ( SendDlgItemMessage(hwndDlg, ID_LISTSECTIONTHICKNESS, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listDomainSource = ( SendDlgItemMessage(hwndDlg, ID_LISTDOMAINSOURCE, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listDomainPixelsize = ( SendDlgItemMessage(hwndDlg, ID_LISTDOMAINPIXELSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listDomainLength = ( SendDlgItemMessage(hwndDlg, ID_LISTDOMAINLENGTH, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listDomainArea = ( SendDlgItemMessage(hwndDlg, ID_LISTDOMAINAREA, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listDomainMidpoint = ( SendDlgItemMessage(hwndDlg, ID_LISTDOMAINMIDPOINT, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listTraceComment = ( SendDlgItemMessage(hwndDlg, ID_LISTTRACECOMMENT, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listTraceLength = ( SendDlgItemMessage(hwndDlg, ID_LISTTRACELENGTH, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listTraceArea = ( SendDlgItemMessage(hwndDlg, ID_LISTTRACEAREA, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listTraceCentroid = ( SendDlgItemMessage(hwndDlg, ID_LISTTRACECENTROID, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listTraceExtent = ( SendDlgItemMessage(hwndDlg, ID_LISTTRACEEXTENT, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listTraceZ = ( SendDlgItemMessage(hwndDlg, ID_LISTTRACEZ, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listTraceThickness = ( SendDlgItemMessage(hwndDlg, ID_LISTTRACETHICKNESS, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listObjectRange = ( SendDlgItemMessage(hwndDlg, ID_LISTOBJECTRANGE, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listObjectCount = ( SendDlgItemMessage(hwndDlg, ID_LISTOBJECTCOUNT, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listObjectSurfarea = ( SendDlgItemMessage(hwndDlg, ID_LISTOBJECTSURFACEAREA, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listObjectFlatarea = ( SendDlgItemMessage(hwndDlg, ID_LISTOBJECTFLATAREA, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listObjectVolume = ( SendDlgItemMessage(hwndDlg, ID_LISTOBJECTVOLUME, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listZTraceNote = ( SendDlgItemMessage(hwndDlg, ID_LISTZTRACENOTE, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listZTraceRange = ( SendDlgItemMessage(hwndDlg, ID_LISTZTRACERANGE, BM_GETCHECK, 0, 0) == BST_CHECKED );
					CurrSeries->listZTraceLength = ( SendDlgItemMessage(hwndDlg, ID_LISTZTRACELENGTH, BM_GETCHECK, 0, 0) == BST_CHECKED );
					return TRUE;
				case PSN_RESET:
					return TRUE;
				}
			break;
		}
	return FALSE;
}


BOOL CALLBACK MovementsDlgProc(HWND hwndDlg, UINT message, WPARAM, LPARAM lParam)
{
	NMHDR *pnmh;
	char txt[MAX_DIGITS];

	switch (message)
		{
		case WM_INITDIALOG:
			if ( LastOptionTab != 5 ) LastOptionTab = 5;	// if just opened options, center window
			else CenterDialog( appWnd, GetParent(hwndDlg) );
			sprintf(txt,"%s",CurrSeries->units);		// fill units descriptors
			SetDlgItemText( hwndDlg, ID_UNITS1, txt );
			SetDlgItemText( hwndDlg, ID_UNITS2, txt );
			sprintf(txt,"1/%s",CurrSeries->units);
			SetDlgItemText( hwndDlg, ID_UNITS3, txt );	// then set button states
			SetDlgItemText( hwndDlg, ID_UNITS4, txt );
			if ( UseDeformKeys ) SendDlgItemMessage(hwndDlg, ID_DEFORMKEYS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_DEFORMKEYS, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			sprintf(txt,"%1.*g",Precision,CurrSeries->Increment.transX);			// fill all increment fields
			SendDlgItemMessage(hwndDlg, ID_TRANSX, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->Increment.transY);
			SendDlgItemMessage(hwndDlg, ID_TRANSY, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,180.0*CurrSeries->Increment.theta/PI);		// show to user in degrees
			SendDlgItemMessage(hwndDlg, ID_THETA, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->Increment.scaleX);
			SendDlgItemMessage(hwndDlg, ID_SCALEX, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->Increment.scaleY);
			SendDlgItemMessage(hwndDlg, ID_SCALEY, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->Increment.slantX);
			SendDlgItemMessage(hwndDlg, ID_SLANTX, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->Increment.slantY);
			SendDlgItemMessage(hwndDlg, ID_SLANTY, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->Increment.deformX);
			SendDlgItemMessage(hwndDlg, ID_DEFORMX, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->Increment.deformY);
			SendDlgItemMessage(hwndDlg, ID_DEFORMY, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->ShiftIncrement.transX);
			SendDlgItemMessage(hwndDlg, ID_TRANSXSHFT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->ShiftIncrement.transY);
			SendDlgItemMessage(hwndDlg, ID_TRANSYSHFT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,180.0*CurrSeries->ShiftIncrement.theta/PI);
			SendDlgItemMessage(hwndDlg, ID_THETASHFT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->ShiftIncrement.scaleX);
			SendDlgItemMessage(hwndDlg, ID_SCALEXSHFT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->ShiftIncrement.scaleY);
			SendDlgItemMessage(hwndDlg, ID_SCALEYSHFT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->ShiftIncrement.slantX);
			SendDlgItemMessage(hwndDlg, ID_SLANTXSHFT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->ShiftIncrement.slantY);
			SendDlgItemMessage(hwndDlg, ID_SLANTYSHFT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->ShiftIncrement.deformX);
			SendDlgItemMessage(hwndDlg, ID_DEFORMXSHFT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->ShiftIncrement.deformY);
			SendDlgItemMessage(hwndDlg, ID_DEFORMYSHFT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->CtrlIncrement.transX);
			SendDlgItemMessage(hwndDlg, ID_TRANSXCTRL, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->CtrlIncrement.transY);
			SendDlgItemMessage(hwndDlg, ID_TRANSYCTRL, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,180.0*CurrSeries->CtrlIncrement.theta/PI);
			SendDlgItemMessage(hwndDlg, ID_THETACTRL, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->CtrlIncrement.scaleX);
			SendDlgItemMessage(hwndDlg, ID_SCALEXCTRL, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->CtrlIncrement.scaleY);
			SendDlgItemMessage(hwndDlg, ID_SCALEYCTRL, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->CtrlIncrement.slantX);
			SendDlgItemMessage(hwndDlg, ID_SLANTXCTRL, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->CtrlIncrement.slantY);
			SendDlgItemMessage(hwndDlg, ID_SLANTYCTRL, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->CtrlIncrement.deformX);
			SendDlgItemMessage(hwndDlg, ID_DEFORMXCTRL, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->CtrlIncrement.deformY);
			SendDlgItemMessage(hwndDlg, ID_DEFORMYCTRL, WM_SETTEXT, 0, (LPARAM)txt);
			return TRUE;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case PSN_APPLY:
					if ( SendDlgItemMessage(hwndDlg, ID_DEFORMKEYS, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						UseDeformKeys = true;
					else UseDeformKeys = false;
					SendDlgItemMessage(hwndDlg, ID_TRANSX, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->Increment.transX = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_TRANSY, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->Increment.transY = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_THETA, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->Increment.theta = PI*atof( txt )/180.0;				// convert to clkwise radians
					SendDlgItemMessage(hwndDlg, ID_SCALEX, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->Increment.scaleX = atof(txt);
					if ( CurrSeries->Increment.scaleX < MIN_SCALE ) CurrSeries->Increment.scaleX = MIN_SCALE;
					SendDlgItemMessage(hwndDlg, ID_SCALEY, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->Increment.scaleY = atof(txt);
					if ( CurrSeries->Increment.scaleY < MIN_SCALE ) CurrSeries->Increment.scaleY = MIN_SCALE;
					SendDlgItemMessage(hwndDlg, ID_SLANTX, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->Increment.slantX = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_SLANTY, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->Increment.slantY = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_DEFORMX, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->Increment.deformX = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_DEFORMY, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->Increment.deformY = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_TRANSXSHFT, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->ShiftIncrement.transX = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_TRANSYSHFT, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->ShiftIncrement.transY = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_THETASHFT, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->ShiftIncrement.theta = PI*atof( txt )/180.0;				// convert to clkwise radians
					SendDlgItemMessage(hwndDlg, ID_SCALEXSHFT, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->ShiftIncrement.scaleX = atof(txt);
					if ( CurrSeries->ShiftIncrement.scaleX < MIN_SCALE ) CurrSeries->ShiftIncrement.scaleX = MIN_SCALE;
					SendDlgItemMessage(hwndDlg, ID_SCALEYSHFT, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->ShiftIncrement.scaleY = atof(txt);
					if ( CurrSeries->ShiftIncrement.scaleY < MIN_SCALE ) CurrSeries->ShiftIncrement.scaleY = MIN_SCALE;
					SendDlgItemMessage(hwndDlg, ID_SLANTXSHFT, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->ShiftIncrement.slantX = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_SLANTYSHFT, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->ShiftIncrement.slantY = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_DEFORMXSHFT, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->ShiftIncrement.deformX = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_DEFORMYSHFT, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->ShiftIncrement.deformY = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_TRANSXCTRL, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->CtrlIncrement.transX = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_TRANSYCTRL, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->CtrlIncrement.transY = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_THETACTRL, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->CtrlIncrement.theta = PI*atof( txt )/180.0;				// convert to clkwise radians
					SendDlgItemMessage(hwndDlg, ID_SCALEXCTRL, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->CtrlIncrement.scaleX = atof(txt);
					if ( CurrSeries->CtrlIncrement.scaleX < MIN_SCALE ) CurrSeries->CtrlIncrement.scaleX = MIN_SCALE;
					SendDlgItemMessage(hwndDlg, ID_SCALEYCTRL, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->CtrlIncrement.scaleY = atof(txt);
					if ( CurrSeries->CtrlIncrement.scaleY < MIN_SCALE ) CurrSeries->CtrlIncrement.scaleY = MIN_SCALE;
					SendDlgItemMessage(hwndDlg, ID_SLANTXCTRL, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->CtrlIncrement.slantX = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_SLANTYCTRL, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->CtrlIncrement.slantY = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_DEFORMXCTRL, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->CtrlIncrement.deformX = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_DEFORMYCTRL, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->CtrlIncrement.deformY = atof(txt);
					return TRUE;
				case PSN_RESET:
					return TRUE;
				}
			break;
		}
	return FALSE;
}

void CountProxies( HWND hwndDlg )		// Don't operate on memory sections, just disk so result will show up when paging
{
	HCURSOR cur;								
	int ft, w, h, bpp, n, sectnum, percentage, NumberOfImages, NumberOfProxies;
	char filename[MAX_PATH], txt[80];
	Section *section;
	Transform *transform;

	cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );			// show hourglass, file access could be slow
	
	NumberOfProxies = 0;									// set globals so can report these
	NumberOfImages = 0;
	sectnum = -1;
	while ( FindSection( sectnum, 1, filename ) )			// do all sections in series
		{
		section = new Section( filename );					// read section data
		if ( section )
		  if ( section->transforms )						// do all transforms in section
			{
			transform = section->transforms->first;
			while ( transform )
				{
				if ( transform->image )						// found image, test it
					{
					ft = TestGBMfile( transform->image->src, &w, &h, &bpp, &n );
					if ( ft && ((w > CurrSeries->widthUseProxies) || (h > CurrSeries->heightUseProxies)) )
						{
						NumberOfImages++;					// count it and proxy
						if ( transform->image->HasProxy() ) NumberOfProxies++;
						}
					}
				transform = transform->next;				// continue with next transform
				}
			}
		delete section;
															// show progess in dialog
		sprintf(txt,"%d/%d",NumberOfProxies,NumberOfImages);
		SendDlgItemMessage(hwndDlg, ID_PERCENTPROXIES, WM_SETTEXT, 0, (LPARAM)txt);
		}

	if ( NumberOfImages ) percentage=100*NumberOfProxies/NumberOfImages; // update progress
	else percentage = 0;
	sprintf(txt,"%d",percentage);
	SendDlgItemMessage(hwndDlg, ID_PERCENTPROXIES, WM_SETTEXT, 0, (LPARAM)txt);
	SetCursor( cur );										// restore cursor
}

void CreateProxies( HWND hwndDlg )					// update % of images with proxies while creating
{
	HCURSOR cur;
	int ft, w, h, bpp, n, percentage, sectnum, NumberOfImages, NumberOfProxies;
	char filename[MAX_PATH], txt[20];
	Section *section;
	Transform *transform;

	cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );			// show hourglass, file access could be slow
    NumberOfImages = 0;
	NumberOfProxies = 0;									// reset counts, increment as create
	sectnum = -1;
	while ( FindSection( sectnum, 1, filename ) )			// do all sections in series
		{
		section = new Section( filename );					// read section data
		if ( section )
		  if ( section->transforms )						// do all transforms in section
			{
			transform = section->transforms->first;
			while ( transform )
				{
				if ( transform->image )						// found image
					{
					NumberOfImages++;						// count it and make proxy
					ft = TestGBMfile( transform->image->src, &w, &h, &bpp, &n );
					if ( ft && ((w > CurrSeries->widthUseProxies) || (h > CurrSeries->heightUseProxies)) )
						{
						section->hasChanged = true;
						transform->image->MakeProxy( CurrSeries->scaleProxies );
						if ( transform->image->adib ) { delete transform->image->adib; transform->image->adib = NULL; }
						NumberOfProxies++;
						}
					}
				transform = transform->next;				// continue with next transform
				}
			} // NOTE: IF SAVE TO DISK HERE, THEN USER SAVES SECTION FROM MEMORY => NO PROXIES CREATED!!!!
		if ( section->hasChanged ) section->Save( filename );
		delete section;

		sprintf(txt,"%d/%d",NumberOfProxies,NumberOfImages);
		SendDlgItemMessage(hwndDlg, ID_PERCENTPROXIES, WM_SETTEXT, 0, (LPARAM)txt);
		}

    if ( NumberOfImages ) percentage=100*NumberOfProxies/NumberOfImages; // update progress
	else percentage = 0;
	sprintf(txt,"%d",percentage);
	SendDlgItemMessage(hwndDlg, ID_PERCENTPROXIES, WM_SETTEXT, 0, (LPARAM)txt);
	SetCursor( cur );										// restore cursor
}

void DeleteProxies( HWND hwndDlg )
{
	HCURSOR cur;
	int sectnum, percentage;
	char filename[MAX_PATH], txt[20];
	Section *section;
	Transform *transform;

	cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );			// show hourglass, file access could be slow

	sectnum = -1;
	while ( FindSection( sectnum, 1, filename ) )			// do all sections in series
		{
		section = new Section( filename );					// read section data
		if ( section )
		  if ( section->transforms )						// do all transforms in section
			{
			transform = section->transforms->first;
			while ( transform )
				{
				if ( transform->image )						// found image, delete proxy if present
					{
					section->hasChanged = true;
					transform->image->DeleteProxy();
					}
				transform = transform->next;				// continue with next transform
				}
			}
		if ( section->hasChanged ) section->Save( filename );
		delete section;

		percentage = 0;
		sprintf(txt,"%d",percentage);
		SendDlgItemMessage(hwndDlg, ID_PERCENTPROXIES, WM_SETTEXT, 0, (LPARAM)txt);
		}

	SetCursor( cur );										// restore cursor
}

BOOL CALLBACK ProxiesDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pnmh;
	int percentage;
	char txt[MAX_PATH];

	switch (message)
		{
		case WM_INITDIALOG:
			if ( LastOptionTab != 6 ) LastOptionTab = 6;	// if just opened options, center window
			else CenterDialog( appWnd, GetParent(hwndDlg) );
			if ( CurrSeries->useProxies ) SendDlgItemMessage(hwndDlg, ID_USEPROXIES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_USEPROXIES, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			sprintf(txt,"%d",CurrSeries->widthUseProxies);
			SendDlgItemMessage(hwndDlg, ID_WIDTH, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%d",CurrSeries->heightUseProxies);
			SendDlgItemMessage(hwndDlg, ID_HEIGHT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%1.*g",Precision,CurrSeries->scaleProxies);
			SendDlgItemMessage(hwndDlg, ID_SCALE, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"??");
			SendDlgItemMessage(hwndDlg, ID_PERCENTPROXIES, WM_SETTEXT, 0, (LPARAM)txt);
			return TRUE;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case PSN_APPLY:
					if ( SendDlgItemMessage(hwndDlg, ID_USEPROXIES, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->useProxies = true;
					else CurrSeries->useProxies = false;
					SendDlgItemMessage(hwndDlg, ID_WIDTH, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->widthUseProxies = atoi(txt);
					if ( CurrSeries->widthUseProxies < 256 ) CurrSeries->widthUseProxies = 256;
					SendDlgItemMessage(hwndDlg, ID_HEIGHT, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->heightUseProxies = atoi(txt);
					if ( CurrSeries->heightUseProxies < 192 ) CurrSeries->heightUseProxies = 192;
					SendDlgItemMessage(hwndDlg, ID_SCALE, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->scaleProxies = atof(txt);
					if ( CurrSeries->scaleProxies < 0.01 ) CurrSeries->scaleProxies = 0.01;
					return TRUE;
				case PSN_RESET:
					return TRUE;
				}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case ID_COUNTPROXIES:		// find out how many proxies are defined
					CountProxies( hwndDlg );
					MessageBeep( MB_ICONEXCLAMATION );
					return TRUE;
				case ID_CREATEPROXIES:
					SendDlgItemMessage(hwndDlg, ID_WIDTH, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->widthUseProxies = atoi(txt);
					if ( CurrSeries->widthUseProxies < 256 ) CurrSeries->widthUseProxies = 256;
					SendDlgItemMessage(hwndDlg, ID_HEIGHT, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->heightUseProxies = atoi(txt);
					if ( CurrSeries->heightUseProxies < 192 ) CurrSeries->heightUseProxies = 192;
					SendDlgItemMessage(hwndDlg, ID_SCALE, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->scaleProxies = atof(txt);
					if ( CurrSeries->scaleProxies < 0.01 ) CurrSeries->scaleProxies = 0.01;
					CreateProxies( hwndDlg );
					MessageBeep( MB_ICONEXCLAMATION );
					return TRUE;
				case ID_DELETEPROXIES:
					DeleteProxies( hwndDlg );
					MessageBeep( MB_ICONEXCLAMATION );
					return TRUE;
				}

		}
	return FALSE;
}


BOOL CALLBACK ThumbnailsDlgProc(HWND hwndDlg, UINT message, WPARAM, LPARAM lParam)
{
	NMHDR *pnmh;
	char txt[MAX_PATH];

	switch (message)
		{
		case WM_INITDIALOG:
			if ( LastOptionTab != 7 ) LastOptionTab = 7;	// if just opened options, center window
			else CenterDialog( appWnd, GetParent(hwndDlg) );
			sprintf(txt,"%d",CurrSeries->thumbWidth);
			SendDlgItemMessage(hwndDlg, ID_WIDTH, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%d",CurrSeries->thumbHeight);							// THUMBFIT and THUMBVIEW are mutually exclusive!
			SendDlgItemMessage(hwndDlg, ID_HEIGHT, WM_SETTEXT, 0, (LPARAM)txt);
			if ( CurrSeries->fitThumbSections ) SendDlgItemMessage(hwndDlg, ID_THUMBFIT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_THUMBVIEW, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			sprintf(txt,"%d",CurrSeries->firstThumbSection);
			SendDlgItemMessage(hwndDlg, ID_FIRSTSECTION, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%d",CurrSeries->lastThumbSection);
			SendDlgItemMessage(hwndDlg, ID_LASTSECTION, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%d",CurrSeries->skipSections);
			SendDlgItemMessage(hwndDlg, ID_SKIPSECTIONS, WM_SETTEXT, 0, (LPARAM)txt);
			if ( CurrSeries->displayThumbContours ) SendDlgItemMessage(hwndDlg, ID_THUMBCONTOURS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_THUMBCONTOURS, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrSeries->useFlipbookStyle ) SendDlgItemMessage(hwndDlg, ID_FLIPBOOKSTYLE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_FLIPBOOKSTYLE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			sprintf(txt,"%d",CurrSeries->flipRate);
			SendDlgItemMessage(hwndDlg, ID_FLIPRATE, WM_SETTEXT, 0, (LPARAM)txt);
			return TRUE;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{						// user clicks OK, update changes
				case PSN_APPLY:
					SendDlgItemMessage(hwndDlg, ID_WIDTH, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->thumbWidth = atoi(txt);
					if ( CurrSeries->thumbWidth < 10 ) CurrSeries->thumbWidth = 10;
					SendDlgItemMessage(hwndDlg, ID_HEIGHT, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->thumbHeight = atoi(txt);
					if ( CurrSeries->thumbHeight < 7 ) CurrSeries->thumbHeight = 7;
					if ( SendDlgItemMessage(hwndDlg, ID_THUMBFIT, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->fitThumbSections = true;
					else CurrSeries->fitThumbSections = false;
					SendDlgItemMessage(hwndDlg, ID_FIRSTSECTION, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->firstThumbSection = atoi(txt);
					SendDlgItemMessage(hwndDlg, ID_LASTSECTION, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->lastThumbSection = atoi(txt);
					SendDlgItemMessage(hwndDlg, ID_SKIPSECTIONS, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->skipSections = atoi(txt);
					if ( CurrSeries->skipSections < 1 ) CurrSeries->skipSections = 1;
					if ( SendDlgItemMessage(hwndDlg, ID_THUMBCONTOURS, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->displayThumbContours = true;
					else CurrSeries->displayThumbContours = false;
					if ( SendDlgItemMessage(hwndDlg, ID_FLIPBOOKSTYLE, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						CurrSeries->useFlipbookStyle = true;
					else CurrSeries->useFlipbookStyle = false;
					SendDlgItemMessage(hwndDlg, ID_FLIPRATE, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)txt);
					CurrSeries->flipRate = atoi(txt);
					if ( CurrSeries->flipRate < 1 ) CurrSeries->flipRate = 1;
					return TRUE;
				case PSN_RESET:
					return TRUE;
				}
			break;
		}
	return FALSE;
}

BOOL CALLBACK AutoTracingDlgProc(HWND hwndDlg, UINT message, WPARAM, LPARAM lParam)
{
	NMHDR *pnmh;
	NM_UPDOWN *pnmud;
	int i;
	char txt[MAX_DIGITS];

	switch (message)
		{
		case WM_INITDIALOG:
			if ( LastOptionTab != 8 ) 
				LastOptionTab = 8;	// if just opened options, center window
			else 
				CenterDialog( appWnd, GetParent(hwndDlg) );
			if ( AutoSimplify ) 
				SendDlgItemMessage(hwndDlg, ID_AUTOSIMPLIFY, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else 
				SendDlgItemMessage(hwndDlg, ID_AUTOSIMPLIFY, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			sprintf(txt,"%1.*g",Precision,SimplifyResolution);
			SendDlgItemMessage(hwndDlg, ID_SIMPLIFYRESOLUTION, WM_SETTEXT, 0, (LPARAM)txt);
			SetDlgItemText( hwndDlg, ID_UNITS, CurrSeries->units );
			if ( AutoShrinkBack ) 
				SendDlgItemMessage(hwndDlg, ID_SHRINKBACK, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else 
				SendDlgItemMessage(hwndDlg, ID_SHRINKBACK, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			sprintf(txt,"%d",CurrSeries->smoothingLength);
			SendDlgItemMessage(hwndDlg, ID_SMOOTHINGLENGTH, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg,ID_HUESTOPCHANGE,UDM_SETRANGE,0,(LPARAM) MAKELONG(3,0));
			SendDlgItemMessage(hwndDlg,ID_HUESTOPCHANGE,UDM_SETPOS,0,(LPARAM) MAKELONG((short)CurrSeries->hueStopWhen,0));
			sprintf(txt,"%.15s",wildfireStopTypes+CurrSeries->hueStopWhen*15);
			SendDlgItemMessage(hwndDlg, ID_HUESTOPTYPE, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%d",CurrSeries->hueStopValue);
			SendDlgItemMessage(hwndDlg, ID_HUESTOPVALUE, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg,ID_SATSTOPCHANGE,UDM_SETRANGE,0,(LPARAM) MAKELONG(3,0));
			SendDlgItemMessage(hwndDlg,ID_SATSTOPCHANGE,UDM_SETPOS,0,(LPARAM) MAKELONG((short)CurrSeries->satStopWhen,0));
			sprintf(txt,"%.15s",wildfireStopTypes+CurrSeries->satStopWhen*15);
			SendDlgItemMessage(hwndDlg, ID_SATSTOPTYPE, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%d",CurrSeries->satStopValue);
			SendDlgItemMessage(hwndDlg, ID_SATSTOPVALUE, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg,ID_BRIGHTSTOPCHANGE,UDM_SETRANGE,0,(LPARAM) MAKELONG(3,0));
			SendDlgItemMessage(hwndDlg,ID_BRIGHTSTOPCHANGE,UDM_SETPOS,0,(LPARAM) MAKELONG((short)CurrSeries->brightStopWhen,0));
			sprintf(txt,"%.15s",wildfireStopTypes+CurrSeries->brightStopWhen*15);
			SendDlgItemMessage(hwndDlg, ID_BRIGHTSTOPTYPE, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%d",CurrSeries->brightStopValue);
			SendDlgItemMessage(hwndDlg, ID_BRIGHTSTOPVALUE, WM_SETTEXT, 0, (LPARAM)txt);
			if ( CurrSeries->tracesStopWhen ) 
				SendDlgItemMessage(hwndDlg, ID_STOPATTRACES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else 
				SendDlgItemMessage(hwndDlg, ID_STOPATTRACES, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			sprintf(txt,"%g",CurrSeries->areaStopPercent);
			SendDlgItemMessage(hwndDlg, ID_AREASTOPPERCENT, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%g",CurrSeries->areaStopSize);
			SendDlgItemMessage(hwndDlg, ID_AREASTOPSIZE, WM_SETTEXT, 0, (LPARAM)txt);

			sprintf(txt,"%d",CurrSeries->ContourMaskWidth);		
			SendDlgItemMessage(hwndDlg, ID_CONTOURMASKWIDTH, WM_SETTEXT, 0, (LPARAM)txt);

			if ( AutoAdjustThreshold ) 
				SendDlgItemMessage(hwndDlg, ID_AUTOADJUSTTHRESHOLD, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else 
				SendDlgItemMessage(hwndDlg, ID_AUTOADJUSTTHRESHOLD, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

			return TRUE;

		case WM_NOTIFY:
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{											// change type of limit check for wildfires
				case UDN_DELTAPOS:
					pnmud = (NM_UPDOWN *)lParam;
					i = pnmud->iPos + pnmud->iDelta;
					if ( (i >= 0) && (i<=3) )
					  {
					  sprintf(txt,"%.15s",wildfireStopTypes+i*15);
					  if ( pnmh->idFrom == ID_HUESTOPCHANGE )
						{
						SendDlgItemMessage(hwndDlg, ID_HUESTOPTYPE, WM_SETTEXT, 0, (LPARAM)txt);
						}
					  if ( pnmh->idFrom == ID_SATSTOPCHANGE )
						{
						SendDlgItemMessage(hwndDlg, ID_SATSTOPTYPE, WM_SETTEXT, 0, (LPARAM)txt);
						}
					  if ( pnmh->idFrom == ID_BRIGHTSTOPCHANGE )
						{
						SendDlgItemMessage(hwndDlg, ID_BRIGHTSTOPTYPE, WM_SETTEXT, 0, (LPARAM)txt);
						}
					  }
					return FALSE;
														// user clicks OK, update changes
				case PSN_APPLY:
					AutoSimplify = ( SendDlgItemMessage(hwndDlg, ID_AUTOSIMPLIFY, BM_GETCHECK, 0, 0) == BST_CHECKED );
					SendDlgItemMessage(hwndDlg, ID_SIMPLIFYRESOLUTION, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					SimplifyResolution = atof(txt);
					AutoShrinkBack = ( SendDlgItemMessage(hwndDlg, ID_SHRINKBACK, BM_GETCHECK, 0, 0) == BST_CHECKED );
					SendDlgItemMessage(hwndDlg, ID_SMOOTHINGLENGTH, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->smoothingLength = atoi(txt);
					if ( CurrSeries->smoothingLength < 1 ) CurrSeries->smoothingLength = 1;
					i = SendDlgItemMessage(hwndDlg,ID_HUESTOPCHANGE,UDM_GETPOS,0,0);
					CurrSeries->hueStopWhen = LOWORD(i);
					SendDlgItemMessage(hwndDlg, ID_HUESTOPVALUE, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					i = atoi(txt);
					if ( (i >= 0 ) && (i <= 255) ) CurrSeries->hueStopValue = i;
					i = SendDlgItemMessage(hwndDlg,ID_SATSTOPCHANGE,UDM_GETPOS,0,0);
					CurrSeries->satStopWhen = LOWORD(i);
					SendDlgItemMessage(hwndDlg, ID_SATSTOPVALUE, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					i = atoi(txt);
					if ( (i >= 0 ) && (i <= 255) ) CurrSeries->satStopValue = i;
					i = SendDlgItemMessage(hwndDlg,ID_BRIGHTSTOPCHANGE,UDM_GETPOS,0,0);
					CurrSeries->brightStopWhen = LOWORD(i);
					SendDlgItemMessage(hwndDlg, ID_BRIGHTSTOPVALUE, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					i = atoi(txt);
					if ( (i >= 0 ) && (i <= 255) ) CurrSeries->brightStopValue = i;
					CurrSeries->tracesStopWhen = ( SendDlgItemMessage(hwndDlg, ID_STOPATTRACES, BM_GETCHECK, 0, 0) == BST_CHECKED );
					SendDlgItemMessage(hwndDlg, ID_AREASTOPPERCENT, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->areaStopPercent = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_AREASTOPSIZE, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->areaStopSize = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_CONTOURMASKWIDTH, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					CurrSeries->ContourMaskWidth = int(atof(txt));	
					AutoAdjustThreshold = ( SendDlgItemMessage(hwndDlg, ID_AUTOADJUSTTHRESHOLD, BM_GETCHECK, 0, 0) == BST_CHECKED );
					return TRUE;
				case PSN_RESET:
					return TRUE;
				}
			break;
		}
	return FALSE;
}

void CmSetOptions( void )				// set the program option flags
{
	HWND hwndDlg;
    PROPSHEETPAGE psp[9];
    PROPSHEETHEADER psh;
											// setup tabs of dialog
    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_DEFAULT;
    psp[0].hInstance = appInstance;
    psp[0].pszTemplate = "GeneralDlg";
    psp[0].pfnDlgProc = GeneralDlgProc;
    psp[0].lParam = 0;
    psp[0].pfnCallback = NULL;

    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_DEFAULT;
    psp[1].hInstance = appInstance;
    psp[1].pszTemplate = "NamesColorsDlg";
    psp[1].pfnDlgProc = NamesColorsDlgProc;
    psp[1].lParam = 0;
    psp[1].pfnCallback = NULL;

    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = PSP_DEFAULT;
    psp[2].hInstance = appInstance;
    psp[2].pszTemplate = "GridsDlg";
    psp[2].pfnDlgProc = GridsDlgProc;
    psp[2].lParam = 0;
    psp[2].pfnCallback = NULL;

    psp[3].dwSize = sizeof(PROPSHEETPAGE);
    psp[3].dwFlags = PSP_DEFAULT;
    psp[3].hInstance = appInstance;
    psp[3].pszTemplate = "ThreedDlg";
    psp[3].pfnDlgProc = ThreedDlgProc;
    psp[3].lParam = 0;
    psp[3].pfnCallback = NULL;

    psp[4].dwSize = sizeof(PROPSHEETPAGE);
    psp[4].dwFlags = PSP_DEFAULT;
    psp[4].hInstance = appInstance;
    psp[4].pszTemplate = "ListsDlg";
    psp[4].pfnDlgProc = ListsDlgProc;
    psp[4].lParam = 0;
    psp[4].pfnCallback = NULL;

    psp[5].dwSize = sizeof(PROPSHEETPAGE);
    psp[5].dwFlags = PSP_DEFAULT;
    psp[5].hInstance = appInstance;
    psp[5].pszTemplate = "MovementsDlg";
    psp[5].pfnDlgProc = MovementsDlgProc;
    psp[5].lParam = 0;
    psp[5].pfnCallback = NULL;

    psp[6].dwSize = sizeof(PROPSHEETPAGE);
    psp[6].dwFlags = PSP_DEFAULT;
    psp[6].hInstance = appInstance;
    psp[6].pszTemplate = "ProxiesDlg";
    psp[6].pfnDlgProc = ProxiesDlgProc;
    psp[6].lParam = 0;
    psp[6].pfnCallback = NULL;

    psp[7].dwSize = sizeof(PROPSHEETPAGE);
    psp[7].dwFlags = PSP_DEFAULT;
    psp[7].hInstance = appInstance;
    psp[7].pszTemplate = "ThumbnailsDlg";
    psp[7].pfnDlgProc = ThumbnailsDlgProc;
    psp[7].lParam = 0;
    psp[7].pfnCallback = NULL;

    psp[8].dwSize = sizeof(PROPSHEETPAGE);
    psp[8].dwFlags = PSP_DEFAULT;
    psp[8].hInstance = appInstance;
    psp[8].pszTemplate = "AutoTracingDlg";
    psp[8].pfnDlgProc = AutoTracingDlgProc;
    psp[8].lParam = 0;
    psp[8].pfnCallback = NULL;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_NOAPPLYNOW;
    psh.hwndParent = appWnd;
    psh.hInstance = appInstance;
    psh.pszCaption = (LPSTR) "Series Options";
	psh.pszIcon = "ASmallIcon";
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = LastOptionTab;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
    psh.pfnCallback = NULL;

	if ( CurrSeries )				// don't open dialog if series not open
		PropertySheet(&psh);

    return;
}

