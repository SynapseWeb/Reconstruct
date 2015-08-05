////////////////////////////////////////////////////////////////////////
//	This file contains the methods for the VRMLObject class
//
//    Copyright (C) 2003-2006  John Fiala (fiala@bu.edu)
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
// modified 2/08/05 by JCF (fiala@bu.edu)
// -+- change: Added copy constructor to VRMLObject.
// modified 10/3/05 by JCF (fiala@bu.edu)
// -+- change: Added methods to Display() to display in OpenGL directly without call list compile.
// modified 4/14/06 by JCF (fiala@bu.edu)
// -+- change: Creation of 3D slabs output.
// modified 5/2/06 by JCF (fiala@bu.edu)
// -+- change: Added OffsetBetweenZTraces for smoothing operation.
//

#include "reconstruct.h"

VRMLObject::VRMLObject()
{
	prev = NULL;
	next = NULL;
	strcpy(name,"unknown");
	comment[0] = '\0';
	diffuseColor = Color( 0.7, 0.7, 0.7 );
	emissiveColor = Color( 0.0, 0.0, 0.0 );
	specularColor = Color( 0.0, 0.0, 0.0 );
	ambientIntensity = 0.2;
	shininess = 0.5;
	transparency = 0.0;
	contour_count = 0;
	surface_area = 0.0;
	flat_area = 0.0;
	volume = 0.0;
	contours = NULL;
	vertices = NULL;
	lines = NULL;
	faces = NULL;
	normals = NULL;
	normalPerVertex = false;
	frontFill = true;
	backFill = true;
	openGLList = 0;
	firstSection = lastSection = 0;
}

VRMLObject::VRMLObject( VRMLObject &copyfrom )				// copy constructor
{
	prev = NULL;											// don't copy pointers
	next = NULL;
	strcpy(name,copyfrom.name);
	strcpy(comment,copyfrom.comment);
	diffuseColor = copyfrom.diffuseColor;
	emissiveColor = copyfrom.emissiveColor;
	specularColor  = copyfrom.specularColor;
	ambientIntensity = copyfrom.ambientIntensity;
	shininess = copyfrom.shininess;
	transparency = copyfrom.transparency;
	min = copyfrom.min;
	max = copyfrom.max;
	firstSection = copyfrom.firstSection;
	lastSection = copyfrom.lastSection;
	normalPerVertex = copyfrom.normalPerVertex;
	frontFill = copyfrom.frontFill;
	backFill = copyfrom.backFill;
	contour_count = copyfrom.contour_count;
	surface_area = copyfrom.surface_area;
	flat_area = copyfrom.flat_area;
	volume = copyfrom.volume;
	openGLList = 0;
	contours = NULL;										// don't copy 3D data, recreate it if needed
	vertices = NULL;
	lines = NULL;
	faces = NULL;
	normals = NULL;
}
	
VRMLObject::~VRMLObject()
{
	if ( contours ) delete contours;
	if ( normals ) delete normals;
	if ( faces ) delete faces;
	if ( lines ) delete lines;
	if ( vertices ) delete vertices;
}

void VRMLObject::CreateZline( Contour *zcontour  )		// put z-contour into 3D Object
{
	Points *xyz;
	Point *p, *q;
	double x, y, z;
	int sectnum;
	unsigned int i, j, l, max_points;

	strcpy(this->name,zcontour->name);
	this->diffuseColor = zcontour->border;				// remember color from contour
	this->emissiveColor = zcontour->border;
	xyz = new Points();									// store final 3D points in xyz list
	
	min.x = MAX_FLOAT;  min.y = MAX_FLOAT;  min.z = MAX_FLOAT;
	max.x = -MAX_FLOAT; max.y = -MAX_FLOAT; max.z = -MAX_FLOAT;

	max_points = 0;
	p = zcontour->points->first;
	while ( p )					// find number of vertices and bounds of vertices
		{
		sectnum = (int)p->z;									// use only points in specified section range
		if ( (sectnum >= CurrSeries->first3Dsection) 
					&& (sectnum <= CurrSeries->last3Dsection) )
			{
			x = p->x + CurrSeries->x3Doffset;
			y = p->y + CurrSeries->y3Doffset;
			z = CurrSectionsInfo->ZDistance( sectnum, CurrSeries->zMidSection ) + CurrSeries->z3Doffset;
			q = new Point( x, y, z );
			xyz->Add( q );
			max_points++;
			if (x < min.x) min.x = x;
			if (x > max.x) max.x = x;
			if (y < min.y) min.y = y;
			if (y > max.y) max.y = y;
			if (z < min.z) min.z = z;
			if (z > max.z) max.z = z;
			}
		p = p->next;
		}		
		
		
	vertices = new VertexSet( max_points );			// now create indexed array representation
	lines = new LineSet( max_points );
	i = 0; j = 0; l = 0;
	p = xyz->first;
	while ( p )
		{
		vertices->vertex[i].x = p->x;
		vertices->vertex[i].y = p->y;
		vertices->vertex[i].z = p->z;
		if ( i )							// first pt in contour, no line segment yet
			{
			lines->line[l].v1 = j;			// add segment of contour
			lines->line[l].v2 = i;
			l++;
			}
		j = i;								// remember previous point of contour for next segment
		i++;								// go to next vertex
		p = p->next;
		}
		
	sprintf( comment, "Z-Trace  shifted by: %1.*g %1.*g %1.*g",
					Precision,CurrSeries->x3Doffset,Precision,CurrSeries->y3Doffset,Precision,CurrSeries->z3Doffset );
	delete xyz;
}


void VRMLObject::CreateContours( void )						// put contours into 3D Object
{
	Section *section;
	Transform *transform;
	Contour *contour, *xyzcontour;
	Point *p, Zoffset;
	int sectnum;
	unsigned int f, i, j, l, max_points;
	char filename[MAX_PATH];

	this->contours = new Contours();						// create local storage list

	sectnum = this->firstSection-1;
	section = GetNextSectionBetween( sectnum, this->lastSection );
	while ( section && !Abort3D )							// get first section of object
	  {
	  if ( section->transforms )
		{
		transform = section->transforms->first;
		while ( transform )									// check each transform for contours
		  {
		  if ( transform->contours )
			{												// make local copy of each contour...
			contour = transform->contours->first;
			while ( contour )
				{
				if ( strcmp(this->name, contour->name) == 0 )	// ...that matched the name...
				  if ( contour->points )						// ...but only if have points
					{
					xyzcontour = new Contour( *contour );		// copy the contour
					xyzcontour->InvNform( transform->nform );	// transform into section
					if ( ApplyZOffset3D )
						{										// shift within section
						Zoffset = CurrSeries->OffsetBetweenZTraces( sectnum, OffsetZTrace1, OffsetZTrace2 );
						xyzcontour->ShiftXYZ( Zoffset.x, Zoffset.y, Zoffset.z );
						}										// shift whole object
					xyzcontour->ShiftXYZ( CurrSeries->x3Doffset, CurrSeries->y3Doffset,
											CurrSectionsInfo->ZDistance(section->index,CurrSeries->zMidSection)+CurrSeries->z3Doffset );
					this->diffuseColor = xyzcontour->border;	// remember color from contour
					this->emissiveColor = xyzcontour->border;
					this->contours->Add( xyzcontour );			// copy contour into local list
					}
				contour = contour->next;					// do next contour in section
				}
			}
		  transform = transform->next;
		  }
		}
	  PutSection( section, false, false, false );			// free section memory
	  section = GetNextSectionBetween( sectnum, this->lastSection );
	  }														// and do next section

	min.x = MAX_FLOAT;  min.y = MAX_FLOAT;  min.z = MAX_FLOAT;
	max.x = -MAX_FLOAT; max.y = -MAX_FLOAT; max.z = -MAX_FLOAT;
	
	max_points = 0;
	contour = this->contours->first;
	while ( contour && !Abort3D )
		{
		p = contour->points->first;
		while ( p )					// find number of vertices and bounds of vertices
			{
			max_points++;
			if (p->x < min.x) min.x = p->x;
			if (p->x > max.x) max.x = p->x;
			if (p->y < min.y) min.y = p->y;
			if (p->y > max.y) max.y = p->y;
			if (p->z < min.z) min.z = p->z;
			if (p->z > max.z) max.z = p->z;
			p = p->next;
			}		
		contour = contour->next;					// do next contour
		}
		
	vertices = new VertexSet( max_points );			// now create indexed array representation
	lines = new LineSet( max_points );
	i = 0; l = 0;

	contour = contours->first;
	while ( contour && !Abort3D )					// fill arrays from contour list
		{
		if ( contour->points )
			{
			p = contour->points->first;
			j = 0; f = i;							// remember starting vertex of contour
			while ( p )
				{
				vertices->vertex[i].x = p->x;
				vertices->vertex[i].y = p->y;
				vertices->vertex[i].z = p->z;
				if ( f == i )  ;					// first pt in contour, no line segment yet
				else {
					lines->line[l].v1 = j;			// add segment of contour
					lines->line[l].v2 = i;
					l++;
					}
				j = i;								// remember previous point of contour for next segment
				i++;								// go to next vertex
				p = p->next;
				}
			if ( contour->closed )					// add closing segment if needed
				{
				lines->line[l].v1 = j;
				lines->line[l].v2 = f;
				l++;
				}
			}
		contour = contour->next;
		}
	
	sprintf( comment, "Traces  shifted by: %1.*g %1.*g %1.*g",
					Precision,CurrSeries->x3Doffset,Precision,CurrSeries->y3Doffset,Precision,CurrSeries->z3Doffset );

	delete contours;					// these are never subsequently used, so just delete
	contours = NULL;
}


void VRMLObject::CreateBoissonnat( void )			// surface the object using 3D Delaunay triangulation
{
	BoissonnatSurface( this );						// Boissonnat's algorithm for contour-perserving surface
	sprintf( comment, "Boissonnat Surface  shifted By: %1.*g %1.*g %1.*g",
					Precision,CurrSeries->x3Doffset,Precision,CurrSeries->y3Doffset,Precision,CurrSeries->z3Doffset );
}

void VRMLObject::CreateAreas( void )				// generate a surface by creating a slab for each trace
{
	Point d12, d13;
	double x1, x2, x3, y1, y2, y3, z1, z2, z3, n;
	unsigned int i;

	SlabSurface( this );							// uses planar delaunay triangulation of Boissonant algorithm

	if ( faces && !Abort3D )
		{
		if ( CurrSeries->faceNormals || CurrSeries->vertexNormals )
		  {
		  normalPerVertex = false;
		  normals = new VertexSet( faces->total );
		  for (i=0; i<faces->total; i++ )							// compute the normals from the faces
			{
			x1 = vertices->vertex[faces->face[i].v1].x; y1 = vertices->vertex[faces->face[i].v1].y; z1 = vertices->vertex[faces->face[i].v1].z;
			x2 = vertices->vertex[faces->face[i].v2].x; y2 = vertices->vertex[faces->face[i].v2].y; z2 = vertices->vertex[faces->face[i].v2].z;
			x3 = vertices->vertex[faces->face[i].v3].x; y3 = vertices->vertex[faces->face[i].v3].y; z3 = vertices->vertex[faces->face[i].v3].z;
			d12.x = x2-x1; d12.y = y2-y1; d12.z = z2-z1;
			d13.x = x3-x1; d13.y = y3-y1; d13.z = z3-z1;
			x1 = d12.y*d13.z-d13.y*d12.z;
			y1 = d12.z*d13.x-d12.x*d13.z;
			z1 = d12.x*d13.y-d12.y*d13.x;
			n = sqrt(x1*x1+y1*y1+z1*z1);
			normals->vertex[i].x = x1/n;
			normals->vertex[i].y = y1/n;
			normals->vertex[i].z = z1/n;
			}
		  }

		sprintf( comment, "Slabs  shifted by: %1.*g %1.*g %1.*g",
					Precision,CurrSeries->x3Doffset,Precision,CurrSeries->y3Doffset,Precision,CurrSeries->z3Doffset );
		}
}

void VRMLObject::CreateSphere( void )
{
	Section *section;
	Transform *transform;
	Contour *contour, *xyzcontour;
	Point *p, c, d12, d13;
	double x1, x2, x3, y1, y2, y3, z1, z2, z3, n, r, theta, phi, dtheta, dphi;
	int i, j, k, nphi, ntheta, total_points, total_faces, sectnum;
	char filename[MAX_PATH];

	this->contours = new Contours();						// create local storage list

	sectnum = this->firstSection-1;
	section = GetNextSectionBetween( sectnum, this->lastSection );
	while ( section && !Abort3D )							// get first section of object
	  {
	  if ( section->transforms )
		{
		transform = section->transforms->first;
		while ( transform )									// check each transform for contours
		  {
		  if ( transform->contours )
			{												// make local copy of each contour...
			contour = transform->contours->first;
			while ( contour )
				{
				if ( strcmp(this->name, contour->name) == 0 )	// ...that matched the name...
				  if ( contour->points )						// ...but only if have points
					{
					xyzcontour = new Contour( *contour );		// copy the contour
					xyzcontour->InvNform( transform->nform );	// transform into section, shift to 3D position
					xyzcontour->ShiftXYZ( CurrSeries->x3Doffset, CurrSeries->y3Doffset,
										CurrSectionsInfo->ZDistance(section->index,CurrSeries->zMidSection)+CurrSeries->z3Doffset );
					this->diffuseColor = xyzcontour->border;	// remember color from contour
					this->contours->Add( xyzcontour );			// copy contour into local list
					}
				contour = contour->next;					// do next contour in section
				}
			}
		  transform = transform->next;
		  }
		}
	  PutSection( section, false, false, false );			// free section memory
	  section = GetNextSectionBetween( sectnum, this->lastSection );
	  }														// and do next section

	min.x = MAX_FLOAT;  min.y = MAX_FLOAT;  min.z = MAX_FLOAT;
	max.x = -MAX_FLOAT; max.y = -MAX_FLOAT; max.z = -MAX_FLOAT;

	contour = this->contours->first;
	while ( contour && !Abort3D )
		{
		p = contour->points->first;
		while ( p )					// find number of vertices and bounds of vertices
			{
			if (p->x < min.x) min.x = p->x;
			if (p->x > max.x) max.x = p->x;
			if (p->y < min.y) min.y = p->y;
			if (p->y > max.y) max.y = p->y;
			if (p->z < min.z) min.z = p->z;
			if (p->z > max.z) max.z = p->z;
			p = p->next;
			}		
		contour = contour->next;					// do next contour
		}

	if ( min.x < MAX_FLOAT/2.0 )					// found something not empty
		{
		c.x = (min.x+max.x)/2.0; c.y = (min.y+max.y)/2.0; c.z = (min.z+max.z)/2.0;
		x2 = (max.x-min.x)/2.0; y2 = (max.y-min.y)/2.0; z2 = (max.z-min.z)/2.0;

		if ( CurrSeries->dim3Da > 0.0 ) r = CurrSeries->dim3Da;
		else {
			r = x2;
			if ( y2 > r ) r = y2;					// compute radius of bounding sphere
			if ( z2 > r ) r = z2;
			}
		min.x = c.x - r; min.y = c.y - r; min.z = c.z - r;
		max.x = c.x + r; max.y = c.y + r; max.z = c.z + r;

		nphi = CurrSeries->facets3D;				// determine # of vertices & faces in sphere
		ntheta = nphi/2;
		dphi = 2.0*PI/(double)nphi;
		dtheta = PI/(double)ntheta;
		total_points = nphi*(ntheta-1) + 2;
		total_faces = 2*nphi*(ntheta-1);
													// vertices of sphere include 2 points at caps
		vertices = new VertexSet( total_points );	
		vertices->vertex[0].x = c.x; vertices->vertex[0].y = c.y; vertices->vertex[0].z = c.z-r;
		vertices->vertex[total_points-1].x = c.x; vertices->vertex[total_points-1].y = c.y; vertices->vertex[total_points-1].z = c.z+r;
		
		i = 1;										// generate vertices of middle latitudes
		for (theta=dtheta-PI/2.0; theta<1.57; theta+=dtheta)
			for (phi=0.0; phi<6.28; phi+=dphi)
				{
				vertices->vertex[i].x = r*cos(theta)*cos(phi)+c.x;
				vertices->vertex[i].y = r*cos(theta)*sin(phi)+c.y;
				vertices->vertex[i].z = r*sin(theta)+c.z;
				i++;
				}
													// triangular faces
		faces = new FaceSet( total_faces );
		i = 0; j = 0;
		for (k=0; k<nphi-1; k++)					//  include nphi triangles of first end cap
			{
			faces->face[i].v1 = j; faces->face[i].v2 = j+k+2; faces->face[i].v3 = j+k+1;
			i++;
			}
		faces->face[i].v1 = j; faces->face[i].v2 = j+1; faces->face[i].v3 = j+k+1;
		i++;
		j = 1;										// generate faces of middle latitudes
		while ( j < (total_points-nphi-1) )
			{
			for (k=0; k<nphi-1; k++)
				{
				faces->face[i].v1 = j+k; faces->face[i].v2 = j+k+1; faces->face[i].v3 = j+k+1+nphi;
				i++;
				faces->face[i].v1 = j+k; faces->face[i].v2 = j+k+1+nphi; faces->face[i].v3 = j+k+nphi;
				i++;
				}
			faces->face[i].v1 = j+nphi-1; faces->face[i].v2 = j; faces->face[i].v3 = j+nphi;
			i++;
			faces->face[i].v1 = j+nphi-1; faces->face[i].v2 = j+nphi; faces->face[i].v3 = j+2*nphi-1;
			i++;
			j += nphi;
			}
		for (k=0; k<nphi-1; k++)					//  include nphi triangles of last end cap
			{
			faces->face[i].v1 = j+k; faces->face[i].v2 = j+k+1; faces->face[i].v3 = j+nphi;
			i++;
			}
		faces->face[i].v1 = j+nphi-1; faces->face[i].v2 = j; faces->face[i].v3 = j+nphi;

		if ( CurrSeries->faceNormals )
		  {
		  normalPerVertex = false;
		  normals = new VertexSet( total_faces );
		  for (i=0; i<total_faces; i++ )							// compute the normals from the faces
			{
			x1 = vertices->vertex[faces->face[i].v1].x; y1 = vertices->vertex[faces->face[i].v1].y; z1 = vertices->vertex[faces->face[i].v1].z;
			x2 = vertices->vertex[faces->face[i].v2].x; y2 = vertices->vertex[faces->face[i].v2].y; z2 = vertices->vertex[faces->face[i].v2].z;
			x3 = vertices->vertex[faces->face[i].v3].x; y3 = vertices->vertex[faces->face[i].v3].y; z3 = vertices->vertex[faces->face[i].v3].z;
			d12.x = x2-x1; d12.y = y2-y1; d12.z = z2-z1;
			d13.x = x3-x1; d13.y = y3-y1; d13.z = z3-z1;
			x1 = d12.y*d13.z-d13.y*d12.z;
			y1 = d12.z*d13.x-d12.x*d13.z;
			z1 = d12.x*d13.y-d12.y*d13.x;
			n = sqrt(x1*x1+y1*y1+z1*z1);
			normals->vertex[i].x = x1/n;
			normals->vertex[i].y = y1/n;
			normals->vertex[i].z = z1/n;
			}
		  }

		if ( CurrSeries->vertexNormals )
		  {
		  normalPerVertex = true;
		  normals = new VertexSet( total_points );
		  for (i=0; i<total_points; i++ )							// compute the normals from the vertices
			{
			x1 = vertices->vertex[i].x - c.x; y1 = vertices->vertex[i].y - c.y; z1 = vertices->vertex[i].z - c.z;
			n = sqrt(x1*x1+y1*y1+z1*z1);
			normals->vertex[i].x = x1/n;
			normals->vertex[i].y = y1/n;
			normals->vertex[i].z = z1/n;
			}
		  }

		sprintf( comment, "Sphere  center: %1.*g %1.*g %1.*g  size: %1.*g  shifted by: %1.*g %1.*g %1.*g",
						Precision,c.x,Precision,c.y,Precision,c.z,Precision,CurrSeries->dim3Da,
						Precision,CurrSeries->x3Doffset,Precision,CurrSeries->y3Doffset,Precision,CurrSeries->z3Doffset );
		}		// end if

	delete contours;					// these are never subsequently used, so just delete
	contours = NULL;
}

void VRMLObject::CreateEllipsoid( void )
{
	Section *section;
	Transform *transform;
	Contour *contour, *xyzcontour;
	Point *p, c, d12, d13, end1, end2;
	VertexSet *face_normals;
	double x1, x2, x3, y1, y2, y3, z1, z2, z3, n, phi, dphi, theta, dtheta;
	double **C, *d, **V, t;
	int i, j, k, s, nphi, ntheta, total_points, total_faces, sectnum;
	char filename[MAX_PATH];

	this->contours = new Contours();						// create local storage list

	sectnum = this->firstSection-1;
	section = GetNextSectionBetween( sectnum, this->lastSection );
	while ( section && !Abort3D )							// get first section of object
	  {
	  if ( section->transforms )
		{
		transform = section->transforms->first;
		while ( transform )									// check each transform for contours
		  {
		  if ( transform->contours )
			{												// make local copy of each contour...
			contour = transform->contours->first;
			while ( contour )
				{
				if ( strcmp(this->name, contour->name) == 0 )	// ...that matched the name...
				  if ( contour->points )						// ...but only if have points
					{
					xyzcontour = new Contour( *contour );		// copy the contour
					xyzcontour->InvNform( transform->nform );	// transform into section, shift to 3D position
					xyzcontour->ShiftXYZ( CurrSeries->x3Doffset, CurrSeries->y3Doffset,
										CurrSectionsInfo->ZDistance(section->index,CurrSeries->zMidSection)+CurrSeries->z3Doffset );
					this->diffuseColor = xyzcontour->border;	// remember color from contour
					this->contours->Add( xyzcontour );			// copy contour into local list
					}
				contour = contour->next;					// do next contour in section
				}
			}
		  transform = transform->next;
		  }
		}
	  PutSection( section, false, false, false );			// free section memory
	  section = GetNextSectionBetween( sectnum, this->lastSection );
	  }														// and do next section

	min.x = MAX_FLOAT;  min.y = MAX_FLOAT;  min.z = MAX_FLOAT;
	max.x = -MAX_FLOAT; max.y = -MAX_FLOAT; max.z = -MAX_FLOAT;

	if ( contours->Number() && !Abort3D )			// if have any data
		{
		C = new double*[3]; for (k=0;k<3;k++) C[k] = new double[3];	// allocate 3x3 matrix
		for (i=0; i<3; i++)
			for (j=0; j<3; j++) C[i][j] = 0.0;		// use to accumulate scatter matrix so start at zero

		n = 0.0;									// use trace points to get axes of cylinder
		contour = this->contours->first;
		while ( contour )
			{
			p = contour->points->first;
			while ( p )
				{
				x1 = p->x; y1 = p->y; z1 = p->z;	// calculate elements of scatter matrix
				C[0][0] += x1*x1; C[0][1] += x1*y1; C[0][2] += x1*z1;
				C[1][0] += x1*y1; C[1][1] += y1*y1; C[1][2] += y1*z1;
				C[2][0] += x1*z1; C[2][1] += y1*z1; C[2][2] += z1*z1;
				c.x += x1; c.y += y1; c.z += z1;	// accumulate centroid
				n += 1.0;							// count number of points
				if (x1 < min.x) min.x = x1;
				if (x1 > max.x) max.x = x1;
				if (y1 < min.y) min.y = y1;
				if (y1 > max.y) max.y = y1;
				if (z1 < min.z) min.z = z1;
				if (z1 > max.z) max.z = z1;
				p = p->next;
				}
			contour = contour->next;					// do next contour
			}

		c.x = c.x/n; c.y = c.y/n; c.z = c.z/n;			// compute centroid of contours

		V = new double*[3]; for (k=0;k<3;k++) V[k] = new double[3];	// allocate 3x3 matrix
		d = new double[3];
														// compute final scatter matrix
		d[0] = c.x; d[1] = c.y; d[2] = c.z;
		for (i=0; i<3; i++)
			for (j=0; j<3; j++) C[i][j] = C[i][j]/n - d[i]*d[j];
		
		svd( C, 3, 3, d, V );							// svd for eigenvectors, then bubble sort descending
		i = 0; j = 1; k = 2;
		if ( d[1] < d[2] ) { t = d[2]; d[1] = d[2]; d[2] = t; s = j; j = k; k = s; }
		if ( d[0] < d[1] ) { t = d[1]; d[0] = d[1]; d[1] = t; s = i; i = j; j = s; }
		if ( d[1] < d[2] ) { t = d[2]; d[1] = d[2]; d[1] = t; s = j; j = k; k = s; }
		
		if ( CurrSeries->dim3Da > 0.0 ) d[i] = CurrSeries->dim3Da;
		if ( CurrSeries->dim3Db > 0.0 ) d[j] = CurrSeries->dim3Db;
		if ( CurrSeries->dim3Dc > 0.0 ) d[k] = CurrSeries->dim3Dc;

		x1 = d[i]*V[0][i]; y1 = d[i]*V[1][i]; z1 = d[i]*V[2][i];	// the principal axis
		x2 = d[j]*V[0][j]; y2 = d[j]*V[1][j]; z2 = d[j]*V[2][j];	// the second minor axis
		x3 = d[k]*V[0][k]; y3 = d[k]*V[1][k]; z3 = d[k]*V[2][k];	// the third axis

		sprintf( comment, "Ellipsoid  center: %1.*g %1.*g %1.*g  axes: %1.*g %1.*g %1.*g, %1.*g %1.*g %1.*g, %1.*g %1.*g %1.*g  size: %1.*g %1.*g %1.*g  shifted by: %1.*g %1.*g %1.*g",
						Precision,c.x,Precision,c.y,Precision,c.z,
						Precision,V[0][i],Precision,V[1][i],Precision,V[2][i],Precision,V[0][j],
						Precision,V[1][j],Precision,V[2][j],Precision,V[0][k],Precision,V[1][k],Precision,V[2][k],
						Precision,d[i],Precision,d[j],Precision,d[k],
						Precision,CurrSeries->x3Doffset,Precision,CurrSeries->y3Doffset,Precision,CurrSeries->z3Doffset );

		delete[] d;										// free dynamic memory used
		for (k=0;k<3;k++) delete[] V[k]; delete[] V;
		for (k=0;k<3;k++) delete[] C[k]; delete[] C;


		nphi = CurrSeries->facets3D;					// determine # of vertices & faces in sphere
		ntheta = nphi/2;
		dphi = 2.0*PI/(double)nphi;
		dtheta = PI/(double)ntheta;
		total_points = nphi*(ntheta-1) + 2;
		total_faces = 2*nphi*(ntheta-1);
													// vertices of ellipsoid include 2 points at caps
		vertices = new VertexSet( total_points );	
		vertices->vertex[0].x = c.x-x3; vertices->vertex[0].y = c.y-y3; vertices->vertex[0].z = c.z-z3;
		vertices->vertex[total_points-1].x = c.x+x3; vertices->vertex[total_points-1].y = c.y+y3;
			vertices->vertex[total_points-1].z = c.z+z3;
		
		i = 1;										// generate vertices of middle latitudes
		for (theta=dtheta-PI/2.0; theta<1.57; theta+=dtheta)
			for (phi=0.0; phi<6.28; phi+=dphi)
				{
				vertices->vertex[i].x = (cos(phi)*x1 + sin(phi)*x2)*cos(theta)+x3*sin(theta)+c.x;
				vertices->vertex[i].y = (cos(phi)*y1 + sin(phi)*y2)*cos(theta)+y3*sin(theta)+c.y;
				vertices->vertex[i].z = (cos(phi)*z1 + sin(phi)*z2)*cos(theta)+z3*sin(theta)+c.z;
				i++;
				}
													// triangular faces
		faces = new FaceSet( total_faces );
		i = 0; j = 0;
		for (k=0; k<nphi-1; k++)					//  include nphi triangles of first end cap
			{
			faces->face[i].v1 = j; faces->face[i].v2 = j+k+2; faces->face[i].v3 = j+k+1;
			i++;
			}
		faces->face[i].v1 = j; faces->face[i].v2 = j+1; faces->face[i].v3 = j+k+1;
		i++;
		j = 1;										// generate faces of middle latitudes
		while ( j < (total_points-nphi-1) )
			{
			for (k=0; k<nphi-1; k++)
				{
				faces->face[i].v1 = j+k; faces->face[i].v2 = j+k+1; faces->face[i].v3 = j+k+1+nphi;
				i++;
				faces->face[i].v1 = j+k; faces->face[i].v2 = j+k+1+nphi; faces->face[i].v3 = j+k+nphi;
				i++;
				}
			faces->face[i].v1 = j+nphi-1; faces->face[i].v2 = j; faces->face[i].v3 = j+nphi;
			i++;
			faces->face[i].v1 = j+nphi-1; faces->face[i].v2 = j+nphi; faces->face[i].v3 = j+2*nphi-1;
			i++;
			j += nphi;
			}
		for (k=0; k<nphi-1; k++)					//  include nphi triangles of last end cap
			{
			faces->face[i].v1 = j+k; faces->face[i].v2 = j+k+1; faces->face[i].v3 = j+nphi;
			i++;
			}
		faces->face[i].v1 = j+nphi-1; faces->face[i].v2 = j; faces->face[i].v3 = j+nphi;


		if ( CurrSeries->faceNormals || CurrSeries->vertexNormals )
			{
			face_normals = new VertexSet( total_faces );
			for (i=0; i<total_faces; i++ )							// compute the normals from the faces
				{
				x1 = vertices->vertex[faces->face[i].v1].x; y1 = vertices->vertex[faces->face[i].v1].y; z1 = vertices->vertex[faces->face[i].v1].z;
				x2 = vertices->vertex[faces->face[i].v2].x; y2 = vertices->vertex[faces->face[i].v2].y; z2 = vertices->vertex[faces->face[i].v2].z;
				x3 = vertices->vertex[faces->face[i].v3].x; y3 = vertices->vertex[faces->face[i].v3].y; z3 = vertices->vertex[faces->face[i].v3].z;
				d12.x = x2-x1; d12.y = y2-y1; d12.z = z2-z1;
				d13.x = x3-x1; d13.y = y3-y1; d13.z = z3-z1;
				x1 = d12.y*d13.z-d13.y*d12.z;
				y1 = d12.z*d13.x-d12.x*d13.z;
				z1 = d12.x*d13.y-d12.y*d13.x;
				n = sqrt(x1*x1+y1*y1+z1*z1);
				face_normals->vertex[i].x = x1/n;
				face_normals->vertex[i].y = y1/n;
				face_normals->vertex[i].z = z1/n;
				}
			normalPerVertex = false;
			if ( CurrSeries->faceNormals ) normals = face_normals;	// just use the computed face normals or
			else													// estimate vertex normals from faces...
				{
				normalPerVertex = true;								// ...in the case vertex normals
				normals = new VertexSet( total_points );
				for (i=0; i<total_points; i++ )						// compute the normals from the vertices
					{
					x1 = 0.0; y1 = 0.0; z1 = 0.0;
					for ( j=0; j<total_faces; j++ )
						if ( (faces->face[j].v1 == (unsigned int)i) || (faces->face[j].v2 == (unsigned int)i) || (faces->face[j].v3 == (unsigned int)i) )
							{
							x1 += face_normals->vertex[j].x;
							y1 += face_normals->vertex[j].y;
							z1 += face_normals->vertex[j].z;
							}
					normals->vertex[i].x = x1;
					normals->vertex[i].y = y1;
					normals->vertex[i].z = z1;
					n = sqrt(x1*x1+y1*y1+z1*z1);
					if ( n > 0.0005 )
						{
						normals->vertex[i].x /= n;
						normals->vertex[i].y /= n;
						normals->vertex[i].z /= n;
						}
					}
				delete face_normals;
				}
			}		// end if normals
		}

	delete contours;					// these are never subsequently used, so just delete
	contours = NULL;	
}


void VRMLObject::CreateCylinder( void )
{
	Section *section;
	Transform *transform;
	Contour *contour, *xyzcontour;
	Point *p, c, d12, d13, end1, end2;
	VertexSet *face_normals;
	double x1, x2, x3, y1, y2, y3, z1, z2, z3, n, theta, dtheta;
	double **C, *d, **V, t;
	int i, j, k, s, nphi, ntheta, total_points, total_faces, sectnum;
	char filename[MAX_PATH];

	this->contours = new Contours();						// create local storage list

	sectnum = this->firstSection-1;
	section = GetNextSectionBetween( sectnum, this->lastSection );
	while ( section && !Abort3D )							// get first section of object
	  {
	  if ( section->transforms )
		{
		transform = section->transforms->first;
		while ( transform )									// check each transform for contours
		  {
		  if ( transform->contours )
			{												// make local copy of each contour...
			contour = transform->contours->first;
			while ( contour )
				{
				if ( strcmp(this->name, contour->name) == 0 )	// ...that matched the name...
				  if ( contour->points )						// ...but only if have points
					{
					xyzcontour = new Contour( *contour );		// copy the contour
					xyzcontour->InvNform( transform->nform );	// transform into section, shift to 3D position
					xyzcontour->ShiftXYZ( CurrSeries->x3Doffset, CurrSeries->y3Doffset,
										CurrSectionsInfo->ZDistance(section->index,CurrSeries->zMidSection)+CurrSeries->z3Doffset );
					this->diffuseColor = xyzcontour->border;
					this->contours->Add( xyzcontour );			// copy contour into local list
					}
				contour = contour->next;					// do next contour in section
				}
			}
		  transform = transform->next;
		  }
		}
	  PutSection( section, false, false, false );			// free section memory
	  section = GetNextSectionBetween( sectnum, this->lastSection );
	  }														// and do next section

	min.x = MAX_FLOAT;  min.y = MAX_FLOAT;  min.z = MAX_FLOAT;
	max.x = -MAX_FLOAT; max.y = -MAX_FLOAT; max.z = -MAX_FLOAT;

	if ( contours->Number() && !Abort3D )			// if have any data
		{
		C = new double*[3]; for (k=0;k<3;k++) C[k] = new double[3];	// allocate 3x3 matrix
		for (i=0; i<3; i++)
			for (j=0; j<3; j++) C[i][j] = 0.0;		// use to accumulate scatter matrix so start at zero

		n = 0.0;									// use trace points to get axes of cylinder
		contour = this->contours->first;
		while ( contour )
			{
			p = contour->points->first;
			while ( p )
				{
				x1 = p->x; y1 = p->y; z1 = p->z;	// calculate elements of scatter matrix
				C[0][0] += x1*x1; C[0][1] += x1*y1; C[0][2] += x1*z1;
				C[1][0] += x1*y1; C[1][1] += y1*y1; C[1][2] += y1*z1;
				C[2][0] += x1*z1; C[2][1] += y1*z1; C[2][2] += z1*z1;
				c.x += x1; c.y += y1; c.z += z1;	// accumulate centroid
				n += 1.0;							// count number of points
				if (x1 < min.x) min.x = x1;
				if (x1 > max.x) max.x = x1;
				if (y1 < min.y) min.y = y1;
				if (y1 > max.y) max.y = y1;
				if (z1 < min.z) min.z = z1;
				if (z1 > max.z) max.z = z1;
				p = p->next;
				}
			contour = contour->next;					// do next contour
			}

		c.x = c.x/n; c.y = c.y/n; c.z = c.z/n;			// compute centroid of contours

		V = new double*[3]; for (k=0;k<3;k++) V[k] = new double[3];	// allocate 3x3 matrix
		d = new double[3];
														// compute final scatter matrix
		d[0] = c.x; d[1] = c.y; d[2] = c.z;
		for (i=0; i<3; i++)
			for (j=0; j<3; j++) C[i][j] = C[i][j]/n - d[i]*d[j];
		
		svd( C, 3, 3, d, V );							// svd for eigenvectors, then bubble sort descending
		i = 0; j = 1; k = 2;
		if ( d[1] < d[2] ) { t = d[2]; d[1] = d[2]; d[2] = t; s = j; j = k; k = s; }
		if ( d[0] < d[1] ) { t = d[1]; d[0] = d[1]; d[1] = t; s = i; i = j; j = s; }
		if ( d[1] < d[2] ) { t = d[2]; d[1] = d[2]; d[1] = t; s = j; j = k; k = s; }
		
		if ( CurrSeries->dim3Da > 0.0 ) d[i] = CurrSeries->dim3Da;
		if ( CurrSeries->dim3Db > 0.0 ) d[j] = CurrSeries->dim3Db;
		if ( CurrSeries->dim3Dc > 0.0 ) d[k] = CurrSeries->dim3Dc;

		x1 = d[i]*V[0][i]; y1 = d[i]*V[1][i]; z1 = d[i]*V[2][i];	// the principal axis
		x2 = d[j]*V[0][j]; y2 = d[j]*V[1][j]; z2 = d[j]*V[2][j];	// the second minor axis
		x3 = d[k]*V[0][k]; y3 = d[k]*V[1][k]; z3 = d[k]*V[2][k];	// the third axis

		sprintf( comment, "Cylinder  center: %1.*g %1.*g %1.*g  axes: %1.*g %1.*g %1.*g, %1.*g %1.*g %1.*g, %1.*g %1.*g %1.*g  size: %1.*g %1.*g %1.*g  shifted by: %1.*g %1.*g %1.*g",
						Precision,c.x,Precision,c.y,Precision,c.z,
						Precision,V[0][i],Precision,V[1][i],Precision,V[2][i],Precision,V[0][j],
						Precision,V[1][j],Precision,V[2][j],Precision,V[0][k],Precision,V[1][k],Precision,V[2][k],
						Precision,d[i],Precision,d[j],Precision,d[k],
						Precision,CurrSeries->x3Doffset,Precision,CurrSeries->y3Doffset,Precision,CurrSeries->z3Doffset );

		delete[] d;										// free dynamic memory used
		for (k=0;k<3;k++) delete[] V[k]; delete[] V;
		for (k=0;k<3;k++) delete[] C[k]; delete[] C;


		ntheta = CurrSeries->facets3D;			// determine # of vertices & faces in sphere
		dtheta = 2.0*PI/(double)ntheta;
		total_points = 2*ntheta;
		total_faces = 2*ntheta;
		end1.x = c.x - x1;
		end1.y = c.y - y1;
		end1.z = c.z - z1;
		end2.x = c.x + x1;
		end2.y = c.y + y1;
		end2.z = c.z + z1;
													// vertices of sphere include 2 points at caps
		vertices = new VertexSet( total_points );	
		i = 0;										// generate vertices of middle latitudes
		for (theta=-PI; theta<3.14; theta+=dtheta)
			{
			vertices->vertex[i].x = cos(theta)*x2 + sin(theta)*x3 + end1.x;
			vertices->vertex[i].y = cos(theta)*y2 + sin(theta)*y3 + end1.y;
			vertices->vertex[i].z = cos(theta)*z2 + sin(theta)*z3 + end1.z;
			vertices->vertex[i+1].x = cos(theta)*x2 + sin(theta)*x3 + end2.x;
			vertices->vertex[i+1].y = cos(theta)*y2 + sin(theta)*y3 + end2.y;
			vertices->vertex[i+1].z = cos(theta)*z2 + sin(theta)*z3 + end2.z;
			i += 2;
			}
													// triangular faces
		faces = new FaceSet( total_faces );
		i = 0;
		while ( i < total_faces-2 )
			{
			faces->face[i].v1 = i; faces->face[i].v2 = i+3; faces->face[i].v3 = i+1;
			faces->face[i+1].v1 = i; faces->face[i+1].v2 = i+2; faces->face[i+1].v3 = i+3;
			i += 2;
			}
		faces->face[i].v1 = i; faces->face[i].v2 = 1; faces->face[i].v3 = i+1;
		faces->face[i+1].v1 = i; faces->face[i+1].v2 = 0; faces->face[i+1].v3 = 1;

		if ( CurrSeries->faceNormals || CurrSeries->vertexNormals )
			{
			face_normals = new VertexSet( total_faces );
			for (i=0; i<total_faces; i++ )							// compute the normals from the faces
				{
				x1 = vertices->vertex[faces->face[i].v1].x; y1 = vertices->vertex[faces->face[i].v1].y; z1 = vertices->vertex[faces->face[i].v1].z;
				x2 = vertices->vertex[faces->face[i].v2].x; y2 = vertices->vertex[faces->face[i].v2].y; z2 = vertices->vertex[faces->face[i].v2].z;
				x3 = vertices->vertex[faces->face[i].v3].x; y3 = vertices->vertex[faces->face[i].v3].y; z3 = vertices->vertex[faces->face[i].v3].z;
				d12.x = x2-x1; d12.y = y2-y1; d12.z = z2-z1;
				d13.x = x3-x1; d13.y = y3-y1; d13.z = z3-z1;
				x1 = d12.y*d13.z-d13.y*d12.z;
				y1 = d12.z*d13.x-d12.x*d13.z;
				z1 = d12.x*d13.y-d12.y*d13.x;
				n = sqrt(x1*x1+y1*y1+z1*z1);
				face_normals->vertex[i].x = x1/n;
				face_normals->vertex[i].y = y1/n;
				face_normals->vertex[i].z = z1/n;
				}
			normalPerVertex = false;
			if ( CurrSeries->faceNormals ) normals = face_normals;	// just use the computed face normals or
			else													// estimate vertex normals from faces...
				{
				normalPerVertex = true;								// ...in the case vertex normals
				normals = new VertexSet( total_points );
				for (i=0; i<total_points; i++ )						// compute the normals from the vertices
					{
					x1 = 0.0; y1 = 0.0; z1 = 0.0;
					for ( j=0; j<total_faces; j++ )
						if ( (faces->face[j].v1 == (unsigned int)i) || (faces->face[j].v2 == (unsigned int)i) || (faces->face[j].v3 == (unsigned int)i) )
							{
							x1 += face_normals->vertex[j].x;
							y1 += face_normals->vertex[j].y;
							z1 += face_normals->vertex[j].z;
							}
					normals->vertex[i].x = x1;
					normals->vertex[i].y = y1;
					normals->vertex[i].z = z1;
					n = sqrt(x1*x1+y1*y1+z1*z1);
					if ( n > 0.0005 )
						{
						normals->vertex[i].x /= n;
						normals->vertex[i].y /= n;
						normals->vertex[i].z /= n;
						}
					}
				delete face_normals;
				}
			}
		}		// end if

	delete contours;					// these are never subsequently used, so just delete
	contours = NULL;
}

void VRMLObject::CreateBox( void )
{
	Section *section;
	Transform *transform;
	Contour *contour, *xyzcontour;
	Point *p, d12, d13;
	int i, sectnum, total_points, total_faces;
	double n, x1, x2, x3, y1, y2, y3, z1, z2, z3;
	char filename[MAX_PATH];

	this->contours = new Contours();						// create local storage list

	sectnum = this->firstSection-1;
	section = GetNextSectionBetween( sectnum, this->lastSection );
	while ( section && !Abort3D )							// get first section of object
	  {
	  if ( section->transforms )
		{
		transform = section->transforms->first;
		while ( transform )									// check each transform for contours
		  {
		  if ( transform->contours )
			{												// make local copy of each contour...
			contour = transform->contours->first;
			while ( contour )
				{
				if ( strcmp(this->name, contour->name) == 0 )	// ...that matched the name...
				  if ( contour->points )						// ...but only if have points
					{
					xyzcontour = new Contour( *contour );		// copy the contour
					xyzcontour->InvNform( transform->nform );	// transform into section, shift to 3D position
					xyzcontour->ShiftXYZ( CurrSeries->x3Doffset, CurrSeries->y3Doffset,
										CurrSectionsInfo->ZDistance(section->index,CurrSeries->zMidSection)+CurrSeries->z3Doffset );
					this->diffuseColor = xyzcontour->border;	// remember color from contour
					this->contours->Add( xyzcontour );			// copy contour into local list
					}
				contour = contour->next;					// do next contour in section
				}
			}
		  transform = transform->next;
		  }
		}
	  PutSection( section, false, false, false );			// free section memory
	  section = GetNextSectionBetween( sectnum, this->lastSection );
	  }														// and do next section

	min.x = MAX_FLOAT;  min.y = MAX_FLOAT;  min.z = MAX_FLOAT;
	max.x = -MAX_FLOAT; max.y = -MAX_FLOAT; max.z = -MAX_FLOAT;

	contour = this->contours->first;
	while ( contour && !Abort3D )
		{
		p = contour->points->first;
		while ( p )					// find extent of contour vertices
			{
			if (p->x < min.x) min.x = p->x;
			if (p->x > max.x) max.x = p->x;
			if (p->y < min.y) min.y = p->y;
			if (p->y > max.y) max.y = p->y;
			if (p->z < min.z) min.z = p->z;
			if (p->z > max.z) max.z = p->z;
			p = p->next;
			}		
		contour = contour->next;					// do next contour
		}
													// calculate center of box and determine extent

	x1 = (min.x+max.x)/2.0; y1 = (min.y+max.y)/2.0; z1 = (min.z+max.z)/2.0;
	if ( CurrSeries->dim3Da > 0.0 ) x2 = CurrSeries->dim3Da/2.0;
	else x2 = (max.x-min.x)/2.0;
	if ( CurrSeries->dim3Db > 0.0 ) y2 = CurrSeries->dim3Db/2.0;
	else y2 = (max.y-min.y)/2.0;
	if ( CurrSeries->dim3Dc > 0.0 ) z2 = CurrSeries->dim3Dc/2.0;
	else z2 = (max.z-min.z)/2.0;

	total_points = 8;								// now create indexed array representation
	vertices = new VertexSet( total_points );
	vertices->vertex[0].x = x1-x2; vertices->vertex[0].y = y1-y2; vertices->vertex[0].z = z1-z2;
	vertices->vertex[1].x = x1-x2; vertices->vertex[1].y = y1-y2; vertices->vertex[1].z = z1+z2;
	vertices->vertex[2].x = x1-x2; vertices->vertex[2].y = y1+y2; vertices->vertex[2].z = z1-z2;
	vertices->vertex[3].x = x1-x2; vertices->vertex[3].y = y1+y2; vertices->vertex[3].z = z1+z2;
	vertices->vertex[4].x = x1+x2; vertices->vertex[4].y = y1-y2; vertices->vertex[4].z = z1-z2;
	vertices->vertex[5].x = x1+x2; vertices->vertex[5].y = y1-y2; vertices->vertex[5].z = z1+z2;
	vertices->vertex[6].x = x1+x2; vertices->vertex[6].y = y1+y2; vertices->vertex[6].z = z1-z2;
	vertices->vertex[7].x = x1+x2; vertices->vertex[7].y = y1+y2; vertices->vertex[7].z = z1+z2;
	total_faces = 12;
	faces = new FaceSet( total_faces );
	faces->face[0].v1 = 0; faces->face[0].v2 = 1; faces->face[0].v3 = 2;
	faces->face[1].v1 = 2; faces->face[1].v2 = 1; faces->face[1].v3 = 3;
	faces->face[2].v1 = 0; faces->face[2].v2 = 4; faces->face[2].v3 = 1;
	faces->face[3].v1 = 1; faces->face[3].v2 = 4; faces->face[3].v3 = 5;
	faces->face[4].v1 = 1; faces->face[4].v2 = 5; faces->face[4].v3 = 7;
	faces->face[5].v1 = 1; faces->face[5].v2 = 7; faces->face[5].v3 = 3;
	faces->face[6].v1 = 3; faces->face[6].v2 = 6; faces->face[6].v3 = 2;
	faces->face[7].v1 = 3; faces->face[7].v2 = 7; faces->face[7].v3 = 6;
	faces->face[8].v1 = 6; faces->face[8].v2 = 0; faces->face[8].v3 = 2;
	faces->face[9].v1 = 6; faces->face[9].v2 = 4; faces->face[9].v3 = 0;
	faces->face[10].v1 = 6; faces->face[10].v2 = 5; faces->face[10].v3 = 4;
	faces->face[11].v1 = 6; faces->face[11].v2 = 7; faces->face[11].v3 = 5;

	if ( CurrSeries->vertexNormals )
		{
		normalPerVertex = true;
		normals = new VertexSet( total_points );
		n = sqrt(1.0/3.0);
		normals->vertex[0].x = -n; normals->vertex[0].y = -n; normals->vertex[0].z = -n;
		normals->vertex[1].x = -n; normals->vertex[1].y = -n; normals->vertex[1].z = n;
		normals->vertex[2].x = -n; normals->vertex[2].y = n; normals->vertex[2].z = -n;
		normals->vertex[3].x = -n; normals->vertex[3].y = n; normals->vertex[3].z = n;
		normals->vertex[4].x = n; normals->vertex[4].y = -n; normals->vertex[4].z = -n;
		normals->vertex[5].x = n; normals->vertex[5].y = -n; normals->vertex[5].z = n;
		normals->vertex[6].x = n; normals->vertex[6].y = n; normals->vertex[6].z = -n;
		normals->vertex[7].x = n; normals->vertex[7].y = n; normals->vertex[7].z = n;
		}
	
	if ( CurrSeries->faceNormals )
		{
		normalPerVertex = false;
		normals = new VertexSet( total_faces );
		for (i=0; i<total_faces; i++ )							// compute the normals from the faces
			{
			x1 = vertices->vertex[faces->face[i].v1].x; y1 = vertices->vertex[faces->face[i].v1].y; z1 = vertices->vertex[faces->face[i].v1].z;
			x2 = vertices->vertex[faces->face[i].v2].x; y2 = vertices->vertex[faces->face[i].v2].y; z2 = vertices->vertex[faces->face[i].v2].z;
			x3 = vertices->vertex[faces->face[i].v3].x; y3 = vertices->vertex[faces->face[i].v3].y; z3 = vertices->vertex[faces->face[i].v3].z;
			d12.x = x2-x1; d12.y = y2-y1; d12.z = z2-z1;
			d13.x = x3-x1; d13.y = y3-y1; d13.z = z3-z1;
			x1 = d12.y*d13.z-d13.y*d12.z;
			y1 = d12.z*d13.x-d12.x*d13.z;
			z1 = d12.x*d13.y-d12.y*d13.x;
			n = sqrt(x1*x1+y1*y1+z1*z1);
			normals->vertex[i].x = x1/n;
			normals->vertex[i].y = y1/n;
			normals->vertex[i].z = z1/n;
			}
		}

	sprintf( comment, "Box  size: %1.*g %1.*g %1.*g  shifted by: %1.*g %1.*g %1.*g",
						Precision,CurrSeries->dim3Da,Precision,CurrSeries->dim3Db,Precision,CurrSeries->dim3Dc,
						Precision,CurrSeries->x3Doffset,Precision,CurrSeries->y3Doffset,Precision,CurrSeries->z3Doffset );

	delete contours;					// these are never subsequently used, so just delete
	contours = NULL;
}

void VRMLObject::CreatePointSet( void )
{
	Section *section;
	Transform *transform;
	Contour *contour, *xyzcontour;
	Point cmin, cmax, Zoffset;
	double x1, x2, y1, y2, z1, z2;
	int i, j, total_points, sectnum;
	char filename[MAX_PATH];

	this->contours = new Contours();						// create local storage list

	sectnum = this->firstSection-1;
	section = GetNextSectionBetween( sectnum, this->lastSection );
	while ( section && !Abort3D )										// get first section of object
	  {
	  if ( section->transforms )
		{
		transform = section->transforms->first;
		while ( transform )									// check each transform for contours
		  {
		  if ( transform->contours )
			{												// make local copy of each contour...
			contour = transform->contours->first;
			while ( contour )
				{
				if ( strcmp(this->name, contour->name) == 0 )	// ...that matched the name...
				  if ( contour->points )						// ...but only if have points
					{
					xyzcontour = new Contour( *contour );		// copy the contour
					xyzcontour->InvNform( transform->nform );	// transform into section
					if ( ApplyZOffset3D )
						{										// shift within section
						Zoffset = CurrSeries->OffsetBetweenZTraces( sectnum, OffsetZTrace1, OffsetZTrace2 );
						xyzcontour->ShiftXYZ( Zoffset.x, Zoffset.y, Zoffset.z );
						}										// shift whole object
					xyzcontour->ShiftXYZ( CurrSeries->x3Doffset, CurrSeries->y3Doffset,
											CurrSectionsInfo->ZDistance(section->index,CurrSeries->zMidSection)+CurrSeries->z3Doffset );
					this->emissiveColor = xyzcontour->border;
					this->diffuseColor = Color( 0.0, 0.0, 0.0 );
					this->contours->Add( xyzcontour );			// copy contour into local list
					}
				contour = contour->next;					// do next contour in section
				}
			}
		  transform = transform->next;
		  }
		}
	  PutSection( section, false, false, false );			// free section memory
	  section = GetNextSectionBetween( sectnum, this->lastSection );
	  }														// and do next section

	total_points = 3*this->contours->Number();				// each contour will receive a triangular face
	vertices = new VertexSet( total_points );
	faces = new FaceSet( this->contours->Number() );		// one face for each contour
	i = 0; j = 0;
	contour = this->contours->first;
	while ( contour )
		{
		contour->Extent( &cmin, &cmax );					// find midpt of face
		
		x1 = (cmin.x+cmax.x)/2.0; y1 = (cmin.y+cmax.y)/2.0; z1 = (cmin.z+cmax.z)/2.0;

		if ( CurrSeries->dim3Da > 0.0 ) x2 = CurrSeries->dim3Da/2.0;	// size face using parameters
		else x2 = (cmax.x-cmin.x)/2.0;								// or make face proportional to trace size
		if ( CurrSeries->dim3Db > 0.0 ) y2 = CurrSeries->dim3Db/2.0;
		else y2 = (cmax.y-cmin.y)/2.0;

		vertices->vertex[i].x = x1-x2; vertices->vertex[i].y = y1-y2; vertices->vertex[i].z = z1;
		vertices->vertex[i+1].x = x1+x2; vertices->vertex[i+1].y = y1-y2; vertices->vertex[i+1].z = z1;
		vertices->vertex[i+2].x = x1; vertices->vertex[i+2].y = y1+y2; vertices->vertex[i+2].z = z1;
		
		faces->face[j].v1 = i; faces->face[j].v2 = i+1; faces->face[j].v3 = i+2;

		i += 3;
		j++;
		contour = contour->next;							// do next contour
		}							// end while

															// find extent of VRML object
	min.x = MAX_FLOAT;  min.y = MAX_FLOAT;  min.z = MAX_FLOAT;
	max.x = -MAX_FLOAT; max.y = -MAX_FLOAT; max.z = -MAX_FLOAT;
	for (i=0; i<total_points; i++)
		{
		if (vertices->vertex[i].x < min.x) min.x = vertices->vertex[i].x;
		if (vertices->vertex[i].x > max.x) max.x = vertices->vertex[i].x;
		if (vertices->vertex[i].y < min.y) min.y = vertices->vertex[i].y;
		if (vertices->vertex[i].y > max.y) max.y = vertices->vertex[i].y;
		if (vertices->vertex[i].z < min.z) min.z = vertices->vertex[i].z;
		if (vertices->vertex[i].z > max.z) max.z = vertices->vertex[i].z;
		}

	sprintf( comment, "Trace Midpoints  size: %1.*g %1.*g  shifted by: %1.*g %1.*g %1.*g",
						Precision,CurrSeries->dim3Da,Precision,CurrSeries->dim3Db,
						Precision,CurrSeries->x3Doffset,Precision,CurrSeries->y3Doffset,Precision,CurrSeries->z3Doffset );

	delete contours;					// these are never subsequently used, so just delete
	contours = NULL;
}


void VRMLObject::Compile( void )				// output data arrays into OpenGL display list
{
	unsigned int h, i, j, k;
	Point *v;
	Line *l;
	Face *f;
	Point *n;
	GLfloat s;
	GLfloat acolor[4], dcolor[4], ecolor[4], scolor[4], black[4];	// setup color vectors for openGL materials
	
	acolor[0] = (GLfloat)(ambientIntensity*diffuseColor.r); acolor[1] = (GLfloat)(ambientIntensity*diffuseColor.g);
	acolor[2] = (GLfloat)(ambientIntensity*diffuseColor.b); acolor[3] = (GLfloat)(1.0-transparency);
	dcolor[0] = (GLfloat)diffuseColor.r; dcolor[1] = (GLfloat)diffuseColor.g; dcolor[2] = (GLfloat)diffuseColor.b; dcolor[3] = (GLfloat)(1.0-transparency);
	ecolor[0] = (GLfloat)emissiveColor.r; ecolor[1] = (GLfloat)emissiveColor.g; ecolor[2] = (GLfloat)emissiveColor.b; ecolor[3] = (GLfloat)(1.0-transparency);
	scolor[0] = (GLfloat)specularColor.r; scolor[1] = (GLfloat)specularColor.g; scolor[2] = (GLfloat)specularColor.b; scolor[3] = (GLfloat)(1.0-transparency);
	black[0] = 0.0; black[1] = 0.0; black[2] = 0.0; black[3] = 1.0;
	s = (GLfloat)(128.0*shininess);		// the VRML parameter is constrained to [0,1] while openGL is [0,128]

	if ( vertices )
	  if ( lines )
		{
		if ( openGLList && glIsList( openGLList ) ) glDeleteLists( openGLList, 1 );
		openGLList = glGenLists(1);
		if ( openGLList )
			{
			glNewList( openGLList, GL_COMPILE );
			v = vertices->vertex;
			l = lines->line;
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ecolor );
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black );
			glBegin( GL_LINES );
		
			for ( k=0; k<lines->total; k++ )
				{
				i = l[k].v1;
				j = l[k].v2;
				glVertex3f( v[i].x, v[i].y, v[i].z );
				glVertex3f( v[j].x, v[j].y, v[j].z );
				}
		
			glEnd();		
			glEndList();
			}
		}
	  else if ( faces )
		{
		if ( openGLList && glIsList( openGLList ) ) glDeleteLists( openGLList, 1 );
		openGLList = glGenLists(1);
		if ( openGLList )
			{
			glNewList( openGLList, GL_COMPILE );
			v = vertices->vertex;
			f = faces->face;
			if ( normals ) n = normals->vertex;
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, acolor );
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dcolor );
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ecolor );
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, scolor );
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, s );
			if ( frontFill ) glPolygonMode( GL_FRONT, GL_FILL );
			else glPolygonMode( GL_FRONT, GL_LINE );
			if ( backFill ) glPolygonMode( GL_BACK, GL_FILL );
			else glPolygonMode( GL_BACK, GL_LINE );

			glBegin( GL_TRIANGLES );

			for ( k=0; k<faces->total; k++ )
				{
				h = f[k].v1;		// h is the index of v1 in the vertex list
				i = f[k].v2;		// so its also the index of v1's normal vector
				j = f[k].v3;		// k is index of face's normal
				if ( normals && !normalPerVertex ) glNormal3f( n[k].x, n[k].y, n[k].z );
				if ( normals && normalPerVertex ) glNormal3f( n[h].x, n[h].y, n[h].z );
				glVertex3f( v[h].x, v[h].y, v[h].z );
				if ( normals && normalPerVertex ) glNormal3f( n[i].x, n[i].y, n[i].z );
				glVertex3f( v[i].x, v[i].y, v[i].z );
				if ( normals && normalPerVertex ) glNormal3f( n[j].x, n[j].y, n[j].z );
				glVertex3f( v[j].x, v[j].y, v[j].z );
				}

			glEnd();		
			glEndList();
			}
		}
}

void VRMLObject::Display( void )				// output data arrays into OpenGL display list
{
	unsigned int h, i, j, k;
	Point *v;
	Line *l;
	Face *f;
	Point *n;
	GLfloat s;
	GLfloat acolor[4], dcolor[4], ecolor[4], scolor[4], black[4];	// setup color vectors for openGL materials
	
	acolor[0] = (GLfloat)(ambientIntensity*diffuseColor.r); acolor[1] = (GLfloat)(ambientIntensity*diffuseColor.g);
	acolor[2] = (GLfloat)(ambientIntensity*diffuseColor.b); acolor[3] = (GLfloat)(1.0-transparency);
	dcolor[0] = (GLfloat)diffuseColor.r; dcolor[1] = (GLfloat)diffuseColor.g; dcolor[2] = (GLfloat)diffuseColor.b; dcolor[3] = (GLfloat)(1.0-transparency);
	ecolor[0] = (GLfloat)emissiveColor.r; ecolor[1] = (GLfloat)emissiveColor.g; ecolor[2] = (GLfloat)emissiveColor.b; ecolor[3] = (GLfloat)(1.0-transparency);
	scolor[0] = (GLfloat)specularColor.r; scolor[1] = (GLfloat)specularColor.g; scolor[2] = (GLfloat)specularColor.b; scolor[3] = (GLfloat)(1.0-transparency);
	black[0] = 0.0; black[1] = 0.0; black[2] = 0.0; black[3] = 1.0;
	s = (GLfloat)(128.0*shininess);		// the VRML parameter is constrained to [0,1] while openGL is [0,128]

	if ( vertices )
	  if ( lines )
		{
		v = vertices->vertex;
		l = lines->line;
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ecolor );
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black );
		glBegin( GL_LINES );
	
		for ( k=0; k<lines->total; k++ )
			{
			i = l[k].v1;
			j = l[k].v2;
			glVertex3f( v[i].x, v[i].y, v[i].z );
			glVertex3f( v[j].x, v[j].y, v[j].z );
			}
	
		glEnd();		
		}
	  else if ( faces )
		{
		v = vertices->vertex;
		f = faces->face;
		if ( normals ) n = normals->vertex;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, acolor );
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dcolor );
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ecolor );
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, scolor );
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, s );
		if ( frontFill ) glPolygonMode( GL_FRONT, GL_FILL );
		else glPolygonMode( GL_FRONT, GL_LINE );
		if ( backFill ) glPolygonMode( GL_BACK, GL_FILL );
		else glPolygonMode( GL_BACK, GL_LINE );

		glBegin( GL_TRIANGLES );

		for ( k=0; k<faces->total; k++ )
			{
			h = f[k].v1;		// h is the index of v1 in the vertex list
			i = f[k].v2;		// so its also the index of v1's normal vector
			j = f[k].v3;		// k is index of face's normal
			if ( normals && !normalPerVertex ) glNormal3f( n[k].x, n[k].y, n[k].z );
			if ( normals && normalPerVertex ) glNormal3f( n[h].x, n[h].y, n[h].z );
			glVertex3f( v[h].x, v[h].y, v[h].z );
			if ( normals && normalPerVertex ) glNormal3f( n[i].x, n[i].y, n[i].z );
			glVertex3f( v[i].x, v[i].y, v[i].z );
			if ( normals && normalPerVertex ) glNormal3f( n[j].x, n[j].y, n[j].z );
			glVertex3f( v[j].x, v[j].y, v[j].z );
			}

		glEnd();		
		}
}

bool VRMLObject::WriteVRML2( HANDLE hFile )
{
	unsigned int i, j, k;
	Point *v;
	Line *l;
	Face *f;
	DWORD byteswritten;
	bool NoErrorOccurred = true;
	char line[MAX_PATH];										// begin Shape, write appearance node

	sprintf(line,"\tShape\r\n\t {\r\n\t appearance Appearance\r\n\t  {\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

	sprintf(line,"\t  material Material\r\n\t\t{\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\tdiffuseColor %g %g %g\r\n",diffuseColor.r,diffuseColor.g,diffuseColor.b);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\temissiveColor %g %g %g\r\n",emissiveColor.r,emissiveColor.g,emissiveColor.b);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\tspecularColor %g %g %g\r\n",specularColor.r,specularColor.g,specularColor.b);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\tshininess %g\r\n",shininess);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\tambientIntensity %g\r\n",ambientIntensity);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\ttransparency %g\r\n",transparency);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

	sprintf(line,"\t\t}\r\n\t }\r\n");									// close material and appearance node
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	
	if ( vertices )									// begin geometry
	  if ( lines )
		{
		sprintf(line,"\t geometry IndexedLineSet\r\n\t  {\r\n\t  coord Coordinate { point [\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

		v = vertices->vertex;									// output vertices of object
		for ( i=0; i<vertices->total; i++ )
			{
			sprintf(line,"\t\t%1.*g %1.*g %1.*g,\r\n",Precision,v[i].x,Precision,v[i].y,Precision,v[i].z);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}
																// begin lines or faces of object
		sprintf(line,"\t\t] }\r\n\t  coordIndex [\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

		l = lines->line;
		for ( k=0; k<lines->total; k++ )						// output each line
			{
			sprintf( line,"\t\t%d %d -1,\r\n", l[k].v1, l[k].v2 );
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}
																// end geometry for lines
		sprintf(line,"\t\t]\r\n\t  }\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
		}	// end if lines

	  else if ( faces )											// begin geometry for faces
		{
		sprintf(line,"\t geometry IndexedFaceSet\r\n\t  {\r\n\t  coord Coordinate { point [\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

		v = vertices->vertex;									// output vertices of object
		for ( i=0; i<vertices->total; i++ )
			{
			sprintf(line,"\t\t%1.*g %1.*g %1.*g,\r\n",Precision,v[i].x,Precision,v[i].y,Precision,v[i].z);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}
																// begin lines or faces of object
		sprintf(line,"\t\t] }\r\n\t  coordIndex [\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

		f = faces->face;
		for ( k=0; k<faces->total; k++ )						// output each line
			{
			sprintf( line,"\t\t%d %d %d -1,\r\n", f[k].v1, f[k].v2, f[k].v3 );
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}
																// end face indices
		sprintf(line,"\t\t]\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

		if ( normals )		// output normals if they were created
			{
			sprintf(line,"\t  normal Normal { vector [\r\n");
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

			v = normals->vertex;
			for ( k=0; k<normals->total; k++ )		// output all the normal vectors
				{
				sprintf( line,"\t\t%1.*g %1.*g %1.*g,\r\n", Precision,v[k].x,Precision,v[k].y,Precision,v[k].z );
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
				}

			if ( normalPerVertex )					// if perVertex output 3 Indices per face
				{
				sprintf(line,"\t\t] }\r\n\t  normalPerVertex TRUE\r\n\t  normalIndex [\r\n");
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
				for ( k=0; k<faces->total; k++ )
					{		// the index of v1 in the vertex list is also the index of v1's normal vector
					sprintf( line,"\t\t%d %d %d -1,\r\n", f[k].v1, f[k].v2, f[k].v3 );
					if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
					}
				}									// if per face output 1 Index per face
			else {
				sprintf(line,"\t\t] }\r\n\t  normalPerVertex FALSE\r\n\t  normalIndex [\r\n");
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
				for ( k=0; k<normals->total; k++ )
					{								// k is index of face's normal
					sprintf( line,"\t\t%d\r\n", k );
					if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
					}
				}

			sprintf(line,"\t\t]\r\n");
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			} // end if normals
																	// end the indexedFaceSet geometry node
		sprintf(line,"\t  }\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
		}	// end if faces
	
	sprintf(line,"\t }\r\n");									// close Shape
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

	return NoErrorOccurred;
}

bool VRMLObject::WriteVRML1( HANDLE hFile )
{
	unsigned int i, j, k;
	Point *v;
	Line *l;
	Face *f;
	DWORD byteswritten;
	bool NoErrorOccurred = true;
	char line[MAX_PATH];										// write shape hints

	sprintf(line,"\tShapeHints\r\n\t {\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t vertexOrdering COUNTERCLOCKWISE\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t shapeType UNKNOWN_SHAPE_TYPE\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t faceType CONVEX\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t creaseAngle 2.0\r\n\t }\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

	if ( normalPerVertex )										// specify normal type used
		sprintf(line,"\tNormalBinding { value PER_VERTEX }\r\n");
	else sprintf(line,"\tNormalBinding { value PER_FACE }\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

	sprintf(line,"\tSeparator\r\n\t {\r\n");					// now the shape itself
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

	sprintf(line,"\t Material\r\n\t\t{\r\n");					// Material node
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\tambientColor %g %g %g\r\n",ambientIntensity*diffuseColor.r,ambientIntensity*diffuseColor.g,ambientIntensity*diffuseColor.b);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\tdiffuseColor %g %g %g\r\n",diffuseColor.r,diffuseColor.g,diffuseColor.b);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\temissiveColor %g %g %g\r\n",emissiveColor.r,emissiveColor.g,emissiveColor.b);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\tspecularColor %g %g %g\r\n",specularColor.r,specularColor.g,specularColor.b);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\tshininess %g\r\n",shininess);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\ttransparency %g\r\n",transparency);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	sprintf(line,"\t\t}\r\n");									// close Material 
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
	
	if ( vertices )												// begin shape geometry woth vertices
		{
		sprintf(line,"\t Coordinate3\r\n\t { point [\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

		v = vertices->vertex;									// output vertices of object
		for ( i=0; i<vertices->total; i++ )
			{
			sprintf(line,"\t\t%1.*g %1.*g %1.*g,\r\n",Precision,v[i].x,Precision,v[i].y,Precision,v[i].z);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}

		sprintf(line,"\t ] }\r\n\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
		}														// end vertices

	if ( normals )												// output normals if they were created
		{
		sprintf(line, "\tNormal\r\n\t { vector [\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

		v = normals->vertex;
		for ( k=0; k<normals->total; k++ )						// output all the normal vectors
			{
			sprintf( line,"\t\t%1.*g %1.*g %1.*g,\r\n",Precision,v[k].x,Precision,v[k].y,Precision,v[k].z );
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}

		sprintf(line,"\t\t] }\r\n\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
		}														// end normals

																// begin lines or faces of shape
	sprintf(line,"\tTransformSeparator\r\n\t {\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

	if ( lines )												// begin IndexedLineSet
		{
		sprintf(line, "\t  IndexedLineSet\r\n\t\t{\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
		sprintf(line, "\t\tcoordIndex [\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

		l = lines->line;
		for ( k=0; k<lines->total; k++ )						// output each line
			{
			sprintf( line,"\t\t%d, %d, -1,\r\n", l[k].v1, l[k].v2 );
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}
																// end of IndexedLineSet
		sprintf(line,"\t\t]\r\n\t  }\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
		}
	else if ( faces )											// begin IndexedFaceSet
		{
		sprintf(line, "\t  IndexedFaceSet\r\n\t\t{\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
		sprintf(line, "\t\tcoordIndex [\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

		f = faces->face;
		for ( k=0; k<faces->total; k++ )						// output each line
			{
			sprintf( line,"\t\t%d, %d, %d, -1,\r\n", f[k].v1, f[k].v2, f[k].v3 );
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}
																// end face indices
		sprintf(line,"\t\t]\r\n");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;														

		if ( normals )										// finally index faces or vertices with normals
			{
			sprintf(line, "\t\tnormalIndex [\r\n");
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

			if ( normalPerVertex )								// if perVertex output 3 Indices per face
				{
				for ( k=0; k<faces->total; k++ )
					{		// the index of v1 in the vertex list is also the index of v1's normal vector
					sprintf( line,"\t\t%d, %d, %d, -1,\r\n", f[k].v1, f[k].v2, f[k].v3 );
					if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
					}
				}												// if per face output 1 Index per face
			else {
				for ( k=0; k<normals->total; k++ )
					{											// k is index of face's normal
					sprintf( line,"\t\t%d,\r\n", k );
					if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
					}
				}
																// end normal indices
			sprintf(line,"\t\t ]\r\n");
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}

		sprintf(line,"\t\t}\r\n");								// close the IndexedFaceSet
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
		}

	sprintf(line,"\t }\r\n\t}\r\n");								// close Transform and Separator
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;

	return NoErrorOccurred;
}

bool VRMLObject::WriteDXF( HANDLE hFile )
{
	unsigned int i, k;
	Point *v;
	Line *l;
	Face *f;
	DWORD byteswritten;
	bool NoErrorOccurred = true;
	char line[MAX_PATH];											

	if ( lines )											// begin line entities
		{
		v = vertices->vertex;
		l = lines->line;
		for ( k=0; k<lines->total; k++ )						// output each line
			{
			sprintf(line, "0\r\nLINE\r\n8\r\n%s\r\n",this->name);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			i = l[k].v1;										// vertex 1
			sprintf(line,"10\r\n%1.*g\r\n20\r\n%1.*g\r\n30\r\n%1.*g\r\n",Precision,v[i].x,Precision,v[i].y,Precision,v[i].z);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			i = l[k].v2;										// vertex 2
			sprintf(line,"11\r\n%1.*g\r\n21\r\n%1.*g\r\n31\r\n%1.*g\r\n",Precision,v[i].x,Precision,v[i].y,Precision,v[i].z);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}
		}													// end line output

	else if ( faces )										// begin face entities
		{
		v = vertices->vertex;
		f = faces->face;
		for ( k=0; k<faces->total; k++ )						// output each triangular face
			{
			sprintf(line, "0\r\n3DFACE\r\n8\r\n%s\r\n",this->name);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			i = f[k].v1;										// vertex 1
			sprintf(line,"10\r\n%1.*g\r\n20\r\n%1.*g\r\n30\r\n%1.*g\r\n",Precision,v[i].x,Precision,v[i].y,Precision,v[i].z);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			i = f[k].v2;										// vertex 2
			sprintf(line,"11\r\n%1.*g\r\n21\r\n%1.*g\r\n31\r\n%1.*g\r\n",Precision,v[i].x,Precision,v[i].y,Precision,v[i].z);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			i = f[k].v3;										// write vertex 3 twice
			sprintf(line,"12\r\n%1.*g\r\n22\r\n%1.*g\r\n32\r\n%1.*g\r\n",Precision,v[i].x,Precision,v[i].y,Precision,v[i].z);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			sprintf(line,"13\r\n%1.*g\r\n23\r\n%1.*g\r\n33\r\n%1.*g\r\n",Precision,v[i].x,Precision,v[i].y,Precision,v[i].z);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) NoErrorOccurred = false;
			}
															// end face output
		}

	return NoErrorOccurred;
}

