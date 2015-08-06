//////////////////////////////////////////////////////////////////////////////////////
// This is a port of the IGL Trace 3D Delaunay triangulation code for Reconstruct.
//
//    Copyright (C) 1996-2006  John Fiala (fiala@bu.edu)
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
// modified 07/17/03 by JCF
// -+- change: Replaced old contour classes with new contour objects for Reconstruct.
// modified 02/12/04 by JCF
// -+- change: Reduced limit on contour_complete from 2.0 to 0.001.
// -+- change: Wrote but did not install code for adding self-intersections to contours.
// modified 04/22/04 by JCF
// -+- change: Removed the storage of contours in VRMLObject
// modified 03/17/05 by JCF
// -+- change: Modified add-tetra, Next, and BoissonnatSurface to hide lower and upper faces
//              only on the first and last section of the object
// modified 4/14/06 by JCF (fiala@bu.edu)
// -+- change: Creation of 3D slabs output. Removed PlanarContourSurface.
// modified 5/4/06 by JCF (fiala@bu.edu)
// -+- change: Added section-by-section offsets using a difference of Z-traces
// modified 6/22/06 by JCF (fiala@bu.edu)
// -+- change: Changed M_PI constants to PI as defined in constants.h
//
#include "reconstruct.h"

//  ************************************************************************************
//  2D Delaunay triangulation and voronoi diagrams from contours
//
//  Classes defined:
//		index_point,index_point_list
//		triangle
//		edge, edge_list
//		triangle_list
//		graph_node,graph_edge,graph
//		delaunay_triangulation
//
//  Copyright (C) 1996,7
//		John C. Fiala
//

#define ORIENTED_UNKNOWN	0					// ORable constants for orientations
#define ORIENTED_INTERNAL	1
#define ORIENTED_BOUNDARY	2
#define ORIENTED_EXTERNAL	4
#define DONE_T1				8
#define DONE_T2				16

//#define RAY_XSMALL -2.0e4				// these constants set the limits of how
//#define RAY_YSMALL -1.6e4				// big an image/object can be handled
//#define RAY_XLARGE 2.0e4
//#define RAY_YLARGE 1.6e4
double RAY_XSMALL, RAY_YSMALL, RAY_XLARGE, RAY_YLARGE;

// ************************************** INDEX POINTS

class index_point {								// a 2D point with integer index
public:
	int index;
	double x, y;
	index_point *next;
};

class index_point_list {						// linked list of indexed points
public:
	index_point *head, *tail;				   // will add items at the tail
	int max_index;                // so can be traversed in order
	index_point_list() { head = NULL;
								tail = NULL;
								max_index = 0; }
	virtual ~index_point_list();
	index_point * add( double x, double y );
	void remove( index_point *ip );			// remove ip from list and delete
};

index_point_list::~index_point_list()		// destructor
{
	index_point  *tmp;

	tmp = head;
	while ( tmp != NULL ) {
	  head = tmp->next;
	  delete tmp;
	  tmp = head;
	  }
}														// add a new index point

index_point * index_point_list::add( double x, double y )
{
	index_point  *tmp = new index_point;	// create new item

	tmp->next = NULL;								// place at end of list
	if ( tail != NULL ) tail->next = tmp;
	if ( head == NULL ) head = tmp;
	tail = tmp;
	tmp->x = x;
	tmp->y = y;
	max_index++;									// assign next available index
	tmp->index = max_index;                // ASSUMES: will not exceed 32-bit capacity

	return tmp;                            // return ptr to item
}

void index_point_list::remove( index_point *ip )
{
	index_point *tmp;

	if ( ip == head ) head = ip->next;
	else {
		tmp = head;
		while ( tmp->next != ip ) tmp = tmp->next;
		tmp->next = ip->next;
		if ( ip == tail ) tail = tmp;
		}
	delete ip;
}
// *****************************************

// ************************************** DIRECTED TRIANGLES

class triangle {
public:
	index_point *v1;
	index_point *v2;
	index_point *v3;
	double xc, yc;
	double radius;
	int edge12;
	int edge23;
	int edge31;
	int oriented;
	triangle *prev, *next;
};
// *****************************************

// ************************************** DIRECTED EDGES

class edge {										// segment between two indexed pts
public:
	index_point *v1;
	index_point *v2;
	edge *next;
};

class edge_list {									// a list of edges
public:
	edge *head;
	edge_list() { head = NULL; }
	virtual ~edge_list();
	void add( index_point *p1, index_point *p2 );
	void add( triangle *t );		// add the edges of the triangle
	void split( edge *e, index_point *m );	// split e, using midpt m
	int orientation( triangle *t ); // -1=>opposite, 0=>unknown, 1=>same
	void remove_common();			// remove all occurrences of repeated edges
};

edge_list::~edge_list()
{
	edge  *tmp;

	tmp = head;
	while ( tmp != NULL ) {
	  head = tmp->next;
	  delete tmp;
	  tmp = head;
	  }
}

void edge_list::add(index_point *p1, index_point *p2)
{
	edge  *tmp;

	tmp = new edge;
	tmp->next = head;
	head = tmp;
	tmp->v1 = p1;		// can assume edge is directed from v1 -> v2
	tmp->v2 = p2;
}

void edge_list::add(triangle *t)
{
	edge  *tmp;

	tmp = new edge;		// add first edge of triangle
	tmp->next = head;
	head = tmp;
	tmp->v1 = t->v1;
	tmp->v2 = t->v2;

	tmp = new edge;		// add second edge
	tmp->next = head;
	head = tmp;
	tmp->v1 = t->v2;
	tmp->v2 = t->v3;

	tmp = new edge;		// add third edge
	tmp->next = head;
	head = tmp;
	tmp->v1 = t->v3;
	tmp->v2 = t->v1;
}

void edge_list::split(edge *e, index_point *m)
{
	edge *tmp;

	tmp = new edge;
	tmp->next = e->next;
	tmp->v1 = e->v1;	 // edge is directed from v1 -> m
	tmp->v2 = m;
	e->next = tmp;
	e->v1 = m;         // edge is directed from m -> v2
}

int edge_list::orientation( triangle *t ) // orientation of triangle wrt ordered
{                                      	// edge list; return <0 if external,
	edge  *e1, *e2;								// return >0 if internal,
	int  i, j, orient;		    				// 0 => not determinable from data
	double  dvx, dvy, dx1, dx2, dy1, dy2;
	double c, s, a1, a2;
	bool found;

	orient = 0;
	for (j=1; j<=3; j++) {						// do each vertex of t, sum results
		if ( j ==1 ) {
			i = t->v1->index;                // set vertex & direction of edge
			dvx = t->v2->x - t->v1->x;
			dvy = t->v2->y - t->v1->y;
			}
		if ( j == 2 ) {
			i = t->v2->index;
			dvx = t->v3->x - t->v2->x;
			dvy = t->v3->y - t->v2->y;
			}
		if ( j == 3 ) {
			i = t->v3->index;
			dvx = t->v1->x - t->v3->x;
			dvy = t->v1->y - t->v3->y;
			}

		found = false;								// search for matching contour vertex
		e2 = head;
		while ( (e2 != NULL) && (!found) ) {
			if ( e2->v1->index == i ) found = true;
			else e2 = e2->next;
			}

		if ( found ) {								// e2:v2-<-v1 == t:vi == e1:v2-<-v1
			e1 = e2->next;
			if ( e1 == NULL ) e1 = head;		// contour edge list is circular
			dx1 = e1->v1->x - e1->v2->x;     //            |  /
			dy1 = e1->v1->y - e1->v2->y;     //          e2| /v
			dx2 = e2->v2->x - e2->v1->x;     //      __e1__|/
			dy2 = e2->v2->y - e2->v1->y;     //
			c = (dx1*dx2 + dy1*dy2);       	// |e1||e2|cos(a1) = (-e1 . e2)
			s = (dx1*dy2 - dx2*dy1);         // |e1||e2|sin(a1) = (-e1 x e2)
			if ( (s != 0.0) || (c != 0.0) ) a2 = atan2(s,c);
			else a2 = 0.0;
			if ( a2 < 0.0 ) a2 = 2.0*PI + a2;
			c = (dx1*dvx + dy1*dvy);       	// |e1||v|cos(a2) = (-e1 . v)
			s = (dx1*dvy - dvx*dy1);       	// |e1||v|sin(a2) = (-e1 x v)
			if ( (s != 0.0) || (c != 0.0) ) a1 = atan2(s,c);
			else a1 = 0.0;
			if ( a1 < 0.0 ) a1 = 2.0*PI + a1;
			if ( a1 < a2 ) orient += -1;     // external (left-handed coord sys)
			if ( a1 > a2 ) orient += 1;	 	// internal
			}
		}

	return orient;
}



void edge_list::remove_common()
{
	edge  *d, *e, *f, *g;	// list is ... d -> e -> ... f -> g ...
	bool  repeated, matched;
									// search each list item
	d = NULL;
	e = head;
	while ( e != NULL ) {
	  repeated = false;
	  f = e;
	  g = e->next;          // for repetitions in remainder of list
	  while ( g != NULL ) {
			matched = false;
			if ( e->v1->index == g->v1->index ) {
				if ( e->v2->index == g->v2->index ) matched = true;
				}
			else if ( e->v2->index == g->v1->index ) {
				if ( e->v1->index == g->v2->index ) matched = true;
				}
			if ( matched ) {        // then delete g and will
				repeated = true;		// need to delete e also
				f->next = g->next;
				delete g;
				g = f->next;
				}
			else {						// otherwise check next list item
				f = g;
				g = g->next;
				}
			}
	  if ( repeated ) {				// then delete e and continue
		 if ( d == NULL ) {			// deleting head of list...
			head = e->next;
			delete e;
			e = head;
			}
		 else {							// reconnect d to rest of list
			d->next = e->next;
			delete e;
			e = d->next;
			}
		 }
	  else {								// otherwise check next list item
		 d = e;
		 e = e->next;
		 }
					// end while ( e != ...
	  }
}

// ************************************** TRIANGLES (LISTS)

class triangle_item {							// a linkable triangle ptr
public:
	triangle *tri;
	triangle_item *next;
};

class triangle_list {							// a list of triangle pointers
public:
	triangle_item *head;
	triangle_list() { head = NULL; }
	virtual ~triangle_list();
	void add( triangle *t );
};

triangle_list::~triangle_list()
{
	triangle_item  *tmp;

	tmp = head;
	while ( tmp != NULL ) {
	  head = tmp->next;
	  delete tmp;
	  tmp = head;
	  }
}

void triangle_list::add(triangle *t)
{
	triangle_item  *tmp;

	tmp = new triangle_item;
	tmp->next = head;
	head = tmp;
	tmp->tri = t;
}

// ******************************************

// *************************************** GRAPHS

class graph_node;			// declare class name only, will finish declaration below

class graph_edge {
public:
	int orientation;
	double dx, dy;			// direction of edge, dy/dx is slope of line from n1 to n2
	double minx, maxx, miny, maxy;	// bounding RPP of edge segment
	bool n1_dual_12;		// n1 duality indicators
	bool n1_dual_23;
	bool n1_dual_31;
	graph_node *n1;		// these are the endpoints, if n2==NULL then infinite
	graph_node *n2;
	bool n2_dual_12;		// n2 duality indicators
	bool n2_dual_23;
	bool n2_dual_31;
	graph_edge *next;
};

class graph_edge_ptr {	// to make a list of pointers to edges in main list
public:
	graph_edge *edge_ptr;
	graph_edge_ptr *next;
};

class graph_node_ptr {	// to make a list of pointers to nodes in main list
public:
	graph_node *node_ptr;
	graph_node_ptr *next;
};

class graph_node {			// a node of a graph (May not need full graph at all for reconstruction)
public:
	index_point *at;        // location, NULL if none
	triangle *tri;				// dual in delaunay triangulation or NULL
	graph_edge_ptr *neighbors;	// nodes connected to this one by edges
	graph_node 	*next,*prev;

	graph_node() {	neighbors = NULL;
						tri = NULL;
						at = NULL;
						next = NULL;
						prev = NULL;
					 }
	virtual ~graph_node();
	void add_neighbor(graph_edge *n);
	void remove_neighbor(graph_edge *e);
};

graph_node::~graph_node()
{
	graph_edge_ptr *tmp;

	tmp = neighbors;
	while ( tmp != NULL ) {
		neighbors = tmp->next;
		delete tmp;
		tmp = neighbors;
		}
}

void graph_node::add_neighbor(graph_edge *n)
{
	graph_edge_ptr *tmp;

	tmp = new graph_edge_ptr;
	tmp->next = neighbors;
	neighbors = tmp;
	tmp->edge_ptr = n;
}

void graph_node::remove_neighbor(graph_edge *e)		// remove *e from neighbors list
{
	graph_edge_ptr *p, *n;

	p = NULL;
	n = neighbors;
	while ( (n != NULL) && (n->edge_ptr != e) ) {	// look for match
			p = n;
			n = n->next;
			}

	if ( n != NULL ) {								// found, so delete it
		if ( n == neighbors ) neighbors = n->next;
		else p->next = n->next;
		delete n;
		}
}

class graph {									// a generic graph of 2D indexed points
public:
	index_point_list *locations;
	graph_node *head, *tail;
	graph_edge *edges;

	graph() {	head = NULL;
					tail = NULL;
					edges = NULL;
					locations = new index_point_list;
				};
	virtual ~graph();
	graph_node * add_node();
	graph_edge * add_edge(graph_node *n1, graph_node *n2);
};

graph::~graph()
{
	graph_node  *tmp;
	graph_edge *e;

	tmp = head;
	while ( tmp != NULL ) {
	  head = tmp->next;
	  delete tmp;
	  tmp = head;
	  }
	e = edges;
	while ( e != NULL ) {
	  edges = e->next;
	  delete e;
	  e = edges;
	  }
	delete locations;
}

graph_node * graph::add_node()
{
	graph_node  *tmp;

	tmp = new graph_node;				// place new node at head
	if ( head == NULL ) {
		head = tmp;
		tail = tmp;
		}
	else {
		tmp->next = head;
		head->prev = tmp;
		head = tmp;
		}

	return tmp;
}

graph_edge * graph::add_edge(graph_node *n1, graph_node *n2)
												// if edge exists, return pointer to it
{                                   // otherwise create new item and return
	graph_edge  *tmp;

	tmp = NULL;
	if ( n2 != NULL ) {					// always create in infinite edge case
		tmp = edges;
		while ( tmp != NULL ) {				// look for match

			if ( ((tmp->n1 == n1) && (tmp->n2 == n2))
				|| ((tmp->n2 == n1) && (tmp->n1 == n2)) ) break;
			tmp = tmp->next;
			}
		}

	if ( tmp == NULL ) {					// not found, so create a new one
		tmp = new graph_edge;
		tmp->next = edges;
		edges = tmp;
		tmp->n1 = n1;						// set endpoint node pointers
		tmp->n2 = n2;
		tmp->orientation = ORIENTED_UNKNOWN;	// set flags
		tmp->n1_dual_12 = false;
		tmp->n1_dual_23 = false;
		tmp->n1_dual_31 = false;
		tmp->n2_dual_12 = false;
		tmp->n2_dual_23 = false;
		tmp->n2_dual_31 = false;		// finally set bounding RPP
		if ( n2 != NULL ) {
			if ( n2->at->x > n1->at->x ) {	// use 2nd endpt for max x
				tmp->minx = n1->at->x;
				tmp->maxx = n2->at->x;
				}
			else {									// otherwise use 1st endpt
				tmp->minx = n2->at->x;
				tmp->maxx = n1->at->x;
				}
			if ( n2->at->y > n1->at->y ) {	// similarly for y
				tmp->miny = n1->at->y;
				tmp->maxy = n2->at->y;
				}
			else {
				tmp->miny = n2->at->y;
				tmp->maxy = n1->at->y;
				}
			}
		else {										// edge is an infinite ray starting a 1st endpt
			tmp->minx = n1->at->x;				// set correct maximums later...
			tmp->maxx = n1->at->x;
			tmp->miny = n1->at->y;
			tmp->maxy = n1->at->y;
			}
		}

	return tmp;								// return pointer
}
// *****************************************

int tris( triangle *head )
{
	int i = 0;
	while ( head != NULL ) { i++; head = head->next; }
	return( i );
}
// **************************************	DELAUNAY TRIANGULATION OF 2D POINTS

class delaunay_triangulation {				// contains a dbly-linked list of triangles
	index_point *bounding_box_pts;			// ptr to first (of 4) box index pts
	edge_list *contour_edges;				// contour segments in term of DT edges
public:
	index_point_list *pts;					// where this DT lies in larger vertex lists
	graph *voronoi;							// ptr to voronoi graph if present
	int index_offset;						// an offset of indices to other indices
	triangle *head, *tail;					// constructor needs initial enclosing box

	delaunay_triangulation();

	virtual ~delaunay_triangulation();		// destructor frees data pts

	void add( index_point *v1, index_point *v2, index_point *v3 );
	void remove( triangle *t );
	void remove_bounding_box();				// remove the initial enclosing box
	index_point * insert( double x, double y );		// add (x,y) to DT; return index
	void add_contour_intersections();		// add any self-intersections (eg. overlaps) 
	bool contour_complete();				// make sure that the contour edges are covered
	void mark_external();					// label triangles that are outside contour
	void triangulate( Contour *acontour, double shiftx = 0.0, double shifty = 0.0 );
	void create_voronoi_graph();			// create voronoi graph of triangulation
};


delaunay_triangulation::~delaunay_triangulation()
{
	triangle  *tmp;

	while ( head != NULL ) {
	  tmp = tail;
	  tail = tail->prev;
	  if ( tmp == head ) head = NULL;
	  delete tmp;
	  }
	delete contour_edges;
	delete pts;
	if ( voronoi != NULL ) delete voronoi;
}

delaunay_triangulation::delaunay_triangulation()
{
	index_point *v1, *v2, *v3, *v4;

	head = NULL;							// initialize data structures
	tail = NULL;
	voronoi = NULL;						// no voronoi graph initially
	pts = new index_point_list;
	contour_edges = new edge_list;
												// use large bounding box so that can be
												// removed later without affecting DT

	v1 = pts->add(RAY_XSMALL,RAY_YSMALL);
	bounding_box_pts = v1;
	v2 = pts->add(RAY_XLARGE,RAY_YSMALL);
	v3 = pts->add(RAY_XLARGE,RAY_YLARGE);
	v4 = pts->add(RAY_XSMALL,RAY_YLARGE);
												// construct enclosing complex
	add( v1, v2, v3 );
	add( v3, v4, v1 );
}

void delaunay_triangulation::add( index_point *v1, index_point *v2, index_point *v3 )
{
	triangle  *tmp;
	double dx21, dy21, dx31, dy31, p21, p31, d;

												// compute difference vectors
	dx21 = v2->x - v1->x;
	dy21 = v2->y - v1->y;
	dx31 = v3->x - v1->x;
	dy31 = v3->y - v1->y;
	d = dx21*dy31 - dx31*dy21;			// cross product gives area
	if ( d == 0.0 ) {						// don't add flat triangle
		return;
		}

	tmp = new triangle;					// create triangle item
	tmp->prev = tail;
	tmp->next = NULL;
	if ( head == NULL ) head = tmp;
	else  tail->next = tmp;
	tail = tmp;

	if ( d < 0.0 ) {						// add pts to triangle
		tmp->v1 = v1;                 // in clockwise order
		tmp->v2 = v3;
		tmp->v3 = v2;
		}
	else {
		tmp->v1 = v1;
		tmp->v2 = v2;
		tmp->v3 = v3;
		}
	tmp->oriented = ORIENTED_UNKNOWN;
	tmp->edge12 = ORIENTED_UNKNOWN;
	tmp->edge23 = ORIENTED_UNKNOWN;
	tmp->edge31 = ORIENTED_UNKNOWN;
	p21 = dx21*(v2->x+v1->x)/2.0 + dy21*(v2->y+v1->y)/2.0;
	p31 = dx31*(v3->x+v1->x)/2.0 + dy31*(v3->y+v1->y)/2.0;

	tmp->xc = ( p21*dy31 - p31*dy21 )/d;
	tmp->yc = ( dx21*p31 - dx31*p21 )/d;
	tmp->radius =  sqrt( (v1->x-tmp->xc)*(v1->x-tmp->xc) + (v1->y-tmp->yc)*(v1->y-tmp->yc) );
}

void delaunay_triangulation::remove(triangle *t)
{
	if ( t == head )
		head = t->next;
	else t->prev->next = t->next;
	if ( t == tail )
		tail = t->prev;
	else t->next->prev = t->prev;
	delete t;
}

void delaunay_triangulation::remove_bounding_box()		// call this once after all
{                                                  	// contour points are inserted
	int i, idx;
	triangle *t;
	index_point *tmp;
	triangle_item *it;
	triangle_list *withvertex;

	for (i=1; i<=4; i++) {					 	// remove triangles assoc. w/1st 4 index points

		if ( bounding_box_pts != NULL ) {

			withvertex = new triangle_list;
			idx = bounding_box_pts->index;

			t = head;								// search tri list for this index value
			while ( t != NULL ) {
				if ( (t->v1->index == idx) || (t->v2->index == idx) || (t->v3->index == idx) )

					withvertex->add( t ); 		// add it to list to remove

				t = t->next;
				}

			it = withvertex->head;				// remove triangles from triangulation
			while ( it != NULL ) {
				remove( it->tri );
				it = it->next;
				}

			delete withvertex;					// delete list of old triangles

			tmp = bounding_box_pts;
			bounding_box_pts = bounding_box_pts->next;	// do next index point
			pts->remove( tmp );
			}

		}												// end for (i...
}

index_point * delaunay_triangulation::insert(double x, double y)
{
	edge *e;                               // insert (x,y) into DT and
	triangle *t;                           // return ptr to its indexed point;
	triangle_item *it;                     // if (x,y) already in DT,
	index_point *p;			               // just return ptr to indexed point
	double d;
	//int i ,j;
	//char txt[80];

	//i = 0; j = 0;
	p = pts->head;                     		// search pts list for this point...
	while ( p != NULL ) {
		d = fabs(p->x-x) + fabs(p->y-y);
		if ( d == 0.0 ) {					// JCF 7/17/03 not all contour pts are integers!
			break;
			}
		else p = p->next;
		}

	if ( p == NULL ) {							// not found!

		p = pts->add(x,y);						// create and insert new index point

		triangle_list *intersected = new triangle_list;
		edge_list *edges = new edge_list;
		t = head;									// search for circum circles containing p
		while ( t != NULL ) {
			d = sqrt( (p->x-t->xc)*(p->x-t->xc) + (p->y-t->yc)*(p->y-t->yc) );
			if ( t->radius > d ) {
				intersected->add( t );			// add triangles intersected to list
				edges->add( t );					// add edges of triangle to edge list
				}
			t = t->next;
			}


		edges->remove_common();					// remove edges which are repeated
		
		it = intersected->head;					// remove old triangles from triangulation
		while ( it != NULL ) {
			remove( it->tri );
			it = it->next;
			//j++;
			}
		
		e = edges->head;							// add new triangles made by p and edges
		while ( e != NULL ) {
			add( e->v1, e->v2, p );
			e = e->next;
			//i++;
			}

		delete intersected;						// cleanup temp space
		delete edges;
		}

	return( p );									// return ptr to index pt for (x,y)
}

void delaunay_triangulation::add_contour_intersections()
{
	bool found, in_e, in_f;
	double d, x, y, edx, fdx, edy, fdy, eminx, emaxx, eminy, emaxy, fminx, fmaxx, fminy, fmaxy;
	index_point *p;
	edge *e, *f;
													// split edges which intersect
	e = contour_edges->head;
	while ( e != NULL )
		{
		eminx = e->v1->x;
		emaxx = e->v2->x;
		if ( emaxx < eminx ) { x = eminx; eminx = emaxx; emaxx = x; } 
		eminy = e->v1->y;
		emaxy = e->v2->y;
		if ( emaxy < eminy ) { y = eminy; eminy = emaxy; emaxy = y; }
		edx = e->v2->x - e->v1->x;
		edy = e->v2->y - e->v1->y;
		f = contour_edges->head;
		while ( f != NULL )
			{
			fminx = f->v1->x;
			fmaxx = f->v2->x;
			if ( fmaxx < fminx ) { x = fminx; fminx = fmaxx; fmaxx = x; } 
			fminy = f->v1->y;
			fmaxy = f->v2->y;
			if ( fmaxy < fminy ) { y = fminy; fminy = fmaxy; fmaxy = y; }
			found = false;
			if ( (e != f) && (fminx < emaxx) && (fmaxx > eminx) && (fminy < emaxy) && (fmaxy > eminy) )
				{
				fdx = f->v2->x - f->v1->x;			// RPPs overlap, check for detailed intersection
				fdy = f->v2->y - f->v1->y;
				if ( (edx != 0.0) && (fdx != 0.0) )
					{
					d = fdy/fdx - edy/edx;					// y - y1 = (dy1/dx1)(x - x1)
					if ( d != 0.0 )	            			// y - y2 = (dy2/dx2)(x - x2)
						{
						x = (f->v1->x*fdy/fdx  -  e->v1->x*edy/edx + e->v1->y - f->v1->y)/d;
						y = e->v1->y + (x-e->v1->x)*edy/edx;
						found = true;
						}
					}
				else if ( (edx == 0.0) && (fdx != 0.0) )	// e is vertical
					{
					x = e->v1->x;                 			// x is constant on e
					y = fdy*(x - f->v1->x)/fdx + f->v1->y;
					found = true;
					}
				else if ( (edx != 0.0) && (fdx == 0.0) )	// f is vertical
					{
					x = f->v1->x;                 			// x if constant on f
					y = edy*(x - e->v1->x)/edx + e->v1->y;
					found = true;
					}
				if ( found )
					{
					in_e = false;				// check if (x,y) is inside e's endpoints
					if ( edx == 0.0 )
						{
						if ( (y>eminy) && (y<emaxy) ) in_e = true;
						}
					else if ( edy == 0.0 )
						{
						if ( (x>eminx) && (x<emaxx) ) in_e = true;
						}
					else if ( (x<emaxx) && (x>eminx) && (y<emaxy) && (y>eminy) ) in_e = true;
					in_f = false;				// check if (x,y) is inside e's endpoints
					if ( fdx == 0.0 )
						{
						if ( (y>fminy) && (y<fmaxy) ) in_f = true;
						}
					else if ( fdy == 0.0 )
						{
						if ( (x>fminx) && (x<fmaxx) ) in_f = true;
						}
					else if ( (x<fmaxx) && (x>fminx) && (y<fmaxy) && (y>fminy) ) in_f = true;
					if ( in_e && in_f ) 
						{
						p = insert(x,y);					// add intersection point to triangulation
						contour_edges->split(e,p);			// split contour segment e
						contour_edges->split(f,p);			// split contour segment f
						}
					}
				}
			f = f->next;				// check rest of edges against this e
			}
		e = e->next;			// test next e edge in list
		}
}

bool delaunay_triangulation::contour_complete()
{
	bool found, changed;
	double epsilon = 0.001;			// JCF 2/12/04 changed to reflect
	double mx, my;
	index_point *p;
	edge *e;
	triangle *t;

	changed = false;					 // split edges which are not in triangulation
	e = contour_edges->head;       // ASSUMES: contours are all clockwise!!!
	while ( e != NULL ) {

			t = head;                // search for this edge in triangulation
			found = false;
			while ( (t != NULL ) ) {
				if ( e->v1->index == t->v1->index ) {
					if ( e->v2->index == t->v2->index ) {
						found = true;
						t->edge12 |= (ORIENTED_BOUNDARY | ORIENTED_INTERNAL);
						}
					else if ( e->v2->index == t->v3->index ) {
						found = true;
						t->edge31 |= (ORIENTED_BOUNDARY | ORIENTED_EXTERNAL);
						}
					}
				else if ( e->v1->index == t->v2->index ) {
					if ( e->v2->index == t->v3->index ) {
						found = true;
						t->edge23 |= (ORIENTED_BOUNDARY | ORIENTED_INTERNAL);
						}
					else if ( e->v2->index == t->v1->index ) {
						found = true;
						t->edge12 |= (ORIENTED_BOUNDARY | ORIENTED_EXTERNAL);
						}
					}
				else if ( e->v1->index == t->v3->index ) {
					if ( e->v2->index == t->v1->index ) {
						found = true;
						t->edge31 |= (ORIENTED_BOUNDARY | ORIENTED_INTERNAL);
						}
					else if ( e->v2->index == t->v2->index ) {
						found = true;
						t->edge23 |= (ORIENTED_BOUNDARY | ORIENTED_EXTERNAL);
						}
					}
				t = t->next;
				}
											// split non-Delaunay edges
			if ( !found ) {
				if ( (fabs(e->v1->x-e->v2->x) > epsilon) || (fabs(e->v1->y-e->v2->y) > epsilon) ) {
					mx = (e->v1->x + e->v2->x)/2.0;	// compute mid-point
					my = (e->v1->y + e->v2->y)/2.0;
					p = insert(mx,my);					// midpt insertion into DT
					contour_edges->split(e,p);			// split contour segment
					changed = true;
					e = contour_edges->head;			// now start over to find new problems
					}
				else {
					e = e->next;		// give up if segments get too small
					}
				}
			else e = e->next;			// go to next edge in list
			}

	return changed;
}

void delaunay_triangulation::mark_external()
{											// indicate which triangles are outside contour
	int i;
	triangle *t;

	t = head;						 // mark each triangle internal (external) if contains
	while ( t != NULL ) {       // an unambiguously internal (external) edge

		if ( (t->edge12 & ORIENTED_INTERNAL) && !(t->edge12 & ORIENTED_EXTERNAL) )
			t->oriented = ORIENTED_INTERNAL;
		else
			if ( (t->edge23 & ORIENTED_INTERNAL) && !(t->edge23 & ORIENTED_EXTERNAL) )
				t->oriented = ORIENTED_INTERNAL;
			else
				if ( (t->edge31 & ORIENTED_INTERNAL) && !(t->edge31 & ORIENTED_EXTERNAL) )
					t->oriented = ORIENTED_INTERNAL;

		if ( (t->edge12 & ORIENTED_EXTERNAL) && !(t->edge12 & ORIENTED_INTERNAL) )
			t->oriented = ORIENTED_EXTERNAL;
		else
			if ( (t->edge23 & ORIENTED_EXTERNAL) && !(t->edge23 & ORIENTED_INTERNAL) )
				t->oriented = ORIENTED_EXTERNAL;
			else
				if ( (t->edge31 & ORIENTED_EXTERNAL) && !(t->edge31 & ORIENTED_INTERNAL) )
					t->oriented = ORIENTED_EXTERNAL;

		if ( t->oriented == ORIENTED_UNKNOWN ) {	// only vertices on contour, no edges
			i = contour_edges->orientation( t );
			if ( i < 0 ) {
				t->oriented = ORIENTED_EXTERNAL;
				t->edge12 = ORIENTED_EXTERNAL;
				t->edge23 = ORIENTED_EXTERNAL;
				t->edge31 = ORIENTED_EXTERNAL;
				}
			else if (i > 0 ) {
				t->oriented = ORIENTED_INTERNAL;
				t->edge12 = ORIENTED_INTERNAL;
				t->edge23 = ORIENTED_INTERNAL;
				t->edge31 = ORIENTED_INTERNAL;
				}
			}													// use oriented to classify unknown edges

		if ( t->edge12 == ORIENTED_UNKNOWN ) t->edge12 = t->oriented;
		if ( t->edge23 == ORIENTED_UNKNOWN ) t->edge23 = t->oriented;
      if ( t->edge31 == ORIENTED_UNKNOWN ) t->edge31 = t->oriented;

		t = t->next;
		}
}

void delaunay_triangulation::triangulate(Contour *acontour, double shiftx, double shifty)
{
	Point *cp;
	index_point *ip, *last_ip, *first_ip;

	if ( acontour->points )							// modified by JCF 7/17/03
	  {
	  cp = acontour->points->first;					// insert contour points into DT
	  last_ip = NULL;
	  while ( cp != NULL )
		{
		ip = insert( cp->x+shiftx,cp->y+shifty );

		if ( last_ip != NULL )						// add segment to contour edges list
			contour_edges->add(last_ip,ip);
		else first_ip = ip;							// remember first index pt for below
		last_ip = ip;								// remember this index pt for next

		cp = cp->next;				// do next contour pt
		}
													// add contour edge which closes contour

	  if ( acontour->closed )
		if ( (first_ip != NULL) && (last_ip != NULL) )
			contour_edges->add(last_ip,first_ip);
	  }
}

void delaunay_triangulation::create_voronoi_graph()
{
	int i, j, k, ii, jj, kk;		// the algorithm also marks external nodes and edges
	triangle *t;                  // and so should only run after mark_externals() !!
	graph_node *nt, *ns, *n12, *n23, *n31;
	graph_edge *e;

	voronoi = new graph();

	t = head;						 // each triangle has dual in graph
	while ( t != NULL ) {
		nt = voronoi->add_node();	 // put circum center as nodes on graph
		nt->at = voronoi->locations->add(t->xc,t->yc);
		nt->tri = t;
		t = t->next;
		}

	nt = voronoi->head;		  	// find neighbors of each circum center node
	while ( nt != NULL ) {
		n12 = NULL;
		n23 = NULL;             // initially no triangles share a side
		n31 = NULL;
		i = nt->tri->v1->index;
		j = nt->tri->v2->index;
		k = nt->tri->v3->index;

		ns = voronoi->head;			// find triangles which share a side with t

		while ( (ns != NULL) && ((n12==NULL)||(n23==NULL)||(n31==NULL)) ) {
			if ( ns != nt ) {
				ii = ns->tri->v1->index;	// because all triangles are clkwise
				jj = ns->tri->v2->index;   // a neighboring triangle edge
				kk = ns->tri->v3->index;   // will match with opposite polarity
				if ( i==ii ) {
					if ( j==kk ) n12 = ns;
					if ( k==jj ) n31 = ns;
					}
				else if ( i==kk ) {
					if ( j==jj ) n12 = ns;
					if ( k==ii ) n31 = ns;
					}
				else if ( i==jj ) {
					if ( j==ii ) n12 = ns;
					if ( k==kk ) n31 = ns;
					}
				else if (( j==ii )&&( k==kk )) n23 = ns;
				else if (( j==kk )&&( k==jj )) n23 = ns;
				else if (( j==jj )&&( k==ii )) n23 = ns;
				}

			ns = ns->next;
			}

		e = voronoi->add_edge(nt,n12);		// Note: only eliminates non-boundary externals
		nt->add_neighbor(e);
		if ( nt == e->n1 ) {
			e->n1_dual_12 = true;
			e->orientation = nt->tri->edge12;
			e->dx = nt->tri->v2->y - nt->tri->v1->y;
			e->dy = nt->tri->v1->x - nt->tri->v2->x;
			if ( n12 == NULL ) {
				if ( e->dx > 0 ) e->maxx = RAY_XLARGE;
				else e->minx = RAY_XSMALL;
				if ( e->dy > 0 ) e->maxy = RAY_YLARGE;
				else e->miny = RAY_YSMALL;
				}
			}
		else {
			e->n2_dual_12 = true;
			e->orientation |= nt->tri->edge12;
			}

		e = voronoi->add_edge(nt,n23);
		nt->add_neighbor(e);
		if ( nt == e->n1 ) {
			e->n1_dual_23 = true;
			e->orientation = nt->tri->edge23;
			e->dx = nt->tri->v3->y - nt->tri->v2->y;
			e->dy = nt->tri->v2->x - nt->tri->v3->x;
			if ( n23 == NULL ) {
				if ( e->dx > 0 ) e->maxx = RAY_XLARGE;
				else e->minx = RAY_XSMALL;
				if ( e->dy > 0 ) e->maxy = RAY_YLARGE;
				else e->miny = RAY_YSMALL;
				}
         }
		else {
			e->n2_dual_23 = true;
			e->orientation |= nt->tri->edge23;
			}

		e = voronoi->add_edge(nt,n31);
		nt->add_neighbor(e);
		if ( nt == e->n1 ) {
			e->n1_dual_31 = true;
			e->orientation = nt->tri->edge31;
			e->dx = nt->tri->v1->y - nt->tri->v3->y;
			e->dy = nt->tri->v3->x - nt->tri->v1->x;
			if ( n31 == NULL ) {
				if ( e->dx > 0 ) e->maxx = RAY_XLARGE;
				else e->minx = RAY_XSMALL;
				if ( e->dy > 0 ) e->maxy = RAY_YLARGE;
				else e->miny = RAY_YSMALL;
            }
			}
		else {
			e->n2_dual_31 = true;
			e->orientation |= nt->tri->edge31;
			}

		nt = nt->next;
		}
}

// *************************************************************************************************
//
//		Reconstruction of 3D surface from planar contours
//
//  ASSUMPTIONS:
//		1. All contours represent real surface of object.
//		2. Clockwise contours bound exterior of object.
//		3. Counterclockwise contours bound interior of object.
//		4. Any counterclockwise contour is surrounded by clockwise contour.
//    5. Contours don't contain loops. (This can produce extraneous veils, etc.)
//
//	Copyright (c) 1998
//		John C. Fiala

#define TO_T1		1
#define FROM_T1	2
#define TO_T2		4
#define FROM_T2	8
#define FACE132	1
#define FACE143	2
#define FACE124	3
#define FACE234	4
// ************************************** TETRAHEDRON and lists of tetrahedra

class tetrahedron;							// complete class definition given later

class neighbors {								// a list of neighboring tetrahedra
public:
	tetrahedron *neighbor;
	int  direction;							// indicate whether neighbor is toward T1 or T2 node
	int  face;									// saves the face (of parent node) of this adjacency
	neighbors *next;
};

// ************************************** INDEXed VERTICES of reconstruction

class vertex {										// 3D point with integer index & normal
public:
	int index;
	double v[3];
	double n[3];
	neighbors *tetrahedra;						// list of tetrahedra which share this vertex
	vertex *prev, *next;
};

class vertex_list {								// doubly linked list of vertices
public:
	vertex *head, *tail;				   		// will add items at the tail
	int max_index;                         // head -> tail = lower section -> higher
	vertex_list() { head = NULL;
						 tail = NULL;
						 max_index = 0; }
	virtual ~vertex_list();
	vertex * add( double x, double y, double z);
};

vertex_list::~vertex_list()					// destructor
{
	vertex  *tmp;
	neighbors *n, *t;

	tmp = head;										// for each vertex...
	while ( tmp != NULL ) {
	  head = tmp->next;
	  n = tmp->tetrahedra;						// delete any neighbor list elements
	  while ( n != NULL ) {
			t = n->next;
			delete n;
			n = t;
			}
	  delete tmp;
	  tmp = head;
	  }

}														// add a new vertex

vertex * vertex_list::add(double x, double y, double z)
{
	vertex  *tmp = new vertex;					// create new item

	tmp->next = NULL;								// place at end of list
	tmp->prev = tail;
	if ( tail != NULL ) tail->next = tmp;
	if ( head == NULL ) head = tmp;
	tail = tmp;
	tmp->v[0] = x;									// set vertex values
	tmp->v[1] = y;
	tmp->v[2] = z;
	tmp->n[0] = 0.0;								// clear normals
	tmp->n[1] = 0.0;
	tmp->n[2] = 0.0;
	max_index++;									// assign next available index
	tmp->index = max_index;
	tmp->tetrahedra = NULL;						// initially no neighbors
	return tmp;                            // return ptr to item
}

// ***************************** Now complete the TETRAHEDRON definition

class tetrahedron {				// a single tetrahedron
public:
	vertex* v1;        			//       3 *
	vertex* v2;        			//
	vertex* v3;        			//    1 *
	vertex* v4;        			//             * 4
	bool up;							//       2 *
	bool impossible;
	int type;						// type = 0: edge in each sect, 1: face in sect1, 2: face in sect2
	int level;
	int index;						// for debugging purposes
	double n132[3];					// exterior-pointing normal vector of faces
	double n143[3];
	double n124[3];
	double n234[3];
	bool face132_visible;		// face visibility flags
	bool face143_visible;
	bool face124_visible;
	bool face234_visible;
	bool visited;
	bool hasT1;						// when has connectivity to both T1 and T2 nodes
	bool hasT2;                // then it belongs in reconstruction
	neighbors *neighbor_list;
	tetrahedron *prev, *next;
};

class section_recon {								// group tetrahedra and vertices by section
public:
	int sectnumber;
	vertex *first_vertex;
	tetrahedron *head, *tail;
	section_recon *prev, *next;
};

// ************************************** SURFACE RECONSTRUCTION FROM PARALLEL SECTIONS
// ++++++++++++++++++++++++++++++++++++++ based on Boissonnat J-D (1988) CVGIP 44:1-29.
class reconstruction {
public:
	delaunay_triangulation *last_dt;			// retained between sections
	double scalex, scaley, scalez, shiftx, shifty;
	Color reconstruction_color;					// JCF modified from unsigned long 7/17/03
	double contour_fidelity;
	vertex_list *vertices;            		// a reconstruction is a set of 3D vertices
	section_recon *head, *tail;					   // and a list of tetrahedra made from them
	int CurrentIndex;
	FILE *debugfp;
	BOOL debugon;

	reconstruction(double sx = 1.0, double sy = 1.0, double sz = 25.0, double fidelity = 1.0 )
		{
		head = NULL;
		tail = NULL;
		last_dt = NULL;
		shiftx = 0.0; shifty = 0.0;	// shiftx = 0.000002; shifty = 0.0000022; // for jiggling pixelized data?
		CurrentIndex = 0;
		vertices = new vertex_list;
		scalex = sx;
		scaley = sy;
		scalez = sz;
		contour_fidelity = fidelity;
      debugon = FALSE;
		}

	virtual ~reconstruction();  			// destructor frees memory
	section_recon * add_section(int sectnum);
	void add_tetra(section_recon *sect, int iv1, int iv2, int iv3, int iv4, int type,
					bool up, bool hideT1face, bool hideT2face);
	void first( Contours *contours, int sectnum, double z );
	void next ( Contours *contours, int sectnum, double z, double max_link_distance, bool hideLowerFaces, bool hideUpperFaces );
	void adjacencies(section_recon *sect);		// find adjacencies within section and eliminate non-solids
	void debug_on();
	void debug_off();
	void remove_neighbors( neighbors *nlist );
	neighbors * add( neighbors *nlist, tetrahedron *tetra, int f = 0 );
	void compute_vertex_normals();
	void compute_midpoint( triangle *t, double &x, double &y );
};


void reconstruction::debug_on()
{
	debugfp = fopen("debug.txt", "w");
	debugon = TRUE;
}

void reconstruction::debug_off()
{
	fclose(debugfp);
	debugon = FALSE;
}


void reconstruction::remove_neighbors( neighbors *nlist )
{
	neighbors *n, *t;

	n = nlist;
	while ( n != NULL ) {
		t = n->next;
		delete n;
		n = t;
		}
}

neighbors * reconstruction::add( neighbors *nlist, tetrahedron *tetra, int f )
{
	neighbors *n;

	n = new neighbors;
	n->neighbor = tetra;
	n->next = nlist;
	n->direction = 0;
	if ( tetra->type == 1 ) n->direction = FROM_T1;
	if ( tetra->type == 2 ) n->direction = FROM_T2;
	n->face = f;

	return n;
}

reconstruction::~reconstruction()
{
	section_recon  *s;
	tetrahedron *t;

	s = head;					// delete tetrahedra section-by-section
	while ( s != NULL ) {
	  t = s->head;
	  while ( t != NULL ) {
			s->head = t->next; 					// clear out neighbor lists before deleting t
			remove_neighbors( t->neighbor_list );
			delete t;
			t = s->head;
			}
	  head = s->next;
	  delete s;
	  s = head;
	  }

	delete vertices;
	if ( last_dt != NULL ) delete last_dt;
}

section_recon * reconstruction::add_section( int sectnum )
{
	section_recon *s;

	s = new section_recon;
	s->sectnumber = sectnum;
	s->first_vertex = NULL;
	s->head = NULL;
	s->tail = NULL;
															// place new section at tail
	if ( head == NULL ) {
		head = s;
		tail = s;
		s->next = NULL;
		s->prev = NULL;
		}
	else {
		s->next = NULL;
		s->prev = tail;
		tail->next = s;
		tail = s;
		}

	return s;
}

void reconstruction::add_tetra(section_recon *sect, int iv1, int iv2, int iv3, int iv4, int type,
								bool up, bool hideT1face, bool hideT2face )
{
	tetrahedron  *n;
	vertex *v;
	int i;
	double sgn, m, o[3], p[3], q[3], r[3], s[3];

	n = new tetrahedron;						// create new tetrahedron
	n->v1 = NULL;
	n->v2 = NULL;
	n->v3 = NULL;
	n->v4 = NULL;
	v = vertices->tail;						// search most recent vertices for these indices
	while ( (v != NULL) && ((n->v1==NULL)||(n->v2==NULL)||(n->v3==NULL)||(n->v4==NULL)) ) {
		if ( v->index == iv1 ) n->v1 = v;
		else if ( v->index == iv2 ) n->v2 = v;
		else if ( v->index == iv3 ) n->v3 = v;
		else if ( v->index == iv4 ) n->v4 = v;
		v = v->prev;
		}											// add n to tetrahedra lists for each vertex

	n->v1->tetrahedra = add( n->v1->tetrahedra, n );
	n->v2->tetrahedra = add( n->v2->tetrahedra, n );
	n->v3->tetrahedra = add( n->v3->tetrahedra, n );
	n->v4->tetrahedra = add( n->v4->tetrahedra, n );

	n->type = type;							// fill-in components of tetrahedron
	CurrentIndex++;
	n->index = CurrentIndex;
	n->impossible = false;
	n->level = sect->sectnumber;
	n->up = up;
	n->visited = false;
	if ( type == 1 ) n->hasT1 = true;
	else n->hasT1 = false;
	if ( type == 2 ) n->hasT2 = true;
	else n->hasT2 = false;
	n->face132_visible = true;          // with all faces visible
	n->face143_visible = true;
	n->face124_visible = true;
	n->face234_visible = true;
	if ( (type == 1) && hideT1face ) n->face132_visible = false;
	if ( (type == 2) && hideT2face ) n->face132_visible = false;
	n->neighbor_list = NULL;

											// compute face normal vectors

	for (i=0; i<3; i++) o[i] = n->v2->v[i] - n->v1->v[i];
	for (i=0; i<3; i++) p[i] = n->v3->v[i] - n->v1->v[i];
	for (i=0; i<3; i++) q[i] = n->v4->v[i] - n->v1->v[i];
	for (i=0; i<3; i++) r[i] = n->v4->v[i] - n->v2->v[i];
	for (i=0; i<3; i++) s[i] = n->v3->v[i] - n->v2->v[i];
	n->n132[0] = (o[1]*p[2]-o[2]*p[1]);
	n->n132[1] = -(o[0]*p[2]-o[2]*p[0]);
	n->n132[2] = (o[0]*p[1]-o[1]*p[0]);
	n->n143[0] = (p[1]*q[2]-p[2]*q[1]);
	n->n143[1] = -(p[0]*q[2]-p[2]*q[0]);
	n->n143[2] = (p[0]*q[1]-p[1]*q[0]);
	n->n124[0] = (q[1]*o[2]-q[2]*o[1]);
	n->n124[1] = -(q[0]*o[2]-q[2]*o[0]);
	n->n124[2] = (q[0]*o[1]-q[1]*o[0]);
	n->n234[0] = (r[1]*s[2]-r[2]*s[1]);
	n->n234[1] = -(r[0]*s[2]-r[2]*s[0]);
	n->n234[2] = (r[0]*s[1]-r[1]*s[0]);


	if ( up ) sgn = -1.0;				// and normalize them
	else sgn = 1.0;
	m = sgn*sqrt(n->n132[0]*n->n132[0] + n->n132[1]*n->n132[1] + n->n132[2]*n->n132[2]);
	for (i=0; i<3; i++) n->n132[i] = n->n132[i]/m;
	m = sgn*sqrt(n->n143[0]*n->n143[0] + n->n143[1]*n->n143[1] + n->n143[2]*n->n143[2]);
	for (i=0; i<3; i++) n->n143[i] = n->n143[i]/m;
	m = sgn*sqrt(n->n124[0]*n->n124[0] + n->n124[1]*n->n124[1] + n->n124[2]*n->n124[2]);
	for (i=0; i<3; i++) n->n124[i] = n->n124[i]/m;
	m = sgn*sqrt(n->n234[0]*n->n234[0] + n->n234[1]*n->n234[1] + n->n234[2]*n->n234[2]);
	for (i=0; i<3; i++) n->n234[i] = n->n234[i]/m;

													// place new tetra at head of section
	if ( sect->head == NULL ) {
		sect->head = n;
		sect->tail = n;
		n->next = NULL;
		n->prev = NULL;
		}
	else {
		n->next = sect->head;
		n->prev = NULL;
		sect->head->prev = n;
		sect->head = n;
		}
}

void reconstruction::compute_midpoint( triangle *t, double &x, double &y )
{
	double a, b, c, d, e, f, m0, m1, x0, x1, y0, y1;
	double EPSILON = 0.00005;
	bool have0 = false;
	bool have1 = false;

	a = (t->v1->x + t->v2->x)/2.0;	// compute midpoints of sides
	b = (t->v1->y + t->v2->y)/2.0;
	c = (t->v2->x + t->v3->x)/2.0;
	d = (t->v2->y + t->v3->y)/2.0;
	e = (t->v1->x + t->v3->x)/2.0;
	f = (t->v1->y + t->v3->y)/2.0;

	if ( fabs( c - t->v1->x ) > EPSILON ) {	// form two linear equations
		have0 = true;                          // from the three possible lines
		x0 = t->v1->x;
		y0 = t->v1->y;
		m0 = (d-y0)/(c-x0);
		}
	if ( fabs( a - t->v3->x ) > EPSILON ) {
		if ( have0 ) {
			have1 = true;
			x1 = t->v3->x;
			y1 = t->v3->y;
			m1 = (b-y1)/(a-x1);
			}
		else {
			//have0 = true;
			x0 = t->v3->x;
			y0 = t->v3->y;
			m0 = (b-y0)/(a-x0);
			}
		}
	if ( fabs( e - t->v2->x ) > EPSILON )
		if ( !have1 ) {
			x1 = t->v2->x;
			y1 = t->v2->y;
			m1 = (f-y1)/(e-x1);
			}

	a = (x0*m0 - x1*m1 - y0 +y1)/(m0 - m1);	// solve for x and y
	x = a;
	y = m0*a - x0*m0 + y0;
}

void reconstruction::first( Contours *contours, int sectnum, double z )
{
	Contour *c;
	index_point *ip;
	vertex *iq;
	section_recon *sect;

	last_dt = new delaunay_triangulation();
														// add each matching contour in list...
	c = contours->first;
	while ( c )
		{
		if ( c->points->Number() > 2 )
			last_dt->triangulate( c );	// add contour pts to triangulation
		c = c->next;
		}

//	last_dt->add_contour_intersections(); // JCF added to test 2/10/04
	last_dt->contour_complete();				// make sure triangulation covers contour
	last_dt->remove_bounding_box();			// remove enclosing complex
	last_dt->mark_external();					// label external triangles
	last_dt->create_voronoi_graph();			// create voronoi graph

														// add final triangulation points to vertices
	ip = last_dt->pts->head;
	iq = vertices->add( ip->x, ip->y, z);
	last_dt->index_offset = ip->index - iq->index;
	ip = ip->next;
	while ( ip != NULL ) {
		vertices->add( ip->x, ip->y, z);
		ip = ip->next;
		}

	sect = add_section( sectnum );			// create a new section of tetrahedra
	sect->first_vertex = iq;
}

void reconstruction::next( Contours *contours, int sectnum, double z, double max_link_distance,
								bool hideLowerFaces, bool hideUpperFaces )
{
	graph_edge *ge1, *ge2;
	index_point *v1, *v2, *v3, *v4;
	int edge12, edge34;
	bool valid;
	index_point_list *pts_to_add;
	Contour *c;
	bool firstcontour, changing;
	delaunay_triangulation *dt;
	double d, x, y, min_d, x1, y1, a1, b1, a2, b2, a3, b3;
	int min_index, iv1, iv2, iv3, iv4, i;
	index_point *ip;
	vertex *iq;
	graph_node *n;
	triangle *t, *t2;
	section_recon *sect;
	const double EPSILON = 0.00005;
														// so create triangulation for it
	dt = new delaunay_triangulation();
														// add each matching contour in list...
	c = contours->first;
	while ( c )
		{
		if ( c->points->Number() > 2 )
			dt->triangulate( c );	// add contour pts to triangulation
		c = c->next;
		}
//	dt->add_contour_intersections(); // JCF added to test 2/10/04
	changing = dt->contour_complete();		// make sure triangulation covers contours
	changing = true;
	while ( changing ) {							// continue until conditions are satisfied:
		changing = false;							// 1) DT must cover the contours
													// 2) no correspondences between circumcenters

		pts_to_add = new index_point_list;	// find conflicts between sections...
		t = last_dt->head;
		while ( t != NULL ) {
			if ( !(t->oriented & ORIENTED_EXTERNAL) ) {
				t2 = dt->head;
				while ( t2 != NULL ) {
					if ( (t->xc==t2->xc) && (t->yc==t2->yc) ) { // coincident circumcenters
						compute_midpoint( t, x, y );			// so find midpt of triangle
						pts_to_add->add( x, y );			    // and add to this triangulation (JCF took out ip=)
						}
					t2 = t2->next;
					}                           // does this approach correctly handle circumcenters
				}                               // which lie outside their delaunay triangles??
			t = t->next;
			}
												// now update the triangulation with conflicts
		ip = pts_to_add->head;
		while ( ip != NULL ) {					// need to ensure that these triangles receive
			dt->insert(ip->x,ip->y);			// the proper orientation since may involve
			ip = ip->next;						// points which do not lie on the contour boundary
			}
		delete pts_to_add;						// free memory

		changing = dt->contour_complete();		// make sure triangulation covers contours
		}
	
	dt->remove_bounding_box();					// remove enclosing complex
	dt->mark_external();						// label external triangles
	dt->create_voronoi_graph();					// create voronoi

												// add final triangulation points to vertices
	ip = dt->pts->head;
	iq = vertices->add( ip->x, ip->y, z);
	dt->index_offset = ip->index - iq->index;
	ip = ip->next;
	while ( ip != NULL ) {
		vertices->add( ip->x, ip->y, z);
		ip = ip->next;
		}

	sect = add_section( sectnum );			// create a new section of tetrahedra
	sect->first_vertex = iq;
	
	n = last_dt->voronoi->head;			// find rest of tetrahedra of DT1 triangulation
	while ( n != NULL ) {
		t = n->tri;
		if ( !(t->oriented & ORIENTED_EXTERNAL) )
			{
			ip = dt->pts->head;				// calc min distance to points in next section
			min_index = -1;
			while ( ip != NULL ) {
				d = (ip->x-t->xc)*(ip->x-t->xc) + (ip->y-t->yc)*(ip->y-t->yc);
				if ( min_index > 0 ) {
					if ( d < min_d ) {
						min_d = d;
						min_index = ip->index;
						}
					}
				else {
					min_d = d;
					min_index = ip->index;
					}
				ip = ip->next;
				}
			if ( (min_index > 0) && (min_d < max_link_distance) )
				{
				iv1 = t->v1->index - last_dt->index_offset;
				iv2 = t->v2->index - last_dt->index_offset;
				iv3 = t->v3->index - last_dt->index_offset;
				iv4 = min_index - dt->index_offset;
				add_tetra(sect,iv1,iv2,iv3,iv4,1,true,hideLowerFaces,hideUpperFaces);
				}
			}
		n = n->next;
		}


	n = dt->voronoi->head;			// find rest of tetrahedra of DT2 triangulation
	while ( n != NULL ) {
		t = n->tri;
		if ( !(t->oriented & ORIENTED_EXTERNAL) )
			{
			ip = last_dt->pts->head;
			min_index = -1; 			 // locate closest pt in first section
			while ( ip != NULL ) {
				d = (ip->x-t->xc)*(ip->x-t->xc) + (ip->y-t->yc)*(ip->y-t->yc);
				if ( min_index > 0 ) {
					if ( d < min_d ) {
						min_d = d;
						min_index = ip->index;
						}
					}
				else {
					min_d = d;
					min_index = ip->index;
					}
				ip = ip->next;			// NOTE: its only necessary to test T1 & T2 nodes for max_link_distance
				}						// becuz absence of T1 and T2 will make T12's non-solid in adjacenies()
			if ( (min_index > 0) && (min_d < max_link_distance) )
				{
				iv1 = t->v1->index - dt->index_offset;
				iv2 = t->v2->index - dt->index_offset;
				iv3 = t->v3->index - dt->index_offset;
				iv4 = min_index - last_dt->index_offset;
				add_tetra(sect,iv1,iv2,iv3,iv4,2,false,hideLowerFaces,hideUpperFaces);
				}
			}
		n = n->next;
		}

											// compute intersection points between voronoi diagrams
											// to find tetrahedra with edge in both DT1 and DT2...

	ge1 = last_dt->voronoi->edges;   // for each edge of g1...
	while ( ge1 != NULL ) {
		ge2 = dt->voronoi->edges;		// find every edge of g2 that intersects it
		while ( ge2 != NULL ) {       // in a single pt, parallel or coincident
			valid = false;             // lines cannot form valid tetrahedra

			if ( (ge1->minx > ge2->maxx) || (ge1->maxx < ge2->minx)
					|| (ge1->miny > ge2->maxy) || (ge1->maxy < ge2->miny) ) ;  // no intersection
			else {
												// RPPs overlap, check for detailed intersection

				if ( (ge1->dx != 0.0) && (ge2->dx != 0.0) ) {

					d = ge2->dy/ge2->dx - ge1->dy/ge1->dx;	// y - y1 = (dy1/dx1)(x - x1)
					if ( d != 0.0 ) {             			// y - y2 = (dy2/dx2)(x - x2)
						valid = true;
						x = (ge2->n1->at->x*ge2->dy/ge2->dx  -  ge1->n1->at->x*ge1->dy/ge1->dx
								+ ge1->n1->at->y - ge2->n1->at->y)/d;
						y = ge1->n1->at->y + (x-ge1->n1->at->x)*ge1->dy/ge1->dx;
						}
					}
				else if ( (ge1->dx == 0.0) && (ge2->dx != 0.0) ) {	// vertical ge1
					valid = true;
					x = ge1->n1->at->x;                 	// x = x1
					y = ge2->dy*(x - ge2->n1->at->x)/ge2->dx + ge2->n1->at->y;
					}
				else if ( (ge1->dx != 0.0) && (ge2->dx == 0.0) ) {	// vertical ge2
					valid = true;
					x = ge2->n1->at->x;                 	// x = x2
					y = ge1->dy*(x - ge1->n1->at->x)/ge1->dx + ge1->n1->at->y;
					}
																	// there is an intersection so check if
				if ( valid &&										// within endpoints + fudge factor
						(x<ge1->maxx+EPSILON) && (x>ge1->minx-EPSILON) && (y<ge1->maxy+EPSILON) && (y>ge1->miny-EPSILON)
						&& (x<ge2->maxx+EPSILON) && (x>ge2->minx-EPSILON) && (y<ge2->maxy+EPSILON) && (y>ge2->miny-EPSILON) )
					{
					edge12 = ge1->orientation;
					edge34 = ge2->orientation;
					if ( ge1->n1_dual_12 ) {				// get edges to which the voronoi edges are duals
						v1 = ge1->n1->tri->v1;
						v2 = ge1->n1->tri->v2;
//							edge12 = ge1->n1->tri->edge12;
						}
					else if ( ge1->n1_dual_23 ) {
						v1 = ge1->n1->tri->v2;
						v2 = ge1->n1->tri->v3;
//							edge12 = ge1->n1->tri->edge23;
						}
					else { // ge1->n1_dual_31
						v1 = ge1->n1->tri->v3;
						v2 = ge1->n1->tri->v1;
//							edge12 = ge1->n1->tri->edge31;
						}
					if ( ge2->n1_dual_12 ) {
						v3 = ge2->n1->tri->v1;
						v4 = ge2->n1->tri->v2;
//							edge34 = ge2->n1->tri->edge12;
						}
					else if ( ge2->n1_dual_23 ) {
						v3 = ge2->n1->tri->v2;
						v4 = ge2->n1->tri->v3;
//							edge34 = ge2->n1->tri->edge23;
						}
					else { // ge2->n1_dual_31
						v3 = ge2->n1->tri->v3;
						v4 = ge2->n1->tri->v1;
//							edge34 = ge2->n1->tri->edge31;
						}
														// create tetra if neither edge is non-boundary external
														// (using & EXTERNAL gets rid of a lot of boundary T12's)

					if ( ((edge12 & ORIENTED_EXTERNAL) && !(edge12 & ORIENTED_INTERNAL))
							|| ((edge34 & ORIENTED_EXTERNAL) && !(edge34 & ORIENTED_INTERNAL)) ) ;
					else {
						iv1 = v1->index - last_dt->index_offset;
						iv2 = v2->index - last_dt->index_offset;
						x1 = v1->x;
						y1 = v1->y;
						a1 = v2->x - x1;
						b1 = v2->y - y1;
						iv3 = v3->index - dt->index_offset;
						iv4 = v4->index - dt->index_offset;
						a2 = v3->x - x1;
						b2 = v3->y - y1;
						a3 = v4->x - x1;
						b3 = v4->y - y1;		
						d = -(a1*b3-b1*a3) + (a1*b2 - b1*a2);
						if ( d < 0 )
							add_tetra(sect,iv1,iv2,iv3,iv4,0,false,hideLowerFaces,hideUpperFaces);
						else add_tetra(sect,iv1,iv2,iv3,iv4,0,true,hideLowerFaces,hideUpperFaces);
						}
					}		// end if ( valid...

				} // end else

			ge2 = ge2->next;
			}					// end while (ge2 != NULL)
		ge1 = ge1->next;
		}      				// end while (ge1 != NULL)

	adjacencies( sect );			// now mark invisible adjacent faces
	
	d = shiftx;
	shiftx = -shifty;
	shifty = -d;
	if ( last_dt != NULL ) delete last_dt;					// prepare for next section
	last_dt = dt;
}


																// find adjacent faces
void reconstruction::adjacencies(section_recon *sect)
{
	vertex *v[4];
	neighbors *nbr, *beenthere, *bt;
	tetrahedron *t, *n, *n0, *n1;
	bool a, b, c, d, e, f, g, h, i, found, changing, donethat;
	int nv1, nv2, nv3, nv4, tv1, tv2, tv3, tv4;
	int n0v1, n0v2, n0v3, n0v4, n1v1, n1v2, n1v3, n1v4;
	int j, k;
	double dot;
													// for every face, find its neighbors
	t = sect->head;
	while ( t != NULL ) {
		tv1 = t->v1->index;					// test vertices by comparing indices
		tv2 = t->v2->index;
		tv3 = t->v3->index;
		tv4 = t->v4->index;
		t->visited = true;
		v[0] = t->v1;							// check all tetrahedra attached at each vertex
		v[1] = t->v2;
		v[2] = t->v3;
		v[3] = t->v4;
		beenthere = NULL;						// keep track of which nodes were already visited

		for (k=0; k<4; k++) {
		 nbr = v[k]->tetrahedra;			// look at tetrahedra which share this vertex
		 while ( nbr != NULL ) {
		  n = nbr->neighbor;					// check each tetrahedron in list of neighbrs
		  donethat = n->visited;
		  bt = beenthere;                 // but make sure we haven't tried it already
		  while ( !donethat && (bt != NULL) ) {
				if ( bt->neighbor == n ) donethat = true;
				bt = bt->next;
				}
		  if ( !donethat ) {
			 beenthere = add( beenthere, n );
			 nv1 = n->v1->index;
			 nv2 = n->v2->index;
			 nv3 = n->v3->index;
			 nv4 = n->v4->index;				// CASE 1: both are T12 nodes

			 if ( (t->type == 0) && (n->type == 0) ) {
																	// 1) bottom edges of 2 T12's match

				if ( ((nv1==tv1) && (nv2==tv2)) || ((nv1==tv2) && (nv2==tv1)) ) {
					if (nv3 == tv3) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + t->n132[j]*n->n132[j];
						if ( dot < 0.0 ) {
							t->neighbor_list = add( t->neighbor_list, n, FACE132 );
							n->neighbor_list = add( n->neighbor_list, t, FACE132 );
							}
						}
					else if (nv3 == tv4) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + t->n124[j]*n->n132[j];
						if ( dot < 0.0 ) {
							t->neighbor_list = add( t->neighbor_list, n, FACE124 );
							n->neighbor_list = add( n->neighbor_list, t, FACE132 );
//							t->face124_visible = false;
//							n->face132_visible = false;
							}
						}
					else if (nv4 == tv4) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + t->n124[j]*n->n124[j];
						if ( dot < 0.0 ) {
							t->neighbor_list = add( t->neighbor_list, n, FACE124 );
							n->neighbor_list = add( n->neighbor_list, t, FACE124 );
//							t->face124_visible = false;
//							n->face124_visible = false;
							}
						}
					else if (nv4 == tv3) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + t->n132[j]*n->n124[j];
						if ( dot < 0.0 ) {
							t->neighbor_list = add( t->neighbor_list, n, FACE132 );
							n->neighbor_list = add( n->neighbor_list, t, FACE124 );
//							t->face132_visible = false;
//							n->face124_visible = false;
							}
						}
					}												// top edges of T12's match
				if ( ((nv3==tv3) && (nv4==tv4)) || ((nv3==tv4) && (nv4==tv3)) ) {
					if (nv1 == tv1) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + t->n143[j]*n->n143[j];
						if ( dot < 0.0 ) {
							t->neighbor_list = add( t->neighbor_list, n, FACE143 );
							n->neighbor_list = add( n->neighbor_list, t, FACE143 );
//							t->face143_visible = false;
//							n->face143_visible = false;
							}
						}
					else if (nv1 == tv2) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + t->n234[j]*n->n143[j];
						if ( dot < 0.0 ) {
							t->neighbor_list = add( t->neighbor_list, n, FACE234 );
							n->neighbor_list = add( n->neighbor_list, t, FACE143 );
//							t->face234_visible = false;
//							n->face143_visible = false;
							}
						}
					else if (nv2 == tv2) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + t->n234[j]*n->n234[j];
						if ( dot < 0.0 ) {
							t->neighbor_list = add( t->neighbor_list, n, FACE234 );
							n->neighbor_list = add( n->neighbor_list, t, FACE234 );
//							t->face234_visible = false;
//							n->face234_visible = false;
							}
						}
					else if (nv2 == tv1) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + t->n143[j]*n->n234[j];
						if ( dot < 0.0 ) {
							t->neighbor_list = add( t->neighbor_list, n, FACE143 );
							n->neighbor_list = add( n->neighbor_list, t, FACE234 );
//							t->face143_visible = false;
//							n->face234_visible = false;
							}
						}
					}
				}
			else if ( (t->type == 0) || (n->type == 0) ) {		// one is type T12, other T1 or T2
				if ( t->type == 0 ) {
					n0 = t;
					n0v1 = tv1;
					n0v2 = tv2;
					n0v3 = tv3;
					n0v4 = tv4;
					n1 = n;
					n1v1 = nv1;
					n1v2 = nv2;
					n1v3 = nv3;
					n1v4 = nv4;
					}
				else {						 							// n0 is type T12 tetra
					n0 = n;
					n0v1 = nv1;
					n0v2 = nv2;
					n0v3 = nv3;
					n0v4 = nv4;
					n1 = t;
					n1v1 = tv1;
					n1v2 = tv2;
					n1v3 = tv3;
					n1v4 = tv4;
					}
				if ( n1->type == 1 ) {
					a = (n1v1 == n0v1);
					b = (n1v1 == n0v2);
					c = (n1v2 == n0v1);
					d = (n1v2 == n0v2);
					e = (n1v3 == n0v1);
					f = (n1v3 == n0v2);
					}
				else {
					a = (n1v1 == n0v3);
					b = (n1v1 == n0v4);
					c = (n1v2 == n0v3);
					d = (n1v2 == n0v4);
					e = (n1v3 == n0v3);
					f = (n1v3 == n0v4);
					}
				if ( (a && d) || (b && c) ) {
					if ( n1v4 == n0v1 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n124[j]*n0->n143[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE124 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE143 );
//							n1->face124_visible = false;
//							n0->face143_visible = false;
							}
						else n0->impossible = true;	// conflict w/ type T1 or T2
						}
					else if ( n1v4 == n0v2 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n124[j]*n0->n234[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE124 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE234 );
//							n1->face124_visible = false;
//							n0->face234_visible = false;
							}
						else n0->impossible = true;
						}
					else if ( n1v4 == n0v3 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n124[j]*n0->n132[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE124 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE132 );
//							n1->face124_visible = false;
//							n0->face132_visible = false;
							}
						else n0->impossible = true;
						}
					else if ( n1v4 == n0v4 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n124[j]*n0->n124[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE124 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE124 );
//							n1->face124_visible = false;
//							n0->face124_visible = false;
							}
						else n0->impossible = true;
						}
					}
				else if ( (c && f) || (d && e) ) {
					if ( n1v4 == n0v1 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n234[j]*n0->n143[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE234 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE143 );
//							n1->face234_visible = false;
//							n0->face143_visible = false;
							}
						else n0->impossible = true;
						}
					else if ( n1v4 == n0v2 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n234[j]*n0->n234[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE234 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE234 );
//							n1->face234_visible = false;
//							n0->face234_visible = false;
							}
						else n0->impossible = true;
						}
					else if ( n1v4 == n0v3 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n234[j]*n0->n132[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE234 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE132 );
//							n1->face234_visible = false;
//							n0->face132_visible = false;
							}
						else n0->impossible = true;
						}
					else if ( n1v4 == n0v4 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n234[j]*n0->n124[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE234 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE124 );
//							n1->face234_visible = false;
//							n0->face124_visible = false;
							}
						else n0->impossible = true;
						}
					}
				else if ( (a && f) || (b && e) ) {
					if ( n1v4 == n0v1 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n143[j]*n0->n143[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE143 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE143 );
//							n1->face143_visible = false;
//							n0->face143_visible = false;
							}
						else n0->impossible = true;
						}
					else if ( n1v4 == n0v2 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n143[j]*n0->n234[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE143 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE234 );
//							n1->face143_visible = false;
//							n0->face234_visible = false;
							}
						else n0->impossible = true;
						}
					else if ( n1v4 == n0v3 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n143[j]*n0->n132[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE143 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE132 );
//							n1->face143_visible = false;
//							n0->face132_visible = false;
							}
						else n0->impossible = true;
						}
					else if ( n1v4 == n0v4 ) {
						dot = 0.0;
						for (j=0; j<3; j++) dot = dot + n1->n143[j]*n0->n124[j];
						if ( dot < 0.0 ) {
							n1->neighbor_list = add( n1->neighbor_list, n0, FACE143 );
							n0->neighbor_list = add( n0->neighbor_list, n1, FACE124 );
//							n1->face143_visible = false;
//							n0->face124_visible = false;
							}
						else n0->impossible = true;
						}
					}
				}
			else if ( (n->type == t->type) && (nv4 == tv4) ) {		// T1-T1 or T2-T2
				a = (nv1 == tv1);
				b = (nv1 == tv2);
				c = (nv1 == tv3);
				d = (nv2 == tv1);
				e = (nv2 == tv2);
				f = (nv2 == tv3);
				g = (nv3 == tv1);
				h = (nv3 == tv2);
				i = (nv3 == tv3);
				if ( (a && e) || (b && d) ) {
					dot = 0.0;
					for (j=0; j<3; j++) dot = dot + n->n124[j]*t->n124[j];
					if ( dot < 0.0 ) {
						n->neighbor_list = add( n->neighbor_list, t, FACE124 );
						t->neighbor_list = add( t->neighbor_list, n, FACE124 );
//						n->face124_visible = false;
//						t->face124_visible = false;
						}
					}
				else if ( (b && f) || (c && e) ) {
					dot = 0.0;
					for (j=0; j<3; j++) dot = dot + n->n124[j]*t->n234[j];
					if ( dot < 0.0 ) {
						n->neighbor_list = add( n->neighbor_list, t, FACE124 );
						t->neighbor_list = add( t->neighbor_list, n, FACE234 );
//						n->face124_visible = false;
//						t->face234_visible = false;
						}
					}
				else if ( (a && f) || (c && d) ) {
					dot = 0.0;
					for (j=0; j<3; j++) dot = dot + n->n124[j]*t->n143[j];
					if ( dot < 0.0 ) {
						n->neighbor_list = add( n->neighbor_list, t, FACE124 );
						t->neighbor_list = add( t->neighbor_list, n, FACE143 );
//						n->face124_visible = false;
//						t->face143_visible = false;
						}
					}
				else if ( (d && h) || (e && g) ) {
					dot = 0.0;
					for (j=0; j<3; j++) dot = dot + n->n234[j]*t->n124[j];
					if ( dot < 0.0 ) {
						n->neighbor_list = add( n->neighbor_list, t, FACE234 );
						t->neighbor_list = add( t->neighbor_list, n, FACE124 );
//						n->face234_visible = false;
//						t->face124_visible = false;
						}
					}
				else if ( (e && i) || (f && h) ) {
					dot = 0.0;
					for (j=0; j<3; j++) dot = dot + n->n234[j]*t->n234[j];
					if ( dot < 0.0 ) {
						n->neighbor_list = add( n->neighbor_list, t, FACE234 );
						t->neighbor_list = add( t->neighbor_list, n, FACE234 );
//						n->face234_visible = false;
//						t->face234_visible = false;
						}
					}
				else if ( (d && i) || (f && g) ) {
					dot = 0.0;
					for (j=0; j<3; j++) dot = dot + n->n234[j]*t->n143[j];
					if ( dot < 0.0 ) {
						n->neighbor_list = add( n->neighbor_list, t, FACE234 );
						t->neighbor_list = add( t->neighbor_list, n, FACE143 );
//						n->face234_visible = false;
//						t->face143_visible = false;
						}
					}
				else if ( (a && h) || (b && g) ) {
					dot = 0.0;
					for (j=0; j<3; j++) dot = dot + n->n143[j]*t->n124[j];
					if ( dot < 0.0 ) {
						n->neighbor_list = add( n->neighbor_list, t, FACE143 );
						t->neighbor_list = add( t->neighbor_list, n, FACE124 );
//						n->face143_visible = false;
//						t->face124_visible = false;
						}
					}
				else if ( (b && i) || (c && h) ) {
					dot = 0.0;
					for (j=0; j<3; j++) dot = dot + n->n143[j]*t->n234[j];
					if ( dot < 0.0 ) {
						n->neighbor_list = add( n->neighbor_list, t, FACE143 );
						t->neighbor_list = add( t->neighbor_list, n, FACE234 );
//						n->face143_visible = false;
//						t->face234_visible = false;
						}
					}
				else if ( (a && i) || (c && g) ) {
					dot = 0.0;
					for (j=0; j<3; j++) dot = dot + n->n143[j]*t->n143[j];
					if ( dot < 0.0 ) {
						n->neighbor_list = add( n->neighbor_list, t, FACE143 );
						t->neighbor_list = add( t->neighbor_list, n, FACE143 );
//						n->face143_visible = false;
//						t->face143_visible = false;
						}
					}
				}
			} // end if (!n->visited)...

		  nbr = nbr->next;	// try next neighbor associated with this vertex
		  }
		 }					// end for (k=0...)
      remove_neighbors( beenthere );
		t = t->next;
		}

													// eliminate T12 nodes with only one neighbor
	changing = true;
	while ( changing ) {                // go thru all tetra until no more chnages
		changing = false;

		t = sect->head;
		while ( t != NULL ) {
			if ( (t->type==0) && !t->impossible ) {
				k = 0;
				nbr = t->neighbor_list;
				while ( nbr != 0 ) {
					if ( !nbr->neighbor->impossible ) k++;
					nbr = nbr->next;
					}
				if ( k < 2 ) { t->impossible = true; changing = true; }
				}
			t = t->next;
			}
		}			// end while (changing...


													// now find the components in solid paths
	changing = true;
	while ( changing ) {                // go thru all tetra until no more chnages
		changing = false;

		t = sect->head;
		while ( t != NULL ) {
			if ( !t->hasT1 && !t->impossible ) {			// propagate T1 path
				nbr = t->neighbor_list;
				found = false;
				while ( !found && (nbr != NULL) ) {
					if ( nbr->neighbor->hasT1 )
						if ( t->type == 0 ) {					// when this is T12...
							if ( nbr->neighbor->type == 2 ) ;// ...from a T1 or T12 (but not from a T2)
							else	found = true;
							}
						else found = true;						// from any neighbor if this not T12
					nbr = nbr->next;
					}
				if ( found ) { t->hasT1 = true; changing = true; }
				}
			if ( !t->hasT2 && !t->impossible ) {			// propagate T2 path
				nbr = t->neighbor_list;
				found = false;
				while ( !found && (nbr != NULL) ) {
					if ( nbr->neighbor->hasT2 )
						if ( t->type == 0 ) {					// when this is T12...
							if ( nbr->neighbor->type == 1 ) ;// ...from a T2 or T12 (but not from a T1)
							else	found = true;
							}
						else found = true;						// from any neighbor if this not T12
					nbr = nbr->next;
					}
				if ( found ) { t->hasT2 = true; changing = true; }
				}
			t = t->next;
			}
		}  // continue while changing...

													// for remaining solid, hide neighboring faces
	t = sect->head;                     // NEED TO CHECK FOR NON_MATCHING, BUT INTERSECTING FACES
	while ( t != NULL ) {
		if ( t->hasT1 && t->hasT2 ) {		// this node belongs to solid reconstruction
			nbr = t->neighbor_list;
			while ( nbr != NULL ) {
				n = nbr->neighbor;
				if ( n->hasT1 && n->hasT2 )	// as does its neighbor
					switch (nbr->face) {
						case FACE132: t->face132_visible = false;   // hide adjacent face
										break;
						case FACE143: t->face143_visible = false;   // hide adjacent face
										break;
						case FACE124: t->face124_visible = false;   // hide adjacent face
										break;
						case FACE234: t->face234_visible = false;   // hide adjacent face
						}
				nbr = nbr->next;
				}
			}
		else {
			t->face132_visible = false;   // otherwise eliminate all faces of this node
			t->face143_visible = false;
			t->face124_visible = false;
			t->face234_visible = false;
			}

		t = t->next;
		}

													// now check for adjacencies between sections...
	t = sect->head;
	while ( t != NULL ) {
		tv1 = t->v1->index;					// test vertices by comparing indices
		tv2 = t->v2->index;
		tv3 = t->v3->index;
		if ( (t->type == 1) && ( sect->prev != NULL) 	// find adjacencies between of T1
				&& ( t->hasT1 && t->hasT2 ) ) {
			n = sect->prev->head;								// ...with previous section T2 faces
			while ( n != NULL ) {
				if ( (n->type == 2) && n->hasT1 && n->hasT2 ) {
            	nv1 = n->v1->index;
					nv2 = n->v2->index;
					nv3 = n->v3->index;
					if ( ((tv1 == nv1) && (tv2 == nv2) && (tv3 == nv3))
							|| ((tv1 == nv3) && (tv2 == nv1) && (tv3 == nv2))
							|| ((tv1 == nv2) && (tv2 == nv3) && (tv3 == nv1)) ) {
						t->face132_visible = false;
						n->face132_visible = false;		// hide faces
						}
					}
				n = n->next;
				}
			}
		t = t->next;
      }
}


																// compute the outward normal at each vertex
void reconstruction::compute_vertex_normals()	// ASSUMES: only exterior faces are visible!
{
	vertex *v;
	section_recon *sect;
	tetrahedron *t;
	double m, a, b, c, a2, b2, c2, x1, x2, x3, x4;
	int j;

	sect = head;					// do every tetrahedron in reconstruction
	while ( sect != NULL ) {
		t = sect->head;
		while ( t != NULL ) {			// process each face of this tetra
			if ( t->face132_visible ) {// if visible, compute face angles
				a2 = (t->v1->v[0]-t->v2->v[0])*(t->v1->v[0]-t->v2->v[0])
						+ (t->v1->v[1]-t->v2->v[1])*(t->v1->v[1]-t->v2->v[1])
						+ (t->v1->v[2]-t->v2->v[2])*(t->v1->v[2]-t->v2->v[2]);
				b2 = (t->v2->v[0]-t->v3->v[0])*(t->v2->v[0]-t->v3->v[0])
						+ (t->v2->v[1]-t->v3->v[1])*(t->v2->v[1]-t->v3->v[1])
						+ (t->v2->v[2]-t->v3->v[2])*(t->v2->v[2]-t->v3->v[2]);
				c2 = (t->v3->v[0]-t->v1->v[0])*(t->v3->v[0]-t->v1->v[0])
						+ (t->v3->v[1]-t->v1->v[1])*(t->v3->v[1]-t->v1->v[1])
						+ (t->v3->v[2]-t->v1->v[2])*(t->v3->v[2]-t->v1->v[2]);
				a = sqrt(a2);
				b = sqrt(b2);
				c = sqrt(c2);
				x1 = acos( (c2 + a2 - b2)/(2.0*a*c) );
				x2 = acos( (a2 + b2 - c2)/(2.0*a*b) );
				x3 = PI - x1 - x2;
				for (j=0; j<3; j++) {	// and weighted sum to each vertex normal
					t->v1->n[j] += x1*t->n132[j];
					t->v2->n[j] += x2*t->n132[j];
					t->v3->n[j] += x3*t->n132[j];
					}
				}
			if ( t->face143_visible ) {
				a2 = (t->v1->v[0]-t->v4->v[0])*(t->v1->v[0]-t->v4->v[0])
						+ (t->v1->v[1]-t->v4->v[1])*(t->v1->v[1]-t->v4->v[1])
						+ (t->v1->v[2]-t->v4->v[2])*(t->v1->v[2]-t->v4->v[2]);
				b2 = (t->v4->v[0]-t->v3->v[0])*(t->v4->v[0]-t->v3->v[0])
						+ (t->v4->v[1]-t->v3->v[1])*(t->v4->v[1]-t->v3->v[1])
						+ (t->v4->v[2]-t->v3->v[2])*(t->v4->v[2]-t->v3->v[2]);
				c2 = (t->v3->v[0]-t->v1->v[0])*(t->v3->v[0]-t->v1->v[0])
						+ (t->v3->v[1]-t->v1->v[1])*(t->v3->v[1]-t->v1->v[1])
						+ (t->v3->v[2]-t->v1->v[2])*(t->v3->v[2]-t->v1->v[2]);
				a = sqrt(a2);
				b = sqrt(b2);
				c = sqrt(c2);
				x1 = acos( (c2 + a2 - b2)/(2.0*a*c) );
				x4 = acos( (a2 + b2 - c2)/(2.0*a*b) );
				x3 = PI - x1 - x4;
				for (j=0; j<3; j++) {
					t->v1->n[j] += x1*t->n143[j];
					t->v3->n[j] += x3*t->n143[j];
					t->v4->n[j] += x4*t->n143[j];
					}
				}
			if ( t->face124_visible ) {
				a2 = (t->v1->v[0]-t->v2->v[0])*(t->v1->v[0]-t->v2->v[0])
						+ (t->v1->v[1]-t->v2->v[1])*(t->v1->v[1]-t->v2->v[1])
						+ (t->v1->v[2]-t->v2->v[2])*(t->v1->v[2]-t->v2->v[2]);
				b2 = (t->v2->v[0]-t->v4->v[0])*(t->v2->v[0]-t->v4->v[0])
						+ (t->v2->v[1]-t->v4->v[1])*(t->v2->v[1]-t->v4->v[1])
						+ (t->v2->v[2]-t->v4->v[2])*(t->v2->v[2]-t->v4->v[2]);
				c2 = (t->v4->v[0]-t->v1->v[0])*(t->v4->v[0]-t->v1->v[0])
						+ (t->v4->v[1]-t->v1->v[1])*(t->v4->v[1]-t->v1->v[1])
						+ (t->v4->v[2]-t->v1->v[2])*(t->v4->v[2]-t->v1->v[2]);
				a = sqrt(a2);
				b = sqrt(b2);
				c = sqrt(c2);
				x1 = acos( (c2 + a2 - b2)/(2.0*a*c) );
				x2 = acos( (a2 + b2 - c2)/(2.0*a*b) );
				x4 = PI - x1 - x2;
				for (j=0; j<3; j++) {
					t->v1->n[j] += x1*t->n124[j];
					t->v2->n[j] += x2*t->n124[j];
					t->v4->n[j] += x4*t->n124[j];
					}
				}
			if ( t->face234_visible ) {
				a2 = (t->v2->v[0]-t->v3->v[0])*(t->v2->v[0]-t->v3->v[0])
						+ (t->v2->v[1]-t->v3->v[1])*(t->v2->v[1]-t->v3->v[1])
						+ (t->v2->v[2]-t->v3->v[2])*(t->v2->v[2]-t->v3->v[2]);
				b2 = (t->v3->v[0]-t->v4->v[0])*(t->v3->v[0]-t->v4->v[0])
						+ (t->v3->v[1]-t->v4->v[1])*(t->v3->v[1]-t->v4->v[1])
						+ (t->v3->v[2]-t->v4->v[2])*(t->v3->v[2]-t->v4->v[2]);
				c2 = (t->v4->v[0]-t->v2->v[0])*(t->v4->v[0]-t->v2->v[0])
						+ (t->v4->v[1]-t->v2->v[1])*(t->v4->v[1]-t->v2->v[1])
						+ (t->v4->v[2]-t->v2->v[2])*(t->v4->v[2]-t->v2->v[2]);
				a = sqrt(a2);
				b = sqrt(b2);
				c = sqrt(c2);
				x2 = acos( (c2 + a2 - b2)/(2.0*a*c) );
				x3 = acos( (a2 + b2 - c2)/(2.0*a*b) );
				x4 = PI - x2 - x3;
				for (j=0; j<3; j++) {
					t->v2->n[j] += x2*t->n234[j];
					t->v3->n[j] += x3*t->n234[j];
					t->v4->n[j] += x4*t->n234[j];
					}
				}
			t = t->next;
			}
		sect = sect->next;
		}


	v = vertices->head;					// do each vertex in reconstruction
	while ( v != NULL ) {
												// normalize vector
		m = sqrt(v->n[0]*v->n[0] + v->n[1]*v->n[1] + v->n[2]*v->n[2]);
		if ( m > 0.0005 ) for (j=0; j<3; j++) v->n[j] = v->n[j]/m;
		v = v->next;
		}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// New part for connecting to the VRMLObject data type in reconstruct. JCF 12/16/03
//

void BoissonnatSurface( VRMLObject *vrmlobject )			// surface VRML Object from contours
{
	Section *section;
	Transform *transform;
	Contour *c, *contour, *xyzcontour;
	Contours *sectcontours;
	Point *cp, Zoffset;
	vertex *p;
	bool firstpiece, hideLower, hideUpper;
	tetrahedron *tet;
	section_recon *sect;
	reconstruction *object;
	int sectnum, offset1;
	double x, y, z, max_link_distance;
	unsigned int i, j, f, l, max_points, max_faces;

	object = new reconstruction();					// create IGL Trace reconstruction object
	firstpiece = true;
	hideLower = !CurrSeries->lower3Dfaces;
	hideUpper = false;
	max_link_distance = CurrSeries->max3Dconnection;
	if ( max_link_distance <= 0.0 ) max_link_distance = MAX_FLOAT;

	max_points = 0;
	sectnum = vrmlobject->firstSection-1;
	section = GetNextSectionBetween( sectnum, vrmlobject->lastSection );
	while ( section && !Abort3D )							// get first section of object
	  {
	  if ( section->transforms )
		{
		sectcontours = new Contours();						// form single list of transform'd contours
		RAY_XSMALL = MAX_FLOAT;  RAY_YSMALL = MAX_FLOAT; 
		RAY_XLARGE = -MAX_FLOAT; RAY_YLARGE = -MAX_FLOAT;				// find extremes on section
		transform = section->transforms->first;
		while ( transform )									// check each transform for contours
		  {
		  if ( transform->contours )
			{												// make local copy of each contour...
			contour = transform->contours->first;
			while ( contour )
				{
				if ( (strcmp(vrmlobject->name, contour->name) == 0)	// ...that matched the name...
						&& contour->points )						// ...but only if have points
				  if ( contour->points->Number() > 2 )
					{
					xyzcontour = new Contour( *contour );		// copy the contour
					xyzcontour->InvNform( transform->nform );	// transform into section
					if ( ApplyZOffset3D )
						{										// shift within section
						Zoffset = CurrSeries->OffsetBetweenZTraces( sectnum, OffsetZTrace1, OffsetZTrace2 );
						xyzcontour->ShiftXYZ( Zoffset.x, Zoffset.y, Zoffset.z );
						}
					cp = xyzcontour->points->first;
					while ( cp )					// shift contour to 3D position and find extremes
						{
						x = cp->x + CurrSeries->x3Doffset;
						y = cp->y + CurrSeries->y3Doffset;
						if (x < RAY_XSMALL) RAY_XSMALL = x;
						if (x > RAY_XLARGE) RAY_XLARGE = x;
						if (y < RAY_YSMALL) RAY_YSMALL = y;
						if (y > RAY_YLARGE) RAY_YLARGE = y;
						cp->x = x;
						cp->y = y;
						cp = cp->next;
						}		
					vrmlobject->diffuseColor = xyzcontour->border;			// remember color from contour				
					sectcontours->Add( xyzcontour );			// add contour to local list
					}
				contour = contour->next;					// do next contour in section
				}
			}
		  transform = transform->next;
		  }
													// now create surface reconstruction for section
		if ( sectcontours->Number() && !Abort3D )
			{
			z = CurrSectionsInfo->ZDistance(section->index,CurrSeries->zMidSection)+CurrSeries->z3Doffset;	// calculate z value
			RAY_XSMALL -= 1.0; RAY_YSMALL -= 1.0;
			RAY_XLARGE += 1.0; RAY_YLARGE += 1.0;		// expand bounding box to hold all points inside
			if ( firstpiece )
				{
				object->first( sectcontours, section->index, z);		// create first section data
				firstpiece = false;
				}
			else											// use next section data to create surface
				{
				if ( sectnum == vrmlobject->lastSection ) hideUpper = !CurrSeries->upper3Dfaces;
				object->next( sectcontours, section->index, z, max_link_distance, hideLower, hideUpper); 
				hideLower = false;							// only hideLower faces on first section
				}
			}

		delete sectcontours;						// done with local contours list
		}

	  PutSection( section, false, false, false );	// free section memory and do next section

	  section = GetNextSectionBetween( sectnum, vrmlobject->lastSection );
	  }				
		
													// now put surfaces into VRMLobject structures

	vrmlobject->min.x = MAX_FLOAT;  vrmlobject->min.y = MAX_FLOAT;  vrmlobject->min.z = MAX_FLOAT;
	vrmlobject->max.x = -MAX_FLOAT; vrmlobject->max.y = -MAX_FLOAT; vrmlobject->max.z = -MAX_FLOAT;
	
	max_points = 0;
	p = object->vertices->head;						// count size of indexed point set
	while ( p != NULL )
		{
		max_points++;
		if (p->v[0] < vrmlobject->min.x) vrmlobject->min.x = p->v[0];
		if (p->v[0] > vrmlobject->max.x) vrmlobject->max.x = p->v[0];	// if outside of bounds, set new bounds
		if (p->v[1] < vrmlobject->min.y) vrmlobject->min.y = p->v[1];
		if (p->v[1] > vrmlobject->max.y) vrmlobject->max.y = p->v[1];
		if (p->v[2] < vrmlobject->min.z) vrmlobject->min.z = p->v[2];
		if (p->v[2] > vrmlobject->max.z) vrmlobject->max.z = p->v[2];
		p = p->next;
		}

	i = 0;
	vrmlobject->vertices = new VertexSet( max_points );			// now create indexed array representation
	p = object->vertices->head;						// output indexed point set
	if ( p != NULL ) {
		offset1 = p->index;							// VRML indices start at zero!
		while ( p != NULL )
			{
			vrmlobject->vertices->vertex[i].x = p->v[0];
			vrmlobject->vertices->vertex[i].y = p->v[1];
			vrmlobject->vertices->vertex[i].z = p->v[2];
			i++;
			p = p->next;
			}
		}

	max_faces = 0;
	sect = object->head;								// for entire object
	while ( sect != NULL )
		{												// count all the visible faces
		tet = sect->head;
		while ( tet != NULL )
		  {
		  if ( tet->up )
			{
			if ( tet->face132_visible ) max_faces++;
			if ( tet->face143_visible ) max_faces++;
			if ( tet->face124_visible ) max_faces++;
			if ( tet->face234_visible ) max_faces++;
			}
		  else {
			if ( tet->face132_visible ) max_faces++;
			if ( tet->face143_visible ) max_faces++;
			if ( tet->face124_visible ) max_faces++;
			if ( tet->face234_visible ) max_faces++;
			}
		  tet = tet->next;
		  }
		sect = sect->next;							// do next section
		}
	
	vrmlobject->faces = new FaceSet( max_faces );			// now create indexed array representation
	i = 0;
	sect = object->head;									// and store faces of object
	while ( sect != NULL )
		{
		tet = sect->head;
		while ( tet != NULL )
		  {
		  if ( tet->up ) {
			 if ( tet->face132_visible )
				{
				vrmlobject->faces->face[i].v1 = tet->v1->index-offset1;
				vrmlobject->faces->face[i].v2 = tet->v3->index-offset1;
				vrmlobject->faces->face[i].v3 = tet->v2->index-offset1;
				i++;
				}
			 if ( tet->face143_visible )
				{
				vrmlobject->faces->face[i].v1 = tet->v1->index-offset1;
				vrmlobject->faces->face[i].v2 = tet->v4->index-offset1;
				vrmlobject->faces->face[i].v3 = tet->v3->index-offset1;
				i++;
				}
			 if ( tet->face124_visible )
				{
				vrmlobject->faces->face[i].v1 = tet->v1->index-offset1;
				vrmlobject->faces->face[i].v2 = tet->v2->index-offset1;
				vrmlobject->faces->face[i].v3 = tet->v4->index-offset1;
				i++;
				}
			 if ( tet->face234_visible )
				{
				vrmlobject->faces->face[i].v1 = tet->v2->index-offset1;
				vrmlobject->faces->face[i].v2 = tet->v3->index-offset1;
				vrmlobject->faces->face[i].v3 = tet->v4->index-offset1;
				i++;
				}
			 }
		  else {
			 if ( tet->face132_visible )
				{
				vrmlobject->faces->face[i].v1 = tet->v1->index-offset1;
				vrmlobject->faces->face[i].v2 = tet->v2->index-offset1;
				vrmlobject->faces->face[i].v3 = tet->v3->index-offset1;
				i++;
				}
			 if ( tet->face143_visible )
				{
				vrmlobject->faces->face[i].v1 = tet->v1->index-offset1;
				vrmlobject->faces->face[i].v2 = tet->v3->index-offset1;
				vrmlobject->faces->face[i].v3 = tet->v4->index-offset1;
				i++;
				}
			 if ( tet->face124_visible )
				{
				vrmlobject->faces->face[i].v1 = tet->v1->index-offset1;
				vrmlobject->faces->face[i].v2 = tet->v4->index-offset1;
				vrmlobject->faces->face[i].v3 = tet->v2->index-offset1;
				i++;
				}
			 if ( tet->face234_visible )
				{
				vrmlobject->faces->face[i].v1 = tet->v2->index-offset1;
				vrmlobject->faces->face[i].v2 = tet->v4->index-offset1;
				vrmlobject->faces->face[i].v3 = tet->v3->index-offset1;
				i++;
				}
			 }
		  tet = tet->next;
		  }
		sect = sect->next;							// do next section
		}

	if ( CurrSeries->faceNormals )
		{
		vrmlobject->normalPerVertex = false;
		vrmlobject->normals = new VertexSet( max_faces );			// now create indexed array representation
		i = 0;
		sect = object->head;										// start with first section of object
		while ( sect != NULL )
			{
			tet = sect->head;										//  ...give normals for every tetrahedron face
			while ( tet != NULL )
				{
				if ( tet->face132_visible )
					{
					vrmlobject->normals->vertex[i].x = tet->n132[0];
					vrmlobject->normals->vertex[i].y = tet->n132[1];
					vrmlobject->normals->vertex[i].z = tet->n132[2];
					i++;
					}
				if ( tet->face143_visible )
					{
					vrmlobject->normals->vertex[i].x = tet->n143[0];
					vrmlobject->normals->vertex[i].y = tet->n143[1];
					vrmlobject->normals->vertex[i].z = tet->n143[2];
					i++;
					}
				if ( tet->face124_visible )
					{
					vrmlobject->normals->vertex[i].x = tet->n124[0];
					vrmlobject->normals->vertex[i].y = tet->n124[1];
					vrmlobject->normals->vertex[i].z = tet->n124[2];
					i++;
					}
				if ( tet->face234_visible )
					{
					vrmlobject->normals->vertex[i].x = tet->n234[0];
					vrmlobject->normals->vertex[i].y = tet->n234[1];
					vrmlobject->normals->vertex[i].z = tet->n234[2];
					i++;
					}
				tet = tet->next;
				}
			sect = sect->next;							// do next section
			}
		}		// end if

	if ( CurrSeries->vertexNormals )	// note only one normals fklag should be set
		{
		vrmlobject->normalPerVertex = true;
		object->compute_vertex_normals();						// compute vertex normals
		i = 0;
		vrmlobject->normals = new VertexSet( max_points );		// create indexed array representation
		p = object->vertices->head;
		while ( p != NULL )
			{
			vrmlobject->normals->vertex[i].x = p->n[0];
			vrmlobject->normals->vertex[i].y = p->n[1];
			vrmlobject->normals->vertex[i].z = p->n[2];
			i++;
			p = p->next;
			}
		}		// end if


	delete object;
}


void SlabSurface( VRMLObject *vrmlobject )		// use Delaunay triangulation of planar sections to create a slab
{
	Section *section;
	Transform *transform;
	Contour *c, *contour, *xyzcontour;
	Contours *sectcontours;
	Point *cp, Zoffset;
	index_point *ip;
	vertex *p;
	delaunay_triangulation *dt;
	triangle *t;
	int sectnum, offset1, offset2, l1, l2, l3, u1, u2, u3, count;
	double x, y, z, slab_top, slab_bottom, thickness;
	unsigned int i, j, k, max_points, max_faces;

	vrmlobject->min.x = MAX_FLOAT;  vrmlobject->min.y = MAX_FLOAT;  vrmlobject->min.z = MAX_FLOAT;
	vrmlobject->max.x = -MAX_FLOAT; vrmlobject->max.y = -MAX_FLOAT; vrmlobject->max.z = -MAX_FLOAT;
	max_points = 0;
	max_faces = 0;

	sectnum = vrmlobject->firstSection-1;
	section = GetNextSectionBetween( sectnum, vrmlobject->lastSection );
	while ( section && !Abort3D )							// get first section of object
	  {
	  if ( section->transforms )
		{
		sectcontours = new Contours();						// form single list of transform'd contours
		RAY_XSMALL = MAX_FLOAT;  RAY_YSMALL = MAX_FLOAT; 
		RAY_XLARGE = -MAX_FLOAT; RAY_YLARGE = -MAX_FLOAT;	// find extremes on section
		transform = section->transforms->first;
		while ( transform )									// check each transform for contours
		  {
		  if ( transform->contours )
			{												// make local copy of each contour...
			contour = transform->contours->first;
			while ( contour )								// ...that matched the name...
				{
				if ( strcmp(vrmlobject->name, contour->name) == 0 )
				  if ( contour->points )					// ...but only if have points
					{
					xyzcontour = new Contour( *contour );	// copy the contour
					xyzcontour->InvNform(transform->nform);	// transform into section
					if ( ApplyZOffset3D )
						{									// shift within section
						Zoffset = CurrSeries->OffsetBetweenZTraces( sectnum, OffsetZTrace1, OffsetZTrace2 );
						xyzcontour->ShiftXYZ( Zoffset.x, Zoffset.y, Zoffset.z );
						}
					cp = xyzcontour->points->first;
					while ( cp )							// shift contour to 3D position and find extremes
						{
						x = cp->x + CurrSeries->x3Doffset;
						y = cp->y + CurrSeries->y3Doffset;
						if (x < RAY_XSMALL) RAY_XSMALL = x;
						if (x > RAY_XLARGE) RAY_XLARGE = x;
						if (y < RAY_YSMALL) RAY_YSMALL = y;
						if (y > RAY_YLARGE) RAY_YLARGE = y;
						cp->x = x;
						cp->y = y;
						cp = cp->next;
						}
					vrmlobject->diffuseColor = xyzcontour->border;			// remember color from contour
					vrmlobject->emissiveColor = Color(0.2*xyzcontour->border.r,0.2*xyzcontour->border.g,0.2*xyzcontour->border.b);
					sectcontours->Add( xyzcontour );		// add contour to local list
					}
				contour = contour->next;					// do next contour in section
				}
			}
		  transform = transform->next;
		  }
													// now create surface reconstruction for section
		if ( sectcontours->Number() && !Abort3D )
			{
			RAY_XSMALL -= 1.0; RAY_YSMALL -= 1.0;
			RAY_XLARGE += 1.0; RAY_YLARGE += 1.0;	// expand bounding box to hold all points inside
			dt = new delaunay_triangulation();
													// add each matching contour in list...
			c = sectcontours->first;
			while ( c )								// add contour pts to triangulation
				{
				if ( c->points->Number() > 2 ) dt->triangulate( c );
				c = c->next;
				}
//			dt->add_contour_intersections(); // THIS NEVER RETURNS FOR CONTOURS WITH LOOPS!
			dt->contour_complete();					// make sure triangulation covers contour
			dt->remove_bounding_box();				// remove enclosing complex
			dt->mark_external();					// label external triangles

			j = max_points;
			count = 0;
			ip = dt->pts->head;						// count size of indexed point set
			while ( ip != NULL )
				{
				count++;
				ip = ip->next;
				}									// now put vertices in indexed array representation
			max_points = max_points + 2*count;
			if ( vrmlobject->vertices ) vrmlobject->vertices->ExpandTo( max_points );
			else vrmlobject->vertices = new VertexSet( max_points );
			i = 0;
													// calculate z value
			z = CurrSectionsInfo->ZDistance(section->index,CurrSeries->zMidSection)+CurrSeries->z3Doffset;
			if ( CurrSeries->dim3Da < 0.0 )
				thickness = section->thickness;		// use the section thickness
			else thickness = CurrSeries->dim3Da;	// or (if present) user's desired value
			if ( CurrSeries->zMidSection )
				{
				slab_top = z + thickness/2;			// slab extends +/- 1/2 thickness
				slab_bottom = z - thickness/2;
				}
			else
				{
				slab_top = z;						// zdistance is top of slab
				slab_bottom = z - thickness;
				}

			ip = dt->pts->head;						// output indexed point set
			if ( ip != NULL )
				{
				offset1 = ip->index;				// indices of triangle vertices won't necessarily start at zero
				while ( ip != NULL )
					{
					x = ip->x;
					y = ip->y;
					vrmlobject->vertices->vertex[i+j].x = x;
					vrmlobject->vertices->vertex[i+j].y = y;
					vrmlobject->vertices->vertex[i+j].z = slab_bottom;
					if (x < vrmlobject->min.x) vrmlobject->min.x = x;
					if (x > vrmlobject->max.x) vrmlobject->max.x = x;	// if outside of bounds, set new bounds
					if (y < vrmlobject->min.y) vrmlobject->min.y = y;
					if (y > vrmlobject->max.y) vrmlobject->max.y = y;
					if (z < vrmlobject->min.z) vrmlobject->min.z = z;
					if (z > vrmlobject->max.z) vrmlobject->max.z = z;
					i++;
					ip = ip->next;
					}
				ip = dt->pts->head;					// go through list again generating slab offset vertices
				offset2 = i;						// second offset marks beginning of upper slab vertices
				while ( ip != NULL )
					{
					x = ip->x;
					y = ip->y;
					vrmlobject->vertices->vertex[i+j].x = x;
					vrmlobject->vertices->vertex[i+j].y = y;
					vrmlobject->vertices->vertex[i+j].z = slab_top;
					if (z < vrmlobject->min.z) vrmlobject->min.z = z;
					if (z > vrmlobject->max.z) vrmlobject->max.z = z;
					i++;
					ip = ip->next;
					}
				}
			k = max_faces;
			count = 0;
			t = dt->head;							// count number of triangle faces to add
			while ( t != NULL )
				{										// each internal has a top and bottom
				if ( t->oriented != ORIENTED_EXTERNAL )
					{
					count = count + 2;					// each boundary edge generates 2 faces
					if ( t->edge12 & ORIENTED_BOUNDARY ) count = count + 2;
					if ( t->edge23 & ORIENTED_BOUNDARY ) count = count + 2;
					if ( t->edge31 & ORIENTED_BOUNDARY ) count = count + 2;
					}
				t = t->next;
				}									// add slab faces to face set
			max_faces = max_faces + count;
			if ( vrmlobject->faces ) vrmlobject->faces->ExpandTo( max_faces );
			else vrmlobject->faces = new FaceSet( max_faces );
			i = k;
			t = dt->head;							// output indexed point set
			while ( t != NULL )
				{
				if ( t->oriented != ORIENTED_EXTERNAL )
					{
					l1 = t->v1->index - offset1 + j;					// lower slab face vertices
					l2 = t->v2->index - offset1 + j;
					l3 = t->v3->index - offset1 + j;
					u1 = l1 + offset2;									// upper slab face vertices
					u2 = l2 + offset2;
					u3 = l3 + offset2;
					vrmlobject->faces->face[i].v1 = l1;					// add lower slab face
					vrmlobject->faces->face[i].v2 = l2;
					vrmlobject->faces->face[i].v3 = l3;
					i++;
					vrmlobject->faces->face[i].v1 = u1;					// add upper slab face
					vrmlobject->faces->face[i].v2 = u2;
					vrmlobject->faces->face[i].v3 = u3;
					i++;
					if ( t->edge12 & ORIENTED_BOUNDARY )
						{
						vrmlobject->faces->face[i].v1 = l1;				// add side slab face
						vrmlobject->faces->face[i].v2 = l2;
						vrmlobject->faces->face[i].v3 = u1;
						i++;
						vrmlobject->faces->face[i].v1 = l2;				// add side slab face
						vrmlobject->faces->face[i].v2 = u2;
						vrmlobject->faces->face[i].v3 = u1;
						i++;
						}
					if ( t->edge23 & ORIENTED_BOUNDARY )
						{
						vrmlobject->faces->face[i].v1 = l2;				// add side slab face
						vrmlobject->faces->face[i].v2 = l3;
						vrmlobject->faces->face[i].v3 = u2;
						i++;
						vrmlobject->faces->face[i].v1 = l3;				// add side slab face
						vrmlobject->faces->face[i].v2 = u3;
						vrmlobject->faces->face[i].v3 = u2;
						i++;
						}
					if ( t->edge31 & ORIENTED_BOUNDARY )
						{
						vrmlobject->faces->face[i].v1 = l3;				// add side slab face
						vrmlobject->faces->face[i].v2 = l1;
						vrmlobject->faces->face[i].v3 = u3;
						i++;
						vrmlobject->faces->face[i].v1 = l1;				// add side slab face
						vrmlobject->faces->face[i].v2 = u1;
						vrmlobject->faces->face[i].v3 = u3;
						i++;
						}
					}
				t = t->next;
				}

			delete dt;
			}

		delete sectcontours;						// done with local contours list
		}

	  PutSection( section, false, false, false );	// free section memory and do next section

	  section = GetNextSectionBetween( sectnum, vrmlobject->lastSection );
	  }				

}

