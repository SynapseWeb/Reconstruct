////////////////////////////////////////////////////////////////////////////////
// Header of global variable declarations for RECONSTRUCT
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
// -+- change: Added wildfireStopTypes string declaration.
// modified 2/03/05 by JCF (fiala@bu.edu)
// -+- change: Added AutoShrinkBack flag.
// modified 4/21/05 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added SrcContour, flag AutoTrace
// modified 4/29/05 by JCF (fiala@bu.edu)
// -+- change: Only changed the order of some definitions for clarity.
// modified 4/28/06 by JCF (fiala@bu.edu)
// -+- change: Added openGL_ortho logical for switching scene perspective.
// modified 5/2/06 by JCF (fiala@bu.edu)
// -+- change: Added Offset variables for future smoothing operation.
// modified 6/5/06 by JCF (fiala@bu.edu)
// -+- change: Added debugLogFile for logging operations leading to crash.
// modified 6/14/06 by JCF (fiala@bu.edu)
// -+- change: Removed NumberOfImage and NumberOfProxies from globals.
// modified 6/19/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added AutoAdjustThreshold flag.
// modified 6/22/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added SrcHistogram and CurrHistogram
// modified 7/2/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added MultiAutoTrace flag and automaticTraces variable.
// modified 11/15/06 by JCF (fiala@bu.edu)
// -+- change: Remove MultiAutotrace and Autoamtic traces since no longer needed.
// -+- change: Remove SrcContour, SrcHistogram as this is no longer used.
// -+- change: Added FindString for remembering find trace requests.
// modified 11/21/06 by JCF (fiala@bu.edu)
// -+- change: Comments. Added PaintingViews flag.
// modified 7/16/07 by JCF (fiala@bu.edu)
// -+- change: Added dialog variables for color channel modifications.
 

extern unsigned int startTime1, totalTime1, nTime1;			// debugging variables
extern unsigned int startTime2, totalTime2, nTime2;
extern unsigned int startTime3, totalTime3, nTime3;
extern int debugit;
extern HANDLE debugLogFile;
															// multithreading variables
extern bool AbortRender, AbortThumbs, AbortObject, AbortDistances, Abort3D;
extern HANDLE hRenderThread, hThumbsThread;						// handles to rendering threads
extern HANDLE hObjectListThread, hDistanceListThread;			// handle to threads for object lists filling
extern HANDLE hGenerate3DThread;								// thread for backgrounding 3D object creation
extern HWND renderProgressDlg, generateProgressDlg, distanceProgressDlg;

															// GDI global variables
extern HWND appWnd;												// store the HANDLE to the main application window
extern HINSTANCE appInstance;									// HANDLE to the instance of the application
extern HDC appDC;												// HANDLE to the current device context
															// handles to toolbar and floating windows
extern HWND toolbarWindow, statusbarWindow, paletteWindow;
extern HWND sectionWindow, thumbnailWindow, domainWindow, traceWindow;
extern HWND openGLWindow, objectWindow, zTraceWindow, distanceWindow;
extern HWND importList, sectionList, domainList, traceList;
extern HWND objectList, zTraceList, distanceList;
extern HWND CurrButtonWnd;
extern HCURSOR StampCursor;										// cursor created from stamp tool contour
extern HBITMAP lockBitmap, unlockBitmap;						// handles to the lock/unlock images

														// series data-related objects
extern VRMLScene *CurrScene;									// holds current 3D scene for openGL window
extern ViewPort *CurrView, *PrevView, *BlendView, *DomainView;	// bitmaps for rendering sections for display
extern ViewPort *FrontView, *BackView;							// global ptrs to view order
extern bool SkipAllMissing, HideAllDomains;						// flags for image loading options
extern Series *CurrSeries;										// holds series file data
extern VRMLObjects *CurrObjects;								// list of 3D objects in series
extern InterObjects *InterObjectDistances;						// list of interobject distances
extern SectionsInfo *CurrSectionsInfo;							// list of sections and their attributes
extern Section *CurrSection, *PrevSection, *DomainSection;		// holds data for sections being edited
extern Transform *CurrDomain;									// domain being edited
extern Contours *CurrContours, *PointContours;					// lists of contours
extern Contour *CurrContour, *ToolContour, *EditContour;		// specific contours selected or being edited
extern Contour *StampContour;									// pixel offset values for stamp tool contour

extern Transform *ClipboardTransform;							// for cut & paste operations

															// tool/toolbar variables that need to persist
extern Buttons *ToolButtons;									// list of tool buttons
extern Button *highlightedToolButton;							// button that has the highlight
extern int CurrentTool;											// currently selected tool
extern bool LToolActive, RToolActive;							// flags for tool activity (see mouse.cpp)
extern RECT LToolRect, RToolRect;								// active tool drag region
extern ZoomState LastZoom;										// remember last zoom
extern HPEN dragPen;											// colored pen created during mouse dragging
extern int EditContourSection;									// marker for where EditContour editing started

															// persistent thumbnail window variables
extern Buttons *ThumbButtons, *PassbackButtons;
extern int xThumbsPos, yThumbsPos, xThumbsOver, yThumbsOver, flippage;

extern Buttons *PaletteButtons, *DefaultPaletteButtons;		// persistant palette button list variables
extern Button *highlightedPaletteButton;
extern Contour *DefaultPaletteContour;
extern int ButtonSize;										// default bitmap size scaled to screen dpi

															// persistent variables for OpenGL window
extern POINT openGLlast;										// remember last cursor position
extern bool openGLanimated, openGL_LToolActive, openGL_RToolActive, openGL_ortho;

															// global variables used for passing dialog and thread values
extern int FirstSection, LastSection;
extern char SeriesName[MAX_BASE], FindString[MAX_CONTOUR_NAME];
extern char InputDlgString[MAX_PATH], InputDlgValue[64], InputDlgName[64];
extern double ImagePixelSize, ImageBrightness, ImageContrast;	// hold the default domain values
extern int ImageRed, ImageGreen, ImageBlue;
extern bool RenderWindow, RenderJPEG, RenderTraces, RenderFill;	// render dialog variables
extern double FullPixelSize, UsePixelSize, UseThickness;
extern double XLength, YLength, XOffset, YOffset;				// for passing to render threads
extern Mvmt Movement;
extern Nform *LastAdjustment, *Recording;
extern int JPEGquality;
extern COLORREF CustomColors[16];
extern Contour *DlgContour;										// contour attribute dialog variables
extern bool ChangeClosed, ChangeHidden, ChangeSelected, ChangeSimplified;
extern int CalibrationScope;
extern VRMLObject *DlgObject;
extern VRMLObjects *selectedObjects;							// used to collect list items for adding to scene

															// persistent global state flags
extern int LastOptionTab;										// remember last Series Option tab user accessed
extern bool PaintingViews;										// prevent dialogs from Regenerating views during Regenerate
extern bool UseDeformKeys, SetSectionThickness, SetSectionPixelSize, LockAllSections, UnlockAllSections;
extern bool WindowWasMaximized, CopyFiles, DeleteFiles;			// series option flags
extern int GlobalId;											// id numbers of traces assigned as read from file
extern int Precision;											// current number of significant digits to output
extern int ScrollX, ScrollY;									// scrolling by mouse moves
extern bool ScrollOccurred, Scrolling, AutoSimplify, AutoShrinkBack, UsePrecisionCursor;
extern int AutoTrace;                                           // +1: trace forward; -1: trace backward; 0: stop

															// Series Options not saved in series file
extern double SimplifyResolution;								// option for pixel size to use when simplifying
extern bool ApplyZOffset3D;										// section-by-section shifts of 3D 
extern Contour *OffsetZTrace1, *OffsetZTrace2;					//   using difference of these Z-traces
extern bool AutoAdjustThreshold;								// flag for automatic adjustment of threshold in autotrace		
																// strings for list row limits
extern char limitSectionList[MAX_CONTOUR_NAME], limitDomainList[MAX_CONTOUR_NAME];
extern char limitTraceList[MAX_CONTOUR_NAME], limitObjectList[MAX_CONTOUR_NAME], limitZTraceList[MAX_CONTOUR_NAME];
extern char limitLeftDistanceList[MAX_CONTOUR_NAME], limitRightDistanceList[MAX_CONTOUR_NAME];

extern char wildfireStopTypes[];							// persistent string information
extern char fileformats[];										// store the known ADib filetypes
extern char BaseName[MAX_BASE];									// basename of series
extern char WorkingPath[MAX_PATH];								// directory strings
extern char ImagesPath[MAX_PATH];
