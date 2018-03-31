////////////////////////////////////////////////////////////////////////
//	This file contains the methods for the VRMLScene class
//
//    Copyright (C) 2003-2006  John Fiala (fiala@bu.edu)
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License version 2 as published by
//    the Free Software Foundation, 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
// 
// modified 7/8/05 by JCF (fiala@bu.edu)
// -+- change: Modified Reset() to recalculate the scene dimensions based on the objects present.
// modified 7/13/05 by JCF (fiala@bu.edu)
// -+- change: Fixed bug introduced by 7/8/05 modification. Removed unneeded min, max calculations from Add().
// modified 10/3/05 by JCF (fiala@bu.edu)
// -+- change: Added DisplayUncompiled to display in OpenGL directly without call lists.
// modified 4/28/06 by JCF (fiala@bu.edu)
// -+- change: Modified behavior of reset to allow orthographic display mode in opengl.cpp.
// modified 6/22/06 by JCF (fiala@bu.edu)
// -+- change: Fixed missing { after Separator in VRML 1.0 output.
//

#include "reconstruct.h"

VRMLScene::VRMLScene()
{
	objects = NULL;
	background.r = 1.0;											// start with white background
	background.g = 1.0;
	background.b = 1.0;
	min.x = 0.0;  min.y = 0.0;  min.z = 0.0;					// the remaining params will be set to reasonable
	max.x = 1.0; max.y = 1.0; max.z = 1.0;						// values as objects are added to the seen
	fovLeft = fovBottom = -10.0;
	fovRight = fovTop = 10.0;									// these are OK assuming the scale of object is ~1 unit
	fov = 12.0*PI/180.0;										// 12 degrees	
	fovNear = 100.0; fovFar = 1000.0;							
	xCenter = 0.0; yCenter = 0.0; zCenter = 0.0;				// and is already centered at origin
	xTrans = 0.0; yTrans = 0.0; zTrans = 110.0;
	xRotInc = 0.0; yRotInc = 0.0;								// allow for no initial scene rotation
	for (int i=0; i<16; i++) m[i] = 0.0;
	m[0] = 1.0; m[5] = 1.0; m[10] = 1.0; m[15] = 1.0;			// model matrix is identity
}

VRMLScene::~VRMLScene()
{
	if ( objects ) delete objects;
}

void VRMLScene::Add( VRMLObject *object )				// put 3D Object into scene
{
	if ( !objects ) objects = new VRMLObjects();		// create object list
	objects->Add( object );								// add object to list 
}

void VRMLScene::ClearObjects( void )
{
	if ( objects ) delete objects;									// delete objects from scene
	objects = NULL;

	if ( IsWindow( objectWindow ) ) ResetSceneIcons( objectList );	// clear listview icons
	if ( IsWindow( zTraceWindow ) ) ResetSceneIcons( zTraceList );
}

void VRMLScene::Compile( void )				// compile each object in scene in openGL list
{											// NOTE: an openGL context must be active
	VRMLObject *object;

	if ( objects )
		{
		object = objects->first;		// firt draw opaque object with depth buffer enabled
		while ( object )
			{
			object->Compile();
			object = object->next;
			}
		}
}

void VRMLScene::Reset( void )
{											// set scene translation to center at origin
	double ax, ay, az, fovSize;
	VRMLObject *object;

	if ( objects )										// only adjust the scene volume
	  if ( objects->Number() )							// if there are objects in the scene
		{
		min.x = MAX_FLOAT;  min.y = MAX_FLOAT;  min.z = MAX_FLOAT;
		max.x = -MAX_FLOAT; max.y = -MAX_FLOAT; max.z = -MAX_FLOAT;
		object = objects->first;
		while ( object )								// recalculate the min, max of the scene
			{
			if ( min.x > object->min.x ) min.x = object->min.x;
			if ( max.x < object->max.x ) max.x = object->max.x;
			if ( min.y > object->min.y ) min.y = object->min.y;
			if ( max.y < object->max.y ) max.y = object->max.y;
			if ( min.z > object->min.z ) min.z = object->min.z;
			if ( max.z < object->max.z ) max.z = object->max.z;
			object = object->next;
			}

		ax = fabs(max.x-min.x);							// calculate size of scene volume
		ay = fabs(max.y-min.y);
		az = fabs(max.z-min.z);
		fovSize = sqrt(ax*ax+ay*ay+az*az);				// calc. length of main diagonal of scene
					
		fovBottom = -fovSize/2.0;						// set x,y dims to half of volume size
		fovTop = fovSize/2.0;
		fovLeft = -fovSize/2.0;
		fovRight = fovSize/2.0;							// calculate the distance from viewpoint to
		fovNear = fovTop/tan(fov/2.0);					// to give viewer's eye this field-of-view
		fovFar = fovNear + fovSize;						// allow scene to rotate without clipping
		
		zTrans = -fovNear - fovSize/2.0;				// move object into frustrum (relative to camera)
		xTrans = 0.0;									// no other translational offset
		yTrans = 0.0;
		xRotInc = 0.0;									// or rotational increments
		yRotInc = 0.0;

		xCenter = -(max.x+min.x)/2.0;
		yCenter = -(max.y+min.y)/2.0;					// set translation that will center scene at origin
		zCenter = -(max.z+min.z)/2.0;

		for (int i=0; i<16; i++) m[i] = 0.0;			// model matrix is identity
		m[0] = 1.0; m[5] = 1.0; m[10] = 1.0; m[15] = 1.0;
		}
}

void VRMLScene::Display( void )				// display each object in scene using openGL list
{											// NOTE: an openGL context must be active
	VRMLObject *object;
	GLuint list;

	if ( objects )
		{
		object = objects->first;
		while ( object )					// first display opaque objects with depth buffering
			{
			list = object->openGLList;
			if ( list && (object->transparency <= 0.0) ) glCallList( list );
			object = object->next;
			}
		glEnable(GL_BLEND);					// enable blending
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		object = objects->first;			// then draw tranparent objects with depth buffer read-only
		while ( object )
			{
			list = object->openGLList;
			if ( list && (object->transparency > 0.0) ) glCallList( list );
			object = object->next;
			}
		glDepthMask(GL_TRUE);				// reenable depth for next time
		glDisable(GL_BLEND);
		}
}

void VRMLScene::DisplayUncompiled( void )		// display each object in scene using openGL directly
{												// NOTE: an openGL context must be active
	VRMLObject *object;
	
	if ( objects )
		{
		object = objects->first;
		while ( object )					// first display opaque objects with depth buffering
			{
			if ( object->transparency <= 0.0 ) object->Display();
			object = object->next;
			}
		glEnable(GL_BLEND);					// enable blending
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		object = objects->first;			// then draw tranparent objects with depth buffer read-only
		while ( object )
			{
			if ( object->transparency > 0.0 ) object->Display();
			object = object->next;
			}
		glDepthMask(GL_TRUE);				// reenable depth for next time
		glDisable(GL_BLEND);
		}
}

											// get axis and angle of rotation matrix for VRML viewpoint

void VRMLScene::ComputeViewpoint( double &ax, double &ay, double &az, double &a, double &x, double &y, double &z )
{
	double s, trace, dx, dy, dz, l[16];

	l[0] = m[0];	l[4] = m[1];	l[8] = m[2];	// transpose rotation matrix to get inverse
	l[1] = m[4];	l[5] = m[5];	l[9] = m[6];
	l[2] = m[8];	l[6] = m[9];	l[10] = m[10];
	dx = -CurrScene->xTrans;
	dy = -CurrScene->yTrans;						// postmultiply by -offset translation
	dz = -CurrScene->zTrans;
	x = l[0]*dx + l[4]*dy + l[8]*dz;
	y = l[1]*dx + l[5]*dy + l[9]*dz;
	z = l[2]*dx + l[6]*dy + l[10]*dz;
	x -= CurrScene->xCenter;
	y -= CurrScene->yCenter;						// premultiply by -center translation
	z -= CurrScene->zCenter;

	trace = l[0] + l[5] + l[10] - 1.0;		// use trace of rotation matrix:
	if ( fabs(trace) <= 2.0 )				//     |  l0   l4   l8  |
		a = acos(trace/2.0);				// L = |  l1   l5   l9  |
	else a = 0.0;							//     |  l2   l6   l10 |
											// to compute rotation angle
	dx = l[6] - l[9];
	dy = l[8] - l[2];
	dz = l[1] - l[4];						// compute rotation axis from L - L^t

	s = sqrt( dx*dx + dy*dy + dz*dz );		// normalize axis vector

	if ( s == 0.0 )		// but if not possible, just use z-axis
		{
		ax = 0.0;
		ay = 0.0;
		az = 1.0;
		}
	else
		{
		ax = dx/s;
		ay = dy/s;
		az = dz/s;
		}
}


void VRMLScene::SaveVRML2( char * filename )
{
	VRMLObject *object;
	HANDLE hFile;
	DWORD byteswritten, dwVersionInfoSize, dwZero;
	int v1, v2, v3, v4;
	double x, y, z, ax, ay, az, a;
	UINT nBytesReturned;
	bool ErrorOccurred = false;
	char line[MAX_PATH], errmsg[1024];
	void *pVersionInfo;
	VS_FIXEDFILEINFO* pFixedFileInfo;
	
	v1 = v2 = v3 = v4 = 0;											// read version number information

	GetModuleFileName( GetModuleHandle(NULL), line, MAX_PATH );		// get the application file name

	dwVersionInfoSize = GetFileVersionInfoSize( line, &dwZero );	// how big is the version info?
	if ( dwVersionInfoSize )
		{
		pVersionInfo = malloc(dwVersionInfoSize);					// allocate space to store version info

		if ( GetFileVersionInfo( line, 0, dwVersionInfoSize, pVersionInfo) )	// get version info from file
			{
			if ( VerQueryValue(pVersionInfo, "\\", (LPVOID *)&pFixedFileInfo, &nBytesReturned) )
				{
				v1 = HIWORD(pFixedFileInfo->dwFileVersionMS); v2 = LOWORD(pFixedFileInfo->dwFileVersionMS);
				v3 = HIWORD(pFixedFileInfo->dwFileVersionLS); v4 = LOWORD(pFixedFileInfo->dwFileVersionLS);
				}
			}
		free(pVersionInfo);											// free memory used for version info
		}
																	// attempt to open file using Win32 call
 
	hFile = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		{															// fail, format error string
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							errmsg, 512, NULL );
		strcat(errmsg, filename );
		ErrMsgOK( ERRMSG_WRITE_FAILED, errmsg );
		return;
		}															// write VRML header info
	
	sprintf(line,"#VRML V2.0 utf8 generated by Reconstruct version %d.%d.%d.%d\r\n\r\n",v1,v2,v3,v4);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	sprintf(line,"Transform\t\t# series: %s, units: %s\r\n {\r\n children\r\n  [\r\n",BaseName,CurrSeries->units);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	sprintf(line,"  NavigationInfo { headlight TRUE type \"EXAMINE\" }\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	sprintf(line,"  Background { skyColor %f %f %f }\r\n",background.r,background.g,background.b);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	ComputeViewpoint( ax, ay, az, a, x, y, z );						// compute VRML viewpoint paramters
	sprintf(line,"  Viewpoint { description \"View\" orientation %1.*g %1.*g %1.*g %1.*g position %1.*g %1.*g %1.*g fieldOfView %1.*g }\r\n",
											Precision,ax,Precision,ay,Precision,az,Precision,a,
											Precision,x,Precision,y,Precision,z,Precision,fov);	// flip orientation to match that used by openGL
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	if ( objects )													// write out each object in scene
		{
		object = objects->first;
		while ( object )											// write object header info
			{
			sprintf(line,"  Transform\t\t# %s %s\r\n   {\r\n   children\t[\r\n",object->name,object->comment);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

			if ( !object->WriteVRML2( hFile ) ) ErrorOccurred = true;	// write shape data

			sprintf(line,"\t]\r\n   }\t\t# end of object %s\r\n\r\n",object->name);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

			object = object->next;								// close transform and do next object
			}
		}																	// write all terminators

	sprintf(line,"  ]\r\n }\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	CloseHandle( hFile );													// close file

	if ( ErrorOccurred ) {
		ErrMsgOK( ERRMSG_WRITE_FAILED, filename );
		}
}

void VRMLScene::SaveVRML1( char * filename )
{
	VRMLObject *object;
	HANDLE hFile;
	DWORD byteswritten, dwVersionInfoSize, dwZero;
	int v1, v2, v3, v4;
	double x, y, z, ax, ay, az, a;
	UINT nBytesReturned;
	bool ErrorOccurred = false;
	char line[MAX_PATH], errmsg[1024];
	void *pVersionInfo;
	VS_FIXEDFILEINFO* pFixedFileInfo;
	
	v1 = v2 = v3 = v4 = 0;											// read version number information

	GetModuleFileName( GetModuleHandle(NULL), line, MAX_PATH );		// get the application file name

	dwVersionInfoSize = GetFileVersionInfoSize( line, &dwZero );	// how big is the version info?
	if ( dwVersionInfoSize )
		{
		pVersionInfo = malloc(dwVersionInfoSize);					// allocate space to store version info

		if ( GetFileVersionInfo( line, 0, dwVersionInfoSize, pVersionInfo) )	// get version info from file
			{
			if ( VerQueryValue(pVersionInfo, "\\", (LPVOID *)&pFixedFileInfo, &nBytesReturned) )
				{
				v1 = HIWORD(pFixedFileInfo->dwFileVersionMS); v2 = LOWORD(pFixedFileInfo->dwFileVersionMS);
				v3 = HIWORD(pFixedFileInfo->dwFileVersionLS); v4 = LOWORD(pFixedFileInfo->dwFileVersionLS);
				}
			}
		free(pVersionInfo);											// free memory used for version info
		}
																	// attempt to open file using Win32 call
 
	hFile = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		{															// fail, format error string
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							errmsg, 512, NULL );
		strcat(errmsg, filename );
		ErrMsgOK( ERRMSG_WRITE_FAILED, errmsg );
		return;
		}															// write VRML header info

	sprintf(line, "#VRML V1.0 ascii\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	sprintf(line,"# generated by Reconstruct version %d.%d.%d.%d\r\n",v1,v2,v3,v4);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	sprintf(line,"# series: %s, units: %s\r\n\r\nSeparator\r\n {\r\n",BaseName,CurrSeries->units);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	ComputeViewpoint(ax,ay,az,a,x,y,z);								// calculate viewpoint from scene parameters
	sprintf(line,"  PerspectiveCamera { orientation %1.*g %1.*g %1.*g %1.*g position %1.*g %1.*g %1.*g heightAngle %1.*g }\r\n",
													Precision,ax,Precision,ay,Precision,az,Precision,a,
													Precision,x,Precision,y,Precision,z,Precision,fov);	// output VRML 1.0 viewpoint
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	if ( objects )													// write out each object in scene
		{
		object = objects->first;
		while ( object )											// write object header info
			{
			sprintf(line,"# %s %s\r\nSeparator\r\n {",object->name,object->comment);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

			if ( !object->WriteVRML1( hFile ) ) ErrorOccurred = true;	// write shape data

			sprintf(line,"\r\n }\r\n");
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

			object = object->next;									// close transform and do next object
			}
		}																	// write all terminators

	sprintf(line,"\r\n}\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	CloseHandle( hFile );													// close file

	if ( ErrorOccurred ) {
		ErrMsgOK( ERRMSG_WRITE_FAILED, filename );
		}
}


void VRMLScene::SaveDXF( char * filename )
{
	VRMLObject *object;
	HANDLE hFile;
	DWORD byteswritten, dwVersionInfoSize, dwZero;
	int v1, v2, v3, v4;
	double x, y, z, ax, ay, az, a;
	UINT nBytesReturned;
	bool ErrorOccurred = false;
	char line[MAX_PATH], errmsg[1024];
	void *pVersionInfo;
	VS_FIXEDFILEINFO* pFixedFileInfo;
	
	v1 = v2 = v3 = v4 = 0;											// read version number information

	GetModuleFileName( GetModuleHandle(NULL), line, MAX_PATH );		// get the application file name

	dwVersionInfoSize = GetFileVersionInfoSize( line, &dwZero );	// how big is the version info?
	if ( dwVersionInfoSize )
		{
		pVersionInfo = malloc(dwVersionInfoSize);					// allocate space to store version info

		if ( GetFileVersionInfo( line, 0, dwVersionInfoSize, pVersionInfo) )	// get version info from file
			{
			if ( VerQueryValue(pVersionInfo, "\\", (LPVOID *)&pFixedFileInfo, &nBytesReturned) )
				{
				v1 = HIWORD(pFixedFileInfo->dwFileVersionMS); v2 = LOWORD(pFixedFileInfo->dwFileVersionMS);
				v3 = HIWORD(pFixedFileInfo->dwFileVersionLS); v4 = LOWORD(pFixedFileInfo->dwFileVersionLS);
				}
			}
		free(pVersionInfo);											// free memory used for version info
		}
																	// attempt to open file using Win32 call
 
	hFile = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		{															// fail, format error string
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							errmsg, 512, NULL );
		strcat(errmsg, filename );
		ErrMsgOK( ERRMSG_WRITE_FAILED, errmsg );
		return;
		}

	sprintf(line,"999\r\nFlat DXF generated by Reconstruct version %d.%d.%d.%d\r\n",v1,v2,v3,v4);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	sprintf(line,"999\r\nSeries: %s, units: %s\r\n",BaseName,CurrSeries->units);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	sprintf(line,"0\r\nSECTION\r\n2\r\nENTITIES\r\n");	// skip header and tables, go to data
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	if ( objects )													// write out each object in scene
		{
		object = objects->first;
		while ( object )											// write objects
			{
			//sprintf(line,"999\r\nObject: %s %s\r\n",object->name,object->comment);
			//if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

			if ( !object->WriteDXF( hFile ) ) ErrorOccurred = true;	// write shape data

			object = object->next;									// close transform and do next object
			}
		}															// write all terminators

	sprintf(line,"0\r\nENDSEC\r\n0\r\nEOF\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	CloseHandle( hFile );													// close file

	if ( ErrorOccurred ) {
		ErrMsgOK( ERRMSG_WRITE_FAILED, filename );
		}
}


