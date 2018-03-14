//////////////////////////////////////////////////////////////////////////////////////////////////
//	This file contains the routines to perform the menu functions in the "Section" pull-down menu.
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
// -+- change: Modified CmNewSection() to just ask for section number.
// modified 6/23/05 by JCF (fiala@bu.edu)
// -+- change: Added Flips from keyboard.cpp and similar rotations to movemvent menu.
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change: Modified CenteredMvmt, XformSectionDlgProc to look for images AND contours.
// -+- change: Made changes to accommodate rewrite of ImageSize() section method.
// -+- change: Modified behavior of sectionList: maintained col widths, removed Ctrl-L,
//             and eliminate DestroyWindow from Page To menu selection.
// modified 7/15/05 by JCF (fiala@bu.edu)
// -+- change: Modified behavior of DeleteSections and RenumberSection to take account of sections in memory.
// modified 11/7/05 by JCF (fiala@bu.edu)
// -+- change: Set Pixel Size dialog changed to YES/NO.
// modified 3/15/06 by JCF (fiala@bu.edu)
// -+- change: Fixed mode of dialog boxes evoked from Section List.
// modified 4/25/06 by JCF (fiala@bu.edu)
// -+- change: Added CM_REFRESHLIST and CM_INFOLIST to Section List.
// modified 5/24/06 by JCF (fiala@bu.edu)
// -+- change: Moved SectionRangeDlgProc to utility.cpp
// modified 6/5/06 by JCF (fiala@bu.edu)
// -+- change: Added debug logging of operations for crash detection.
// modified 6/21/06 by JCF (fiala@bu.edu)
// -+- change: Fixed bug that found a file with no extension as section zero in SectionNumber().
// modified 11/15/06 by JCF (fiala@bu.edu)
// -+- change: Added CmReverseSections to section list Modify.
// modified 11/22/06 by JCF (fiala@bu.edu)
// -+- change: Fixed error message when unable to delete image file part of section.
// -+- change: FindSection() fills filename even when fail so get reasonable error msg in GoToSection() in Section List.
// -+- change: Close series after delete all sections.
// modified 11/24/06 by JCF (fiala@bu.edu)
// -+- change: Removed unnecessary assignment in ReverseSections().
// modified 2/5/07 by JCF (fiala@bu.edu)
// -+- change: Added some debug logging.
// modified 5/11/07 by JCF (fiala@bu.edu)
// -+- change: Added CmMagnification() for Zoom menu.
// modified 5/18/07 by JCF (fiala@bu.edu)
// -+- change: Added wait cursor to Section List Modify menu operations.
// modified 7/16/07 by JCF (fiala@bu.edu)
// -+- change: Added enable of Maximize sys menu cmd in WM_CREATE of Section List Window.
// -+- change: Modified Contrast dialog to set color channels as well as brightness/contrast.
//

#include "reconstruct.h"


int SectionNumber( char * filename )					// return the integer extension of filename
{
	int sectnum = -1;
	char noperiod[MAX_PATH], ext[MAX_PATH], dir[MAX_PATH], file[MAX_PATH];
	
	SplitPath(filename,dir,file,ext);					// parse filename using utility routine
	if ( ext[0] != '\0' )
		if ( sscanf(ext+1," %[0-9]",noperiod) ) sectnum = atoi(noperiod);

	return( sectnum );									// return -1 if not valid section extension
}

bool FindSection( int &sectnum, int dir, char * filename )
{														// search for section file in series
	char workingfile[MAX_PATH];							// dir=0, find sectnum or fail
	WIN32_FIND_DATA	f;									// dir=1, find next section after sectnum
	HANDLE			search;								// dir=-1, find previous section
	int	foundsect, i;
	bool FileFound;										

	sprintf(workingfile,"%s%s.*",WorkingPath,BaseName);	// look for "basename.nnn" in the WorkingPath

	FileFound = false;

	if ( dir > 0 ) foundsect = MAX_INT32;				// set bounds for section search
	else if ( dir < 0 ) foundsect = -1;					// NEED TO ALLOW 0 AS VALID SECTION NUMBER
	else foundsect = sectnum;
														// begin file search
	search = FindFirstFile( workingfile, &f );
	if ( search != INVALID_HANDLE_VALUE )
		do {
			i = SectionNumber( f.cFileName );			// parse general form for section number
			if ( i >= 0 )								// -1 means file extension not valid section number
			  {
			  if ( (dir == 0) && (i == foundsect) )		// found this section!
				{
				FileFound = true;						// can end search now
				strcpy(workingfile,f.cFileName);
				break;									// otherwise, if i between sectnum and foundsect...
				}	
			  if ( ( (dir > 0) && (i > sectnum) && (i < foundsect) )
				|| ( (dir < 0) && (i < sectnum) && (i > foundsect) ) )
				{
				FileFound = true;						// remember this section number
				foundsect = i;							// but keep searching in case there's
				strcpy(workingfile,f.cFileName);		// a closer section number
				}
			  }
			}
		while ( FindNextFile( search, &f ) );			// continue file search

	FindClose( search );

	if ( FileFound )									// found file, return filename and section number
		{												// f.cFileNam is just the filename, return the full path!
		sprintf(filename,"%s%s",WorkingPath,workingfile);
		sectnum = foundsect;
		}												// put something reasonable in filename even when fail
	else sprintf(filename,"%s%s.*",WorkingPath,BaseName); // becuz it might not have been initialized yet

	return FileFound;
}
														// this function will return either a section from memory
Section * GetNextSectionBetween( int &last, int end )	// or from disk in the specified range (last,end]
{
	Section *section;
	char filename[MAX_PATH];

	section = NULL;										// return NULL if fail
	if ( FindSection( last, 1, filename ) )				// determine what's next from directory listing
		{												// fail if past end of section range
		if ( last <= end )								// if this section is in memory, return memory structure
			{											// load section from file if not in memory
			if ( PrevSection )
				if ( PrevSection->index == last ) section = PrevSection;
			if ( CurrSection )
				if ( CurrSection->index == last ) section = CurrSection;
			if ( !section ) section = new Section( filename );
			}
		}
	return( section );
}
														// this function will return section to disk or memory
void PutSection( Section *section, bool saveit, bool reRender, bool reDraw )	
{							
	char filename[MAX_PATH];

	if ( CurrSection && CurrView )						// case I. section is CurrSection in memory
		if ( CurrSection->index == section->index ) 
			{											// flag view redisplay
			if ( reRender ) CurrView->needsRendering = true;
			if ( reDraw ) CurrView->needsDrawing = true;
			if ( saveit )								// save it to disk becuz user will expect this
				{
				sprintf(filename,"%s%s.%d",WorkingPath,BaseName,section->index);
				section->Save( filename );
				}										// invoke view redisplay
			if ( reRender || reDraw ) InvalidateRect(appWnd,NULL,FALSE);
			return;										// don't delete memory section!
			}

	if ( PrevSection && PrevView )						// case II. section is PrevSection in memory		
		if ( PrevSection->index == section->index )
			{											// flag for later view redisplay
			if ( reRender ) PrevView->needsRendering = true;
			if ( reDraw ) PrevView->needsDrawing = true;
			if ( saveit )								// save it to disk becuz user will expect this
				{
				sprintf(filename,"%s%s.%d",WorkingPath,BaseName,section->index);
				section->Save( filename );
				}
			return;										// don't delete memory section
			}

	if ( saveit )	// section not in memory but belongs on disk, so save it before delete
		{
		sprintf(filename,"%s%s.%d",WorkingPath,BaseName,section->index);
		section->Save( filename );
		}

	delete section;										// delete memory structure
}

void UpdateLists( int sectnum )			// routine called from GotoSection to update lists when section changes
{
	if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
	if ( IsWindow(domainWindow) ) PostMessage( domainWindow, UM_UPDATESECTION, 0, (LPARAM)sectnum );
	if ( IsWindow(sectionWindow) ) PostMessage( sectionWindow, UM_UPDATESECTION, 0, (LPARAM)sectnum );
}

bool GotoSection( int sectnum, char *sectionfile )		// goto section
{
	HCURSOR cur;
	Section *section;
// begin debug logging...
	DWORD byteswritten;
	char line[MAX_PATH];
	if ( debugLogFile != INVALID_HANDLE_VALUE )
		{
		sprintf(line,"Entered GotoSection( %d, %s )\r\n",sectnum,sectionfile);
		WriteFile(debugLogFile, line, strlen(line), &byteswritten, NULL);
		}
// ...end debug logging

	CurrContour = NULL;			// clear current contour since changing sections

	if ( DomainView )			// if there is an active domain view give an entirely different behavior
		{
		if ( PrevSection )						// if request is for existing PrevSection...
			if ( sectnum == PrevSection->index )
				{
				FrontView = PrevView;					// display it in front
				BackView = DomainView;					// with domain in back
				if ( BlendView ) CmBlend();				// turn off blend (OR UPDATE BLEND VIEW)
				InvalidateRect( appWnd, NULL, FALSE );	// redraw window
				UpdateLists(sectnum);
				return true;
				}
		if ( CurrSection )						// if request is for CurrSection to which the Domain belongs...
			if ( sectnum == CurrSection->index )
				{
				FrontView = CurrView;					// display it in front
				BackView = DomainView;					// with domain in back
				if ( BlendView ) CmBlend();				// turn off blend (OR UPDATE BLEND VIEW)
				InvalidateRect( appWnd, NULL, FALSE );	// redraw window
				UpdateLists(sectnum);
				return true;
				}
												// request is for new section, so replace PrevSection
												// since must keep CurrSection which is being edited
		cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow
		section = new Section( sectionfile );				// load section from file
		SetCursor( cur );									// restore cursor

		if ( section->index >= 0 )				// 0  is a valid section?
			{
			if ( PrevSection )
				{
				if ( !PrevSection->SaveIfNeeded() )			// if user cancels the save, then fail
					{
					delete section;
					return false;
					}
				delete PrevSection;
				}											// move new section to previous
			PrevSection = section;							// if PrevView doesn't exist, create it
			if ( !PrevView ) PrevView = new ViewPort( appWnd );
			PrevView->section = PrevSection;
			PrevView->needsRendering = true;				// regenerate PrevView with new section data	
			FrontView = PrevView;							// display it in front
			BackView = DomainView;							// with domain in back
			if ( BlendView ) CmBlend();						// turn off blend (OR UPDATE BLEND VIEW)
			InvalidateRect( appWnd, NULL, FALSE );			// redraw window
															// update child windows: domain list is closed now
			UpdateLists(section->index);
			return true;									// signal OK status on return
			}

		delete section;										// if there was a problem with the section
		return false;										// as read from file, delete it and fail
		}
								// no active domain view so change prev/curr sections normally
		
	if ( PrevSection ) if ( sectnum == PrevSection->index ) { Previous(); return true; }
	if ( CurrSection ) if ( sectnum == CurrSection->index ) { return( Reload( sectionfile ) ); }

	cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow
	section = new Section( sectionfile );						// load section from file
	SetCursor( cur );											// restore cursor
	
	if ( section->index >= 0 )
		{
		if ( PrevSection )
			{
			if ( !PrevSection->SaveIfNeeded() )					//if user cancels the save, then fail
				{
				delete section;
				return false;
				}
			delete PrevSection;
			}

		if ( CurrSection )										// move current section to previous
			{
			PrevSection = CurrSection;
			if ( !PrevView ) PrevView = new ViewPort( appWnd );	// move old current section to previous view
			PrevView->section = PrevSection;
			BackView = PrevView;
			BackView->needsRendering = true;
			}

		CurrSection = section;									// update current section in series
		CurrSeries->index = section->index;
		if ( !CurrView ) CurrView = new ViewPort( appWnd );		// if no viewport already, create one
		CurrView->section = CurrSection;
		FrontView = CurrView;									// put current in front
		FrontView->needsRendering = true;						// flag for regeneration
		
		InvalidateRect( appWnd, NULL, FALSE );					// redraw (Windows will generate and execute a paint msg immediately!)
																// update child windows that display section info
		UpdateLists(section->index);
		return true;											// signal OK status on return
		}

	delete section;												// if there was a problem with the section
	return false;												// as read from file, delete it and fail
}

bool Reload( char *sectionfile )					// reload current section
{
	if ( CurrSection )
		{
		if ( !CurrSection->SaveIfNeeded() )	return false;	//if user cancels the save, then fail
		delete CurrSection;									// if it exists, delete it
		}
	HCURSOR cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass
	CurrSection = new Section( sectionfile );				// reload from file
	SetCursor( cur );										// restore cursor

	if ( !CurrView ) CurrView = new ViewPort( appWnd );		// create CurrView if needed
	CurrView->section = CurrSection;
	FrontView  = CurrView;
	FrontView->needsRendering = true;						// flag for regeneration
	InvalidateRect( appWnd, NULL, FALSE );
	return true;
}

void Previous( void )			// display prev section
{
	Section *tmpSection;
	ViewPort *tmpView;

	if ( PrevSection )						// assume only have PrevSection if also have CurrSection
		{
		tmpSection = PrevSection;
		PrevSection = CurrSection;
		CurrSection = tmpSection;
		tmpView = PrevView;
		PrevView = CurrView;
		CurrView = tmpView;
		CurrSeries->index = CurrSection->index;
		FrontView = CurrView;
		BackView = PrevView;				// update child windows that display section info
		UpdateLists(CurrSection->index);
		InvalidateRect( appWnd, NULL, FALSE );
		}
	else if ( CurrSeries->beepPaging ) MessageBeep(0xFFFFFFFF);
}


int SectionSelected( HWND hList )					// return the sectnum of selection
{
	LV_ITEM lvi;
	int sectnum = -1;
	char txt[MAX_PATH];

	lvi.iItem = SendMessage(hList, LVM_GETNEXTITEM, -1, (LPARAM)LVNI_SELECTED );
	if ( lvi.iItem >= 0 )
		{											// an item was selected, retrieve section number
		lvi.mask = LVIF_TEXT; 
		lvi.iSubItem = 0; lvi.pszText = txt; lvi.cchTextMax = MAX_PATH;
		SendMessage(hList, LVM_GETITEM, 0, (LPARAM)(&lvi));
		sectnum = atoi( txt );
		}
	return( sectnum );
}

void FillSectionList( HWND list )		// fill listview from CurrSectionsInfo
{
	SectionInfo *sectioninfo;
	LV_ITEM lvi;
	LV_COLUMN column;
	int i, colwidth[MAX_SECTIONLISTCOLS];
	char txt[64];

	SendMessage( list, LVM_DELETEALLITEMS, 0, 0 );		// clear list

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;	// setup columns masks
	column.fmt = LVCFMT_LEFT; column.pszText = txt;	column.cchTextMax = 64;

	for (i=MAX_SECTIONLISTCOLS-1; i>0; i--)				// clear all columns
		{
		colwidth[i] = 100;								// default column width
		if ( SendMessage(list, LVM_GETCOLUMN, i, (LPARAM)(&column)) );
			{
			colwidth[i] = column.cx;					// if exists, remember old width
			SendMessage(list, LVM_DELETECOLUMN, i, 0);	// delete column from listview
			}
		}

	i = 1;												// recreate columns in new format
	if ( CurrSeries->listSectionThickness )
		{
		column.cx = colwidth[i]; column.iSubItem = i; strcpy(txt,"Thickness");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}

	lvi.mask = LVIF_TEXT | LVIF_IMAGE; lvi.state = 0; lvi.stateMask = 0;
	lvi.iItem = 0;
	sectioninfo = CurrSectionsInfo->first;				// fill list from sectioninfo
	while ( sectioninfo )
		{
		lvi.iSubItem = 0; lvi.pszText = txt;
		if ( sectioninfo->alignLocked ) lvi.iImage = 0; else lvi.iImage = 1;
		sprintf(txt,"%d", sectioninfo->index);
		if ( MatchLimit( txt, limitSectionList ) )
			{
			SendMessage(list, LVM_INSERTITEM, lvi.iItem, (LPARAM)(&lvi));
			if ( CurrSeries->listSectionThickness )
				{
				lvi.iSubItem = 1; lvi.pszText = txt;
				sprintf(txt,"%g", sectioninfo->thickness);
				SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
				}
			lvi.iItem++;
			}
		sectioninfo = sectioninfo->next;
		}
}

void LockSections( HWND list, bool lockit )		// modify lock status of listview selections
{
	HCURSOR cur;
	Section *section;
	int sectnum, prevsect;
	LV_ITEM lvi;
	char  txt[64];

	cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow

	lvi.mask = LVIF_TEXT; lvi.iSubItem = 0; lvi.pszText = txt; lvi.cchTextMax = 64;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	while ( lvi.iItem >= 0 )
		{
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));	// retrieve section number
		sectnum = atoi(txt);
		prevsect = sectnum-1;
		section = GetNextSectionBetween( prevsect, sectnum );	// fetch the section
		if ( section )
			{
			section->alignLocked = lockit;
			PutSection( section, true, false, false );
			}
		CurrSectionsInfo->SetLock( sectnum, lockit );		// update sections info list
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		}

	FillSectionList( sectionList );						// refresh section list
	SetCursor( cur );									// restore cursor
}

void ModifyThickness( HWND list )		// modify thicknesses of listview selections
{
	HCURSOR cur;
	Section *section;
	int sectnum, prevsect;
	double thickness;
	LV_ITEM lvi;
	char  txt[64];

	lvi.mask = LVIF_TEXT; lvi.iSubItem = 0; lvi.pszText = txt; lvi.cchTextMax = 64;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{											// ask user for desired thickness value
		sprintf(InputDlgName,"Section Thickness");
		sprintf(InputDlgString,"%1.*g", Precision,CurrSeries->defaultThickness);
		sprintf(InputDlgValue,"Set thickness (in %s) to:",CurrSeries->units);
		if ( DialogBox( appInstance, "InputWarning", list, (DLGPROC)InputDlgProc ) )
			{
			thickness = atof(InputDlgString);				// new section thickness
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass, file access could be slow
			while ( lvi.iItem >= 0 )						// now modify selected sections
				{
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));	// retrieve section number
				sectnum = atoi(txt);
				prevsect = sectnum-1;
				section = GetNextSectionBetween( prevsect, sectnum );	// fetch the section
				if ( section )
					{
					section->thickness = thickness;
					PutSection( section, true, false, false );
					}
				CurrSectionsInfo->SetThickness( sectnum, thickness );	// update sections info list
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				}
			FillSectionList( sectionList );					// refresh section list
			SetCursor( cur );								// restore cursor
			}
		}
}

BOOL CALLBACK ModifyContrastDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	char txt[MAX_DIGITS];

	switch (message)			// global variable CurrDomain is used to pass the domain to be modified
		{
		case WM_INITDIALOG:
			sprintf(txt,"%.3f",ImageContrast);
			SendDlgItemMessage(hwndDlg, ID_CONTRAST, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%.3f",ImageBrightness);
			SendDlgItemMessage(hwndDlg, ID_BRIGHTNESS, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_REDCHANNEL, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
			SendDlgItemMessage(hwndDlg, ID_GREENCHANNEL, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
			SendDlgItemMessage(hwndDlg, ID_BLUECHANNEL, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{										// user says OK, so store values from dialog in CurrDomain
				case IDOK:
					SendDlgItemMessage(hwndDlg, ID_CONTRAST, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					ImageContrast = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_BRIGHTNESS, WM_GETTEXT, (WPARAM)MAX_DIGITS, (LPARAM)txt);
					ImageBrightness = atof(txt);
					ImageRed = SendDlgItemMessage(hwndDlg, ID_REDCHANNEL, BM_GETCHECK, 0, 0);
					ImageGreen = SendDlgItemMessage(hwndDlg, ID_GREENCHANNEL, BM_GETCHECK, 0, 0);
					ImageBlue = SendDlgItemMessage(hwndDlg, ID_BLUECHANNEL, BM_GETCHECK, 0, 0);
					EndDialog(hwndDlg, 1);
					return TRUE;

				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				}
		}
	return FALSE;
}

void ModifyContrast( HWND list )		// modify thicknesses of listview selections
{
	HCURSOR cur;
	Section *section;
	Transform *transform;
	int sectnum, prevsect;
	LV_ITEM lvi;
	char  txt[64];

	lvi.mask = LVIF_TEXT; lvi.iSubItem = 0; lvi.pszText = txt; lvi.cchTextMax = 64;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{											// ask user for desired brightness, contrast values
		ImageContrast = 1.0;
		ImageBrightness = 0.0;
		if ( DialogBox( appInstance, "ModifyContrastDlg", list, (DLGPROC)ModifyContrastDlgProc ) )
			{
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass, file access could be slow
			while ( lvi.iItem >= 0 )						// now modify selected sections
				{
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));	// retrieve section number
				sectnum = atoi(txt);
				prevsect = sectnum-1;
				section = GetNextSectionBetween(prevsect, sectnum);	// fetch the section
				if ( section )
				  {
				  if ( section->transforms )						// process only if has transforms
					{
					transform = section->transforms->first;
					while ( transform )								// do all transforms in section...
						{
						if ( transform->image )						// found image, set attributes
							{										// based on Contrast dialog values
							transform->image->contrast = ImageContrast;
							transform->image->brightness = ImageBrightness;
							if ( ImageRed == BST_CHECKED ) transform->image->red = true;
							else if ( ImageRed == BST_UNCHECKED ) transform->image->red = false;
							if ( ImageGreen == BST_CHECKED ) transform->image->green = true;
							else if ( ImageGreen == BST_UNCHECKED ) transform->image->green = false;
							if ( ImageBlue == BST_CHECKED ) transform->image->blue = true;
							else if ( ImageBlue == BST_UNCHECKED ) transform->image->blue = false;
							}
						transform = transform->next;				// continue with next transform
						}
					}			
				  PutSection( section, true, true, false );		// save section
				  }
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				}
			if ( IsWindow(domainWindow) ) PostMessage( domainWindow, UM_UPDATESECTION, 0, (LPARAM)sectnum );
			SetCursor( cur );								// restore cursor
			}
		}
}

void ModifyPixelSize( HWND list )		// modify thicknesses of listview selections
{
	HCURSOR cur;
	Section *section;
	Transform *transform;
	int sectnum, prevsect;
	LV_ITEM lvi;
	char  txt[64];

	lvi.mask = LVIF_TEXT; lvi.iSubItem = 0; lvi.pszText = txt; lvi.cchTextMax = 64;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{											// ask user for desired thickness value
		sprintf(InputDlgName,"Section Pixel Size");
		sprintf(InputDlgString,"%f",ImagePixelSize);
		sprintf(InputDlgValue,"Set pixel size (in %s) of all domains to:",CurrSeries->units);
		if ( DialogBox( appInstance, "InputWarning", list, (DLGPROC)InputDlgProc ) )
			{
			ImagePixelSize = atof(InputDlgString);			// new pixel size
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass, file access could be slow
			while ( lvi.iItem >= 0 )						// now modify selected sections
				{
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));	// retrieve section number
				sectnum = atoi(txt);
				prevsect = sectnum-1;
				section = GetNextSectionBetween(prevsect, sectnum);	// fetch the section
				if ( section )
				  {
				  if ( section->transforms )						// process only if has transforms
					{
					transform = section->transforms->first;
					while ( transform )								// do all transforms in section...
						{
						if ( transform->image )						// found image, reset pixel size
							transform->image->mag = ImagePixelSize;
						transform = transform->next;				// continue with next transform
						}
					}			
				  PutSection( section, true, true, false );		// save section
				  }
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				}
			if ( IsWindow(domainWindow) ) PostMessage( domainWindow, UM_UPDATESECTION, 0, (LPARAM)sectnum );
			SetCursor( cur );								// restore cursor
			}
		}
}

void RenumberSections( HWND list )		// renumber selected entries of listview
{
	HCURSOR cur;
	Section *section;
	int i, direction, sectnum, increment;
	bool path_blocked;
	LV_ITEM lvi;
	WIN32_FIND_DATA	f;
	HANDLE			search;
	char  txt[64], filename[MAX_PATH], newfile[MAX_PATH];

	lvi.mask = LVIF_TEXT | LVIF_STATE; lvi.stateMask = LVIS_SELECTED;
	lvi.iSubItem = 0; lvi.pszText = txt; lvi.cchTextMax = 64;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{													// found at least one list item to move
		sprintf(InputDlgName,"Renumber Sections");
		sprintf(InputDlgString,"-1");						// ask user for shift amount
		sprintf(InputDlgValue,"Shift section numbers by:");
		if ( DialogBox( appInstance, "InputWarning", list, (DLGPROC)InputDlgProc ) )
			{
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass, file access could be slow

			increment = atof(InputDlgString);				// shift indices by this increment
			if ( increment < 0 )							// start from first selected list item
				direction = 1;								// process list in ascending order
			else if ( increment > 0 )
				{
				i = lvi.iItem;								// process list in descending order
				while ( i >= 0 )
					{										// by starting at last selected iItem
					lvi.iItem = i;
					i = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
					}
				direction = -1;								// and moving upward toward beginning of list
				}
															// abort renumbering if overwrite required
			path_blocked = false;
			while ( !path_blocked && SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi)) )
			  {
			  if ( lvi.state & LVIS_SELECTED )				// move selected sections only
				{
				sectnum = atoi(txt);
				if ( FindSection( sectnum, 0, filename ) )
					{
					section = new Section( filename );		// fetch the section
					section->index += increment;
					sprintf(newfile,"%s%s.%d",WorkingPath,BaseName,section->index);
					search = FindFirstFile( newfile, &f );
					if ( search == INVALID_HANDLE_VALUE )	// new section does not exist
						{
						DeleteFile( filename );				// remove old section file and list info
						CurrSectionsInfo->DeleteSection( sectnum );
						if ( PrevSection )					// check in memory and shift these also
							if ( PrevSection->index == sectnum ) PrevSection->index = section->index;
						if ( CurrSection )
							if ( CurrSection->index == sectnum ) CurrSection->index = section->index;
						section->Save( newfile );			// save file and update list
						CurrSectionsInfo->AddSection( section, newfile );
						CurrSeries->RenumberZTraces( sectnum, section->index );	// update z-traces
						}
					else {									// this section already exists!
						path_blocked = true;				// abandon renumbering
						FindClose( search );				// close the open file search
						}
					delete section;							// clean section memory
					}
				}											// move up(down) in list
			  lvi.iItem += direction;						// and search for more selected items
			  }		// end while

			FillSectionList( sectionList );					// refresh section list
			SetCursor( cur );								// restore cursor

			if ( path_blocked ) ErrMsgOK( ERRMSG_RENUMBER, newfile ); // inform user if error

			sectnum = -1;
			if ( FindSection( sectnum, 1, filename ) )		// try new section display
				{											// since current and previous 
				GotoSection( sectnum, filename );			// might have changed
				}
			}
		}
}

void ReverseSections( HWND list )		// renumber selected entries of listview in reverse order
{
	HCURSOR cur;
	Section *section1, *section2;
	SectionInfo *info, *info1, *info2;
	XYPoints *todo;
	XYPoint *pair, *opair;
	int sect1, sect2, sectnum;
	LV_ITEM lvi;
	char  txt[64], warntxt[128];

	lvi.mask = LVIF_TEXT | LVIF_STATE; lvi.stateMask = LVIS_SELECTED;
	lvi.iSubItem = 0; lvi.pszText = txt; lvi.cchTextMax = 64;
	sectnum = SendMessage(list,LVM_GETSELECTEDCOUNT,0,0);
	if ( sectnum > 1 )									// need more than one section to be selected
		{
		sprintf(warntxt,"Reversing order of %d sections! Continue?",sectnum);
		if ( MessageBox(list,warntxt,"WARNING!",MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDYES )
			{
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass, file access could be slow

			todo = new XYPoints();
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			while ( lvi.iItem >= 0 )
				{													// for each selected section...
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));	// ...retrieve section number
				pair = new XYPoint( atoi(txt), 0 );					// ...add section number to list
				todo->Add( pair );									// then do next item
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				}
				
			pair = todo->first;
			opair = todo->last;
			while ( pair && opair )				// fill todo list with pairs of section number reversals
				{
				pair->y = opair->x;
				pair = pair->next;
				opair = opair->prev;
				}
												// do each pair, up to point where pairs reverse order
			pair = todo->first;
			while ( pair && (pair->x < pair->y)  )
				{
				sect1 = pair->x;								// get sections, whether in memory or not
				sect2 = pair->y;
				sectnum = sect1 - 1;							// Get. looks for NEXT larger section index
				section1 = GetNextSectionBetween( sectnum, sect1 );
				sectnum = sect2 - 1;
				section2 = GetNextSectionBetween( sectnum, sect2 );
				if ( section1 ) section1->index = sect2;		// switch indices
				if ( section2 ) section2->index = sect1;
				PutSection( section1, true, true, true );		// write them back out
				PutSection( section2, true, true, true );
				info = CurrSectionsInfo->first;
				while ( info )									// switch SectionsInfo attributes
					{
					if ( info->index == sect1 ) info1 = info;
					if ( info->index == sect2 ) info2 = info;
					info = info->next;
					}
				info = new SectionInfo( *info1 );				// copy info1 into info
				info1->alignLocked = info2->alignLocked;
				info2->alignLocked = info->alignLocked;			// swap non-section number attributes only so that					
				info1->thickness = info2->thickness;			// correct section order will be retained in info list
				info2->thickness = info->thickness;
				info1->min = info2->min;
				info2->min = info->min;
				info1->max = info2->max;
				info2->max = info->max;	
				delete info;									// discard copy
				//CurrSeries->RenumberZTraces( sectnum, section->index );	// update z-traces??
				pair = pair->next;
				}

			delete todo;									// clean up todo list memory
			FillSectionList( sectionList );					// refresh section list
			SetCursor( cur );								// restore cursor
			}
		}
}


BOOL CALLBACK DeleteSectionsProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	switch (message)
		{
		case WM_INITDIALOG:
			if ( DeleteFiles )
				SendDlgItemMessage(hwndDlg, ID_DELETEFILES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			MessageBeep(MB_ICONHAND);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{	
				case IDYES:
					if ( SendDlgItemMessage(hwndDlg, ID_DELETEFILES, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						DeleteFiles = true;
					else DeleteFiles = false;
					EndDialog(hwndDlg, 1);			// return non-zero value if user selects OK
					return TRUE;
				case IDNO:
					EndDialog(hwndDlg, 0);			// otherwise return zero
					return TRUE;
				}
		}
	return FALSE;
}

void DeleteSections( HWND list )		// delete listview selections from series
{
	HCURSOR cur;
	Section *section;
	Transform *transform;
	int sectnum;
	LV_ITEM lvi;
	char  txt[64], filename[MAX_PATH];

	lvi.mask = LVIF_TEXT; lvi.iSubItem = 0; lvi.pszText = txt; lvi.cchTextMax = MAX_PATH;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{											// at least one section is selected
		if ( DialogBox( appInstance, "DeleteSections", list, (DLGPROC)DeleteSectionsProc ) )
			{
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass, file access could be slow
			while ( lvi.iItem >= 0 )						// user wants to delete them so...
				{
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));	// retrieve section number
				sectnum = atoi(txt);
				if ( FindSection( sectnum, 0, filename ) )			// find section file
					{
					if ( DeleteFiles )								// load section into memory so can delete images
						{
						section = new Section( filename );			// read section data
						if ( section->transforms )
							{
							transform = section->transforms->first;	// do all transforms in section...
							while ( transform )
								{
								if ( transform->image )							// delete proxy if present
									{
									if ( transform->image->HasProxy() ) transform->image->DeleteProxy();

									if ( !DeleteFile( transform->image->src ) )	// delete domain source
										ErrMsgSplash( ERRMSG_DELETE_FAILED, transform->image->src );
									}
								transform = transform->next;		// continue with next transform
								}
							}
						delete section;				// done with this section data
						}
					if ( PrevSection )								// check in memory and don't autosave these
						if ( PrevSection->index == sectnum ) PrevSection->hasChanged = false;
					if ( CurrSection )
						if ( CurrSection->index == sectnum ) CurrSection->hasChanged = false;

					if ( !DeleteFile( filename ) )					// now delete section file from file system
						ErrMsgSplash( ERRMSG_DELETE_FAILED, filename );
					}

				CurrSectionsInfo->DeleteSection( sectnum );			// remove from sections list 

				SendMessage(list, LVM_DELETEITEM, lvi.iItem, 0);		// delete also from listview
				lvi.iItem--;											// backup so next item will be considered
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				}

			SetCursor( cur );								// restore cursor
			sectnum = -1;
			if ( FindSection( sectnum, 1, filename ) )		// try new section display
				{											// since current and previous 
				GotoSection( sectnum, filename );			// might have changed
				}
			else CmClose();									// no sections => clean up everything else
			}
		}
}


LRESULT APIENTRY ListSectionsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pnmh;
	LV_FINDINFO lvf;
	LV_COLUMN column;
	LV_ITEM lvi;
	LV_KEYDOWN *key;
	RECT r;
	HMENU hMenu;
	HIMAGELIST himl;
	OPENFILENAME ofn;
	int i, w, h, sectnum;
	char txt[MAX_PATH], filename[MAX_PATH];

	switch (message)
		{										// initialize the list view section info
		case WM_CREATE:
			GetClientRect( hWnd, &r );
			w = r.right-r.left-10; h = r.bottom-r.top-10,
			sectionList = CreateWindow( WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER,
														5, 5, w, h, hWnd, NULL, appInstance, NULL);
			himl = ImageList_Create(16, 16, TRUE, 2, 1);				//  ImageList is deleted by ListView
			ImageList_AddIcon(himl, LoadIcon(appInstance, "LockIcon"));
			ImageList_AddIcon(himl, LoadIcon(appInstance, "UnlockIcon"));
			SendMessage(sectionList, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)(himl));	// setup icons
			column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; column.fmt = LVCFMT_LEFT; 
			column.pszText = txt; strcpy(txt,"Section"); column.cx = 100;				// setup columns
			for ( column.iSubItem = 0; column.iSubItem < MAX_SECTIONLISTCOLS; column.iSubItem++ )
				SendMessage(sectionList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			FillSectionList( sectionList );
			sprintf(txt,"Sections %s",limitSectionList);
			SetWindowText(hWnd,txt);
			if ( FrontView )						// update selected item to current section
				if ( FrontView->section )
						PostMessage(hWnd, UM_UPDATESECTION, 0, (LPARAM)FrontView->section->index );	
			SetFocus( sectionList );
			hMenu = GetSystemMenu( hWnd, FALSE );						// allow Maximize on titlebar menu
			EnableMenuItem( hMenu, SC_MAXIMIZE, MF_ENABLED );
			break;

		case UM_UPDATESECTION:
			i = (int)lParam;								// select section i and make it visible
			sprintf(txt,"%d", i);
			lvf.flags = LVFI_STRING; lvf.psz = txt;
			lvi.iItem = SendMessage(sectionList, LVM_FINDITEM, -1, (LPARAM)(&lvf) );
			if ( lvi.iItem >= 0 )
				{											// an item was found -- select it
				lvi.mask = LVIF_STATE; lvi.stateMask = LVIS_FOCUSED	| LVIS_SELECTED;
				lvi.state = LVIS_FOCUSED | LVIS_SELECTED; lvi.iSubItem = 0;
				SendMessage(sectionList, LVM_SETITEMSTATE, lvi.iItem, (LPARAM)(&lvi));
				SendMessage(sectionList, LVM_ENSUREVISIBLE, lvi.iItem, (LPARAM)TRUE );
				}
			break;

		case WM_SIZE:							// resize list view control to match dialog box
			GetClientRect( hWnd, &r );
			SetWindowPos( sectionList, 0, 0, 0, r.right-r.left-10, r.bottom-r.top-10, SWP_NOMOVE | SWP_NOOWNERZORDER);
			break;
			
		case WM_DESTROY:
			sectionWindow = NULL;				// clear global handle pointer
			hMenu = GetMenu( appWnd );
			CheckMenuItem( hMenu, CM_SECTIONLIST, MF_BYCOMMAND | MF_UNCHECKED );	// uncheck menu item
			SetFocus(appWnd);					// return to main window
			break;

		case WM_NOTIFY:							// process listview notification messages
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case LVN_KEYDOWN:
					key = (LV_KEYDOWN *)lParam;	// allow control+TAB to switch active window
					if ( (key->wVKey==VK_TAB) && (GetKeyState(VK_CONTROL) & 0x8000) ) CmNextWindow( sectionWindow );
					if ( key->wVKey==VK_DELETE )
						{ DeleteSections( sectionList ); break; }
					//if ( ( MapVirtualKey( key->wVKey, 2 ) == 'L' ) && (GetKeyState(VK_CONTROL) & 0x8000) ) // Ctrl+L
					//	{ LockSections( sectionList, true ); break; }
					if ( key->wVKey !=  VK_RETURN ) break;
												// fall through to double-click	if Enter key pressed

				case NM_DBLCLK:					// user double-clicks or presses enter, go to selected
					sectnum = SectionSelected( sectionList );
					if ( sectnum >= 0 )
						{
						FindSection( sectnum, 0, filename );		// locate the file
						GotoSection( sectnum, filename );			// load it and display it
						SetFocus(appWnd);							// switch to main window
						DestroyWindow( sectionWindow );				// eliminate list after select
						}
					return TRUE;
				}
			break;

		case WM_SETFOCUS:
			SetFocus( sectionList );			// activate list view if window activated
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{								// default pushbutton: goto first selected item
				case CM_LISTGOTO:
					sectnum = SectionSelected( sectionList );
					if ( sectnum >= 0 )
						{
						FindSection( sectnum, 0, filename );		// locate the file
						GotoSection( sectnum, filename );			// load it and display it
						SetFocus(appWnd);							// switch to main window
						}
					break;

				case CM_REFRESHLIST:									// regenerate the list
					FillSectionList( sectionList );
					break;

				case CM_INFOLIST:										// display count of items
					DisplayListInfo( sectionList );
					break;

				case CM_SAVELIST:
					sprintf(filename,"sections.csv");
					ZeroMemory(&ofn, sizeof(ofn));						// clear the OPENFILENAME fields
					ofn.lStructSize = sizeof(ofn);						// set only the necessary values
					ofn.hwndOwner = appWnd;
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "All Files\0*.*\0Comma Separated Value file\0*.csv\0\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrTitle = "Save Section List As...";
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY; // ask user if overwrite desired
					if ( GetSaveFileName(&ofn) == TRUE )				// display the Open dialog box
						{
						SaveList( sectionList, filename );
						}
					break;

				case CM_LOCKSECTIONS:
					LockSections( sectionList, true );
					break;

				case CM_UNLOCKSECTIONS:
					LockSections( sectionList, false );
					break;

				case CM_MODIFYCONTRAST:
					ModifyContrast( sectionList );
					break;

				case CM_MODIFYPIXELSIZE:
					ModifyPixelSize( sectionList );
					break;

				case CM_MODIFYTHICKNESS:
					ModifyThickness( sectionList );
					break;

				case CM_RENUMBERSECTIONS:
					RenumberSections( sectionList );
					break;

				case CM_REVERSESECTIONS:
					ReverseSections( sectionList );
					break;

				case CM_DELETESECTIONS:
					DeleteSections( sectionList );
					break;
				}
		}
	return( DefWindowProc(hWnd, message, wParam, lParam) );
}


HWND MakeSectionListWindow( void )			// display list for user selection of section to go to
{
	WNDCLASS wndclass;
	HWND sWnd;
	HMENU hMenu;
	RECT r;
	int width;
											// create the toolbar window...
	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)ListSectionsProc;
	wndclass.cbClsExtra  = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance = appInstance;
	wndclass.hCursor   = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon     = NULL; // LoadIcon(appInstance,"ASmallIcon"); // not used for WS_EX_TOOLWINDOW
	wndclass.lpszMenuName = "SectionListMenu";
	wndclass.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wndclass.lpszClassName = "ReconSectListClass";
	RegisterClass(&wndclass);

	ClientRectLessStatusBar( &r );					// position window in upper left corner of client
	ClientRectToScreen( appWnd, &r );
	width = 140;										// set width based on how many data fields present
	if ( CurrSeries->listSectionThickness ) width += 100;	
    sWnd = CreateWindowEx( WS_EX_TOOLWINDOW, "ReconSectListClass", "Sections", WS_OVERLAPPEDWINDOW,
										r.left, r.top, width, r.bottom-r.top, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

	if ( sWnd )
		{											// if window created, display it
		ShowWindow(sWnd, SW_SHOW);			// and check corresponding menu item in main window	
		hMenu = GetMenu( appWnd );
		CheckMenuItem( hMenu, CM_SECTIONLIST, MF_BYCOMMAND | MF_CHECKED );
		}

	return( sWnd );
}

void CmSectionList( void )
{													// create or destroy section list window
	HCURSOR cur;
	if ( IsWindow( sectionWindow ) ) DestroyWindow( sectionWindow );
	else {
		cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// show hourglass, file access could be slow
		sectionWindow = MakeSectionListWindow();
		SetCursor( cur );								// restore cursor
		}
}

void CmRestoreThumbnails( void )
{													// create or destroy thumbnails window
	if ( IsWindow( thumbnailWindow ) ) DestroyWindow( thumbnailWindow );
	else thumbnailWindow = MakeThumbsWindow();
}

void CmNewSection(void)							// Create a new empty section
{
	int sectnum;
	Section *newSection;
	char srcname[MAX_PATH];
												// default new section number will be +1
	sectnum = 1;
	if ( CurrSection ) sectnum = sectnum + CurrSection->index;
	sprintf(InputDlgName,"New Section");
	sprintf(InputDlgString,"%d", sectnum);
	sprintf(InputDlgValue,"Enter new section number:");
	if ( DialogBox( appInstance, "InputDlg", appWnd, (DLGPROC)InputDlgProc ) )
		{
		sectnum = atoi(InputDlgString);			// check new section number
		if ( sectnum < 0 )
			ErrMsgOK( ERRMSG_SECTIONINVALID, InputDlgString );
		else if ( CurrSectionsInfo->IsSection(sectnum) )
			ErrMsgOK( ERRMSG_SECTIIONEXISTS, InputDlgString );
		else 
			{									// if valid, create section object
			newSection = new Section();
			sprintf(srcname,"%s%s.%d",WorkingPath,BaseName,sectnum);
			newSection->index = sectnum;
			newSection->Save( srcname );		// write file and update list

			CurrSectionsInfo->AddSection( newSection, srcname );
			if ( sectionWindow ) FillSectionList( sectionList );

			delete newSection;					// free local memory

			GotoSection( sectnum, srcname );	// load new section and display it
			}
		}
}

void CmPredecessor( void )			// display curr-1 section
{
	int sectnum;
	char filename[MAX_PATH];

	sectnum = 0;						// relative to visible section or domain
	if ( CurrSeries && FrontView )
		{
		if ( FrontView->section ) sectnum = FrontView->section->index;

		if ( FindSection( sectnum, -1, filename ) )
			GotoSection( sectnum, filename );		// load it and display it
		else if ( CurrSeries->beepPaging ) MessageBeep(0xFFFFFFFF);
		}
}

void CmSuccessor( void )			// display curr+1 section
{
	int sectnum;
	char filename[MAX_PATH];

	sectnum = 0;						// relative to visible section or domain
	if ( CurrSeries && FrontView )
		{
		if ( FrontView->section ) sectnum = FrontView->section->index;
	
		if ( FindSection( sectnum, 1, filename ) )
			GotoSection( sectnum, filename );		// load it and display it
		else if ( CurrSeries->beepPaging ) MessageBeep(0xFFFFFFFF);
		}
}

void CmSaveSection( void )								// save section data to XML file
{													
	char filename[MAX_PATH];
	if ( CurrSection )
		{												// section file will be "seriesname.index"
		sprintf(filename,"%s%s.%d",WorkingPath,BaseName,CurrSection->index);
		CurrSection->Save( filename );
		}
}

void CmSectionAttributes( void )				// allow user to adjust section thickness for section
{												
	if ( FrontView )							// only if have section,
		if ( FrontView->section )				// get new value from user dialog
			{
			sprintf(InputDlgName,"Section Thickness:");
			sprintf(InputDlgString,"%g",FrontView->section->thickness);
			sprintf(InputDlgValue,"Section %d thickness in %s:",FrontView->section->index,CurrSeries->units);
			if ( DialogBox( appInstance, "InputDlg", appWnd, (DLGPROC)InputDlgProc ) )
				{
				FrontView->section->thickness = atof(InputDlgString);	// modify section thickness
				FrontView->section->hasChanged = true;					// update also in Info list used for 3D computations
				CurrSectionsInfo->SetThickness( FrontView->section->index, FrontView->section->thickness );
				if ( sectionWindow ) FillSectionList( sectionList );
				}
			}
}

void CmBlend( void )		// change blend state
{
	HMENU	hMenu = GetMenu( appWnd );

	if ( BlendView )			// the existence of viewport indicates views are blended
		{
		CheckMenuItem( hMenu, CM_BLEND, MF_BYCOMMAND | MF_UNCHECKED );
		delete BlendView;
		BlendView = NULL;
		InvalidateRect( appWnd, NULL, FALSE );
		}
   else if ( FrontView && BackView )		// only blend if have views to blend!
		{
		CheckMenuItem( hMenu, CM_BLEND, MF_BYCOMMAND | MF_CHECKED );
		BlendView = new ViewPort( appWnd );
		UpdateBlendView();
		InvalidateRect( appWnd, NULL, FALSE );
		}
}

void CmCenter( void )							// shift and zoom view so section is well-centered
{
	Point min, max;
	RECT r;
	int sh;
	double x, y, w, h;

	if ( FrontView )								// if have a view present
		if ( FrontView->section )					// that has a section defined...
			if ( FrontView->section->HasImage() )
				{										// get bounds of all image domains
				FrontView->section->ImageSize( min, max );

				if ( IsWindow( statusbarWindow ) )		// excepting the space taken up by the status bar
					{
					GetWindowRect( statusbarWindow, &r );
					sh = r.bottom-r.top;
					}
				else sh = 0;
														// there must be room to display something
				if ( FrontView->height > sh )
					{
					LastZoom.pixel_size = CurrSeries->pixel_size; // remember current zoom state
					LastZoom.offset_x = CurrSeries->offset_x;
					LastZoom.offset_y = CurrSeries->offset_y;					

					w = (double)FrontView->width;			// compute new zoom to fill viewport
					h = (double)FrontView->height;
					x = (max.x-min.x)/(0.96*w);				// scale image to 96% of window
					y = (max.y-min.y)/(0.96*(h-sh));
					if ( x > y )										// use which ever fits in 96% window
						{												// and leave 2% border at edges
						CurrSeries->pixel_size = x;
						CurrSeries->offset_x = min.x - 0.02*x*w;
						y = x*(h-sh) - (max.y-min.y);
						CurrSeries->offset_y = min.y - y/2.0;		// also center y direction		
						}
					else {
						CurrSeries->pixel_size = y;
						CurrSeries->offset_y = min.y - 0.02*y*(h-sh) - y*sh;
						x = y*w - (max.x-min.x);	
						CurrSeries->offset_x = min.x - x/2.0;
						}
																	// flag views for regeneration
					InvalidateAllViews();
					InvalidateRect( appWnd, NULL, FALSE );			// repaint window
					}
				}

}

void CmLastZoom( void )						// go back to last zoom settings
{
	double fx, fy;

	if ( FrontView && CurrSeries )
		{
		fx = LastZoom.pixel_size;
		LastZoom.pixel_size = CurrSeries->pixel_size;
		CurrSeries->pixel_size = fx;						// use this as new pixel size
		fx = LastZoom.offset_x;
		LastZoom.offset_x = CurrSeries->offset_x;
		CurrSeries->offset_x = fx;
		fy = LastZoom.offset_y;
		LastZoom.offset_y = CurrSeries->offset_y;
		CurrSeries->offset_y = fy;
		InvalidateAllViews();								// update views
		InvalidateRect( appWnd, NULL, FALSE );
		}
}

void CmActualPixels( void )		// zoom view so screen PixelSize = image mag 
{
	double x, y, nx, ny;
	
	if ( FrontView && CurrSeries )						// if have a view present
		if ( FrontView->section )						// that has a section defined...
			{
			if ( FrontView->section->HasImage() )				// change Series view parameters to match
				{
				LastZoom.pixel_size = CurrSeries->pixel_size;				// remember current zoom state
				LastZoom.offset_x = CurrSeries->offset_x;
				LastZoom.offset_y = CurrSeries->offset_y;
				x = (((double)FrontView->width)*CurrSeries->pixel_size)/2.0;  // center of window in units
				y = (((double)FrontView->height)*CurrSeries->pixel_size)/2.0;
				CurrSeries->pixel_size = FrontView->section->MinPixelsize();  // set new pixel size for window
				nx = (((double)FrontView->width)*CurrSeries->pixel_size)/2.0; // new center of window in units
				ny = (((double)FrontView->height)*CurrSeries->pixel_size)/2.0;
				CurrSeries->offset_x += x - nx;								// calculate offsets to keep center
				CurrSeries->offset_y += y - ny;
				InvalidateAllViews();										// flag views for regeneration
				InvalidateRect( appWnd, NULL, FALSE );						// repaint window
				}
			}
}

void CmLockSection( void )				// switch section alignment to locked or unlocked
{
	if ( FrontView )
	  if ( FrontView->section && (FrontView != DomainView) ) // prevent locking of DomainSection
		{
		FrontView->section->alignLocked = !FrontView->section->alignLocked;
		FrontView->section->hasChanged = true;
		CurrSectionsInfo->SetLock( FrontView->section->index, FrontView->section->alignLocked );
		if ( sectionWindow ) FillSectionList( sectionList );
		UpdateMenus();										// reflect change on section menu
		}
}

BOOL CALLBACK XformSectionDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	char txt[128];
	double min_mag;
	Point min, max, allmin, allmax;
	Contour *contour, *c;

	switch (message)
		{
		case WM_INITDIALOG:
			sprintf(txt,"%s",CurrSeries->units);		// fill units descriptors
			SetDlgItemText( hwndDlg, ID_UNITS1, txt );
			SetDlgItemText( hwndDlg, ID_UNITS2, txt );
			sprintf(txt,"1/%s",CurrSeries->units);
			SetDlgItemText( hwndDlg, ID_UNITS3, txt );
			sprintf(txt,"%1.*g", Precision,Movement.centerX);			// fill remembered & default values
   			SetDlgItemText( hwndDlg, ID_XCENTER, txt );
			sprintf(txt,"%1.*g", Precision,Movement.centerY);
   			SetDlgItemText( hwndDlg, ID_YCENTER, txt );
			sprintf(txt,"%1.*g", Precision,Movement.scaleX);
   			SetDlgItemText( hwndDlg, ID_SCALEX, txt );
			sprintf(txt,"%1.*g", Precision,Movement.scaleY);
   			SetDlgItemText( hwndDlg, ID_SCALEY, txt );	// leave zero ones blank
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case ID_CENTER:							// set movement center
					if ( FrontView->section )					// if nothing selected, use everything
						{
						if ( !FrontView->section->active )
							{
							FrontView->section->ImageSize( allmin, allmax );
							FrontView->section->ContourSize( min, max );
							if ( min.x < allmin.x ) allmin.x = min.x;			// accummulate min, max
							if ( min.y < allmin.y ) allmin.y = min.y;
							if ( max.x > allmax.x ) allmax.x = max.x;
							if ( max.y > allmax.y ) allmax.y = max.y;
							}
						else									// if contours are selected, set to their center
							if ( FrontView->section->active->contours )
								{
								allmin.x = MAX_FLOAT;				// will need to search for extent over all contours
								allmin.y = MAX_FLOAT;
								allmax.x = -MAX_FLOAT;
								allmax.y = -MAX_FLOAT;
								contour = FrontView->section->active->contours->first;
								while ( contour )
									{
									c = new Contour( *contour );					// create a copy of contour
									c->InvNform( FrontView->section->active->nform );// transform into section
									c->Extent( &min, &max );						//  find extremes...
									delete c;										// delete transformed contour
									if ( min.x < allmin.x ) allmin.x = min.x;		// accummulate min, max
									if ( min.y < allmin.y ) allmin.y = min.y;
									if ( max.x > allmax.x ) allmax.x = max.x;
									if ( max.y > allmax.y ) allmax.y = max.y;
									contour = contour->next;						// use all active contours
									}
								}								// must be domain section with domain image active
							else FrontView->section->ImageSize( allmin, allmax );
						 
						Movement.centerX = (allmin.x + allmax.x)/2.0;		// set section center
						Movement.centerY = (allmin.y + allmax.y)/2.0;
						sprintf(txt,"%1.*g", Precision,Movement.centerX);			// to dialog with values
						SetDlgItemText( hwndDlg, ID_XCENTER, txt );
						sprintf(txt,"%1.*g", Precision,Movement.centerY);
						SetDlgItemText( hwndDlg, ID_YCENTER, txt );
						}
					return TRUE;

				case IDOK:
					GetDlgItemText( hwndDlg, ID_XCENTER, txt, sizeof(txt) );
					Movement.centerX = atof( txt );
					GetDlgItemText( hwndDlg, ID_YCENTER, txt, sizeof(txt) );
					Movement.centerY = atof( txt );
					GetDlgItemText( hwndDlg, ID_TRANSX, txt, sizeof(txt) );
					Movement.transX = atof( txt );
					GetDlgItemText( hwndDlg, ID_TRANSY, txt, sizeof(txt) );
					Movement.transY = atof( txt );
					GetDlgItemText( hwndDlg, ID_THETA, txt, sizeof(txt) );
					Movement.theta = -PI*atof( txt )/180.0;				// convert to clkwise radians
					GetDlgItemText( hwndDlg, ID_SCALEX, txt, sizeof(txt) );
					Movement.scaleX = atof( txt );						// protect against 0.0
					if ( Movement.scaleX < MIN_SCALE ) Movement.scaleX = MIN_SCALE;
					GetDlgItemText( hwndDlg, ID_SCALEY, txt, sizeof(txt) );
					Movement.scaleY = atof( txt );						// protect against 0.0
					if ( Movement.scaleY < MIN_SCALE ) Movement.scaleY = MIN_SCALE;
					GetDlgItemText( hwndDlg, ID_SLANTX, txt, sizeof(txt) );
					Movement.slantX = -atof( txt );
					GetDlgItemText( hwndDlg, ID_SLANTY, txt, sizeof(txt) );
					Movement.slantY = -atof( txt );
					GetDlgItemText( hwndDlg, ID_DEFORMX, txt, sizeof(txt) );
					Movement.deformX = atof( txt );
					GetDlgItemText( hwndDlg, ID_DEFORMY, txt, sizeof(txt) );
					Movement.deformY = atof( txt );
					EndDialog(hwndDlg, 1);
					return TRUE;

				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				}
		}
	return FALSE;
}

void CmTypeSectionTform( void )				// allow user to type-in a movement cmd for the section
{
	Transform *moveit;
	Nform *adjustment;							// will also remember move so can repeat it

	adjustment = NULL;
									
	if ( FrontView )							// do only if have active transform or unlocked section movements
	  if ( FrontView->section )
		if ( FrontView->section->active )			// move active transform!
			{ 
			Movement.Clear();							// dialog will modify global Movement value
						
			if ( DialogBox( appInstance, "XformDlg", appWnd, (DLGPROC)XformSectionDlgProc ) == IDOK )
				{
				FrontView->section->PushUndoState();
				FrontView->section->active->nform->PostApply( Movement );
				adjustment = new Nform();
				adjustment->PostApply( Movement );
				FrontView->section->hasChanged = true;
				if ( FrontView->section->active->image )
					FrontView->needsRendering = true;	// flag view for regeneration
				else FrontView->needsDrawing = true;
				InvalidateRect( appWnd, NULL, FALSE );	// repaint window
				}
			}
		else if ( !FrontView->section->alignLocked )// apply to whole section
			  if ( FrontView->section->transforms )
				{								
				Movement.Clear();						// clear everything but center point
							
				if ( DialogBox( appInstance, "XformDlg", appWnd, (DLGPROC)XformSectionDlgProc ) == IDOK )
					{
					FrontView->section->PushUndoState();
					adjustment = new Nform();			// remember adjustment made
					adjustment->PostApply( Movement );
					moveit = FrontView->section->transforms->first;
					while ( moveit )
						{
						moveit->nform->PostApply( Movement );
						moveit = moveit->next;
						}
					FrontView->section->hasChanged = true;
					FrontView->needsRendering = true;		// flag view for regeneration
					InvalidateRect( appWnd, NULL, FALSE );	// repaint window
					}
				}

	if ( adjustment )								// if movement was made, update LastAdjustment
		{
		if ( LastAdjustment ) delete LastAdjustment;
		LastAdjustment = adjustment;				// and add to Recording if present
		if ( Recording ) Recording->PostApply( Movement );
		}
}

void CmByCorrelation( void )	// use cross-correlation with back view to shift front view section
{
	HCURSOR cur;
	Mvmt *mvmt;
	Transform *moveit;
	Nform *adjustment;							// will also remember move so can repeat it

	adjustment = NULL;
									
	if ( FrontView && BackView )		// do only if have 2 images and unlocked section movements
	  if ( FrontView->section )
		if ( BackView->section )
			if ( !FrontView->section->alignLocked )		// apply to whole section
			  if ( FrontView->section->transforms )		// but only if there are things to move
				{
				cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// this will be slow

				mvmt = FrontView->view->Correlate( BackView->view );
				mvmt->transX *= CurrSeries->pixel_size;
				mvmt->transY *= CurrSeries->pixel_size;	// compute shift from cross-correlation
				FrontView->section->PushUndoState();
				adjustment = new Nform();				// remember so can repeat later
				adjustment->PostApply( *mvmt );
				moveit = FrontView->section->transforms->first;
				while ( moveit )
					{									// move each transform in section
					moveit->nform->PostApply( *mvmt );
					moveit = moveit->next;
					}
				delete mvmt;							// free shift data structure
				FrontView->section->hasChanged = true;
				FrontView->needsRendering = true;		// flag view for regeneration
				InvalidateRect( appWnd, NULL, FALSE );
		
				SetCursor( cur );						// restore cursor		
				}

	if ( adjustment )								// if movement was made, update LastAdjustment
		{
		if ( LastAdjustment ) delete LastAdjustment;
		LastAdjustment = adjustment;				// and add to Recording if present
		if ( Recording ) Recording->PostApply( Movement );
		}
}

void CmUndoSection( void )				// go back to last section in undo LIFO
{
	if ( FrontView )
	  if ( FrontView->section )
		if ( FrontView->section->PopUndoState() )	// backup section to undo state
			{
			FrontView->needsRendering = true;		// flag need to regenerate view
			InvalidateRect( appWnd, NULL, FALSE );
			}
}

void CmRedoSection( void )
{
	if ( FrontView )
	  if ( FrontView->section )
		if ( FrontView->section->PushRedoState() )	// backup to previous undo state
			{
			FrontView->needsRendering = true;		// flag need to regenerate view
			InvalidateRect( appWnd, NULL, FALSE );
			}
}

void CmResetSection( void )				// go back to first section in undo LIFO
{
	if ( FrontView )
	  if ( FrontView->section )
		if ( FrontView->section->ResetUndoState() )	// backup section to first undo state
			{
			FrontView->needsRendering = true;		// flag need to regenerate view
			InvalidateRect( appWnd, NULL, FALSE );
			}
}

void CmRepeatAdjustment( void )		// repeat last adjustment on current section or active transform
{
	Transform *transform;

	if ( FrontView )
	  if ( LastAdjustment && FrontView->section )		// only doable if have a LastAdjustment to apply
		{
		if ( FrontView->section->active )						// apply to active transform
			{
			transform = FrontView->section->active;
			FrontView->section->PushUndoState();
			FrontView->section->hasChanged = true;				// use multiplication for linear, or numerical approx.
			if ( LastAdjustment->dim > 3 )
				transform->nform->Compose( LastAdjustment, transform->nform, 1.0, 1.0 );
			else transform->nform->ApplyLinearOf( LastAdjustment );
			if ( transform->image ) FrontView->needsRendering = true;
			else FrontView->needsDrawing = true;
			InvalidateRect( appWnd, NULL, FALSE );				// repaint window
			}
		else if ( !FrontView->section->alignLocked  )			// move everything if section mvmt enabled
			{
			FrontView->section->PushUndoState();
			transform = FrontView->section->transforms->first;
			while ( transform )									// by moving each transform
				{
				FrontView->section->hasChanged = true;
				if ( LastAdjustment->dim > 3 )
					transform->nform->Compose( LastAdjustment, transform->nform, 1.0, 1.0 );
				else transform->nform->ApplyLinearOf( LastAdjustment );
				transform = transform->next;
				}
			FrontView->needsRendering = true;
			InvalidateRect( appWnd, NULL, FALSE );				// repaint window
			}
		}
}


void CmPropagateAdjustment( void )				// apply last adjustment to many sections
{
	HCURSOR cur;
	Section *section;
	Transform *transform;
	int sectnum;
	char *msg = "Propagate moves locked sections, and there is no Undo.";
									
	if ( LastAdjustment && CurrSectionsInfo )	// do only if have last adjustment and sections
		{
		FirstSection = CurrSectionsInfo->first->index;
		LastSection = CurrSectionsInfo->last->index;		// ask user for section range

		if ( DialogBoxParam( appInstance, "SectionRange", appWnd, (DLGPROC)SectionRangeDlgProc, (LPARAM)msg ) == IDOK )
		  {
		  cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow

		  sectnum = FirstSection - 1;
		  section = GetNextSectionBetween( sectnum, LastSection );	// fetch section
		  while ( section )
			{
			if ( section->transforms )						// move all transforms in section
				{
				transform = section->transforms->first;
				while ( transform )
					{										// apply last adjustment to each transform
					section->hasChanged = true;
					if ( LastAdjustment->dim > 3 )
						transform->nform->Compose( LastAdjustment, transform->nform, 1.0, 1.0 );
					else transform->nform->ApplyLinearOf( LastAdjustment );
					transform = transform->next;			// continue with next transform
					}
				}											// save if needed, delete section if not
			PutSection( section, section->hasChanged, section->hasChanged, section->hasChanged );
			section = GetNextSectionBetween( sectnum, LastSection );
			}

		  SetCursor( cur );									// undo hour glass
		  }
		
		}
}

void CmCopyActiveNform( void )			// set last adjustment to nform of selection
{
	if ( FrontView )
	  if ( FrontView->section )
		if ( FrontView->section->active )			// copy tform verbatim
			{
			if ( !LastAdjustment )  LastAdjustment = new Nform();
			for (int i=0; i<DIM; i++)
				{
				LastAdjustment->a[i] = FrontView->section->active->nform->a[i];
				LastAdjustment->b[i] = FrontView->section->active->nform->b[i];
				}
			}
}

BOOL CALLBACK CalibrateSectionsProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	double ps;
	char dlgItemTxt[100];

	switch (message)
		{
		case WM_INITDIALOG:
			sprintf(dlgItemTxt,"%d",FirstSection);
			SetDlgItemText( hwndDlg, ID_FIRSTSECTION, dlgItemTxt );
			sprintf(dlgItemTxt,"%d",LastSection);
   			SetDlgItemText( hwndDlg, ID_LASTSECTION, dlgItemTxt );
			sprintf(dlgItemTxt,"%g",ImagePixelSize);
   			SetDlgItemText( hwndDlg, ID_PIXELSIZE, dlgItemTxt );
			sprintf(dlgItemTxt,"%s/pixel",CurrSeries->units);
   			SetDlgItemText( hwndDlg, ID_UNITS, dlgItemTxt );
			//MessageBeep(MB_ICONHAND);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{	
				case IDYES:
   					GetDlgItemText( hwndDlg, ID_FIRSTSECTION, dlgItemTxt, sizeof(dlgItemTxt) );
					FirstSection = atoi(dlgItemTxt);
   					GetDlgItemText( hwndDlg, ID_LASTSECTION, dlgItemTxt, sizeof(dlgItemTxt) );
					LastSection = atoi(dlgItemTxt);
					GetDlgItemText( hwndDlg, ID_PIXELSIZE, dlgItemTxt, sizeof(dlgItemTxt) );
					ps = atof(dlgItemTxt);
					if ( ps > 0.0 ) ImagePixelSize = ps;
					EndDialog(hwndDlg, 1);			// return non-zero value if user selects Yes
					return TRUE;
				case IDNO:
					EndDialog(hwndDlg, 0);			// otherwise return zero
					return TRUE;
				}
		}
	return FALSE;
}


void CmCalibrateSections( void )	// this only gets called internally from CmCalibrateTraces
{									// to allow reRender to complete before entering this dialog
	Section *section;
	Transform *transform;
	bool changed;
	int sectnum;					// ImagePixelSize is set in CmCalibrateTraces, then in dialog
	
	if ( DialogBox( appInstance, "CalibrateSections", appWnd, (DLGPROC)CalibrateSectionsProc ) )
		{
		sectnum = FirstSection - 1;
		section =  GetNextSectionBetween( sectnum, LastSection );
		while ( section )										// do next section in sequence
			{
			if ( section->transforms )							// do all transforms in section...
				{
				changed = false;
				transform = section->transforms->first;
				while ( transform )
					{
					if ( transform->image )						// found image, delete proxy if present
						{
						transform->image->mag = ImagePixelSize;
						changed = true;
						}
					transform = transform->next;				// continue with next transform
					}
																// save result
				PutSection( section, changed, true, false );
				}												
			section = GetNextSectionBetween( sectnum, LastSection );// find next section > sectnum
			}
		if ( IsWindow(domainWindow) ) PostMessage( domainWindow, UM_UPDATESECTION, 0, (LPARAM)sectnum );
		}

	InvalidateRect( appWnd, NULL, FALSE );		// finally, rerender current section (if needed)
}


void CmRecord( void )
{
	char	menutxt[32];
	HMENU	hMenu;

	if ( Recording )							// if Recording, then Stop
		{
		strcpy(menutxt,"&Start");				// switch start/stop menu text
		if ( LastAdjustment ) delete LastAdjustment;
		LastAdjustment = Recording;
		Recording = NULL;						// set recording to repeat and clear ptr
		}
	else {
		strcpy(menutxt,"&Stop");				// switch start/stop menu text
		Recording = new Nform();				// create an nform to store mvmts
		}

	hMenu = GetMenu( appWnd );					// get menu handle and update menu
	ModifyMenu( hMenu, CM_RECORD, MF_BYCOMMAND | MF_STRING | MF_ENABLED, CM_RECORD, menutxt );
	DrawMenuBar( appWnd );
}


void CenteredMovement( Mvmt &mvmt )				// apply mvmt with respect to center of section or selected traces or domain
{
	Transform *transform, *moveit;
	Contour *domain, *contour, *c;
	Point min, max, allmin, allmax;
	double pixelsize;
	Nform *adjustment;									// will also remember move so can repeat it
	
	adjustment = NULL;

	if ( FrontView )
	  if ( FrontView->section )
		if ( FrontView->section->active )			// flip active image of FrontView Section
			{
			transform = FrontView->section->active;
			if ( transform->image ) 						// image is present
				{
				if ( transform->domain ) // && transform->nform )	// use domain boundary to determine nform...
					{	
					domain = new Contour( *(transform->domain) );// create a copy of domain
					domain->Scale( transform->image->mag );		// go from pixels to units
					domain->InvNform( transform->nform );		// transform into section
					domain->Extent( &min, &max );				//  find extremes...
					delete domain;								// delete transformed domain
					mvmt.centerX = (min.x + max.x)/2.0;			// set domain center
					mvmt.centerY = (min.y + max.y)/2.0;
					FrontView->section->PushUndoState();		// save this for undo
					transform->nform->PostApply( mvmt );
					FrontView->section->hasChanged = true;
					FrontView->needsRendering = true;
					InvalidateRect( appWnd, NULL, FALSE );
					}
				}
			else if ( transform->contours )					// contours are selected
				{
				allmin.x = MAX_FLOAT;								// allmin, allmax will hold extents region to flip
				allmin.y = MAX_FLOAT;
				allmax.x = -MAX_FLOAT;
				allmax.y = -MAX_FLOAT;
				contour = transform->contours->first;
				while ( contour )
					{
					c = new Contour( *contour );					// create a copy of contour
					c->InvNform( transform->nform );				// transform into section
					c->Extent( &min, &max );						//  find extremes...
					delete c;										// delete transformed contour
					if ( min.x < allmin.x ) allmin.x = min.x;		// accummulate min, max
					if ( min.y < allmin.y ) allmin.y = min.y;
					if ( max.x > allmax.x ) allmax.x = max.x;
					if ( max.y > allmax.y ) allmax.y = max.y;
					contour = contour->next;						// use all active contours
					}
				mvmt.centerX = (allmin.x + allmax.x)/2.0;			// set center for flip
				mvmt.centerY = (allmin.y + allmax.y)/2.0;
				FrontView->section->PushUndoState();		// save this for undo
				transform->nform->PostApply( mvmt );			// execute movement
				FrontView->section->hasChanged = true;
				FrontView->needsDrawing = true;
				InvalidateRect( appWnd, NULL, FALSE );
				}
			}
		else
			if ( !FrontView->section->alignLocked )// if no active elements, move entire section if unlocked
			  if ( FrontView->section->HasImage() || FrontView->section->HasContour() )
				{
				FrontView->section->ImageSize( allmin, allmax );
				FrontView->section->ContourSize( min, max );
				if ( min.x < allmin.x ) allmin.x = min.x;			// accummulate min, max
				if ( min.y < allmin.y ) allmin.y = min.y;
				if ( max.x > allmax.x ) allmax.x = max.x;
				if ( max.y > allmax.y ) allmax.y = max.y;
				mvmt.centerX = (allmin.x + allmax.x)/2.0;			// set center for flip
				mvmt.centerY = (allmin.y + allmax.y)/2.0;
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
				InvalidateRect( appWnd, NULL, FALSE );	// repaint window
				}

	if ( adjustment )								// if movement was made, update LastAdjustment
		{
		if ( LastAdjustment ) delete LastAdjustment;
		LastAdjustment = adjustment;				// and add mvmt to recording
		if ( Recording ) Recording->PostApply( mvmt );
		}
}

void CmFlipHorz( void )							// flip selected elements left for right
{
	Mvmt mvmt;									// initial value is no movement (identity)
	mvmt.scaleX = -1.0;							// this mvmt defines mirror around y-axis
	CenteredMovement( mvmt );					// apply w.r.t. center of selected object
}

void CmFlipVert( void )							// flip top for bottom
{
	Mvmt mvmt;									// initial value is no movement (identity)			
	mvmt.scaleY = -1.0;							// this mvmt defines a mirror around x-axis
	CenteredMovement( mvmt );					// apply w.r.t. center of selected object
}

void Cm90Clkwise( void )						// 90 degrees clockwise
{
	Mvmt mvmt;									// initial value is no movement (identity)			
	mvmt.theta = -PI/2.0;						// this mvmt defines a 90 clockwise rotation
	CenteredMovement( mvmt );					// apply w.r.t. center of selected object
}

void Cm90Counterclkwise( void )					// 90 degrees counterclockwise
{
	Mvmt mvmt;									// initial value is no movement (identity)			
	mvmt.theta = PI/2.0;						// this mvmt defines a 90 counterclockwise rotation
	CenteredMovement( mvmt );					// apply w.r.t. center of selected object
}

void CmRotate180( void )						// 180 degrees rotation
{
	Mvmt mvmt;									// initial value is no movement (identity)			
	mvmt.theta = PI;							// this mvmt defines a 180 counterclockwise rotation
	CenteredMovement( mvmt );					// apply w.r.t. center of selected object
}


void CmMagnification( void )		// zoom view to user-specified magnification 
{
	double x, y, nx, ny, pixelsPerUnit;
	
	if ( FrontView && CurrSeries )						// if have a view present
		{
		sprintf(InputDlgName,"Displayed Magnification");
		sprintf(InputDlgString,"%g",1/CurrSeries->pixel_size);
		sprintf(InputDlgValue,"Pixels per %s:",CurrSeries->units);
		if ( DialogBox( appInstance, "InputDlg", appWnd, (DLGPROC)InputDlgProc ) )
			{
			pixelsPerUnit = atof(InputDlgString);		// get user's choice but disallow too small value
			if ( pixelsPerUnit < MIN_FLOAT ) pixelsPerUnit = MIN_FLOAT;
			LastZoom.pixel_size = CurrSeries->pixel_size;				// remember current zoom state
			LastZoom.offset_x = CurrSeries->offset_x;
			LastZoom.offset_y = CurrSeries->offset_y;
			x = (((double)FrontView->width)*CurrSeries->pixel_size)/2.0;// center of window in units
			y = (((double)FrontView->height)*CurrSeries->pixel_size)/2.0;
			CurrSeries->pixel_size = 1/pixelsPerUnit;					// set new pixel size for window
			nx = (((double)FrontView->width)*CurrSeries->pixel_size)/2.0; // new center of window in units
			ny = (((double)FrontView->height)*CurrSeries->pixel_size)/2.0;
			CurrSeries->offset_x += x - nx;								// calculate offsets to keep center
			CurrSeries->offset_y += y - ny;
			InvalidateAllViews();										// flag views for regeneration
			InvalidateRect( appWnd, NULL, FALSE );						// repaint window
			}
		}
}
