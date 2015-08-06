//////////////////////////////////////////////////////////////////////////////////////////
// A Device Independent Bitmap class.
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
// modified 10/29/04 by JCF (fiala@bu.edu)
// -+- change: Added GetPixel, SetIndex and replaced GetMaskByte with GetPixel.
// modified 4/29/05 by JCF (fiala@bu.edu)
// -+- change: Added MaskOutBorder() method for marking contours to stop region growing.
// modified 7/14/05 by JCF (fiala@bu.edu)
// -+- change: Added CopyBits method used in RenderThread.
// modified 5/11/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified MaskOutBorder method to mask out a strip along the contour with width
//	set in ContourMaskWidth by the user. 
// modified 6/5/06 by JCF (fiala@bu.edu)
// -+- change: Added debug logging of operations for crash detection.
// modified 11/14/06 by JCF (fiala@bu.edu)
// -+- change: Added Wildfire methods and auxillary routines.
// -+- change: Modified Wilfdfire() to zero index after region growing so can use multiple times on same region,
//              and to check for min_size before creating boundary. Still need to deal with histogram stuff.
// modified 11/21/06 by JCF (fiala@bu.edu)
// -+- change: Modified debug code and how LoadAdib is called in XMask ... no changes in functionality.
// modified 3/15/07 by JCF (fiala@bu.edu)
// -+- change: Added Image color channel masks to RGB output of MaskXform().
// modified 4/3/07 by JCF (fiala@bu.edu)
// -+- change: Added subimage index parameter to readGBMfile for GIF and TIFF stack imports.
// modified 4/5/07 by JCF (fiala@bu.edu)
// -+- change: No substantive changes made, but debugged Mask() looking for black lines problem.
// modified 4/27/07 by JCF (fiala@bu.edu)
// -+- change: Removed setting of hue to initial value when undefined. On pure grayscale hue is 255.
// modified 4/30/07 by JCF (fiala@bu.edu)
// -+- change: Moved RGBtoHSB routine to utilities.cpp.
// -+- change: Changed Stop Criterion "equals" to "does not equal".
// modified 7/18/07 by JCF (fiala@bu.edu)
// -+- change: Moved RGB2HSB routine into ADib, and added GetIndex(), GetHSBPixel() routines that test x,y limits.

#include "reconstruct.h"

#ifndef IJG						// if JPEG library included then read this filetype also
#define MAX_TYPE 17				// the array this pertains to is defined in globals.cpp
#else
#define MAX_TYPE 18
#endif							
								// define the masks for going from RGB values to integers and back
#define B(i) ((BYTE)i)
#define G(i) ((BYTE)((i&0xFF00)>>8))
#define R(i) ((BYTE)((i&0xFF0000)>>16))
#define RGBINT(d) (*((int *)d)&0xffffff)

ADib::ADib( int w, int h, int p )				// blank image constructor
{
	filetype = 0;
	width = w;									// width in pixels
	height = h;									// height in pixels
	byteswidth = ((w*p + 31)/32)*4;				// number of bytes per line, including pad to DWORD boundary
	bitsperpixel = p;
	if ( p < 24 ) numcolors = 1 << p;			// only have palette if less than 24 bpp
	else numcolors = 0;
												
	bits = (BYTE *)GlobalAllocOrDie(byteswidth*height);	// allocate pixel memory

														// allocate bmi memory
	bmi = (BITMAPINFO *)GlobalAllocOrDie(sizeof(BITMAPINFOHEADER)+numcolors*sizeof(RGBQUAD));

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth = width;
	bmi->bmiHeader.biHeight = height;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = bitsperpixel;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biXPelsPerMeter = 3780;
	bmi->bmiHeader.biYPelsPerMeter = 3780;							// set default 96dpi
	bmi->bmiHeader.biSizeImage = 0;
	bmi->bmiHeader.biClrUsed = 0;
	bmi->bmiHeader.biClrImportant = 0;

	for ( int i=0; i<numcolors; i++ )			// create standard gray color table palette
		{		
		bmi->bmiColors[i].rgbBlue = (BYTE)i;
		bmi->bmiColors[i].rgbRed = (BYTE)i;
		bmi->bmiColors[i].rgbGreen = (BYTE)i;
		bmi->bmiColors[i].rgbReserved = 0;
		}
}

ADib::~ADib()								// destructor
{
	if ( bits ) free( bits );
	if ( bmi ) free( bmi );
}

int ADib::FileType( const char * fn )
{
	HANDLE file;
	char buf[32];
	DWORD numbytesread;
	int  ft;								// first see what GBM makes of the filename

	if ( gbm_guess_filetype(fn, &ft) != GBM_ERR_OK ) ft = 0;
	else ft++;								// offset GBM types by one so can have unknown '???'

   											// now check header, so open the file...

	file = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( file )  							// if successful, read first characters
		{
		strcpy(buf,"000000000000000000000");				// clear buffer
		ReadFile( file, buf, 20, &numbytesread, NULL );	// read from file
		CloseHandle( file );
		if ( !(strncmp(buf, "\x42\x4D", 2)) ) ft = 1;				// Windows Bitmap
		else if ( !(strncmp(buf, "\x47\x49\x46", 3)) ) ft = 2;		// GIF
		else if ( !(strncmp(buf, "\x49\x49\x2A\x00", 4)) ) ft = 4;	// TIFF
		else if ( !(strncmp(buf, "\x4D\x4D\x00\x2A", 4)) ) ft = 4;	// TIFF
		else if ( !(strncmp(buf, "\xff\xd8\xff", 3)) ) ft = 17;		// JPEG
		}

	return ft;
}

												// code for using the GBM library to read file into DIB memory
bool ADib::readGBMtoADib( const char *fn, int image_num )
{
	int fd, ft;
	GBM gbm;
	GBMRGB *gbmrgb;
//	GBM_ERR rc;
	char opt[64], errmsg[80];

	ft = filetype - 1;										// revert to GBM index

	if ( (fd = gbm_io_open(fn, O_RDONLY|O_BINARY)) == -1 )	// open file
		{	
//		sprintf(errmsg,"Can't open %s", fn);
//		ErrMsgOK( ERRMSG_READ_FAILED, errmsg );
		return false;
		}
															// no option string for most file types
	opt[0] = '\0';											// but index to subimage in GIF and TIFF stacks
	if ( image_num > 0 ) sprintf(opt,"index=%d",image_num);
	if ( gbm_read_header(fn, fd, ft, &gbm, opt) != GBM_ERR_OK )
		{
//		sprintf(errmsg,"Can't read header of %s.\nError is %d.", fn, rc);
//		ErrMsgOK( ERRMSG_READ_FAILED, errmsg );
		gbm_io_close(fd);
		return false;
		}

	numcolors = 0;
	if ( gbm.bpp < 24 )										// if less than 24-bit, then have palette
		{
   		numcolors = 1 << gbm.bpp;
   		gbmrgb = new GBMRGB[numcolors];						// read palette
		if ( gbm_read_palette(fd, ft, &gbm, gbmrgb) != GBM_ERR_OK )
			{
//			sprintf(errmsg,"Can't read palette of %s", fn);
//			ErrMsgOK( ERRMSG_READ_FAILED, errmsg );
      		gbm_io_close(fd);
      		return false;
			}
		}
	else gbmrgb = NULL;
															// allocate memory for bits

	width = gbm.w;									// width in pixels
	height = gbm.h;									// height in pixels
	byteswidth = ((gbm.w*gbm.bpp + 31)/32)*4;		// number of bytes per line, including pad to DWORD boundary
	bitsperpixel = gbm.bpp;
										
	bits = (BYTE *)GlobalAllocOrDie(byteswidth*height);	// allocate pixel memory

														// read bits from file
	if ( gbm_read_data(fd, ft, &gbm, bits) != GBM_ERR_OK )
		{
//		sprintf(errmsg,"Can't read bitmap data of %s", fn);
//		ErrMsgOK( ERRMSG_READ_FAILED, errmsg );
		gbm_io_close(fd);
		return false;
		}

	gbm_io_close(fd);
														// allocate bmi memory

	bmi = (BITMAPINFO *)GlobalAllocOrDie(sizeof(BITMAPINFOHEADER)+numcolors*sizeof(RGBQUAD));

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth = width;
	bmi->bmiHeader.biHeight = height;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = bitsperpixel;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biXPelsPerMeter = 3780;
	bmi->bmiHeader.biYPelsPerMeter = 3780;							// set default 96dpi
	bmi->bmiHeader.biSizeImage = 0;
	bmi->bmiHeader.biClrUsed = 0;
	bmi->bmiHeader.biClrImportant = 0;

	for ( int i=0; i<numcolors; i++ )			// retain original palette
		{		
		bmi->bmiColors[i].rgbBlue = gbmrgb[i].b;
		bmi->bmiColors[i].rgbRed = gbmrgb[i].r;
		bmi->bmiColors[i].rgbGreen = gbmrgb[i].g;
		bmi->bmiColors[i].rgbReserved = 0;
		}

   delete[] gbmrgb;
   return true;									// success!
}


ADib::ADib( const char *fn )			// create ADIB from file
{
	bits = NULL;
	bmi = NULL;
// begin debug logging...
	DWORD byteswritten;
	char debugline[MAX_PATH];
	if ( debugLogFile != INVALID_HANDLE_VALUE )
		{
		sprintf(debugline,"Entered ADib::ADib( %s )\r\n",fn);
		WriteFile(debugLogFile, debugline, strlen(debugline), &byteswritten, NULL);
		}
// ...end debug logging
	filetype = FileType( fn );		// see if know filetype

	if ( (filetype > 0) && (filetype < MAX_TYPE) )	// don't know filetype, fail!
		if ( !readGBMtoADib( fn, 0 ) )	// no stacks here, so image_num is zero
			{
      		if ( bits ) free( bits );
			bits = NULL;				// if fails -- clean up crap
			if ( bmi ) free( bmi );
			bmi = NULL;
			}
}
				
ADib::ADib( const ADib& copyfrom, double scale )			// copy constructor with scaling (1.0 => none)
{
	int i, j, k, x, y, bytesperpixel;
	BYTE *destbits, *srcbits, *d, *s;

	width = (int)(scale*(double)copyfrom.width);			// scale image params
	height = (int)(scale*(double)copyfrom.height);
	bitsperpixel = copyfrom.bitsperpixel;
	bytesperpixel = bitsperpixel/8;
	numcolors = copyfrom.numcolors;
	filetype = copyfrom.filetype;
	byteswidth = ((width*bitsperpixel + 31)/32)*4;				// number of bytes per line, padded to DWORD

	bmi = (BITMAPINFO *)GlobalAllocOrDie(sizeof(BITMAPINFOHEADER)+numcolors*sizeof(RGBQUAD));

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);			// initialize bmi
	bmi->bmiHeader.biWidth = width;
	bmi->bmiHeader.biHeight = height;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = bitsperpixel;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biXPelsPerMeter = 3780;
	bmi->bmiHeader.biYPelsPerMeter = 3780;							// set default 96dpi
	bmi->bmiHeader.biSizeImage = 0;
	bmi->bmiHeader.biClrUsed = 0;
	bmi->bmiHeader.biClrImportant = 0;

	for ( i=0; i<numcolors; i++ )								// retain original palette
		{		
		bmi->bmiColors[i].rgbBlue = copyfrom.bmi->bmiColors[i].rgbBlue;
		bmi->bmiColors[i].rgbRed = copyfrom.bmi->bmiColors[i].rgbRed;
		bmi->bmiColors[i].rgbGreen = copyfrom.bmi->bmiColors[i].rgbGreen;
		bmi->bmiColors[i].rgbReserved = 0;
		}

	bits = (BYTE *)GlobalAllocOrDie(byteswidth*height);			// allocate pixel memory

																// copy each pixel to destination...
	for (i=0; i<height; i++)
		{
		destbits = bits + i*byteswidth;							// address to ith row of scaled bitmap
		y = (int)(((double)i)/scale);
		srcbits = copyfrom.bits + y*copyfrom.byteswidth;		// address to yth row of original bitmap
		for (j=0; j<width; j++ )
			{
			x = (int)(((double)j)/scale);						// address actual pixel in source bitmap
			s = srcbits + x*bytesperpixel;
			for ( k=0; k<bytesperpixel; k++ )					// copy bytes of color info
				destbits[k] = s[k];										
			destbits += bytesperpixel;
			}
		}
}

void ADib::CopyBits( ADib *adib )			// copy bits to adib with similar DIB format
{											// e.g. from 32 or 24 bitsperpixel to 24 bits
	BYTE *destbits, *srcbits;
	int i, j, k, srcpixel, destpixel;
	
	srcpixel = this->bitsperpixel/8;
	destpixel = adib->bitsperpixel/8;
	for (i=0; i<height; i++)
		{
		destbits = adib->bits + i*adib->byteswidth;				// address to ith row of adib bitmap
		srcbits = this->bits + i*this->byteswidth;				// address to ith row of original bitmap
		for (j=0; j<width; j++ )
			{
			for ( k=0; k<destpixel; k++ )						// copy bytes of pixel
				destbits[k] = srcbits[k];										
			destbits += destpixel;
			srcbits += srcpixel;								// skip to next pixel
			}
		}
}

void ADib::Clear( void )							// clear entire ADib to zeros
{
	ZeroMemory( bits, byteswidth*height );
}

void ADib::Clear( RECT region )						// clear region of ADib to zeros
{
	BYTE *clearbits;
	int i, j, k, length, bytesperpixel;

	bytesperpixel = bitsperpixel/8;
	length = (region.right - region.left + 1 )*bytesperpixel;
	for (i=region.top; i<=region.bottom; i++)
		{
		clearbits = bits + i*byteswidth + region.left*bytesperpixel;	// address to ith row of region
		ZeroMemory(clearbits, length );
		}
}

int next_higher_pwr_of_2( int x )		// get next higher power of 2 if x not power of 2
{
	int i, j, y;
	i = 0; j = 0; y = x;
	do {											// count number of set bits (i) and total shifts (j)
	  if ( y & 1 ) i++;
	  j++;
	  }
	while ( (y>>=1) > 0 );
	if ( i <= 1 ) y = x;						// x is power of 2
	else {
		y = 1;
		for (i=0; i<j; i++) y<<=1;			// make y next higher power of 2
		}
	return( y );
}

Mvmt * ADib::Correlate( ADib *dib )			// compute correlation of *dib with this
{											// and return the offset to the peak
	Mvmt *offset;
	COMPLEX *data;
	float f, a, b, aN, bN;
	BYTE *d;
	int i, j, k, l, w, h, index, N;
	h = next_higher_pwr_of_2( height );				// use power of two
	w = next_higher_pwr_of_2( width );

	data = (COMPLEX *)GlobalAllocOrDie(w*h*sizeof(COMPLEX));	// allocate memory

	for (j=0; j<h; j++)								// zero memory
		{
		index = j*w;
		for (i=0; i<w; i++)
			{
			data[index].re = 0.0;
			data[index].im = 0.0;
			index++;
			}
		}
											
	for (j=0; j<height; j++)						// put this in real part of data array
		{
		d = this->bits + j * this->byteswidth;
		index = j*w;
		for (i=0; i<width; i++)						// if data present, use grayscale intensity
			{
			if ( d[3] )
				data[index].re =(float)d[0]/3.0+(float)d[1]/3.0+(float)d[2]/3.0 - 128.0;
			else data[index].re = 0.0;
			index++;
			d += 4;									// ASSUMES:  dib is 32-bit!
			}
		}

	for (j=0; j<height; j++)						// put *dib in imaginary part of data
		{
		d = dib->bits + j * dib->byteswidth;
		index = j*w;
		for (i=0; i<width; i++)						// if data present, use grayscale intensity
			{
			if ( d[3] )
				data[index].im =(float)d[0]/3.0+(float)d[1]/3.0+(float)d[2]/3.0 - 128.0;
			else data[index].im = 0.0;
			index++;
			d += 4;									// ASSUMES:  dib is 32-bit!
			}
		}

	forward_fft2f(data, h, w);						// fast fourier transform both adibs

	for (j=0; j<h; j++)								// compute the cross-correlation:
		{											// for the zero-frequncy bands...
		index = j*w;
		data[index].re = data[index].re*data[index].im;
		data[index].im = 0.0;
		}
	index = 1;										// ...this is just the product of
	for (i=1; i<w; i++)								// of the components
		{
		data[index].re = data[index].re*data[index].im;
		data[index].im = 0.0;
		index++;
		}
	for (j=1; j<=h/2; j++)							// for the rest of the frequencies
		{											// have to get the 2 separate FFTs
		index = j*w + 1;							// using symmetries (see NRinC, p.511)
		N = (h-j+1)*w - 1;							// and then cross multiply
		for (i=1; i<w; i++)
			{
			a = data[index].re;
			b = data[index].im;
			aN = data[N].re;
			bN = data[N].im;
			data[index].re = 0.5*(a*bN+aN*b);
			data[index].im = 0.25*(a*a+b*b-aN*aN-bN*bN);
			data[N].re = data[index].re;			// use real symmetry on cross-product
			data[N].im = -data[index].im;
			if ( index == N ) break;				// at this point, all elements have been computed
			index++;
			N--;
			}
		}

	inverse_fft2f(data, h, w );						// invert fourier transform

	f = 0.0; k = 0; l = 0;
	for (j=0; j<h; j++)								// find peak in correlation
		{
		index = j*w;
		for (i=0; i<w; i++)
			{
			if ( data[index].re > f ) 
				{
				f = data[index].re;
				k = i;
				l = j;
				}
			index++;
			}
		}
	if ( k >= w/2 ) k -= w;							// allow for negative offsets
	if ( l >= h/2 ) l -= h;

	free( data );									// free working memory

	offset = new Mvmt();							// return offset movement with correct sign
	offset->transX = -(double)k;
	offset->transY = -(double)l;
	return( offset );
}

void ADib::Shift( int x, int y )
{
	BYTE *destbits, *srcbits;
	int i, j, k, bytesincrement, bytesperpixel;
	int sx1, sx2, sxi, sy1, sy2, syi;

	if ( (x == 0) && (y == 0) ) return;
	
	if ( x <= 0 ) { sx1 = -x; sx2 = width; sxi = 1; }
	  else		{ sx1 = width-1-x; sx2 = -1; sxi = -1; }
	
	if ( y <= 0 ) { sy1 = -y; sy2 = height; syi = 1; }
	  else	    { sy1 = height-1-y; sy2 = -1; syi = -1; }

	bytesperpixel = bitsperpixel/8;
	bytesincrement = sxi*bytesperpixel;
	i = sy1;
	while ( i != sy2 )
		{
		srcbits = bits + i*byteswidth + sx1*bytesperpixel;
		destbits = bits + (i+y)*byteswidth + (sx1+x)*bytesperpixel;
		j = sx1;
		while ( j != sx2 )
			{
			for ( k=0; k<bytesperpixel; k++ )
				{
				destbits[k] = srcbits[k];
				}
			destbits += bytesincrement;
			srcbits += bytesincrement;										
			j += sxi;
			}
		i += syi;
		}
}


void ADib::Blend( ADib *src1, ADib *src2 )			// blend two 32-bit src dibs into this one
{													// ASSUMES: all dibs are 32-bit!
	BYTE *s1, *s2, *d;
	int i, j;
											
	for (j=0; j<height; j++)						// go through all pixels of this Adib
		{
		d = this->bits + j * this->byteswidth;		// set pointers to start of rows
		s1 = src1->bits + j * src1->byteswidth;
		s2 = src2->bits + j * src2->byteswidth;
		for (i=0; i<width; i++)
			{										// blend RGB from array data
			d[0] = s1[0]/2 + s2[0]/2;
			d[1] = s1[1]/2 + s2[1]/2;
			d[2] = s1[2]/2 + s2[2]/2;
			d += 4;
			s1 += 4;
			s2 += 4;
			}
		}
}


											// colorize interior pixels in r with c

void ADib::Fill( Contour *contour, double pixel_size, RECT r, Color c )
{
	Point *p;
	Points *walls;
	bool outside;
	double ybelow, yabove;
	int i, j, y, linesize, num_pts, cl, cr, cg, cb, dr, dg, db, dc, dl;
	int *d, *line;
	double *xx, *xy;

	cr = (int)(c.r*255.0);					// get 8-bit color values and intensity level of c
	cg = (int)(c.g*255.0);
	cb = (int)(c.b*255.0);
	cl = (cr+cg+cb)/3;
											// get points where region transitions between inside/outside
	contour->Pixels( pixel_size );
	contour->AddPixelsAtBoundaries( (double)r.left, (double)r.right, (double)r.top, (double)r.bottom );
	walls = contour->WallsInRegion( (double)r.left, (double)r.right, (double)r.top, (double)r.bottom );
	
	num_pts = walls->Number();				// create arrays for wall points
	if ( num_pts )
	  {
	  xx = new double[num_pts+1];	
	  xy = new double[num_pts+1];

	  i = 1;								// put points into arrays for fast look up
	  p = walls->first;
	  while ( p )
		{
		xx[i] = p->x;
		xy[i] = p->y;
		i++;
		p = p->next;
		}

	  Sort( 1, num_pts, xy, xx );				// sort walls on y values
			
	  linesize = (int)(r.right-r.left)+1;		// create scanline for region
	  line = new int[linesize];

	  i = 1;									// start with lowest y-value	
	  for (y=r.top; y<=r.bottom; y++ )			// for each scanline, determine inside/outside pixels
		{
		for (j=0; j<linesize; j++) line[j] = 0;	// clear scanline
		ybelow = (double)y - 0.5;
		yabove = (double)y + 0.5;				// find this y value
		while ( (xy[i] < ybelow) && (i<=num_pts) ) i++;
		while ( (xy[i] < yabove) && (i<=num_pts) )
			{
			j = (int)(xx[i]) - r.left;			// count walls on line
			line[j]++;
			i++;
			}
												// label pixels between walls...
		d = (int *)bits + y*width + r.left;
		outside = true;
		for (j=0; j<linesize; j++)
			{
			if ( line[j]%2 )					// odd combinations of walls switches in/out of line
				if ( !outside ) outside = true;	
				else outside = false;	
			if ( !outside )						// if inside contour...
				{
				dc = *d;
				dr = (dc&0xFF0000)>>16;
				dg = (dc&0x00FF00)>>8;
				db = (dc&0x0000FF);
				dl = (dr+dg+db)/3;
				if ( dl < cl )					// ...colorize interior pixel
					{
					dr = cr*dl/cl;
					dg = cg*dl/cl;
					db = cb*dl/cl;
					}
				else {
					dr = cr;
					dg = cg;
					db = cb;					// but keep index byte unmodified
					}
				*d = (dc&0xFF000000)|(dr<<16)|(dg<<8)|db;
				}
			d++;								// increment to next pixel in scanline
			}
		}

	  delete [] line;							// clean up memory used for fill
	  delete [] xx;
	  delete [] xy;
	  }

	delete walls;
}
											// label interior pixels in r with index

RECT ADib::Mask( Contour *contour, double pixel_size, int index, RECT r )
{
	Point *p;
	Points *walls;
	RECT masked;
	bool outside;
	double ybelow, yabove;
	int i, j, y, linesize, num_pts, jmin, jmax;
	int *d, *line;
	double *xx, *xy;

											// get points where region transitions between inside/outside
	contour->Pixels( pixel_size );
	contour->AddPixelsAtBoundaries( (double)r.left, (double)r.right, (double)r.top, (double)r.bottom );
	walls = contour->WallsInRegion( (double)r.left, (double)r.right, (double)r.top, (double)r.bottom );
	
	masked.left = 1;
	masked.right = -1;
	num_pts = walls->Number();				// create arrays for wall points
	if ( num_pts )
	  {
	  xx = new double[num_pts+1];	
	  xy = new double[num_pts+1];

	  i = 1;								// put points into arrays for fast look up
	  p = walls->first;
	  while ( p )
		{
		xx[i] = p->x;
		xy[i] = p->y;
		i++;
		p = p->next;
		}

	  Sort( 1, num_pts, xy, xx );				// sort walls on y values
			
	  linesize = (int)(r.right-r.left)+1;		// create scanline for region
	  line = new int[linesize];
	  jmax = -1;
	  jmin = linesize+1;						// initially no region masked
	  masked.bottom = r.top-1;
	  masked.top = r.bottom+1;
	  i = 1;									// start with lowest y-value	
	  for (y=r.top; y<=r.bottom; y++ )			// for each scanline, determine inside/outside pixels
		{
		for (j=0; j<linesize; j++) line[j] = 0;	// clear scanline
		ybelow = (double)y - 0.5;
		yabove = (double)y + 0.5;				// find this y value
		while ( (xy[i] < ybelow) && (i<=num_pts) ) i++;
		while ( (xy[i] < yabove) && (i<=num_pts) )
			{
			j = (int)(xx[i]) - r.left;			// count walls on line
			line[j]++;
			i++;
			}
												// label pixels between walls...
		d = (int *)bits + y*width + r.left;
		outside = true;
		for (j=0; j<linesize; j++)
			{
			if ( line[j]%2 )					// odd combinations of walls switches in/out of line
				if ( !outside ) outside = true;	
				else outside = false;	
			if ( !outside )						// if inside contour
				{
				*d = (index<<24);				// store index in high byte of 32-bit pixel
				if ( y < masked.top ) masked.top = y;
				if ( y > masked.bottom ) masked.bottom = y;
				if ( j < jmin ) jmin = j;
				if ( j > jmax ) jmax = j;
				}
			d++;								// increment to next pixel in scanline
			}
		}
	  masked.left = jmin + r.left;
	  masked.right = jmax + r.left;

	  delete [] line;							// clean up memory used for fill
	  delete [] xx;
	  delete [] xy;
	  }

	delete walls;

	return masked;

}

void ADib::MaskOutBorder( Contour *contour, double pixel_size, int ContourMaskWidth ) // parameter ContourMaskWidth added
{
	Point *p, min, max;						// set pixels that lie on contours pts to index zero
	int *d, *dbits, x, y;					// contour WILL BE MODIFIED BY THIS ROUTINE so make sure its a copy!

	contour->Pixels( pixel_size );			// convert into pixels
	contour->Extent( &min, &max );			// but before chain code check if it is visible
	if ( (max.x < 0) || (min.x >= width) || (max.y < 0) || (max.y >= height) ) return;
	contour->ChainCode( 1.0 );				// now add all 8-connected pixels

	if ( contour->points )					// label pixels with zero index
		{
		dbits = (int *)bits;
		p = contour->points->first;	
		while ( p != NULL )					// label each pixel
			{
			x = (int)(p->x);
			y = (int)(p->y);
			if ( (x>=0) && (x<width) && (y>=0) && (y<height) )	// only valid if in bitmap
			{
				if (ContourMaskWidth)	// if ContourMaskWidth == 0, only mask out the pixels on the contour
				{						// otherwise, mask out ContourMaskWidth on both sides of the pixel
						int nx, ny;
						for (int i=-ContourMaskWidth; i<=ContourMaskWidth ;i++ )	// maybe need ContourMaskWidth*pixel_size? 
						{
							for (int j=-ContourMaskWidth; j<=ContourMaskWidth ;j++ )
							{
								nx = x+i;
								ny = y+j;
								if ((nx>=0) && (nx<width) && (ny>=0) && (ny<height))
								{
									d = dbits + ny*width + nx;
									*d = (*d)&0x00ffffff;		// clear index in high-byte of 32-bit pixel
								}
							}
						}
				}
				else
				{
					//printf("ContourMaskWidth=%d",ContourMaskWidth);
					d = dbits + y*width + x;
					*d = (*d)&0x00ffffff;		// clear index in high-byte of 32-bit pixel
				}
			}
			p = p->next;
			}
		}
}

int ADib::GetPixel( int x, int y )				// return the index + RBG values at point x, y
{
	int *dbits, *d;
												// no limit checking! to use this routine, check x,y beforehand
	dbits = (int *)bits;
	d = dbits + y*width + x;
	return( *d );
}

void ADib::SetIndex( int x, int y, BYTE index )	// set index w/o changing RGB values at point x, y
{
	int *dbits, *d, rgb;

	dbits = (int *)bits;
	d = dbits + y*width + x;
	rgb = (*d)&0x00ffffff;
	*d = (index<<24) + rgb;
}

int ADib::GetIndex( int x, int y )				// return the index (in least sig. byte) at point x, y
{
	int *dbits, *d, dd;
	int index;
	if ( (x>=0) && (x<width) && (y>=0) && (y<height) )	// only valid if (x,y) in bitmap
		{
		dbits = (int *)bits;
		d = dbits + y*width + x;
		dd = *d;
		index = ((dd&0xff000000)>>24);			// move index into lower 8-bits of int
		}
	else index = 0;
	return( index );							// return zero if not legal (x,y)
}


int RGB2HSB( int rgb )			// return hue-saturation-lightness (from msdn.com code)
{
	int R, G, B, H, S, L, cMax, cMin, Rdelta, Gdelta, Bdelta;

	R = (rgb&0x00ff0000)>>16;
	G = (rgb&0x0000ff00)>>8;
	B = (rgb&0x000000ff);									
	cMax = max( max(R,G), B);							// calculate brightness
	cMin = min( min(R,G), B);
	L = ( ((cMax+cMin)*HLSMAX) + RGBMAX )/(2*RGBMAX);

	if ( cMax == cMin )									// r=g=b so greyscale
		{
		S = 0;											// set saturation to zero
		H = 255;										// but hue is undefined
		}
	else {												// otherwise calculate saturation & hue
		if (L <= (HLSMAX/2))
			S = (((cMax-cMin)*HLSMAX)+((cMax+cMin)/2))/(cMax+cMin);
		else
			S = (((cMax-cMin)*HLSMAX)+((2*RGBMAX-cMax-cMin)/2))/(2*RGBMAX-cMax-cMin);

		Rdelta = ( ((cMax-R)*(HLSMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin);
		Gdelta = ( ((cMax-G)*(HLSMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin);
		Bdelta = ( ((cMax-B)*(HLSMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin);

		if (R == cMax) H = Bdelta - Gdelta;
		else if (G == cMax) H = (HLSMAX/3) + Rdelta - Bdelta;
		else H = ((2*HLSMAX)/3) + Gdelta - Rdelta;

		if (H < 0) H += HLSMAX;
		if (H > HLSMAX) H -= HLSMAX;
		}										// return h,s,b values as integer
	return ( L|(S<<8)|(H<<16) );	
}

int ADib::GetHSBPixel( int x, int y )		// return hus, saturation, brightness at point x, y
{
	int *dbits, *d, dd;
	int hsb;
	if ( (x>=0) && (x<width) && (y>=0) && (y<height) )	// only valid if (x,y) in bitmap
		{
		dbits = (int *)bits;
		d = dbits + y*width + x;
		dd = *d;
		hsb = RGB2HSB(dd);					// put 3 bytes of HSB into integer's lower 24 bits
		}
	else hsb = 0x00ffffff;
	return( hsb );							// return all 255 if not legal (x,y)
}

													// transform an Image into this bitmap using mask byte

void ADib::MaskXform( Image *image, Nform *nform, int index, POINT min, POINT max,
							double dest_pixel_size, double dest_offset_x, double dest_offset_y, bool use_proxy )
{
	BYTE *sbits, *s, sindex;
	int *dbits, *d, id, r, g, b, color_mask;
	RGBQUAD *colors;
	int sbytesperpixel, sbyteswidth, swidth, sheight, brightness, snumcolors;
	int i, j, k, x, y, dim;
	double u, v, off_x, off_y, src_pixel_size, contrast;
	double av, a1, a3v, a4, bv, b1, b3v, b4;
// begin debug logging...
	DWORD byteswritten;
	char debugline[MAX_PATH];
	if ( debugLogFile != INVALID_HANDLE_VALUE )
		{
		sprintf(debugline,"Entered ADib::MaskXform\r\n");
		WriteFile(debugLogFile, debugline, strlen(debugline), &byteswritten, NULL);
		}
// ...end debug logging
			
	if ( !image ) return;
	color_mask = 0xffffffff;							// set color channels that will be displayed
	if ( !image->red )   color_mask = color_mask&0xff00ffff;
	if ( !image->green ) color_mask = color_mask&0xffff00ff;
	if ( !image->blue )  color_mask = color_mask&0xffffff00;
	dbits = (int *)this->bits;							// convert complex pointers to local variables
	sbits = NULL;										// will fill this later if index is found
	contrast = 1.0;										// set parameters for contrast...
	if ( fabs(image->contrast-1.0) > 0.02 )				
		contrast = image->contrast;
	brightness = 0;
	if ( fabs(image->brightness) > 0.01 )				// ... and brightness adjustments
		brightness = (int)(255.0*image->brightness);
	colors = NULL;
	dim = nform->dim;									// put partial computations into local variables
	a1 = nform->a[1]; b1 = nform->b[1];
	a4 = nform->a[4]; b4 = nform->b[4];
	off_x = dest_offset_x + dest_pixel_size*0.5;
	off_y = dest_offset_y + dest_pixel_size*0.5;	
	
	for (j=min.y; j<=max.y; j++)					// Modify all relevant pixels of this Adib
		{
		d = dbits + j*width + min.x;
		v = off_y + dest_pixel_size*(double)j;			// precompute y-term of dest pixel coord.
		if ( dim > 3 )
			{
			av = nform->a[0] + (nform->a[2]+nform->a[5]*v)*v;
			a3v = nform->a[3]*v;
			bv = nform->b[0] + (nform->b[2]+nform->b[5]*v)*v;
			b3v = nform->b[3]*v;
			}
		else											// if only affine terms needs, disregard quadratic
			{
			av = nform->a[0] + nform->a[2]*v;
			bv = nform->b[0] + nform->b[2]*v;
			}

		for (i=min.x; i<=max.x; i++)				// do each row
			{
			id = (*d&0xff000000)>>24;					// extract index byte to see if this image is here
			if ( id == index )							// found a pixel of image!
				{				
				if ( !sbits )						// 1. First time only, load and configure src image
					{
					if ( image->LoadAdib(use_proxy,dest_pixel_size) ) ;	// try to load adib into memory
					else return;						// if unsuccessful (no image) abort routine
					sbits = image->adib->bits;			// convert complex pointers to local variables for speed
					snumcolors = image->adib->numcolors;
					swidth = image->adib->width;
					sheight = image->adib->height;
					sbyteswidth = image->adib->byteswidth;
					sbytesperpixel = image->adib->bitsperpixel/8;
					if ( image->loaded == PROXY ) src_pixel_size = image->mag/image->proxyScale;
					else src_pixel_size = image->mag;
						
					if ( snumcolors )					// if have color table, modify brightness, contrast
						{
						colors = new RGBQUAD[snumcolors];
						for (k=0; k<snumcolors; k++)
							{
							r = image->adib->bmi->bmiColors[k].rgbRed;
							g = image->adib->bmi->bmiColors[k].rgbGreen;
							b = image->adib->bmi->bmiColors[k].rgbBlue;
							r = (int)( (double)r*contrast ) + brightness;
							if ( r < 0 ) r = 0;
							if ( r > 255 ) r = 255;
							g = (int)( (double)g*contrast ) + brightness;
							if ( g < 0 ) g = 0;
							if ( g > 255 ) g = 255;
							b = (int)( (double)b*contrast ) + brightness;
							if ( b < 0 ) b = 0;
							if ( b > 255 ) b = 255;
							colors[k].rgbRed = (BYTE)r;
							colors[k].rgbGreen = (BYTE)g;
							colors[k].rgbBlue = (BYTE)b;
							}		
						}
					else {								// otherwise create a brightness/contrast table for 8-bit colors
						snumcolors = 256;
						colors = new RGBQUAD[snumcolors];
						for (k=0; k<snumcolors; k++)
							{
							r = (int)( (double)k*contrast ) + brightness;
							if ( r < 0 ) r = 0;
							if ( r > 255 ) r = 255;		// contrast has the same effect on green and blue channels
							colors[k].rgbRed = (BYTE)r;
							colors[k].rgbGreen = (BYTE)r;
							colors[k].rgbBlue = (BYTE)r;
							}				
						}
					}
														// 2. Image processing:
														
				u = off_x + dest_pixel_size*(double)i;		// compute dest pixel x-coord.
				if ( dim > 3 )								// compute corresponding source pixel coord.
					{
					x = (int)((av+(a1+a3v+a4*u)*u)/src_pixel_size);
					y = (int)((bv+(b1+b3v+b4*u)*u)/src_pixel_size);
					}
				else if ( dim > 0 )							// affine only transform
					{
					x = (int)((av+a1*u)/src_pixel_size);
					y = (int)((bv+b1*u)/src_pixel_size);
					}
				else										// no transform (identity)
					{
					x = (int)(u/src_pixel_size);
					y = (int)(v/src_pixel_size);
					}
				if ( (x>=0) && (x<swidth) && (y>=0) && (y<sheight) )	// only valid if in bitmap
					{													// ONLY HERE DO YOU NEED TO LOAD IT!
					s = sbits + y * sbyteswidth + x * sbytesperpixel;
					if ( sbytesperpixel < 3 )
						{
						sindex = *s;								// have color table, look up RGB values
						r = colors[sindex].rgbRed;
						g = colors[sindex].rgbGreen;
						b = colors[sindex].rgbBlue;
						}
					else {											// get RGB from array data
						b = colors[s[0]].rgbRed;
						g = colors[s[1]].rgbGreen;
						r = colors[s[2]].rgbBlue;
						}
					*d = color_mask&(b|(g<<8)|(r<<16)|(id<<24));   // set color bytes
					}
				}
			d++;
			}		// do next pixel in row
		
		}		// do next row

	if ( colors ) delete[] colors;								// clean up allocation of color table

}

bool ADib::SaveADibAsBMP( const char * fn )					// save bitmap at whatever color width
{
	BITMAPFILEHEADER bmfh;
	DWORD byteswritten, error;
	HANDLE hFile;
	int i;
	unsigned char * nextrow;
	bool ErrorOccurred;
	char errmsg[1024];

	if ( (bmi == NULL) || (bits == NULL) ) return FALSE;

	ErrorOccurred = false;
	errmsg[0] = '\0';													// attempt to create file

	hFile = CreateFile( fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		{
		ErrorOccurred = true;
		error = GetLastError();
		}

	if ( !ErrorOccurred )
		{
		bmfh.bfType = 0x4d42;											// = "BM"
		bmfh.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER)					// file header size
										+ sizeof(BITMAPINFOHEADER)		// BITMAPINFO size
										+ numcolors*sizeof(RGBQUAD)		// color table size
										+ byteswidth*height );			// image bits size
		bmfh.bfReserved1 = 0;
		bmfh.bfReserved2 = 0;
		bmfh.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER)				// file header size
										+ sizeof(BITMAPINFOHEADER)		// BITMAPINFO size
										+ numcolors*sizeof(RGBQUAD) );	// color table size

																		// write the header info

		if ( !WriteFile(hFile, &bmfh, sizeof(BITMAPFILEHEADER), &byteswritten, NULL) )
			{
			ErrorOccurred = true;
			error = GetLastError();
			}
																		// write BITMAPINFO
		if ( !WriteFile(hFile, bmi, sizeof(BITMAPINFOHEADER)
									+ numcolors*sizeof(RGBQUAD), &byteswritten, NULL) )
			{
			ErrorOccurred = true;
			error = GetLastError();
			}
																		// write the bits
		for ( i=0; i<bmi->bmiHeader.biHeight; i++ )
			{
			nextrow = bits + i*byteswidth;
			if ( !WriteFile(hFile, nextrow, byteswidth, &byteswritten, NULL) )
				{
				ErrorOccurred = true;
				error = GetLastError();
				break;
				}
			}

		CloseHandle( hFile );
		}
											// MAKE ERROR MSG OPTIONAL FOR MULTISECTION OPERATIONS?
	if ( ErrorOccurred )
		{																// format error string
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
    						NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    						errmsg, 512, NULL );
		strcat(errmsg,"\nError occurred writing file:\n");
		strcat(errmsg,fn);
		ErrMsgOK( ERRMSG_WRITE_FAILED, errmsg );
		return( false );
		}

	return( true );
}
/*
void ADib::Crop( int x, int y, int w, int h )		// shrink to subimage of w X h starting at x,y
{
	int i, j, stride, start;
	unsigned char *data, *src, *dest;
	start = ( ((x * bitsperpixel + 31)/32) * 4 );		// create buffer to store subimage
	stride = ( ((w * bitsperpixel + 31)/32) * 4 );
	data = (BYTE *)GlobalAllocOrDie(stride * h);

	for ( j = 0; j < h; j++ )
		{												// copy each scanline...
		src = bits + (j+y) * byteswidth + start;
		dest = data + j * stride;
		for ( i = 0; i < stride; i++ )
			*dest++ = *src++;
		}

	if ( bits ) free( bits );							// delete old bits representation
	bits = data;
														// update bmi...
	bmi->bmiHeader.biWidth = w;
	bmi->bmiHeader.biHeight = h;
	width = w;											// ... and ADib properties
	height = h;
	byteswidth = stride;
}
*/
void ADib::ConvertUpTo24bit( void )
{														// convert to 24-bit DIB
	int x, y, w, h, stride;
	unsigned char c, *data, *src, *dest;
														// if src is < 24 bpp, expand to 24-bit
	if ( bitsperpixel < 24 )
		{
		w = width;
		h = height;
		stride = ( ((w * 24 + 31)/32) * 4 );
		data = (BYTE *)GlobalAllocOrDie(stride * h);

		for ( y = 0; y < h; y++ )
			{												// do each scanline...
			src = bits + y * byteswidth;
			dest = data + y * stride;
			switch ( bitsperpixel )
				{
				case 1:										// average color components from palette
					for ( x = 0; x < w; x++ )
						{
						if ( (x & 7) == 0 ) c = *src++;
						else c <<= 1;
						*dest++ = (unsigned char)bmi->bmiColors[c >> 7].rgbBlue;
						*dest++ = (unsigned char)bmi->bmiColors[c >> 7].rgbGreen;
						*dest++ = (unsigned char)bmi->bmiColors[c >> 7].rgbRed;
						}
					break;

				case 4:
					for ( x = 0; x + 1 < w; x += 2 )
						{
						c = *src++;
						*dest++ = (unsigned char)bmi->bmiColors[c >> 4].rgbBlue;
						*dest++ = (unsigned char)bmi->bmiColors[c >> 4].rgbGreen;
						*dest++ = (unsigned char)bmi->bmiColors[c >> 4].rgbRed;
						*dest++ = (unsigned char)bmi->bmiColors[c & 15].rgbBlue;
						*dest++ = (unsigned char)bmi->bmiColors[c & 15].rgbGreen;
						*dest++ = (unsigned char)bmi->bmiColors[c & 15].rgbRed;
						}
					if ( x < w )
						{
						c = *src;
						*dest++ = (unsigned char)bmi->bmiColors[c >> 4].rgbBlue;
						*dest++ = (unsigned char)bmi->bmiColors[c >> 4].rgbGreen;
						*dest = (unsigned char)bmi->bmiColors[c >> 4].rgbRed;
						}
					break;

				case 8:
					for ( x = 0; x < w; x++ )
						{
						c = *src++;
						*dest++ = (unsigned char)bmi->bmiColors[c].rgbBlue;
						*dest++ = (unsigned char)bmi->bmiColors[c].rgbGreen;
						*dest++ = (unsigned char)bmi->bmiColors[c].rgbRed;
						}
					break;
				}		// end switch
			}		// end for


		if ( bits ) free( bits );							// delete old representations
		bits = data;
		if ( bmi ) free( bmi );
															// now create new 24-bit bmi...
		numcolors = 0;
		bitsperpixel = 24;
		byteswidth = stride;
		bmi = (BITMAPINFO *)GlobalAllocOrDie(sizeof(BITMAPINFOHEADER));

		bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi->bmiHeader.biWidth = w;
		bmi->bmiHeader.biHeight = h;
		bmi->bmiHeader.biPlanes = 1;
		bmi->bmiHeader.biBitCount = bitsperpixel;
		bmi->bmiHeader.biCompression = BI_RGB;
		bmi->bmiHeader.biXPelsPerMeter = 3780;
		bmi->bmiHeader.biYPelsPerMeter = 3780;							// set default 96dpi
		bmi->bmiHeader.biSizeImage = 0;
		bmi->bmiHeader.biClrUsed = 0;
		bmi->bmiHeader.biClrImportant = 0;
		}
}

bool ADib::SaveADibAsJPEG( const char *fn, int quality )		// save ADIB as JPEG using gbm library
{																// NOTE: gbm only supports 24-bit saves
	int i, fd, ft, num_colors;
	GBM gbm;
	GBMRGB *gbmrgb;
	char opt[64];
	char errmsg[MAX_PATH];

	if ( (fd = gbm_io_create(fn, O_WRONLY|O_BINARY)) == -1 )
		{
		sprintf(errmsg,"Can't create %s", fn);
		ErrMsgOK( ERRMSG_WRITE_FAILED, errmsg );
		return false;
		}

	gbm.bpp = bmi->bmiHeader.biBitCount;
	gbm.w = bmi->bmiHeader.biWidth;
	gbm.h = bmi->bmiHeader.biHeight;
	gbmrgb = NULL;									// bpp=24 therefore no color table!

	sprintf(opt,"quality=%d",quality);
	ft = 16;										// JPEG format is type 16

	if ( (i = gbm_write(fn, fd, ft, &gbm, gbmrgb, bits, opt)) != GBM_ERR_OK )
		{
		ErrMsgOK( ERRMSG_WRITE_FAILED, gbm_err(i) );
		gbm_io_close(fd);
		return false;
		}

	gbm_io_close(fd);

	delete[] gbmrgb;

	return true;									// success!
}


											// set criteria for region similarity to target color

bool StopGrowing( int x, int y, RECT limits, int hue, int sat, int bright, ADib *adib, int* pxl_hue, int* pxl_sat, int* pxl_bright)
{
	int c, index, h, s, b, hh;									// point must be inside limits
	bool hStop, sStop, bStop;
	if ( (x<limits.left) || (x>limits.right) || (y<limits.bottom) || (y>limits.top) ) return( true );
	c = adib->GetPixel( x, y );
	index = (c&0xff000000)>>24;								// pt must belong to an image region
	if ( (index > 0) && (index < MAX_DOMAINS) )				// and must not have been visited already
		{
		c = RGB2HSB( c );									// calculate hue-saturation-lightness from RGB
		h = (c&0x00ff0000)>>16;								// check hsb for region growth
		s = (c&0x0000ff00)>>8;
		b = (c&0x000000ff);
		//if ( h == 255 ) h = hue;							// for invalid hue use origin's hue?

		*pxl_hue = h;
		*pxl_sat = s;
		*pxl_bright = b;

		switch (CurrSeries->hueStopWhen)					// test hue color
			{
			case 0: hStop = (h < CurrSeries->hueStopValue);		// stop when less than
					break;
			case 1: hh = abs(h-CurrSeries->hueStopValue);		// stop when equals about
					if ( hh > HALFHUE ) hh = HLSMAX - hh;
					hStop = (hh > 5);
					break;
			case 2: hStop = (h > CurrSeries->hueStopValue);		// stop when greater than
					break;
			case 3: hh = abs(h-hue);							// stop when differs by
					if ( hh > HALFHUE ) hh = HLSMAX - hh;
					hStop = (hh >= CurrSeries->hueStopValue);
					break;
			default: hStop = (h < CurrSeries->hueStopValue);	// stop when less than
			}
		switch (CurrSeries->satStopWhen)					// test saturation
			{
			case 0: sStop = (s < CurrSeries->satStopValue);		// stop when less than
					break;	
			case 1: sStop = (abs(s-CurrSeries->satStopValue)>5);// stop when equals about
					break;
			case 2: sStop = (s > CurrSeries->satStopValue);		// stop when greater than
					break;
			case 3: sStop = (abs(s - sat) >= CurrSeries->satStopValue);	// differs by
					break;
			default: sStop = (s < CurrSeries->satStopValue);	// stop when less than
			}
		switch (CurrSeries->brightStopWhen)					// test intensity
			{
			case 0: bStop = (b < CurrSeries->brightStopValue);	// stop when less than
					break;
			case 1: bStop = (abs(b-CurrSeries->brightStopValue)>5);// stop when equals about
					break;
			case 2: bStop = (b > CurrSeries->brightStopValue);	// stop when greater than
					break;
			case 3: bStop = (abs(b - bright) >= CurrSeries->brightStopValue); // differs by
					break;
			default: bStop = (b < CurrSeries->brightStopValue);	// stop when less than
			}

		return( hStop || sStop || bStop );	// stop when any color channels say stop
		}
	return( true );
}

void next_clkwise_neighbor( int *x, int *y )	// return increments to next clockwise neighbor
{													// (-1,1)  (0,1)  (1,1)  
	if ((*x==-1) && (*y==-1))     {*x=-1; *y=0;}	// (-1,0)  (0,0)  (1,0)  
	else if ((*x==-1) && (*y==0)) {*x=-1; *y=1;}	// (-1,-1) (0,-1) (1,-1) 
	else if ((*x==-1) && (*y==1)) {*x=0; *y=1;}
	else if ((*x==0) && (*y==1))  {*x=1; *y=1;}
	else if ((*x==1) && (*y==1))  {*x=1; *y=0;}
	else if ((*x==1) && (*y==0))  {*x=1; *y=-1;}
	else if ((*x==1) && (*y==-1)) {*x=0; *y=-1;}
	else						  {*x=-1; *y=-1;}
}


Contour * ADib::Wildfire( int x, int y, double min_size )	// grow contour starting from the pixel location (x,y)
{															// this ADib is view that may be masked by contours					
	Histogram *histogram;
	Contour *boundary, *contour, *c;
	Point *np;
	XYPoints *frontier, *nextfront;
	XYPoint *p, *n;
	int px, py, nx, ny, lx, ly, cx, cy, xmax, xmin, ymax, ymin;
	int i, j, s, hue, sat, bright, pxl_hue, pxl_sat, pxl_bright, pixels;
	RECT limits;
	bool changing;

	boundary = NULL;						// return NULL if fail
	limits.left = 3;
	limits.right = this->width-3;			// set slightly smaller limit region
	limits.bottom = 3;						// to prevent bitmap addressing errors
	limits.top = this->height-3;
	s = this->GetPixel(x,y);
	s = RGB2HSB( s );
	hue = (s&0x00ff0000)>>16;
	sat = (s&0x0000ff00)>>8;
	bright = (s&0x000000ff);
	
											// start region growing only if starting pixel is valid
	if ( !StopGrowing( x, y, limits, hue, sat, bright, this, &pxl_hue, &pxl_sat, &pxl_bright ) )	
		{
		histogram = new Histogram();					// need to keep histogram, so first create
		histogram->AddHSBPixel(pxl_hue, pxl_sat, pxl_bright);		
		pixels = 1;										// keep track of size of region for limit
		xmax = x;  xmin = x;  ymax = y;  ymin = y;
		this->SetIndex( x, y, MAX_DOMAINS );			// mark first pixel as part of growing region
		frontier = new XYPoints();						// create frontier points list
		p = new XYPoint( x, y );
		frontier->Add( p );
		changing = true;
		while ( changing )				// continue growing region as long as frontier is changing
			{
			changing = false;
			nextfront = new XYPoints();
			p = frontier->first;
			while ( p )					// use invalid domain value of MAX_DOMAINS to mark
				{						// visited pixels in region while growing 4-connected
				px = p->x;  py = p->y;
				nx = px + 1; ny = py;
				if ( !StopGrowing( nx, ny, limits, hue, sat, bright, this, &pxl_hue, &pxl_sat, &pxl_bright ))
					{
					changing = true;
					pixels++;
					if ( nx < xmin ) xmin = nx;
					if ( ny < ymin ) ymin = ny;
					if ( nx > xmax ) xmax = nx;
					if ( ny > ymax ) ymax = ny;
					this->SetIndex( nx, ny, MAX_DOMAINS );
					n = new XYPoint( nx, ny );
					nextfront->Add( n );
					if (AutoAdjustThreshold)
						histogram->AddHSBPixel(pxl_hue, pxl_sat, pxl_bright);
					}
				nx = px - 1; ny = py;
				if ( !StopGrowing( nx, ny, limits, hue, sat, bright, this, &pxl_hue, &pxl_sat, &pxl_bright ))
					{
					changing = true;
					pixels++;
					if ( nx < xmin ) xmin = nx;
					if ( ny < ymin ) ymin = ny;
					if ( nx > xmax ) xmax = nx;
					if ( ny > ymax ) ymax = ny;
					this->SetIndex( nx, ny, MAX_DOMAINS );
					n = new XYPoint( nx, ny );
					nextfront->Add( n );
					if (AutoAdjustThreshold)
						histogram->AddHSBPixel(pxl_hue, pxl_sat, pxl_bright);
					}
				nx = px; ny = py + 1;
				if ( !StopGrowing( nx, ny, limits, hue, sat, bright, this, &pxl_hue, &pxl_sat, &pxl_bright))
					{
					changing = true;
					pixels++;
					if ( nx < xmin ) xmin = nx;
					if ( ny < ymin ) ymin = ny;
					if ( nx > xmax ) xmax = nx;
					if ( ny > ymax ) ymax = ny;
					this->SetIndex( nx, ny, MAX_DOMAINS );
					n = new XYPoint( nx, ny );
					nextfront->Add( n );
					if (AutoAdjustThreshold)
						histogram->AddHSBPixel(pxl_hue, pxl_sat, pxl_bright);
					}
				nx = px; ny = py - 1;
				if ( !StopGrowing( nx, ny, limits, hue, sat, bright, this, &pxl_hue, &pxl_sat, &pxl_bright ))
					{
					changing = true;
					pixels++;
					if ( nx < xmin ) xmin = nx;
					if ( ny < ymin ) ymin = ny;
					if ( nx > xmax ) xmax = nx;
					if ( ny > ymax ) ymax = ny;
					this->SetIndex( nx, ny, MAX_DOMAINS );
					n = new XYPoint( nx, ny );
					nextfront->Add( n );
					if (AutoAdjustThreshold)
						histogram->AddHSBPixel(pxl_hue, pxl_sat, pxl_bright);
					}
				p = p->next;
				}
			delete frontier;									// done with this frontier
			frontier = nextfront;								// switch to next one
			}
		delete frontier;
		
		px = 0;  py = y; changing = true;						// find valid exterior pixel left of start
		while ( changing && (px < this->width) )				// need a wide, clear path to ensure closure later
			{
			if ( ( ((this->GetPixel(px+1,py)&0xff000000)>>24) < MAX_DOMAINS )
				 && ( ((this->GetPixel(px,py+1)&0xff000000)>>24) < MAX_DOMAINS )
				 && ( ((this->GetPixel(px,py-1)&0xff000000)>>24) < MAX_DOMAINS ) ) px++;
			else changing = false;
			}
		
		if ( px && ((double)pixels >= min_size))	// found exterior pixel for large enough region -- generate boundary
			{
			lx = px;  ly = py;
			i = -1;   j = -1;									// start search from the bottom of the clear path
			nx = lx + i; ny = ly + j;
			next_clkwise_neighbor( &i, &j );
			cx = lx + i; cy = ly + j;
			boundary = new Contour(); 							// trace contour from boundary of region
			boundary->closed = true;
			boundary->simplified = true;
			boundary->histogram = new Histogram(*histogram);	// copy histogram so can delete below
			boundary->points = new Points();
			np = new Point( px, py, 0 );						// px, py is the beginning of contour
			boundary->points->AddFirst( np );
			changing = true;									// keep looking for boundary pts until hit px,py
			while ( changing )
				{												// search clockwise relative to last contour point
				while ( ((this->GetPixel(cx,cy)&0xff000000)>>24) < MAX_DOMAINS )
					{
					nx = cx;									// nx, ny is an exterior pixel
					ny = cy;
					next_clkwise_neighbor( &i, &j );
					cx = lx + i;
					cy = ly + j;								// ii, jj might be interior
					}
				if ( (nx==px) && (ny==py) ) changing = false;	// joined back with beginning?
				if ( i==0 )
					if ( (ny-ly) == j )						// small loop is possible depending
						{									
						s = nx - lx;						// on location of next interior pt
						if ( s )
							if ( !(((this->GetPixel(lx+2*s,ly)&0xff000000)>>24) < MAX_DOMAINS) )
								{
								nx = lx + s; ny = ly;		// so lift boundary pt out of hole
								}
						}
				if ( j==0 )									// consider other orientation of hole
					if ( (nx-lx) == i )
						{									
						s = ny - ly;						// and location of next interior pt
						if ( s )
							if ( !(((this->GetPixel(lx,ly+2*s)&0xff000000)>>24) < MAX_DOMAINS) )
								{
								nx = lx; ny = ly + s;		// to lift boundary pt if necessary
								}
						}
				if ( (nx==px) && (ny==py) ) changing = false;	// double check since modified nx, ny
				else {
					np = new Point( nx, ny, 0 );					// add new boundary pt
					boundary->points->AddFirst( np );
					i = lx - nx; j = ly - ny;
					lx = nx;  ly = ny;								// remember pt search is around
					nx = lx + i; ny = ly + j;
					next_clkwise_neighbor( &i, &j );				// continue search for next boundary pt
					cx = lx + i; cy = ly + j;
					}
				}
			} // end if

		for (j=ymin; j<=ymax; j++)				// after finding boundary (or not), set index of region to zero
			for (i=xmin; i<=xmax; i++)										// so don't trace it again
				if ( ((this->GetPixel(i,j)&0xff000000)>>24) == MAX_DOMAINS )
					this->SetIndex( i, j, 0 );

		delete histogram;						// free memory
		}

	return( boundary );												// return the contour created (if any)
}

