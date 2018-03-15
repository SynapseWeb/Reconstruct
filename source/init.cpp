/////////////////////////////////////////////////////////////////////////////
//	This file contains the routines for initializing the application
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
// modified 2/03/05 by JCF (fiala@bu.edu)
// -+- change: initial AutoShrinkBack flag variable.
// modified 4/21/05 by Ju Lu (julu@fas.harvard.edu)
// -+- change: AutoTrace flag added. SrcContour initiated
// modified 4/29/05 by JCF (fiala@bu.edu)
// -+- change: Only changed the order for clarity.
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change: Modified InitCurrSeries to use new MinPixelsize() section method.
// modified 4/28/06 by JCF (fiala@bu.edu)
// -+- change: Added openGL_ortho=false for to start scene in perspective mode.
// modified 5/2/06 by JCF (fiala@bu.edu)
// -+- change: Added Offset variables for future smoothing operation.
// modified 5/11/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added ContourMaskWidth for mask out a strip along existing contour as implemented 
//	in adib->MaskOutBorder
// modified 6/12/06 by JCF (fiala@bu.edu)
// -+- change: Added debug logging file handle.
// modified 6/16/06 by JCF (fiala@bu.edu)
// -+- change: Removed commented debugLogFile create to program_menu.cpp
// modified 6/19/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added AutoAdjustThreshold flag
// modified 6/22/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added SrcHistogram and CurrHistogram
// modified 7/2/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added automaticTraces and MultiAutoTrace flag
// modified 11/15/06 by JCF (fiala@bu.edu)
// -+- change: Removed automaticTraces and MultiAutoTrace flag
// -+- change: Remove SrcContour as this is no longer used.
// -+- change: Added FindString for remembering find trace requests.
// modified 11/21/06 by JCF (fiala@bu.edu)
// -+- change: Added PaintingViews flag to prevent WM_PAINT overlaps.
// modified 5/30/07 by JCF (fiala@bu.edu)
// -+- change: cleaned up some extraneous variables in Init()

#include "reconstruct.h"

void Init( HWND hWnd )				// Note: init gets called before window is created so need to pass hWnd
{
	int i;
	Contour *contour;
	Point *p;
														// debugging variables/flags
	totalTime1 = 0;
	nTime1 = 0;
	totalTime2 = 0;
	nTime2 = 0;
	totalTime3 = 0;
	nTime3 = 0;
	debugit = 0;
	debugLogFile = INVALID_HANDLE_VALUE;				// disable debug logging
	
	srand(GetTickCount());			// randomize seed of random number generator

	appWnd = hWnd;					// remember main Window handle
	WindowWasMaximized = false;

	InitCommonControls();			// common controls used: PropertySheet, ListView, ImageList, HotKey, ColorDialog
	
	AbortRender = false;
	AbortThumbs = false;
	AbortObject = false;
	AbortDistances = false;
	Abort3D = false;
	hRenderThread = NULL;			// clear thread handles while not active
	hThumbsThread = NULL;
	hObjectListThread = NULL;
	hDistanceListThread = NULL;
	hGenerate3DThread = NULL;

	strcpy(BaseName,"aSeries\0");
	strcpy(FindString,"\0");
	ExpandEnvironmentStrings("%HOMEPATH%",WorkingPath,MAX_PATH);
	SkipAllMissing = false;
	HideAllDomains = false;
	CurrSeries = NULL;
	CurrSectionsInfo = NULL;
	CurrSection = NULL;
	CurrView = NULL;
	PrevSection = NULL;
	PrevView = NULL;
	DomainSection = NULL;
	DomainView = NULL;
	FrontView = NULL;
	BackView = NULL;
	BlendView = NULL;
	CurrContour = NULL;
	CurrDomain = NULL;
	ClipboardTransform = NULL;

	toolbarWindow = NULL;								// tool window variables
	ToolButtons = NULL;									// buttons are created when window is created
	highlightedToolButton = NULL;						// none highlighted initially
	CurrentTool = ARROW_TOOL;
	RToolActive = false;
	LToolActive = false;
	ToolContour = NULL;
	EditContour = NULL;
	statusbarWindow = NULL;
	dragPen = NULL;

	CmRestoreToolbar();									// create toolbar and statusbar windows...
	CmStatusBar();

	openGLWindow = NULL;								// ...but not openGL...
	openGL_LToolActive = false;
	openGL_RToolActive = false;
	openGL_ortho = false;
	openGLanimated = true;
	CurrScene = NULL;
	distanceWindow = NULL;
	zTraceWindow = NULL;
	objectWindow = NULL;
	CurrObjects = NULL;
	InterObjectDistances = NULL;
	sectionWindow = NULL;								// ...or other doubleing windows or dialogs
	thumbnailWindow = NULL;
	ThumbButtons = NULL;
	PassbackButtons = NULL;
	domainWindow = NULL;
	traceWindow = NULL;
	CurrContours = NULL;
	paletteWindow = NULL;
	PaletteButtons = NULL;
	DefaultPaletteButtons = NULL;
	highlightedPaletteButton = NULL;
	PaintingViews = false;
	ImagePixelSize = 0.00254;							// more dialog elements defaults
	ImageBrightness = 0.0;
	ImageContrast = 1.0;
	RenderWindow = false;
	RenderJPEG = false;
	RenderFill = false;
	RenderTraces = false;
	JPEGquality = 80;
	CopyFiles = true;									// initialize custom colors (will load these later from Series)
	for (i=0; i<16; i++)
		CustomColors[i] = RGB( 0, 0, 0 );
	GlobalId = 0;
	Precision = 6;
	LastOptionTab = 0;
	CalibrationScope = APPLY_TRACES;
														// use fullsize buttons at 120 dpi
	appDC = GetDC( appWnd );
	ButtonSize = BUTTON_SIZE*GetDeviceCaps( appDC, LOGPIXELSX )/120;
	ReleaseDC( appWnd, appDC );

	ScrollOccurred = false;
	Scrolling = false;
	AutoSimplify = true;
	AutoShrinkBack = false;
	UsePrecisionCursor = false;
	AutoTrace = 0;
	AutoAdjustThreshold = false;
	
	SimplifyResolution = -ImagePixelSize;
	ApplyZOffset3D = false;
	OffsetZTrace1 = NULL;
	OffsetZTrace2 = NULL;
														// load lock/unlock bitmaps for status bar
	lockBitmap = LoadBitmap(appInstance, "LockBitmap");
	unlockBitmap = LoadBitmap(appInstance, "UnlockBitmap");

														// keyboard movement options
	LastAdjustment = NULL;
	Recording = NULL;
	UseDeformKeys = false;
												// set strings for list row limits to allow all names
	strcpy(limitSectionList,"*\0");
	strcpy(limitDomainList,"*\0");
	strcpy(limitTraceList,"*\0");
	strcpy(limitObjectList,"*\0");
	strcpy(limitLeftDistanceList,"*\0");
	strcpy(limitRightDistanceList,"*\0");
	strcpy(limitZTraceList,"*\0");

	PointContours = new Contours();				// set pixel offset values for stamp tool contours
	contour = new Contour();
	PointContours->Add( contour );
	sprintf(contour->name,"a$+");						// orange
	contour->mode = R2_COPYPEN;
	contour->simplified = true;
	contour->border = Color( 1.0, 0.5, 0.0 );
	contour->fill = Color( 1.0, 0.5, 0.0 );
	contour->points = new Points();						// hexagonal shape
	int hexagon[] = { -3, 1, -3, -1, -1, -3, 1, -3, 3, -1, 3, 1, 1, 3, -1, 3 };
	for ( i=0; i<16; i=i+2 )
		{
		p = new Point((double)hexagon[i],(double)hexagon[i+1],0.0);
		contour->points->Add(p);
		}

	contour = new Contour();
	PointContours->Add( contour );
	sprintf(contour->name,"b$+");						// purple
	contour->mode = R2_COPYPEN;
	contour->simplified = true;
	contour->border = Color( 0.5, 0.0, 1.0 );
	contour->fill = Color( 0.5, 0.0, 1.0 );
	contour->points = new Points();						// 8-star shape
	int star[] = { -4, 4, -1, 2, 0, 5, 1, 2, 4, 4, 2, 1, 5, 0, 2, -1, 4, -4, 1, -2, 0, -5, -1, -2, -4, -4, -2, -1, -5, 0, -2, 1 };
	for ( i=0; i<32; i=i+2 )
		{
		p = new Point((double)star[i],(double)star[i+1],0.0);
		contour->points->Add(p);
		}
	contour->Reverse();									// make for clockwise creation

	contour = new Contour();
	PointContours->Add( contour );
	sprintf(contour->name,"pink$+");					// pink
	contour->mode = -R2_COPYPEN;
	contour->simplified = true;
	contour->border = Color( 1.0, 0.0, 0.5 );
	contour->fill = Color( 1.0, 0.0, 0.5 );
	contour->points = new Points();						// trianglar shape
	int triangle[] = { -6, -6, 6, -6, 0, 5 };
	for ( i=0; i<6; i=i+2 )
		{
		p = new Point((double)triangle[i],(double)triangle[i+1],0.0);
		contour->points->Add(p);
		}

	contour = new Contour();
	PointContours->Add( contour );
	sprintf(contour->name,"X$+");
	contour->mode = -R2_COPYPEN;
	contour->simplified = true;
	contour->border = Color( 1.0, 0.0, 0.0 );			// red
	contour->fill = Color( 1.0, 0.0, 0.0 );
	contour->points = new Points();						// x shape
	int x[] = { -7, 7, -2, 0, -7, -7, -4, -7, 0, -1, 4, -7, 7, -7, 2, 0, 7, 7, 4, 7, 0, 1, -4, 7 };
	for ( i=0; i<24; i=i+2 )
		{
		p = new Point((double)x[i],(double)x[i+1],0.0);
		contour->points->Add(p);
		}

	contour = new Contour();
	PointContours->Add( contour );
	sprintf(contour->name,"yellow$+");					// yellow
	contour->mode = -R2_COPYPEN;
	contour->simplified = true;
	contour->border = Color( 1.0, 1.0, 0.0 );
	contour->fill = Color( 1.0, 1.0, 0.0 );
	contour->points = new Points();						// square annulus shape
	int square[] = { -4, 4, -5, 5, 5, 5, 5, -5, -5, -5, -5, 4, -4, 3, -4, -4, 4, -4, 4, 4 };
	for ( i=0; i<20; i=i+2 )
		{
		p = new Point((double)square[i],(double)square[i+1],0.0);
		contour->points->Add(p);
		}
	contour->Reverse();									// make for clockwise creation
	contour->Scale( 2.0 );								// double in size

	contour = new Contour();
	PointContours->Add( contour );
	sprintf(contour->name,"blue$+");					// blue
	contour->mode = R2_MASKPEN;
	contour->simplified = true;
	contour->border = Color( 0.0, 0.0, 1.0 );
	contour->fill = Color( 0.0, 0.0, 1.0 );
	contour->points = new Points();						// diamond shape
	int diamond[] = { 0, 7, -7, 0, 0, -7, 7, 0 };
	for ( i=0; i<8; i=i+2 )
		{
		p = new Point((double)diamond[i],(double)diamond[i+1],0.0);
		contour->points->Add(p);
		}

	contour = new Contour();
	PointContours->Add( contour );
	sprintf(contour->name,"magenta$+");
	contour->mode = R2_MASKPEN;
	contour->simplified = true;
	contour->border = Color( 1.0, 0.0, 1.0 );			// magenta
	contour->fill = Color( 1.0, 0.0, 1.0 );
	contour->points = new Points();						// larger hexagonal shape
	for ( i=0; i<16; i=i+2 )
		{
		p = new Point((double)hexagon[i],(double)hexagon[i+1],0.0);
		contour->points->Add(p);
		}
	contour->Scale( 2.0 );								// double in size

	contour = new Contour();
	PointContours->Add( contour );
	sprintf(contour->name,"red$+");						// red
	contour->mode = R2_MASKPEN;
	contour->simplified = true;
	contour->border = Color( 1.0, 0.0, 0.0 );
	contour->fill = Color( 1.0, 0.0, 0.0 );
	contour->points = new Points();						// curved arrow shape
	int curved[] = { -1, -4, -4, -2, -2, -2, -2, 0, -1, 2, 1, 4, 2, 2, 4, 1, 1, 0, 0, -1, 0, -2, 2, -2 };
	for ( i=0; i<24; i=i+2 )
		{
		p = new Point((double)curved[i],(double)curved[i+1],0.0);
		contour->points->Add(p);
		}
	contour->Reverse();									// make for clockwise creation
	contour->Scale( 3.0 );								// triple in size

	contour = new Contour();
	PointContours->Add( contour );
	sprintf(contour->name,"green$+");
	contour->mode = R2_MASKPEN;
	contour->simplified = true;
	contour->border = Color( 0.0, 1.0, 0.0 );			// yellow
	contour->fill = Color( 0.0, 1.0, 0.0 );
	contour->points = new Points();						// large cross shape
	int cross[] = { -3, 1, -3, -1, -1, -1, -1, -3, 1, -3, 1, -1, 3, -1, 3, 1, 1, 1, 1, 3, -1, 3, -1, 1 };
	for ( i=0; i<24; i=i+2 )
		{
		p = new Point((double)cross[i],(double)cross[i+1],0.0);
		contour->points->Add(p);
		}
	contour->Scale( 4.0 );								// double in size

	contour = new Contour();
	PointContours->Add( contour );
	sprintf(contour->name,"cyan$+");
	contour->mode = R2_MASKPEN;
	contour->simplified = true;
	contour->border = Color( 0.0, 1.0, 1.0 );			// green
	contour->fill = Color( 0.0, 1.0, 1.0 );
	contour->points = new Points();						// large arrow shape
	int arrow[] = { 0, 3, 1, 2, -3, -2, -2, -3, 2, 1, 3, 0, 3, 3 };
	for ( i=0; i<14; i=i+2 )
		{
		p = new Point((double)arrow[i],(double)arrow[i+1],0.0);
		contour->points->Add(p);
		}
	contour->Scale( 4.0 );								// quadruple in size

														// finally setup stamp tool contour and cursor
	StampContour = new Contour( *(PointContours->first) );
	StampCursor = MakeStampCursor( StampContour );
}

void InitCurrSeries( void )			// after CurrSeries is created, initialize params
{
	Section *section;
	double pixel_size;
	Point min, max;
	int sectnum;
	char filename[MAX_PATH];
	HCURSOR cur;

	cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );		// show hourglass, file access could be slow

	LastZoom.pixel_size = CurrSeries->pixel_size;		// set last zoom params
	LastZoom.offset_x = CurrSeries->offset_x;
	LastZoom.offset_y = CurrSeries->offset_y;
	if ( LastAdjustment ) delete LastAdjustment;		// clear last adjustment
	LastAdjustment = NULL;
	strcpy(limitSectionList,"*\0");						// and any list limitations
	strcpy(limitDomainList,"*\0");
	strcpy(limitTraceList,"*\0");
	strcpy(limitObjectList,"*\0");
	strcpy(limitLeftDistanceList,"*\0");
	strcpy(limitRightDistanceList,"*\0");
	strcpy(limitZTraceList,"*\0");
	
	CurrSectionsInfo = new SectionsInfo();				// create list of sections

	ImagePixelSize = 0.00254;							// look for default pixel_size in sections
	sectnum = -1;										// start with section 0
	while ( FindSection( sectnum, 1, filename ) )		// load section from disk
		{
		section = new Section( filename );				// read section data
			
		if ( section->HasImage() ) ImagePixelSize = section->MinPixelsize();

		CurrSectionsInfo->AddSection(section, filename );// add to section list

		delete section;
		}												// find next section

//	GlobalId = 0;										// take opportunity to reset contour id numbers?
	SimplifyResolution = -ImagePixelSize;				// restore default simplifcation params
	ApplyZOffset3D = false;
	OffsetZTrace1 = NULL;
	OffsetZTrace2 = NULL;
	Precision = CurrSeries->significantDigits;			// set the output Precision for program
	AutoSimplify = true;
	SkipAllMissing = false;
	HideAllDomains = false;
	SetCursor( cur );									// restore cursor
}
