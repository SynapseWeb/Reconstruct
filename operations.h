//////////////////////////////////////////////////////////////////////////////////////////////////
//  Definitions for all user interface operations for Reconstruct
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
// -+- change: Added CmWildfire() tool button response routine.
// modified 2/03/05 by JCF (fiala@bu.edu)
// -+- change: Added CmScalpel() tool button response routine. 
// modified 3/07/05 by JCF (fiala@bu.edu)
// -+- change: Added InvalidateAllViews() to handle rerendering cases when have a domain selected.
// modified 4/21/05 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added DoAutoTrace and associated auxiliary functions.
//             Added CmTraceForward, CmTraceBackward, CmStopTrace
// modified 4//29/05 by JCF (fiala@bu.edu)
// -+- change: Added CmImportLines to Series menu. Cleaned up unnecessary global operation definitions.
// modified 6/23/05 by JCF (fiala@bu.edu)
// -+- change: Added rotation operations Cm90Clkwise, Cm90Counterclkwise, and CmRotate180.
// modified 9/13/05 by JCF (fiala@bu.edu)
// -+- change: Added UnPushPaletteButtons() from palette.cpp.
// modified 10/3/05 by JCF (fiala@bu.edu)
// -+- change: Added MButtonUp for select with any tool
// modified 4/14/06 by JCF (fiala@bu.edu)
// -+- change: Creation of SlabSurface operation and removal of PlanarContourSurface in boissonnat.cpp.
// modified 4/25/06 by JCF (fiala@bu.edu)
// -+- change: Added DisplayListInfo utility.
// modified 5/3/06 by JCF (fiala@bu.edu)
// -+- change: Added CmSmoothSelected() for smoothing traces.
// modified 5/25/06 by JCF (fiala@bu.edu)
// -+- change: Added SectionRangeDlgProc and CmExportLines to support DXF export.
// modified 6/14/06 by JCF (fiala@bu.edu)
// -+- change: Added CloseAllChildWindows() operation from end.cpp.
// modified 6/16/06 by JCF (fiala@bu.edu)
// -+- change: Moved debug items to program menu. Added debug logging item.
// modified 7/2/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added AddContour and StoreTracingInfo functions under tools.cpp.
// modified 7/7/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Moved PrepareADib function (defined in viewport.cpp)
// modified 11/13/06 by JCF (fiala@bu.edu)
// -+- change: Eliminated compiler WARNINGS/ERRORS so would compile under VC++.
// modified 11/15/06 by JCF (fiala@bu.edu)
// -+- change: Removed StoreTracingInfo(), AddContour(), and PrepareADib global functions declarations.
//             This functionality is now incorporated into methods of Section and ViewPort.
// modified 11/16/06 by JCF (fiala@bu.edu)
// -+- change: Added CmGetClipboardAttributes() and cleaned up organization of keyboard functions.
// -+- change: Added CmFindTrace function for new trace menu function.
// modified 11/22/06 by JCF (fiala@bu.edu)
// -+- change: Added MakeAbsolutePath to utility.cpp
// modified 2/5/07 by JCF (fiala@bu.edu)
// -+- change: Added CmDebugTimes() to allow more debug options.
// modified 4/2/07 by JCF (fiala@bu.edu)
// -+- change: Modified TestGBMfile to also return the number of images in an image stack
// modified 4/23/07 by JCF (fiala@bu.edu)
// -+- change: Modified CmFindTrace() to split out ShowSectionTrace() to be also called from Object List.
// modified 4/30/07 by JCF (fiala@bu.edu)
// -+- change: Added RGBtoHSB routine to utilities.
// modified 5/11/07 by JCF (fiala@bu.edu)
// -+- change: Added CmMagnification() for Zoom menu.
// modified 7/18/07 by JCF (fiala@bu.edu)
// -+- change: Moved RGB2HSB back into ADib (so no need for global declaration)

															// program initialization (init.cpp)
void Init( HWND hWnd );											// init application
void InitCurrSeries( void );									// init variables when open series

															// program termination (end.cpp)
bool SaveSeriesObjects( void );									// try to save series objects
void CloseAllChildWindows( void );								// destroy subordinate windows
void FreeSeriesObjects( void );									// free objects and clear ptrs
void FreeProgramObjects( void );								// for program termination

															// multithreading (threads.cpp)
void RenderThreadBegin( void );									// backgnd thread for exporting images
void RenderThreadEnd( void );									// terminate rendering thread
void ThumbsThreadBegin( void );									// invoke backgnd thread for thumbnails
void ThumbsThreadEnd( void );									// terminate thumbnails thread
void ObjectListThreadBegin( void );								// invoke backgnd thread for Object list fill
void ObjectListThreadEnd( void );								// terminate Object list thread
void Generate3DThreadBegin( void );								// invoke thread for 3D object generation
void Generate3DThreadEnd( void );								// terminate and clear thread handle
void DistanceListThreadBegin( void );							// invoke thread for interobject distances
void DistanceListThreadEnd( void );								// terminate distances thread

															// program menu operations (program_menu.cpp)
void CmRestoreToolbar( void );									// create/destroy toolbar window
void CmStatusBar( void );										// create/destroy status bar
void CmDebugLog( void );										// turn on/off logging of subroutine calls
void CmDebugTimes( void );										// report status of execution timing variables
void CmDebug( void );											// just for programmer testing

															// series menu operations (series_menu.cpp)
void CmClose( void );											// Close series: free memory, gray menu bar
void Open( char *path, char *name );							// Open series given in BaseName and WorkingPath
void ValidOpen( char * seriesPath );							// validate the series path and open series if OK
void CmOpen( void );											// respond to a Series Open menu request
void CmNewSeries( void );										// create a new series file and open it
void CmSaveSeries( void );										// save series current state to file
void CmSeriesSource( void );									// display source in text editor
void CmSetOptions( void );										// open option settings dialog
void CmRenderSections( void );									// create fullsize bitmap file output
void CmExportLines( void );										// export the traces into DXF files
void CmExportTraceLists( void );								// output all trace lists to one file
void CmImportImages( void );									// creates sections from a set of image
void CmImportLines( void );									// creates sections from a set of imagess
void CmImportSeries( void );									// copy elements of foreign series into CurrSeries

															// section menu operations (section_menu.cpp)
bool FindSection( int &sectnum, int dir, char * filename );		// find filename of prev, same, or next (dir=-1,0,1)
Section * GetNextSectionBetween( int &last, int end );			// return section either from memory or disk
void PutSection( Section *section, bool saveit,					// save the section to disk if not Curr or Prev Section
								bool reRender, bool reDraw );	// and use re... to update display of memory sections
void FillSectionList( HWND list );								// refill section list from CurrSectionsInfo
bool GotoSection( int sectnum, char * sectionfile );			// display the section of sectionfile
bool Reload( char * sectionfile );								// reload current section from disk
void Previous( void );											// swap Prev and Curr Sections
void CmNewSection( void );										// create a new (empty) section
void CmPredecessor( void );										// page down a section
void CmSuccessor( void );										// page up a section
void CmSectionList( void );										// create/destroy section list window
void CmSaveSection( void );										// save section to disk
void CmSectionAttributes( void );								// dialog for modifying section attributes
void CmRestoreThumbnails( void );								// create/destroy thumbnail window
void CmBlend( void );											// switch to blend view mode
void CmCenter( void );											// center section image in window
void CmLastZoom( void );										// go back to last zoom settings
void CmActualPixels( void );									// make screen pixelsize = image pixelsize
void CmMagnification( void );									// set view to user-specified magnification
void CmLockSection( void );										// prevent/allow section movements
void CmTypeSectionTform( void );								// use type-in dialog to move section
void CmByCorrelation( void );									// shift to peak of cross-correlation
void CmUndoSection( void );										// undo last FrontView->section change
void CmRedoSection( void );										// restore section prior to last undo
void CmResetSection( void );									// go to first undo in undo LIFO
void CmRepeatAdjustment( void );								// repeat last adjustment on current section
void CmPropagateAdjustment( void );								// repeat last adjustment on range of sections
void CmRecord( void );											// record a sequence of mvmts
void CmCopyActiveNform( void );									// copy tform exactly
void CmCalibrateSections( void );								// message posted from CmCalibrateTraces
void Cm90Clkwise( void );										// 90 degrees clockwise
void Cm90Counterclkwise( void );								// 90 degrees counterclockwise
void CmRotate180( void );										// 180 degrees rotation
void CmFlipHorz( void );										// flip domain left for right
void CmFlipVert( void );										// flip domain top for bottom

															// domain menu operations (domain_menu.cpp)
void EnableDomainMenu( void );									// enable active domain items
void DisableDomainMenu( void );									// gray active domain menu items
void SelectDomainFromSection( Section *section, Transform *transform );
void CmNewDomainFile( void );									// create active domain from image file
void CmMergeFront( void );										// put active domain on top of section
void CmMergeRear( void );										// put active domain underneath rest of section
void CmRestoreContrast( void );									// restore the contrast of the domain
void CmDomainAttributes( void );								// display info about the image
void CmDomainList( void );										// list the domains in the section
void CmDeleteDomain( void );									// delete active domain from section
void CmDefaultDomain( void );									// replace domain boundary with image boundary
void CmClearDomainTransform( void );							// reset entire domain transform

															// trace menu operations (trace_menu.cpp)
void CmSelectAll( void );										// select all contours on section
void CmUnSelectAll( void );										// unselect all contours on section
void CmZoomSelected( void );									// zoom in to selected traces
void CmCutSelected( void );										// put contours in clipTransform, remove from section
void CmCopySelected( void );									// put contours into clipTransform 
void CmPasteSelected( void );									// put clipTransform contours into section
void CmPasteAttributes( void );									// use 1st clipboard ccontour to set attributes
void CmDeleteSelected( void );									// delete selected contours
void CmListTraces( void );										// display (or destroy) trace list for section
void FillTraceList( HWND list, Section *section );				// fill the trace list from section data
BOOL CALLBACK AttributesDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM);
void CmTraceAttributes( void );									// modify selected trace attributes
void CmTracePalette( void );									// display the palette of trace attributes
void CmCloseContour( void );									// close the selected contours
void CmAlignRigid( void );										// align trace centroids using translation and rotation
void CmAlignLinear( void );										// align with full affine transformation
void CmAlignDeform( void );										// align with xy coupling term
void CmAlignQuadratic( void );									// align with full quadratic binomial
void CmCalibrateTraces( void );									// set length of straight line contours
void CmMergeSelected( void );									// merge the selected traces
void CmReverseSelected( void );									// reverse orientation of selected traces
void CmSimplifySelected( void );								// simplify the selected traces
void CmSmoothSelected( void );									// smooth selected traces
void ShowSectionTrace( int sectnum, char *tracename );			// goto section and center tracename in main window
void CmFindTrace( void );										// find and zoom to trace

															// object menu operations (object_menu.cpp)
void CmObjectList( void );										// list window of 3D object in series
void CmRestore3DWindow( void );									// create/destroy openGL 3D window
void CmZList( void );											// create/destroy list of Z-Traces
void ResetSceneIcons( HWND list );								// refresh scene icons in list when change scene
void CmDistanceList( void );									// display interobject distances list

															// help menu operations (help_menu.cpp)
void CmHelpReconstruct( void );									// open general help for program
void CmHelpKeyTable( void );									// show keyboard shortcuts
void CmHelpMouse( void );										// show mouse actions
void CmHelpLicense( void );										// show license info dialog
void CmHelpAbout( void );										// show version info dialog
						
HWND MakeToolWindow( void );								// toolbar and tools operations (tools.cpp)
void SetToolCursor( HWND hWnd );								// set cursor based on current tool
void CmArrow( void );											// use Arrow Tool
void CmZoom( void );											// use Zoom Tool
void CmMagnify( void );											// use Magnify Tool
void CmDomain( void );											// use Domain Tool
void CmPoint( void );											// use Point Tool
void CmEllipse( void );											// use Ellipse Tool
void CmRectangle( void );										// use rectangle drawing tool
void CmLine( void );											// use Line Tool
void CmMultiLine( void );										// use MultiLine Tool
void CmPencil( void );											// use Pencil Tool
void CmCultiLine( void );										// use closed multiline Tool
void CmScissor( void );											// use tool for cutting traces
void CmZLine( void );											// use tool for drawing Z-Traces
void CmGrid( void );											// use tool for drawing grids
void CmWildfire( void );										// use region growing tool
void CmScalpel( void );											// use scalpel tool
void SwitchTool( int toolId );									// switch tool, cursor, and button
HCURSOR MakeStampCursor( Contour *contour );					// convert offset contour to cursor

HWND MakeThumbsWindow( void );								// thumbnail window (thumbnails.cpp)
HWND MakeOpenGLWindow( void );								// OpenGL 3D operations (opengl.cpp)
void UpdateSceneMenu( void );									// use CurrScene to update list of objects
															// boissonnat.cpp
void BoissonnatSurface( VRMLObject *vrmlobject );				// surface VRML Object from contours
void SlabSurface( VRMLObject *vrmlobject );

HWND MakePaletteWindow( void );								// trace palette window (palette.cpp)
void PushPaletteButton( int index );							// push the indexed palette contour button
void UnPushPaletteButtons( void );								// unpush all palette buttons
void UpdatePaletteButton( int index );							// redraw the bitmap for button index
void CreatePaletteButtons( HWND hWnd, int x, int y, Contours *contours, Buttons *buttons );
HBITMAP MakeContourBitmap( HWND hWnd, Contour *contour );		// convert pixel offset contour into a bitmap for button
void CmUseDefaultContour( int index );							// use stamp contour and attributes from default
void CmSetDefaultContour( int index );							// set stamp contour and default contour attributes

															// keyboard input operations (keyboard.cpp)
void CmAdjustContrast( HWND hWnd, double bfactor, double cfactor );
void CmToggleViews( HWND hWnd );								// move other view into CurrView
void CmMovement( HWND hWnd, WORD command );						// move all transforms of Front Section
void CmBackspace( void );										// backspace key, undo tracing point
void CmEscapeCurrentTool( void );								// escape current tool drawing operation
void CmPrecisionCursor( void );									// switch to/from precision cursor
void CmHideTraces( void );										// hide selected traces
void CmNextWindow( HWND currWnd );								// switch to next doubleing (or main) window
void CmGetClipboardAttributes(void);							// put clipboard attributes into drawing defaults
void CmTraceForward(void);			                            // enable automatic trace forward 
void CmTraceBackward(void);										// enable automatic trace backward
void DoAutoTrace(void);											// autotracing message function

															// mouse clicks and cursor movements (mouse.cpp)
void BeginDrag( void );											// generic cursor/window DC capture
void EndDrag( void );											// nonspecific end of a mouse drag
void LButtonDown( HWND hWnd, WPARAM, LPARAM lParam);			// tool operations for left mouse button
void RButtonDown( HWND hWnd, WPARAM, LPARAM lParam);			// for right mouse button
void MouseMove(HWND hWnd, WPARAM, LPARAM lParam);				// for mouse movements
void LButtonUp( HWND hWnd, WPARAM, LPARAM lParam );				// for release of left mouse button
void MButtonUp( HWND hWnd, WPARAM, LPARAM lParam );				// for middle mouse button
void RButtonUp( HWND hWnd, WPARAM, LPARAM lParam );				// for release of right mouse button

															// utility functions (utility.cpp)
void CenterDialog( HWND hwndOwner, HWND hwndDlg );				// center a child window in the client area
void SizeViewPorts( void );										// adjust viewports when window is resized
void ErrMsgOK( int msgno, const char *msgtxt );					// display error and wait for user OK
BOOL CALLBACK WaitUntilOK(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM);
BOOL CALLBACK WaitTimeout(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM);
void ErrMsgSplash( int msgno, const char *msgtxt );				// error message display w/o wait
void * GlobalAllocOrDie( DWORD num_bytes );						// global memory allocation
bool CanOverwrite( char *filename );							// ask user if can overwrite an existing file
void SplitPath( char *src, char *dir, char *file, char *ext );	// get extension, directory, and filename strings of src
void MakeLocalPath( char *wpath, char *ipath, char *localpath );// make ipath local relative to wpath 
void MakeAbsolutePath( char *adir, char *rdir, char *fulldir );	// combine a rel/abs dir with an abs one
void UpdateTitles( void );										// update the application title bar
void UpdateMenus( void );										// update menus based on front section attributes
void PaintViews( HDC hdc, RECT region );						// paint the region in hdc, regenerating views if needed
void InvalidateAllViews( void );								// flag front, back & previous views for rerendering & redrawing
void ScrollViews( void );										// shift section display in the main window
void UpdateBlendView( void );									// blend front and back views
void ClientRectLessStatusBar( RECT *r );						// return client area w/o status bar included
void ClientRectToScreen( HWND hWnd, RECT *r );					// convert client rectangle to screen rectangle
BOOL CALLBACK InputDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM);	// simple input dialog procedure
BOOL CALLBACK SectionRangeDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam); // ask for section range
int TestGBMfile( const char *fn, int *w, int *h, int *bpp, int *n ); // get image file params using GBM library
void ColorButton( HWND hWnd, Color color );						// apply color to button hWnd
void UpdateStatusBar( void );									// update the status bar info
void SaveList( HWND list, char *filename );						// save listview to a text file
void RemoveIllegalChars( char *s );								// legalize user input
bool MatchLimit( char *s, char *t );							// true if match upto '*' in second string
void DisplayListInfo( HWND list );								// report item count of listview
void Sort(int begin, int end, double arr[], double brr[]);		// ascending sort of arr, moving also brr
void svd(double **a, int m, int n, double w[], double **v);		// singular value decomposition
