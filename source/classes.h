//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Object class definitions for the RECONSTRUCT application.
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
// -+- change: Added ADib and Viewport methods, and Series attributes, to support Wildfire region growing.
// modified 2/03/05 by JCF (fiala@bu.edu)
// -+- change: Added Contour method Inteior() to allow shrink back of Exterior trace.
// -+- change: Added Contour method XOR() to allow cutting of contour at intersections with a bisector.
// modified 2/08/05 by JCF (fiala@bu.edu)
// -+- change: Added copy constructor to VRMLObject.
// modified 3/07/05 by JCF (fiala@bu.edu)
// -+- change: Added ZTraceFromId for series ztrace list operations.
// modified 4/21/05 by JL (julu@fas.harvard.edu)
// -+- change: Added MaskADibWithContours(ADib* region) to ViewPort class.
// modified 4/27/05 by JCF (fiala@bu.edu)
// -+- change: Added IsSection() to CurrSectionsInfo methods.
// modified 4/29/05 by JCF (fiala@bu.edu)
// -+- change: Added method AddDXFLines() to Section for import of DXF traces.
//             Modified Viewport and ADib to cleanup 1.0.4.4 bug.
// modified 6/8/05 by JCF (fiala@bu.edu)
// -+- change: Added WildfireRegion method to viewport.
// modified 6/17/05 by JCF (fiala@bu.edu)
// -+- change: Added min_size param to WildfireRegion() to allow user to avoid small regions if desired.
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change: Added pixelsize and min, max components to SectionInfo. Added HasImage and related methods to Section.
// modified 7/14/05 by JCF (fiala@bu.edu)
// -+- change: Added CopyBits ADib method and ViewDCtoImage viewport method for RenderThread. Removed Capture method.
// modified 10/3/05 by JCF (fiala@bu.edu)
// -+- change: Added methods to VRMLObjects and scene to display in OpenGL directly without call lists.
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Renamed file to avoid confusion with Objects in Reconstruct. Modified params of SetDefaultName().
// modified 5/2/06 by JCF (fiala@bu.edu)
// -+- change: Added OffsetBetweenZTraces for future smoothing operation.
// -+- change: Added Contour::Smooth() and Series parameter smoothingLength.
// modified 5/11/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added Series::ContourMaskWidth;
//			   Changed the definition of ADib::MaskOutBorder to accept ContourMaskWidth parameter
// modified 5/11/06 by JCF (fiala@bu.edu)
// -+- change: Moved CreateGridSections to method of Section.
// modified 6/19/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added Contour::AverageIntensity for the option of automatic adjustment of intensity threshold
// modified 6/22/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Removed Contour::AverageIntensity. 
//             Added class Histogram
// modified 7/3/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added AutomaticTrace and AutomaticTraces classes.
//             Modified Color class to add assignment operator =. 
// modified 7/5/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified part of AutomaticTrace class to provide access function 
// modified 7/6/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Changed automatic adaptation of bright threshold to be based on brightest pixels and added necessary functions
// modified 7/7/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Changed definition of Wildfire 
// modified 11/13/06 by JCF (fiala@bu.edu)
// -+- change: Eliminated compiler WARNINGS/ERRORS so would compile under VC++.
// modified 11/14/06 by JCF (fiala@bu.edu)
// -+- change: added CopyView() to ViewPort class to replace PrepareADib global routine.
// -+- change: Moved Wildfire methods to ADib class since viewport dependency was removed to PrepareADib by Ju.
// -+- change: Straightened up some comments. Deleted WildfireRegions method as this can now be done by Wildfire().
// -+- change: Added new method AddViewPortContour() to Section to replace Ju's AddContour() global function.
// -+- change: Removed AutomaticTrace class since now use Active contours for same thing.
// modified 3/15/07 by JCF (fiala@bu.edu)
// -+- change: Added color channel mask attributes to Image class.
// modified 4/3/07 by JCF (fiala@bu.edu)
// -+- change: Added subimage index parameter to readGBMfile for GIF and TIFF stack imports.
// modified 7/18/07 by JCF (fiala@bu.edu)
// -+- change: To ADib class added GetIndex(), GetHSBPixel() routines that test x,y limits.

class EXCEPTION { };	// empty exception class for aborting operations

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The template LIST class defines generic linked list operations.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class Element> class List {		// template for doubly-linked, ordered list of Elements
public:
	Element *first, *last;					// hidden pointers to head and tail of list
	List() { first = NULL; last = NULL; };	// constructor of empty list
	virtual ~List();						// destructor frees all elements
	List( List<Element> &l );				// copy constructor
	int Number(void);						// return number of items in list
	void Extract( Element *element );		// remove element from list (but don't delete it) 
	void Add( Element *element ); 			// add element after last of the list
	void AddFirst( Element *element );		// add element before first of list
	void DeleteFirst( void );				// delete first element in list
	void Insert( Element *insert, Element *after );
	void Reverse( void );					// reverse order of entire list
};

#include "list.hpp"		// full template declaration is long and messy, keep it in a separate file

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The Points class definitions for linked lists of points derived from our template.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class XYPoint {								// integer point list
public:										
	XYPoint	*prev, *next;					// these pointers are for linking XYPoint into List
	int x, y;								// note all link-listable objects must have these ptrs
	XYPoint( int nx, int ny ) { x = nx; y = ny; };
};

class XYPoints : public List<XYPoint> { };	// used in ADib.cpp for keeping track of zero equivalencies

class Point {								// 3D points in real coordinates
public:
	Point	*prev, *next;
	double x, y, z;
	Point() { x = 0.0; y = 0.0; z = 0.0; };
	Point( double nx, double ny, double nz ) { x = nx; y = ny; z = nz; };
};

class Points : public List<Point> {};			// a list of Points

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Generic RGB colors class. All methods are right here...
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Color {
public:
	double r, g, b;
	Color() { r = 0.0; g = 0.0; b = 0.0; };
	Color( double nr, double ng, double nb ) { r = nr; g = ng; b = nb; };
	Color( COLORREF c ) { r = (double)GetRValue(c)/255.0; g = (double)GetGValue(c)/255.0; b = (double)GetBValue(c)/255.0; };
	COLORREF ref() { return( RGB( (BYTE)floor(fabs(r)*255.0),(BYTE)floor(fabs(g)*255.0),(BYTE)floor(fabs(b)*255.0) ) ); };
	bool operator ==( Color &c ) {	COLORREF a=c.ref(); COLORREF b=this->ref();
									return( (GetRValue(a)==GetRValue(b))&&(GetGValue(a)==GetGValue(b))&&(GetBValue(a)==GetBValue(b)) ); };
	void negate( void ) { r = -fabs(r);  g = -fabs(g); b = -fabs(b); };
	bool invalid( void ) { return( (r<0.0)||(g<0.0)||(b<0.0) ); };
	Color& operator=(const Color &c) { if (&c != this) { r = c.r; g = c.g; b =c.b; } return (*this); };	
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The Histogram class is a container for the histogram of pixels in a region. Methods in histogram.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Histogram {						
public:
	Histogram();							// since memory is not allocated can let compiler define destructor
	Histogram(Histogram &copyfrom);
//	Histogram& operator=(const Histogram& rhs);
//	void AddRGBPixel(int r, int g, int b);	// for future use of RGB in tracing criterion
	void AddHSBPixel(int h, int s, int b);
	void ResetHistogram();					// set all counts to zero
	int MeanValue(char choice);
	int PercentileValue(char choice, double percentile);
	
//private:
	int	count_red[RGBMAX+1];
	int	count_green[RGBMAX+1];
	int	count_blue[RGBMAX+1];
	int count_hue[HLSMAX+1];
	int count_sat[HLSMAX+1];
	int count_bright[HLSMAX+1];	
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The Contour class is a list of points with additional color attributes. Methods in contour.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ADib;								// define temporary placeholders so can add pointers as needed
class Nform;
class Series;

class Contour {							// the actual contour definition
public:
	Contour	*prev, *next;					// for linking Contour into List
	char	name[MAX_CONTOUR_NAME];
	char	comment[MAX_COMMENT];
	bool	hidden;
	bool	closed;
	bool	simplified;
	Color	border;
	Color	fill;
	int		mode;							// [1,16] is ROP2 fill mode, [-1,-16] => fill when unselected
	int		Id;								// number assigned at creation used to uniquely locate contour
    Histogram *histogram;					// pointer to histogram of interior image pixels
	Points	*points;						// these are the points of the contour
	Contour();								// empty contour constructor
	~Contour();								// destructor: ASSUMES NO DERIVED CLASSES SINCE NOT virtual
	Contour( Contour &copyfrom );			// copy constructor
	void Scale( double scale );				// multiply contour points by scale
	void Pixels( double pixelsize );		// scale to integer pixels
	void ScaleXY( double scalex, double scaley );	// use different scale factors for x and y
	void Shift( double x, double y );		// shift contour points by x, y
	void ShiftXYZ( double x, double y, double z );
	void YInvert( double y );				// invert y coordinate around double value y
	void FwdNform( Nform *N );				// pass contour points through forward transformation
	void InvNform( Nform *N );				// pass contour points through inverse transformation
	void Extent( Point *min, Point *max );	// determine min(x,y,z) and max(x,y,z) for all points
	double Length( void );					// return length of contour
	Points * WallsInRegion( double minx, double maxx, double miny, double maxy ); // return list of walls 
	void AddPixelsAtBoundaries( double minx, double maxx, double miny, double maxy );// add points at boundaries
	void ChainCode( double pixel_size );	// convert arbitrary contour to chain code on integers
	void Reduce( double max_error );		// reduce contour points until just within max_error of original
	void Exterior( void );					// take the exterior envelope of the pixelized point set
	void Interior( void );					// shrink-back the Exterior by one pixel
	void Simplify( double pixelsize, bool hull );// reduce contour points and remove loops
	double Distance( double x, double y );	// find minimal distance from x,y to contour
	void Add( Contour *addfrom );			// add points from contour to this one
	void GreensCentroidArea( double &x, double &y, double &area ); // calculation is inaccurate w/loops
	void Reverse( void );					// reverse the order of contour points
	bool IsInRegion( double left, double top, double right, double bottom );
	void CutAtNearest(double x, double y);	// cut the trace at pt nearest (x,y) so it can be edited
	double MinSqrdDistance( Contour *c );	// return distance from another contour in same plane
	Points * Contour::XOR( Contour *bisector );	// return the pieces of defined by the intersections
	void Smooth( const int length );		// apply moving average filter to x and y values
};

class Contours : public List<Contour> {
public:
	 void Merge( double pixelsize );		// merge list of overlapping contours into one contour	 
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The Image class is a container for the image src path and the image attributes. Methods in image.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Image {
public:
	double	mag;							// pixel size scaling
	double	contrast;						// pixel intensity scaling
	double	brightness;						// pixel intensity offset
	bool	red, green, blue;				// color channel mask (true == show channel)
	char	src[MAX_PATH];					// path to full image
	char	proxySrc[MAX_PATH];				// path to scaled image
	double	proxyScale;						// (0.0,1.0) amount of scaling in proxy
	int		loaded;							// NONE=0, SKIPPED, PROXY, or SOURCE
	ADib	*adib;
	Image();								// constructors do not fill adib
	Image ( Image &copyfrom );				// copy constructor does not copy adib, just NULLs it
	~Image();								// destructor frees adib memory
	bool LoadAdib( bool useProxy, double pixel_size );	// load Adib from file, return true if success
	bool HasProxy( void );					// return true if image has proxy
	void MakeProxy( double scale );			// from full adib, make scaled version and save to file
	void DeleteProxy( void );				// remove proxy reference and file
};

typedef Image* Image_Ptr;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Coordinate transformations classes. Methods in nform.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Mvmt {									// set of movement parameters for user input
public:
	double theta, transX, transY, scaleX, scaleY, slantX, slantY, deformX, deformY, centerX, centerY;
	Mvmt()	{	theta=0.0; transX=0.0; transY=0.0; scaleX=1.0; scaleY=1.0; slantX=0.0; slantY=0.0;
				 deformX=0.0; deformY=0.0; centerX=0.0; centerY=0.0; };
	Mvmt( Mvmt &copyfrom )
			{	theta=copyfrom.theta; scaleX=copyfrom.scaleX; scaleY=copyfrom.scaleY;
				slantX=copyfrom.slantX; slantY=copyfrom.slantY; transX=copyfrom.transX;
				transY=copyfrom.transY; centerX=copyfrom.centerX; centerY=copyfrom.centerY;
				deformX=copyfrom.deformX; deformY=copyfrom.deformY; };
	void Clear( void ) { theta=0.0; scaleX=1.0; scaleY=1.0; deformX=0.0; deformY=0.0;
				slantX=0.0; slantY=0.0; transX=0.0; transY=0.0; };	// no movement
};
	
class Nform {									// nonlinear transform by linear comb. of 2nd order poly.
public:
	int	dim;											// number significant of a and b (dim=0,..,DIM)
	double a[DIM];										// array of x params: X = a0 + a1*x + a2*y +...
	double b[DIM];										// array of y params: Y = b0 + b1*x + b2*y +...
	Nform();											// create identity Nform (dim = 0)
	Nform ( Nform &copyfrom );							// copy constructor
	void Clear();										// return to identity Nform
	double X( double x, double y );						// forward mapping: X part
	double Y( double x, double y );						// forward mapping: Y part
	void XYinverse( double *x, double *y );				// inverse mapping (in place)
	void ComputeMapping( double *fx, double *fy,		// compute nform params by correspondence
							double *rx, double *ry, int nopts, int d );
	void ComputeRigid( double *x, double *y, 			// nforms for rigid body correspondence
							double *u, double *v, int nopts );
	Nform * Inverse( double w, double h );				// approximate inverse of Nform
	void Compose(Nform *t1,Nform *t2,double w,double h);	// approximate the composition of Nforms
	void Decompose(Nform *t1,Nform *t2,double w,double h);// approx. T1_inverse * T2
	void ApplyLinearOf( Nform *p );						// compose only with linear part
	void ApplyInverseOf( double *aa, double *ab );		// get inverse of only linear part
	void PreApply( Mvmt mvmt );							// do mvmt in local coords
	void PostApply( Mvmt mvmt );						// do mvmt in extrinsic (e.g. screen) coords
	void SetToDifference( Nform *t1, Nform *t2 );		// set this to difference t1 - t2
};

typedef Nform* Nform_Ptr;								// for array of Nform ptrs (while rendering)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The Transform class contains an Nform, an image and/or lists of contours. Some methods in tfromsLIFO.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Transform {
public:
	Transform	*prev, *next;							// for linking Transform into List
	bool		isActive;								// mark an active transform with true
	Nform		*nform;
	Contours	*contours;
	Image		*image;
	Contour		*domain;
	Transform() { isActive = false; nform = new Nform(); contours = NULL; image = NULL; domain = NULL; };
	Transform( Transform &copyfrom ) {
		isActive = copyfrom.isActive;
		if ( copyfrom.nform ) nform = new Nform( *(copyfrom.nform) );
		else nform = NULL;
		if ( copyfrom.contours ) contours = new Contours( *(copyfrom.contours) );
		else contours = NULL;
		if ( copyfrom.image ) image = new Image( *(copyfrom.image) );
		else image = NULL;
		if ( copyfrom.domain ) domain = new Contour( *(copyfrom.domain) );
		else domain = NULL;
		};
	~Transform() {
		if ( nform ) delete nform;
		if ( contours ) delete contours;
		if ( image ) delete image;
		if ( domain ) delete domain;
		};
};

class Transforms : public List<Transform> { };
typedef Transforms* Transforms_Ptr;						// for array of ptrs to undo Transforms lists

class TformsLIFO {										// a limited LIFO for undos of transform lists
public:
	int last;
	Transforms_Ptr lifo[MAX_UNDOS];
	TformsLIFO();
	~TformsLIFO();
	TformsLIFO( TformsLIFO &copyfrom );
	void Push( Transforms *transforms );				// add to stack
	Transforms * Pop( void );							// remove from stack
	void Clear( void );									// empty the stack
	Transforms * Reset( void );							// get bottom then clear stack
	Transforms * First( void );							// get bottom w/o changing stack
};	

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  A device independent bitmap class. Methods in adib.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ADib {
public:
	int width;											// width in pixels
	int height;											// height in scanlines
	int	bitsperpixel;									// bits per pixels
	int numcolors;										// number of colors in color table, 0 if no color table
	int byteswidth;										// number of bytes in scanline w/pad to DWORD boundary
	int filetype; 										// see body of class adib.cpp for types
    BITMAPINFO			*bmi;							// bitmap info structure
    BYTE				*bits;							// pixels of image
    ADib( int w, int h, int p );						// create wXh ADib with p bits per pixels
    ~ADib();											// destructor
	ADib( const char *fn );								// fill bmi and bits from file
	ADib( const ADib& copyfrom, double scale );			// copy constructor with scaling (1.0 => none)
	void CopyBits( ADib *adib );						// copy the pixel bits from this to ADib
    int FileType( const char *fn );						// guess filetype of the file
    bool readGBMtoADib( const char *fn, int image_num );// use GBM library to read file
	void Clear( void );									// reset entire ADib to zero
	void Clear( RECT region );							// reset ADib in region to zero value
	Mvmt * Correlate( ADib *dib );
	void Shift( int x, int y );							// shift view by x and y pixel offsets
	void Blend( ADib *src1, ADib *src2 );				// blend two src dibs into this one
	void Fill( Contour *contour, double pixel_size, RECT r, Color c );
														// label interior pixels in region w/index, return region labeled
	RECT Mask( Contour *contour, double pixel_size, int index, RECT region );
	void MaskOutBorder( Contour *contour, double pixel_size, int ContourMaskWidth );// clear index of contour pixels
	int GetPixel( int x, int y );						// get entire pixel value at x,y
	void SetIndex( int x, int y, BYTE index );			// set index byte of pixel x,y
	int GetIndex( int x, int y );						// get index byte at pixel x,y
	int GetHSBPixel( int x, int y );					// return hue, sat, brightness values in lower 24 bits
	void MaskXform( Image *image, Nform *nform, int index, POINT min, POINT max,
								double dest_pixel_size, double dest_offset_x, double dest_offset_y, bool use_proxy );
	bool SaveADibAsBMP( const char * fn );				// save at current color size in Windows Bitmap format
	bool SaveADibAsJPEG( const char *fn, int quality );	// or in JPEG format
	void ConvertUpTo24bit( void );						// convert to a 24-bit bitmap (need for JPEG save)
	Contour * Wildfire( int x, int y, double min_size );// generate a contour by region growing from (x,y) origin
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The Section object is the highest level data representation of the section file. Methods in section.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Section {
public:
	Section		*prev, *next;						// for linking Section into list
	bool		hasChanged;							// flag to indicate object needs saving
	bool		alignLocked;						// flag to indicate whether movements are allowed
	int			index;								// section number, -1 indicates not valid
	double		thickness;
	Transforms	*transforms;						// list of Transforms in the Section
	Transform	*active;							// pointer to the Transform currently being edited
	TformsLIFO	*undos;								// undo LIFO for transforms
	Transforms	*redo;								// remember state before last undo
	Section();
	Section( Section &copyfrom );					// copy constructor doesn't copy active, just NULLs it
	Section( char * sectionfile );					// load Section data structure from Section file
	~Section();
	void Save( char * sectionfile );				// output Section data structure components
	bool SaveIfNeeded();							// save section file before deleting it?
	void AddNewContour( Contour *contour );			// add contour to active transform
	void SetDefaultName(char *name, char *defname, bool domain); // interpret default string to generate name
	Transform * AddNewImage( char * imagefile, double mag );// read image, on success return domain transform
	void AddViewPortContour( Contour *contour );	// add contour from viewport's pixel coordinates
	Transform * ExtractActiveTransform( void );		// remove *active from *transforms and return pointer
	Contour * FindClosestContour( double x, double y );	// return ptr to contour nearest x,y
	Contour * ExtractClosestContour( double x, double y );// remove and return contour from section
	bool SelectClosestContour( double x, double y );	// nearest to x,y is moved to active transform
	bool SelectContoursInRegion( double left, double top, double right, double bottom );
	Contour * SelectContourId( int id );			// find contour with Id, move to active transform
	Contour * FindContourId( int id );				// return ptr to contour with Id or NULL if fail
	Transform * FindDomainId( int id );				// return the transform for domain with id
	void UnSelectClosestActive( double x, double y );	// nearest active contour is removed from active transform
	void SelectAll( void );							// move all contours to active transform
	void UnSelectAll( void );						// move all active contours back into section
	void UnselectHidden( void );					// remove hidden traces from active list
	bool HasImage( void );							// report whether any images in section
	double MinPixelsize( void );					// report smallest pixelsize in section
	void ImageSize( Point &min, Point &max );		// report extremes of all image domains
	bool HasContour( void );						// report whether any contours in section
	void ContourSize( Point &min, Point &max );		// get extremes of all contours in section
	void PushUndoState( void );						// remember current section state for later undo
	bool PushRedoState( void );						// push state onto undos and set state to redo
	bool PopUndoState( void );						// return to last pushed state and flag change with true
	bool ResetUndoState( void );					// jump back to first undo state
	bool HasUndos( void );							// true if undo lifo not empty
	bool HasRedo( void );							// report whether redo != NULL
	void AddDXFLines( char *filename );				// add polylines from DXF file to section
	void CreateGridTraces( double fx, double fy );	// place grid trace(s) at location (fx,fy) on section
};

class Sections : public List<Section> { };			// for undo stacks when editing sections

class SectionInfo {
public:
	SectionInfo	*prev, *next;						// for linking into list
	bool		alignLocked;						
	int			index;								// section number
	double		thickness;
	Point		min, max;							// bounds of section in x, y
	double		pixelsize;							// smallest pixelsize in section
	char		filename[MAX_PATH];					// path to actual section file
	SectionInfo() { prev = NULL; next = NULL; };
};

class SectionsInfo : public List<SectionInfo> { 	// for list of sections in series
public:
	bool IsSection( int sectnum );					// return true if sectnum is in section list
	void AddSection( Section *add, char *src );		//	add section to list in correct order
	double ZDistance( int sectnum, bool middle );	//	find total distance to sectnum
	void DeleteSection( int sectnum );				// delete section with index from list
	void SetThickness( int sectnum, double thickness );
	void SetLock( int sectnum, bool lock );
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   A ViewPort class for representing a displayed section. Methods in viewport.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ViewPort {
public:
	Section *section;								// the Section that is displayed
	HDC viewDC;										// memory device context BitBlt to display DC
	HBITMAP canvas, orig;							// actual bitmap canvas for rendering scene
	ADib *view;										// editable bitmap for rendering section images
	int	width, height;								// width and height of scene in pixels
	bool needsRendering, needsDrawing;				// flags to indicate what needs to be regenerated
	ViewPort( HWND hWnd );							// create drawing surface from window handle
	ViewPort( int w, int h );						// create a non-drawing version
	ViewPort( int w, int h, HWND hWnd );			// create drawing version with specified w and h
	~ViewPort();									// destructor
	HBITMAP ReleaseCanvas( void );					// release the window-compatible bitmap for use elsewhere
	void Resize( int w, int h );					// resize the viewport to w, h
	bool ViewDCToImage( void );						// put dc bits into view DIB
	void Display( HDC dc, RECT region );			// put DIB bits to device context dc
	void Zoom( HDC dc, double scale, int x, int y );// zoom display of viewDC
	void Pan( HDC dc, int x, int y );				// shift display of viewDC
	void ImageToViewDC( void );						// copies the already rendered view to DC
	void ClearImages( void );						// clears entire view so can render images on clean slate
	void ClearImages( RECT region );				// clears only region of view
	void RenderImages( RECT region, double pixel_size, double offset_x, double offset_y, bool use_proxy );
	void FillContours( RECT region, double pixel_size, double offset_x, double offset_y );
	void DrawContours( double pixel_size, double offset_x, double offset_y );
	void DrawActiveDomain( double pixel_size, double offset_x, double offset_y );
	void DrawEditContour( double pixel_size, double offset_x, double offset_y );
	Transform * DomainFromPixel( int x, int y );	// use index byte to locate domain in section
	bool Regenerate( RECT region, double pixel_size, double offset_x, double offset_y, bool use_proxy );
	void ShiftView( int x, int y, double pixel_size, double offset_x, double offset_y, bool use_proxy );
	ADib * CopyView( bool maskContours );			// create copy of bitmap w/contour pixels masked or not
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  The Series object stores info from series file including series options. Methods in series.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Series {
public:
	char	units[MAX_UNITS_STRING];		// descriptor of units of measure used
	int		index;							// current position in series
	double	pixel_size;						// dimensions of square pixel of "view" in "units"
	double	offset_x, offset_y;				// offset of lower-left image origin in "units"
	double	defaultThickness;				// default section thickness for new sections
	bool	zMidSection;					// general options for saving series and sections, etc.
	bool	autoSaveSeries, warnSaveSection, autoSaveSection, beepDeleting, beepPaging;
	bool	hideTraces, unhideTraces, hideDomains, unhideDomains, useAbsolutePaths;
	int		thumbWidth, thumbHeight;		// thumbnail options
	bool	displayThumbContours, fitThumbSections, useFlipbookStyle;
	int		firstThumbSection, lastThumbSection, skipSections, flipRate;
	bool	useProxies;						// image proxy options
	int		widthUseProxies, heightUseProxies;
	double	scaleProxies;
	int		significantDigits;				// how trace values will be represented in XML files
	Color	defaultBorder;					// values to use when creating contours
	Color	defaultFill;
	int		defaultMode;
	char	defaultName[MAX_CONTOUR_NAME];
	char	defaultComment[MAX_COMMENT];	// enable/disable list fields with these flags...
	bool	listSectionThickness;
	bool	listDomainSource, listDomainPixelsize, listDomainLength, listDomainArea, listDomainMidpoint;
	bool	listTraceComment, listTraceLength, listTraceArea, listTraceCentroid, listTraceExtent, listTraceZ, listTraceThickness;
	bool	listObjectRange, listObjectCount, listObjectSurfarea, listObjectFlatarea, listObjectVolume;
	bool	listZTraceNote, listZTraceRange, listZTraceLength;
	Color	borderColors[16];				// custom colors
	Color	fillColors[16];
	double  x3Doffset, y3Doffset, z3Doffset;// offset added to generated 3D objects
	double	max3Dconnection;				// limit reach of surfacing algorithm
	int		first3Dsection, last3Dsection;	// range for 3D object generation
	int		type3Dobject;					// traces, surface, etc.
	bool	upper3Dfaces, lower3Dfaces, faceNormals, vertexNormals;
	int		facets3D;						// resolution for generation of spheres & cylinders
	double	dim3Da, dim3Db, dim3Dc;			// size dimensions of generated 3D objects
	int		gridType, gridXNumber, gridYNumber;
	double	gridXSize, gridYSize, gridXDistance, gridYDistance;
	int		hueStopWhen, hueStopValue, satStopWhen, satStopValue, brightStopWhen, brightStopValue;
	bool	tracesStopWhen;
	double	areaStopPercent, areaStopSize;
	int		ContourMaskWidth;				// width of the masked strip along contours. 
	int		smoothingLength;
	Mvmt	Increment, ShiftIncrement, CtrlIncrement;
	Contours	*contours;					// default contours: ordered list 0-9
	Contours	*zcontours;					// multisection spanning contours
	Series();
	Series( Series &copyfrom );				// copy constructor
	Series( char * seriesfile );			// load Series data structure from Series file
	~Series();
	void SetDefaultAttributes( Contour *contour );	// set defaults to contour
	void Dump( void );						// debug Series data structure components
	void Save( char * seriesfile );			// save series info to file
	void SaveIfNeeded();					// save series file before deleting it?
	void AddZContour( Contour *contour );	// add contour to zcontour list
	double ZLength( Contour *contour );		// compute length of z-contour
	Contour * ZMatch( char *name );			// return ptr to contour that matches name
	Contour * ZTraceFromId( int id );		// find z-contour with this id number
	void RenumberZTraces( int fromsect, int tosect );	// change section number in all zcontours
	Point OffsetBetweenZTraces( int sectnum, Contour *Z1, Contour *Z2 ); // x,y distances Z1->Z2 at z
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  An XML Data class for holding raw XML ASCII data and parsing into components. Methods in xmldata.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class XML_DATA {													// Data Object for reading and parsing XML
public:
	char *data;														// bytes of raw ascii XML
	unsigned int size;												// how much of it there is
	XML_DATA( char * filename );									// intialize XML_DATA from file
	~XML_DATA();													// free memory used
	char * findString( char *start, char *pattern, char *stopper );	// locate a substring
	char * openQuote( char *src );									// advance past opening quote mark
	void getString( char *dest, char *src, int max_len );			// retrieve a string attribute
	double getdouble( char *src );									// retrieve a single double attribute
	int getInt( char *src );										// retrieve a siingle int attribute
	bool getBool( char *src );										// retrieve a boolean attribute
	char * getNextdouble( double *f, char *src );						// retreive double and advance to next one
};																	//  return 0 when no more points available


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Some user interface objects.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Button {											// class for bitmapped buttons
public:
	Button *prev, *next;								// for linking Button into list
	HWND	hwnd;										// handle to button
	int		number;										// section number or tool number, depending on context
	HBITMAP	canvas;										// bitmap displayed on button 
	Button() { hwnd = NULL; canvas = NULL; }
	Button( Button &b ) { hwnd = b.hwnd; number = b.number; canvas = b.canvas; }
	~Button() { if ( canvas ) DeleteObject( canvas ); } // hwnd is destroyed when parent is destroyed
	void HighlightButton( void )						// draw/undraw rectangle around border of button face
		{
		RECT r;
		HPEN hpen = CreatePen( PS_DOT, 1, RGB(0,0,0) );
		HDC hdc = GetDC( hwnd );
		GetClientRect( hwnd, &r );
		SelectObject( hdc, hpen );
		SelectObject( hdc, GetStockObject(NULL_BRUSH) );
		SetROP2( hdc, R2_NOT );
		Rectangle( hdc, r.left+2, r.top+2, r.right-2, r.bottom-2 );
		ReleaseDC( hwnd, hdc );
		};
};

class Buttons : public List<Button> { };				// list of buttons

class ZoomState {										// structure for holding zoom params
public:
	double pixel_size;
	double offset_x;
	double offset_y;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VRML-like 3D objects. Methods in VRMLobject.cpp, VRMLObjects.cpp, and VRMLScene.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class VertexSet {										// class for point coordinates of object
public:
	unsigned int total;									// allows over 4 billion Points
	unsigned int last;
	Point *vertex;										// vertex points to indexed array of Points
	VertexSet( unsigned int n ) { total = n; vertex = new Point[n]; last = 0; };
	~VertexSet() { delete [] vertex; };
	void ExpandTo( unsigned int n )							// dynamic expansion of vertex set
		{
		Point *v;
		v = new Point[n];
		for (unsigned int i=0; i<total; i++) v[i] = vertex[i];
		delete [] vertex;
		vertex = v;
		total = n;
		}
};

class Line {											// a line segment
public:
	unsigned int v1, v2;								// indices into vertices connecting two points
	Line() { v1 = v2 = 0; };
	Line( unsigned int i, unsigned int j ) { v1 = i; v2 = j; };
};

class LineSet {											// an array of indexed line segments
public:
	unsigned int total;
	Line *line;											// line points to indexed array of Lines
	LineSet( unsigned int n ) { total = n; line = new Line[n]; };
	~LineSet() { delete [] line; };
};

class Face {											// a trianglular face class
public:
	unsigned int v1, v2, v3;							// indices of vertices of triangular face
	Face() { v1 = v2 = v3 = 0; };
	Face( unsigned int i, unsigned int j, unsigned int k ) { v1 = i; v2 = j; v3 = k; };
};

class FaceSet {											// an array of indexed faces
public:
	unsigned int total;
	Face *face;											// face points to indexed array of Faces
	FaceSet( unsigned int n ) { total = n; face = new Face[n]; };
	~FaceSet() { delete [] face; };
	void ExpandTo( unsigned int n )							// dynamic expansion of face set
		{
		Face *f;
		f = new Face[n];
		for (unsigned int i=0; i<total; i++) f[i] = face[i];
		delete [] face;
		face = f;
		total = n;
		}
};

class VRMLObject {										// a VRML-like 3D mesh object
public:
	VRMLObject *prev, *next;							// for linking VRMLObject into list
	char name[MAX_CONTOUR_NAME];
	char comment[MAX_VRML_COMMENT];							// contains parameter setting used for object generation
	Color diffuseColor, emissiveColor, specularColor;
	double ambientIntensity, shininess, transparency;
	Point min, max;										// bounding box of object in x,y,z
	unsigned int openGLList;							// openGLList is index of compiled object in openGL
	int firstSection, lastSection;						// section range
	bool normalPerVertex;								// false => 1 normal per face, true => 1 normal per vertex
	bool frontFill, backFill;							// whether to draw faces as lines or filled
	int contour_count;									// the number of contours in object
	double surface_area, flat_area, volume;
	Contours *contours;									// the contours in 3D
	VertexSet *vertices;
	LineSet  *lines;
	FaceSet  *faces;
	VertexSet *normals;
	VRMLObject();										// default constructor
	VRMLObject( VRMLObject &copyfrom );					// copy constructor
	~VRMLObject();
	void CreateZline( Contour *zcontour  );				// create zline 3D object from z-contour
	void CreateContours( void );						// create object as a set of 3D contour lines
	void CreateSphere( void );							// create a sphere sized to max object extent
	void CreateCylinder( void );						// create a cylinder aligned with principal axis
	void CreateEllipsoid( void );						// create scatter ellipsoid
	void CreateAreas( void );							// create trace areas on sections 
	void CreateBox( void );								// create a solid RPP equal to the extent
	void CreatePointSet( void );						// create a face at each trace midpoint
	void CreateBoissonnat( void );						// surface object using delaunay triangulation
	void Compile( void );								// compile the object into OpenGL list
	void Display( void );								// display the object in OpenGL context
	bool WriteVRML2( HANDLE hFile );					// output shape in VRML 2.0 format
	bool WriteVRML1( HANDLE hFile );					// output shape in VRML 1.0 format
	bool WriteDXF( HANDLE hFile );						// output shape in DXF format
};

class VRMLObjects : public List<VRMLObject> {			// a list of 3D objects
public:
	VRMLObject * Match( char * name );					// find matching object
};

class VRMLScene {										// current 3D scene
public:
	Color	background;									// background of scene window is initially white
	Point	min, max;									// min, max give bounds of scene
	double fovLeft, fovRight, fovBottom, fovTop;		// params of scene's perspective transformation
	double fov, fovNear, fovFar;						// fov in radians, and z-axis clipping planes
	double xCenter, yCenter, zCenter;					// scene centering translation			
	double xTrans, yTrans, zTrans;						// users offset and viewing transformation
	double xRotInc, yRotInc;							// user's desired rotational changes
	GLdouble m[16];										// openGL model matrix
	VRMLObjects *objects;								// list of objects in the scene
	VRMLScene();
	~VRMLScene();
	void Add( VRMLObject *object );						// Add object to scene
	void ClearObjects( void );							// remove all objects from scene
	void Compile( void );								// recompile entire scene
	void Reset( void );
	void Display( void );								// execute the display lists for all objects
	void DisplayUncompiled( void );						// display directly w/o compiling first (for bitmaps)
	void ComputeViewpoint( double &ax, double &at, double &az, double &a, double &x, double &y, double &z );
	void SaveVRML2( char * filename );					// save the scene as VRML 2.0 file
	void SaveVRML1( char * filename );					// save the scene as VRML 1.0 file
	void SaveDXF( char * filename );					// save scene as flat DXF file
};

class InterObject {										// relationships between objects
public:
	InterObject	*prev, *next;							// all list element class need these
	char object1[MAX_CONTOUR_NAME];
	char object2[MAX_CONTOUR_NAME];
	double distance;
};

class InterObjects : public List<InterObject> { };		// list of relationships

