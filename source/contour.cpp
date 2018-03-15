////////////////////////////////////////////////////////////////////////
//	This file contains the methods for the Contour and Contours classes
//
//    Copyright (C) 1996-2007  John Fiala (fiala@bu.edu)
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
// -+- change: Added Interior() to allow shrink back of trace within Reduce().
// -+- change: Added XOR() to return pieces after intersection of 2 clockwise contours.
// modified 3/10/05 by JCF (fiala@bu.edu)
// -+- change: Modified XOR to eliminate bad behavior of non-crossing intersections.
// modified 5/2/02 by JCF (fiala@bu.edu)
// -+- change: Added Smooth() that smooths x,y by moving average.
// modified 6/19/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Modified Contour::Contour and Contour::Contour( Contour &copyfrom) to accomodate Contour::AverageIntensity
// modified 6/23/06 by JCF (fiala@bu.edu)
// -+- change: Fixed bugs in XOR(), Interior(), and ChainCode().
// modified 6/24/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Removed Contour::AverageIntensity from Contour::Contour and Contour::Contour( Contour &copyfrom)
// modified 11/15/06 by JCF (fiala@bu.edu)
// -+- change: Added histogram pointer to contour attributes for storing image pixel info.
// modified 4/5/07 by JCF (fiala@bu.edu)
// -+- change: Fixed bug in WallsInRegion that sometimes left black lines next to domain images.

#include "reconstruct.h"

Contour::Contour()
{
	points = NULL;
	hidden = false;
	closed = true;
	simplified = false;
	strcpy(name,"unknown");
	comment[0] = 0;
	border = Color( 1.0, 0.0, 1.0 );
	fill = Color( 1.0, 0.0, 1.0 );
	mode = R2_MASKPEN;
	Id = GlobalId++;
	histogram = NULL;
}
	
Contour::~Contour()
{
	if ( points ) delete points;
	if ( histogram ) delete histogram;
}

Contour::Contour ( Contour &copyfrom )								// copy constructor
{
	Point *p, *q;
	
	strcpy(name,copyfrom.name);				// copy attributes
	strcpy(comment,copyfrom.comment);
	hidden = copyfrom.hidden;
	closed = copyfrom.closed;
	simplified = copyfrom.simplified;
	border = copyfrom.border;
	fill = copyfrom.fill;
	mode = copyfrom.mode;
	Id = GlobalId++;						// Id will be unique!

    if ( copyfrom.histogram ) histogram = new Histogram( *(copyfrom.histogram) );
	else histogram = NULL;

	if ( copyfrom.points )
		{									// copy points if present
		points = new Points();
		p = copyfrom.points->first;
		while ( p != NULL ) {
			q = new Point(*p);				// use compiler-defined copy constructor for Point
			points->Add(q);
			p = p->next;
			}
		}
	else points = NULL;
}

void Contour::Scale( double scale )			// multiply contour points by scale
{
	Point *p;
	
	if ( points )
		{									// do all points if present
		p = points->first;
		while ( p != NULL ) {
			p->x *= scale;
			p->y *= scale;
			p->z *= scale;
			p = p->next;
			}
		}
}

void Contour::Pixels( double scale )		// scale points and round to integers
{
	Point *p;
	
	if ( points )
		{									// do all points if present
		p = points->first;
		while ( p )
			{
			p->x = floor((p->x/scale)+0.5);
			p->y = floor((p->y/scale)+0.5);
			p = p->next;
			}
		}
}

void Contour::ScaleXY( double scalex, double scaley )	// multiply contour points by scales
{
	Point *p;
	
	if ( points )
		{									// do all points if present
		p = points->first;
		while ( p != NULL ) {
			p->x *= scalex;
			p->y *= scaley;
			p = p->next;
			}
		}
}

void Contour::Shift( double x, double y )		// shift contour points by offset
{
	Point *p;
	
	if ( points )
		{									// do all points if present
		p = points->first;
		while ( p != NULL ) {
			p->x += x;
			p->y += y;
			p = p->next;
			}
		}
}

void Contour::ShiftXYZ( double x, double y, double z ) // shift contour points by 3D offset
{
	Point *p;
	
	if ( points )
		{		
		p = points->first;
		while ( p != NULL )
			{
			p->x += x;
			p->y += y;
			p->z += z;
			p = p->next;
			}
		}
}

void Contour::YInvert( double y )			// invert y value around offset
{
	Point *p;
	
	if ( points )
		{									// do all points if present
		p = points->first;
		while ( p != NULL ) {
			p->y = y - p->y;
			p = p->next;
			}
		}
}

void Contour::FwdNform( Nform *N )			// pass contour points through forward transformation
{
	Point *p;
	double x,y;
	
	if ( points )
		{									// do all points if present
		p = points->first;
		while ( p != NULL ) {
			x = N->X( p->x, p->y );
			y = N->Y( p->x, p->y );
			p->x = x;
			p->y = y;
			p = p->next;
			}
		}
}

void Contour::InvNform( Nform *N )			// pass contour points through inverse transformation
{
	Point *p;
	
	if ( points ) {							// do all points if present
		p = points->first;
		while ( p != NULL ) {
			N->XYinverse( &(p->x), &(p->y) );
			p = p->next;
			}
		}
}

void Contour::Extent( Point *min, Point *max )	// determine min(x,y,z) and max(x,y,z) for all points
{
	Point *p;
	
	if ( points )										// do all points if present
		{
		min->x = MAX_FLOAT;  min->y = MAX_FLOAT;  min->z = MAX_FLOAT;
		max->x = -MAX_FLOAT; max->y = -MAX_FLOAT; max->z = -MAX_FLOAT;
		p = points->first;
		while ( p != NULL ) {
			if (p->x < min->x) min->x = p->x;			// if outside of bounds, set new bound
			if (p->x > max->x) max->x = p->x;
			if (p->y < min->y) min->y = p->y;
			if (p->y > max->y) max->y = p->y;
			if (p->z < min->z) min->z = p->z;
			if (p->z > max->z) max->z = p->z;
			p = p->next;
			}
		}
	else {
		min->x = 0.0; min->y = 0.0; min->z = 0.0;		// in case no points, return something reasonable?
		max->x = 0.0; max->y = 0.0; max->z = 0.0;
		}
}

double Contour::Length( void )		// compute and return contour length
{
	Point  *p, *q;
	double dx, dy;
	double length;
	
	length = 0.0;
	if ( points )
		{
		p = points->first;						// sum length contribution from all segments
		if ( p ) q = p->next; else q = NULL;
		while ( (p != NULL) && (q != NULL) )
			{
			dx = q->x - p->x;
			dy = q->y - p->y;
			length += sqrt(dx*dx+dy*dy);
			p = q;
			q = q->next;
			}
		if ( closed )
			{
			p = points->last;						// include tail to head segment
			q = points->first;
			dx = q->x - p->x;
			dy = q->y - p->y;
			length += sqrt(dx*dx+dy*dy);
			}
		}
	return( length );
}



bool between( double x, double x1, double x2 )			// return true if x is between x1 and x2
{
	if ( (x < x1) && (x > x2) ) return true;
	if ( (x < x2) && (x > x1) ) return true;
	return false;
}


double YatX( double x, Point *p, Point *q )
{
	double x0, x1, y0, y1, y;						// compute and insert point (x,y) of intersection

	x0 = p->x; y0 = p->y;							// local copy of values
	x1 = q->x; y1 = q->y;

	if ( ( x1 == x0 ) || (y1 == y0) ) y = y0;		// do nothing if vertical or horz. line
	else y = (y1-y0)*(x-x0)/(x1-x0) + y0;			// non-axis line: y-y0 = m*(x-x0)

	return y;
}

double XatY( double y, Point *p, Point *q )
{
	double x0, x1, y0, y1, x;						// compute and insert point (x,y) of intersection

	x0 = p->x; y0 = p->y;							// local copy of values
	x1 = q->x; y1 = q->y;

	if ( (y1 == y0) || (x1 == x0) )	x = x0;			// do nothing if horz. or vertical line
	else x = (y-y0)*(x1-x0)/(y1-y0) + x0;			// non-axis line: x-x0 = (y-y0)/m
	
	return x;
}

Points * Contour::WallsInRegion( double minx, double maxx, double miny, double maxy )
{
	Points *walls;					// return array of inside/outside labels for all pixels
	int *count;						// ASSUMES ALL BOUNDARY INTERSECTIONS ARE CONTOUR POINTS
	Point *n, *p, *q;
	double x, y, exminx, exmaxx, exminy, exmaxy;
	int i, height, start, end, skip, last;

	walls = new Points();					// create list for points of y-transitions
	if ( !points ) return walls;
	if ( !points->first ) return walls;

	height = (int)(maxy - miny) + 1;		// create array to count y-transitions to left of minx
	count = new int[height];
	for (i=0; i<height; i++) count[i] = 0;
	
											// use 1/2 pixel buffer for boundary test on doubles
	exminx = minx-0.5; exmaxx = maxx+0.5;
	exminy = miny-0.5; exmaxy = maxy+0.5;
											// Step 1: find y-direction at last segment which will
	q = points->last;						//         be the transition prior to the first segment
	p = points->first;
	last = 0;								// last is direction of last y-transition
	if ( q->y > p->y ) last = -1;			// last == -1 is a downward transition in y value
	if ( q->y < p->y ) last = 1;			// last == 1 is an upward transition of y-value
	while ( !last && q )					// back up through contour to last real y-transition
		{
		if ( q->y > p->y ) last = -1;
		if ( q->y < p->y ) last = 1;
		p = q;
		q = q->prev;
		}
											// Step 2: find (x,y) points where there is a y-transition
	p = points->first;						//         these are the walls to limit fill operation.
	while ( p )								//         To left of Region, (x,y) value not needed but still
		{									//         need to know whether on inside or outside of contour.
		q = p->next;
		if ( q == NULL ) q = points->first;
		skip = 0;							// assume last transition does not require skipping 1st chain code

											// Case I. walls to the left of Region...

		if ( (p->x < exminx) || (q->x < exminx) )
			{
			if ( q->y > p->y )					// up y-transition of at least one pixel
				{
				if ( last > 0 ) skip = 1;		// don't count p twice if not change of direction
				start = (int)(p->y-miny)+skip;
				end = (int)(q->y-miny);
				}
			else if ( q->y < p->y )				// down y-transition at at least one pixel	
				{
				start = (int)(q->y-miny);
				if ( last < 0 ) skip = 1;		// don't count p twice if not change of direction
				end = (int)(p->y-miny)-skip;
				}
			else { start = 0; end = -1; }		// ignore horizontal lines and repeated points
		
			for (i=start; i<=end; i++)			// count walls for only those transitions within Region
				if ( (i>=0) && (i<height) ) count[i]++;
			}
											// Case II. walls inside Region...

		else if ( (p->x>exminx) && (p->x<exmaxx) && (p->y>exminy) && (p->y<exmaxy)
					&& (q->x>exminx) && (q->x<exmaxx) && (q->y>exminy) && (q->y<exmaxy) )
			{
			if ( q->y > p->y )					// up transition of at least one pixel
				{
				if ( last > 0 ) skip = 1;		// don't count p twice if not change of direction
				start = (int)p->y+skip;
				end = (int)q->y;
				}
			else if ( q->y < p->y )				// down transition of at least one pixel
				{	
				if ( last < 0 ) skip = 1;		// don't count p twice if not change of direction
				start = (int)q->y;
				end = (int)p->y-skip; 
				}
			else { start = 0; end = -1; }		// ignore horizontal lines and repeated points

			for (i=start; i<=end; i++)			// add y-transition points to walls list
				{
				y = (double)i;
				x = floor(XatY(y,p,q)+0.5);		// determine x-value where a transition occurs
				n = new Point( x, y, 0.0 );
				walls->Add( n );				// add wall to list
				}
			}
											// if segment is above, below, or right of Region: Do Nothing!

		if ( q->y > p->y ) last = 1;		// set last based on this segment's trend but
		else if ( q->y < p->y ) last = -1;	// if p->y == q->y, no change from previous segment

		p = p->next;						// loop terminates when p (not q) reaches end
		}

	x = minx;								// finally, convert counts to walls on left edge
	for (i=0; i<height; i++)				// an odd number of y-transitions => inside at minx
		if ( count[i]%2 )
			{
			y = (double)i+miny;
			n = new Point( x, y, 0.0 );
			walls->Add( n );
			}

	delete [] count;						// clean up memory
	return walls;
}

void Contour::AddPixelsAtBoundaries( double minx, double maxx, double miny, double maxy )
{
	Point *n, *p, *q;						// Add all intersections with these limits...
	double x, y;							// ASSUMES this IS INTEGER VALUES
												
	if ( !points ) return;					// require at least 2 points
	p = points->first;
	if ( !p ) return;
	q = p->next;
	if ( !q ) return;
												// I. min x intersections
	while ( p && q )								// p---q line segment
		{
		if ( between(minx,p->x,q->x) )				// if there is intersection
			{
			y = floor(YatX(minx,p,q)+0.5);				// insert intersection point after p
			n = new Point( minx, y, p->z );
			points->Insert( n, p );
			}
		p = q;											// set p to q
		q = q->next;									// q is next point
		}
	if ( closed )										// handle last segment for closed contour
		{
		q = points->first;
		p = points->last;
		if ( between(minx,p->x,q->x) )				// if there is intersection
			{
			y = floor(YatX(minx,p,q)+0.5);				// insert intersection point after p
			n = new Point( minx, y, p->z );
			points->Insert( n, p );
			}
		}

	p = points->first;							// II. max x intersections
	q = p->next;
	while ( p && q )								// p---q line segment
		{
		if ( between(maxx,p->x,q->x) )				// if there is intersection
			{
			y = floor(YatX(maxx,p,q)+0.5);				// insert intersection point after p
			n = new Point( maxx, y, p->z );
			points->Insert( n, p );
			}
		p = q;											// set p to q
		q = q->next;									// q is next point
		}
	if ( closed )										// handle last segment for closed contour
		{
		q = points->first;
		p = points->last;
		if ( between(maxx,p->x,q->x) )				// if there is intersection
			{
			y = floor(YatX(maxx,p,q)+0.5);				// insert intersection point after p
			n = new Point( maxx, y, p->z );
			points->Insert( n, p );
			}
		}

	p = points->first;							// III. min y intersections
	q = p->next;
	while ( p && q )								// p---q line segment
		{
		if ( between(miny,p->y,q->y) )				// if there is intersection
			{
			x = floor(XatY(miny,p,q)+0.5);				// insert intersection point after p
			n = new Point( x, miny, p->z );
			points->Insert( n, p );
			}
		p = q;											// set p to q
		q = q->next;									// q is next point
		}
	if ( closed )										// handle last segment for closed contour
		{
		q = points->first;
		p = points->last;
		if ( between(miny,p->y,q->y) )				// if there is intersection
			{
			x = floor(XatY(miny,p,q)+0.5);				// insert intersection point after p
			n = new Point( x, miny, p->z );
			points->Insert( n, p );
			}
		}

	p = points->first;							// IV. max y intersections
	q = p->next;
	while ( p && q )								// p---q line segment
		{
		if ( between(maxy,p->y,q->y) )				// if there is intersection
			{
			x = floor(XatY(maxy,p,q)+0.5);				// insert intersection point after p
			n = new Point( x, maxy, p->z );
			points->Insert( n, p );
			}
		p = q;											// set p to q
		q = q->next;									// q is next point
		}
	if ( closed )										// handle last segment for closed contour
		{
		q = points->first;
		p = points->last;
		if ( between(maxy,p->y,q->y) )				// if there is intersection
			{
			x = floor(XatY(maxy,p,q)+0.5);				// insert intersection point after p
			n = new Point( x, maxy, p->z );
			points->Insert( n, p );
			}
		}
}

void Contour::ChainCode( double pixel_size )		// convert arbitrary contour to chain code on integers
{													// NOTE: THIS DOES NOT MAKE INTEGERS POSITIVE!
	double rows, cols, abs_rows, abs_cols, sgn_rows, sgn_cols;
	double i, j;
	Point  *n, *o, *p, *q;

	if ( !points ) return;							// only valid if have 2 or more points
	if ( points->Number() < 2 ) return;

	p = points->first;
	p->x = floor( p->x/pixel_size + 0.5);			// convert to integer pixels
	p->y = floor( p->y/pixel_size + 0.5);
	q = p->next;
	q->x = floor( q->x/pixel_size + 0.5);			// convert to integer pixels
	q->y = floor( q->y/pixel_size + 0.5);

	while ( (p != NULL) && (q != NULL) ) {			// convert each p---q line segment to 8-connected segments

		cols = q->x - p->x;							// look only at pixels in rectangle pq
		rows = q->y - p->y;
		abs_cols = fabs( cols );
		abs_rows = fabs( rows );
		sgn_cols = 1.0; if ( cols < 0.0 ) sgn_cols = -1.0;
		sgn_rows = 1.0; if ( rows < 0.0 ) sgn_rows = -1.0;

		if ( (abs_rows < 0.5) && (abs_cols < 0.5) )	// repeating point in chain code, so delete q
			{
			points->Extract(q);
			delete q;
			}
		else
		  if ( abs_rows > abs_cols )				// incrementing row index and calculate col values
			{
			o = p;									// will insert progressively after o
			j = sgn_rows;
			while ( fabs(j) < abs_rows )
				{
				i = floor(j*cols/rows + 0.5);		// 0.5 to round to nearest pixel
				n = new Point( p->x+i, p->y+j, 0.0 );
				points->Insert( n, o );				// insert new point into segment o--q
				o = n;								// adjust insertion point
				j += sgn_rows;
				}
			p = o->next;
			}
		  else {									// incrementing col index, calculating row values
			o = p;									// will insert progressively after o
			i = sgn_cols;
			while ( fabs(i) < abs_cols )
				{
				j = floor(i*rows/cols + 0.5);
				n = new Point( p->x+i, p->y+j, 0.0 );
				points->Insert( n, o );
				o = n;
				i += sgn_cols;
				}
			p = o->next;
			}

													// move to q from the point just inserted
		if ( p )									// if this is NULL then finished closing contour
			{										// otherwise do next segment...
			q = p->next;							// get next point to do
			if ( q )
				{									// it exists so pixelize to it
				q->x = floor( q->x/pixel_size + 0.5);// but first convert q to integer pixels
				q->y = floor( q->y/pixel_size + 0.5);
				}
			else									// q == NULL => do closing segment from last to first
				{
				if ( closed )						// if open contour, then done already
					if ( p != points->first )		// pixel_size may be so large that all pts are same
						q = points->first;			//   in which case also done when down to 1 pt
				}
			}
		}

}

											// remove points until error would be exceeded
void Contour::Reduce( double max_error )	// NOTE: REDUCE CAN CREATE LOOPS (but OK for max_error=1)?
{
	Point *p, *o, *q, midpt;
	double error;
	bool point_deleted;
	
	point_deleted = true;					// start with this true to enter loop
	while ( point_deleted )					// continue going through contour removing every other
		{									// point until no points are deleted for a 1 pixel error
		point_deleted = false;
		p = points->first;
		while ( p ) 						// do each point in contour
			{
			o = p->next;					// o is candidate to remove between p...
			if ( o )
			  {
			  q = o->next;					//  ...and q
			  if ( q )
				{
				midpt.x = (p->x + q->x)/2.0;// test distance to contour if remove o
				midpt.y = (p->y + q->y)/2.0;
				error = (midpt.x - o->x)*(midpt.x - o->x) + (midpt.y - o->y)*(midpt.y - o->y);

				if ( error < max_error )	// here's where the error tolerance is set
					{
					points->Extract(o);		// remove it
					delete o;
					o = q;					// will continue process from q
					point_deleted = true;
					}
				}
			  }
			p = o;							// if o deleted, this is really q
			}
		}
}

void next_8_neighbor( double *x, double *y )	// return increments to next clockwise neighbor
{												// (-1,1)  (0,1)  (1,1)
	if ( *x < -0.5 )							// (-1,0)  (0,0)  (1,0)
		{										// (-1,-1) (0,-1) (1,-1)
		if ( *y < -0.5 ) { *x=0.0; *y=-1.0; }
		else if ( *y > 0.5 ) { *x=-1.0; *y=0.0; }
		else { *x=-1.0; *y=-1.0; }
		}
	else if ( *x > 0.5 )
		{
		if ( *y < -0.5 ) { *x=1.0; *y=0.0; }
		else if ( *y > 0.5 ) { *x=0.0; *y=1.0; }
		else { *x=1.0; *y=1.0; }
		}
	else // *x == 0.0
		{
		if ( *y < -0.5 ) { *x=1.0; *y=-1.0; }	
		else if ( *y > 0.5 ) { *x=-1.0; *y=1.0; }
		else { *x=-1.0; *y=-1.0; }
		}
}

bool same( double x, double y )				// the meaning of "same" for integer pixels
{											// NOTE: algorithm must be done in double to allow full range
	if ( fabs( x - y ) < 0.5 ) return true; // of x,y values, not just as pixels on screen
	return false;							// Also, same algorithm in integers is not faster. I tested it.
}											// Nor is it faster to use bitmap-based connected components.

void Contour::Exterior( void )				// smooth by tracing around exterior of ChainCode contour
{
	Points *exterior;
	Point *p, *o, min, max;
	int first_index, index, num_pts, last_index;
	double first_x, first_y, last_x, last_y, *xx, *xy, x, y, i, j, ii, jj;
	double x_plus, x_minus, y_plus, y_minus;
	bool found, done, every_other;

	num_pts = points->Number();
	if ( num_pts < 1 ) return;
												// put points into ordered arrays...
	xx = new double[num_pts+1];					// arrays will be number from 1..num_pts in sort routines
	xy = new double[num_pts+1];
	xx[0] = 0.0;								// make sure 0th item are valid doubles
	xy[0] = 0.0;
	index = 1;
	p = points->first;
	while ( p )									// fill arrays from contour
		{
		xx[index] = p->x;
		xy[index] = p->y;
		index++;
		p = p->next;
		}
	Sort( 1, num_pts, xx, xy );					// sort entire set on the x values
	
	index = 1;
	while ( index <= num_pts )					// for each unique x-value, sort the y subarray
		{
		x_plus = xx[index] + 0.005;
		first_index = index;
		index++;
		while ( index <= num_pts )
			if ( xx[index] < x_plus ) index++;		// find range of all same x value
			else {									// and sort this subarray on y
				last_index = index - 1;
				if ( last_index > first_index ) Sort( first_index, last_index, xy, xx );
				break;
				}
		}											// complete last elements in list
	last_index = index - 1;
	if ( last_index > first_index ) Sort( first_index, last_index, xy, xx );

											// now, with contour data sorted on x and y, trace outside...
	x = xx[1];
	y = xy[1];

	exterior = new Points();					// generate a new contour that surrounds this one
//	every_other = false;						// use this flag to only add every other point to exterior
												//  this will reduce by half the number of pts with < .7 error
	ii = 0;
	jj = -1;
	first_x = x + jj;							// take point to left of leftmost pt as first new pt
	first_y = y + ii;
	x = first_x;
	y = first_y;								// start searching left of leftmost pt where there are no old pts
	ii = 1;
	jj = 0;
	last_x = first_x;
	last_y = first_y;
	o = new Point( first_x, first_y, 0.0 );
	exterior->Add(o);							// add the first pt to new contour
	next_8_neighbor( &jj, &ii );			// now search the next neighboring pixel
	done = false;
	while ( !done )								// continuous this over entire contour...
		{
		found = false;
		while ( !found )						// check neighbors in clockwise order until find old contour pt
			{
			i = ii;								// j,i is candidate next new contour pt
			j = jj;
			next_8_neighbor( &jj, &ii );	// iff jj,ii is on the old contour
			x_minus = x + jj - 0.005;
			x_plus = x_minus + 0.01;
			y_minus = y + ii - 0.005;
			y_plus = y_minus + 0.01;

			index = (num_pts+1)/2;		// do binary search using increment last_index
			last_index = index+1;
			while ( last_index )			// still have array elements to check
				{
				if ( last_index > 1 ) last_index = (last_index+1)/2;
				else last_index = 0;
				if ( xx[index] < x_minus )
					{ index += last_index; if ( index > num_pts ) index = num_pts; }
				else if ( xx[index] > x_plus )
					{ index -= last_index; if ( index < 1 ) index = 1; }
				else
					{							// x values are equal, check y
					if ( xy[index] < y_minus )
						{ index += last_index; if ( index > num_pts ) index = num_pts; }
					else if ( xy[index] > y_plus )
						{ index -= last_index; if ( index < 1 ) index = 1; }
					else					// found it!
						{ found = true; break; }
					}
				}
			// try next clkwise neighbor if last one was not found on contour
			}

		x = x + j;								// if haven't reached the beginning we'll add this new pt
		y = y + i;
		if ( same(x,first_x) && same(y,first_y) ) done = true;
		else {
//			if ( every_other ) {				// add every other pt becuz simplify will later remove
				o = new Point( x, y, 0.0 );
				exterior->Add( o );
//				every_other = false;
//				}
//			else every_other = true;
			jj = last_x - x;					// commence search for next new pt
			ii = last_y - y;
			last_x = x;
			last_y = y;
			next_8_neighbor( &jj, &ii );
			}
		}

	delete[] xx;
	delete[] xy;
	delete points;
	points = exterior;

}

void Contour::Interior( void )				// trace around interior of Exterior contour
{											// only valid immediately after an Exterior call
	Points *interior;						// but still can create contour with self-intersections
	Point *n, *o, *p, *q;
	double first_x, first_y, last_x, last_y, x, y, i, j, ii, jj;
	bool found;

	interior = new Points();					// generate a new contour that is inside this one
												
	o = points->first;							// get first three pts of trace => must HAVE 3 pts!
	if ( o )									
		{
		p = o->next;
		last_x = p->x;
		last_y = p->y;
		}
	if ( p ) q = p->next;
	while ( o && p && q )						// consider three consecutive pts (o,p,q) at a time
		{										// note this assumes trace is clockwise
		x = p->x;
		y = p->y;
		jj = q->x - x;
		ii = q->y - y;
		next_8_neighbor( &jj, &ii );			// start with first pt clockwise interior of q
		first_x = o->x - x;						// check neighbors in clockwise order until find first_x,y
		first_y = o->y - y;						// then use last pt as new interior pt
		found = false;
		while ( !found )						
			{
			i = ii;								// j,i is candidate next new contour pt...
			j = jj;
			next_8_neighbor( &jj, &ii );		// ...iff jj,ii is on the old contour...
			if ( same(first_x,jj) && same(first_y,ii) )
				{
				found = true;					// ...and haven't already added with pt!
				if ( same(last_x,j+x) && same(last_y,i+y) ) ;
				else {
					last_x = x + j;						// add new pt
					last_y = y + i;						// remember for next pt also
					n = new Point( last_x, last_y, 0.0 );
					interior->Add( n );
					}
				}
			}
		o = o->next;
		p = p->next;					// commence search for next new pt
		q = q->next;
		if ( !q ) q = points->first;	// close contour at end for last point
		}

	delete points;						// replace points with new interior
	points = interior;
}

											// chain code and remove points until error is exceeded
											// if hull = true then take clockwise outside of points

void Contour::Simplify( double pixelsize, bool hull )	
{
	Point *p, *o, *q, midpt, min, max;
	double error, psize, ysize, xrange, yrange, xscale, yscale, xcenter, ycenter;
	bool point_deleted;

	if ( pixelsize <= 0.0 )				// negative pixelsize means don't use
		{
		this->Extent( &min, &max );			// find the bounds of the contour
		psize = max.x - min.x;				// divide in to discrete pieces
		ysize = max.y - min.y;				// NOTE: both psize and ysize cannot be zero
		if ( ysize > psize ) psize = ysize;
		psize = psize/100.0;			// pixelize to at least 100 discrete parts
		}
	else psize = pixelsize;				// otherwise use spcified pixelsize
	this->ChainCode( psize );

	if ( hull )								// if desired, take exterior of points
		{
		this->Exterior();					// usually do this for closed contour but
		if ( AutoShrinkBack ) this->Interior();	// this will expand it so shrink back one pixel
		}
	this->Reduce( 1.0 );					// reduce points until within a pixel
	this->Scale( psize );					// scale to original units
	this->simplified = true;
}


double Contour::Distance(double x, double y)				// from geometryalgorithms.com
{
	Point *p, *q;
	double x0, y0, x1, y1, vx, vy, wx, wy, b, c1, c2;
	double sq_distance, min_sq_distance;

	min_sq_distance = MAX_FLOAT;						// return MAX if no points
	q = NULL;
	if ( points )
	  p = points->first;	// p might be NULL if no points
	  if ( p ) q = p->next;
	  while ( p && q )		// quit when p passes end of closed contour
		{
		x0 = p->x; y0 = p->y;							// use local variables for efficiency
		x1 = q->x; y1 = q->y;
		vx = x1 - x0; vy = y1 - y0;
		wx = x - x0;  wy = y - y0;
		c1 = vx*wx + vy*wy;
		if ( c1 <= 0.0 ) sq_distance = wx*wx + wy*wy;		// NEED EPSILON HERE??
		else {
			c2 = vx*vx + vy*vy;
			if ( c2 <= c1 ) sq_distance = (x-x1)*(x-x1) + (y-y1)*(y-y1);
			else
				{
				b = c1/c2;
				wx = x0 + b*vx;
				wy = y0 + b*vy;
				sq_distance = (x-wx)*(x-wx) + (y-wy)*(y-wy);
				}
			}												// save if smallest distance so far

		if (sq_distance < min_sq_distance) min_sq_distance = sq_distance;

		p = p->next;										// do next segment pf contour
		q = q->next;
		if ( (q == NULL) && closed ) q = points->first;
		}

	return (min_sq_distance);								// return distance to closest segment
}

void Contour::Add( Contour *addfrom )			// add points from contour to this one
{
	Point *p, *q;
	
	if ( addfrom->points )								// copy only if points present
		{
		if ( points == NULL ) points = new Points();	// create this->points if doesn't exist
		p = addfrom->points->first;
		while ( p != NULL )								// add individual points
			{
			q = new Point(*p);							// use compiler-defined copy constructor for Point
			points->Add(q);
			p = p->next;
			}
		}
}

void Contour::GreensCentroidArea( double &x, double &y, double &area )
{									// compute the centroid and enclosed area of contour
	Point *p, *q;					// This doesn't work for a contour with loops!
	double px, py, qx, qy;
	double sum, sum_x, sum_y, ai;

	area = 0.0;						// start with nothing
	x = 0.0;
	y = 0.0;
	sum = 0.0;
	sum_x = 0.0;
	sum_y = 0.0;
	if ( points )					// algorithm adapted from 
		{							// Bashein & Detmer, Graphic Gems IV, pp. 3-5
		p = points->first;
		px = p->x;					// copy to local variables for efficiency
		py = p->y;
		q = p->next;
		while ( q )					// IS THERE A BETTER WAY TO COMPUTE THIS?
			{						// NOTE THAT DIFFERENCING PRODUCTS IS INACCURATE FOR LARGE VALUES
			qx = q->x;
			qy = q->y;
			ai = px*qy - qx*py;
			sum += ai;
			sum_x += (qx + px)*ai;
			sum_y += (qy + py)*ai;
			px = qx;
			py = qy;
			q = q->next;
			}
		if ( closed )
			{
			q = points->first;			// do closing segment
			qx = q->x;
			qy = q->y;
			ai = px*qy - qx*py;
			sum += ai;
			sum_x += (qx + px)*ai;
			sum_y += (qy + py)*ai;
			area = 0.5*sum;				// only compute area when clsoed
			}
		if ( sum != 0.0 )				// compute centroid
			{
			x = sum_x / (3.0*sum);
			y = sum_y / (3.0*sum);
			}	
		}
}


void Contour::Reverse( void )					// reverse order of contour points
{
	if ( points ) points->Reverse();
}

bool Contour::IsInRegion( double left, double top, double right, double bottom )
{												// true if any point of contour inside reactangle
	Point *p;
	double x, y;

	if ( points )
		{
		p = points->first;
		while ( p )					// for now look only at endpts, ignore segments themselves
			{						// Note: bottom < top becuz in section coord. not GDI coord.
			x = p->x; y = p->y;
			if ( (x<=right) && (x>=left) && (y<=top) && (y>=bottom) ) return( true );		
			p = p->next;
			}
		}

	return( false );
}


void Contour::CutAtNearest(double x, double y)				// again from geometryalgorithms.com
{
	Point *p, *q, *minp;
	double x0, y0, x1, y1, vx, vy, wx, wy, b, c1, c2;
	double sq_distance, min_sq_distance;

	min_sq_distance = MAX_FLOAT;
	minp = NULL;
	q = NULL;
	if ( points ) p = points->first;				// p might be NULL if no points
	if ( p ) q = p->next;
	while ( p && q )								// quit when p passes end of closed contour
		{
		x0 = p->x; y0 = p->y;						// use local variables for efficiency
		x1 = q->x; y1 = q->y;
		vx = x1 - x0; vy = y1 - y0;
		wx = x - x0;  wy = y - y0;
		c1 = vx*wx + vy*wy;
		if ( c1 <= 0.0 ) sq_distance = wx*wx + wy*wy;
		else {
			c2 = vx*vx + vy*vy;
			if ( c2 <= c1 ) sq_distance = (x-x1)*(x-x1) + (y-y1)*(y-y1);
			else
				{
				b = c1/c2;
				wx = x0 + b*vx;
				wy = y0 + b*vy;
				sq_distance = (x-wx)*(x-wx) + (y-wy)*(y-wy);
				}
			}										// save if smallest distance so far
		if (sq_distance < min_sq_distance)
			{
			min_sq_distance = sq_distance;
			minp = p;
			}
		p = p->next;								// do next segment of contour
		q = q->next;
		if ( (q == NULL) && closed ) q = points->first;
		}

	if ( minp )										// found a nearest point, open here
		{
		p = minp->prev;
		q = minp;
		if ( p )									// if p is NULL split was at tail, so nothing more required
			if ( closed )
				{
				p->next = NULL;						// break p--q segment
				q->prev = NULL;
				points->last->next = points->first;	// connect tail to head
				points->first->prev = points->last;	// and head to tail
				points->first = q;					// reset head and tail of list
				points->last = p;
				}
			else									// for an open contour, remove every point before q
				while ( p )
					{
					minp = p->prev;
					points->Extract(p);
					delete p;
					p = minp;
					}								// this will leave q at the head of the list
		}
}

												// trace outside of contours in list that overlap;
void Contours::Merge( double pixelsize )		// if all overlap, leave one merged contour as result		
{
	Contour *c, *cc, *merged;
	Points *exterior;
	Point *p, *o, min, max;
	int first_index, index, num_pts, last_index;
	double first_x, first_y, last_x, last_y, *xx, *xy, x, y, ii, jj;
	double x_plus, x_minus, y_plus, y_minus;
	bool found, done, allclosed;

	c = this->first;							// list must contain a contour
	if ( c )
		{
		merged = new Contour( *c );					// create candidate merged contour
		merged->ChainCode( pixelsize );				// discretize to pixels
		num_pts = merged->points->Number();
		allclosed = c->closed;						// remember if there were open contours
		c = c->next;
		while ( c )									// follow next link to rest of list of mergees
			{
			allclosed = (allclosed && c->closed);
			cc = new Contour( *c );					// copy each contour,
			cc->ChainCode( pixelsize );				// discretize, and add to this
			merged->Add( cc );
			num_pts += cc->points->Number();
			delete cc;
			c = c->next;
			}
												// put points into ordered arrays...

		xx = new double[num_pts+1];					// arrays will be number from 1..num_pts in sort routines
		xy = new double[num_pts+1];
		xx[0] = 0.0;								// make sure 0th item are valid doubles
		xy[0] = 0.0;
		index = 1;
		p = merged->points->first;
		while ( p )									// fill arrays from contour
			{
			xx[index] = p->x;
			xy[index] = p->y;
			index++;
			p = p->next;
			}
		Sort( 1, num_pts, xx, xy );					// sort entire set on the x values
		
		index = 1;
		while ( index <= num_pts )					// for each unique x-value, sort the y subarray
			{
			x_plus = xx[index] + 0.005;
			first_index = index;
			index++;
			while ( index <= num_pts )
				if ( xx[index] < x_plus ) index++;		// find range of all same x value
				else {									// and sort this subarray on y
					last_index = index - 1;
					if ( last_index > first_index ) Sort( first_index, last_index, xy, xx );
					break;
					}
			}											// complete last elements in list
		last_index = index - 1;
		if ( last_index > first_index ) Sort( first_index, last_index, xy, xx );

												// now, with contour data sorted on x and y, trace outside...

		exterior = new Points();					// generate new points for outside of current set

		x = xx[1];									// start at lowest (left-most) x value
		y = xy[1];
		first_x = x;
		first_y = y;
		last_x = first_x;
		last_y = first_y;
		ii = 0;										// start searching left of leftmost
		jj = -1;									// pt where there are no contour pts
		o = new Point( x, y, 0.0 );
		exterior->Add(o);							// add the first pt to new contour
		done = false;
		while ( !done )								// continue this over entire contour...
			{
			found = false;
			while ( !found )						// check neighbors in clockwise order until find old contour pt
				{
				next_8_neighbor( &jj, &ii );	// if jj,ii is on the old contour
				x_minus = x + jj - 0.005;
				x_plus = x_minus + 0.01;
				y_minus = y + ii - 0.005;
				y_plus = y_minus + 0.01;

				index = (num_pts+1)/2;		// do binary search using increment last_index
				last_index = index+1;
				while ( last_index )			// still have array elements to check
					{
					if ( last_index > 1 ) last_index = (last_index+1)/2;
					else last_index = 0;
					if ( xx[index] < x_minus )
						{ index += last_index; if ( index > num_pts ) index = num_pts; }
					else if ( xx[index] > x_plus )
						{ index -= last_index; if ( index < 1 ) index = 1; }
					else
						{							// x values are equal, check y
						if ( xy[index] < y_minus )
							{ index += last_index; if ( index > num_pts ) index = num_pts; }
						else if ( xy[index] > y_plus )
							{ index -= last_index; if ( index < 1 ) index = 1; }
						else					// found it!
							{ found = true; break; }
						}
					}
				// try next clkwise neighbor if last one was not found on contour
				}

			x = x + jj;								// if haven't reached the beginning we'll add this new pt
			y = y + ii;
			if ( same(x,first_x) && same(y,first_y) ) done = true;
			else {
				o = new Point( x, y, 0.0 );
				exterior->Add( o );
				jj = last_x - x;					// commence search for next new pt
				ii = last_y - y;
				last_x = x;
				last_y = y;
				next_8_neighbor( &jj, &ii );
				}
			}

		delete[] xx;								// clean up old data points
		delete[] xy;
		delete merged->points;

		merged->points = exterior;					// and replace with new ones

		if ( !allclosed )
			merged->Exterior();						// eliminate any thin parts of open lines

		merged->Reduce( 1.0 );						// reduce number of contour points
		merged->Scale( pixelsize );					// scale to original units
		merged->simplified = true;
													// Now get rid of list contours that were merged
		merged->Extent( &min, &max );				// find extent of merged result
		c = this->first;
		while ( c )									// don't delete contour not in merged area
			if ( c->IsInRegion(min.x,max.y,max.x,min.y) )
				{
				cc = c->next;						// delete all of list that was merged
				this->Extract( c );
				delete c;
				c = cc;
				}
			else c = c->next;

		this->Add( merged );				// finally, add new merged contour to list
		}	// end if (c)
}

double Contour::MinSqrdDistance( Contour *contour )		// return squared distance from c
{
	Point *p, *q, *cp, *cq;
	double d0x, d0y, d1x, d1y, dx, dy, a00, a01, a11, b0, b1, c, dd, mind, e, f, g, t0, t1;
	char txt[64];
 
	mind = MAX_FLOAT;
	cp = NULL;
	cq = NULL;
	if ( contour->points ) cp = contour->points->first;	// might be NULL if no points
	if ( cp ) cq = cp->next;
	while ( cp && cq && (mind > 0.0) )					// check every segment of c, but quit if intersection
		{
		p = NULL;
		q = NULL;
		if ( points ) p = points->first;				// compare every segment p--q of this contour
		if ( p ) q = p->next;							// to each segment cp--cq of input contour c
		while ( p && q && (mind > 0.0) )				// quit if find intersection (mind == 0.0)
			{
			d0x = cq->x - cp->x;						// will use parametric form p + ti*di, ti=[0,1]
			d0y = cq->y - cp->y;						// (for notation and motivation see:
			d1x = q->x - p->x;							//  Schneider PJ, Eberly DH (2003) Geometric Tools
			d1y = q->y - p->y;							//    for Computer Graphics, pp.224-229)
			dx = cp->x - p->x;							// to examine where lines intersect relative to t=[0,1]
			dy = cp->y - p->y;
			e = d0x*d1y - d1x*d0y;
			e = e*e;
			if ( e == 0.0 )								// parallel and degenerate (repeated pts) cases
				{
				a00 = d0x*d0x+d0y*d0y;
				a11 = d1x*d1x+d1y*d1y;
				c = dx*dx+dy*dy;
				if ( (a00 == 0.0) && (a11 == 0.0) )		// cp == cq and p == q
					dd = c;
				else if ( a00 == 0.0 )					// cp == cq and p != q
					{
					b1 = d1x*dx+d1y*dy;					// compute distance of point to segment
					if ( b1 <= 0.0 )					// if ( b1/a11 < 0 ) t = 0
						dd = c;
					else if ( b1 >= a11 )				// if ( b1/a11 > 1 ) t = 1
						dd = a11-2.0*b1+c;
					else dd = c - b1*b1/a11;			// 0 < t < 1
					}
				else if ( a11 == 0.0 )					// p == q and cp != cq
					{
					b0 = d0x*dx+d0y*dy;					// compute distance of point to segment
					if ( -b0 <= 0.0 )					// if ( -b0/a11 < 0 ) t = 0
						dd = c;
					else if ( -b0 >= a00 )				// if ( b1/a11 > 1 ) t = 1
						dd = a00+2.0*b0+c;
					else dd = c - b0*b0/a00;			// 0 < t < 1
					}
				else									// line segments are not degenerate
					{									// they lie on parallel lines
					b0 = d0x*dx+d0y*dy;
					a01 = d0x*d1x+d0y*d1y;
					if ( (a01<0.0) && (-b0 <= 0.0) )	// p and cp are closest
						dd = c;
					else if ( (a01>0.0) && (-b0>=a00) )	// p and cq are closest
						dd = a00 + 2.0*b0 +c;
					else if ( (a01>0.0) && (b0>=a01) )	// q and cp are closest
						{
						b1 = d1x*dx+d1y*dy;
						dd = a11 - 2.0*b1 + c;
						}
					else if ( (a01<0.0) && ((a01-b0)>=a00)) // q and cq are closest
						{
						b1 = d1x*dx+d1y*dy;
						dd = a00 -2.0*a01 + a11 +2.0*b0 -2.0*b1 +c;
						}
					else dd = c - b0*b0/a00;			// segments overlap, use distance between lines
					}
				}
			else										// non-parallel, non-degenerate cases
				{										// there are nine cases to consider
				b0 = d0x*dx+d0y*dy;
				a00 = d0x*d0x+d0y*d0y;					// squared distance =
				a01 = d0x*d1x+d0y*d1y;					//   a00*t0*t0-2*a01*t0*t1+a11*t1*t1+2*b0*t0-2*b1*t1+c
				b1 = d1x*dx+d1y*dy;						// find minima at (t0,t1) by solving
				a11 = d1x*d1x+d1y*d1y;					//   a00*t0-a01*t1+b0 = 0
				f = (a01*b1-a11*b0);					//   a11*t1-a01*t0-b1 = 0
				g = (a00*b1-a01*b0);					
				c = dx*dx+dy*dy;						// (formulas in Schneider & Eberly are wrong!)
				if ( f <= 0.0 )							
					{									
					if ( (g < 0.0) && (b1 < 0.0) )		// t1 = 0
						{								// t0 = -b0/a00;
						if ( -b0 < 0.0 ) dd = c;
						else if ( -b0 > a00 ) dd = a00+2.0*b0+c;
						else dd = c - b0*b0/a00;
						}
					else if ( (g > e) && (b1 > a11)	)	// t1 = 1
						{								// t0 = (a01-b0)/a00;
						if ( a01 < b0 ) dd = a11-2.0*b1+c;
						else if ( (a01-b0) > a00 ) dd = a00-2.0*a01+a11+2.0*b0-2.0*b1+c;
						else dd = a11-2.0*b1+c-(a01-b0)*(a01-b0)/a00;
						}
					else {								// t0 = 0.0;
						if ( b1 < 0.0 ) dd = c;			// t1 = b1/a11;
						else if ( b1 > a11 ) dd = a11-2.0*b1+c;
						else dd = c - b1*b1/a11;
						}
					}
				else if ( f >= e )
					{
					if ( (g < 0.0) && ((a01+b1) < 0.0) )		// t1 = 0
						{								// t0 = -b0/a00;
						if ( -b0 < 0.0 ) dd = c;
						else if ( -b0 > a00 ) dd = a00+2.0*b0+c;
						else dd = c - b0*b0/a00;
						}
					else if ( (g > e) && ((a01+b1) > a11)	)	// t1 = 1
						{								// t0 = (a01-b0)/a00;
						if ( a01 < b0 ) dd = a11-2.0*b1+c;
						else if ( (a01-b0) > a00 ) dd = a00-2.0*a01+a11+2.0*b0-2.0*b1+c;
						else dd = a11-2.0*b1+c-(a01-b0)*(a01-b0)/a00;
						}
					else {								// t0 = 1 &&  t1 = (a01+b1)/a11;
						if ( (a01+b1) < 0.0 ) dd = a00+2.0*b0+c;
						else if ( (a01+b1) > a11 ) dd = a00+2.0*b0+c-2.0*a01-2.0*b1+a11;
						else dd = a00+2.0*b0+c-(a01+b1)*(a01+b1)/a11;
						}
					}
				else
					{
					if ( g <= 0.0 )						// t1 = 0.0;
						{								// t0 = -b0/a00;
						if ( -b0 < 0.0 ) dd = c;
						else if ( -b0 > a00 ) dd = a00+2.0*b0+c;
						else dd = c - b0*b0/a00;
						}
					else if ( g >= e )					// t1 = 1.0;
						{								// t0 = (a01-b0)/a00;
						if ( a01 < b0 ) dd = a11-2.0*b1+c;
						else if ( (a01-b0) > a00 ) dd = a00-2.0*a01+a11+2.0*b0-2.0*b1+c;
						else dd = a11-2.0*b1+c-(a01-b0)*(a01-b0)/a00;
						}
					else //dd = 0.0;					// 0<t0<1 AND 0<t1<1 so have intersection dd = 0.0
						{								// but check to make sure not numerical error!
						dx = cp->x - p->x + (f*d0x - g*d1x)/e;
						dy = cp->y - p->y + (f*d0y - g*d1y)/e;
						dd = dx*dx + dy*dy;
						if ( dd < MIN_FLOAT ) dd = 0.0;
						}						
					}				
				}
			if ( dd < mind ) mind = dd;					// save minimum distance over all segments
			p = p->next;								// do next segment of this
			q = q->next;
			if ( (q == NULL) && closed ) q = points->first;
			}
		cp = cp->next;									// do next segment of c
		cq = cq->next;
		if ( (cq == NULL) && contour->closed ) cq = contour->points->first;
		}

	return( mind );
}


Points * Contour::XOR( Contour *bisector )		// given a chain coded Contour and a chain coded bisector
{												// return a partial subset of the exclusive XOR boundary
	Points *pts, *tracea, *traceb, *tmp;		// To get more, make additional calls until return NULL
	Point *n, *p, *q;
	double x, y, d, mind;
	bool usenext, complete, done;

	complete = true;
	q = bisector->points->first;
	while ( q )									// clear z labels in closed bisector
		{
		q->z = 0.0;
		q = q->next;
		}
	p = points->first;
	while ( p )									// find and marking all intersections
		{
		x = p->x;	y = p->y;   p->z = 0.0;		// make sure p->z is unlabeled
		q = bisector->points->first;
		while ( q )
			{									// test distance to bisector
			d = (x-q->x)*(x-q->x)+(y-q->y)*(y-q->y);
			if ( d < 1.2 )
				{
				p->z = -1.0;	q->z = -1.0;	// mark intersection point by z = -1
				complete = false;				// XOR only if there are intersections
				}
			q = q->next;
			}
		p = p->next;
		}

	pts = NULL;
	usenext = true;								// these variables will keep track of path
	tracea = points;							// as it switches between contours a and b
	p = tracea->first;
	traceb = bisector->points;
	while ( !complete )
		{
												// if p still in intersection, just skip over it
		while ( p->z < 0.0 )
			{
			p->z = 1.0;							// mark point as visited (can't loop forever!)
			if ( usenext ) p = p->next; else p = p->prev;	// follow tracea in correct direction
			if ( p == NULL )
				if  ( usenext ) p = tracea->first; else p = tracea->last; // allow wrap around
			}

		 if ( !usenext )						// when come out of intersection on bisector
			{									// check local orientation to confirm boundary crossing
			mind = MAX_FLOAT;
			x = p->x;	y = p->y;				// find closet point (n) on traceb (points)
			q = points->first;
			while ( q )
				{
				d = (x - q->x)*(x - q->x) + (y - q->y)*(y - q->y);
				if ( d < mind ) { mind = d; n = q; }
				q = q->next;
				}
			q = n->next;						// compute local direction (x,y) at n of points
			if ( q == NULL ) q = points->first;
			x = q->x - n->x;
			y = q->y - n->y;					// now do the same for the bisector (tracea)
			q = p->next;
			if ( q == NULL ) q = tracea->first;
			d = y*(q->x-p->x) - x*(q->y-p->y);	// compute cross-product of local directions
			if ( d < 0.0 )
				{								// intersection did not cross boundary!
				tracea = points;
				traceb = bisector->points;		// switch back to points
				usenext = true;
				p = n;
				while ( p->z < 0.0 )			// and exiting this intersection
					{
					p->z = 1.0;					// mark point as visited (can't loop forever!)
					p = p->next;
					if ( p == NULL ) p = tracea->first; // allow wrap around
					}
				}
			else if ( d == 0.0 )				// if can't detect orientation, abort XOR
				{
				if ( pts ) delete pts;
				return NULL;
				}
			}

		 while ( p->z == 0.0 )					// while search for next intersection, add pts
			{
			if ( usenext ) n = p->next; else n = p->prev;	// save pointer advance
			tracea->Extract( p );
			if ( !pts ) pts = new Points();		// prepare for new boundary piece
			pts->Add( p );
			p = n;								// move p to pts and advance pointer
			if ( p == NULL )
				if  ( usenext ) p = tracea->first; else p = tracea->last; // allow wrap around
			}

		if ( (p->z > 0.0) || !closed ) complete = true;	// terminate this piece

		if ( !complete )
			{									// next intersection => switch to other trace
			mind = MAX_FLOAT;
			x = p->x;	y = p->y;				// look in direction opposite will be moving
			q = traceb->first;
			while ( q )							// find closet point to p on traceb
				{
				d = (x - q->x)*(x - q->x) + (y - q->y)*(y - q->y);
				if ( d < mind ) { mind = d; n = q; }
				q = q->next;
				}
			p->z = 1.0;							// mark this pt on tracea as visited
			tmp = tracea;
			tracea = traceb;					// switch to follow traceb...
			traceb = tmp;
			usenext = !usenext;					// ...in opposite direction
			p = n;
			}
		}

	return pts;
}

void Contour::Smooth( const int length )	// smooth (x,y) pts using moving average filter
{											// (Note: Leave z untouched so can use for z-traces)
	Point  *p, *q;
	int i;
	double xMA, yMA, oldx, oldy;
	double *x, *y;

	x = new double[length];							// create lifos for filtering
	y = new double[length];

	if ( this->points )
		if (this->points->Number() > 2 )			// nothing to filter with fewer points
			{
			p = this->points->first;
			for (i=0; i<length/2+1; i++)
				{										// initialize filter
				x[i] = p->x; y[i] = p->y;
				}
			q = p;
			for (i=length/2+1; i<length; i++)			// redo second half to cover data
				{
				x[i] = q->x; y[i] = q->y;
				if ( q->next ) q = q->next;
				}
			xMA = 0; yMA = 0;							// initialize moving averages
			for (i=0; i<length; i++)
				{
				xMA += x[i]/(double)length;
				yMA += y[i]/(double)length;
				}
			while ( p )
				{
				p->x = xMA; p->y = yMA;					// store moving average in p
				oldx = x[0]; oldy = y[0];
				for (i=0; i<length-1; i++)
					{
					x[i] = x[i+1]; y[i] = y[i+1];		// shift filter values
					}
				if ( q )								// update filter with next data
					{									// if available
					q = q->next;
					if ( q )
						{							
						x[length-1] = q->x;
						y[length-1] = q->y;
						}
					}									// update moving averages
				xMA += (x[length-1]-oldx)/(double)length;
				yMA += (y[length-1]-oldy)/(double)length;
				p = p->next;							// shift to next point
				}
			}

	delete[] x;					// free memory
	delete[] y;
}