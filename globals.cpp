//////////////////////////////////////////////////////////////////////////////////
// Actual global variables for RECONSTRUCT
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
// -+- change: Added wildfireStopTypes string for text output in dialog.
// modified 2/03/05 by JCF (fiala@bu.edu)
// -+- change: Added AutoShrinkBack flag variable.
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
// -+- change: Added MultiAutoTrace flag and automaticTraces variable
// modified 11/15/06 by JCF (fiala@bu.edu)
// -+- change: Remove MultiAutotrace and Autoamtic traces since no longer needed.
// -+- change: Remove SrcContour, SrcHistogram as this is no longer used.
// -+- change: Added FindString for remembering find trace requests.
// modified 11/21/06 by JCF (fiala@bu.edu)
// -+- change: Comments. Added PaintingViews flag.
// modified 4/20/07 by JCF (fiala@bu.edu)
// -+- change: Changed Stop Criterion "equals" to "does not equal".
// modified 7/16/07 by JCF (fiala@bu.edu)
// -+- change: Added dialog variables for color channel modifications.

#include "reconstruct.h"


unsigned int startTime1, totalTime1, nTime1;			// debugging variables
unsigned int startTime2, totalTime2, nTime2;
unsigned int startTime3, totalTime3, nTime3;
int debugit;
HANDLE debugLogFile;
													// multithreading variables
bool AbortRender, AbortThumbs, AbortObject, AbortDistances, Abort3D;
HANDLE hRenderThread, hThumbsThread;					// handles to rendering threads
HANDLE hObjectListThread, hDistanceListThread;			// handle to threads for object lists filling
HANDLE hGenerate3DThread;								// thread for backgrounding 3D object creation
HWND renderProgressDlg, generateProgressDlg, distanceProgressDlg;

													// GDI global variables
HWND appWnd;											// store the HANDLE to the main application window
HINSTANCE appInstance;									// HANDLE to the instance of the application
HDC appDC;												// HANDLE to the current device context
														// handles to toolbar and floating windows
HWND toolbarWindow, statusbarWindow, paletteWindow;
HWND sectionWindow, thumbnailWindow, domainWindow, traceWindow;
HWND openGLWindow, objectWindow, zTraceWindow, distanceWindow;
HWND importList, sectionList, domainList, traceList;
HWND objectList, zTraceList, distanceList;
HWND CurrButtonWnd;
HCURSOR StampCursor;									// cursor created from stamp tool contour
HBITMAP lockBitmap, unlockBitmap;						// handles to the lock/unlock images

													// series data-related objects
VRMLScene *CurrScene;									// holds current 3D scene for openGL window
ViewPort *CurrView, *PrevView, *BlendView, *DomainView;	// bitmaps for rendering sections for display
ViewPort *FrontView, *BackView;							// global ptrs to view order
bool SkipAllMissing, HideAllDomains;					// flags for image loading options
Series *CurrSeries;										// holds series file data
VRMLObjects *CurrObjects;								// list of 3D objects in series
InterObjects *InterObjectDistances;						// list of interobject distances
SectionsInfo *CurrSectionsInfo;							// list of sections and their attributes
Section *CurrSection, *PrevSection, *DomainSection;		// holds data for sections being edited
Transform *CurrDomain;									// domain being edited
Contours *CurrContours, *PointContours;					// lists of contours
Contour *CurrContour, *ToolContour, *EditContour;		// specific contours being edited
Contour *StampContour;									// pixel offset values for stamp tool contour

Transform *ClipboardTransform;							// for cut & paste operations

													// tool/toolbar variables that need to persist
Buttons *ToolButtons;									// list of tool buttons
Button *highlightedToolButton;							// button that has the highlight
int CurrentTool;										// currently selected tool
bool LToolActive, RToolActive;							// flags for tool activity (see mouse.cpp)
RECT LToolRect, RToolRect;								// active tool drag region
ZoomState LastZoom;										// remember last zoom
HPEN dragPen;											// colored pen created during mouse dragging
int EditContourSection;									// marker for where EditContour editing started

													// persistent thumbnail window variables
Buttons *ThumbButtons, *PassbackButtons;
int xThumbsPos, yThumbsPos, xThumbsOver, yThumbsOver, flippage;

Buttons *PaletteButtons, *DefaultPaletteButtons;	// persistant palette button list variables
Button *highlightedPaletteButton;
Contour *DefaultPaletteContour;
int ButtonSize;											// default bitmap size scaled to screen dpi

													// persistent variables for OpenGL window
POINT openGLlast;										// remember last cursor position
bool openGLanimated, openGL_LToolActive, openGL_RToolActive, openGL_ortho;

												// global variables used for passing dialog and thread values
int FirstSection, LastSection;
char SeriesName[MAX_BASE], FindString[MAX_CONTOUR_NAME];
char InputDlgString[MAX_PATH], InputDlgValue[64], InputDlgName[64];
double ImagePixelSize, ImageBrightness, ImageContrast;	// hold the default domain values
int ImageRed, ImageGreen, ImageBlue;
bool RenderWindow, RenderJPEG, RenderTraces, RenderFill;// render dialog variables
double FullPixelSize, UsePixelSize, UseThickness;
double XLength, YLength, XOffset, YOffset;				// for passing to render threads
Mvmt Movement;
Nform *LastAdjustment, *Recording;
int JPEGquality;
COLORREF CustomColors[16];
Contour *DlgContour;									// contour attribute dialog variables
bool ChangeClosed, ChangeHidden, ChangeSelected, ChangeSimplified;
int CalibrationScope;
VRMLObject *DlgObject;
VRMLObjects *selectedObjects;							// used to collect list items for adding to scene

													// persistent global state flags
int LastOptionTab;										// remember last Series Option tab user accessed
bool PaintingViews;										// prevent dialogs from Regenerating views during Regenerate
bool UseDeformKeys, SetSectionThickness, SetSectionPixelSize, LockAllSections, UnlockAllSections;
bool WindowWasMaximized, CopyFiles, DeleteFiles;		// series option flags
int GlobalId;											// id numbers of traces assigned as read from file
int Precision;											// current number of significant digits to output
int ScrollX, ScrollY;									// scrolling by mouse moves
bool ScrollOccurred, Scrolling, AutoSimplify, AutoShrinkBack, UsePrecisionCursor;
int AutoTrace;                                           // +1: trace forward; -1: trace backward; 0: stop

													// Series Options not saved in series file
double SimplifyResolution;								// option for pixel size to use when simplifying
bool ApplyZOffset3D;									// section-by-section shifts of 3D 
Contour *OffsetZTrace1, *OffsetZTrace2;					//   using difference of these Z-traces
bool AutoAdjustThreshold;								// flag for automatic adjustment of threshold in autotrace		
														// strings for list row limits
char limitSectionList[MAX_CONTOUR_NAME], limitDomainList[MAX_CONTOUR_NAME];
char limitTraceList[MAX_CONTOUR_NAME], limitObjectList[MAX_CONTOUR_NAME], limitZTraceList[MAX_CONTOUR_NAME];
char limitLeftDistanceList[MAX_CONTOUR_NAME], limitRightDistanceList[MAX_CONTOUR_NAME];

char wildfireStopTypes[] = " is less than  does not equal is greater than  differs by   ";
char fileformats[] = "??? BMP GIF PCX TIFFTGA LBM VID PGM PPM KPS IAX XBM SPR PSG GEM CVP JPEG";
char BaseName[MAX_BASE];							// will fill remaining strings when open series
char WorkingPath[MAX_PATH];
char ImagesPath[MAX_PATH];
