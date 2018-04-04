////////////////////////////////////////////////////////////////////////////////
// WinMain for RECONSTRUCT, an open source Win32 GUI application
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
// -+- change: Added wildfire tool button response.
// modified 4/21/05 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added autotrace.
// modified 4//29/05 by JCF (fiala@bu.edu)
// -+- change: Added CmImportLines to Series menu. Removed used code for stopping wildfires.
// modified 5/21/05 by JCF (fiala@bu.edu)
// -+- change: Fixed missing break after Ctrl-D message processing.
// modified 6/8/05 by JCF (fiala@bu.edu)
// -+- change: Changed 'F' and 'B' key cmds to only work when Wildfire tool is active.
// modified 6/23/05 by JCF (fiala@bu.edu)
// -+- change: Modified domain menu and section menu to include flip and rotate.
// modified 10/3/05 by JCF (fiala@bu.edu)
// -+- change: Added MButtonUp for select with any tool
// modified 3/29/06 by JCF (fiala@bu.edu)
// -+- change: Rewrote JU Lu's autotracing stuff so it would work properly.
// modified 5/3/06 by JCF (fiala@bu.edu)
// -+- change: Added CmSmoothSelected() for smoothing traces.
// modified 5/24/06 by JCF (fiala@bu.edu)
// -+- change: Added CmExportLines for DXF trace export.
// modified 6/16/06 by JCF (fiala@bu.edu)
// -+- change: Added Debug Logging menu item.
// modified 11/16/06 by JCF (fiala@bu.edu)
// -+- change: Added CmGetClipboardAttributes() for Ctrl-G keyboard access.
// -+- change: Added CmFindTrace() to trace menu and to Ctrl+f
// modified 11/21/06 by JCF (fiala@bu.edu)
// -+- change: Added PaintingViews flag to block reentry into WM_PAINT before completion.
// -+- change: UnRegisterHotkey() before exit program.
// modified 2/5/07 by JCF (fiala@bu.edu)
// -+- change: Added CmDebugTimes() to allow more debug options.
// modified 5/8/07 by JCF (fiala@bu.edu)
// -+- change: Fixed ObjectList (and DistancesList?) hanging by having thread procedures post to appWnd only.
// modified 5/11/07 by JCF (fiala@bu.edu)
// -+- change: Added CM_MAGNIFICATION for Zoom menu.

#include "reconstruct.h"

LRESULT APIENTRY WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HDC dc;
	RECT wrect;
	POINT mpoint;
	int p;
	char txt[64];
	bool shift, control;
	PAINTSTRUCT ps;

	switch(msg)
		{
		case WM_CLOSE:
			if ( SaveSeriesObjects() )				// try to save series objects
				{									// if successful, then
				FreeSeriesObjects();				// free objects and destroy Wnd
				FreeProgramObjects();
				DestroyWindow( hWnd );				// end application
				}
			break;

		case WM_CREATE:								// setup application
			Init( hWnd );
			RegisterHotKey(appWnd, 0, MOD_CONTROL, 0x54);  // Ctrl-T to stop Wildfire autotracing: ID is 0, 0x54 is T
			break;

		case WM_DESTROY:
			UnregisterHotKey(appWnd, 0);			// release Hotkey from above
			PostQuitMessage(0);
			break;

		case WM_PAINT:								// repaint window but only if not already repainting it!
			if ( !PaintingViews && GetUpdateRect(hWnd,&wrect,FALSE) )
				{
				dc = BeginPaint( hWnd, &ps );
				PaintingViews = true;
				PaintViews( dc, wrect );
				PaintingViews = false;
				EndPaint( hWnd, &ps );
				}
			break;
													// resize window
		case WM_SIZE:
			if ( IsWindow( statusbarWindow ) ) SendMessage(statusbarWindow, WM_SIZE,wParam,lParam );
			if ( wParam == SIZE_MAXIMIZED )
				{
				WindowWasMaximized = true;
				SizeViewPorts();					// this is not caught by EXITSIZEMOVE
				}
			else if ( (wParam == SIZE_RESTORED) && WindowWasMaximized )
				{
				WindowWasMaximized = false;			// here window was unmaximized
				SizeViewPorts();
				}
			break;

		case WM_EXITSIZEMOVE:					// do only one resize after user drags edge
			SizeViewPorts();
			break;

		case WM_SETCURSOR:
			SetToolCursor( hWnd );				// set cursor based on current tool
			break;

		case WM_TIMER:							// timer interrupt processing
			switch ( wParam )
			  {
			  case STATUS_TIMER:						// timer created when statusbar created
				UpdateStatusBar();							// update the stataus bar each interval
				break;
			  case SCROLL_TIMER:						// timer created when captured mouse near edge
				ScrollViews();								// scroll views in window
				ScrollOccurred = true;						// and flag event for corrected drawing
				break;
			  case AUTOTRACE_TIMER:						// timer created when autotracing invoked
				Sleep(100);									// a little delay is needed to make paging not jerky
				DoAutoTrace();								// trace then page section
			  }
      		break;

		case WM_LBUTTONDOWN:					// left mouse button press
			LButtonDown( hWnd, wParam, lParam );
			break;
		case WM_RBUTTONDOWN:					// right mouse button press
			RButtonDown( hWnd, wParam, lParam );
			break;
		case WM_MOUSEMOVE:						// mouse is moving in window
			MouseMove( hWnd, wParam, lParam );
			break;
		case WM_LBUTTONUP:						// left mouse button release
			LButtonUp( hWnd, wParam, lParam );
			break;
		case WM_MBUTTONUP:						// middle mouse button press+release
			MButtonUp( hWnd, wParam, lParam );
			break;
		case WM_RBUTTONUP:						// right mouse button release
			RButtonUp( hWnd, wParam, lParam );
			break;
		case WM_MOUSEWHEEL:						// mouse wheel movements are the same as page up/down
			if ( (int)wParam > 0 ) CmSuccessor();
			else CmPredecessor();
			break;
		case WM_HOTKEY:							// registered to ctrl-t
			AutoTrace=0;
			break;

		case UM_RENDERDONE: RenderThreadEnd();			// end render images thread
			break;
		case UM_THUMBSDONE: ThumbsThreadEnd();			// end thumbnails background thread
			break;
		case UM_GENERATE3DDONE: Generate3DThreadEnd();	// end 3D thread and show scene
			break;										// relay progress update to list view window
		case UM_OBJECTPROGRESS: PostMessage(objectWindow,UM_OBJECTPROGRESS,0,lParam);
			break;
		case UM_OBJECTDONE: ObjectListThreadEnd();		// end thread that scans files for objects
			break;										// posting from thread to list doesn't always work!
		case UM_DISTANCEPROGRESS: PostMessage(distanceWindow,UM_DISTANCEPROGRESS,0,lParam);
			break;
		case UM_DISTANCEDONE: DistanceListThreadEnd();	// end distance calcs thread
			break;

		case WM_COMMAND:						// Process menu commands...
			switch (LOWORD(wParam)) {
														// program cmds
			case CM_TOOLBAR: CmRestoreToolbar();
				break;
			case CM_STATUSBAR: CmStatusBar();
				break;
			case CM_DEBUGLOG: CmDebugLog();
				break;
			case CM_DEBUGTIMES: CmDebugTimes();
				break;
			case CM_DEBUG: CmDebug();
				break;

			case CM_EXIT:								// same as WM_CLOSE
				if ( SaveSeriesObjects() )				// try to save series objects
					{									// if successful, then
					FreeSeriesObjects();				// free objects and destroy Wnd
					FreeProgramObjects();
					DestroyWindow( hWnd );
					}
				break;
														// series menu cmds
			case CM_OPEN: CmOpen();
				break;
			case CM_CLOSE: CmClose();
				break;
			case CM_NEWSERIES: CmNewSeries();
				break;
			case CM_SAVESERIES: CmSaveSeries();
				break;
			case CM_SETOPTIONS: CmSetOptions();
				break;
			case CM_RENDERSECTIONS: CmRenderSections();
				break;
			case CM_EXPORTLINES: CmExportLines();
				break;
			case CM_EXPORTTRACELISTS: CmExportTraceLists();
				break;
			case CM_IMPORTIMAGES: CmImportImages();
				break;
			case CM_IMPORTLINES: CmImportLines();
				break;
			case CM_IMPORTSERIES: CmImportSeries();
				break;
														// section menu cmds
			case CM_NEWSECTION: CmNewSection();
				break;
			case CM_SECTIONLIST: CmSectionList();
				break;
			case CM_THUMBNAILS: CmRestoreThumbnails();
				break;
			case CM_PREDECESSOR: CmPredecessor();
				break;
			case CM_SUCCESSOR: CmSuccessor();
				break;
			case CM_SAVESECTION: CmSaveSection();
				break;
			case CM_SECTIONATTRIBUTES: CmSectionAttributes();
				break;
			case CM_CENTER: CmCenter();
				break;
			case CM_LASTZOOM: CmLastZoom();
				break;
			case CM_ACTUALPIXELS: CmActualPixels();
				break;
			case CM_MAGNIFICATION: CmMagnification();
				break;
			case CM_BLEND: CmBlend();
				break;
			case CM_LOCKSECTION: CmLockSection();
				break;
			case CM_TYPESECTIONTFORM: CmTypeSectionTform();
				break;
			case CM_BYCORRELATION: CmByCorrelation();
				break;
			case CM_UNDOSECTION: CmUndoSection();
				break;
			case CM_REDOSECTION: CmRedoSection();
				break;
			case CM_RESETSECTION: CmResetSection();
				break;
			case CM_REPEATADJUSTMENT: CmRepeatAdjustment();
				break;
			case CM_PROPAGATEADJUST: CmPropagateAdjustment();
				break;
			case CM_RECORD: CmRecord();
				break;
			case CM_COPYACTIVENFORM: CmCopyActiveNform();
				break;
			case CM_CALIBRATESECTIONS: CmCalibrateSections();
				break;
			case CM_90CLKWISE: Cm90Clkwise();			// F2 keyboard accel
				break;
			case CM_90COUNTERCLKWISE: Cm90Counterclkwise();
				break;
			case CM_ROTATE180: CmRotate180();
				break;
			case CM_FLIPHORZ: CmFlipHorz();					// F1 keyboard accel
				break;
			case CM_FLIPVERT: CmFlipVert();
				break;

														// domain menu cmds
			case CM_DOMAINLIST: CmDomainList();
				break;
			case CM_NEWDOMAINFILE: CmNewDomainFile();
				break;
			case CM_MERGEFRONT: CmMergeFront();
				break;
			case CM_MERGEREAR: CmMergeRear();
				break;
			case CM_RESTORECONTRAST: CmRestoreContrast();
				break;
			case CM_DOMAINATTRIBUTES: CmDomainAttributes();
				break;
			case CM_DELETEDOMAIN: CmDeleteDomain();
				break;
			case CM_DEFAULTDOMAIN: CmDefaultDomain();
				break;
			case CM_CLEARDOMAINTFORM: CmClearDomainTransform();
				break;
														// trace menu cmds
			case CM_SELECTALL: CmSelectAll();
				break;
			case CM_UNSELECTALL: CmUnSelectAll();
				break;
			case CM_ZOOMSELECTED: CmZoomSelected();
				break;
			case CM_CUTSELECTED: CmCutSelected();
				break;
			case CM_COPYSELECTED: CmCopySelected();
				break;
			case CM_PASTESELECTED: CmPasteSelected();
				break;
			case CM_DELETESELECTED: CmDeleteSelected();
				break;
			case CM_LISTTRACES: CmListTraces();
				break;
			case CM_TRACEATTRIBUTES: CmTraceAttributes();
				break;
			case CM_TRACEPALETTE: CmTracePalette();
				break;
			case CM_ALIGNRIGID: CmAlignRigid();
				break;
			case CM_ALIGNLINEAR: CmAlignLinear();
				break;
			case CM_ALIGNDEFORM: CmAlignDeform();
				break;
			case CM_ALIGNQUADRATIC: CmAlignQuadratic();
				break;
			case CM_CALIBRATETRACES: CmCalibrateTraces();
				break;
			case CM_MERGESELECTED: CmMergeSelected();
				break;
			case CM_REVERSESELECTED: CmReverseSelected();
				break;
			case CM_SIMPLIFYSELECTED: CmSimplifySelected();
				break;
			case CM_SMOOTHSELECTED: CmSmoothSelected();
				break;
			case CM_FINDTRACE: CmFindTrace();
				break;
														// 3D menu cmds
			case CM_OBJECTLIST: CmObjectList();
				break;
			case CM_3DSCENE: CmRestore3DWindow();
				break;
			case CM_ZLIST: CmZList();
				break;
			case CM_DISTANCELIST: CmDistanceList();
				break;
														// help menu cmds
			case CM_HELPRECON: CmHelpReconstruct();
				break;
			case CM_HELPKEYTABLE: CmHelpKeyTable();
				break;
			case CM_HELPMOUSE: CmHelpMouse();
				break;
			case CM_HELPLICENSE: CmHelpLicense();
				break;
			case CM_HELPABOUT: CmHelpAbout();
				break;
														// movement cmds generated by function keys
			case CM_CLKWISE:
			case CM_COUNTERCLKWISE:
			case CM_SLOWCLKWISE:
			case CM_SLOWCOUNTERCLKWISE:
			case CM_FASTCLKWISE:
			case CM_FASTCOUNTERCLKWISE:
			case CM_XSMALLER:
			case CM_XLARGER:
			case CM_YSMALLER:
			case CM_YLARGER:
			case CM_SLOWXSMALLER:
			case CM_SLOWXLARGER:
			case CM_SLOWYSMALLER:
			case CM_SLOWYLARGER:
			case CM_FASTXSMALLER:
			case CM_FASTXLARGER:
			case CM_FASTYSMALLER:
			case CM_FASTYLARGER:
			case CM_SLANTXLEFT:
			case CM_SLANTXRIGHT:
			case CM_SLANTYUP:
			case CM_SLANTYDOWN:
			case CM_SLOWSLANTXLEFT:
			case CM_SLOWSLANTXRIGHT:
			case CM_SLOWSLANTYUP:
			case CM_SLOWSLANTYDOWN:
			case CM_FASTSLANTXLEFT:
			case CM_FASTSLANTXRIGHT:
			case CM_FASTSLANTYUP:
			case CM_FASTSLANTYDOWN:
				CmMovement( hWnd, LOWORD(wParam) );
				break;
															// tool cmds
			case TB_ARROW: CmArrow();
				break;
			case TB_ZOOM: CmZoom();
				break;
			case TB_MAGNIFY: CmMagnify();
				break;
			case TB_DOMAIN: CmDomain();
				break;
			case TB_POINT: CmPoint();
				break;
			case TB_ELLIPSE: CmEllipse();
				break;
			case TB_RECTANGLE: CmRectangle();
				break;
			case TB_LINE: CmLine();
				break;
			case TB_MULTILINE: CmMultiLine();
				break;
			case TB_PENCIL: CmPencil();
				break;
			case TB_CULTILINE: CmCultiLine();
				break;
			case TB_SCISSOR: CmScissor();
				break;
			case TB_ZLINE: CmZLine();
				break;
			case TB_GRID: CmGrid();
				break;
			case TB_WILDFIRE: CmWildfire();
				break;
			case TB_SCALPEL: CmScalpel();
				break;
			}

			break;

		case WM_SYSKEYDOWN:		// block processing of system functions for F keys
			switch (wParam)
				{
				case VK_F1:
				case VK_F2:
				case VK_F3:
				case VK_F4:
				case VK_F5:
				case VK_F6:
				case VK_F7:
				case VK_F8:
				case VK_F9:
				case VK_F10:
				case VK_F11:
				case VK_F12:
					break;		// but allow ALT keys and others with DefWindowProc()
				default:
					return( DefWindowProc(hWnd, msg, wParam, lParam) );
				}
			break;
						// implement rest of accel. table here so no child key commands will be intercepted
		case WM_KEYDOWN:
			if ( GetKeyState(VK_SHIFT) & 0x8000 ) shift = true; else shift = false;
			if ( GetKeyState(VK_CONTROL) & 0x8000 ) control = true; else control = false;
			switch (wParam)
				{
				case VK_NEXT: CmPredecessor();
					break;
				case VK_PRIOR: CmSuccessor();
					break;
				case VK_ESCAPE:	CmEscapeCurrentTool();
					break;
				case VK_DELETE:
					if ( FrontView == DomainView ) CmDeleteDomain();
					else CmDeleteSelected();
					break;
				case VK_INSERT: CmPasteSelected();
					break;
				case VK_BACK: CmBackspace();
					break;
				case VK_END: CmLastZoom();
					break;
				case VK_HOME:
					if ( control ) CmZoomSelected();
					else CmCenter();
					break;
				case VK_TAB:
					if ( control ) CmNextWindow( appWnd );
					break;
				case VK_UP:
					if ( control ) CmMovement( hWnd, CM_SLOWUP );
					else if ( shift ) CmMovement( hWnd, CM_FASTUP );
					else CmMovement( hWnd, CM_UP );
					break;
				case VK_DOWN:
					if ( control ) CmMovement( hWnd, CM_SLOWDOWN );
					else if ( shift ) CmMovement( hWnd, CM_FASTDOWN );
					else CmMovement( hWnd, CM_DOWN );
					break;
				case VK_LEFT:
					if ( control ) CmMovement( hWnd, CM_SLOWLEFT );
					else if ( shift ) CmMovement( hWnd, CM_FASTLEFT );
					else CmMovement( hWnd, CM_LEFT );
					break;
				case VK_RIGHT:
					if ( control ) CmMovement( hWnd, CM_SLOWRIGHT );
					else if ( shift ) CmMovement( hWnd, CM_FASTRIGHT );
					else CmMovement( hWnd, CM_RIGHT );
					break;

				default:	// if not an identified virtual key code, check for ascii character
					switch ( MapVirtualKey( wParam, 2 ) )
						{
						case ' ': CmBlend();
							break;
						case '`':
						case '/': CmToggleViews( hWnd );
							break;
						case 'A':
							if ( control ) CmTraceAttributes();
							break;
						case 'B':
							if ( control ) CmPasteAttributes();
							else if ( CurrentTool == WILDFIRE_TOOL ) CmTraceBackward();
							break;
						case 'C':
							if ( control ) CmCopySelected();
							break;
						case 'D':
							if ( control ) CmUnSelectAll();
							break;
						case 'E':
						    /* Add new key to hide all traces */
							if ( control )  exit(0);
							break;
						case 'F':
							if ( control ) CmFindTrace();
							else if ( CurrentTool == WILDFIRE_TOOL ) CmTraceForward();
							break;
						case 'G':
							if ( control ) CmGetClipboardAttributes();
							break;
						case 'H':
							if ( control ) CmHideTraces();
							break;
						case 'L':
							if ( control ) CmLockSection();
							break;
						case 'M':
							if ( control ) CmMergeSelected();
							break;
						case 'O':
							if ( control ) CmSetOptions();
							break;
						case 'P':
							if ( control ) CmPrecisionCursor();
							break;
						case 'R':
							if ( control ) CmRepeatAdjustment();
							break;
						case 'S':
							if ( control ) CmSelectAll();
							break;
						case 'U':
							if ( control ) CmResetSection();
							break;
						case 'V':
							if ( control ) CmPasteSelected();
							break;
						case 'X':
							if ( control ) CmCutSelected();
							break;
						case 'Z':
							if ( control ) CmUndoSection();
							break;
						case '=': CmAdjustContrast( hWnd, 0.015, 1.0 );
							break;
						case '-': CmAdjustContrast( hWnd, -0.015, 1.0 );
							break;
						case ']': CmAdjustContrast( hWnd, 0.0, 1.05 );
							break;
						case '[': CmAdjustContrast( hWnd, 0.0, 0.95 );
							break;
						case '\\':
							if ( control ) CmByCorrelation();
							break;
						case '0':
							if ( control ) CmSetDefaultContour( 0 );
							else CmUseDefaultContour( 0 );
							break;
						case '1':
							if ( control ) CmSetDefaultContour( 1 );
							else CmUseDefaultContour( 1 );
							break;
						case '2':
							if ( control ) CmSetDefaultContour( 2 );
							else CmUseDefaultContour( 2 );
							break;
						case '3':
							if ( control ) CmSetDefaultContour( 3 );
							else CmUseDefaultContour( 3 );
							break;
						case '4':
							if ( control ) CmSetDefaultContour( 4 );
							else CmUseDefaultContour( 4 );
							break;
						case '5':
							if ( control ) CmSetDefaultContour( 5 );
							else CmUseDefaultContour( 5 );
							break;
						case '6':
							if ( control ) CmSetDefaultContour( 6 );
							else CmUseDefaultContour( 6 );
							break;
						case '7':
							if ( control ) CmSetDefaultContour( 7 );
							else CmUseDefaultContour( 7 );
							break;
						case '8':
							if ( control ) CmSetDefaultContour( 8 );
							else CmUseDefaultContour( 8 );
							break;
						case '9':
							if ( control ) CmSetDefaultContour( 9 );
							else CmUseDefaultContour( 9 );
							break;
						}
				}
			break;

		default:
			return( DefWindowProc(hWnd, msg, wParam, lParam) );
		}

	return 0L;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int nCmdShow )
{
	MSG msg;
	WNDCLASSEX wndclass;
	HACCEL haccel;
	RECT drect;
	char *cmdPath;

	appInstance = hInstance;					// remember HINSTANCE in global variable

	if (!hPrevInstance) {
		wndclass.cbSize = sizeof(WNDCLASSEX);
		wndclass.style = CS_OWNDC;
		wndclass.lpfnWndProc = (WNDPROC)WndProc;
		wndclass.cbClsExtra  = 0;
		wndclass.cbWndExtra  = 0;
		wndclass.hInstance = appInstance;
		wndclass.hCursor   = LoadCursor(NULL,IDC_ARROW);
		wndclass.hIcon     = LoadIcon(hInstance,"ALargeIcon");
		wndclass.lpszMenuName = "MainMenu";
		wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndclass.lpszClassName = "ReconstructWinClass";
		wndclass.hIconSm	= LoadIcon(hInstance,"ASmallIcon");
		if ( !RegisterClassEx(&wndclass) ) return FALSE;
		}

	GetClientRect( GetDesktopWindow(), &drect);			// PUT THE INITIAL WINDOW SIZE IN REGISTRY OR .ini?

	appWnd = CreateWindow( "ReconstructWinClass", "Reconstruct",
								WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
								0, 0, drect.right-drect.left, drect.bottom-drect.top-30,
								(HWND)NULL, (HMENU)NULL, appInstance, (LPVOID)NULL);

	ShowWindow( appWnd, nCmdShow );						// display window

	haccel = LoadAccelerators(appInstance, "FunctionKeyAccels");// translate F keys into command messages

	cmdPath = strtok(cmdLine,"\'\"");					// remove enclosing quotes around command line string
	if ( cmdPath ) ValidOpen( cmdPath );				// and try to open series file (e.g. from drag and drop on icon)

	while( GetMessage(&msg,(HWND)NULL,0,0) )			// main message loop
		{
		TranslateAccelerator(appWnd,haccel,&msg);
		DispatchMessage(&msg);
		}

	return msg.wParam;
}
