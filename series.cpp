////////////////////////////////////////////////////////////////////////
//	This file contains the methods for the Series class
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
// -+- change: Added wildfire Stop limits to series object and series options to XML file i/o. 
// modified 11/12/04 by JCF (fiala@bu.edu)
// -+- change: Switched wildfire stop criteria to HSB instead of RGB.
// modified 3/07/05 by JCF (fiala@bu.edu)
// -+- change: Added ZTraceFromId for list operations.
// modified 4/29/05 by JCF (fiala@bu.edu)
// -+- change: Added traceStopWhen attribute to series to remember Wildfire option.
// modified 6/17/05 by JCF (fiala@bu.edu)
// -+- change: Added areaStopSize and areaStopPercent attributes to series for Wildfire options.
// modified 3/15/06 by JCF (fiala@bu.edu)
// -+- change: Modified to prevent crash when loading corrupted XML file.
// modified 3/29/06 by JCF (fiala@bu.edu)
// -+- change: fixed missing brightStopValue read from Series file
// modified 5/2/06 by JCF (fiala@bu.edu)
// -+- change: Added OffsetBetweenZTraces for future smoothing operation.
// modified 5/3/06 by JCF (fiala@bu.edu)
// -+- change: Added smoothingLength parameter to Series.
// modified 5/11/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added ContourMaskWidth parameter to Series.
// modified 5/18/06 by JCF (fiala@bu.edu)
// -+- change: Added code in Series(filename) constructor to create 20 palette entries if less
// modified 2/5/07 by JCF (fiala@bu.edu)
// -+- change: SaveSeriesContour() to use MAX_SIGDIGITS for z of z-traces for large section numbers.
// modified 3/15/07 by JCF (fiala@bu.edu)
// -+- change: modified SaveIfNeeded() to ask user when autosave is turned off

#include "reconstruct.h"

Series::Series()												// empty constructor
{
	int i;

	strcpy(units,"microns\0");
	index = 0;
	offset_x = 0.0;
	offset_y = 0.0;
	pixel_size = 0.00254;		// =25400/1000dpi/10000 (10,000x microfilm scanned at 1000dpi)
	defaultThickness = 0.05;
	zMidSection = false;
	autoSaveSeries = true;
	autoSaveSection = true;
	warnSaveSection = true;
	beepDeleting = true;
	beepPaging = true;
	hideTraces = false;
	unhideTraces = false;
	hideDomains = false;
	unhideDomains = false;
	useAbsolutePaths = false;
	thumbWidth = THUMB_WIDTH;
	thumbHeight = THUMB_HEIGHT;
	displayThumbContours = true;
	useFlipbookStyle = false;
	fitThumbSections = false;
	firstThumbSection = 1;
	lastThumbSection = MAX_INT32;
	skipSections = 1;
	flipRate = 5;
	useProxies = true;
	widthUseProxies = 2048;
	heightUseProxies = 1536;
	scaleProxies = 0.25;
	significantDigits = 6;
	defaultBorder = Color( 1.0, 0.0, 1.0 );
	defaultFill = Color( 1.0, 0.0, 1.0 );
	defaultMode = R2_MASKPEN;
	strcpy(defaultName,"domain$+");
	defaultComment[0] = 0;
	listSectionThickness = true;
	listDomainSource = true; listDomainPixelsize = true; listDomainLength = false; listDomainArea = false; listDomainMidpoint = false;
	listTraceComment = true; listTraceLength = false; listTraceArea = true; listTraceCentroid = false;
	listTraceExtent = false; listTraceZ = false; listTraceThickness = false;
	listObjectRange = true; listObjectCount = true; listObjectSurfarea = false; listObjectFlatarea = false; listObjectVolume = false;
	listZTraceNote = true; listZTraceRange = true; listZTraceLength = true;
	for (i=0; i<16; i++)
		{
		borderColors[i] = Color( 0.0, 0.0, 0.0 );
		fillColors[i] = Color( 0.0, 0.0, 0.0 );
		}
	x3Doffset = 0.0;
	y3Doffset = 0.0;
	z3Doffset = 0.0;
	max3Dconnection = -1.0;
	first3Dsection = 1;
	last3Dsection = MAX_INT32;
	type3Dobject = 0;
	upper3Dfaces = true;
	lower3Dfaces = true;
	faceNormals = false;
	vertexNormals = true;
	facets3D = 8;
	dim3Da = -1.0; dim3Db = -1.0; dim3Dc = -1.0;
	gridType = 0; gridXNumber = 1; gridYNumber = 1;
	gridXSize = 1.0; gridYSize = 1.0; gridXDistance = 1.0; gridYDistance = 1.0;
	hueStopWhen = 3; hueStopValue = 50;
	satStopWhen = 3; satStopValue = 50;
    brightStopWhen = 0; brightStopValue = 100;
	tracesStopWhen = false;
	areaStopPercent = 999.0;
	areaStopSize = 0.0;
	ContourMaskWidth = 0;
	smoothingLength = 7;
	Increment.transX = 1.0;
	Increment.transY = 1.0;
	CtrlIncrement.transX = 0.01;
	CtrlIncrement.transY = 0.01;
	ShiftIncrement.transX = 100.0;
	ShiftIncrement.transY = 100.0;
	Increment.scaleX = 1.01;
	Increment.scaleY = 1.01;
	CtrlIncrement.scaleX = 1.002;
	CtrlIncrement.scaleY = 1.002;
	ShiftIncrement.scaleX = 1.05; 
	ShiftIncrement.scaleY = 1.05;
	Increment.theta = 0.022;
	CtrlIncrement.theta = 0.0044; 
	ShiftIncrement.theta = 0.11; 
	Increment.slantX = 0.02; 
	Increment.slantY = 0.02;
	CtrlIncrement.slantX = 0.004;
	CtrlIncrement.slantY = 0.004;
	ShiftIncrement.slantX = 0.1;
	ShiftIncrement.slantY = 0.1;
	Increment.deformX = 0.001; 
	Increment.deformY = 0.001;
	CtrlIncrement.deformX = 0.0002;
	CtrlIncrement.deformY = 0.0002;
	ShiftIncrement.deformX = 0.005;
	ShiftIncrement.deformY = 0.005;
	contours = new Contours( *PointContours );			// copy default contours list (0-9)
	zcontours = NULL;
}

Series::Series( Series &copyfrom )								// copy constructor
{
	if ( copyfrom.contours ) contours = new Contours( *(copyfrom.contours) );
	else contours = new Contours( *PointContours );				// copy default contours list (0-9)
	if ( copyfrom.zcontours ) zcontours = new Contours( *(copyfrom.zcontours) );
	else zcontours = NULL;
	strcpy(units,copyfrom.units);
	index = copyfrom.index;
	pixel_size = copyfrom.pixel_size;
	offset_x = copyfrom.offset_x;
	offset_y = copyfrom.offset_y;
	defaultThickness = copyfrom.defaultThickness;
	zMidSection = copyfrom.zMidSection;
	autoSaveSeries = copyfrom.autoSaveSeries;
	autoSaveSection = copyfrom.autoSaveSection;
	warnSaveSection = copyfrom.warnSaveSection;
	beepDeleting = copyfrom.beepDeleting;
	beepPaging = copyfrom.beepPaging;
	hideTraces = copyfrom.hideTraces;
	unhideTraces = copyfrom.unhideTraces;
	hideDomains = copyfrom.hideDomains;
	unhideDomains = copyfrom.unhideDomains;
	useAbsolutePaths = copyfrom.useAbsolutePaths;
	thumbWidth = copyfrom.thumbWidth;
	thumbHeight = copyfrom.thumbHeight;
	displayThumbContours = copyfrom.displayThumbContours;
	useFlipbookStyle = copyfrom.useFlipbookStyle;
	fitThumbSections = copyfrom.fitThumbSections;
	firstThumbSection = copyfrom.firstThumbSection;
	lastThumbSection = copyfrom.lastThumbSection;
	skipSections = copyfrom.skipSections;
	flipRate = copyfrom.flipRate;
	useProxies = copyfrom.useProxies;
	widthUseProxies = copyfrom.widthUseProxies;
	heightUseProxies = copyfrom.heightUseProxies;
	scaleProxies = copyfrom.scaleProxies;
	significantDigits = copyfrom.significantDigits;
	defaultBorder = copyfrom.defaultBorder;
	defaultFill = copyfrom.defaultFill;
	defaultMode = copyfrom.defaultMode;
	strcpy(defaultName,copyfrom.defaultName);
	strcpy(defaultComment,copyfrom.defaultComment);
	listSectionThickness = copyfrom.listSectionThickness;
	listDomainSource = copyfrom.listDomainSource;
	listDomainPixelsize = copyfrom.listDomainPixelsize;
	listDomainLength = copyfrom.listDomainLength;
	listDomainArea = copyfrom.listDomainArea;
	listDomainMidpoint = copyfrom.listDomainMidpoint;
	listTraceComment = copyfrom.listTraceComment;
	listTraceLength = copyfrom.listTraceLength;
	listTraceArea = copyfrom.listTraceArea;
	listTraceCentroid = copyfrom.listTraceCentroid;
	listTraceExtent = copyfrom.listTraceExtent;
	listTraceZ = copyfrom.listTraceZ;
	listTraceThickness = copyfrom.listTraceThickness;
	listObjectRange = copyfrom.listObjectRange;
	listObjectCount = copyfrom.listObjectCount;
	listObjectSurfarea = copyfrom.listObjectSurfarea;
	listObjectFlatarea = copyfrom.listObjectFlatarea;
	listObjectVolume = copyfrom.listObjectVolume;
	listZTraceNote = copyfrom.listZTraceNote;
	listZTraceRange = copyfrom.listZTraceRange;
	listZTraceLength = copyfrom.listZTraceLength;
	for (int i=0; i<16; i++)
		{
		borderColors[i] = copyfrom.borderColors[i];
		fillColors[i] = copyfrom.fillColors[i];
		}
	x3Doffset = copyfrom.x3Doffset;
	y3Doffset = copyfrom.y3Doffset;
	z3Doffset = copyfrom.z3Doffset;
	max3Dconnection = copyfrom.max3Dconnection;
	first3Dsection = copyfrom.first3Dsection;
	last3Dsection = copyfrom.last3Dsection;
	type3Dobject = copyfrom.type3Dobject;
	upper3Dfaces = copyfrom.upper3Dfaces;
	lower3Dfaces = copyfrom.lower3Dfaces;
	faceNormals = copyfrom.faceNormals;
	vertexNormals = copyfrom.vertexNormals;
	facets3D = copyfrom.facets3D;
	dim3Da = copyfrom.dim3Da;
	dim3Db = copyfrom.dim3Db;
	dim3Dc = copyfrom.dim3Dc;
	gridType = copyfrom.gridType;
	gridXNumber = copyfrom.gridXNumber;
	gridYNumber = copyfrom.gridYNumber;
	gridXSize = copyfrom.gridXSize;
	gridYSize = copyfrom.gridYSize;
	gridXDistance = copyfrom.gridXDistance;
	gridYDistance = copyfrom.gridYDistance;
	hueStopWhen = copyfrom.hueStopWhen;
	hueStopValue = copyfrom.hueStopValue;
	satStopWhen = copyfrom.satStopWhen;
	satStopValue = copyfrom.satStopValue;
    brightStopWhen = copyfrom.brightStopWhen;
	brightStopValue = copyfrom.brightStopValue;
	tracesStopWhen = copyfrom.tracesStopWhen;
	areaStopPercent = copyfrom.areaStopPercent;
	areaStopSize = copyfrom.areaStopSize;
	ContourMaskWidth = copyfrom.ContourMaskWidth;
	smoothingLength = copyfrom.smoothingLength;
	Increment = copyfrom.Increment;
	CtrlIncrement = copyfrom.CtrlIncrement;
	ShiftIncrement = copyfrom.ShiftIncrement;
}

Series::~Series()												// destructor frees memory of transforms
{
	if ( contours ) delete contours;
	if ( zcontours ) delete zcontours;
}

void Series::SetDefaultAttributes( Contour *contour )
{
	contour->border = defaultBorder;			// set default color attributes
	contour->fill = defaultFill;
	contour->mode = defaultMode;				// but set (and parse) defaultName string elsewhere
	strcpy(contour->comment,defaultComment);
}
																// the following routines are local routines
																// designed to simplify loading from file

char * LoadSeriesContour( XML_DATA *xml, char *begin_contour, bool Z, Contour *contour )
{
	Point *p;													// load contour from XML data
	double x, y, z;
	char *begin_attr, *end_contour;

	begin_attr = xml->findString(begin_contour,"name=","/>");
	if ( begin_attr ) xml->getString( contour->name, begin_attr, MAX_CONTOUR_NAME );
	else strcpy( contour->name, "unknown" );
	begin_attr = xml->findString(begin_contour,"comment=","/>");
	if ( begin_attr ) xml->getString( contour->comment, begin_attr, MAX_COMMENT );
	begin_attr = xml->findString(begin_contour,"closed=",">");
	if ( begin_attr ) contour->closed = xml->getBool( begin_attr );
	else contour->closed = true;
	begin_attr = xml->findString(begin_contour,"border=",">");
	if ( begin_attr ) {
		begin_attr = xml->openQuote( begin_attr );
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->border.r = x;
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->border.g = x;
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->border.b = x;
		}
	begin_attr = xml->findString(begin_contour,"fill=",">");
	if ( begin_attr ) {
		begin_attr = xml->openQuote( begin_attr );
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->fill.r = x;
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->fill.g = x;
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->fill.b = x;
		}
	begin_attr = xml->findString(begin_contour,"mode=",">");
	if ( begin_attr ) contour->mode = xml->getInt( begin_attr );
	begin_attr = xml->findString(begin_contour,"points=","/>");
	if ( begin_attr )
		{
		contour->points = new Points();
		begin_attr = xml->openQuote( begin_attr );	// read and store contour points
		while ( begin_attr )
			{
			begin_attr = xml->getNextdouble( &x, begin_attr );
			if ( begin_attr )
				{
				begin_attr = xml->getNextdouble( &y, begin_attr );
				if ( begin_attr )
					{
					p = new Point();
					p->x = x;
					p->y = y;
					if ( Z )
						{
						begin_attr = xml->getNextdouble( &z, begin_attr );
						if ( begin_attr ) p->z = z;
						}
					contour->points->Add(p);
					}
				}
			}										// until reach end of list
		}
	if ( Z )
		end_contour = xml->findString(begin_contour,"/>","Contour");
	else end_contour = xml->findString(begin_contour,"/>","Contour");
	return( end_contour );
}

void LoadMvmtIncrement( XML_DATA *xml, char *begin_mvmt, Mvmt *increment )
{
	double f;
	char *begin_attr;
														// advance through quoted attribute values
	begin_attr = xml->openQuote( begin_mvmt );
	if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
	if ( begin_attr ) increment->theta = f;
	if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
	if ( begin_attr ) increment->transX = f;
	if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
	if ( begin_attr ) increment->transY = f;
	if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
	if ( begin_attr ) increment->scaleX = f;
	if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
	if ( begin_attr ) increment->scaleY = f;
	if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
	if ( begin_attr ) increment->slantX = f;
	if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
	if ( begin_attr ) increment->slantY = f;
	if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
	if ( begin_attr ) increment->deformX = f;
	if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
	if ( begin_attr ) increment->deformY = f;
}

Series::Series( char * seriesfile )									// Constructor for Series from file!
{														
	XML_DATA *xml;
	int i;
	double f, r, g, b;
	Contour *contour, *ncontour;
	char *begin_element, *begin_attr, *end_element, *begin_contours;
	char txt[100];

	xml = new XML_DATA( seriesfile );								// read entire file into memory
	contours = NULL;
	zcontours = NULL;
	strcpy(units,"microns\0");
	index = 0;														// set defaults for all options
	offset_x = 0.0;													// in case file doesn't specify them
	offset_y = 0.0;
	pixel_size = 0.00254;
	defaultThickness = 0.05;
	zMidSection = false;
	autoSaveSeries = true;
	autoSaveSection = true;
	warnSaveSection = true;
	beepDeleting = true;
	beepPaging = true;
	hideTraces = false;
	unhideTraces = false;
	hideDomains = false;
	unhideDomains = false;
	useAbsolutePaths = false;
	thumbWidth = THUMB_WIDTH;
	thumbHeight = THUMB_HEIGHT;
	displayThumbContours = true;
	useFlipbookStyle = false;
	fitThumbSections = false;
	firstThumbSection = 1;
	lastThumbSection = MAX_INT32;
	skipSections = 1;
	flipRate = 5;
	useProxies = true;
	widthUseProxies = 2048;
	heightUseProxies = 1536;
	scaleProxies = 0.25;
	significantDigits = 6;
	defaultBorder = Color( 1.0, 0.0, 1.0 );
	defaultFill = Color( 1.0, 0.0, 1.0 );
	defaultMode = R2_MASKPEN;
	strcpy(defaultName,"domain$+");
	defaultComment[0] = 0;
	listSectionThickness = true;
	listDomainSource = true; listDomainPixelsize = true; listDomainLength = false; listDomainArea = false; listDomainMidpoint = false;
	listTraceComment = true; listTraceLength = false; listTraceArea = true; listTraceCentroid = false;
	listTraceExtent = false; listTraceZ = false; listTraceThickness = false;
	listObjectRange = true; listObjectCount = true; listObjectSurfarea = false; listObjectFlatarea = false; listObjectVolume = false;
	listZTraceNote = true; listZTraceRange = true; listZTraceLength = true;
	for (i=0; i<16; i++)
		{
		borderColors[i] = Color( 0.0, 0.0, 0.0 );
		fillColors[i] = Color( 0.0, 0.0, 0.0 );
		}
	x3Doffset = 0.0;
	y3Doffset = 0.0;
	z3Doffset = 0.0;
	max3Dconnection = -1.0;
	first3Dsection = 1;
	last3Dsection = MAX_INT32;
	type3Dobject = 0;
	upper3Dfaces = true;
	lower3Dfaces = true;
	faceNormals = false;
	vertexNormals = true;
	facets3D = 8;
	dim3Da = -1.0; dim3Db = -1.0; dim3Dc = -1.0;
	gridType = 0; gridXNumber = 1; gridYNumber = 1;
	gridXSize = 1.0; gridYSize = 1.0; gridXDistance = 1.0; gridYDistance = 1.0;
	hueStopWhen = 3; hueStopValue = 50;
	satStopWhen = 3; satStopValue = 50;
    brightStopWhen = 0; brightStopValue = 100;
	tracesStopWhen = false;
	areaStopPercent = 999.0;
	areaStopSize = 0.0;
	smoothingLength = 7;
	Increment.transX = 1.0;
	Increment.transY = 1.0;
	CtrlIncrement.transX = 0.01;
	CtrlIncrement.transY = 0.01;
	ShiftIncrement.transX = 100.0;
	ShiftIncrement.transY = 100.0;
	Increment.scaleX = 1.01;
	Increment.scaleY = 1.01;
	CtrlIncrement.scaleX = 1.002;
	CtrlIncrement.scaleY = 1.002;
	ShiftIncrement.scaleX = 1.05; 
	ShiftIncrement.scaleY = 1.05;
	Increment.theta = 0.022;
	CtrlIncrement.theta = 0.0044; 
	ShiftIncrement.theta = 0.11; 
	Increment.slantX = 0.02; 
	Increment.slantY = 0.02;
	CtrlIncrement.slantX = 0.004;
	CtrlIncrement.slantY = 0.004;
	ShiftIncrement.slantX = 0.1;
	ShiftIncrement.slantY = 0.1;
	Increment.deformX = 0.001; 
	Increment.deformY = 0.001;
	CtrlIncrement.deformX = 0.0002;
	CtrlIncrement.deformY = 0.0002;
	ShiftIncrement.deformX = 0.005;
	ShiftIncrement.deformY = 0.005;

	if ( xml->data == NULL )			// if no file data, then return with index=-1
		{
		index = -1;
		delete xml;
		return;
		}

	begin_element = xml->findString(xml->data,"<Series","</Series");	// search for start of series
	if ( begin_element )	
		{																// fill series from attributes
		begin_attr = xml->findString(begin_element,"index=",">");
		if ( begin_attr ) index = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"viewport=",">");		// get section position and zoom factors
		if ( begin_attr )
			{
			begin_attr = xml->openQuote( begin_attr );
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr) offset_x = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr) offset_y = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr) pixel_size = f;
			}															// get remaining option attributes
		begin_attr = xml->findString(begin_element,"units=",">");
		if ( begin_attr ) xml->getString( units, begin_attr, MAX_UNITS_STRING );
		begin_attr = xml->findString(begin_element,"autoSaveSeries=",">");
		if ( begin_attr ) autoSaveSeries = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"autoSaveSection=",">");
		if ( begin_attr ) autoSaveSection = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"warnSaveSection=",">");
		if ( begin_attr ) warnSaveSection = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"beepDeleting=",">");
		if ( begin_attr ) beepDeleting = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"beepPaging=",">");
		if ( begin_attr ) beepPaging = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"hideTraces=",">");
		if ( begin_attr ) hideTraces = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"unhideTraces=",">");
		if ( begin_attr ) unhideTraces = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"hideDomains=",">");
		if ( begin_attr ) hideDomains = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"unhideDomains=",">");
		if ( begin_attr ) unhideDomains = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"useAbsolutePaths=",">");
		if ( begin_attr ) useAbsolutePaths = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"defaultThickness=",">");
		if ( begin_attr ) defaultThickness = xml->getdouble( begin_attr );
		begin_attr = xml->findString(begin_element,"zMidSection=",">");
		if ( begin_attr ) zMidSection = xml->getBool( begin_attr );
																		// thumbnail options
		begin_attr = xml->findString(begin_element,"thumbWidth=",">");
		if ( begin_attr ) thumbWidth = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"thumbHeight=",">");
		if ( begin_attr ) thumbHeight = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"fitThumbSections=",">");
		if ( begin_attr ) fitThumbSections = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"firstThumbSection=",">");
		if ( begin_attr ) firstThumbSection = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"lastThumbSection=",">");
		if ( begin_attr ) lastThumbSection = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"skipSections=",">");
		if ( begin_attr ) skipSections = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"displayThumbContours=",">");
		if ( begin_attr ) displayThumbContours = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"useFlipbookStyle=",">");
		if ( begin_attr ) useFlipbookStyle = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"flipRate=",">");
		if ( begin_attr ) flipRate = xml->getInt( begin_attr );
																		// image proxy options
		begin_attr = xml->findString(begin_element,"useProxies=",">");
		if ( begin_attr ) useProxies = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"widthUseProxies=",">");
		if ( begin_attr ) widthUseProxies = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"heightUseProxies=",">");
		if ( begin_attr ) heightUseProxies = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"scaleProxies=",">");
		if ( begin_attr ) scaleProxies = xml->getdouble( begin_attr );
																		// default contour options
		begin_attr = xml->findString(begin_element,"significantDigits=",">");
		if ( begin_attr ) significantDigits = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"defaultBorder=",">");
		if ( begin_attr )
			{
			begin_attr = xml->openQuote( begin_attr );
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) defaultBorder.r = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) defaultBorder.g = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) defaultBorder.b = f;
			}
		begin_attr = xml->findString(begin_element,"defaultFill=",">");
		if ( begin_attr )
			{
			begin_attr = xml->openQuote( begin_attr );
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) defaultFill.r = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) defaultFill.g = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) defaultFill.b = f;
			}
		begin_attr = xml->findString(begin_element,"defaultMode=",">");
		if ( begin_attr ) defaultMode = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"defaultName=",">");
		if ( begin_attr ) xml->getString( defaultName, begin_attr, MAX_CONTOUR_NAME );
		begin_attr = xml->findString(begin_element,"defaultComment=",">");
		if ( begin_attr ) xml->getString( defaultComment, begin_attr, MAX_COMMENT );

																		// list field flags
		begin_attr = xml->findString(begin_element,"listSectionThickness=",">");
		if ( begin_attr ) listSectionThickness = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listDomainSource=",">");
		if ( begin_attr ) listDomainSource = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listDomainPixelsize=",">");
		if ( begin_attr ) listDomainPixelsize = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listDomainLength=",">");
		if ( begin_attr ) listDomainLength = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listDomainArea=",">");
		if ( begin_attr ) listDomainArea = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listDomainMidpoint=",">");
		if ( begin_attr ) listDomainMidpoint = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listTraceComment=",">");
		if ( begin_attr ) listTraceComment = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listTraceLength=",">");
		if ( begin_attr ) listTraceLength = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listTraceArea=",">");
		if ( begin_attr ) listTraceArea = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listTraceCentroid=",">");
		if ( begin_attr ) listTraceCentroid = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listTraceExtent=",">");
		if ( begin_attr ) listTraceExtent = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listTraceZ=",">");
		if ( begin_attr ) listTraceZ = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listTraceThickness=",">");
		if ( begin_attr ) listTraceThickness = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listObjectRange=",">");
		if ( begin_attr ) listObjectRange = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listObjectCount=",">");
		if ( begin_attr ) listObjectCount = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listObjectSurfarea=",">");
		if ( begin_attr ) listObjectSurfarea = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listObjectFlatarea=",">");
		if ( begin_attr ) listObjectFlatarea = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listObjectVolume=",">");
		if ( begin_attr ) listObjectVolume = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listZTraceNote=",">");
		if ( begin_attr ) listZTraceNote = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listZTraceRange=",">");
		if ( begin_attr ) listZTraceRange = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"listZTraceLength=",">");
		if ( begin_attr ) listZTraceLength = xml->getBool( begin_attr );

																		// custom border colors
		begin_attr = xml->findString(begin_element,"borderColors=","/>");
		if ( begin_attr )
			{
			begin_attr = xml->openQuote( begin_attr );
			i = 0;
			while ( begin_attr && (i<16) )
				{
				begin_attr = xml->getNextdouble( &r, begin_attr );
				if ( begin_attr )
					{
					begin_attr = xml->getNextdouble( &g, begin_attr );
					if ( begin_attr )
						{
						begin_attr = xml->getNextdouble( &b, begin_attr );
						if ( begin_attr )
							{
							borderColors[i] = Color( r, g, b );
							i++;
							}
						}
					}
				}
			}															// custom fill colors
		begin_attr = xml->findString(begin_element,"fillColors=","/>");
		if ( begin_attr )
			{
			begin_attr = xml->openQuote( begin_attr );
			i = 0;
			while ( begin_attr && (i<16) )
				{
				begin_attr = xml->getNextdouble( &r, begin_attr );
				if ( begin_attr )
					{
					begin_attr = xml->getNextdouble( &g, begin_attr );
					if ( begin_attr )
						{
						begin_attr = xml->getNextdouble( &b, begin_attr );
						if ( begin_attr )
							{
							fillColors[i] = Color( r, g, b );
							i++;
							}
						}
					}
				}
			}

																		// 3D object generation options
		begin_attr = xml->findString(begin_element,"offset3D=",">");
		if ( begin_attr )
			{
			begin_attr = xml->openQuote( begin_attr );
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) x3Doffset = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) y3Doffset = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) z3Doffset = f;
			}
		begin_attr = xml->findString(begin_element,"type3Dobject=",">");
		if ( begin_attr ) type3Dobject = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"first3Dsection=",">");
		if ( begin_attr ) first3Dsection = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"last3Dsection=",">");
		if ( begin_attr ) last3Dsection = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"max3Dconnection=",">");
		if ( begin_attr ) max3Dconnection = xml->getdouble( begin_attr );
		begin_attr = xml->findString(begin_element,"upper3Dfaces=",">");
		if ( begin_attr ) upper3Dfaces = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"lower3Dfaces=",">");
		if ( begin_attr ) lower3Dfaces = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"faceNormals=",">");
		if ( begin_attr ) faceNormals = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"vertexNormals=",">");
		if ( begin_attr ) vertexNormals = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"facets3D=",">");
		if ( begin_attr ) facets3D = xml->getInt( begin_attr );

		begin_attr = xml->findString(begin_element,"dim3D=",">");
		if ( begin_attr )
			{
			begin_attr = xml->openQuote( begin_attr );
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) dim3Da = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) dim3Db = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) dim3Dc = f;
			}		
																		// grid drawing parameters
		begin_attr = xml->findString(begin_element,"gridType=",">");
		if ( begin_attr ) gridType = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"gridSize=",">");
		if ( begin_attr )
			{
			begin_attr = xml->openQuote( begin_attr );
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) gridXSize = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) gridYSize = f;
			}
		begin_attr = xml->findString(begin_element,"gridDistance=",">");
		if ( begin_attr )
			{
			begin_attr = xml->openQuote( begin_attr );
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) gridXDistance = f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) gridYDistance = f;
			}
		begin_attr = xml->findString(begin_element,"gridNumber=",">");
		if ( begin_attr )
			{
			begin_attr = xml->openQuote( begin_attr );
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) gridXNumber = (int)f;
			if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
			if ( begin_attr ) gridYNumber = (int)f;
			}
																	// extract wildfire limit info
		begin_attr = xml->findString(begin_element,"hueStopWhen=",">");
		if ( begin_attr ) hueStopWhen = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"hueStopValue=",">");
		if ( begin_attr ) hueStopValue = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"satStopWhen=",">");
		if ( begin_attr ) satStopWhen = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"satStopValue=",">");
		if ( begin_attr ) satStopValue = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"brightStopWhen=",">");
		if ( begin_attr ) brightStopWhen = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"brightStopValue=",">");
		if ( begin_attr ) brightStopValue = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"tracesStopWhen=",">");
		if ( begin_attr ) tracesStopWhen = xml->getBool( begin_attr );
		begin_attr = xml->findString(begin_element,"areaStopPercent=",">");
		if ( begin_attr ) areaStopPercent = xml->getdouble( begin_attr );
		begin_attr = xml->findString(begin_element,"areaStopSize=",">");
		if ( begin_attr ) areaStopSize = xml->getdouble( begin_attr );
		begin_attr = xml->findString(begin_element,"ContourMaskWidth=",">");
		if ( begin_attr)  ContourMaskWidth = xml->getInt( begin_attr);			// added
		begin_attr = xml->findString(begin_element,"smoothingLength=",">");
		if ( begin_attr ) smoothingLength = xml->getInt( begin_attr );

		begin_attr = xml->findString(begin_element,"mvmtIncrement=",">");
		if ( begin_attr ) LoadMvmtIncrement( xml, begin_attr, &Increment );
		begin_attr = xml->findString(begin_element,"ctrlIncrement=",">");
		if ( begin_attr ) LoadMvmtIncrement( xml, begin_attr, &CtrlIncrement );
		begin_attr = xml->findString(begin_element,"shiftIncrement=",">");
		if ( begin_attr ) LoadMvmtIncrement( xml, begin_attr, &ShiftIncrement );

												// end series attributes, read default contours
		begin_contours = begin_element;
		begin_element = xml->findString(begin_contours,"<Contour","</Series");
		if ( begin_element )
			{
			contours = new Contours();
			while ( begin_element )												// for each contour found
				{
				contour = new Contour();										// create default contour
				end_element = LoadSeriesContour( xml, begin_element, false, contour );
				contours->Add(contour);											// add contour to default list
				begin_element = xml->findString(end_element,"<Contour","</Series");
				}
			}

		begin_element = xml->findString(begin_contours,"<ZContour","</Series");
		if ( begin_element )
			{
			zcontours = new Contours();
			while ( begin_element )												// for each contour found
				{
				contour = new Contour();										// create z contour
				end_element = LoadSeriesContour( xml, begin_element, true, contour );
				zcontours->Add(contour);										// add contour to z list
				begin_element = xml->findString(end_element,"<ZContour","</Series");
				}
			}

		}		

		
	else				// if no <Series data element, then signal failure by setting index = -1
		{
		ErrMsgOK( ERRMSG_READ_FAILED, seriesfile );
		index = -1;
		}

	delete xml;																		// free xml memory

	if ( !contours ) contours = new Contours( *PointContours );			// if nothing add default contours list (0-9)
	if ( contours->Number() < 11 )										// if still less than 20, add again
		{
		contour = PointContours->first;
		while ( contour )
			{
			ncontour = new Contour( *contour );							// copy each PointContour
			contours->Add( ncontour );									// and add to palette contours list
			contour = contour->next;
			}
		}
}


bool WriteBool( HANDLE hFile, char *attr, bool value )		// routine for output boolean
{
	DWORD byteswritten;
	char line[MAX_PATH];

	sprintf(line,"\t%s=\"",attr);
	if ( value ) strcat(line,"true\"\r\n"); else strcat(line,"false\"\r\n");
	return ( WriteFile(hFile, line, strlen(line), &byteswritten, NULL) );
}

bool SaveSeriesContour( HANDLE hFile, Contour *contour, bool Z )
{
	Point *p;
	DWORD byteswritten;
	char line[MAX_PATH];
	bool error = false;

	if ( Z )													// open element according to type
		sprintf(line,"<ZContour name=\"%s\"", contour->name);	// output name
	else
		sprintf(line,"<Contour name=\"%s\"", contour->name);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
	if ( strlen(contour->comment) )
		{														// output comment if exists
		sprintf(line," comment=\"%s\"", contour->comment);
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
		}
	if ( contour->closed ) sprintf(line," closed=\"true\"");	// output attributes
	else sprintf(line," closed=\"false\"");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
	sprintf(line," border=\"%.3f %.3f %.3f\" fill=\"%.3f %.3f %.3f\" mode=\"%d\"",
					contour->border.r, contour->border.g, contour->border.b,
						contour->fill.r, contour->fill.g, contour->fill.b, contour->mode );
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
	if ( contour->points )
		{														// output points
		sprintf(line,"\r\n points=\"");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
		p = contour->points->first;
		while ( p != NULL )
		  {											// output at most 'Precision' significant digits
		  if ( Z )									// except z which needs more digits for max section index
			sprintf(line,"%1.*g %1.*g %1.*g,\r\n\t",Precision,p->x,Precision,p->y,MAX_SIGDIGITS,p->z);
		  else
			sprintf(line,"%1.*g %1.*g,\r\n\t",Precision,p->x,Precision,p->y);

		  if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
		  p = p->next;
		  }
		sprintf(line,"\"");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
		}
	sprintf(line,"/>\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;

	return error;
}

void Series::Save( char * seriesfile )					// save seriesfile
{													
	HANDLE hFile;
	DWORD byteswritten;
	int i;
	Contour *contour;
	bool ErrorOccurred = false;
	char line[MAX_PATH], errmsg[1024];
																	// attempt to open file using Win32 call
 
	hFile = CreateFile( seriesfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		{															// fail, format error string
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							errmsg, 512, NULL );
		strcat(errmsg, seriesfile );
		ErrMsgOK( ERRMSG_WRITE_FAILED, errmsg );
		return;
		}																	// write XML header info
	
	sprintf(line,"<?xml version=\"1.0\"?>\r\n<!DOCTYPE Series SYSTEM \"series.dtd\">\r\n\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
																			// write Series opening
	sprintf(line,"<Series index=\"%d\" viewport=\"%1.*g %1.*g %1.*g\"\r\n",
													index, Precision, offset_x, Precision, offset_y, Precision, pixel_size);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	sprintf(line,"\tunits=\"%s\"\r\n", units);								// write units
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
																				// write save options
	if ( !WriteBool( hFile, "autoSaveSeries", autoSaveSeries ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "autoSaveSection", autoSaveSection ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "warnSaveSection", warnSaveSection ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "beepDeleting", beepDeleting ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "beepPaging", beepPaging ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "hideTraces", hideTraces ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "unhideTraces", unhideTraces ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "hideDomains", hideDomains ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "unhideDomains", unhideDomains ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "useAbsolutePaths", useAbsolutePaths ) ) ErrorOccurred = true;
	sprintf(line,"\tdefaultThickness=\"%1.*g\"\r\n", Precision, defaultThickness);		// default section thickness
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "zMidSection", zMidSection ) ) ErrorOccurred = true;
																			// write thumbnail options
	sprintf(line,"\tthumbWidth=\"%d\"\r\n", thumbWidth);				
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tthumbHeight=\"%d\"\r\n", thumbHeight);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "fitThumbSections", fitThumbSections ) ) ErrorOccurred = true;
	sprintf(line,"\tfirstThumbSection=\"%d\"\r\n", firstThumbSection);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tlastThumbSection=\"%d\"\r\n", lastThumbSection);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tskipSections=\"%d\"\r\n", skipSections);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "displayThumbContours", displayThumbContours ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "useFlipbookStyle", useFlipbookStyle ) ) ErrorOccurred = true;
	sprintf(line,"\tflipRate=\"%d\"\r\n", flipRate);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
																			// write proxy options
	if ( !WriteBool( hFile, "useProxies", useProxies ) ) ErrorOccurred = true;
	sprintf(line,"\twidthUseProxies=\"%d\"\r\n", widthUseProxies);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\theightUseProxies=\"%d\"\r\n", heightUseProxies);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tscaleProxies=\"%1.*g\"\r\n", Precision, scaleProxies);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
																			// default contour values
	sprintf(line,"\tsignificantDigits=\"%d\"\r\n", significantDigits);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tdefaultBorder=\"%.3f %.3f %.3f\"\r\n", defaultBorder.r, defaultBorder.g, defaultBorder.b);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tdefaultFill=\"%.3f %.3f %.3f\"\r\n", defaultFill.r, defaultFill.g, defaultFill.b);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tdefaultMode=\"%d\"\r\n", defaultMode);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tdefaultName=\"%s\"\r\n", defaultName);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tdefaultComment=\"%s\"\r\n", defaultComment);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	if ( !WriteBool( hFile, "listSectionThickness", listSectionThickness ) ) ErrorOccurred = true;	// list field flags
	if ( !WriteBool( hFile, "listDomainSource", listDomainSource ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listDomainPixelsize", listDomainPixelsize ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listDomainLength", listDomainLength ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listDomainArea", listDomainArea ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listDomainMidpoint", listDomainMidpoint) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listTraceComment", listTraceComment ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listTraceLength", listTraceLength ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listTraceArea", listTraceArea ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listTraceCentroid", listTraceCentroid ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listTraceExtent", listTraceExtent ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listTraceZ", listTraceZ ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listTraceThickness", listTraceThickness ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listObjectRange", listObjectRange ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listObjectCount", listObjectCount ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listObjectSurfarea", listObjectSurfarea ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listObjectFlatarea", listObjectFlatarea ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listObjectVolume", listObjectVolume ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listZTraceNote", listZTraceNote ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listZTraceRange", listZTraceRange ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "listZTraceLength", listZTraceLength ) ) ErrorOccurred = true;
																			// custom colors
	sprintf(line,"\tborderColors=\"");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	for (i=0; i<16; i++)
		{
		sprintf(line,"%.3f %.3f %.3f,\r\n\t\t", borderColors[i].r, borderColors[i].g, borderColors[i].b);
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
		}
	sprintf(line,"\"\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tfillColors=\"");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	for (i=0; i<16; i++)
		{
		sprintf(line,"%.3f %.3f %.3f,\r\n\t\t", fillColors[i].r, fillColors[i].g, fillColors[i].b);
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
		}
	sprintf(line,"\"\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

																		// 3D object generation options
	sprintf(line,"\toffset3D=\"%1.*g %1.*g %1.*g\"\r\n", Precision, x3Doffset, Precision, y3Doffset, Precision, z3Doffset );
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\ttype3Dobject=\"%d\"\r\n", type3Dobject);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tfirst3Dsection=\"%d\"\r\n", first3Dsection);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tlast3Dsection=\"%d\"\r\n", last3Dsection);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tmax3Dconnection=\"%1.*g\"\r\n", Precision, max3Dconnection);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "upper3Dfaces", upper3Dfaces ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "lower3Dfaces", lower3Dfaces ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "faceNormals", faceNormals ) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "vertexNormals", vertexNormals ) ) ErrorOccurred = true;
	sprintf(line,"\tfacets3D=\"%d\"\r\n", facets3D);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tdim3D=\"%1.*g %1.*g %1.*g\"\r\n", Precision, dim3Da, Precision, dim3Db, Precision, dim3Dc);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tgridType=\"%d\"\r\n", gridType);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tgridSize=\"%1.*g %1.*g\"\r\n", Precision, gridXSize, Precision, gridYSize);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tgridDistance=\"%1.*g %1.*g\"\r\n", Precision, gridXDistance, Precision, gridYDistance);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tgridNumber=\"%d %d\"\r\n", gridXNumber, gridYNumber);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
																			// wildfire limits
	sprintf(line,"\thueStopWhen=\"%d\"\r\n", hueStopWhen);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\thueStopValue=\"%d\"\r\n", hueStopValue);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tsatStopWhen=\"%d\"\r\n", satStopWhen);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tsatStopValue=\"%d\"\r\n", satStopValue);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tbrightStopWhen=\"%d\"\r\n", brightStopWhen);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tbrightStopValue=\"%d\"\r\n", brightStopValue);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	if ( !WriteBool( hFile, "tracesStopWhen", tracesStopWhen ) ) ErrorOccurred = true;
	sprintf(line,"\tareaStopPercent=\"%1.*g\"\r\n", Precision, areaStopPercent);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tareaStopSize=\"%1.*g\"\r\n", Precision, areaStopSize);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tContourMaskWidth=\"%d\"\r\n", ContourMaskWidth);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tsmoothingLength=\"%d\"\r\n", smoothingLength);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
																			// write mvmt increments
	sprintf(line,"\tmvmtIncrement=\"%1.*g %1.*g %1.*g %1.*g %1.*g %1.*g %1.*g %1.*g %1.*g\"\r\n",
					Precision,Increment.theta,Precision,Increment.transX,Precision,Increment.transY,
					Precision,Increment.scaleX,Precision,Increment.scaleY,Precision,Increment.slantX,
					Precision,Increment.slantY,Precision,Increment.deformX,Precision,Increment.deformY );
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tctrlIncrement=\"%1.*g %1.*g %1.*g %1.*g %1.*g %1.*g %1.*g %1.*g %1.*g\"\r\n",
					Precision,CtrlIncrement.theta,Precision,CtrlIncrement.transX,Precision,CtrlIncrement.transY,
					Precision,CtrlIncrement.scaleX,Precision,CtrlIncrement.scaleY,Precision,CtrlIncrement.slantX,
					Precision,CtrlIncrement.slantY,Precision,CtrlIncrement.deformX,Precision,CtrlIncrement.deformY );
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	sprintf(line,"\tshiftIncrement=\"%1.*g %1.*g %1.*g %1.*g %1.*g %1.*g %1.*g %1.*g %1.*g\"\r\n",
					Precision,ShiftIncrement.theta,Precision,ShiftIncrement.transX,Precision,ShiftIncrement.transY,
					Precision,ShiftIncrement.scaleX,Precision,ShiftIncrement.scaleY,Precision,ShiftIncrement.slantX,
					Precision,ShiftIncrement.slantY,Precision,ShiftIncrement.deformX,Precision,ShiftIncrement.deformY );
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
	

	sprintf(line,"\t>\r\n");											// end of series attributes
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	if ( contours )														// write out default contours
		{
		contour = contours->first;
		while ( contour )												// output is in pixels so
			{															// precision is zero
			ErrorOccurred = SaveSeriesContour( hFile, contour, false );
			contour = contour->next;
			}
		}

	if ( zcontours )													// write out z contours
		{
		contour = zcontours->first;
		while ( contour )
			{
			ErrorOccurred = SaveSeriesContour( hFile, contour, true );
			contour = contour->next;
			}
		}
																			// close Series element
	sprintf(line,"</Series>");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	CloseHandle( hFile );													// close file

	if ( ErrorOccurred ) {
		ErrMsgOK( ERRMSG_WRITE_FAILED, seriesfile );
		}
}

void Series::SaveIfNeeded( void )					// check to save Series before deleting
{
	char filename[MAX_PATH], msgtxt[100];
	sprintf(filename,"%s%s.ser",WorkingPath,BaseName);
	if ( autoSaveSeries ) Save( filename );					// save automatically or
	else													// ask user if wants to save
		{
		sprintf(msgtxt,"Save z-traces and series options?");
		if ( MessageBox(appWnd,msgtxt,"WARNING",MB_YESNO|MB_ICONWARNING) == IDYES ) Save( filename );
		}
}

void Series::AddZContour( Contour *contour )		// add contour to zcontour list
{
	if ( !zcontours ) zcontours = new Contours();
	zcontours->Add( contour );
}

double Series::ZLength( Contour *contour )			// compute and return z-contour length
{
	Point  *p, *q;
	double dx, dy, dz, pz, qz;
	double length;
	
	length = 0.0;
	if ( contour->points )
		{
		p = contour->points->first;						// sum length contribution from all segments
		if ( p )
			{
			pz = CurrSectionsInfo->ZDistance( (int)p->z, zMidSection );
			q = p->next;
			}
		while ( p && q )
			{
			dx = q->x - p->x;
			dy = q->y - p->y;
			qz = CurrSectionsInfo->ZDistance( (int)q->z, zMidSection );
			dz = qz - pz;
			length += sqrt(dx*dx+dy*dy+dz*dz);
			p = q;
			pz = qz;
			q = q->next;
			}
		}
	return( length );
}

Contour * Series::ZMatch( char *name )
{
	Contour *zcontour;
	zcontour = NULL;

	if ( zcontours )
		{
		zcontour = zcontours->first;
		while ( zcontour )
			if ( strcmp(zcontour->name,name) ) zcontour = zcontour->next;
			else break;
		}
	return(zcontour);
}

Contour * Series::ZTraceFromId( int id )			// locate contour by Id and return ptr to it
{
	Contour *zcontour;

	zcontour = NULL;
	if ( zcontours )									// check all zcontours
		{
		zcontour = zcontours->first;
		while ( zcontour )								// stop search if found					
			if ( zcontour->Id == id ) break;
			else zcontour = zcontour->next;				// otherwise do next contour in list
		}
	return zcontour;
}

void Series::RenumberZTraces( int fromsect, int tosect )
{
	Contour *zcontour;
	Point *p;

	if ( zcontours )							// check all contours for fromsect
		{
		zcontour = zcontours->first;
		while ( zcontour )
			{
			if ( zcontour->points )
				{
				p = zcontour->points->first;		// check all points for fromsect
				while ( p )
					{
					if ( ((int)p->z) == fromsect ) p->z = (double)tosect;
					p = p->next;
					}
				}
			zcontour = zcontour->next;
			}
		}
}
												// compute the x,y offset between 2 zcontours Z1 and Z2
												// at a particular z-distance given by z

Point Series::OffsetBetweenZTraces( int sectnum, Contour *Z1, Contour *Z2 )
{
	Point offset, *p1,*p2;
//	char txt[200];
												// return zero offset in cases where offset is not meaningful
	offset.x = 0; offset.y = 0; offset.z = 0;

	if ( Z1 && Z1 )								// the zcontours must exist
		{
		p1 = Z1->points->first;
		while ( p1 )							// find section in z-trace
			{
			if ( p1->z == sectnum ) break;
			p1 = p1->next;
			}
		p2 = Z2->points->first;
		while ( p2 )							// find section in second z-trace
			{
			if ( p2->z == sectnum ) break;
			p2 = p2->next;
			}
		if ( p1 && p2 )							// found a common section in each
			{
			offset.x = p2->x - p1->x;			// calculate offset between z traces
			offset.y = p2->y - p1->y;
			}
		}

	return offset;
}
