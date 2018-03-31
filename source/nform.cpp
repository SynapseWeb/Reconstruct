//////////////////////////////////////////////////////////////////////////////
// methods for nonlinear coordinate transformations
//
//    Copyright (C) 1999-2006  John Fiala (fiala@bu.edu)
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
// modified 11/16/04 by JCF (fiala@bu.edu)
// -+- change: Modified ComputeRigid() to use more accurate atan2() function.
// modified 4/20/06 by JCF (fiala@bu.edu)
// -+- change: Put check for atan2(0,0) in ComputeRigid to prevent domain error.
//

#include "reconstruct.h"


Nform::Nform()
{
	Clear();
}

Nform::Nform ( Nform &copyfrom )								// copy constructor
{
	dim = copyfrom.dim;
	for ( int i = 0; i < DIM; i++ )
		{
		a[i] = copyfrom.a[i];
		b[i] = copyfrom.b[i];
		}
}

void Nform::Clear()
{
	for (int i=0; i<DIM; i++) {
   		a[i] = 0.0;
		b[i] = 0.0;
		}
   a[1] = 1.0;
   b[2] = 1.0;
   dim = 0;				// the identity Nform requires no computations
}

double Nform::X( double x, double y )
{
	double result;
	switch (dim) {
	case 1:								 // translation only
		result = a[0] + x;
		break;
	case 2:
		result = a[0] + a[1]*x;
		break;
	case 3:								 // affine transformation
		result = a[0] + a[1]*x + a[2]*y;
		break;
	case 4:
		result = a[0] + (a[1] + a[3]*y)*x + a[2]*y;
		break;
	case 5:
		result = a[0] + (a[1] + a[3]*y + a[4]*x)*x + a[2]*y;
		break;
	case 6:
		result = a[0] + (a[1] + a[3]*y + a[4]*x)*x + (a[2] + a[5]*y)*y;
		break;
	default:
		result = x;						 // identity
	}
	return result;
}

double Nform::Y( double x, double y )
{
	double result;
	switch (dim) {
	case 1:								 // translation only
		result = b[0] + y;
		break;
	case 2:
		result = b[0] + b[1]*y;
		break;
	case 3:								 // affine transformation
		result = b[0] + b[1]*x + b[2]*y;
		break;
	case 4:
		result = b[0] + (b[1] + b[3]*y)*x + b[2]*y;
		break;
	case 5:
		result = b[0] + (b[1] + b[3]*y + b[4]*x)*x + b[2]*y;
		break;
	case 6:
		result = b[0] + (b[1] + b[3]*y + b[4]*x)*x + (b[2] + b[5]*y)*y;
		break;
	default:
		result = y;						 // identity
	}
   return result;
}

void Nform::XYinverse( double *x, double *y )
{
	int i;
	double epsilon = 5e-10;
	double e,l,m,n,o,p,x0,y0,u0,v0,u,v;

	switch (dim) {
	case 0:	
		break;							// no change when identity Nform
	case 1:
		*x = *x - a[0];
		*y = *y - b[0];
		break;
	case 2:
	case 3:
		u = *x - a[0];
		v = *y - b[0];
		p = a[1]*b[2] - a[2]*b[1];
		if ( fabs(p) > epsilon ) {
			*x = (b[2]*u - a[2]*v)/p;       // inverse of rotational part
			*y = (a[1]*v - b[1]*u)/p;
			}
		break;
	
	case 4:
	case 5:
	case 6:
		u = *x;							// (u,v) for which we want (x,y)
		v = *y;
		x0 = 0.0;						// initial guess of (x,y)
		y0 = 0.0;
		u0 = X(x0,y0);					//	get forward tform of initial guess
		v0 = Y(x0,y0);
		i = 0;							// allow no more than 10 iterations
		e = 1.0;						// to reduce error to this limit
		while ( (e > epsilon) && (i<10) ) {
			i++;
			l = a[1] + a[3]*y0 + 2.0*a[4]*x0;	// compute Jacobian
			m = a[2] + a[3]*x0 + 2.0*a[5]*y0;
			n = b[1] + b[3]*y0 + 2.0*b[4]*x0;
			o = b[2] + b[3]*x0 + 2.0*b[5]*y0;
			p = l*o - m*n;						// determinant for inverse
			if ( fabs(p) > epsilon ) {
				x0 += (o*(u-u0) - m*(v-v0))/p;	// inverse of Jacobian
				y0 += (l*(v-v0) - n*(u-u0))/p;	// and use to increment (x0,y0)
				}
			else {
				x0 += l*(u-u0) + n*(v-v0);		// try Jacobian transpose instead
				y0 += m*(u-u0) + o*(v-v0);
				}
			u0 = X(x0,y0);						//	get forward tform of current guess
			v0 = Y(x0,y0);
			e = fabs(u-u0) + fabs(v-v0);		// compute closeness to goal
			}
		*x = x0;
		*y = y0;						// return final estimate of (x,y)
	}	// end switch
}

															// compute the x- and y-params for this nform that
															// will map array (fx,fy) into (rx,ry);
															// compute only first "d" number of terms

void Nform::ComputeMapping( double *fx, double *fy, double *rx, double *ry, int nopts, int d )
{
	int i, j, k;
	double **U, *w, **V, *ux;
	double s, scale, wmin, wmax;							// vectors have zero as first index

	U = new double*[nopts]; for (k=0;k<nopts;k++) U[k] = new double[d];
	V = new double*[d]; for (k=0;k<d;k++) V[k] = new double[d];
	w = new double[d];
	ux = new double[d];

	scale = 1.0;											// get scaling factor for matrix U
	for (i=0; i<nopts; i++)
		{
		if ( fx[i] > scale ) scale = fx[i];
		if ( fy[i] > scale ) scale = fy[i];
		}
															// compute U from points, d limits
	for (i=0; i<nopts; i++) {								// which terms of a,b are computed
		if ( d > 0 ) U[i][0] = 1.0;
		if ( d > 1 ) U[i][1] = fx[i]/scale;
		if ( d > 2 ) U[i][2] = fy[i]/scale;
		if ( d > 3 ) U[i][3] = fx[i]*fy[i]/(scale*scale);
		if ( d > 4 ) U[i][4] = fx[i]*fx[i]/(scale*scale);
		if ( d > 5 ) U[i][5] = fy[i]*fy[i]/(scale*scale);
		}

	svd( U, nopts, d, w, V );								// compute SVD of U


	wmax = 0.0;												// find max singular value
	for (i=0; i<d; i++) if (w[i] > wmax) wmax = w[i];
	wmin = wmax*1.0e-11;									// zero all small singular values
	for (i=0; i<d; i++) if (w[i] < wmin) w[i] = 0.0;

															//                  t
	for (i=0; i<d; i++) {									// solve U*diag(w)*V *a = rx
		s = 0.0;
		if ( w[i] ) {
			for (j=0; j<nopts; j++)		 					//      t
				s += U[j][i]*rx[j];							// s = U *rx
			s /= w[i];										// ux = diag(1/w)*s
			}
		ux[i] = s;
		}
	for (i=0; i<d; i++) {									// this->a = V*ux
		s = 0.0;
		for (j=0; j<d; j++) s += V[i][j]*ux[j];
		a[i] = s;
		}
															//                  t
	for (i=0; i<d; i++) {									// solve U*diag(w)*V *b = ry
		s = 0.0;
		if ( w[i] ) {
			for (j=0; j<nopts; j++)							//      t
				s += U[j][i]*ry[j];							// s = U *ry
			s /= w[i];										// ux = diag(1/w)*s
			}
		ux[i] = s;
		}
	for (i=0; i<d; i++) {									// this->b = V*ux
		s = 0.0;
		for (j=0; j<d; j++) s += V[i][j]*ux[j];
		b[i] = s;
		}
															// unscale the resulting params
	if ( d > 1 ) { a[1] /= scale; b[1] /= scale; }
	if ( d > 2 ) { a[2] /= scale; b[2] /= scale; }
	if ( d > 3 ) { a[3] /= scale*scale; b[3] /= scale*scale; }
	if ( d > 4 ) { a[4] /= scale*scale; b[4] /= scale*scale; }
	if ( d > 5 ) { a[5] /= scale*scale; b[5] /= scale*scale; }

	for (i=d; i<DIM; i++) {									// set remaining parameters to zero!
		a[i] = 0.0;
		b[i] = 0.0;
		}
	dim = d;												// remember only need first d parameters
		
															// free working memory
	delete[] ux;
	delete[] w;
	for (k=0;k<d;k++) delete[] V[k]; delete[] V;
	for (k=0;k<nopts;k++) delete[] U[k]; delete[] U;
}

											// determine approximate average rigid-body
											// nform to align all points
                                          
void Nform::ComputeRigid( double *x, double *y, double *u, double *v, int nopts )
{
	int i;
	double a0, b0, c, s, n;
	double *px, *py, *qx, *qy;
	
											// p,q will be difference vectors
   px = new double[nopts];
   py = new double[nopts];
   qx = new double[nopts];
   qy = new double[nopts];

	for (i=0; i<nopts-1; i++) {				// compute difference vectors
		px[i] = x[i] - x[i+1];
		py[i] = y[i] - y[i+1];
		qx[i] = u[i] - u[i+1];
		qy[i] = v[i] - v[i+1];
		}
	px[i] = x[i] - x[0];					// i = nopts-1
	py[i] = y[i] - y[0];
	qx[i] = u[i] - u[0];
	qy[i] = v[i] - v[0];

	n = 0.0;								// acummulate estimates of rotation angle
	for (i=0; i<nopts; i++)
		if ( (fabs(px[i]*qy[i]-qx[i]*py[i]) < MIN_FLOAT) && (fabs(px[i]*qx[i] + py[i]*qy[i]) < MIN_FLOAT) ) ;
		else n += atan2(px[i]*qy[i]-qx[i]*py[i],px[i]*qx[i] + py[i]*qy[i]); // both x and y can't be zero

	n = n/((double)nopts);					// get average rotation
	c = cos( n );
	s = sin( n );							// calculate corresponding sine and cosine

	a0 = 0.0;
	b0 = 0.0;
	for (i=0; i<nopts; i++) {				// compute average translations
		a0 += u[i] - c*x[i] + s*y[i];
		b0 += v[i] - s*x[i] - c*y[i];
		}
	a0 = a0/((double)nopts);				// get average x translation
	b0 = b0/((double)nopts);				// get average y translation

											// put result in output arrays
	a[0] = a0;
	a[1] = c;
	a[2] = -s;
	b[0] = b0;
	b[1] = s;
	b[2] = c;
	for (i=3; i<DIM; i++) {					// only first 3 terms are significant
		a[i] = 0.0;
		b[i] = 0.0;
		}
	dim = 3;
}


											// estimate inverse of this Nform in same form
Nform * Nform::Inverse( double w, double h )
{
	int i;
	double fx[DIM], fy[DIM], rx[DIM], ry[DIM];
	Nform *r;

	r = new Nform();
   											// create spanning set of image
	fx[0] = 0.0; fy[0] = 0.0;
	fx[1] = w; fy[1] = 0.0;
	fx[2] = 0.0; fy[2] = h;
	fx[3] = w; fy[3] = h;
	fx[4] = w; fy[4] = h/2.0;
	fx[5] = w/2.0; fy[5] = h;

	for (i=0; i<DIM; i++) {					// map them through this Nform
		rx[i] = X( fx[i], fy[i] );
		ry[i] = Y( fx[i], fy[i] );
		}
   											// determine the inverse mapping

	r->ComputeMapping( rx, ry, fx, fy, DIM, dim );

	return r;
}
											// make current Nform the composition of t1 and t2

void Nform::Compose( Nform *t1, Nform *t2, double w, double h )
{
	int i;
	double x[DIM], y[DIM], u[DIM], v[DIM], fx[DIM], fy[DIM];

   											// create spanning set of image
	fx[0] = 0.0; fy[0] = 0.0;
	fx[1] = w; fy[1] = 0.0;
	fx[2] = 0.0; fy[2] = h;
	fx[3] = w; fy[3] = h;
	fx[4] = w; fy[4] = h/2.0;
	fx[5] = w/2.0; fy[5] = h;

	for (i=0; i<DIM; i++) {					// map them through t1
		u[i] = t1->X( fx[i], fy[i] );
		v[i] = t1->Y( fx[i], fy[i] );
		}

	for (i=0; i<DIM; i++) {					// map them through t2
		x[i] = t2->X( u[i], v[i] );
		y[i] = t2->Y( u[i], v[i] );
		}
   											// determine the mapping
	ComputeMapping( fx, fy, x, y, DIM, DIM );
}

											// remove t1 from Nform t2

void Nform::Decompose( Nform *t1, Nform *t2, double w, double h )
{
	int i;
	double x[DIM], y[DIM], u[DIM], v[DIM], fu, fv, fx[DIM], fy[DIM];

   											// create spanning set of image
	fx[0] = 0.0; fy[0] = 0.0;
	fx[1] = w; fy[1] = 0.0;
	fx[2] = 0.0; fy[2] = h;
	fx[3] = w; fy[3] = h;
	fx[4] = w; fy[4] = h/2.0;
	fx[5] = w/2.0; fy[5] = h;

	for (i=0; i<DIM; i++) {					// map them through thr inverse of t1
		fu = fx[i];  fv = fy[i];
		t1->XYinverse( &fu, &fv );			// invert in place
		u[i] = fu;
		v[i] = fv;
		}

	for (i=0; i<DIM; i++) {					// map them through t2
		x[i] = t2->X( u[i], v[i] );
		y[i] = t2->Y( u[i], v[i] );
		}
   											// determine the mapping
	ComputeMapping( fx, fy, x, y, DIM, DIM );
}

void Nform::ApplyLinearOf( Nform *p )		// apply linear part of p to this Nform
{
	double na[DIM], nb[DIM];
	double A,B,C,D,E,F;						// get linear components of p

	A = p->a[1];
	B = p->a[2];
	C = p->b[1];
	D = p->b[2];
	E = p->a[0];
	F = p->b[0];
											// Apply p Nform first, then this Nform...
											//
											//  result = | a0 a1 ... an | | f0(u,v) |
											//           | b0 b1 ... bn |*|   ...   |
											//                            | fn(u,v) |
											//	where (u,v) = | A B E |
											//                | C D F |*(x,y)
	na[5] = a[3]*B*D + a[4]*B*B + a[5]*D*D;
	nb[5] = b[3]*B*D + b[4]*B*B + b[5]*D*D;

	na[4] = a[3]*A*C + a[4]*A*A + a[5]*C*C;
	nb[4] = b[3]*A*C + b[4]*A*A + b[5]*C*C;

	na[3] = a[3]*(A*D + B*C) + 2.0*a[4]*A*B + 2.0*a[5]*C*D;
	nb[3] = b[3]*(A*D + B*C) + 2.0*b[4]*A*B + 2.0*b[5]*C*D;
	
	na[2] = a[1]*B + a[2]*D + a[3]*(B*F + D*E) + 2.0*a[4]*B*E + 2.0*a[5]*D*F;
	nb[2] = b[1]*B + b[2]*D + b[3]*(B*F + D*E) + 2.0*b[4]*B*E + 2.0*b[5]*D*F;

	na[1] = a[1]*A + a[2]*C + a[3]*(A*F + C*E) + 2.0*a[4]*A*E + 2.0*a[5]*C*F;
	nb[1] = b[1]*A + b[2]*C + b[3]*(A*F + C*E) + 2.0*b[4]*A*E + 2.0*b[5]*C*F;
	
	na[0] = a[0] + a[1]*E + a[2]*F + a[3]*E*F + a[4]*E*E + a[5]*F*F;
	nb[0] = b[0] + b[1]*E + b[2]*F + b[3]*E*F + b[4]*E*E + b[5]*F*F;


	for (int i=0; i<DIM; i++) {				// put result into this Nform
		a[i] = na[i];
		b[i] = nb[i];
		}
	dim = DIM;								// TEST FOR SIGNIFICANCE
}

void Nform::ApplyInverseOf( double *aa, double *ab )		// apply linear inverse to this Nform
{
	double det, na[DIM], nb[DIM];
	double A,B,C,D,E,F;						// compute inverse of matrix form of aa,ab

	det = aa[1]*ab[2] - aa[2]*ab[1];
	A = ab[2]/det;
	B = -aa[2]/det;
	C = -ab[1]/det;
	D = aa[1]/det;
	E = (aa[2]*ab[0]-ab[2]*aa[0])/det;
	F = (ab[1]*aa[0]-aa[1]*ab[0])/det;
											// Premultiply by this inverse to modify Nform
											//
											//  result = | a0 a1 ... an | | f0(u,v) |
											//           | b0 b1 ... bn |*|   ...   |
											//                            | fn(u,v) |
											//	where (u,v) = | A B E |
											//                | C D F |*(x,y)

	na[0] = a[0] + a[1]*E + a[2]*F + a[3]*E*F + a[4]*E*E + a[5]*F*F;
	nb[0] = b[0] + b[1]*E + b[2]*F + b[3]*E*F + b[4]*E*E + b[5]*F*F;

	na[1] = a[1]*A + a[2]*C + a[3]*(A*F + C*E) + 2.0*a[4]*A*E + 2.0*a[5]*C*F;
	nb[1] = b[1]*A + b[2]*C + b[3]*(A*F + C*E) + 2.0*b[4]*A*E + 2.0*b[5]*C*F;

	na[2] = a[1]*B + a[2]*D + a[3]*(B*F + D*E) + 2.0*a[4]*B*E + 2.0*a[5]*D*F;
	nb[2] = b[1]*B + b[2]*D + b[3]*(B*F + D*E) + 2.0*b[4]*B*E + 2.0*b[5]*D*F;

	na[3] = a[3]*(A*D + B*C) + 2.0*a[4]*A*B + 2.0*a[5]*C*D;
	nb[3] = b[3]*(A*D + B*C) + 2.0*b[4]*A*B + 2.0*b[5]*C*D;

	na[4] = a[3]*A*C + a[4]*A*A + a[5]*C*C;
	nb[4] = b[3]*A*C + b[4]*A*A + b[5]*C*C;

	na[5] = a[3]*B*D + a[4]*B*B + a[5]*D*D;
	nb[5] = b[3]*B*D + b[4]*B*B + b[5]*D*D;

	for (int i=0; i<DIM; i++) {						// put result into this Nform
		a[i] = na[i];
		b[i] = nb[i];
		}
	dim = DIM;									// TEST FOR SIGNIFICANCE
}


void Nform::PreApply( Mvmt mvmt )
{
	int i;													// apply to left of current Nform
	double cosine, sine, a1, b1, c1, d1, a, b, c, d, e, f;	// ASSUMES: quadratic binomial basis
	double aa[DIM], bb[DIM];

	cosine = cos(mvmt.theta);				// ASSUMES: scaleX, scaleY is not zero!
	sine = sin(mvmt.theta);					// | C  S  0 | | 1/Sx  0    0  |
	a1 = cosine/mvmt.scaleX;				// |-S  C  0 |*|  0   1/Sy  0  |
	b1 = sine/mvmt.scaleY;					// | 0  0  1 | |  0    0    1  |
	c1 = -sine/mvmt.scaleX;
	d1 = cosine/mvmt.scaleY;
	a = a1 + mvmt.slantX*c1;					// | 1  Shx 0 | | a1 b1 0 |
	b = b1 + mvmt.slantX*d1;					// | Shy 1  0 |*| c1 d1 0 |
	c = c1 + mvmt.slantY*a1;					// | 0   0  1 | | 0  0  1 |
	d = d1 + mvmt.slantY*b1;
										// scale,rotation and slant wrt center

	e = -(mvmt.centerX+mvmt.transX)*a - (mvmt.centerY+mvmt.transY)*b + mvmt.centerX;
	f = -(mvmt.centerX+mvmt.transX)*c - (mvmt.centerY+mvmt.transY)*d + mvmt.centerY;

										//  result = | a b || a0 a1 ... an | | f0(x,y) |   | e |
										//           | c d || b0 b1 ... bn |*|   ...   | + | f |
										//                                   | fn(x,y) |

	aa[0] = this->a[0]*a + this->b[0]*b + e;
	bb[0] = this->a[0]*c + this->b[0]*d + f;
   
	aa[1] = this->a[1]*a + this->b[1]*b;
	bb[1] = this->a[1]*c + this->b[1]*d;

	aa[2] = this->a[2]*a + this->b[2]*b;
	bb[2] = this->a[2]*c + this->b[2]*d;

	aa[3] = this->a[3]*a + this->b[3]*b + mvmt.deformX;
	bb[3] = this->a[3]*c + this->b[3]*d + mvmt.deformY;

	aa[4] = this->a[4]*a + this->b[4]*b;
	bb[4] = this->a[4]*c + this->b[4]*d;

	aa[5] = this->a[5]*a + this->b[5]*b;
	bb[5] = this->a[5]*c + this->b[5]*d;

	for ( i=0; i<DIM; i++ )
		{
		this->a[i] = aa[i];
		this->b[i] = bb[i];
		if ( (this->a[i] != 0.0) || (this->b[i] != 0.0) ) this->dim = i+1;
		}
}

												
void Nform::PostApply( Mvmt mvmt )
{
	int i;													// apply to right of current Nform
	double cosine, sine, a1, b1, c1, d1, a, b, c, d, e, f;	// ASSUMES quadratic binomial basis!
	double aa[DIM], bb[DIM];

	cosine = cos(mvmt.theta);				// ASSUMES: scaleX, scaleY is not zero!
	sine = sin(mvmt.theta);					// | C  S  0 | | 1/Sx  0    0  |
	a1 = cosine/mvmt.scaleX;				// |-S  C  0 |*|  0   1/Sy  0  |
	b1 = sine/mvmt.scaleY;					// | 0  0  1 | |  0    0    1  |
	c1 = -sine/mvmt.scaleX;
	d1 = cosine/mvmt.scaleY;
	a = a1 + mvmt.slantX*c1;					// | 1  Shx 0 | | a1 b1 0 |
	b = b1 + mvmt.slantX*d1;					// | Shy 1  0 |*| c1 d1 0 |
	c = c1 + mvmt.slantY*a1;					// | 0   0  1 | | 0  0  1 |
	d = d1 + mvmt.slantY*b1;
										// scale,rotation and slant wrt center

	e = -(mvmt.centerX+mvmt.transX)*a - (mvmt.centerY+mvmt.transY)*b + mvmt.centerX;
	f = -(mvmt.centerX+mvmt.transX)*c - (mvmt.centerY+mvmt.transY)*d + mvmt.centerY;

										//  result = | a0 a1 ... an | | f0(u,v) |
										//           | b0 b1 ... bn |*|   ...   |
										//                            | fn(u,v) |
										//	where (u,v) = | a b e |
										//                | c d f |*(x,y)

	this->a[3] += mvmt.deformX;	// IS THIS CORRECT?
	this->b[3] += mvmt.deformY;

	aa[0] = this->a[0] + this->a[1]*e + this->a[2]*f
   					 + this->a[3]*e*f + this->a[4]*e*e + this->a[5]*f*f;
	bb[0] = this->b[0] + this->b[1]*e + this->b[2]*f
   					 + this->b[3]*e*f + this->b[4]*e*e + this->b[5]*f*f;
   
	aa[1] = this->a[1]*a + this->a[2]*c
   					 + this->a[3]*(a*f + c*e) + 2.0*this->a[4]*a*e + 2.0*this->a[5]*c*f;
	bb[1] = this->b[1]*a + this->b[2]*c
   					 + this->b[3]*(a*f + c*e) + 2.0*this->b[4]*a*e + 2.0*this->b[5]*c*f;

	aa[2] = this->a[1]*b + this->a[2]*d
   					 + this->a[3]*(b*f + d*e) + 2.0*this->a[4]*b*e + 2.0*this->a[5]*d*f;
	bb[2] = this->b[1]*b + this->b[2]*d
   					 + this->b[3]*(b*f + d*e) + 2.0*this->b[4]*b*e + 2.0*this->b[5]*d*f;

	aa[3] = this->a[3]*(a*d + b*c) + 2.0*this->a[4]*a*b + 2.0*this->a[5]*c*d;
	bb[3] = this->b[3]*(a*d + b*c) + 2.0*this->b[4]*a*b + 2.0*this->b[5]*c*d;

	aa[4] = this->a[3]*a*c + this->a[4]*a*a + this->a[5]*c*c;
	bb[4] = this->b[3]*a*c + this->b[4]*a*a + this->b[5]*c*c;

	aa[5] = this->a[3]*b*d + this->a[4]*b*b + this->a[5]*d*d;
	bb[5] = this->b[3]*b*d + this->b[4]*b*b + this->b[5]*d*d;

	for ( i=0; i<DIM; i++ )
		{
		this->a[i] = aa[i];
		this->b[i] = bb[i];
		if ( i == 1 ) { if ( (this->a[i] != 1.0) || (this->b[i] != 0.0) ) this->dim = 3; }
		else if ( i == 2 ) { if ( (this->a[i] != 0.0) || (this->b[i] != 1.0) ) this->dim = 3; }
		else if ( (this->a[i] != 0.0) || (this->b[i] != 0.0) ) this->dim = i+1;
		}
}

void Nform::SetToDifference( Nform *t1, Nform *t2 )		// set this to difference t1 - t2
{
	for (int i=0; i<DIM; i++)
		{
		a[i] = t1->a[i]-t2->a[i];
		b[i] = t1->b[i]-t2->b[i];
		}
	a[1] += 1.0;				// preserve identity nform
	b[2] += 1.0;
	dim = t1->dim;				// use largest dim (TEST?)
	if ( dim < t2->dim ) dim = t2->dim;
}
