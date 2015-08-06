////////////////////////////////////////////////////////////////////////
//	This file contains the methods for the ViewPort class
//
//    Copyright (C) 2002-2007  John Fiala (fiala@bu.edu)
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
// -+- change: Added Wildfire region growing routine.
// modified 11/12/04 by JCF (fiala@bu.edu)
// -+- change: Modified Wildfire stop criteria to be HSB instead of RGB.
// modified 04/21/05 by Ju Lu  (julu@fas.harvard.edu)
// -+- change: Added MaskADibWithContours(region) function in Wildfire so contours will avoid each other
//             Added DoAutoTrace() function and its auxiliary functions to execute automatic tracing
// modified 4/29/05 by JCF (fiala@bu.edu)
// -+- change: Cleaned Wildfire routine to fix version 1.0.4.4 bug and make stopAtTraces optional.
//             Moved DoAutoTrace routines to keyboard.cpp
// modified 6/8/05 by JCF (fiala@bu.edu)
// -+- change: Added WildfireRegion to trace all regions in view within bounding rectangle.
// modified 6/17/05 by JCF (fiala@bu.edu)
// -+- change: Added min_size param to WildfireRegion() and fixed bug that was crashing program.
// modified 6/29/05 by JCF (fiala@bu.edu)
// -+- change: Fixed single pixel area bug in Wildfire() and corrected min/ax bug in WildfireRegion().
// modified 7/14/05 by JCF (fiala@bu.edu)
// -+- change: Added ViewDCtoImage() and removed Capture().
// modified 7/15/05 by JCF (fiala@bu.edu)
// -+- change: FillContours() now doesn't display hidden contours.
// modified 8/12/05 by JCF (fiala@bu.edu)
// -+- change: Added error message with CreateCompatibleBitmap fails.
// modified 10/3/05 by JCF (fiala@bu.edu)
// -+- change: Removed general error message when viewport create canvas fails (not always a problem)
// modified 5/12/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified MaskOutBorder called by wildfire to accept ContourMaskWidth parameter
// modified 6/5/06 by JCF (fiala@bu.edu)
// -+- change: Added debug logging of operations for crash detection.
// modified 6/19/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified ViewPort::Wildfire to get Contour::AverageIntensity.
//             Added GetBrightness(x,y) to return the brightness of pixel (x,y) in adib. May put it as a member function in future
// modified 6/20/06 by JCF (fiala@bu.edu)
// -+- change: Added wrap around on hue parameter in StopGrowing() for wildfire.
// modified 6/22/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified ViewPort::Wildfire to use histogram instead of AverageIntensity.
//             Removed GetBrightness function. Modified StopGrowing to return (h,s,b) of the pixel. 
// modified 6/24/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Moved RGBMAX, HLSMAX to constant.h
// modified 7/1/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added PrepareADib function for Modified MaskOutBorder method to mask out not only existing contours on current adib, 
//  but also contours that do not belong to the same object in previous sections from current adib,
//  based on the assumption that different objects do not intersect.
//  This problem is not solved. How to find the ancestor contour?
// modified 7/7/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified Wildfire behavior to pass the prepared ADib externally. The reason of doing this is to avoid repeatedly generating 
//             ADib when multiautotracing. 
// modified 7/7/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Changed definition of Wildfire 
// modified 11/13/06 by JCF (fiala@bu.edu)
// -+- change: Eliminated compiler ERRORS in PrepareAdib declaration.
// modified 11/14/06 by JCF (fiala@bu.edu)
// -+- change: Made PrepareADib a member of Viewport and removed *series param since only possible value is CurrSeries. 
// -+- change: Renamed PrepareADib to more descriptive name: CopyView( maskContours ) with option to not mask contours
// -+- change: Move Wildfire methods to ADib since now, with CopyView removed, they have no ViewPort dependencies.
// modified 7/18/07 by JCF (fiala@bu.edu)
// -+- change: Fix error in GetDomainFromPixel by using new GetIndex() routine that tests x,y limits.

#include "reconstruct.h"

ViewPort::ViewPort( int w, int h )					// create canvasless ViewPort for image rendering
{
	viewDC = NULL;
	orig = canvas = NULL;
	width = w;
	height = h;
	section = NULL;
	needsRendering = false;
	needsDrawing = false;
	view = new ADib( width, height, IRGB_PIXELS );
}

ViewPort::ViewPort( HWND hWnd )
{
	RECT wrect;
	HDC winDC;

	winDC = GetDC( hWnd );							// create memory DC to reflect display window					
	viewDC = CreateCompatibleDC( winDC );
	GetClientRect( hWnd, &wrect );
	width = wrect.right-wrect.left;
	height = wrect.bottom-wrect.top;				// use window size for memory DC
	canvas = CreateCompatibleBitmap( winDC, width, height );
	if ( !canvas ) ErrMsgOK( ERRMSG_GDIFAILED, "Insufficient memory available." );
	ReleaseDC( hWnd, winDC );						// done with display DC for now

	orig = (HBITMAP)SelectObject( viewDC, canvas );	// select bitmap in DC to store view image

	view = new ADib( width, height, IRGB_PIXELS );	// create device-independent bitmap for view
	section = NULL;
	needsRendering = false;
	needsDrawing = false;
}

ViewPort::ViewPort( int w, int h, HWND hWnd )		// create arbitrary w,h DC based on hWnd's DC
{
	RECT wrect;
	HDC winDC;
	width = w;
	height = h;
	winDC = GetDC( hWnd );							// create memory DC for GDI drawing					
	viewDC = CreateCompatibleDC( winDC );
	canvas = CreateCompatibleBitmap( winDC, width, height );
	if ( !canvas ) ErrMsgOK( ERRMSG_GDIFAILED, "Insufficient memory available." );
	ReleaseDC( hWnd, winDC );						// done with display DC for now

	orig = (HBITMAP)SelectObject( viewDC, canvas );	// select bitmap in DC to store view image

	view = new ADib( width, height, IRGB_PIXELS );	// create device-independent bitmap for view
	section = NULL;
	needsRendering = false;
	needsDrawing = false;
}

ViewPort::~ViewPort()								// destructor frees dynamic memory objects
{													// don't delete section becuz will do so explicitly
	if ( view ) delete view;
	if ( canvas ) DeleteObject( canvas );
	if ( viewDC ) DeleteDC( viewDC );	
}

HBITMAP ViewPort::ReleaseCanvas( void )	// release and return the canvas BITMAP
{
	HBITMAP tmpbmp;
	SelectObject( viewDC, orig );		// select orig bitmap into DC to make canvas available elsewhere
	tmpbmp = canvas;
	canvas = NULL;						// don't delete canvas but null this pointer for destructor
	return tmpbmp;
}

void ViewPort::Resize( int w, int h )							// resize the viewport to the window's client area
{
	if ( (w != width) || (h != height) )
		{											// trash old bitmap constructions since are wrong size
		if ( view ) delete view;
		if ( canvas ) DeleteObject( canvas );
		width = w;
		height = h;									// recreate at new size
		needsRendering = true;					
		canvas = CreateCompatibleBitmap( viewDC, width, height );
		if ( !canvas ) ErrMsgOK( ERRMSG_GDIFAILED, "Insufficient memory available." );
		SelectObject( viewDC, canvas );
		view = new ADib( width, height, IRGB_PIXELS );	// create device-independent bitmap for view
		}
}

bool ViewPort::ViewDCToImage( void )			// copy from canvas (where drawings are) back into image DIB
{												// this can fail if canvas bitmap is too big, so return status
	if ( viewDC )
		if ( GetDIBits( viewDC, canvas, 0, height, view->bits, view->bmi, DIB_RGB_COLORS) )
			return true;
	return false;
}

void ViewPort::Display( HDC dc, RECT region )		// copy region of viewDC to display
{
	if ( viewDC )													// make sure region not empty
	  if ( region.right && region.bottom )
		BitBlt( dc, region.left, region.top, region.right-region.left, region.bottom-region.top,
																viewDC,  region.left, region.top, SRCCOPY );
	  else
		BitBlt( dc, 0, 0, width, height, viewDC, 0, 0, SRCCOPY );	// if no region, do whole view
}

void ViewPort::Zoom( HDC dc, double scale, int x, int y )	// stretch viewDC to display according to scale
{
	int dx, dy, dw, dh, sx, sy, sw, sh;
	if ( scale <= 1.0 )
		{
		sx = sy = 0;
		sw = width;
		sh = height;
		dw = (int)floor( scale*(double)width );
		dh = (int)floor( scale*(double)height );
		dx = (int)floor( (1.0-scale)*(double)x );
		dy = (int)floor( (1.0-scale)*(double)y );
		BitBlt( dc, 0, 0, width, dy, NULL, 0, 0, BLACKNESS );				// clear border pixels
		BitBlt( dc, 0, dy, dx, dh, NULL, 0, 0, BLACKNESS );
		BitBlt( dc, dx+dw, dy, width-(dx+dw), dh, NULL, 0, 0, BLACKNESS );
		BitBlt( dc, 0, dy+dh, width, height-(dy+dh), NULL, 0, 0, BLACKNESS );
		}
	else {
		sw = (int)floor( (double)width/scale );
		sh = (int)floor( (double)height/scale );
		sx = (int)floor( (scale-1.0)*(double)x/scale );
		sy = (int)floor( (scale-1.0)*(double)y/scale );
		dx = dy = 0;
		dw = width;
		dh = height;
		}
	if ( viewDC )															// copy view pixels
		StretchBlt( dc, dx, dy, dw, dh, viewDC, sx, sy, sw, sh, SRCCOPY );
}

void ViewPort::Pan( HDC dc, int x, int y )	// shift viewDC according to x,y offset
{
	int dx, dy, dw, dh, sx, sy, bx, bw, by, bh;
	if ( x <= 0 ) { sx = -x; dx = 0; bx = width + x; bw = -x; }
	else { sx = 0; dx = x; bx = 0; bw = x; }
	dw = width - x;
	if ( y <= 0 ) { sy = -y; dy = 0; by = height + y; bh = -y; }
	else { sy = 0; dy = y; by = 0; bh = y; }
	dh = height - y;
	BitBlt( dc, bx, 0, bw, height, NULL, 0, 0, BLACKNESS );				// clear border pixels
	BitBlt( dc, 0, by, width, bh, NULL, 0, 0, BLACKNESS );
	if ( viewDC )
		BitBlt( dc, dx, dy, dw, dh, viewDC, sx, sy, SRCCOPY );			// copy view pixels
}

void ViewPort::ImageToViewDC( void )			// copies the already rendered view to DC
{
	if ( view->bmi && viewDC )						// do only if have necessary ingredients
		SetDIBitsToDevice( viewDC, 0, 0, view->width, view->height,
								0, 0, 0, view->height, view->bits,
								view->bmi, DIB_RGB_COLORS );
}

void ViewPort::ClearImages( void )
{
	if ( view->bmi ) view->Clear();
}

void ViewPort::ClearImages( RECT region )
{
	if ( view->bmi ) view->Clear( region );
}

void ViewPort::RenderImages( RECT region, double pixel_size, double offset_x, double offset_y, bool use_proxy )
{
	Transform *transform;									// render image onto view using pixel_size
	Point *p, min, max;
	Contour *domain;
	RECT r, labeled;
	double x, y, area;
	int index, i;
	char txt[128];
	Image_Ptr images[MAX_DOMAINS];						// number of domains is limited by 8bit mask size
	Nform_Ptr nforms[MAX_DOMAINS];
	POINT mins[MAX_DOMAINS], maxs[MAX_DOMAINS];
// begin debug logging...
	DWORD byteswritten;
	char line[MAX_PATH];
	if ( debugLogFile != INVALID_HANDLE_VALUE )
		{
		sprintf(line,"Entered ViewPort::RenderImages\r\n");
		WriteFile(debugLogFile, line, strlen(line), &byteswritten, NULL);
		}
// ...end debug logging
startTime3 = GetTickCount();
															// check for valid render region
	if ( (region.right - region.left) < 0 ) return;
	if ( (region.bottom - region.top) < 0 ) return;
															// limit render region to view pixels
	if ( region.left < 0 ) r.left = 0; else r.left = region.left;
	if ( region.right >= width ) r.right = width-1; else r.right = region.right;
	if ( region.top < 0 ) r.top = 0; else r.top = region.top;
	if ( region.bottom >= height ) r.bottom = height-1; else r.bottom = region.bottom;
	
	ClearImages( r );								// clear the region to all zeros before rendering
	
	for (index=0; index < MAX_DOMAINS; index++)		// clear the image and nform pointer arrays
		{
		images[index] = NULL;
		nforms[index] = NULL;
		}

	if ( section->transforms )
		{											// FIRST PASS... Create index mask for images on view
		index = 0;													// count every domain in section with index
		transform = section->transforms->first;						// render in order, back to front
		while ( transform && (index < MAX_DOMAINS) )
			{			
			if ( transform->image ) 								// do only if image is present and not hidden
				{
				if ( transform->domain && !transform->domain->hidden )
					{												// use domain boundary to define render area...
					index++;										
					domain = new Contour( *(transform->domain) );	// create a copy of domain
					domain->Scale( transform->image->mag );			// go from pixels to units
					domain->InvNform( transform->nform );			// transform into section
					domain->Shift( -offset_x, -offset_y );			// shift from view offset			
					labeled = view->Mask( domain, pixel_size, index, r );	// label pixels for rendering
					if ( labeled.left <= labeled.right )			// only use if some interior pixels were set
						{									
						images[index] = transform->image;			// store ptr to image for later rendering step
						nforms[index] = transform->nform;
						mins[index].x = labeled.left; mins[index].y = labeled.top;
						maxs[index].x = labeled.right; maxs[index].y = labeled.bottom;
						}
					delete domain;									// delete copy of domain
					}
				}
			transform = transform->next;				// do next transform in list
			}

		while ( index > 0 )							// SECOND PASS... render each indexed subregion from image sources
			{										
			view->MaskXform( images[index], nforms[index], index, mins[index], maxs[index],
																	pixel_size, offset_x, offset_y, use_proxy );
			index--;
			}
		}
totalTime3 += GetTickCount() - startTime3;	// DEBUGGING
nTime3++;
}

void ViewPort::FillContours( RECT region, double pixel_size, double offset_x, double offset_y )
{
	Transform *transform;								// draw from all transforms in section
	Contour *contour, *c;

	if ( section && view )									// do only if section and view are defined
	  if ( section->transforms )							// render all contours in all transforms
		{
		transform = section->transforms->first;
		while ( transform )									// render all contours in this transform
			{	
			if ( transform->contours )						// do only if have contours
				{
				contour = transform->contours->first;
															// do each contour in list
				while ( contour )
					{
					if ( contour->points && !contour->hidden )		// don't draw hidden contours
						{
						c = new Contour( *contour );					// copy the contour
						c->InvNform( transform->nform );				// transform into section
						c->Shift( -offset_x, -offset_y );				// shift into view
						if ( c->closed )
							view->Fill( c, pixel_size, region, c->border );
						delete c;
						}
					contour = contour->next;					// do next contour in list
					}
				}

			transform = transform->next;					// do next transform in list
			}
		}		// end if ( section->transforms )

}


void ViewPort::DrawContours( double pixel_size, double offset_x, double offset_y )
{
	Transform *transform;								// render all transforms in section
	HPEN pen, oldpen;
	HBRUSH brush, oldbrush;
	LOGBRUSH lbrush;
	int oldmode, i, x, y, numpts;
	Contour *contour, *c;
	Point *p;
	POINT *lpPoints;

//startTime3 = GetTickCount();

	if ( section && viewDC )							// do only if section and drawing surface are defined
	  if ( section->transforms )							// render all contours in all transforms
		{
		transform = section->transforms->first;
		while ( transform )									// render all contours in this transform
			{	
			if ( transform->contours )						// do only if have contours
				{
				contour = transform->contours->first;
															// do each contour in list
				while ( contour )
					{
					if ( contour->points && !contour->hidden )		// don't draw hidden contours
						{
						c = new Contour( *contour );					// copy the contour
						c->InvNform( transform->nform );				// transform into section
						c->Shift( -offset_x, -offset_y );				// shift into view
						c->Scale( 1.0/pixel_size );						// scale to view's pixels

						numpts = c->points->Number();
						lpPoints = new POINT[ numpts ];					// create Window POINT array for drawing

						i = 0;
						p = c->points->first;
						while ( p != NULL )
							{
							lpPoints[i].x = (int)floor(p->x);
							lpPoints[i].y = height - (int)floor(p->y);
							i++;
							p = p->next;
							}
																		// create pen for border of object
						if ( transform->isActive ) pen = CreatePen( PS_DOT, 1, c->border.ref() );
						else pen = CreatePen( PS_SOLID, 1, c->border.ref() );
						oldpen = (HPEN)SelectObject( viewDC, pen );		// set pen into device context

						if ( c->closed && ((transform->isActive && (c->mode > 0))
												|| (!transform->isActive && (c->mode < 0))) )
							{
							brush = CreateSolidBrush( c->fill.ref() );	// interior will be filled
							oldbrush = (HBRUSH)SelectObject( viewDC, brush );
							SetROP2( viewDC, abs(c->mode) );			// using contour fill mode and color
							Polygon( viewDC, lpPoints, numpts );
							SelectObject( viewDC, oldbrush );			// clean up fill brush
							DeleteObject(brush);
							}

						SelectObject( viewDC, (HBRUSH)GetStockObject(NULL_BRUSH) );	// without coloring interior
						SetROP2( viewDC, R2_COPYPEN );								// draw contour border with pen
						if ( c->closed ) Polygon( viewDC, lpPoints, numpts );
						else Polyline( viewDC, lpPoints, numpts );
						
						SelectObject( viewDC, oldpen );					// clean up pen
						DeleteObject(pen);
						delete[] lpPoints;								// and dynamic memory
						delete c;
						}
					contour = contour->next;					// do next contour in list
					}
				}

			transform = transform->next;					// do next transform in list
			}
		}		// end if ( section->transforms )

//totalTime3 += GetTickCount() - startTime3;			// debugging
//nTime3++;

}

void ViewPort::DrawActiveDomain( double pixel_size, double offset_x, double offset_y )
{
	Transform *transform;								// render all transforms in section
	HPEN pen, oldpen;
	HBRUSH brush, oldbrush;
	LOGBRUSH lbrush;
	int oldmode, i, x, y, numpts;
	Contour *contour, *c;
	Point *p;
	POINT *lpPoints;

	if ( section && viewDC )							// need a drawing surface for domain contour
	  if ( section->active )							// do only if have an active transform
		{
		transform = section->active;
		if ( transform->domain )						// do only if have domain
			{
			contour = transform->domain;
			if ( contour->points )
				{
				c = new Contour( *contour );					// copy the contour
																// if image, then contour is in pixels
				if ( transform->image ) c->Scale( transform->image->mag );
				c->InvNform( transform->nform );				// transform into section
				c->Shift( -offset_x, -offset_y );				// shift into view
				c->Scale( 1.0/pixel_size );						// scale to view's pixels
					
				numpts = c->points->Number();
				lpPoints = new POINT[ numpts ];					// create Window POINT array for drawing

				i = 0;
				p = c->points->first;
				while ( p != NULL )
					{
					lpPoints[i].x = (int)floor(p->x);
					lpPoints[i].y = height - (int)floor(p->y);
					i++;
					p = p->next;
					}
																// create pen for border of object
				pen = CreatePen( PS_SOLID, 1, c->border.ref() );
				oldpen = (HPEN)SelectObject( viewDC, pen );		// set pen into device context

				if ( c->mode < 0 )
					{
					brush = CreateSolidBrush( c->fill.ref() );	// interior will be filled
					oldbrush = (HBRUSH)SelectObject( viewDC, brush );
					SetROP2( viewDC, abs(c->mode) );			// using contour fill mode and color
					Polygon( viewDC, lpPoints, numpts );
					SelectObject( viewDC, oldbrush );			// clean up fill brush
					DeleteObject(brush);
					}

				SelectObject( viewDC, (HBRUSH)GetStockObject(NULL_BRUSH) );	// without coloring interior
				SetROP2( viewDC, R2_COPYPEN );								// draw contour border with pen
				if ( c->closed ) Polygon( viewDC, lpPoints, numpts );
				else Polyline( viewDC, lpPoints, numpts );
				
				SelectObject( viewDC, oldpen );					// clean up pen
				DeleteObject(pen);
				delete[] lpPoints;								// and dynamic memory
				delete c;
				}
			}
		}

}

void ViewPort::DrawEditContour( double pixel_size, double offset_x, double offset_y )
{
	HPEN pen, oldpen;
	HBRUSH brush, oldbrush;
	LOGBRUSH lbrush;
	int oldmode, i, x, y, numpts;
	Contour *c;
	Point *p;
	POINT *lpPoints;

	if ( EditContour && viewDC )
		if ( EditContour->points )
			{
			c = new Contour( *EditContour );				// copy the contour
			
			c->Shift( -offset_x, -offset_y );				// shift into view
			c->Scale( 1.0/pixel_size );						// scale to view's pixels
				
			numpts = c->points->Number();
			lpPoints = new POINT[ numpts ];					// create Window POINT array for drawing

			i = 0;
			p = c->points->first;
			while ( p != NULL )
				{
				lpPoints[i].x = (int)floor(p->x);
				lpPoints[i].y = height - (int)floor(p->y);
				i++;
				p = p->next;
				}
															// create pen for border of object
			pen = CreatePen( PS_DOT, 1, c->border.ref() );
			//pen = CreatePen( PS_SOLID, 1, c->border.ref() );
			oldpen = (HPEN)SelectObject( viewDC, pen );		// set pen into device context

			if ( (c->mode > 0) && c->closed )
				{
				brush = CreateSolidBrush( c->fill.ref() );	// interior will be filled
				oldbrush = (HBRUSH)SelectObject( viewDC, brush );
				SetROP2( viewDC, abs(c->mode) );			// using contour fill mode and color
				Polygon( viewDC, lpPoints, numpts );
				SelectObject( viewDC, oldbrush );			// clean up fill brush
				DeleteObject(brush);
				}

			SelectObject( viewDC, (HBRUSH)GetStockObject(NULL_BRUSH) );	// without coloring interior
			SetROP2( viewDC, R2_COPYPEN );								// draw contour border with pen
			if ( c->closed ) Polygon( viewDC, lpPoints, numpts );
			else Polyline( viewDC, lpPoints, numpts );
			
			SelectObject( viewDC, oldpen );					// clean up pen
			DeleteObject(pen);
			delete[] lpPoints;								// and dynamic memory
			delete c;
			}

}

Transform * ViewPort::DomainFromPixel( int x, int y )	// use index byte to locate domain in section
{
	Transform *transform;
	int index, mask_index;

	mask_index = view->GetIndex( x, y );	// read index byte from 32-bit pixel
	index = 0;

	if ( section && (mask_index > 0) && (mask_index < MAX_DOMAINS) )
	  if ( section->transforms )
		{
		transform = section->transforms->first;
		while ( transform )							// check each transform
			{
			if ( transform->image && !transform->domain->hidden ) // if visible image, count this domain
				{
				index++;
				if ( index == mask_index )			// if found right one...
					return( transform );			// ...return transform
				}
			transform = transform->next;			// otherwise try next transform
			}
		}

	return( NULL );
}
													// regenerate the view as needed for the given screen region

bool ViewPort::Regenerate( RECT region, double pixel_size, double offset_x, double offset_y, bool use_proxy )
{
	HCURSOR cur;
	bool wasregenerated = false;

	if ( view->bmi && viewDC )
		{
		if ( needsRendering )										// render the domains
			{
			cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );			// show hourglass, this could be slow
			RenderImages( region, pixel_size, offset_x, offset_y, use_proxy );	// render images
			SetCursor( cur );										// restore cursor
			}
		if ( needsRendering || needsDrawing )						// if images were rendered, redraw traces
			{
			ImageToViewDC();										// blt to DC
			DrawContours( pixel_size, offset_x, offset_y );			// draw contours
			DrawActiveDomain( pixel_size, offset_x, offset_y );		// or domain boundary if this is domain view
			DrawEditContour( pixel_size, offset_x, offset_y );		// finally render the contour being created
			}
		wasregenerated = needsRendering || needsDrawing;			// report whether changes were made
		needsRendering = false;
		needsDrawing = false;										// view bitmap is up to date -- can display it
		}

	return( wasregenerated );
}

													// shift rendered view and add rendering to border

void ViewPort::ShiftView( int x, int y, double pixel_size, double offset_x, double offset_y, bool use_proxy )
{
	RECT rx, ry;
	HCURSOR cur;

	if ( view->bmi && viewDC )
		{
		cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );				// show hourglass, this could be slow
		view->Shift( x, -y );										// view y is opposite from screen
		if ( x != 0 )
			{
			if ( x < 0 ) { rx.left = width+x; rx.right = width-1; }
				  else	{ rx.left = 0; rx.right = x; }
			rx.top = 0; rx.bottom = height-1;
			RenderImages( rx, pixel_size, offset_x, offset_y, use_proxy );	// render empty side rectangle
			}
		if ( y != 0 )
			{
			if ( -y < 0 ) { ry.top = height-y; ry.bottom = height-1; }
				  else { ry.top = 0; ry.bottom = -y; }
			ry.left = 0; ry.right = width-1;
			RenderImages( ry, pixel_size, offset_x, offset_y, use_proxy );	// empty top/bottom rectangle
			}
		ImageToViewDC();											// blt to DC
		DrawContours( pixel_size, offset_x, offset_y );			// draw contours
		DrawActiveDomain( pixel_size, offset_x, offset_y );		// or domain boundary if this is domain view
		DrawEditContour( pixel_size, offset_x, offset_y );		// finally render the contour being created
		SetCursor( cur );											// restore cursor
		needsRendering = false;			// still need an Invalidation of window to display viewDC
		}
}
											

ADib * ViewPort::CopyView( bool maskContours )	// copy view bitmap and, if desired, mask pixels of section contours
{												// use this now for WildfireRegions as well
	ADib *region;										// Ju: see style notes in comments...
	Transform *transform;
	Contour *contour, *c;
//	Section* currSection = this->section;				// don't need this extra variable
	double offset_x	= CurrSeries->offset_x;				// OK to put all uses of global CurrSeries
	double offset_y = CurrSeries->offset_y;				// at top where obvious, but if change 
	double pixel_size = CurrSeries->pixel_size;			// type in class definition will also need to
	int mask_width = CurrSeries->ContourMaskWidth;		// find and change types here

	region = new ADib( *view, 1.0 );					// copy view  bitmap
	if ( maskContours )									// having an early return is OK but in this case logic is more direct
		{
		if ( section )									// mask out existing contours on current section
		  if ( section->transforms )
			{  //	if (currSection && currSection->transforms)	{ if section is NULL then 2nd have of condition could crash is evaluated!
			transform = section->transforms->first;
			while ( transform  )
				{										// put { here, indented to next line
				if ( transform->contours )
					{ 									// do only if contours exist
					contour = transform->contours->first;
					while ( contour )
						{
						if ( contour->points && !contour->hidden )		// don't mask hidden contours
							{
							c = new Contour( *contour );						// create a copy
							c->InvNform( transform->nform );					// transform into section
							c->Shift( -offset_x, -offset_y );					// shift from view offset
							region->MaskOutBorder( c, pixel_size, mask_width );	// clear index of border pixels
							delete c;											// delete modified copy
							}
						contour = contour->next;						// do next contour in transform
						}
					}
				transform = transform->next;			// do next transform in list
				}
			}
		}
	return region;										// return ptr to copy bitmap
}
