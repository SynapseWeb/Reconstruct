///////////////////////////////////////////////////////////////////////////////
//	This file contains the routines for menu operations on an active domain
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
// modified 06/23/05 by JCF
// -+- change: Modified menu enable/disable to accommodate new Reintitialize submenu.
//             Fixed tool button switch when select a domain from section.
//             Modified CmNewDomainFile to merge image into section without selecting it.
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change: Modified NewDomainFile to use new MinPixelsize() method.
//             domainList col widths set by user are remember when refill list. 
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Modified SetDefaultName params.
// modified 3/15/06 by JCF (fiala@bu.edu)
// -+- change: Fixed mode of dialog boxes evoked from Domain List.
// modified 4/25/06 by JCF (fiala@bu.edu)
// -+- change: Added CM_REFRESHLIST and CM_INFOLIST to Domain List.
// modified 6/14/06 by JCF (fiala@bu.edu)
// -+- change: Made domainWindow initial size smaller.
// modified 12/12/06 by JCF (fiala@bu.edu)
// -+- change: Added Copy command to Domain List
// modified 3/15/07 by JCF (fiala@bu.edu)
// -+- change: Added color channel options to domain image Attributes dialog.
// modified 4/2/07 by JCF (fiala@bu.edu)
// -+- change: Modified TestGBMfile to also return the number of images in an image stack
// modified 4/3/07 by JCF (fiala@bu.edu)
// -+- change: Fixed bug in counting proxies, and wrong path when copy in Absolute path mode.
// modified 4/5/07 by JCF (fiala@bu.edu)
// -+- change: Corrected initial domain boundary extent in CmDefaultDomain()
// modified 7/16/07 by JCF (fiala@bu.edu)
// -+- change: Added enable of Maximize sys menu cmd in WM_CREATE of Domain List Window.
//

#include "reconstruct.h"

void EnableDomainMenu(void )			// enable menus items that apply to an active domain
{										// and disable those that don't apply
	int i;
	HMENU hMenu;
	hMenu = GetSubMenu( GetMenu( appWnd ), 3 );	// CHANGE these numbers when add new menus
	EnableMenuItem( hMenu, CM_DOMAINLIST, MF_BYCOMMAND | MF_GRAYED );
	EnableMenuItem( hMenu, CM_NEWDOMAINFILE, MF_BYCOMMAND | MF_GRAYED );
	i = GetMenuItemCount( hMenu );
	do { i--; EnableMenuItem( hMenu, i, MF_BYPOSITION | MF_ENABLED ); }
	while ( i > 2 );
																	// adjust lists for active domain
	if ( IsWindow( domainWindow ) ) DestroyWindow( domainWindow );	// no domain list with one domain
	if ( IsWindow( traceWindow ) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
}

void DisableDomainMenu(void )			// disable menus items that apply to an active domain
{										// and enable those that apply to whole section
	int i;
	HMENU hMenu;
	hMenu = GetSubMenu( GetMenu( appWnd ), 3 );	// CHANGE these numbers when add new menus
	EnableMenuItem( hMenu, CM_DOMAINLIST, MF_BYCOMMAND | MF_ENABLED );
	EnableMenuItem( hMenu, CM_NEWDOMAINFILE, MF_BYCOMMAND | MF_ENABLED );
	i = GetMenuItemCount( hMenu );
	do { i--; EnableMenuItem( hMenu, i, MF_BYPOSITION | MF_GRAYED ); }
	while ( i > 2 );
																	// refresh display of trace list
	if ( IsWindow( traceWindow ) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
}

void SelectDomainFromSection( Section *section, Transform *transform )
{
// begin debug logging...
	DWORD byteswritten;
	char line[MAX_PATH];
	if ( debugLogFile != INVALID_HANDLE_VALUE )
		{
		sprintf(line,"Entered SelectDomainFromSection()\r\n");
		WriteFile(debugLogFile, line, strlen(line), &byteswritten, NULL);
		}
// ...end debug logging
	if ( transform && section )							// if have domain, lift it
		{
		if ( DomainSection ) return;
		DomainSection = new Section();					// create section to for new domain
		DomainSection->index = section->index;
		DomainSection->alignLocked = false;
		section->PushUndoState();						// save section state for later undo
		section->transforms->Extract( transform );		// move domain transform to new domain section
		DomainSection->transforms = new Transforms();	// and add to new transforms list
		DomainSection->transforms->Add( transform );
		DomainSection->active = transform;				// make active for domain contour drawing
		DomainSection->active->isActive = true;
		DomainView = new ViewPort( appWnd );			// create domain view for selected domain
		DomainView->section = DomainSection;
		BackView = FrontView;
		FrontView = DomainView;							// after putting domain in front, update lists
		EnableDomainMenu();								// enable menu operations on domain
		FrontView->needsRendering = true;				// ASSUMES section is FrontView
		BackView->needsRendering = true;				// show with lifted domain removed
		if ( BlendView ) CmBlend();						// turn off blending if present
		InvalidateRect( appWnd, NULL, FALSE );			// because this blend is between sections!
		}
}

BOOL CALLBACK DomainAttrDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	int w, h, pw, ph, bpp, ft, i, n;
	char msgtxt[256], txt[256], ff[6];

	switch (message)			// global variable CurrDomain is used to pass the domain to be modified
		{
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg, ID_DOMAINNAME, WM_SETTEXT, 0, (LPARAM)CurrDomain->domain->name);
			SendDlgItemMessage(hwndDlg, ID_DOMAINFILE, WM_SETTEXT, 0, (LPARAM)CurrDomain->image->src);
			n = 0;
			ft = TestGBMfile( CurrDomain->image->src, &w, &h, &bpp, &n );
			strncpy(ff, fileformats+4*ft, 4);
			ff[4] = '\0';									// retrieve format descriptor string
			if ( n < 2 ) sprintf(msgtxt,"Format: %s",ff);	// and note whether file contains multiple images
			else sprintf(msgtxt,"Format: %s stack of %d",ff,n);
			SendDlgItemMessage(hwndDlg, ID_DOMAINFORMAT, WM_SETTEXT, 0, (LPARAM)msgtxt);
			sprintf(msgtxt,"%d bits/pixel",bpp);
			SendDlgItemMessage(hwndDlg, ID_DOMAINBPP, WM_SETTEXT, 0, (LPARAM)msgtxt);
			sprintf(msgtxt,"Size: %d x %d pixels",w,h);
			SendDlgItemMessage(hwndDlg, ID_DOMAINDIMENSIONS, WM_SETTEXT, 0, (LPARAM)msgtxt);
			sprintf(msgtxt,"%d Kb", w*h*bpp/8192 );
			SendDlgItemMessage(hwndDlg, ID_DOMAINSIZE, WM_SETTEXT, 0, (LPARAM)msgtxt);
			if ( strlen(CurrDomain->image->proxySrc) ) 
				{
				pw = (int)floor((double)w*CurrDomain->image->proxyScale);
				ph = (int)floor((double)h*CurrDomain->image->proxyScale);
				sprintf(msgtxt,"Proxy: %s    (%d x %d)",CurrDomain->image->proxySrc, pw, ph );
				SendDlgItemMessage(hwndDlg, ID_DOMAINPROXY, WM_SETTEXT, 0, (LPARAM)msgtxt);
				}
			sprintf(txt,"%1.*g", Precision,CurrDomain->image->mag);
			SendDlgItemMessage(hwndDlg, ID_PIXELSIZE, WM_SETTEXT, 0, (LPARAM)txt);
			SendDlgItemMessage(hwndDlg, ID_UNITS, WM_SETTEXT, 0, (LPARAM)CurrSeries->units);
			sprintf(txt,"%.3f", CurrDomain->image->contrast);
			SendDlgItemMessage(hwndDlg, ID_CONTRAST, WM_SETTEXT, 0, (LPARAM)txt);
			sprintf(txt,"%.3f", CurrDomain->image->brightness);
			SendDlgItemMessage(hwndDlg, ID_BRIGHTNESS, WM_SETTEXT, 0, (LPARAM)txt);
			if ( CurrDomain->image->red )
				SendDlgItemMessage(hwndDlg, ID_REDCHANNEL, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_REDCHANNEL, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrDomain->image->green )
				SendDlgItemMessage(hwndDlg, ID_GREENCHANNEL, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_GREENCHANNEL, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( CurrDomain->image->blue )
				SendDlgItemMessage(hwndDlg, ID_BLUECHANNEL, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_BLUECHANNEL, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{									// user says OK, so store values from dialog in CurrDomain
				case IDOK:
					SendDlgItemMessage(hwndDlg, ID_DOMAINNAME, WM_GETTEXT, (WPARAM)MAX_CONTOUR_NAME, (LPARAM)CurrDomain->domain->name);
					RemoveIllegalChars( CurrDomain->domain->name );			
					SendDlgItemMessage(hwndDlg, ID_DOMAINFILE, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)CurrDomain->image->src);
					SendDlgItemMessage(hwndDlg, ID_PIXELSIZE, WM_GETTEXT, (WPARAM)256, (LPARAM)txt);
					CurrDomain->image->mag = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_CONTRAST, WM_GETTEXT, (WPARAM)256, (LPARAM)txt);
					CurrDomain->image->contrast = atof(txt);
					SendDlgItemMessage(hwndDlg, ID_BRIGHTNESS, WM_GETTEXT, (WPARAM)256, (LPARAM)txt);
					CurrDomain->image->brightness = atof(txt);
					i = SendDlgItemMessage(hwndDlg, ID_REDCHANNEL, BM_GETCHECK, 0, 0);
					if ( i == BST_CHECKED ) CurrDomain->image->red = true;
					else if ( i == BST_UNCHECKED ) CurrDomain->image->red = false;
					i = SendDlgItemMessage(hwndDlg, ID_GREENCHANNEL, BM_GETCHECK, 0, 0);
					if ( i == BST_CHECKED ) CurrDomain->image->green = true;
					else if ( i == BST_UNCHECKED ) CurrDomain->image->green = false;
					i = SendDlgItemMessage(hwndDlg, ID_BLUECHANNEL, BM_GETCHECK, 0, 0);
					if ( i == BST_CHECKED ) CurrDomain->image->blue = true;
					else if ( i == BST_UNCHECKED ) CurrDomain->image->blue = false;
					EndDialog(hwndDlg, 1);
					return TRUE;

				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				}
		}
	return FALSE;
}

void CmDomainAttributes( void )				// display and changes the domain attributes
{
	char name[MAX_CONTOUR_NAME];

	if ( DomainSection )					// do only if have active domain section image
		if ( DomainSection->active )
			if ( DomainSection->active->image )
				{
				CurrDomain = DomainSection->active;		// dialog will use the CurrDomain pointer
				DomainSection->PushUndoState();
				if ( DialogBox( appInstance, "DomainAttrDlg", appWnd, (DLGPROC)DomainAttrDlgProc ) == IDOK )
					{
					DomainSection->hasChanged = true;			// current section always corresponds to domain section
					CurrSection->SetDefaultName(name,CurrDomain->domain->name,true);
					strcpy(CurrDomain->domain->name,name);
					if ( DomainView )
						{
						DomainView->needsRendering = true;
						if ( DomainView == FrontView ) InvalidateRect(appWnd,NULL,FALSE);
						}
					}
				}
}

void FillDomainList( HWND list )			// fill listview with domains from FrontView section
{
	Transform *transform;
	Contour *c;
	Point min, max;
	LV_ITEM lvi;
	LV_COLUMN column;
	int i, colwidth[MAX_DOMAINLISTCOLS];
	double x, y, area, length;
	char txt[MAX_COMMENT];

	SendMessage( list, LVM_DELETEALLITEMS, 0, 0 );		// clear list

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;	// setup columns masks
	column.fmt = LVCFMT_LEFT; column.pszText = txt;	column.cchTextMax = MAX_COMMENT;

	for (i=MAX_DOMAINLISTCOLS-1; i>0; i--)				// clear all columns
		{
		colwidth[i] = 100;								// default column width
		if ( SendMessage(list, LVM_GETCOLUMN, i, (LPARAM)(&column) ) )
			{
			colwidth[i] = column.cx;					// if exists, remember old width
			SendMessage(list, LVM_DELETECOLUMN, i, 0);	// delete column from listview
			}
		}

	i = 1;
	if ( CurrSeries->listDomainSource )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Source");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listDomainPixelsize)
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Pixel size");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listDomainLength )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Length");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listDomainArea )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Area");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listDomainMidpoint )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Midpoint X");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		column.cx = colwidth[i]; column.iSubItem = i; strcpy(txt,"Midpoint Y");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}

	lvi.iItem = 0;
	if ( FrontView->section )
	  if ( FrontView->section->transforms )			// find and list every domain in section
		{
		transform = FrontView->section->transforms->first;
		while ( transform )
			{
			if ( transform->image && transform->domain )
			  if ( MatchLimit( transform->domain->name, limitDomainList ) )
				{												// add list item...
				lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
				lvi.state = 0; lvi.stateMask = 0; lvi.iImage = 0;
				if ( transform->domain->hidden ) lvi.iImage = 1;
				lvi.lParam = transform->domain->Id;				// to reference specific domain in the section
				lvi.iSubItem = 0; lvi.pszText = transform->domain->name;
				SendMessage(list, LVM_INSERTITEM, lvi.iItem, (LPARAM)(&lvi));

				lvi.mask = LVIF_TEXT;							// now set text of other columns
				i = 1;
				if ( CurrSeries->listDomainSource )
					{
					lvi.iSubItem = i++; lvi.pszText = transform->image->src;
					SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
					}
				if ( CurrSeries->listDomainPixelsize )
					{
					lvi.iSubItem = i++; lvi.pszText = txt;
					sprintf(txt,"%1.*g", Precision, transform->image->mag);
					SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
					}
				if ( CurrSeries->listDomainArea || CurrSeries->listDomainLength || CurrSeries->listDomainMidpoint )
					{
					c = new Contour( *(transform->domain) );
					c->Scale( transform->image->mag );			// put into section units
					c->InvNform( transform->nform );			// transform into section
					if ( CurrSeries->listDomainLength )
						{
						length = c->Length();
						lvi.iSubItem = i++; lvi.pszText = txt;	// display length
						sprintf(txt,"%1.*g", Precision, length);
						SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
						}
					if ( CurrSeries->listDomainArea )
						{
						c->GreensCentroidArea( x, y, area );
						lvi.iSubItem = i++; lvi.pszText = txt;	// display area
						sprintf(txt,"%1.*g", Precision, area);
						SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
						}
					if ( CurrSeries->listDomainMidpoint )
						{
						c->Extent( &min, &max );
						x = (max.x+min.x)/2.0;							// compute midpoint of contour
						y = (max.y+min.y)/2.0;
						lvi.iSubItem = i++; lvi.pszText = txt;			// display midpoint x
						sprintf(txt,"%1.*g", Precision, x);
						SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
						lvi.iSubItem = i; lvi.pszText = txt;			// display midpoint y
						sprintf(txt,"%1.*g", Precision, y);
						SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
						}
					delete c;
					}
				}
			transform = transform->next;			// do next transform
			}
		}					
}

void EditDomain( HWND list )		// get listview selection and edit attributes
{
	Transform *transform;
	bool found = false;
	LV_ITEM lvi;
	char name[MAX_CONTOUR_NAME];
													// looking for domain with Id mathing lParam of selection 
	lvi.mask = LVIF_PARAM;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{														// an item was selected
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));		// get the lParam (Domain Id) of iItem
		transform = FrontView->section->FindDomainId( lvi.lParam );		// find domain in section with matching Id
		if ( transform )
			{
			found = true;
			CurrDomain = transform;
			FrontView->section->PushUndoState();
			if ( DialogBox( appInstance, "DomainAttrDlg", list, (DLGPROC)DomainAttrDlgProc ) == IDOK )
				{
				FrontView->section->hasChanged = true;			// set default name here for correct increment
				FrontView->section->SetDefaultName(name,CurrDomain->domain->name,true);
				strcpy(CurrDomain->domain->name,name);
				FrontView->needsRendering = true;
				}
			}
		}

	if ( found )
		{
		InvalidateRect(appWnd,NULL,FALSE);
		FillDomainList( domainList );			// update list to reflect changes
		}
}

void SelectDomain( HWND list )		// get listview selection and select it from section
{
	Transform *transform;
	LV_ITEM lvi;
														// looking for domain with Id mathing lParam of selection
	lvi.mask = LVIF_PARAM;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));		// get the lParam (Domain Id) of iItem
		transform = FrontView->section->FindDomainId( lvi.lParam );	// find domain with matching Id
		if ( transform )
			{													// select the domain from section
			FrontView->section->PushUndoState();
			transform->domain->hidden = false;					// but first unhide it if hidden
			SelectDomainFromSection( FrontView->section, transform );
			CmEscapeCurrentTool();								// abandon current tool and
			SwitchTool(DOMAIN_TOOL);							// switch to domain tool
			}
		}
}

void DeleteDomain( HWND list )			// delete listview selection
{
	Transform *transform;
	bool found = false;
	LV_ITEM lvi;
															// find selected item in list and get lParam
	lvi.mask = LVIF_PARAM;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{														// an item was selected
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));		// get the lParam (Domain Id) of iItem
		transform = FrontView->section->FindDomainId( lvi.lParam );	// find domain with matching Id
		if ( transform )
			{
			found = true;
			FrontView->section->PushUndoState();
			FrontView->section->hasChanged = true;
			FrontView->section->transforms->Extract( transform );
			delete transform;
			if ( CurrSeries->beepDeleting ) MessageBeep(0xFFFFFFFF);
			FrontView->needsRendering = true;
			}	
		}

	if ( found )
		{
		InvalidateRect( appWnd, NULL, FALSE );
		FillDomainList( list );			// refill list to note deletion
		}
}

void CopyDomain( HWND list )			// copy listview selection
{
	Transform *transform, *domain;
	bool found = false;
	LV_ITEM lvi;
// begin debug logging...
	DWORD byteswritten;
	char line[MAX_PATH];
	if ( debugLogFile != INVALID_HANDLE_VALUE )
		{
		sprintf(line,"Entered CopyDomain()\r\n");
		WriteFile(debugLogFile, line, strlen(line), &byteswritten, NULL);
		}
// ...end debug logging
															// find selected item in list and get lParam
	lvi.mask = LVIF_PARAM;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{														// an item was selected
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));		// get the lParam (Domain Id) of iItem
		transform = FrontView->section->FindDomainId( lvi.lParam );	// find domain with matching Id
		if ( transform )
			{
			found = true;
			domain = new Transform( *transform );			// create copy of the domain
			FrontView->section->PushUndoState();
			FrontView->section->hasChanged = true;
			FrontView->section->transforms->Add( domain );	// add it to the section
			FrontView->needsRendering = true;
			}	
		}

	if ( found )
		{
		InvalidateRect( appWnd, NULL, FALSE );
		FillDomainList( list );								// refill list to show addition
		}
}

void HideListDomain( HWND list, bool hideit )		// hide/unhide listview selection
{
	Transform *transform;
	bool found = false;
	LV_ITEM lvi;
															// find selected item in list and get lParam
	lvi.mask = LVIF_PARAM;
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{													// an item was selected
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));	// get the lParam (Domain Id) of iItem
		transform = FrontView->section->FindDomainId( lvi.lParam );	// find domain with matching Id
		if ( transform )
		  if ( transform->domain->hidden != hideit )		// change only if hidden attribute changes...
			{
			found = true;									// list will need updating
			FrontView->section->PushUndoState();
			FrontView->section->hasChanged = true;
			transform->domain->hidden = hideit;				// hide/unhide domain
			FrontView->needsRendering = true;				// and redisplay section with change
			}	
		}

	if ( found )
		{
		InvalidateRect( appWnd, NULL, FALSE );
		FillDomainList( list );								// refill list to note deletion
		}
}


LRESULT APIENTRY ListDomainsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pnmh;
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
		{												// initialize the list view section info
		case WM_CREATE:
			GetClientRect( hWnd, &r );
			w = r.right-r.left-10; h = r.bottom-r.top-10,
			domainList = CreateWindow( WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL,
														5, 5, w, h, hWnd, NULL, appInstance, NULL);

			himl = ImageList_Create(16, 16, TRUE, 2, 1);								// setup icons
			ImageList_AddIcon(himl, LoadIcon(appInstance, "DomainIcon"));
			ImageList_AddIcon(himl, LoadIcon(appInstance, "HdomainIcon"));
			SendMessage(domainList, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)(himl));
			column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; column.fmt = LVCFMT_LEFT; 
			column.pszText = txt; strcpy(txt,"Domain"); column.cx = 100;				// setup columns
			for ( column.iSubItem = 0; column.iSubItem < MAX_DOMAINLISTCOLS; column.iSubItem++ )
				SendMessage(domainList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			FillDomainList( domainList );								// fill listview from FrontView->section
			sprintf(txt,"Section %d domains %s",FrontView->section->index,limitDomainList);
			SetWindowText(hWnd,txt);									// window title displays section number
			SetFocus( domainList );
			hMenu = GetSystemMenu( hWnd, FALSE );						// allow Maximize on titlebar menu
			EnableMenuItem( hMenu, SC_MAXIMIZE, MF_ENABLED );
			break;

		case WM_SIZE:									// resize list view control to match dialog box
			GetClientRect( hWnd, &r );
			SetWindowPos( domainList, 0, 0, 0, r.right-r.left-10, r.bottom-r.top-10, SWP_NOMOVE | SWP_NOOWNERZORDER);
			break;
			
		case WM_DESTROY:
			domainWindow = NULL;						// clear global handle pointer
			hMenu = GetMenu( appWnd );
			CheckMenuItem( hMenu, CM_DOMAINLIST, MF_BYCOMMAND | MF_UNCHECKED );	// uncheck menu item
			SetFocus(appWnd);							// return to main window
			break;

		case WM_NOTIFY:									// process listview notification messages
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case LVN_KEYDOWN:
					key = (LV_KEYDOWN *)lParam;			// allow control+TAB to switch active window
					if ( (key->wVKey==VK_TAB) && (GetKeyState(VK_CONTROL) & 0x8000) ) CmNextWindow( domainWindow );
					if ( key->wVKey==VK_DELETE ) DeleteDomain( domainList );
					if ( key->wVKey !=  VK_RETURN )		// fall through to double-click if Enter key pressed
						break;

				case NM_DBLCLK:							// user double-clicks or presses enter, select them
					SelectDomain( domainList );
					SetFocus(appWnd);					// and switch to main window
					return TRUE;
				}
			break;

		case UM_UPDATESECTION:							// the front section has changed!
			FillDomainList( domainList );
			sprintf(txt,"Section %d domains %s",FrontView->section->index,limitDomainList);
			SetWindowText(hWnd,txt);
			break;

		case WM_SETFOCUS:
			SetFocus(domainList );						// activate list view if window activated
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{										// edit domain attributes
				case CM_SELECTDOMAIN:
					SelectDomain( domainList );
					SetFocus(appWnd);					// and switch to main window
					break;

				case CM_REFRESHLIST:					// regenerate the list
					FillDomainList( domainList );
					break;

				case CM_INFOLIST:						// display count of items
					DisplayListInfo( domainList );
					break;

				case CM_SAVELIST:
					sprintf(filename,"domains%d.csv",FrontView->section->index);
					ZeroMemory(&ofn, sizeof(ofn));		// clear the OPENFILENAME fields
					ofn.lStructSize = sizeof(ofn);		// set only the necessary values
					ofn.hwndOwner = appWnd;
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "All Files\0*.*\0Comma Separated Value file\0*.csv\0\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrTitle = "Save Domain List As...";
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
					if ( GetSaveFileName(&ofn) == TRUE )				// display the Open dialog box
						{
						SaveList( domainList, filename );
						}
					break;

				case CM_EDITDOMAIN:						// change attributes of listview selection
					EditDomain( domainList );
					break;

				case CM_HIDELISTDOMAIN:					// hide selected domain from view
					HideListDomain( domainList, true );
					break;

				case CM_UNHIDELISTDOMAIN:				// unhide selected domain from view
					HideListDomain( domainList, false );
					break;

				case CM_DELETELISTDOMAIN:				// delete selected domain
					DeleteDomain( domainList );
					break;

				case CM_COPYLISTDOMAIN:				// delete selected domain
					CopyDomain( domainList );
					break;
				}
		}
	return( DefWindowProc(hWnd, message, wParam, lParam) );
}


HWND MakeDomainListWindow( void )			// display list for user selection of objects to view
{
	WNDCLASS wndclass;
	HWND sWnd;
	HMENU hMenu;
	RECT r;
	int width;
														// create the object list window...
	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)ListDomainsProc;
	wndclass.cbClsExtra  = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance = appInstance;
	wndclass.hCursor   = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon     = NULL;							// not used for WS_EX_TOOLWINDOW
	wndclass.lpszMenuName = "DomainListMenu";
	wndclass.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wndclass.lpszClassName = "DomainListClass";
	RegisterClass(&wndclass);

	ClientRectLessStatusBar( &r );						// position window in upper left corner of client
	ClientRectToScreen( appWnd, &r );
	width = 140;										// set width based on how many data fields present
	if ( CurrSeries->listDomainSource )
		width += 100;
	if ( CurrSeries->listDomainPixelsize)
		width += 100;
	if ( CurrSeries->listDomainLength )
		width += 100;
	if ( CurrSeries->listDomainArea )
		width += 100;
	if ( CurrSeries->listDomainMidpoint )
		width += 200;

	sWnd = CreateWindowEx( WS_EX_TOOLWINDOW, "DomainListClass", "Domains", WS_OVERLAPPEDWINDOW,
								r.left+20, r.top+20, width, r.top+200, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

	if ( sWnd )
		{											// if window created, display it
		ShowWindow(sWnd, SW_SHOW);			// and check corresponding menu item in main window	
		hMenu = GetMenu( appWnd );
		CheckMenuItem( hMenu, CM_DOMAINLIST, MF_BYCOMMAND | MF_CHECKED );
		}

	return( sWnd );
}

void CmDomainList( void )									// create or destroy domain list window
{
	if ( IsWindow( domainWindow ) ) DestroyWindow( domainWindow );						
	else if ( FrontView )
		  if ( FrontView->section )			
			domainWindow = MakeDomainListWindow();			// create list window
}
													// customization procedure for ImportFile dialog

UINT APIENTRY ImportDlgProc(HWND hwndDlg, UINT message, WPARAM, LPARAM lParam)
{
	char txt[64];
	OFNOTIFY *ofmsg;
	switch (message)
		{											// set magnification to default value
		case WM_INITDIALOG:
			sprintf(txt,"%1.*g", Precision,ImagePixelSize);
   			SetDlgItemText( hwndDlg, ID_INPUTVALUE, txt );
			if ( CopyFiles ) SendDlgItemMessage(hwndDlg, ID_COPYFILES, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_COPYFILES, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			break;
		case WM_NOTIFY:								// detect OK button so can update value
			ofmsg = (LPOFNOTIFY)lParam;
			switch ( ofmsg->hdr.code )
			case CDN_FILEOK:						// update ImagePixelSize value before exit
				{
   				GetDlgItemText( hwndDlg, ID_INPUTVALUE, txt, sizeof(txt) );
				ImagePixelSize = atof( txt );
				if ( SendDlgItemMessage(hwndDlg, ID_COPYFILES, BM_GETCHECK, 0, 0) == BST_CHECKED )
					CopyFiles = true;
				else CopyFiles = false;
				}
		}
	return 0;
}

void CmNewDomainFile( void )					// import a new domain image from a file
{
	Transform *transform;
	OPENFILENAME  ofn;		  								// common dialog box structure
	HCURSOR cur;
	Point min, max;
	int d;
	double pixel_size;
	char srcpath[MAX_PATH], destpath[MAX_PATH];
	char localpath[MAX_PATH], filename[MAX_PATH], msgtxt[MAX_PATH];

	if ( !CurrSection )
		{ ErrMsgOK( ERRMSG_NEEDSECTION, NULL ); return; }	// need a section to import into

															// look for likely pixelsize in memory
	if ( PrevSection )
		if ( PrevSection->HasImage() ) ImagePixelSize = PrevSection->MinPixelsize();
	if ( CurrSection->HasImage() ) ImagePixelSize = CurrSection->MinPixelsize();
	
	strcpy(srcpath,"*.bmp\0");								// initialize OPENFILENAME data
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = appWnd;
	ofn.hInstance = appInstance;
	ofn.lpstrFile = srcpath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = "All Files\0*.*\0Windows Bitmaps\0*.bmp\0TIFF Images\0*.tif\0GIF Images\0*.gif\0JPEG Images\0*.jpg\0\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrTitle = "Import Image\0";
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE
					| OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	ofn.lpfnHook = ImportDlgProc;
	ofn.lpTemplateName = "ImportCustom\0";				// customize to allow use to input ImagePixelSize

	if ( GetOpenFileName(&ofn) == TRUE )				// display the Open dialog box.
		{
		cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );	// file access could be slow

		strcpy(filename,srcpath+ofn.nFileOffset);		// break off just filename
		MakeLocalPath(WorkingPath,srcpath,localpath);	// make local path to file
		if ( strcmp(filename,localpath) )				// if workingpath is not same as file path
			if ( CopyFiles )							// try to copy file to workingpath
				{
				sprintf(destpath,"%s%s",WorkingPath,filename);	// make path to folder of series
				if ( !CopyFile( srcpath, destpath, FALSE ) )	// if copy fails
					ErrMsgOK( ERRMSG_COPY_FAILED, srcpath );
				else 
					{
					strcpy(localpath,filename);			// adjust paths to reflect copied locale
					strcpy(srcpath,destpath);			// in either local or absolute path mode
					}
				}

		FrontView->section->PushUndoState();			// save section state for later undo
														// add image domain transform to section
		if ( CurrSeries->useAbsolutePaths )
			transform = FrontView->section->AddNewImage(srcpath, ImagePixelSize);
		else transform = FrontView->section->AddNewImage(localpath, ImagePixelSize);

		if ( transform )								// if successful, then redraw section with new domain
			{
			FrontView->needsRendering = true;
			if ( BlendView ) CmBlend();						// turn off blending if present
			InvalidateRect( appWnd, NULL, FALSE );
			}
		else											// if reading of image file fails
			FrontView->section->PopUndoState();			// forget about change

		SetCursor( cur );							// done with import					
		}
}

void CmMergeFront( void )
{	
	Transform *domain;
// begin debug logging...
	DWORD byteswritten;
	char line[MAX_PATH];
	if ( debugLogFile != INVALID_HANDLE_VALUE )
		{
		sprintf(line,"Entered CmMergeFront()\r\n");
		WriteFile(debugLogFile, line, strlen(line), &byteswritten, NULL);
		}
// ...end debug logging
					
	if ( DomainSection )					// move active domain to front of Curr section		
		{
		domain = DomainSection->ExtractActiveTransform();
		domain->isActive = false;
		if ( !CurrSection->transforms ) 
			CurrSection->transforms = new Transforms();
		CurrSection->transforms->Add( domain );
		FrontView = CurrView;				// repair Current View
		CurrSection->hasChanged = true;		// mark as changed now, instead of when change domain
		FrontView->needsRendering = true;
		BackView = PrevView;				// return PrevView (if any) to BackView
		if ( BlendView ) CmBlend();			// turn off blending to show merged domain
		delete DomainView;					// discard stuff for lifted domain
		DomainView = NULL;
		delete DomainSection;
		DomainSection = NULL;
		DisableDomainMenu();				// disable menu items
		InvalidateRect( appWnd, NULL, FALSE );
		}
}

void CmMergeRear( void )
{
	Transform *domain;
										
	if ( DomainSection )					// move active domain to back of Curr section		
		{
		domain = DomainSection->ExtractActiveTransform();
		domain->isActive = false;
		if ( !CurrSection->transforms ) 
			CurrSection->transforms = new Transforms();
		CurrSection->transforms->AddFirst( domain );
		FrontView = CurrView;				// repair Current View
		CurrSection->hasChanged = true;
		FrontView->needsRendering = true;
		BackView = PrevView;				// return PrevView (if any) to BackView
		if ( BlendView ) CmBlend();			// turn off blending to show merged domain
		delete DomainView;					// discard stuff for lifted domain
		DomainView = NULL;
		delete DomainSection;
		DomainSection = NULL;
		DisableDomainMenu();				// disable domain menu
		InvalidateRect( appWnd, NULL, FALSE );
		}
}

void CmRestoreContrast( void )
{												
	if ( DomainSection )					// do only if have active domain section image
		if ( DomainSection->active )
			if ( DomainSection->active->image )				// reset contrast and brightness values
				{											// and rerender on screen
				DomainSection->PushUndoState();
				DomainSection->active->image->contrast = 1.0;
				DomainSection->active->image->brightness = 0.0;
				DomainView->needsRendering = true;
				InvalidateRect( appWnd, NULL, FALSE );
				}
}

void CmDeleteDomain( void )
{												
	if ( DomainSection )		
		{									// CurrView is already rendered without domain
		FrontView = CurrView;				// so just use it and previous view if present
		FrontView->section->hasChanged = true;
		BackView = PrevView;
		if ( BlendView ) CmBlend();			// turn off blending to show section
		delete DomainView;					// discard stuff of lifted domain
		DomainView = NULL;
		delete DomainSection;				// delete the domain's section
		DomainSection = NULL;
		if ( CurrSeries->beepDeleting ) MessageBeep(0xFFFFFFFF);
		DisableDomainMenu();				// disable domain menu
		InvalidateRect( appWnd, NULL, FALSE );
		}
}

void CmDefaultDomain( void )				// reset domain contour to default domain
{
	Point *p;
	int ft, w, h, bpp, n;
	char name[MAX_CONTOUR_NAME];

	if ( DomainSection )					// do only if have active domain section image
	  if ( DomainSection->active )
		if ( DomainSection->active->image )
		  {
		  ft = TestGBMfile( DomainSection->active->image->src, &w, &h, &bpp, &n );	// might have proxy loaded!
		  if ( ft ) 																// filetype was recognized
			{
			DomainSection->PushUndoState();
			strcpy( name, DomainSection->active->domain->name );
			delete DomainSection->active->domain;
			DomainSection->active->domain = new Contour();			// create domain contour for image boundary
			strcpy(DomainSection->active->domain->name,name);
			DomainSection->active->domain->points = new Points();	// make domain image boundary in pixels
			p = new Point(0.0,0.0,0.0);	
			DomainSection->active->domain->points->Add(p);
			p = new Point((double)(w-1),0.0,0.0);
			DomainSection->active->domain->points->Add(p);
			p = new Point((double)(w-1),(double)(h-1),0.0);
			DomainSection->active->domain->points->Add(p);
			p = new Point(0.0,(double)(h-1),0.0);
			DomainSection->active->domain->points->Add(p);
			DomainView->needsRendering = true;						// redraw image becuz boundary of image changed
			InvalidateRect( appWnd, NULL, FALSE );
			}
		  }						
}

void CmClearDomainTransform( void )				// clear entire transform of selected domain
{
	if ( DomainSection )
	  if ( DomainSection->active )
		if ( DomainSection->active->image )
			{
			DomainSection->PushUndoState();		// save this for undo
			DomainSection->active->nform->Clear();
			DomainSection->hasChanged = true;
			DomainView->needsRendering = true;
			InvalidateRect( appWnd, NULL, FALSE );
			}
}
