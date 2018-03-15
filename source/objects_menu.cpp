////////////////////////////////////////////////////////////////////////////////
//	This file contains the routines for processing the Objects menu operations
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
// modified 02/08/05 by JCF (fiala@bu.edu)
// -+- change: Added check for object != NULL after CurrObject match to prevent fatal when list out of date
// -+- change: Added proper deletion of object_list at end of all list processing operations
// -+- change: Added CopyObjects on Object List; extracted AddObjectToList from CompleteObjectList to use in CopyObjects
// -+- change: Added message after Trace Attributes changed to indicate list should be reopened.
// modified 3/07/05 by JCF (fiala@bu.edu)
// -+- change: Switched ZTrace list operations to use ZTrace Id parameter for identification.
// modified 3/10/05 by JCF (fiala@bu.edu)
// -+- change: Modified SimplifyObjects to report when nothing changes.
// modified 7/7/05 by JCF (fiala@bu.edu)
// -+- change: Fixed order of processing in ObjectAttributes() to only load each section once.
// modified 7/8/05 by JCF (fiala@bu.edu)
// -+- change: Reorganized Object List menus and added HideObject and ZTraceObject.
// modified 7/13/05 by JCF (fiala@bu.edu)
// -+- change: Added ResetSceneIcons and Reset to AddSelectedZTracesToScene().
//             ztraceList col widths set by user are remember when refill list.
//             fixed Reset, update display, and icons when change scene from z-trace list.
// modified 11/1/05 by JCF (fiala@bu.edu)
// -+- change:  Added Rename Objects command, and customized Object Attributes dialog.
// modified 11/2/05 by JCF (fiala@bu.edu)
// -+- change:  Fixed Rename Objects to work over a selected range.
// modified 11/7/05 by JCF (fiala@bu.edu)
// -+- change:  Fixed bug in generating 3D objects while thread still working on selectedObjects.
// modified 11/17/05 by JCF (fiala@bu.edu)
// -+- change:  Added progress to object list reading.
// modified 1/26/06 by JCF (fiala@bu.edu)
// -+- change: Modified RenameObject to delete old item names from list after renaming to prevent crash if use them.
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Modified SetDefaultName params.
// modified 3/15/06 by JCF (fiala@bu.edu)
// -+- change: Fixed dialog modes for Object List dialogs so user can't run multiple dialogs simultaneously.
// modified 3/28/06 by JCF (fiala@bu.edu)
// -+- change: Remove object from scene when rename.
// modified 4/25/06 by JCF (fiala@bu.edu)
// -+- change: Added CM_REFRESHLIST and CM_INFOLIST to ZTrace List, Object List, and Distances List.
// modified 5/2/06 by JCF (fiala@bu.edu)
// -+- change: Complete addition of Create Z Tracce from Object List item.
// modified 5/4/06 by JCF (fiala@bu.edu)
// -+- change: Added CopyZTraces, SmoothZTraces to ZTrace List, SmoothObjects to Object List.
// modified 5/11/06 by JCF (fiala@bu.edu)
// -+- change: Added GridAtZTraces to create grids at z-trace points.
// modified 6/8/06 by JCF (fiala@bu.edu)
// -+- change: Changed message at end of Rename Objects.
// modified 6/14/06 by JCF (fiala@bu.edu)
// -+- change: Fixed bug that caused ObjectList reading to appear to hang after ztraceWindow closed.
// modified 6/23/06 by JCF (fiala@bu.edu)
// -+- change: Changed name of SetFillMode() to SetFillModeObject() to avoid conflict with trace_menu.cpp
// modified 2/5/07 by JCF (fiala@bu.edu)
// -+- change: Added some debug logging to look for the Object List hanging problem.
// modified 4/23/07 by JCF (fiala@bu.edu)
// -+- change: Modified CmFindObjectNamed() to Object List. (SHOULD THIS BE FIRST MENU ITEM?)
// modified 5/8/07 by JCF (fiala@bu.edu)
// -+- change: Fixed ObjectList (and DistancesList?) hanging by having thread procedure post to appWnd only.
// modified 7/16/07 by JCF (fiala@bu.edu)
// -+- change: Added enable of Maximize sys menu cmd in WM_CREATE of Object, Z-Trace, and Distances Windows.
//

#include "reconstruct.h"

void AddSelectedObjectsToScene( HWND list )		// add listview selections to CurrScene and display them
{
	VRMLObject *object, *selected;
	LV_ITEM lvi;
	char name[MAX_CONTOUR_NAME];
	
	if ( selectedObjects ) return;				// do nothing if selectedObjects still processing
	selectedObjects = new VRMLObjects();

	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	while ( lvi.iItem >= 0 )
		{												// an item was selected, retrieve object name
		lvi.mask = LVIF_TEXT; 
		lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
		
		object = CurrObjects->Match( name );			// find object info in list

		selected = new VRMLObject();
		strcpy(selected->name,object->name);			// copy info to scene object
		selected->firstSection = max(object->firstSection,CurrSeries->first3Dsection);
		selected->lastSection = min(object->lastSection,CurrSeries->last3Dsection);
		selectedObjects->Add( selected );				// add to selected list

		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		}

	if ( selectedObjects->Number() ) Generate3DThreadBegin();
	else { delete selectedObjects; selectedObjects = NULL; }
}

void ResetSceneIcons( HWND list )		// reset all list icons to reflect status in scene
{
	VRMLObject *sceneObject;
	LV_ITEM lvi;
	char name[MAX_CONTOUR_NAME];

	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL) );
	while ( lvi.iItem >= 0 )
		{			
		lvi.iSubItem = 0; lvi.iImage = 0;				// change back to object icon
		if ( CurrScene )
		  if ( CurrScene->objects )
			{
			lvi.mask = LVIF_TEXT; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
			SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
			sceneObject = CurrScene->objects->Match( name );// object already exists in scene!
			if ( sceneObject ) lvi.iImage = 1;				// change back to object icon
			}
		lvi.mask = LVIF_IMAGE;
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL) );
		}
}

void RemoveSelectedObjectsFromScene( HWND list )		// remove listview selections from CurrScene
{
	VRMLObject *sceneObject;
	LV_ITEM lvi;
	char name[MAX_CONTOUR_NAME];

	if ( CurrScene )			// if there is a scene, search for matching selected objects
		{
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		while ( lvi.iItem >= 0 )
			{													// an item was selected, retrieve object name
			lvi.mask = LVIF_TEXT; 
			lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
			SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
		
			if ( CurrScene->objects )
				{
				sceneObject = CurrScene->objects->Match( name );// object already exists in scene!
				if ( sceneObject )
					{
					CurrScene->objects->Extract( sceneObject );	// remove and delete it
					delete sceneObject;
					}
				}
		
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			}

		if ( IsWindow( openGLWindow ) )
			{
			CurrScene->Compile();								// recompile scene to OpenGL
			InvalidateRect(openGLWindow,NULL,FALSE);
			}
		ResetSceneIcons( list );								// update icons in list	
		}
}

void CmFindObjectNamed( HWND list )		// remove listview selections from CurrScene
{
	VRMLObject *object;
	LV_ITEM lvi;
	char name[MAX_CONTOUR_NAME];

	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{													// an item was selected, retrieve object name
		lvi.mask = LVIF_TEXT; 
		lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
	
		object = CurrObjects->Match( name );				// find object info in list
		if ( object )
			ShowSectionTrace( object->firstSection, name );	// jump to first trace of object in series
		}
}

void HideObjects( HWND list, bool hide )		// modify traces of listview selections to set hidden==hide
{
	VRMLObjects *object_list;
	VRMLObject *object, *todo_object;
	Section *section;
	Transform *transform;
	Contour *contour;
	int i, sectnum, first, last;
	LV_ITEM lvi;
	HCURSOR cur;
	bool changed;
	char name[MAX_CONTOUR_NAME];
	
	object_list = new VRMLObjects();					// create a list of objects to modify
	first = MAX_INT32;
	last = 0;											// remember total section range of objects

	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	while ( lvi.iItem >= 0 )
		{												// an item was selected, retrieve object name
		lvi.mask = LVIF_TEXT; 
		lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
	
		object = CurrObjects->Match( name );			// find object info in list
		if ( object )
			{
			todo_object = new VRMLObject();
			strcpy(todo_object->name,object->name);			// copy info to object to be modified
			todo_object->firstSection = object->firstSection;
			todo_object->lastSection = object->lastSection;
			object_list->Add( todo_object );
			if ( first > todo_object->firstSection ) first = todo_object->firstSection;
			if ( last < todo_object->lastSection ) last = todo_object->lastSection;
			}
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		}

	i = object_list->Number();
	if ( i > 0 )					// have at least one object to modify
		{
		cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );// show hourglass, file access could be slow

		sectnum = first-1;
		section = GetNextSectionBetween( sectnum, last );	// fetch section
		while ( section )
			{
			changed = false;								// initially, no changes to section
			object = object_list->first;
			while ( object )								// search object list for objects to do
				{
				if ( (sectnum <= object->lastSection) && (sectnum >= object->firstSection) )
					{
					if ( section->transforms )				// object could be in section, search contours
						{
						transform = section->transforms->first;
						while ( transform )								// check each transform for contours
						  {
						  if ( transform->contours )
							{											// for all contours, look for object contours
							contour = transform->contours->first;
							while ( contour )
								{
								if ( strcmp(contour->name,object->name) == 0 )		// FOUND! change it!
									{
									if ( contour->hidden != hide ) changed = true;
									contour->hidden = hide;				// change hidden state
									}
								contour = contour->next;				// do next contour in section
								}
							}
						  transform = transform->next;
						  }
						}
					} // end if sectnum

				object = object->next;	
				}						// end while object

			  PutSection( section, changed, false, changed );			// save or delete section and redraw if current
			  section = GetNextSectionBetween( sectnum, last );
			  }

		SetCursor( cur );									// restore cursor 
		if ( CurrSection ) CurrSection->UnselectHidden();	// remove any just hidden traces	
		if ( PrevSection ) PrevSection->UnselectHidden();	// from activ ones in sections
		if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
		}											

	delete object_list;							// free memory
}

void SetFillModeObject( HWND hwndDlg, int nofill, int maskfill, int copyfill, int mergefill )
{
	SendDlgItemMessage(hwndDlg, ID_NOFILL, BM_SETCHECK, (WPARAM)nofill, 0);
	SendDlgItemMessage(hwndDlg, ID_MASKFILL, BM_SETCHECK, (WPARAM)maskfill, 0);
	SendDlgItemMessage(hwndDlg, ID_COPYFILL, BM_SETCHECK, (WPARAM)copyfill, 0);			
	SendDlgItemMessage(hwndDlg, ID_MERGEFILL, BM_SETCHECK, (WPARAM)mergefill, 0);
}

void FillDlgObject( HWND hwndDlg )
{
	SendDlgItemMessage(hwndDlg, ID_COMMENT, WM_SETTEXT, 0, (LPARAM)DlgContour->comment);
	ColorButton( GetDlgItem( hwndDlg, ID_BORDERCOLOR ), DlgContour->border );
	ColorButton( GetDlgItem( hwndDlg, ID_FILLCOLOR ), DlgContour->fill );
	if ( !ChangeClosed )									// gray fill selected state
		SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
	else if ( DlgContour->closed )						// or match DlgContour state
			SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		 else SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	if ( !ChangeHidden )									// gray fill selected state
		SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
	else if ( DlgContour->hidden )						// or match DlgContour state
			SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		 else SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	if ( !ChangeSimplified )									// gray fill selected state
		SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
	else if ( DlgContour->simplified )						// or match DlgContour state
			SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		 else SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	if ( !ChangeSelected )									// gray fill selected state
		SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
	else if ( DlgContour->mode > 0 )						// or match DlgContour state
			SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
		 else SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	if ( abs(DlgContour->mode) == R2_NOP )
		SetFillModeObject( hwndDlg, BST_CHECKED, BST_UNCHECKED, BST_UNCHECKED, BST_UNCHECKED );
	else if ( abs(DlgContour->mode) == R2_MASKPEN )
		SetFillModeObject( hwndDlg, BST_UNCHECKED, BST_CHECKED, BST_UNCHECKED, BST_UNCHECKED );
	else if ( abs(DlgContour->mode) == R2_COPYPEN )
		SetFillModeObject( hwndDlg, BST_UNCHECKED, BST_UNCHECKED, BST_CHECKED, BST_UNCHECKED );
	else if ( abs(DlgContour->mode) == R2_MERGEPEN )
		SetFillModeObject( hwndDlg, BST_UNCHECKED, BST_UNCHECKED, BST_UNCHECKED, BST_CHECKED );
	else SetFillModeObject( hwndDlg, BST_INDETERMINATE, BST_INDETERMINATE, BST_INDETERMINATE, BST_INDETERMINATE );
}

BOOL CALLBACK ObjectAttributesDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	CHOOSECOLOR cc;
	Contour *c;
	int i;
	char txt[64];

	switch (message)					// enter with CurrContour set from first active contour so can set values after OK
		{
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg, ID_CONTOURNAME, WM_SETTEXT, 0, (LPARAM)InputDlgName);
			FillDlgObject( hwndDlg );
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case ID_BORDERCOLOR:
					ZeroMemory(&cc, sizeof(CHOOSECOLOR));			// setup color dialog structure
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					cc.rgbResult = DlgContour->border.ref();		// set current color
					for (i=0; i<16; i++) CustomColors[i] = CurrSeries->borderColors[i].ref();
					cc.lpCustColors = CustomColors;					// set custom colors
					if ( ChooseColor(&cc) )							// open color dialog
						{
						DlgContour->border = Color( cc.rgbResult );// reset default color
						ColorButton( GetDlgItem( hwndDlg, ID_BORDERCOLOR ), DlgContour->border );
						for (i=0; i<16; i++) CurrSeries->borderColors[i] = Color( CustomColors[i] );
						}
					return TRUE;

				case ID_FILLCOLOR:
					ZeroMemory(&cc, sizeof(CHOOSECOLOR));			// setup color dialog structure
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					cc.rgbResult = DlgContour->fill.ref();			// set current color
					for (i=0; i<16; i++) CustomColors[i] = CurrSeries->fillColors[i].ref();
					cc.lpCustColors = CustomColors;					// set custom colors
					if ( ChooseColor(&cc) )							// open color dialog
						{
						DlgContour->fill = Color( cc.rgbResult );	// reset default color
						ColorButton( GetDlgItem( hwndDlg, ID_FILLCOLOR ), DlgContour->fill );
						for (i=0; i<16; i++) CurrSeries->fillColors[i] = Color( CustomColors[i] );
						}
					return TRUE;

				case ID_CLOSED:		// if user clicks on INDETERMINATE state then must change it
					i = SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_GETCHECK, 0, 0);
					if ( i == BST_INDETERMINATE ) ChangeClosed = true;
					if ( i == BST_CHECKED )
						{
						SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						DlgContour->closed = false;
						}
					else {
						SendDlgItemMessage(hwndDlg, ID_CLOSED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						DlgContour->closed = true;
						}
					return TRUE;

				case ID_HIDDEN:		// if user clicks on INDETERMINATE state then must change it
					i = SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_GETCHECK, 0, 0);
					if ( i == BST_INDETERMINATE ) ChangeHidden = true;
					if ( i == BST_CHECKED )
						{
						SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						DlgContour->hidden = false;
						}
					else {
						SendDlgItemMessage(hwndDlg, ID_HIDDEN, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						DlgContour->hidden = true;
						}
					return TRUE;

				case ID_SIMPLIFIED:		// if user clicks on INDETERMINATE state then must change it
					i = SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_GETCHECK, 0, 0);
					if ( i == BST_INDETERMINATE ) ChangeSimplified = true;
					if ( i == BST_CHECKED )
						{
						SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						DlgContour->simplified = false;
						}
					else {
						SendDlgItemMessage(hwndDlg, ID_SIMPLIFIED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
						DlgContour->simplified = true;
						}
					return TRUE;

				case ID_FILLSELECTED:
					i = SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_GETCHECK, 0, 0);
					if ( i == BST_INDETERMINATE ) ChangeSelected = true;
					if ( i == BST_CHECKED )
						SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
					else SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
					return TRUE;

				case ID_NOFILL:
					DlgContour->mode = R2_NOP;
					SetFillModeObject( hwndDlg, BST_CHECKED, BST_UNCHECKED, BST_UNCHECKED, BST_UNCHECKED );
					return TRUE;

				case ID_MASKFILL:
					DlgContour->mode = R2_MASKPEN;
					SetFillModeObject( hwndDlg, BST_UNCHECKED, BST_CHECKED, BST_UNCHECKED, BST_UNCHECKED );
					return TRUE;

				case ID_COPYFILL:
					DlgContour->mode = R2_COPYPEN;
					SetFillModeObject( hwndDlg, BST_UNCHECKED, BST_UNCHECKED, BST_CHECKED, BST_UNCHECKED );
					return TRUE;

				case ID_MERGEFILL:
					DlgContour->mode = R2_MERGEPEN;
					SetFillModeObject( hwndDlg, BST_UNCHECKED, BST_UNCHECKED, BST_UNCHECKED, BST_CHECKED );
					return TRUE;

				case ID_GETDEFAULTS:			// copy default attributes into dialog
					DlgContour->border = CurrSeries->defaultBorder;
					DlgContour->fill = CurrSeries->defaultFill;
					DlgContour->mode =	CurrSeries->defaultMode;
					ChangeSelected = true;
					FillDlgObject( hwndDlg );
					return TRUE;

				case ID_GETCLIPBOARD:			// copy clipboard attributes into dialog
					if ( ClipboardTransform )
					  if ( ClipboardTransform->contours )
						{
						c = ClipboardTransform->contours->first;
						DlgContour->border = c->border;
						DlgContour->fill = c->fill;
						DlgContour->mode =	c->mode;
						ChangeSelected = true;
						FillDlgObject( hwndDlg );
						}
					return TRUE;
																	// user says OK, so store values from dialog in DlgContour
				case IDOK:
					SendDlgItemMessage(hwndDlg, ID_COMMENT, WM_GETTEXT, (WPARAM)MAX_COMMENT, (LPARAM)DlgContour->comment);
					RemoveIllegalChars( DlgContour->comment );
					if ( ChangeSelected )
					  if ( SendDlgItemMessage(hwndDlg, ID_FILLSELECTED, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						DlgContour->mode = -abs(DlgContour->mode);
					  else DlgContour->mode = abs(DlgContour->mode);	// change this here so can add it to mode
					EndDialog(hwndDlg, 1);
					return TRUE;

				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				}
		}
	return FALSE;
}

void ObjectAttributes( HWND list )		// modify traces of listview selections
{
	VRMLObjects *object_list;
	VRMLObject *object, *todo_object;
	Section *section;
	Transform *transform;
	Contour *contour;
	int i, sectnum, first, last;
	LV_ITEM lvi;
	HCURSOR cur;
	bool changed, msg_changed;
	char name[MAX_CONTOUR_NAME], filename[MAX_PATH];
	
	object_list = new VRMLObjects();					// create a list of objects to modify
	first = MAX_INT32;
	last = 0;											// remember total section range of objects
	msg_changed = false;

	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	while ( lvi.iItem >= 0 )
		{												// an item was selected, retrieve object name
		lvi.mask = LVIF_TEXT; 
		lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
	
		object = CurrObjects->Match( name );			// find object info in list
		if ( object )
			{
			todo_object = new VRMLObject();
			strcpy(todo_object->name,object->name);			// copy info to object to be modified
			todo_object->firstSection = object->firstSection;
			todo_object->lastSection = object->lastSection;
			object_list->Add( todo_object );
			if ( first > todo_object->firstSection ) first = todo_object->firstSection;
			if ( last < todo_object->lastSection ) last = todo_object->lastSection;
			}
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		}

	i = object_list->Number();
	if ( i > 0 )					// have at least one object to modify
		{
		DlgContour = new Contour();					// use it here as global value holder
		if ( i > 1 )
			{
			strcpy(DlgContour->name,"*");				// so use wildcard to match all names
			sprintf(InputDlgName,"Changing %d objects to...",i);
			}
		else {
			strcpy(DlgContour->name,todo_object->name);// just use object name
			sprintf(InputDlgName,"Changing object \"%s\" to...",todo_object->name);
			}
		strcpy(DlgContour->comment,"*");				// use wildcard so don't change comments
		DlgContour->border.negate();					// dont change border color
		DlgContour->fill.negate();						// dont change fill color
		ChangeHidden = false;							// dont change closure state
		ChangeClosed = false;							// dont change closure state
		ChangeSelected = false;							// dont change fill selected
		ChangeSimplified = false;
		DlgContour->mode =  R2_BLACK;					// dont change fill mode
		
		if ( DialogBox( appInstance, "ObjectAttributesDlg", list, (DLGPROC)ObjectAttributesDlgProc ) == IDOK )
			{
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );// show hourglass, file access could be slow

			sectnum = first-1;
			section = GetNextSectionBetween( sectnum, last );	// fetch section
			while ( section )
				{
				changed = false;								// initially, no changes to section
				object = object_list->first;
				while ( object )								// search object list for objects to do
					{
					if ( (sectnum <= object->lastSection) && (sectnum >= object->firstSection) )
						{
						if ( section->transforms )				// object could be in section, search contours
							{
							transform = section->transforms->first;
							while ( transform )								// check each transform for contours
							  {
							  if ( transform->contours )
								{											// for all contours, look for object contours
								contour = transform->contours->first;
								while ( contour )
									{
									if ( strcmp(contour->name,object->name) == 0 )		// FOUND! change it!
										{
										changed = true;					
										if ( DlgContour->comment[0] == '*' )			// concatenate if wild card was used
											strncat(contour->comment,DlgContour->comment+1,MAX_COMMENT-strlen(contour->comment));
										else strcpy(contour->comment,DlgContour->comment);	// otherwise just copy comment
										if ( ChangeClosed )
											contour->closed = DlgContour->closed;		// change open/close state
										if ( ChangeHidden )
											contour->hidden = DlgContour->hidden;		// change hidden state
										if ( ChangeSimplified )
											contour->simplified = DlgContour->simplified;// change simplified state
										if ( !DlgContour->border.invalid() ) 
											contour->border = DlgContour->border;		// set border color
										if ( !DlgContour->fill.invalid() )
											contour->fill = DlgContour->fill;			// set fill color
										if ( abs(DlgContour->mode) != R2_BLACK )
											contour->mode = abs(DlgContour->mode);		// set fill mode
										if ( ChangeSelected )
											if ( DlgContour->mode < 0 )					// set fill selected to match DlgContour
												contour->mode = -abs(contour->mode);
											else contour->mode = abs(contour->mode);
										}
									contour = contour->next;				// do next contour in section
									}
								}
							  transform = transform->next;
							  }
							}
						} // end if sectnum

					object = object->next;	
					}						// end while object

				  PutSection( section, changed, false, changed );			// save or delete section and redraw if current
				  section = GetNextSectionBetween( sectnum, last );
				  }

			SetCursor( cur );									// restore cursor 
			if ( CurrSection ) CurrSection->UnselectHidden();	// remove any just hidden traces	
			if ( PrevSection ) PrevSection->UnselectHidden();	// from activ ones in sections
			if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );
			msg_changed = true;
			}
		delete DlgContour;
		}										

	delete object_list;							// free memory and tell user things were changes
	if ( msg_changed ) MessageBox(list,"Attributes have been changed.","Object Attributes",MB_OK);
}

BOOL CALLBACK ObjectRenameDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	char dlgItemTxt[64];

	switch (message)					// enter with CurrContour set from first active contour so can set values after OK
		{
		case WM_INITDIALOG:
			SetWindowText( hwndDlg, InputDlgName );					// fill in the values that will result from user OK
			SendDlgItemMessage(hwndDlg, ID_CONTOURNAME, WM_SETTEXT, 0, (LPARAM)DlgContour->name);
			sprintf(dlgItemTxt,"%d",FirstSection);
			SetDlgItemText( hwndDlg, ID_FIRSTSECTION, dlgItemTxt );
			sprintf(dlgItemTxt,"%d",LastSection);
   			SetDlgItemText( hwndDlg, ID_LASTSECTION, dlgItemTxt );
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{													// user says OK, so store values from dialog in DlgContour
				case IDOK:
					SendDlgItemMessage(hwndDlg, ID_CONTOURNAME, WM_GETTEXT, (WPARAM)MAX_CONTOUR_NAME, (LPARAM)DlgContour->name);
					RemoveIllegalChars( DlgContour->name );
					GetDlgItemText( hwndDlg, ID_FIRSTSECTION, dlgItemTxt, sizeof(dlgItemTxt) );
					FirstSection = atoi(dlgItemTxt);
   					GetDlgItemText( hwndDlg, ID_LASTSECTION, dlgItemTxt, sizeof(dlgItemTxt) );
					LastSection = atoi(dlgItemTxt);
					EndDialog(hwndDlg, 1);
					return TRUE;

				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				}
		}
	return FALSE;
}

void ObjectRename( HWND list )		// modify traces of listview selections over a selected range of sections
{
	VRMLObjects *object_list;
	VRMLObject *object, *todo_object, *sceneObject;
	Section *section;
	Transform *transform;
	Contour *contour;
	int i, sectnum, first, last;
	LV_ITEM lvi;
	HCURSOR cur;
	bool changed, msg_changed, scene_changed;
	char name[MAX_CONTOUR_NAME];
	
	object_list = new VRMLObjects();					// create a list of objects to modify
	first = MAX_INT32;
	last = 0;											// remember total section range of objects
	msg_changed = false;
	scene_changed = false;

	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	while ( lvi.iItem >= 0 )
		{												// an item was selected, retrieve object name
		lvi.mask = LVIF_TEXT; 
		lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
	
		object = CurrObjects->Match( name );			// find object info in list
		if ( object )
			{
			todo_object = new VRMLObject();
			strcpy(todo_object->name,object->name);			// copy info to object to be modified
			todo_object->firstSection = object->firstSection;
			todo_object->lastSection = object->lastSection;
			object_list->Add( todo_object );
			if ( first > todo_object->firstSection ) first = todo_object->firstSection;
			if ( last < todo_object->lastSection ) last = todo_object->lastSection;
			}
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		}

	i = object_list->Number();
	if ( i > 0 )									// have at least one object to modify
		{
		DlgContour = new Contour();						// use it here as global value holder
		if ( i > 1 )
			{
			strcpy(DlgContour->name,"*");				// so use wildcard to match all names
			sprintf(InputDlgName,"Renaming %d objects to...",i);
			}
		else {
			strcpy(DlgContour->name,todo_object->name);// just use object name
			sprintf(InputDlgName,"Renaming 1 object to...");
			}
		FirstSection = first;							// set section range to change whole of all objects
		LastSection = last;
		if ( DialogBox( appInstance, "ObjectRenameDlg", list, (DLGPROC)ObjectRenameDlgProc ) == IDOK )
			{
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );// show hourglass, file access could be slow

			sectnum = FirstSection-1;
			section = GetNextSectionBetween( sectnum, LastSection );	// fetch section
			while ( section )
				{
				changed = false;								// initially, no changes to section
				object = object_list->first;
				while ( object )								// search object list for objects to do
					{
					if ( (sectnum <= object->lastSection) && (sectnum >= object->firstSection) )
						{
						if ( section->transforms )				// object could be in section, search contours
							{
							transform = section->transforms->first;
							while ( transform )								// check each transform for contours
							  {
							  if ( transform->contours )
								{											// for all contours, look for object contours
								contour = transform->contours->first;
								while ( contour )
									{
									if ( strcmp(contour->name,object->name) == 0 )		// FOUND! change it!
										{
										changed = true;					
										section->SetDefaultName(name,DlgContour->name,false);
										if ( name[0] == '*' )							// concatenate if wild card was used
											strncat(contour->name,name+1,MAX_CONTOUR_NAME-strlen(contour->name));
										else strcpy(contour->name,name);				// otherwise just copy contour name
										}
									contour = contour->next;				// do next contour in section
									}
								}
							  transform = transform->next;				// do next transform
							  }
							}
						} // end if sectnum

					object = object->next;	
					}						// end while object

				  PutSection( section, changed, false, changed );			// save or delete section and redraw if current
				  section = GetNextSectionBetween( sectnum, LastSection );	// keep going to limit of section range
				  }

			SetCursor( cur );									// restore cursor 
			if ( CurrSection ) CurrSection->UnselectHidden();	// remove any just hidden traces	
			if ( PrevSection ) PrevSection->UnselectHidden();	// from activ ones in sections
			if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );

			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			while ( lvi.iItem >= 0 )
				{												// deleted selected items from list
				lvi.mask = LVIF_TEXT; 
				lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
				object = object_list->first;
				while ( object )								// search object list for object name to be sure
					{
					if ( strcmp(name,object->name) == 0 )		// found in listview AND in object_list
						{
						SendMessage(list, LVM_DELETEITEM, lvi.iItem, 0);// delete it from list!				
						lvi.iItem--;							// backup so next item will be considered
						if ( CurrScene )
						  if ( CurrScene->objects )
							{									// check if object in 3D scene
							sceneObject = CurrScene->objects->Match( name );
							if ( sceneObject )								// found it so...
								{
								CurrScene->objects->Extract( sceneObject );	// remove and delete it
								delete sceneObject;				// otherwise won't be able to later
								scene_changed = true;
								}
							}
						break;		// break out of while( object ) loop 
						}
					
					object = object->next;						// after searching object list, go to next list item					
					}
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				}
			msg_changed = true;
			}
		delete DlgContour;
		}

	delete object_list;										// free memory
	if ( scene_changed && IsWindow( openGLWindow ) )		// if scene changed redraw it
		{
		CurrScene->Compile();								// recompile scene to OpenGL
		InvalidateRect(openGLWindow,NULL,FALSE);
		}													// tell user things changed
	if ( msg_changed ) MessageBox(list,"Objects have been renamed.\n\nRefresh list to see name changes.","Rename Objects",MB_OK);
}

BOOL CALLBACK SceneDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	CHOOSECOLOR cc;
	Contour *c;
	int i;
	char txt[MAX_DIGITS];

	switch (message)					// enter with CurrContour set from first active contour so can set values after OK
		{
		case WM_INITDIALOG:
			SetWindowText( hwndDlg, InputDlgName );					// fill in the values that will result from user OK
			ColorButton( GetDlgItem( hwndDlg, ID_DIFFUSECOLOR ), DlgObject->diffuseColor );
			ColorButton( GetDlgItem( hwndDlg, ID_EMISSIVECOLOR ), DlgObject->emissiveColor );
			ColorButton( GetDlgItem( hwndDlg, ID_SPECULARCOLOR ), DlgObject->specularColor );
			sprintf(txt,"%g",DlgObject->ambientIntensity);
			SetDlgItemText( hwndDlg, ID_AMBIENTINTENSITY, txt );
			sprintf(txt,"%g",DlgObject->transparency);
			SetDlgItemText( hwndDlg, ID_TRANSPARENCY, txt );
			sprintf(txt,"%g",DlgObject->shininess);
			SetDlgItemText( hwndDlg, ID_SHININESS, txt );
			if ( DlgObject->frontFill ) SendDlgItemMessage(hwndDlg, ID_FRONTFILL, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_FRONTFILL, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			if ( DlgObject->backFill ) SendDlgItemMessage(hwndDlg, ID_BACKFILL, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else SendDlgItemMessage(hwndDlg, ID_BACKFILL, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case ID_DIFFUSECOLOR:
					ZeroMemory(&cc, sizeof(CHOOSECOLOR));			// setup color dialog structure
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					cc.rgbResult = DlgObject->diffuseColor.ref();		// set current color
					cc.lpCustColors = CustomColors;					// set custom colors
					if ( ChooseColor(&cc) )							// open color dialog
						{
						DlgObject->diffuseColor = Color( cc.rgbResult );// reset default color
						ColorButton( GetDlgItem( hwndDlg, ID_DIFFUSECOLOR ), DlgObject->diffuseColor );
						}
					return TRUE;

				case ID_EMISSIVECOLOR:
					ZeroMemory(&cc, sizeof(CHOOSECOLOR));			// setup color dialog structure
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					cc.rgbResult = DlgObject->emissiveColor.ref();		// set current color
					cc.lpCustColors = CustomColors;					// set custom colors
					if ( ChooseColor(&cc) )							// open color dialog
						{
						DlgObject->emissiveColor = Color( cc.rgbResult );// reset default color
						ColorButton( GetDlgItem( hwndDlg, ID_EMISSIVECOLOR ), DlgObject->emissiveColor );
						}
					return TRUE;

				case ID_SPECULARCOLOR:
					ZeroMemory(&cc, sizeof(CHOOSECOLOR));			// setup color dialog structure
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					cc.rgbResult = DlgObject->specularColor.ref();		// set current color
					cc.lpCustColors = CustomColors;					// set custom colors
					if ( ChooseColor(&cc) )							// open color dialog
						{
						DlgObject->specularColor = Color( cc.rgbResult );// reset default color
						ColorButton( GetDlgItem( hwndDlg, ID_SPECULARCOLOR ), DlgObject->specularColor );
						}
					return TRUE;
												// user says OK, so store values from dialog in DlgContour
				case IDOK:
					SendDlgItemMessage(hwndDlg, ID_AMBIENTINTENSITY, WM_GETTEXT, MAX_DIGITS, (LPARAM)txt);
					DlgObject->ambientIntensity = atof(txt);
					if ( DlgObject->ambientIntensity < 0.0 ) DlgObject->ambientIntensity = 0.0;
					if ( DlgObject->ambientIntensity > 1.0 ) DlgObject->ambientIntensity = 1.0;
					SendDlgItemMessage(hwndDlg, ID_TRANSPARENCY, WM_GETTEXT, MAX_DIGITS, (LPARAM)txt);
					DlgObject->transparency = atof(txt);
					if ( DlgObject->transparency < 0.0 ) DlgObject->transparency = 0.0;
					if ( DlgObject->transparency > 1.0 ) DlgObject->transparency = 1.0;
					SendDlgItemMessage(hwndDlg, ID_SHININESS, WM_GETTEXT, MAX_DIGITS, (LPARAM)txt);
					DlgObject->shininess = atof(txt);
					if ( DlgObject->shininess < 0.0 ) DlgObject->shininess = 0.0;
					if ( DlgObject->shininess > 1.0 ) DlgObject->shininess = 1.0;
					if ( SendDlgItemMessage(hwndDlg, ID_FRONTFILL, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						DlgObject->frontFill = true;
					else DlgObject->frontFill = false;
					if ( SendDlgItemMessage(hwndDlg, ID_BACKFILL, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
						DlgObject->backFill = true;
					else DlgObject->backFill = false;
					EndDialog(hwndDlg, 1);
					return TRUE;

				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					return TRUE;
				}
		}
	return FALSE;
}

void SceneAttributes( HWND list )		// add listview selections to CurrScene and display them
{
	VRMLObjects *object_list;
	VRMLObject *object, *todo_object;
	int n;
	char name[MAX_CONTOUR_NAME];
	LV_ITEM lvi;
	HCURSOR cur;
	
	object_list = new VRMLObjects();					// create a list of objects to modify
	DlgObject = new VRMLObject();

	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	while ( lvi.iItem >= 0 )
		{												// an item was selected, retrieve object name
		lvi.mask = LVIF_TEXT; 
		lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
		if ( CurrScene )
		  if ( CurrScene->objects )
			{
			object = CurrScene->objects->Match( name );		// only modify objects in scene
			if ( object )
				{
				todo_object = new VRMLObject();
				strcpy(todo_object->name,object->name);			// copy name of object to be modified
				DlgObject->diffuseColor = object->diffuseColor;
				DlgObject->emissiveColor = object->emissiveColor;
				DlgObject->specularColor = object->specularColor;
				DlgObject->ambientIntensity = object->ambientIntensity;
				DlgObject->transparency = object->transparency;
				DlgObject->shininess = object->shininess;
				DlgObject->frontFill = object->frontFill;
				DlgObject->backFill = object->backFill;
				object_list->Add( todo_object );
				}
			}
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		}

	n = object_list->Number();
	if ( n > 0 )					// have at least one object to modify
		{
		sprintf(InputDlgName,"Changing %d scene objects to...",n);	
		if ( DialogBox( appInstance, "SceneDlg", list, (DLGPROC)SceneDlgProc ) == IDOK )
			{
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );// show hourglass, file access could be slow

			object = object_list->first;
			while ( object )
				{											// find this object name in scene objects
				todo_object = CurrScene->objects->Match( object->name );
				todo_object->diffuseColor = DlgObject->diffuseColor;
				todo_object->emissiveColor = DlgObject->emissiveColor;
				todo_object->specularColor = DlgObject->specularColor;
				todo_object->ambientIntensity = DlgObject->ambientIntensity;
				todo_object->transparency = DlgObject->transparency;
				todo_object->shininess = DlgObject->shininess;	
				todo_object->frontFill = DlgObject->frontFill;
				todo_object->backFill = DlgObject->backFill;							
				object = object->next;
				}

			if ( IsWindow( openGLWindow ) )
				{
				CurrScene->Compile();								// recompile scene to OpenGL
				InvalidateRect(openGLWindow,NULL,FALSE);
				}

			SetCursor( cur );			
			}									// restore cursor 
		} // end if

	delete object_list;							// free memory
	delete DlgObject;
}

void SimplifyObjects( HWND list )					// simplify all traces of selected objects
{
	VRMLObjects *object_list;
	VRMLObject *object, *sobject;
	Section *section;
	Transform *transform;
	Contour *contour;
	LV_ITEM lvi;
	HCURSOR cur;
	int num_selected, first_sect, last_sect, sectnum;
	bool changed, simplified;
	char name[MAX_CONTOUR_NAME], warntxt[100];

	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	if ( num_selected )
		{
		sprintf(warntxt,"All unsimplified traces of %d objects will be\npermanently altered! Continue?",num_selected);
		if ( MessageBox(list,warntxt,"WARNING!",MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDYES )
			{
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow
			first_sect = MAX_INT32; last_sect = -1;
			object_list = new VRMLObjects();					// create a list of objects to delete
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			while ( lvi.iItem >= 0 )
				{												// an item was selected, retrieve object name
				lvi.mask = LVIF_TEXT; 
				lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
			
				object = CurrObjects->Match( name );			// find object in list of Current Objects
				sobject = new VRMLObject();						// copy name and section info
				strcpy(sobject->name,object->name);
				sobject->firstSection = object->firstSection;
				sobject->lastSection = object->lastSection;
				object_list->Add( sobject );					// add copy to simplification list
																// update the range of sections to process
				if ( first_sect > sobject->firstSection ) first_sect = sobject->firstSection;
				if ( last_sect < sobject->lastSection ) last_sect = sobject->lastSection;
			
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				} // end While

			simplified = false;									// now go thru the sections deleting traces
			sectnum=first_sect-1;
			section = GetNextSectionBetween( sectnum, last_sect );	// fetch section
			while ( section )
				{
				changed = false;									// start with no change to section
				if ( section->transforms )
					{
					transform = section->transforms->first;
					while ( transform )								// check each transform for contours
						{
						if ( transform->contours )
							{											// for all contours, look for object contours
							object = object_list->first;
							while ( object )							// simplify each object that appears in contours
								{
								contour = transform->contours->first;
								while ( contour )
									{
									if ( (strcmp(contour->name,object->name) == 0)	// FOUND!
										&& !contour->simplified )
										{								// reduce number of contour points
										contour->Simplify( SimplifyResolution, contour->closed  );
										changed = true;					// mark section as changed
										simplified = true;				// mrk object as simplified
										}
									contour = contour->next;			// do next contour in section
									}
								object = object->next;					// do next object
								} // end While object
							}
						transform = transform->next;				// do next transform in section
						} // end While transform
					}												// save if needed and redraw if needed
				PutSection( section, changed, false, changed );
				section = GetNextSectionBetween( sectnum, last_sect );
				}	// end While section							

			if ( simplified )									// show trace window updates
				{ if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 ); }
			else ErrMsgOK( ERRMSG_NOTSIMPLIFIED, "" );			// tell user why nothing happended

			delete object_list;									// free memory
			SetCursor( cur );									// restore cursor
			}	// end if IDYES

		}  // end if ( num_selected )
}


void DeleteObjects( HWND list )					// delete all traces of selected objects
{
	VRMLObjects *object_list;
	VRMLObject *object;
	Section *section;
	Transform *transform;
	Contour *contour, *ncontour;
	LV_ITEM lvi;
	HCURSOR cur;
	int num_selected, first_sect, last_sect, sectnum;
	bool changed, sceneChanged;
	char name[MAX_CONTOUR_NAME], warntxt[100];

	sceneChanged = false;
	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	if ( num_selected )
		{
		sprintf(warntxt,"All traces of %d objects will be\npermanently deleted! Continue?",num_selected);
		if ( MessageBox(list,warntxt,"WARNING!",MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDYES )
			{
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow
			first_sect = MAX_INT32; last_sect = -1;
			object_list = new VRMLObjects();					// create a list of objects to delete
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			while ( lvi.iItem >= 0 )
				{												// an item was selected, retrieve object name
				lvi.mask = LVIF_TEXT; 
				lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
			
				object = CurrObjects->Match( name );			// find object in list of Current Objects
				if ( object )									// if found...
					{
					CurrObjects->Extract( object );					// remove it from list
					object_list->Add( object );						// add it to deletion list
																	// update the range of sections to process
					if ( first_sect > object->firstSection ) first_sect = object->firstSection;
					if ( last_sect < object->lastSection ) last_sect = object->lastSection;
					if ( CurrScene )								// delete object from scene also...
					  if ( CurrScene->objects )
						{
						object = CurrScene->objects->Match( name );	// find object in scene
						if ( object )
							{
							sceneChanged = true;					// make scene as changed so can update later
							CurrScene->objects->Extract( object );	//
							delete object;
							}										// proceed to next selected list object
						}
						
					SendMessage(list, LVM_DELETEITEM, lvi.iItem, 0);// delete also from listview
					lvi.iItem--;									// backup so next item will be considered
					}
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				} // end While
																// now go thru the sections deleting traces
			sectnum=first_sect-1;
			section = GetNextSectionBetween( sectnum, last_sect );	// fetch section
			while ( section )
			  {
			  changed = false;									// start with no change to section
			  if ( section->transforms )
				{
				transform = section->transforms->first;
				while ( transform )								// check each transform for contours
				  {
				  if ( transform->contours )
					{											// for all contours, look for object contours
					object = object_list->first;
					while ( object )							// delete each object that appears in contours
						{
						contour = transform->contours->first;
						while ( contour )
							if ( strcmp(contour->name,object->name) == 0 )	// FOUND! delete it!
								{
								changed = true;							// mark section as changed	
								ncontour = contour->next;				// remember what comes next
								transform->contours->Extract( contour );// remove contour from list
								delete contour;							// delete contour
								contour = ncontour;						// continue with next list item
								}
							else contour = contour->next;				// do next contour in section
						object = object->next;					// do next object
						} // end While
					}
				  transform = transform->next;					// do next transform in section
				  } // end While
				}												// save if needed and redraw if needed
			  PutSection( section, changed, false, changed );
			  section = GetNextSectionBetween( sectnum, last_sect );
			  }	// end While ( section )							
				
			if ( IsWindow( openGLWindow ) && CurrScene && sceneChanged )
				{
				CurrScene->Compile();
				InvalidateRect(openGLWindow,NULL,FALSE);
				}
			if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );

			delete object_list;									// free memory
			SetCursor( cur );									// restore cursor
			}	// end if IDYES

		}  // end if ( num_selected )
}

void AddObjectToList( HWND list, VRMLObject *object )	// put an object in list view
{
	LV_ITEM lvi;
	int i;
	char txt[64];
	
	lvi.mask = LVIF_TEXT | LVIF_IMAGE; lvi.state = 0; lvi.stateMask = 0;
	lvi.iItem = 0; lvi.iSubItem = 0; lvi.pszText = object->name; lvi.iImage = 0;
	if ( CurrScene )
		if ( CurrScene->objects )				// use alternate icon if object is in scene
			if ( CurrScene->objects->Match( object->name ) ) lvi.iImage = 1;
	lvi.iItem = SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)(&lvi));
	i = 1;
	if ( CurrSeries->listObjectRange )			// add section range 
		{
		sprintf(txt,"%d", object->firstSection);
		lvi.iSubItem = i++; lvi.pszText = txt;
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
		sprintf(txt,"%d", object->lastSection);
		lvi.iSubItem = i++; lvi.pszText = txt;
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
		}
	if ( CurrSeries->listObjectCount)			// add contour count
		{
		sprintf(txt,"%d", object->contour_count);
		lvi.iSubItem = i++; lvi.pszText = txt;
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
		}
	if ( CurrSeries->listObjectSurfarea )		// add surface area
		{
		sprintf(txt,"%1.*g", Precision, object->surface_area);
		lvi.iSubItem = i++; lvi.pszText = txt;
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
		}
	if ( CurrSeries->listObjectFlatarea )		// add flat area
		{
		sprintf(txt,"%1.*g", Precision, object->flat_area);
		lvi.iSubItem = i++; lvi.pszText = txt;
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
		}
	if ( CurrSeries->listObjectVolume )			// add volume
		{
		sprintf(txt,"%1.*g", Precision, object->volume);
		lvi.iSubItem = i; lvi.pszText = txt;
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
		}
}

void CopyObjects( HWND list )					// create copy of all traces of selected objects
{
	VRMLObjects *object_list;
	VRMLObject *object, *cobject;
	Section *section;
	Transform *transform;
	Contour *contour, *ncontour;
	LV_ITEM lvi;
	HCURSOR cur;
	int num_selected, first_sect, last_sect, sectnum;
	bool changed;
	char name[MAX_CONTOUR_NAME];

	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	if ( num_selected )
		{
		cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow
		first_sect = MAX_INT32; last_sect = -1;
		object_list = new VRMLObjects();					// create a list of objects to copy
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		while ( lvi.iItem >= 0 )
			{												// an item was selected, retrieve object name
			lvi.mask = LVIF_TEXT; 
			lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
			SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
		
			object = CurrObjects->Match( name );			// find object in list of Current Objects
			if ( object )
				{
				cobject = new VRMLObject( *object );		// make a copy of object
				object_list->Add( cobject );				// add it to the copy list
				if ( first_sect > object->firstSection ) first_sect = object->firstSection;
				if ( last_sect < object->lastSection ) last_sect = object->lastSection;
				}
			
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			} // end While
															// now go thru the sections copying traces
		sectnum=first_sect-1;
		section = GetNextSectionBetween( sectnum, last_sect );	// fetch section
		while ( section )
		  {
		  changed = false;									// start with no change to section
		  if ( section->transforms )
			{
			transform = section->transforms->first;
			while ( transform )								// check each transform for contours
			  {
			  if ( transform->contours )
				{											// for all contours, look for object contours
				object = object_list->first;
				while ( object )							// copy each object that appears in contours
					{
					strcpy(name,"Copy of ");				// limit name to MAX_CONTOUR_NAME
					strncat(name,object->name,MAX_CONTOUR_NAME-strlen(name));
					contour = transform->contours->first;
					while ( contour )
						{
						if ( strcmp(contour->name,object->name) == 0 )	// FOUND! copy it!
							{
							changed = true;							// mark section as changed	
							ncontour = new Contour( *contour );		// create copy of contour
							strcpy(ncontour->name,name);			// add "Copy of" to name
							transform->contours->Add( ncontour );	// add copy contour from list
							}
						contour = contour->next;					// do next contour in section
						}
					object = object->next;					// do next object
					} // end While
				}
			  transform = transform->next;					// do next transform in section
			  } // end While
			}												// save if needed and redraw if needed
		  PutSection( section, changed, false, changed );
		  section = GetNextSectionBetween( sectnum, last_sect );
		  }	// end While ( section )							

		object = object_list->first;						// update object list with copied objects
		while ( object )
			{
			cobject = object->next;							// remember next object in list
			object_list->Extract( object );					// remove it from list
			strcpy(name,"Copy of ");						// limit name to MAX_CONTOUR_NAME
			strncat(name,object->name,MAX_CONTOUR_NAME-strlen(name));
			strcpy(object->name,name);						// update name of copied object
			AddObjectToList( list, object );				// insert into existing list view
			CurrObjects->Add( object );						// add also to CurrObjects list	
			object = cobject;								// do remainder of list
			}

		if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );

		delete object_list;									// free memory
		SetCursor( cur );									// restore cursor
		}  // end if ( num_selected )
}

void SmoothObjects( HWND list )					// create smoothed copy of all traces of selected objects
{
	VRMLObjects *object_list;
	VRMLObject *object, *cobject;
	Section *section;
	Transform *transform;
	Contour *contour, *ncontour;
	LV_ITEM lvi;
	HCURSOR cur;
	int num_selected, first_sect, last_sect, sectnum;
	bool changed;
	char name[MAX_CONTOUR_NAME], txt[24];

	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	if ( num_selected )
		{
		cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow
		first_sect = MAX_INT32; last_sect = -1;
		object_list = new VRMLObjects();					// create a list of objects to copy
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		while ( lvi.iItem >= 0 )
			{												// an item was selected, retrieve object name
			lvi.mask = LVIF_TEXT; 
			lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
			SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
		
			object = CurrObjects->Match( name );			// find object in list of Current Objects
			if ( object )
				{
				cobject = new VRMLObject( *object );		// make a copy of object
				object_list->Add( cobject );				// add it to the copy list
				if ( first_sect > object->firstSection ) first_sect = object->firstSection;
				if ( last_sect < object->lastSection ) last_sect = object->lastSection;
				}
			
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			} // end While
															// now go thru the sections copying traces
		sectnum=first_sect-1;
		section = GetNextSectionBetween( sectnum, last_sect );	// fetch section
		while ( section )
		  {
		  changed = false;									// start with no change to section
		  if ( section->transforms )
			{
			transform = section->transforms->first;
			while ( transform )								// check each transform for contours
			  {
			  if ( transform->contours )
				{											// for all contours, look for object contours
				object = object_list->first;
				while ( object )							// smooth each object that appears in contours
					{
					strcpy(name,object->name);						// add suffix to name
					sprintf(txt," smooth%d",CurrSeries->smoothingLength);
					strncat(name,txt,MAX_CONTOUR_NAME-strlen(name));
					contour = transform->contours->first;
					while ( contour )
						{
						if ( strcmp(contour->name,object->name) == 0 )	// FOUND! copy it!
							{
							changed = true;							// mark section as changed	
							ncontour = new Contour( *contour );		// create copy of contour
							strcpy(ncontour->name,name);			// update to "smoothed" name
							ncontour->Smooth( CurrSeries->smoothingLength ); // smooth it
							ncontour->simplified = false;			// smoothing can induce loops
							transform->contours->Add( ncontour );	// add copy contour from list
							}
						contour = contour->next;					// do next contour in section
						}
					object = object->next;					// do next object
					} // end While
				}
			  transform = transform->next;					// do next transform in section
			  } // end While
			}												// save if needed and redraw if needed
		  PutSection( section, changed, false, changed );
		  section = GetNextSectionBetween( sectnum, last_sect );
		  }	// end While ( section )							

		object = object_list->first;						// update object list with copied objects
		while ( object )
			{
			cobject = object->next;							// remember next object in list
			object_list->Extract( object );					// remove it from list
			strcpy(name,object->name);						// add suffix to name
			sprintf(txt," smooth%d",CurrSeries->smoothingLength);
			strncat(name,txt,MAX_CONTOUR_NAME-strlen(name));
			strcpy(object->name,name);						// update name of copied object
			AddObjectToList( list, object );				// insert into existing list view
			CurrObjects->Add( object );						// add also to CurrObjects list	
			object = cobject;								// do remainder of list
			}

		if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );

		delete object_list;									// free memory
		SetCursor( cur );									// restore cursor
		}  // end if ( num_selected )
}

void ZTraceObjects( HWND list )		// create z-trace through midpoints of all traces of each selected object
{
	VRMLObjects *object_list;
	Contours *zcontour_list;
	VRMLObject *object, *cobject;
	Section *section;
	Transform *transform;
	Contour *contour, *ncontour, *zcontour;
	Point min, max, allmin, allmax, *p;
	LV_ITEM lvi;
	HCURSOR cur;
	int num_selected, first_sect, last_sect, sectnum;
	char name[MAX_CONTOUR_NAME];

	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	if ( num_selected )
		{
		cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow
		first_sect = MAX_INT32; last_sect = -1;
		object_list = new VRMLObjects();					// create a list of objects to do
		zcontour_list = new Contours();						// and an associated list of z-traces
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		while ( lvi.iItem >= 0 )
			{												// an item was selected, retrieve object name
			lvi.mask = LVIF_TEXT; 
			lvi.iSubItem = 0; lvi.pszText = name; lvi.cchTextMax = MAX_CONTOUR_NAME;
			SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
		
			object = CurrObjects->Match( name );			// find object in list of Current Objects
			if ( object )
				{
				cobject = new VRMLObject( *object );		// make a copy of object
				object_list->Add( cobject );				// add it to the copy list
				if ( first_sect > object->firstSection ) first_sect = object->firstSection;
				if ( last_sect < object->lastSection ) last_sect = object->lastSection;
				zcontour = new Contour();
				zcontour->closed = false;
				zcontour->simplified = false;				// add corresponding ztrace
				zcontour->border = object->emissiveColor;
				strcpy(zcontour->name,name);
				strncat(zcontour->name,"_Z",MAX_CONTOUR_NAME-strlen(zcontour->name));
				zcontour->points = new Points();
				zcontour_list->Add( zcontour );
				}
			
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			} // end While
															// now go thru the sections forming z-traces
		sectnum = first_sect-1;
		section = GetNextSectionBetween( sectnum, last_sect );	// fetch section
		while ( section )
		  {
		  if ( section->transforms )						// if it has trace data...
			{
			object = object_list->first;
			zcontour = zcontour_list->first;
			while ( object )								// find midpt for each object
				{
				allmin.x = MAX_FLOAT;						// allmin, allmax will hold final extents
				allmin.y = MAX_FLOAT;
				allmax.x = -MAX_FLOAT;
				allmax.y = -MAX_FLOAT;
				transform = section->transforms->first;
				while ( transform )							// check each transform for contours
				  {
				  if ( transform->contours )
					{										// for all contours, look for object contours
					contour = transform->contours->first;
					while ( contour )
						{
						if ( strcmp(contour->name,object->name) == 0 )	// FOUND! copy it!
							{
							ncontour = new Contour( *contour );		// transform contour into section coord.
							ncontour->InvNform( transform->nform );	// to correctly calculate centroid + length
							ncontour->Extent( &min, &max );			//  find extremes...
							delete ncontour;						// delete transformed contour
							if ( min.x < allmin.x ) allmin.x = min.x;	// accummulate min, max
							if ( min.y < allmin.y ) allmin.y = min.y;
							if ( max.x > allmax.x ) allmax.x = max.x;
							if ( max.y > allmax.y ) allmax.y = max.y;
							}
						contour = contour->next;					// do next contour in section
						}
					}
				  transform = transform->next;					// do next transform in section

				  }											// end While (transform )

				if ( allmin.x < MAX_FLOAT )					// found a contour for this object
					{
					p = new Point();						// add to ztrace for object
					p->x = (allmin.x+allmax.x)/2;
					p->y = (allmin.y+allmax.y)/2;
					p->z = (double)(section->index);
					zcontour->points->Add( p );
					}

				object = object->next;						// do next object and ztrace
				zcontour = zcontour->next;
				}										// end While ( object )	
			}

		  PutSection( section, false, false, false );	// save nothing, just delete section if needed
		  section = GetNextSectionBetween( sectnum, last_sect );

		  }											// end While ( section )							

		if ( !CurrSeries->zcontours ) CurrSeries->zcontours = new Contours();
		zcontour = zcontour_list->first;
		while ( zcontour )
			{
			contour = zcontour;
			zcontour = zcontour->next;
			zcontour_list->Extract( contour );		// extract it from list
			CurrSeries->zcontours->Add( contour );
			}
			
		if ( IsWindow(zTraceWindow) ) PostMessage( zTraceWindow, WM_COMMAND, CM_REFRESHLIST, 0 );

		delete object_list;									// free memory
		delete zcontour_list;
		SetCursor( cur );									// restore cursor
		}  // end if ( num_selected )
}


void StartObjectList( HWND list )				// create and fill object list
{
	VRMLObject *object;
	LV_ITEM lvi;
	LV_COLUMN column;
	int i, colwidth[MAX_OBJECTLISTCOLS];
	char txt[MAX_COMMENT];

	SendMessage(list, LVM_DELETEALLITEMS, 0, 0 );		// clear the list view

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; 	// setup columns masks
	column.fmt = LVCFMT_LEFT; column.pszText = txt;	column.cchTextMax = MAX_COMMENT;

	for (i=MAX_OBJECTLISTCOLS-1; i>0; i--)				// clear all columns
		{
		colwidth[i] = 100;								// default column width
		if ( SendMessage(list, LVM_GETCOLUMN, i, (LPARAM)(&column) ) )
			{
			colwidth[i] = column.cx;					// if exists, remember old width
			SendMessage(list, LVM_DELETECOLUMN, i, 0);	// delete column from listview
			}
		}

	i = 1;
	if ( CurrSeries->listObjectRange )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Start"); 
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"End");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listObjectCount)
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Count");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listObjectSurfarea )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Surface area");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listObjectFlatarea )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Flat area");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listObjectVolume )
		{
		column.cx = colwidth[i]; column.iSubItem = i; strcpy(txt,"Volume");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}

	lvi.mask = LVIF_TEXT | LVIF_IMAGE; lvi.state = 0; lvi.stateMask = 0;
	sprintf(txt,"Reading...");
	lvi.iSubItem = 0; lvi.pszText = txt; lvi.iImage = 0;
	lvi.iItem = SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)(&lvi));

	ObjectListThreadBegin();
}


void CompleteObjectList( HWND list )		// after thread is done, fill in list info
{
	VRMLObject *object;
	LV_ITEM lvi;
	int i;
	char txt[64];

	SendMessage( list, LVM_DELETEALLITEMS, 0, 0 );		// clear list

	if ( CurrObjects )
		{
		object = CurrObjects->first;
		while ( object )								// put each object in list view
			{
			AddObjectToList( list, object );
			object = object->next;
			}
		}
}


LRESULT APIENTRY ListObjectsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
		{										// initialize the list view section info
		case WM_CREATE:
			GetClientRect( hWnd, &r );
			w = r.right-r.left-10; h = r.bottom-r.top-10,
			objectList = CreateWindow( WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER | LVS_SORTASCENDING,
														5, 5, w, h, hWnd, NULL, appInstance, NULL);
			himl = ImageList_Create(16, 16, TRUE, 2, 1);
			ImageList_AddIcon(himl, LoadIcon(appInstance, "ObjectIcon"));
			ImageList_AddIcon(himl, LoadIcon(appInstance, "SceneIcon"));
			SendMessage(objectList, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)(himl));	// setup icons
			column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; 
			column.fmt = LVCFMT_LEFT; column.pszText = txt;							// setup columns
			strcpy(txt,"Object"); column.cx = 100;
			for ( column.iSubItem = 0; column.iSubItem < MAX_OBJECTLISTCOLS; column.iSubItem++ )
				SendMessage(objectList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			StartObjectList( objectList );										// invoke the thread for reading CurrObjects
			sprintf(txt,"Objects %s",limitObjectList);
			SetWindowText(hWnd,txt);
			hMenu = GetSystemMenu( hWnd, FALSE );						// allow Maximize on titlebar menu
			EnableMenuItem( hMenu, SC_MAXIMIZE, MF_ENABLED );
			break;

		case UM_OBJECTDONE:
			CompleteObjectList( objectList );					// fill listview with CurrObjects
			hMenu = GetMenu( hWnd );
			EnableMenuItem( hMenu, CM_REFRESHLIST, MF_BYCOMMAND | MF_ENABLED );
			DrawMenuBar( hWnd );
			break;

		case UM_OBJECTPROGRESS:									// reuse appWnd message number
			lvi.mask = LVIF_TEXT | LVIF_IMAGE; lvi.state = 0; lvi.stateMask = 0;
			sprintf(txt,"%d",lParam);							// report progress reading sections
			lvi.iSubItem = 0; lvi.pszText = txt; lvi.iImage = 0;
			lvi.iItem = SendMessage(objectList, LVM_SETITEM, 0, (LPARAM)(&lvi));
			break;

		case WM_SIZE:							// resize list view control to match dialog box
			GetClientRect( hWnd, &r );
			SetWindowPos( objectList, 0, 0, 0, r.right-r.left-10, r.bottom-r.top-10, SWP_NOMOVE | SWP_NOOWNERZORDER);
			break;
			
		case WM_DESTROY:
			if ( hObjectListThread ) AbortObject = true;
			objectWindow = NULL;				// clear global handle pointer
			hMenu = GetMenu( appWnd );
			CheckMenuItem( hMenu, CM_OBJECTLIST, MF_BYCOMMAND | MF_UNCHECKED );	// uncheck menu item
			SetFocus(appWnd);					// return to main window
			break;

		case WM_NOTIFY:							// process listview notification messages
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case LVN_KEYDOWN:
					key = (LV_KEYDOWN *)lParam;	// allow control+TAB to switch active window
					if ( (key->wVKey==VK_TAB) && (GetKeyState(VK_CONTROL) & 0x8000) ) CmNextWindow( objectWindow );
					if ( key->wVKey==VK_DELETE ) DeleteObjects( objectList );
					if ( key->wVKey !=  VK_RETURN ) break;
												// fall through to double-click	if Enter key pressed

				case NM_DBLCLK:					// user double-clicks or presses enter, display selected
					AddSelectedObjectsToScene( objectList );
					return TRUE;
				}
			break;

		case WM_SETFOCUS:
			SetFocus(objectList );				// activate list view if window activated
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{								// default pushbutton: display selected item
				case CM_OBJECTTOSCENE:
					AddSelectedObjectsToScene( objectList );
					break;

				case CM_REMOVEFROMSCENE:
					RemoveSelectedObjectsFromScene( objectList );
					break;

				case CM_SCENEATTRIBUTES:
					SceneAttributes( objectList );
					break;

				case CM_REFRESHLIST:					// regenerate the list
					hMenu = GetMenu( hWnd );			// but block repeat 'til thread done
					EnableMenuItem( hMenu, CM_REFRESHLIST, MF_BYCOMMAND | MF_GRAYED );
					StartObjectList( objectList );
					break;

				case CM_INFOLIST:						// display count of items
					DisplayListInfo( objectList );
					break;

				case CM_SAVELIST:
					sprintf(filename,"objects.csv");
					ZeroMemory(&ofn, sizeof(ofn));						// clear the OPENFILENAME fields
					ofn.lStructSize = sizeof(ofn);						// set only the necessary values
					ofn.hwndOwner = appWnd;
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "All Files\0*.*\0Comma Separated Value file\0*.csv\0\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrTitle = "Save Object List As...";
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY; // ask user if overwrite desired
					if ( GetSaveFileName(&ofn) == TRUE )				// display the Open dialog box
						{
						SaveList( objectList, filename );
						}
					break;

				case CM_HIDEOBJECTS:
					HideObjects( objectList, true );
					break;

				case CM_UNHIDEOBJECTS:
					HideObjects( objectList, false );
					break;

				case CM_OBJECTATTRIBUTES:
					ObjectAttributes( objectList );
					break;

				case CM_OBJECTRENAME:
					ObjectRename( objectList );
					break;

				case CM_SIMPLIFYOBJECTS:
					SimplifyObjects( objectList );
					break;

				case CM_DELETEOBJECTS:
					DeleteObjects( objectList );
					break;

				case CM_COPYOBJECTS:
					CopyObjects( objectList );
					break;

				case CM_SMOOTHOBJECTS:
					SmoothObjects( objectList );
					break;

				case CM_ZTRACEOBJECTS:
					ZTraceObjects( objectList );
					break;

				case CM_FINDOBJECTNAMED:
					CmFindObjectNamed( objectList );
					break;
				}
		}
	return( DefWindowProc(hWnd, message, wParam, lParam) );
}


HWND MakeObjectListWindow( void )			// display list for user selection of objects to view
{
	WNDCLASS wndclass;
	HWND sWnd;
	HMENU hMenu;
	int width;
	RECT r;
														// create the object list window...
	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)ListObjectsProc;
	wndclass.cbClsExtra  = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance = appInstance;
	wndclass.hCursor   = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon     = NULL;							// not used for WS_EX_TOOLWINDOW
	wndclass.lpszMenuName = "ObjectListMenu";
	wndclass.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wndclass.lpszClassName = "ObjectListClass";
	RegisterClass(&wndclass);

	ClientRectLessStatusBar( &r );						// position window in upper left corner of client
	ClientRectToScreen( appWnd, &r );
	width = 140;										// set width based on how many data fields present
	if ( CurrSeries->listObjectRange )
		width += 200;
	if ( CurrSeries->listObjectCount)
		width += 100;
	if ( CurrSeries->listObjectSurfarea )
		width += 100;
	if ( CurrSeries->listObjectFlatarea )
		width += 100;
	if ( CurrSeries->listObjectVolume )
		width += 100;

	sWnd = CreateWindowEx( WS_EX_TOOLWINDOW, "ObjectListClass", "Objects", WS_OVERLAPPEDWINDOW,
								r.left, r.top, width, r.bottom-r.top, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

	if ( sWnd )
		{											// if window created, display it
		ShowWindow(sWnd, SW_SHOW);			// and check corresponding menu item in main window	
		hMenu = GetMenu( appWnd );
		CheckMenuItem( hMenu, CM_OBJECTLIST, MF_BYCOMMAND | MF_CHECKED );
		}

	return( sWnd );
}

void CmObjectList( void )								// create or destroy object list window
{
	if ( IsWindow( objectWindow ) ) DestroyWindow( objectWindow );						
	else objectWindow = MakeObjectListWindow();			// create list window
}

void CmRestore3DWindow( void )						// create scene and display
{
	if ( IsWindow( openGLWindow ) ) DestroyWindow( openGLWindow );
	else {
		if ( !CurrScene ) CurrScene = new VRMLScene();	// if not already created, create the scene
		openGLWindow = MakeOpenGLWindow();
		}
}


void AddSelectedZTraceToScene( HWND list )
{
	Contour *zcontour;
	VRMLObject *sceneObject;
	LV_ITEM lvi;
	HCURSOR cur;
	bool reset = true;
	
	cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );			// show hourglass, file access could be slow

	if ( !CurrScene ) CurrScene = new VRMLScene();			// create new scene if necessary
	if ( CurrScene->objects )								// reset scene only if scene is empty
		if ( CurrScene->objects->Number() ) reset = false;

	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	while ( lvi.iItem >= 0 )
		{													// an item was selected, retrieve object name
		lvi.mask = LVIF_PARAM; 
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
		zcontour = CurrSeries->ZTraceFromId( lvi.lParam );

		if ( zcontour )
			{
			if ( CurrScene->objects )						// see if object already exists in scene!
				{
				sceneObject = CurrScene->objects->Match( zcontour->name );
				if ( sceneObject )
					{
					CurrScene->objects->Extract( sceneObject );	// remove and delete it
					delete sceneObject;
					}
				}

			sceneObject = new VRMLObject();

			sceneObject->CreateZline( zcontour );
				
			CurrScene->Add( sceneObject );					// add to scene and update scene params
//			lvi.mask = LVIF_IMAGE; lvi.iImage = 1;			// change to scene icon and get next selected object
//			SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
			}
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		}

	if ( IsWindow( openGLWindow ) )
		{
		CurrScene->Compile();								// recompile into openGL
		if ( reset ) PostMessage( openGLWindow, WM_COMMAND, CM_RESETSCENE, 0 );
		else InvalidateRect(openGLWindow,NULL,FALSE);		// update display of scene
		}
	else openGLWindow = MakeOpenGLWindow();					// or create new 3D scene window for it

	ResetSceneIcons( list );								// update icons of new scene items

	SetCursor( cur );										// restore cursor 
}

void SetZTraceColor( HWND list )				// update color of z-traces in scene and in series
{
	VRMLObject *sceneObject;
	CHOOSECOLOR cc;
	Color color;
	Contour *zcontour;
	LV_ITEM lvi;
	bool sceneChanged;

	sceneChanged = false;
	zcontour = NULL;

	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{												// an item was selected, retrieve ztrace name
		lvi.mask = LVIF_PARAM; 
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
		zcontour = CurrSeries->ZTraceFromId( lvi.lParam );
		}		
	
	if ( zcontour )
		{
		color = zcontour->border;
		ZeroMemory(&cc, sizeof(CHOOSECOLOR));			// setup color dialog structure
		cc.lStructSize = sizeof(CHOOSECOLOR);
		cc.hwndOwner = list;
		cc.Flags = CC_FULLOPEN | CC_RGBINIT;
		cc.rgbResult = color.ref();							// set current color
		cc.lpCustColors = CustomColors;					// set custom colors
		if ( ChooseColor(&cc) )							// open color dialog
			color = Color( cc.rgbResult );				// set change color

		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		while ( lvi.iItem >= 0 )
			{											// for each selected item, update color
			lvi.mask = LVIF_PARAM; 
			SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
			zcontour = CurrSeries->ZTraceFromId( lvi.lParam );

			zcontour->border = color;

			if ( CurrScene )
			  if ( CurrScene->objects )
				{
				sceneObject = CurrScene->objects->Match( zcontour->name );	// find object in scene
				if ( sceneObject )
					{
					sceneChanged = true;
					sceneObject->emissiveColor = color;
					sceneObject->diffuseColor = color;
					}
				}
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			}

		if ( IsWindow( openGLWindow ) && CurrScene && sceneChanged )
			CurrScene->Compile();								// update scene if open

		}  // end if ( zcontour )
}

void FillZList( HWND list )
{
	Contour *contour;
	Point min, max;
	LV_ITEM lvi;
	LV_COLUMN column;
	int i, colwidth[MAX_ZTRACELISTCOLS];
	char txt[MAX_COMMENT];

	SendMessage(list, LVM_DELETEALLITEMS, 0, 0 );		// clear the list view

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;	// setup columns masks 
	column.fmt = LVCFMT_LEFT; column.pszText = txt;	column.cchTextMax = MAX_COMMENT;	

	for (i=MAX_ZTRACELISTCOLS-1; i>0; i--)				// clear all columns
		{
		colwidth[i] = 100;								// default column width
		if ( SendMessage(list, LVM_GETCOLUMN, i, (LPARAM)(&column) ) )
			{
			colwidth[i] = column.cx;					// if exists, remember old width
			SendMessage(list, LVM_DELETECOLUMN, i, 0);	// delete column from listview
			}
		}

	i = 1;											// add back columns for each list subitem
	if ( CurrSeries->listZTraceNote )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Note"); 
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listZTraceRange )
		{
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"Start");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		column.cx = colwidth[i]; column.iSubItem = i++; strcpy(txt,"End");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	if ( CurrSeries->listZTraceLength )
		{
		column.cx = colwidth[i]; column.iSubItem = i; strcpy(txt,"Length");
		SendMessage(list, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
		}
	
	lvi.iItem = 0; lvi.state = 0; lvi.stateMask = 0;
	contour = CurrSeries->zcontours->first;
	while ( contour )									// put each object in list view
	  {
	  if ( MatchLimit( contour->name, limitZTraceList ) )
		{
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		lvi.iSubItem = 0; lvi.pszText = contour->name; lvi.iImage = 0;
		lvi.lParam = contour->Id;							// use lParam value to store contourId
		if ( CurrScene )
			if ( CurrScene->objects )						// use alternate icon if object is in scene
				if ( CurrScene->objects->Match( contour->name ) ) lvi.iImage = 1;
		lvi.iItem = SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)(&lvi));

		i = 1; lvi.mask = LVIF_TEXT;						// now set text of other columns
		if ( CurrSeries->listZTraceNote )
			{
			lvi.iSubItem = i++; lvi.pszText = contour->comment;
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
			}
		if ( CurrSeries->listZTraceRange )
			{
			contour->Extent( &min, &max );
			sprintf(txt,"%d", (int)min.z);
			lvi.iSubItem = i++; lvi.pszText = txt;
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
			sprintf(txt,"%d", (int)max.z);
			lvi.iSubItem = i++; lvi.pszText = txt;
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
			}
		if ( CurrSeries->listZTraceLength)
			{
			sprintf(txt,"%1.*g", Precision, CurrSeries->ZLength( contour ) );
			lvi.iSubItem = i; lvi.pszText = txt;
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
			}
		}					// end if MatchLimit
	  contour = contour->next;
	  }
}

void SetZTraceName( HWND list )				// update name of all selected z-traces
{
	Contour *zcontour;
	LV_ITEM lvi;
	int num_selected;
	char name[MAX_CONTOUR_NAME];

	zcontour = NULL;
	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{												// an item was selected, retrieve ztrace name
		lvi.mask = LVIF_PARAM; 
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
		zcontour = CurrSeries->ZTraceFromId( lvi.lParam );
		}		
	
	if ( zcontour )
		{
		if ( num_selected > 1 )
			{ 
			sprintf(InputDlgName,"Changing %d Z-Traces...");
			strcpy(InputDlgString,"*");
			}
		else {
			sprintf(InputDlgName,"Changing 1 Z-Trace...");
			strcpy(InputDlgString,zcontour->name);
			}
		strcpy(InputDlgValue,"Z-Trace Name:");
		if ( DialogBox( appInstance, "InputDlg", list, (DLGPROC)InputDlgProc ) )
			{
			RemoveIllegalChars(InputDlgString);
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			while ( lvi.iItem >= 0 )
				{											// for each selected item, update name
				lvi.mask = LVIF_PARAM; 
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
				zcontour = CurrSeries->ZTraceFromId( lvi.lParam );
				strcpy(name,InputDlgString);
				if ( FrontView )
					if ( FrontView->section )
						FrontView->section->SetDefaultName(name,InputDlgString,false);
				if ( name[0] == '*' )				// if * is still there, concatenate rest 
					strncat(zcontour->name,name+1,MAX_CONTOUR_NAME-strlen(zcontour->name));
				else strcpy(zcontour->name,name);
	
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				}
			FillZList(list);
			}
		}  // end if ( zcontour )
}

void SetZTraceComment( HWND list )				// update comment of all selected z-traces
{
	Contour *zcontour;
	LV_ITEM lvi;
	int num_selected;

	zcontour = NULL;
	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
	if ( lvi.iItem >= 0 )
		{												// an item was selected, retrieve ztrace name
		lvi.mask = LVIF_PARAM; 
		SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
		zcontour = CurrSeries->ZTraceFromId( lvi.lParam );
		}		
	
	if ( zcontour )
		{
		if ( num_selected > 1 )
			{ 
			sprintf(InputDlgName,"Changing %d Z-Traces...");
			strcpy(InputDlgString,"*");
			}
		else {
			sprintf(InputDlgName,"Changing 1 Z-Trace...");
			strcpy(InputDlgString,zcontour->comment);
			}
		strcpy(InputDlgValue,"Z-Trace Note:");
		if ( DialogBox( appInstance, "InputDlg", list, (DLGPROC)InputDlgProc ) )
			{
			RemoveIllegalChars(InputDlgString);
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			while ( lvi.iItem >= 0 )
				{											// for each selected item, update comment
				lvi.mask = LVIF_PARAM; 
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
				zcontour = CurrSeries->ZTraceFromId( lvi.lParam );
				if ( InputDlgString[0] == '*' )				// if * is still there, concatenate rest 
					strncat(zcontour->comment,InputDlgString+1,MAX_COMMENT-strlen(zcontour->comment));
				else strcpy(zcontour->comment,InputDlgString);
	
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				}
			FillZList(list);
			}
		}  // end if ( zcontour )
}

void DeleteZTraces( HWND list )
{
	VRMLObject *sceneObject;
	Contour *zcontour;
	LV_ITEM lvi;
	int num_selected;
	bool sceneChanged;
	char warntxt[100];

	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	if ( num_selected )
		{
		sprintf(warntxt,"%d z-traces will be permanently\ndeleted from the series. Continue?",num_selected);
		if ( MessageBox(list,warntxt,"WARNING!",MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDYES )
			{
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			while ( lvi.iItem >= 0 )
				{											// for each selected item...
				lvi.mask = LVIF_PARAM; 
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
				zcontour = CurrSeries->ZTraceFromId( lvi.lParam );
				if ( zcontour )								// ... extract the z-contour
					{
					CurrSeries->zcontours->Extract( zcontour );		
					if ( CurrScene )						// delete it from scene if there
						if ( CurrScene->objects )
							{
							sceneObject = CurrScene->objects->Match( zcontour->name );	// find object in scene
							if ( sceneObject )
								{
								sceneChanged = true;
								CurrScene->objects->Extract( sceneObject );
								delete sceneObject;
								}
							}
					delete zcontour;						// and delete it from memory
					} // end if ( zcontour )
	
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				}
															// now update the windows that show z-traces
			FillZList(list);
			if ( IsWindow( openGLWindow ) && CurrScene && sceneChanged )
				{
				CurrScene->Compile();							// recompile scene to OpenGL
				InvalidateRect(openGLWindow,NULL,FALSE);		// update display
				}

			}	// end if IDYES

		}  // end if ( num_selected )
}

void CopyZTraces( HWND list )			// create copies of highlighted z-traces
{
	Contour *zcontour, *ncontour;
	LV_ITEM lvi;
	int num_selected;

	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	if ( num_selected )
		{
		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		while ( lvi.iItem >= 0 )
			{											// for each selected item...
			lvi.mask = LVIF_PARAM; 
			SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
			zcontour = CurrSeries->ZTraceFromId( lvi.lParam );
			if ( zcontour )
				{
				ncontour = new Contour( *zcontour );	// copy the ztrace and name
				strcpy(ncontour->name,"Copy of ");
				strncat(ncontour->name,zcontour->name,MAX_CONTOUR_NAME-strlen(ncontour->name));
				CurrSeries->zcontours->Add( ncontour );		// add it to the series zTraces
				}
			
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			}
														// now update the z-traces list
		FillZList(list);

		}  // end if ( num_selected )
}


void SmoothZTraces( HWND list )			// create smoothed copies of selected z-traces
{
	Contour *zcontour, *ncontour;
	LV_ITEM lvi;
	int num_selected, filter_length;
	char txt[24];

	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	if ( num_selected )
		{
		sprintf(InputDlgName,"Create Smoother Z-Trace");
		sprintf(InputDlgString,"10");						// ask user for filter length
		sprintf(InputDlgValue,"Moving Average Filter Length:");
		if ( DialogBox( appInstance, "InputDlg", list, (DLGPROC)InputDlgProc ) )
			{
			filter_length = atof(InputDlgString);				// filter length must be >= 1
			if ( filter_length < 1 ) filter_length = 1;

			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			while ( lvi.iItem >= 0 )
				{											// for each selected item...
				lvi.mask = LVIF_PARAM; 
				SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
				zcontour = CurrSeries->ZTraceFromId( lvi.lParam );
				if ( zcontour )
					{
					ncontour = new Contour( *zcontour );		// copy the ztrace and extend name
					sprintf(txt," smooth%d",filter_length);
					strncat(ncontour->name,txt,MAX_CONTOUR_NAME-strlen(ncontour->name));
					ncontour->Smooth( filter_length );			// smooth it
					CurrSeries->zcontours->Add( ncontour );		// add it to the series zTraces
					}
				
				lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
				}
															// now update the windows that show z-traces
			FillZList(list);
			}	// end if DialogBox()

		}  // end if ( num_selected )
}


void GridAtZTraces( HWND list )		// place grid trace at each point along each of the selected z-traces
{
	Contours *zcontour_list;
	Section *section;
	Contour *ncontour, *zcontour;
	Point *p;
	LV_ITEM lvi;
	HCURSOR cur;
	int num_selected, first_sect, last_sect, sectnum;

	first_sect = MAX_INT32; last_sect = -1;
	num_selected = SendMessage(list, LVM_GETSELECTEDCOUNT, 0, 0 );
	if ( num_selected )
		{
		zcontour_list = new Contours();						// create list for highlighted items

		lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
		while ( lvi.iItem >= 0 )
			{												// get each selected z-trace from list
			lvi.mask = LVIF_PARAM; 
			SendMessage(list, LVM_GETITEM, 0, (LPARAM)(&lvi));
			zcontour = CurrSeries->ZTraceFromId( lvi.lParam );
			if ( zcontour )
			  if ( zcontour->points )
				{
				ncontour = new Contour( *zcontour );		// copy the ztrace 
				zcontour_list->Add( ncontour );				// and add it to the list
				p = ncontour->points->first;				// update section range
				while ( p )
					{
					if ( ((int)p->z) < first_sect ) first_sect = (int)p->z;
					if ( ((int)p->z) > last_sect ) last_sect = (int)p->z;
					p = p->next;
					}
				}
			
			lvi.iItem = SendMessage(list, LVM_GETNEXTITEM, lvi.iItem, (LPARAM)(LVNI_ALL | LVNI_SELECTED) );
			}
		}

	if ( zcontour_list->Number() )							// found some traces, so do it...
		{
		cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow
															// now go thru the sections adding grid traces
		sectnum = first_sect-1;
		section = GetNextSectionBetween( sectnum, last_sect );	// fetch section
		while ( section )
			{
			zcontour = zcontour_list->first;				// add grids at every zcontour that has this section
			while ( zcontour )
				{
				p = zcontour->points->first;				// check every pt of every z-trace
				while ( p )
					{
					if ( ((int)p->z) == sectnum )			// add a grid for each pt on this section
						{
						section->hasChanged = true;
						section->CreateGridTraces( p->x, p->y );
						}
					p = p->next;
					}
				zcontour = zcontour->next;
				}										// end While ( zcontour )	
															// save if changed, and redraw if in memory
			PutSection( section, section->hasChanged, false, true );
			section = GetNextSectionBetween( sectnum, last_sect );

			}											// end While ( section )							

		if ( IsWindow(traceWindow) ) PostMessage( traceWindow, UM_UPDATESECTION, 0, 0 );

		delete zcontour_list;							// free memory
		SetCursor( cur );								// restore cursor
		}  // end if
}



LRESULT APIENTRY ListZTracesProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
		{										// initialize the list view section info
		case WM_CREATE:
			GetClientRect( hWnd, &r );
			w = r.right-r.left-10; h = r.bottom-r.top-10,
			zTraceList = CreateWindow( WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER | LVS_SORTASCENDING,
														5, 5, w, h, hWnd, NULL, appInstance, NULL);
			himl = ImageList_Create(16, 16, TRUE, 2, 1);
			ImageList_AddIcon(himl, LoadIcon(appInstance, "ZlineIcon"));
			ImageList_AddIcon(himl, LoadIcon(appInstance, "ZSceneIcon"));
			SendMessage(zTraceList, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)(himl));	// setup icons
			column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; 
			column.fmt = LVCFMT_LEFT; column.pszText = txt;							// setup columns
			strcpy(txt,"Z-Trace"); column.cx = 100;
			for ( column.iSubItem = 0; column.iSubItem < MAX_ZTRACELISTCOLS; column.iSubItem++ )
				SendMessage(zTraceList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			FillZList( zTraceList );
			sprintf(txt,"Z-Traces %s",limitZTraceList);
			SetWindowText(hWnd,txt);
			SetFocus( zTraceList );
			hMenu = GetSystemMenu( hWnd, FALSE );						// allow Maximize on titlebar menu
			EnableMenuItem( hMenu, SC_MAXIMIZE, MF_ENABLED );
			break;

		case WM_SIZE:							// resize list view control to match dialog box
			GetClientRect( hWnd, &r );
			SetWindowPos( zTraceList, 0, 0, 0, r.right-r.left-10, r.bottom-r.top-10, SWP_NOMOVE | SWP_NOOWNERZORDER);
			break;
			
		case WM_DESTROY:
			zTraceWindow = NULL;				// clear global handle pointer
			hMenu = GetMenu( appWnd );
			CheckMenuItem( hMenu, CM_ZLIST, MF_BYCOMMAND | MF_UNCHECKED );	// uncheck menu item
			SetFocus(appWnd);					// return to main window
			break;

		case WM_NOTIFY:							// process listview notification messages
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case LVN_KEYDOWN:
					key = (LV_KEYDOWN *)lParam;	// allow control+TAB to switch active window
					if ( (key->wVKey==VK_TAB) && (GetKeyState(VK_CONTROL) & 0x8000) ) CmNextWindow( zTraceWindow );
					if ( key->wVKey==VK_DELETE ) DeleteZTraces( zTraceList );
					if ( key->wVKey !=  VK_RETURN ) break;
												// fall through to double-click	if Enter key pressed

				case NM_DBLCLK:					// user double-clicks or presses enter, display selected
					AddSelectedZTraceToScene( zTraceList );
					SetFocus(openGLWindow);		// switch to 3D Scene window
					return TRUE;
				}
			break;

		case WM_SETFOCUS:
			SetFocus(zTraceList );				// activate list view if window activated
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{										// default pushbutton: display selected item
				case CM_ZTRACETOSCENE:
					AddSelectedZTraceToScene( zTraceList );
					SetFocus(openGLWindow);				// switch to 3D Scene window
					break;

				case CM_REMOVEFROMSCENE:
					RemoveSelectedObjectsFromScene( zTraceList );
					SetFocus(openGLWindow);				// switch to 3D Scene window
					break;

				case CM_REFRESHLIST:					// regenerate the list
					FillZList( zTraceList );
					break;

				case CM_INFOLIST:						// display count of items
					DisplayListInfo( zTraceList );
					break;

				case CM_SAVELIST:
					sprintf(filename,"ztraces.csv");
					ZeroMemory(&ofn, sizeof(ofn));						// clear the OPENFILENAME fields
					ofn.lStructSize = sizeof(ofn);						// set only the necessary values
					ofn.hwndOwner = appWnd;
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "All Files\0*.*\0Comma Separated Value file\0*.csv\0\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrTitle = "Save Z-Trace List As...";
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY; // ask user if overwrite desired
					if ( GetSaveFileName(&ofn) == TRUE )				// display the Open dialog box
						{
						SaveList( zTraceList, filename );
						}
					break;

				case CM_ZTRACECOLOR:
					SetZTraceColor( zTraceList );
					break;

				case CM_ZTRACENAME:
					SetZTraceName( zTraceList );
					break;

				case CM_ZTRACECOMMENT:
					SetZTraceComment( zTraceList );
					break;

				case CM_DELETEZTRACES:
					DeleteZTraces( zTraceList );
					break;

				case CM_COPYZTRACES:
					CopyZTraces( zTraceList );
					break;

				case CM_SMOOTHZTRACES:
					SmoothZTraces( zTraceList );
					break;

				case CM_GRIDATZTRACES:
					GridAtZTraces( zTraceList );
					break;
				}
		}
	return( DefWindowProc(hWnd, message, wParam, lParam) );
}


HWND MakeZTraceListWindow( void )			// display list for user selection of objects to view
{
	WNDCLASS wndclass;
	HWND sWnd;
	HMENU hMenu;
	int width;
	RECT r;
														// create the object list window...
	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)ListZTracesProc;
	wndclass.cbClsExtra  = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance = appInstance;
	wndclass.hCursor   = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon     = NULL;							// not used for WS_EX_TOOLWINDOW
	wndclass.lpszMenuName = "ZListMenu";
	wndclass.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wndclass.lpszClassName = "ZListClass";
	RegisterClass(&wndclass);

	ClientRectLessStatusBar( &r );						// position window in upper left corner of client
	ClientRectToScreen( appWnd, &r );
	width = 140;										// set width based on how many data fields present
	if ( CurrSeries->listZTraceNote )
		width += 100;
	if ( CurrSeries->listZTraceRange )
		width += 200;
	if ( CurrSeries->listZTraceLength )
		width += 100;

	sWnd = CreateWindowEx( WS_EX_TOOLWINDOW, "ZListClass", "Z-Traces", WS_OVERLAPPEDWINDOW,
								r.right-width, r.top, width, r.bottom-r.top, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

	if ( sWnd )
		{											// if window created, display it
		ShowWindow(sWnd, SW_SHOW);			// and check corresponding menu item in main window	
		hMenu = GetMenu( appWnd );
		CheckMenuItem( hMenu, CM_ZLIST, MF_BYCOMMAND | MF_CHECKED );
		}

	return( sWnd );
}

void CmZList( void )									// create or destroy object list window
{
	if ( IsWindow( zTraceWindow ) ) DestroyWindow( zTraceWindow );						
	else if ( CurrSeries->zcontours )
		{
		if ( CurrSeries->zcontours->Number() > 0 )			// if z contours found					
			zTraceWindow = MakeZTraceListWindow();			// create list window
		else
			ErrMsgOK( ERRMSG_NOZTRACES, "" );				// otherwise report error
		}
	else ErrMsgOK( ERRMSG_NOZTRACES, "" );
}


void FillDistanceList( HWND list )
{
	VRMLObject *object1, *object2;
	LV_ITEM lvi;
	double distance;
	char txt[MAX_CONTOUR_NAME];

	SendMessage(list, LVM_DELETEALLITEMS, 0, 0 );	// clear the list view

	lvi.mask = LVIF_TEXT | LVIF_IMAGE; lvi.state = 0; lvi.stateMask = 0;
	sprintf(txt,"computing...");
	lvi.iSubItem = 0; lvi.pszText = txt; lvi.iImage = 0;
	lvi.iItem = SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)(&lvi));

	DistanceListThreadBegin();
}

void CompleteDistanceList( HWND list )
{
	InterObject *interobject;
	int n;
	LV_ITEM lvi;
	char txt[MAX_CONTOUR_NAME];

	SendMessage( list, LVM_DELETEALLITEMS, 0, 0 );		// clear list

	if ( InterObjectDistances && !AbortDistances )		// if too many items, as user if OK to continue
		{												// because may be excessively slow
		n = InterObjectDistances->Number();
		if ( n > 0xFFFF )
			if ( MessageBox(list,"More than 65,535 distances. Display?","WARNING!",MB_YESNO|MB_ICONEXCLAMATION) == IDNO )
				n = 0;
		if ( n )
		  {
		  SendMessage(list, LVM_SETITEMCOUNT, (WPARAM)n, 0);
		  interobject = InterObjectDistances->first;
		  while ( interobject )							// put each distance in list view
			{
			lvi.mask = LVIF_TEXT | LVIF_IMAGE; lvi.state = 0; lvi.stateMask = 0;
			lvi.iSubItem = 0; lvi.pszText = interobject->object1; lvi.iImage = 0;
			lvi.iItem = SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)(&lvi));
			lvi.mask = LVIF_TEXT; lvi.iSubItem = 1; lvi.pszText = interobject->object2;
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
			sprintf(txt,"%1.*g", Precision,interobject->distance);
			lvi.iSubItem = 2; lvi.pszText = txt;
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)(&lvi));
			interobject = interobject->next;			// and do next pair of objects
			}
		  }
		}
}

LRESULT APIENTRY ListDistanceProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pnmh;
	LV_KEYDOWN *key;
	LV_COLUMN column;
	LV_ITEM lvi;
	RECT r;
	HMENU hMenu;
	HIMAGELIST himl;
	OPENFILENAME ofn;		
	int w, h;
	char txt[MAX_PATH], filename[MAX_PATH];

	switch (message)
		{										// initialize the list view section info
		case WM_CREATE:
			GetClientRect( hWnd, &r );
			w = r.right-r.left-10; h = r.bottom-r.top-10,
			distanceList = CreateWindow( WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER | LVS_SORTASCENDING,
														5, 5, w, h, hWnd, NULL, appInstance, NULL);	
			himl = ImageList_Create(16, 16, TRUE, 1, 1);
			ImageList_AddIcon(himl, LoadIcon(appInstance, "DistanceIcon"));
			SendMessage(distanceList, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)(himl));	// setup icons
			sprintf(txt,"Distances from %s to %s",limitLeftDistanceList,limitRightDistanceList);
			SetWindowText(hWnd,txt);
			column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
			column.fmt = LVCFMT_LEFT; column.pszText = txt;		// setup columns
			column.iSubItem = 0; strcpy(txt,"Object 1"); column.cx = 120;
			SendMessage(distanceList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			column.iSubItem = 1; strcpy(txt,"Object 2"); column.cx = 120;
			SendMessage(distanceList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			column.iSubItem = 2; strcpy(txt,"Distance"); column.cx = 100;
			SendMessage(distanceList, LVM_INSERTCOLUMN, column.iSubItem, (LPARAM)(&column));
			FillDistanceList( distanceList );
			SetFocus( distanceList );
			hMenu = GetSystemMenu( hWnd, FALSE );						// allow Maximize on titlebar menu
			EnableMenuItem( hMenu, SC_MAXIMIZE, MF_ENABLED );
			break;

		case UM_DISTANCEDONE:
			CompleteDistanceList( distanceList );			// fill listview with InterObjectDistances
			hMenu = GetMenu( hWnd );
			EnableMenuItem( hMenu, CM_REFRESHLIST, MF_BYCOMMAND | MF_ENABLED );
			DrawMenuBar( hWnd );
			break;

		case UM_DISTANCEPROGRESS:							// report count of distances calculated so far
			lvi.mask = LVIF_TEXT | LVIF_IMAGE; lvi.state = 0; lvi.stateMask = 0;
			sprintf(txt,"%d",lParam);
			lvi.iSubItem = 0; lvi.pszText = txt; lvi.iImage = 0;
			lvi.iItem = SendMessage(distanceList, LVM_SETITEM, 0, (LPARAM)(&lvi));
			break;

		case WM_SIZE:							// resize list view control to match dialog box
			GetClientRect( hWnd, &r );
			SetWindowPos( distanceList, 0, 0, 0, r.right-r.left-10, r.bottom-r.top-10, SWP_NOMOVE | SWP_NOOWNERZORDER);
			break;
			
		case WM_DESTROY:
			if ( hDistanceListThread ) AbortDistances = true;
			hMenu = GetMenu( appWnd );
			CheckMenuItem( hMenu, CM_DISTANCELIST, MF_BYCOMMAND | MF_UNCHECKED );	// uncheck menu item
			distanceWindow = NULL;							// clear global handle pointer
			SetFocus(appWnd);								// return to main window
			break;

		case WM_NOTIFY:							// process listview notification messages
			pnmh = (LPNMHDR)lParam;
			switch (pnmh->code)
				{
				case LVN_KEYDOWN:
					key = (LV_KEYDOWN *)lParam;	// allow control+TAB to switch active window
					if ( (key->wVKey==VK_TAB) && (GetKeyState(VK_CONTROL) & 0x8000) ) CmNextWindow( distanceWindow );
					return TRUE;
				}
			break;

		case WM_SETFOCUS:
			SetFocus( distanceList );				// activate list view if window activated
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case CM_REFRESHLIST:					// regenerate the list
					hMenu = GetMenu( hWnd );			// but block repeat 'til thread done
					EnableMenuItem( hMenu, CM_REFRESHLIST, MF_BYCOMMAND | MF_GRAYED );
					FillDistanceList( distanceList );
					break;

				case CM_INFOLIST:						// display count of items
					DisplayListInfo( distanceList );
					break;

				case CM_SAVELIST:
					sprintf(filename,"distances.csv");
					ZeroMemory(&ofn, sizeof(ofn));						// clear the OPENFILENAME fields
					ofn.lStructSize = sizeof(ofn);						// set only the necessary values
					ofn.hwndOwner = appWnd;
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "All Files\0*.*\0Comma Separated Value file\0*.csv\0\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrTitle = "Save Distance List As...";
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY; // ask user if overwrite desired
					if ( GetSaveFileName(&ofn) == TRUE )				// display the Open dialog box
						{
						SaveList( distanceList, filename );
						}
					break;
				}
		}
	return( DefWindowProc(hWnd, message, wParam, lParam) );
}


HWND MakeDistanceListWindow( void )			// display list for user selection of objects to view
{
	WNDCLASS wndclass;
	HWND sWnd;
	HMENU hMenu;
	int width;
	RECT r;
														// create the object list window...
	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)ListDistanceProc;
	wndclass.cbClsExtra  = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance = appInstance;
	wndclass.hCursor   = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon     = NULL;							// not used for WS_EX_TOOLWINDOW
	wndclass.lpszMenuName = "DistanceListMenu";
	wndclass.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wndclass.lpszClassName = "DistanceListClass";
	RegisterClass(&wndclass);

	ClientRectLessStatusBar( &r );						// position window in upper left corner of client
	ClientRectToScreen( appWnd, &r );
	width = 340;										// set width based on how many data fields present

	sWnd = CreateWindowEx( WS_EX_TOOLWINDOW, "DistanceListClass", "InterObject Distances", WS_OVERLAPPEDWINDOW,
								r.left, r.top, width, r.bottom-r.top, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

	if ( sWnd )
		{											// if window created, display it
		ShowWindow(sWnd, SW_SHOW);			// and check corresponding menu item in main window	
		hMenu = GetMenu( appWnd );
		CheckMenuItem( hMenu, CM_DISTANCELIST, MF_BYCOMMAND | MF_CHECKED );
		}

	return( sWnd );
}

void CmDistanceList( void )									// create or destroy object list window
{
	if ( IsWindow( distanceWindow ) ) DestroyWindow( distanceWindow );						
	else distanceWindow = MakeDistanceListWindow();			// create list window
}
