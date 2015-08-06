/////////////////////////////////////////////////////////////////////////
//	This file contains code for the open GL 3D display window (3D Scene)
//
//    Copyright (C) 1998-2007  John Fiala (fiala@bu.edu)
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
// modified 11/11/04 by JCF (fiala@bu.edu)
// -+- change: Replaced horz & vert spin with adjustable spin rates dialog.
// modified 2/8/05 by JCF (fiala@bu.edu)
// -+- change: Added CM_INFOSCENE to main menu of 3D Scene window and function DisplaySceneInfo().
// modified 2/24/05 by JCF (fiala@bu.edu)
// -+- change: Added Objects count to DisplaySceneInfo().
// modified 3/7/05 by JCF (fiala@bu.edu)
// -+- change: Modified animation mode so only SetModelView() when have nonzero rotational increment.
// modified 3/16/05 by JCF (fiala@bu.edu)
// -+- change: Modified SpinRate dialog to provide single rotational increment when not spinning.
// modified 4/11/05 by JCF (fiala@bu.edu)
// -+- change: Fixed Save as Bitmap bug that created bogus images. Added Save Scene as JPEG.
// modified 7/13/05 by JCF (fiala@bu.edu)
// -+- change: Deleted bmpDC after save scene as image.
// modified 7/15/05 by JCF (fiala@bu.edu)
// -+- change: Fixed text in file type of JPEG scene export.
// modified 9/13/05 by JCF (fiala@bu.edu)
// -+- change: Removed erroneous ReleaseDC() from Save as... Bitmap/JPEG to fix problems after save.
// modified 10/3/05 by JCF (fiala@bu.edu)
// -+- change: Modified RenderSceneToBitmap() to use GL directly without call lists.
// modified 3/15/06 by JCF (fiala@bu.edu)
// -+- change: Fixed mode of dialog boxes evoked from 3D Scene.
// modified 4/28/06 by JCF (fiala@bu.edu)
// -+- change: Added orthographic display mode.
// modified 5/2/06 by JCF (fiala@bu.edu)
// -+- change: Fixed animation of scene when resizing window.
// modified 6/22/06 by JCF (fiala@bu.edu)
// -+- change: Fixed uninitialized hMenu in WM_LBUTTONDOWN and WM_RBUTTONDOWN of OpenGLProc().
// modified 11/16/06 by JCF (fiala@bu.edu)
// -+- change: Reset scene using Home key.
// modified 4/23/07 by JCF (fiala@bu.edu)
// -+- change: Fixed bug in CANCEL button of spin/rate dialog when not in spin mode.
// modified 4/25/07 by JCF (fiala@bu.edu)
// -+- change: Added 360 degree Bitmap output of scene.
// modified 7/16/07 by JCF (fiala@bu.edu)
// -+- change: Added enable of Maximize sys menu cmd in WM_CREATE of OpenGL Window.
//

#include "reconstruct.h"

bool SelectPixelFormat(HDC hdc)		// chose pixel format for openGL window
{
	PIXELFORMATDESCRIPTOR pfd = {  	sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd  
									1,								// version number
									PFD_DRAW_TO_WINDOW |			// support window 
									PFD_SUPPORT_OPENGL |			// support OpenGL 
									PFD_DOUBLEBUFFER,				// double buffered 
									PFD_TYPE_RGBA,			        // RGBA type 
									24,								// 24-bit color depth 
									0, 0, 0, 0, 0, 0,				// color bits ignored 
									0,								// no alpha buffer 
									0,								// shift bit ignored 
									0,								// no accumulation buffer 
									0, 0, 0, 0,						// accum bits ignored 
									32,								// 32-bit z-buffer	 
									0,								// no stencil buffer 
									0,								// no auxiliary buffer 
									PFD_MAIN_PLANE,					// main layer 
									0,								// reserved 
									0, 0, 0							// layer masks ignored 
								}; 
    
	int pfi = ChoosePixelFormat(hdc, &pfd);					// find closest supported format

	return( SetPixelFormat(hdc, pfi, &pfd) );				// set it if possible
}

void SetPerspective( HWND hWnd )					// set the perspective transformation for the scene
{
	RECT r;
	int w, h;
	double s;
	GLdouble fovNear, fovFar, fovLeft, fovRight, fovTop, fovBottom;

	if ( CurrScene )
		{
		s = 0.1; if ( openGL_ortho ) s = 1;						// enlarge frustrum in perspective case
		fovFar = (GLdouble)(CurrScene->fovFar/s);				// so won't cutoff scene as zoom in/out
		fovNear = (GLdouble)(s*CurrScene->fovNear);
		fovTop = (GLdouble)(s*CurrScene->fovTop);				// but fov stays the same
		fovBottom = (GLdouble)(s*CurrScene->fovBottom);
		fovLeft = (GLdouble)(s*CurrScene->fovLeft);
		fovRight = (GLdouble)(s*CurrScene->fovRight);

		GetClientRect(hWnd,&r);									// set window client area for dim
		w = (r.right - r.left);							
		h = (r.bottom - r.top);
		if ( (w > 0) && (h > 0) )								// if bad rect, don't change frustrum
			{
			glViewport(r.left, r.top, r.right, r.bottom);		// view port will be whole window			
			fovLeft = (GLdouble)w*fovLeft/(GLdouble)h;			// set frustrum to same aspect ratio 
			fovRight = (GLdouble)w*fovRight/(GLdouble)h;
			glMatrixMode(GL_PROJECTION);						// set perspective transformation
			glLoadIdentity();
			if ( openGL_ortho ) glOrtho(fovLeft, fovRight, fovBottom, fovTop, fovNear, fovFar);
			else glFrustum(fovLeft, fovRight, fovBottom, fovTop, fovNear, fovFar);
			}
		}
}

bool RenderSceneToBitmap( HDC bmpDC, HWND hWnd )	// openGL rendering on a bitmap DC !bitmap MUST be DIBSection!
{
	HDC ghDC;
	HGLRC ghRC, bmpRC;
	GLfloat local[16];
	bool success = false;
	PIXELFORMATDESCRIPTOR pfd = {  	sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd  
									1,								// version number
									PFD_DRAW_TO_BITMAP |			// support bitmap
									PFD_SUPPORT_OPENGL |			// support OpenGL
									PFD_SUPPORT_GDI,				// allow GDI operations 
									PFD_TYPE_RGBA,			        // RGBA type 
									24,								// 24-bit color depth 
									0, 0, 0, 0, 0, 0,				// color bits ignored 
									0,								// no alpha buffer 
									0,								// shift bit ignored 
									0,								// no accumulation buffer 
									0, 0, 0, 0,						// accum bits ignored 
									32,								// 32-bit z-buffer	 
									0,								// no stencil buffer 
									0,								// no auxiliary buffer 
									PFD_MAIN_PLANE,					// main layer 
									0,								// reserved 
									0, 0, 0							// layer masks ignored 
								}; 
    
	int pfi = ChoosePixelFormat(bmpDC, &pfd);					// find closest supported format
	
	if ( SetPixelFormat(bmpDC, pfi, &pfd) )					// set it if possible
		{
		ghDC = wglGetCurrentDC();
		ghRC = wglGetCurrentContext();						// remember orig window drawing contexts
		bmpRC = wglCreateContext( bmpDC );
		if ( wglMakeCurrent( bmpDC, bmpRC ) )
			{
			glEnable(GL_DEPTH_TEST);						// initialize gl parameters: this should
			glEnable(GL_LIGHTING);							// duplicate the code in WM_CREATE below
			glEnable(GL_LIGHT0);
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
			if ( CurrScene )								// now render the scene
				{
				SetPerspective( hWnd );						// first duplicate perspective and model view
				for (int i=0; i<16; i++) local[i] = CurrScene->m[i];
				local[12] += CurrScene->xTrans;
				local[13] += CurrScene->yTrans;
				local[14] += CurrScene->zTrans;
				local[12] += local[0]*CurrScene->xCenter + local[4]*CurrScene->yCenter + local[8]*CurrScene->zCenter;
				local[13] += local[1]*CurrScene->xCenter + local[5]*CurrScene->yCenter + local[9]*CurrScene->zCenter;
				local[14] += local[2]*CurrScene->xCenter + local[6]*CurrScene->yCenter + local[10]*CurrScene->zCenter;
				glMatrixMode(GL_MODELVIEW);
				glLoadMatrixf( local );
				glClearColor( CurrScene->background.r, CurrScene->background.g, CurrScene->background.b, 0.0 );
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear background
				CurrScene->DisplayUncompiled();				// and draw everything in scene directly into context
				glFlush();
				}
			success = true;
			}
		wglMakeCurrent( ghDC, ghRC );					// restore rendering to window
		wglDeleteContext( bmpRC );						// delete bitmap DC rendering context
		}

	return success;										// signal success
}

void SetModelView( double xRotInc, double yRotInc )			// transformation of scene
{
	GLfloat c, s, local[16];
											
	if ( CurrScene )
		{
		for (int i=0; i<16; i++) local[i] = CurrScene->m[i];	// copy rotation matrix
		c = (GLfloat)(cos(yRotInc*PI/180.0));
		s = (GLfloat)(sin(yRotInc*PI/180.0));					// compute y rotation components
		CurrScene->m[0] = c*local[0] + s*local[2];				// preapply y increment to scene matrix
		CurrScene->m[4] = c*local[4] + s*local[6];
		CurrScene->m[8] = c*local[8] + s*local[10];
		CurrScene->m[2] = c*local[2] - s*local[0];
		CurrScene->m[6] = c*local[6] - s*local[4];
		CurrScene->m[10] = c*local[10] - s*local[8];
		for (int i=0; i<16; i++) local[i] = CurrScene->m[i];	// copy modified matrix
		c = (GLfloat)(cos(-xRotInc*PI/180.0));
		s = (GLfloat)(sin(-xRotInc*PI/180.0));					// compute x rotation components
		CurrScene->m[1] = c*local[1] - s*local[2];				// preapply x increment to scene matrix
		CurrScene->m[5] = c*local[5] - s*local[6];
		CurrScene->m[9] = c*local[9] - s*local[10];
		CurrScene->m[2] = s*local[1] + c*local[2];
		CurrScene->m[6] = s*local[5] + c*local[6];
		CurrScene->m[10] = s*local[9] + c*local[10];
		for (int i=0; i<16; i++) local[i] = CurrScene->m[i];	// copy modified matrix
		local[12] += CurrScene->xTrans;
		local[13] += CurrScene->yTrans;							// premultiply by offset, postmultiply by center translation
		local[14] += CurrScene->zTrans;
		local[12] += local[0]*CurrScene->xCenter + local[4]*CurrScene->yCenter + local[8]*CurrScene->zCenter;
		local[13] += local[1]*CurrScene->xCenter + local[5]*CurrScene->yCenter + local[9]*CurrScene->zCenter;
		local[14] += local[2]*CurrScene->xCenter + local[6]*CurrScene->yCenter + local[10]*CurrScene->zCenter;

		glMatrixMode(GL_MODELVIEW);								// apply transform to scene
		glLoadMatrixf( local );
		}
}

BOOL CALLBACK SpinRateDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM)
{
	char dlgItemTxt[64];		// dialog has two interpretations:

	switch (message)
		{
		case WM_INITDIALOG:
			if ( openGLanimated )	// 1) if spinning, set spin rate
				{
				sprintf(dlgItemTxt,"%g",CurrScene->yRotInc*1000.0/FRAMETIME);
				SetDlgItemText( hwndDlg, ID_HORZSPINRATE, dlgItemTxt );
				sprintf(dlgItemTxt,"%g",CurrScene->xRotInc*1000.0/FRAMETIME);
   				SetDlgItemText( hwndDlg, ID_VERTSPINRATE, dlgItemTxt );
				}
			else {					// 2) if not spinning, specify rotational increment
				sprintf(dlgItemTxt,"%g",CurrScene->yRotInc);
				SetDlgItemText( hwndDlg, ID_HORZSPINRATE, dlgItemTxt );
				sprintf(dlgItemTxt,"%g",CurrScene->xRotInc);
   				SetDlgItemText( hwndDlg, ID_VERTSPINRATE, dlgItemTxt );
				SetDlgItemText( hwndDlg, ID_SPINRATEMSG, "Enter rotational increment in degrees" );
				}
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{									// get user input values
				case IDOK:
					GetDlgItemText( hwndDlg, ID_HORZSPINRATE, dlgItemTxt, sizeof(dlgItemTxt) );
					CurrScene->yRotInc = atof(dlgItemTxt);
   					GetDlgItemText( hwndDlg, ID_VERTSPINRATE, dlgItemTxt, sizeof(dlgItemTxt) );
					CurrScene->xRotInc = atof(dlgItemTxt);
					if ( openGLanimated )			// if spinning, convert from degrees/second
						{							// to increment on each WM_TIMER
						CurrScene->yRotInc = CurrScene->yRotInc/(1000.0/FRAMETIME);
						CurrScene->xRotInc = CurrScene->xRotInc/(1000.0/FRAMETIME);
						}
					EndDialog(hwndDlg, 1);
					return TRUE;
				case IDCANCEL:
					EndDialog(hwndDlg, 0);			// to cancel, return zero
					return TRUE;
				}
		}
	return FALSE;
}

void DisplaySceneInfo( HWND hWnd )		// display statistics of scene in dialog box
{
	VRMLObject *object;
	char txt[512];
	int object_count = 0;
	long int vertex_count = 0;
	long int normal_count = 0;
	long int line_count = 0;
	long int face_count = 0;

	if ( CurrScene )
	  if ( CurrScene->objects )
		{
		object = CurrScene->objects->first;
		while ( object )
			{
			object_count++;
			if ( object->vertices ) vertex_count += object->vertices->total;
			if ( object->normals ) normal_count += object->normals->total;
			if ( object->lines ) line_count += object->lines->total;
			if ( object->faces ) face_count += object->faces->total;
			object = object->next;
			}
		}

	sprintf(txt,"Scene contains:  \n%d Objects\n%u Vertices\n%u Normal vectors\n%u Line segments\n%u Triangles",
					object_count, vertex_count, normal_count, line_count, face_count );
	MessageBox(hWnd,txt,"Scene Info",MB_OK);
}

void ResetScene( HWND hWnd )		// simple routine to reset becuz called multiple places in OpenGLProc
{
	if ( CurrScene )
		{
		CurrScene->Reset();							// scene might have changed, recalculate fov
		SetPerspective( hWnd );
		SetModelView(0.0,0.0);						// no rotations defined yet
		InvalidateRect(hWnd,NULL,FALSE);
		}
}

void SaveSceneAsImage( HWND hWnd, char *filename, int compress )	// render scene onto a bitmap and save to file
{																	// if compress>0 use JPEG with this as quality
	int w, h;
	RECT r;
	HDC ghDC, bmpDC;
	HBITMAP bmp, orig;
	ADib *bmpADib;
	void *bmpBits;

	GetClientRect( hWnd, &r );					// get size for bitmap from scene window client area
	w = r.right-r.left;
	h = r.bottom-r.top;
	bmpADib = new ADib( w, h, 24 );				// create bitmap for view
	ghDC = wglGetCurrentDC();
	bmpDC = CreateCompatibleDC( ghDC );
	bmp = CreateDIBSection( ghDC, bmpADib->bmi, DIB_RGB_COLORS, &bmpBits, NULL, 0 );
	orig = (HBITMAP)SelectObject( bmpDC, bmp );	// select bitmap into DC to store view image	
	if ( !RenderSceneToBitmap( bmpDC, hWnd ) )	// render scene on bitmapDC
		ErrMsgOK( ERRMSG_BADPIXELFORMAT, "" );
	GetDIBits( bmpDC, bmp, 0, h, bmpADib->bits, bmpADib->bmi, DIB_RGB_COLORS);
	GdiFlush();
	if ( compress )
		bmpADib->SaveADibAsJPEG( filename, compress );
	else
		bmpADib->SaveADibAsBMP( filename );		// output BMP format when no compress selected
	SelectObject( bmpDC, orig );
	DeleteObject( bmp );
	delete bmpADib;
	DeleteDC( bmpDC );
}
													// Windows procedure for a Open GL 3D display...

LRESULT APIENTRY OpenGLProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i, x, y, w, h, border;
	float fx, fy;
	RECT r, clientarea, winarea;
	HMENU hMenu;
	HCURSOR cur;
	POINT position, lefttop, rightbottom;
	HDC ghDC;
	HGLRC ghRC;
	PAINTSTRUCT ps;
	CHOOSECOLOR cc;
	OPENFILENAME ofn;
	char filename[MAX_PATH], path[MAX_PATH], prefix[MAX_PATH], ext[MAX_PATH];

	switch(msg)
		{
		case WM_CREATE:										// setup window
			CenterDialog( appWnd, hWnd );
			ghDC = GetDC(hWnd);					  			// set GL device context
			if ( !SelectPixelFormat(ghDC) )
				{
				ErrMsgOK(ERRMSG_BADPIXELFORMAT,"");
				DestroyWindow( hWnd );
				}
			else {											// if successfully created
				ghRC = wglCreateContext(ghDC);
				wglMakeCurrent(ghDC, ghRC);					// activate gl Context
				glEnable(GL_DEPTH_TEST);					// initial some gl parameters
				glEnable(GL_LIGHTING);
				glEnable(GL_LIGHT0);
				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
				if ( CurrScene )
					{
					CurrScene->Compile();					// compile scene into context
					CurrScene->Reset();						// reset viewing params
					glClearColor( CurrScene->background.r, CurrScene->background.g, CurrScene->background.b, 0.0 );
					openGLanimated = true;					// flag that timer is running
					SetTimer( hWnd, OPENGL_TIMER, FRAMETIME, NULL );	// activate drift mode
					}
				}
			// there will be a automatic WM_SIZE message issued after WM_CREATE to set frustrum
			hMenu = GetSystemMenu( hWnd, FALSE );						// allow Maximize on titlebar menu
			EnableMenuItem( hMenu, SC_MAXIMIZE, MF_ENABLED );
			break;

		case WM_DESTROY:						// if thread has a current rendering context, delete it
			ghRC = wglGetCurrentContext();
			if( ghRC )
				{ 
				ghDC = wglGetCurrentDC(); 
				wglMakeCurrent(NULL, NULL);		// make the rendering context not current 
				ReleaseDC(hWnd, ghDC);
				wglDeleteContext(ghRC);
				}
			KillTimer( hWnd, OPENGL_TIMER );
			hMenu = GetMenu( appWnd );
			CheckMenuItem( hMenu, CM_3DSCENE, MF_BYCOMMAND | MF_UNCHECKED );
			openGLWindow = NULL;							// clear global handle
			SetFocus(appWnd);					// return to main window
			break;

		case WM_PAINT:
			if ( GetUpdateRect(hWnd,NULL,FALSE) )
				{
				ghDC = BeginPaint( hWnd, &ps );
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// repaint background
				if ( CurrScene ) CurrScene->Display();	// objects to draw are in openGL call list
				glFlush();
				SwapBuffers( ghDC );
				EndPaint( hWnd, &ps );
				}
			break;
	
		case WM_SIZE:										// resize viewport with user resizes window
			SetPerspective( hWnd );
			SetModelView(0.0,0.0);							// don't apply any incremental rotations
			InvalidateRect(hWnd,NULL,FALSE);				// redraw scene
			break;

		case WM_SETCURSOR:
			GetCursorPos(&position);						// in screen coordinates
			GetClientRect(hWnd, &clientarea);				// client rect is in client coordinates
			lefttop.x = clientarea.left;
			lefttop.y = clientarea.top;
			rightbottom.x = clientarea.right;
			rightbottom.y = clientarea.bottom;
			ClientToScreen( hWnd, &lefttop );				// convert client rect to screen coordinates
			ClientToScreen( hWnd, &rightbottom );
			clientarea.left = lefttop.x;
			clientarea.top = lefttop.y;
			clientarea.right = rightbottom.x;
			clientarea.bottom = rightbottom.y;
			GetWindowRect( hWnd, &winarea );				// get window rect in screen coordinates
			border = winarea.left - clientarea.left;		// determine (negative) width of resizing border
			InflateRect( &winarea, border, border );		// shrink winarea by this amount
			if ( PtInRect( &clientarea, position ) )
				{
				if ( openGL_LToolActive && openGL_RToolActive ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(XYCUR) );
				else if ( openGL_RToolActive ) cur = LoadCursor( appInstance, MAKEINTRESOURCE(ZOOMCUR) );
				else cur = LoadCursor( appInstance, MAKEINTRESOURCE(ROTATECUR) );
				}
			else 
				if ( PtInRect( &winarea, position ) ) cur = LoadCursor( 0, IDC_ARROW );
				else cur = LoadCursor( 0, IDC_SIZEALL );
			SetCursor( cur );
			return 0L;										// don't used default processing on this message

		case WM_LBUTTONDOWN:					// left mouse button press
			if ( !openGL_RToolActive )
				{								
				x = LOWORD(lParam);					// get input pt in client coordinates
				y = HIWORD(lParam);
				openGLlast.x = x;					// remember starting point
				openGLlast.y = y;
				GetClientRect( hWnd, &r );			// get client region for clipping cursor
				ClientRectToScreen( hWnd, &r );
				ClipCursor( &r );					// capture cursor and prepare for mouse move
				SetCursor( LoadCursor( appInstance, MAKEINTRESOURCE(ROTATECUR) ) );
				}
			else SetCursor( LoadCursor( appInstance, MAKEINTRESOURCE(XYCUR) ) );
			openGL_LToolActive = true;
			hMenu = GetMenu( hWnd );
			if ( openGLanimated ) CheckMenuItem( hMenu, CM_ANIMATE, MF_BYCOMMAND | MF_CHECKED );
			else CheckMenuItem( hMenu, CM_ANIMATE, MF_BYCOMMAND | MF_UNCHECKED );
			break;

		case WM_RBUTTONDOWN:					// right mouse button press
			if ( !openGL_LToolActive )
				{
				x = LOWORD(lParam);					// get input pt in client coordinates
				y = HIWORD(lParam);
				openGLlast.x = x;					// remember starting point
				openGLlast.y = y;
				GetClientRect( hWnd, &r );			// get client region for clipping cursor
				ClientRectToScreen( hWnd, &r );
				ClipCursor( &r );					// capture cursor and prepare for mouse move
				SetCursor( LoadCursor( appInstance, MAKEINTRESOURCE(ZOOMCUR) ) );
				}
			else SetCursor( LoadCursor( appInstance, MAKEINTRESOURCE(XYCUR) ) );
			CurrScene->xRotInc = 0.0;
			CurrScene->yRotInc = 0.0;				// clear rotational increments so won't have any
			hMenu = GetMenu( hWnd );
			if ( openGLanimated ) CheckMenuItem( hMenu, CM_ANIMATE, MF_BYCOMMAND | MF_CHECKED );
			else CheckMenuItem( hMenu, CM_ANIMATE, MF_BYCOMMAND | MF_UNCHECKED );
			openGL_RToolActive = true;				// during zoom or x-y translation of scene
			break;

		case WM_MOUSEMOVE:						// mouse is moving in window
			if ( CurrScene && (openGL_LToolActive || openGL_RToolActive)  )
				{
				x = LOWORD(lParam);							// get mouse pt in client coordinates
				y = HIWORD(lParam);
				GetClientRect( hWnd, &r );					// get client region for clipping cursor
				w = r.right-r.left;
				h = r.bottom-r.top;							// determine fraction of window cursor moved
				fx = (float)(x - openGLlast.x)/(float)w;
				fy = (float)(openGLlast.y - y)/(float)h;	// y is inverted on screen
				if ( openGL_LToolActive && openGL_RToolActive )
					{										// translate in x and y (pan)
					CurrScene->xTrans += 20.0*fx*CurrScene->fovRight;
					CurrScene->yTrans += 20.0*fy*CurrScene->fovTop;
					}
				else if ( openGL_LToolActive )				// rotate scene
					{
					CurrScene->yRotInc = 90.0*fx;			// compute new angular change
					CurrScene->xRotInc = 90.0*fy;
					}
				else if ( openGL_RToolActive )				// move in z direction (zoom)
					CurrScene->zTrans += -50.0*fy*CurrScene->fovTop;	
				openGLlast.x = x;							// update new cursor position
				openGLlast.y = y;
				SetModelView(CurrScene->xRotInc,CurrScene->yRotInc);							
				InvalidateRect(hWnd,NULL,FALSE);			// redraw scene
				}
			break;

		case WM_LBUTTONUP:						// left mouse button release
			if ( openGL_LToolActive )
				{
				if ( !openGL_RToolActive ) ClipCursor( NULL );
				openGL_LToolActive = false;
				}
			break;

		case WM_RBUTTONUP:						// right mouse button release
			if ( openGL_RToolActive ) {				// make sure cursor is right also
				if ( !openGL_LToolActive ) ClipCursor( NULL );
				SetCursor( LoadCursor( appInstance, MAKEINTRESOURCE(ROTATECUR) ) );
				openGL_RToolActive = false;
				}
			break;

		case WM_TIMER:							// repeat last incremental rotation
			if ( CurrScene && openGLanimated )	
			  if ( (CurrScene->xRotInc != 0.0) || (CurrScene->yRotInc != 0.0) )
				{
				SetModelView(CurrScene->xRotInc,CurrScene->yRotInc);
				InvalidateRect(hWnd,NULL,FALSE);	// redisplay scene here
				}
			break;

		case WM_KEYDOWN:
			switch ( wParam )					// allow control+TAB to switch active window
				{
				case VK_TAB: 
					if ( GetKeyState(VK_CONTROL) & 0x8000 ) CmNextWindow( openGLWindow );
					break;
				case VK_HOME: ResetScene( hWnd );
					break;
				}
			break;

		case WM_COMMAND:						// menu commands
			switch (LOWORD(wParam)) {

			case CM_SPINSPEED:					// set the amount/rate to rotate
				if ( CurrScene )
					{
					if ( DialogBox( appInstance, "SpinRateDlg", hWnd, (DLGPROC)SpinRateDlgProc ) )
					  if ( !openGLanimated )
						{									// if user says OK and not spinning
						SetModelView(CurrScene->xRotInc,CurrScene->yRotInc);	
						InvalidateRect(hWnd,NULL,FALSE);	// rotate just once and redraw scene
						}
					}
				break;

			case CM_ANIMATE:					// switch spin state
				if ( CurrScene )
					{
					hMenu = GetMenu( hWnd );
					if ( openGLanimated )							/// if spinning, stop
						{
						CurrScene->xRotInc = 0.0;
						CurrScene->yRotInc = 0.0;						
						openGLanimated = false;
						KillTimer( hWnd, OPENGL_TIMER );
						CheckMenuItem( hMenu, CM_ANIMATE, MF_BYCOMMAND | MF_UNCHECKED );
						}
					else {											// use timer to keep repeating last rotation
						CurrScene->xRotInc = 0.0;					//   i.e. "spin"
						CurrScene->yRotInc = 1.0;
						openGLanimated = true;
						SetTimer( hWnd, OPENGL_TIMER, FRAMETIME, NULL );
						CheckMenuItem( hMenu, CM_ANIMATE, MF_BYCOMMAND | MF_CHECKED );
						}
					}
				break;

			case CM_RESETSCENE:									// reset the transform params and redisplay
				ResetScene( hWnd );
				break;

			case CM_ORTHOGRAPHIC:								// switch display to/from orthographic projection
				if ( CurrScene )
					{
					openGL_ortho = !openGL_ortho;
					hMenu = GetMenu( hWnd );
					if ( openGL_ortho )
						 CheckMenuItem( hMenu, CM_ORTHOGRAPHIC, MF_BYCOMMAND | MF_CHECKED );
					else CheckMenuItem( hMenu, CM_ORTHOGRAPHIC, MF_BYCOMMAND | MF_UNCHECKED );
					ResetScene( hWnd );
					}
				break;

			case CM_BACKGROUND:									// select a new background color
				if ( CurrScene )
					{
					ZeroMemory(&cc, sizeof(CHOOSECOLOR));			// setup color dialog structure
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hWnd;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					cc.rgbResult = CurrScene->background.ref();		// set current color
					cc.lpCustColors = CustomColors;					// set custom colors
					if ( ChooseColor(&cc) )							// open color dialog
						{
						CurrScene->background = Color( cc.rgbResult );// reset color
						glClearColor( CurrScene->background.r, CurrScene->background.g, CurrScene->background.b, 0.0 );
						InvalidateRect(hWnd,NULL,FALSE);
						}
					}
				break;

			case CM_SAVEVRML2:									// output scene as VRML 2.0 file
				if ( CurrScene )
					{
					sprintf(filename,"%s.wrl",BaseName);			// the VRML file will be "seriesname.wrl"
					ZeroMemory(&ofn, sizeof(ofn));					// clear the OPENFILENAME fields
					ofn.lStructSize = sizeof(ofn);					// set only the necessary values
					ofn.hwndOwner = hWnd;
					ofn.lpstrFile = filename;
					ofn.lpstrDefExt = "wrl";
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "All Files\0*.*\0VRML files\0*.wrl\0\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrTitle = "Save As";						// ask user permission to overwrite existing
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

					if ( GetSaveFileName(&ofn) )					// display the SaveAs dialog box
						CurrScene->SaveVRML2( filename );
					}
				break;

			case CM_SAVEVRML1:									// output scene as VRML 1.0 file
				if ( CurrScene )
					{
					sprintf(filename,"%s.wrl",BaseName);			// the VRML file will be "seriesname.wrl"
					ZeroMemory(&ofn, sizeof(ofn));					// clear the OPENFILENAME fields
					ofn.lStructSize = sizeof(ofn);					// set only the necessary values
					ofn.hwndOwner = hWnd;
					ofn.lpstrFile = filename;
					ofn.lpstrDefExt = "wrl";
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "All Files\0*.*\0VRML files\0*.wrl\0\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrTitle = "Save As";						// ask user permission to overwrite existing
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

					if ( GetSaveFileName(&ofn) )					// display the SaveAs dialog box
						CurrScene->SaveVRML1( filename );
					}
				break;
					
			case CM_SAVEDXF:										// output scene as DXF file
				if ( CurrScene )
					{
					sprintf(filename,"%s.dxf",BaseName);			// the VRML file will be "seriesname.wrl"
					ZeroMemory(&ofn, sizeof(ofn));					// clear the OPENFILENAME fields
					ofn.lStructSize = sizeof(ofn);					// set only the necessary values
					ofn.hwndOwner = hWnd;
					ofn.lpstrFile = filename;
					ofn.lpstrDefExt = "dxf";
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = "All Files\0*.*\0DXF files\0*.dxf\0\0";
					ofn.nFilterIndex = 2;
					ofn.lpstrTitle = "Save As";						// ask user permission to overwrite existing
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

					if ( GetSaveFileName(&ofn) )					// display the SaveAs dialog box
						CurrScene->SaveDXF( filename );
					}
				break;

			case CM_SAVEBITMAP:								// output image of scene
				sprintf(filename,"%s.bmp",BaseName);			// the bitmap file will be "seriesname.bmp"
				ZeroMemory(&ofn, sizeof(ofn));					// clear the OPENFILENAME fields
				ofn.lStructSize = sizeof(ofn);					// set only the necessary values
				ofn.hwndOwner = hWnd;
				ofn.lpstrFile = filename;
				ofn.lpstrDefExt = "bmp";
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = "All Files\0*.*\0Windows Bitmaps\0*.bmp\0\0";
				ofn.nFilterIndex = 2;
				ofn.lpstrTitle = "Save As";						// ask user permission to overwrite existing
				ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

				if ( GetSaveFileName(&ofn) )					// display the SaveAs dialog box
					SaveSceneAsImage( hWnd, filename, 0 );		// render and save to BMP
				break;

			case CM_SAVEJPEG:								// output image of scene as JPEG file
				sprintf(filename,"%s.jpg",BaseName);			// the bitmap file will be "seriesname.bmp"
				ZeroMemory(&ofn, sizeof(ofn));					// clear the OPENFILENAME fields
				ofn.lStructSize = sizeof(ofn);					// set only the necessary values
				ofn.hwndOwner = hWnd;
				ofn.lpstrFile = filename;
				ofn.lpstrDefExt = "jpg";
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = "All Files\0*.*\0JPEG\0*.jpg\0\0";
				ofn.nFilterIndex = 2;
				ofn.lpstrTitle = "Save As";						// ask user permission to overwrite existing
				ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

				if ( GetSaveFileName(&ofn) )					// display the SaveAs dialog box
					{
					sprintf(InputDlgName,"JPEG Quality");
					sprintf(InputDlgString,"80");
					sprintf(InputDlgValue,"Image Quality: 1 (worst) - 100 (best)");
					if ( DialogBox( appInstance, "InputDlg", hWnd, (DLGPROC)InputDlgProc ) )
						{
						i = atoi(InputDlgString);				// get compression factor
						if ( i < 1 ) i = 1;
						if ( i > 100 ) i = 100;
						SaveSceneAsImage( hWnd, filename, i );	// render and save as JPEG
						}
					}
				break;

			case CM_SAVE360BMP:								// output a sequence of images to 360 degrees
				if ( CurrScene )
					{
					if ( openGLanimated )							// if spinning, stop
						{
						CurrScene->xRotInc = 0.0;
						CurrScene->yRotInc = 0.0;						
						openGLanimated = false;
						KillTimer( hWnd, OPENGL_TIMER );
						CheckMenuItem( hMenu, CM_ANIMATE, MF_BYCOMMAND | MF_UNCHECKED );
						}
																	// get new increment angles for 360
					if ( DialogBox( appInstance, "SpinRateDlg", hWnd, (DLGPROC)SpinRateDlgProc ) )
						{

						sprintf(filename,"%s.bmp",BaseName);		// files will be "seriesname001.bmp", etc.
						ZeroMemory(&ofn, sizeof(ofn));				// clear the OPENFILENAME fields
						ofn.lStructSize = sizeof(ofn);				// set only the necessary values
						ofn.hwndOwner = hWnd;
						ofn.lpstrFile = filename;
						ofn.lpstrDefExt = "bmp";
						ofn.nMaxFile = MAX_PATH;
						ofn.lpstrFilter = "All Files\0*.*\0Windows Bitmaps\0*.bmp\0\0";
						ofn.nFilterIndex = 2;
						ofn.lpstrTitle = "Save As";					// ask user permission to overwrite existing
						ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

						if ( GetSaveFileName(&ofn) )				// display the SaveAs dialog box
							{
							SplitPath( filename, path, prefix, ext );	 // get parts so can insert numbers
							cur = SetCursor( LoadCursor( 0, IDC_WAIT ) );// show hourglass, file access could be slow
							fx = 0.0; fy = 0.0; i = 1;
							while ( (fabs(fx) < 360.0) && (fabs(fy) < 360.0) && (i<1000) )
								{
								sprintf(filename,"%s%s%03d%s", path, prefix, i, ext);
								SaveSceneAsImage( hWnd, filename, 0 );		// render and save to BMP
								fx = fx + CurrScene->xRotInc;
								fy = fy + CurrScene->yRotInc;
								i++;										// increment image filename number
								SetModelView(CurrScene->xRotInc,CurrScene->yRotInc);	
								}
							SetCursor( cur );						// restore original cursor
							InvalidateRect(hWnd,NULL,FALSE);		// redraw scene to show final pose
							}
						}
					}
				break;


			case CM_CLEAROBJECTS:
				if ( CurrScene ) CurrScene->ClearObjects();
				InvalidateRect( hWnd, NULL, FALSE );
				break;

			case CM_CLOSESCENE:
				DestroyWindow( hWnd );				// close of openGL scene window
				break;

			case CM_INFOSCENE:
				DisplaySceneInfo( hWnd );			// display statistics on scene
				break;
			}
			break;
		}

	return( DefWindowProc(hWnd, msg, wParam, lParam) );
}


HWND MakeOpenGLWindow( void )				// create window for 3D display of contour data
{
	HWND openGLWnd;
	HMENU hMenu;
	WNDCLASS wndclass;
											// create the toolbar window...
	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)OpenGLProc;
	wndclass.cbClsExtra  = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance = appInstance;
	wndclass.hCursor   = LoadCursor( 0, IDC_ARROW );
	wndclass.hIcon     = NULL;
	wndclass.lpszMenuName = "3DMenu";
	wndclass.hbrBackground = NULL;
	wndclass.lpszClassName = "Reconstruct3DClass";
	RegisterClass(&wndclass);
		
    openGLWnd = CreateWindowEx( WS_EX_TOOLWINDOW, "Reconstruct3DClass", "3D Scene", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
									0, 0, GL_WIDTH, GL_HEIGHT, appWnd, (HMENU)NULL, appInstance, (LPVOID)NULL); 

	if ( openGLWnd )			// if window created, display it and check corresponding menu item in main window
		{
		ShowWindow(openGLWnd, SW_SHOW);
		hMenu = GetMenu( appWnd );											// check menuitem in main menu to indicate window is open
		CheckMenuItem( hMenu, CM_3DSCENE, MF_BYCOMMAND | MF_CHECKED );
		}

	return openGLWnd;
}
