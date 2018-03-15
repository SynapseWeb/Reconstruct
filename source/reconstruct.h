///////////////////////////////////////////////////////////////////////////////////////
// The combined header file for the RECONSTRUCT application.
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
// 
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Changed name of objects.h to classes.h
// modified 6/22/06 by JCF (fiala@bu.edu)
// -+- change: Added definition to display MS VC deprecation warnings (temporary)
// modified 6/22/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added histogram.cpp
// modified 11/2/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added automatictrace.cpp

//#define _CRT_SECURE_NO_DEPRECATE 1	// turn off MS VC warnings

						// Standard library includes
#include <math.h>
#include <stdio.h>
#include <fcntl.h>		// needed for GBM_io params

//#ifndef O_BINARY		// missing from winelib on Gnu/LINUX?
//#define O_BINARY    0x8000
//#endif
						// Win32 API includes
#include <windows.h>
#include <commctrl.h>
						// Open GL libraries
#include <GL/gl.h>

						// APPLICATION CONSTANTS
#include "constants.h"	// these #defines are shared with .rc resources

#include "gbm.h"		// generalized bitmap I/O library
#include "kube-gustavson-fft.h"	// code for Fast Fourier Transforms

						// OBJECT CLASSES
#include "classes.h"	// Bodies of the object classes are:
						//	lists.hpp	list template methods
						//	contour.cpp	manipulation of point lists
						//	image.cpp	loading bitmap data
						//	nform.cpp	nonlinear transformations
						//  tformslifo.cpp undo stacks
						//  adib.cpp	device independent bitmaps
						//  section.cpp methods on section data
						//  sectionsinfo.cpp lists of sections
						//  viewport.cpp display bitmaps and rendering
						//  series.cpp	methods on series data
						//  xmldata.cpp xml i/o methods
						//  vrmlobject.cpp creation of 3D data rep.
						//  vrmlobjects.cpp methods on lists of objects
						//  vrmlscene.cpp	openGL compilation of objects
						//  histogram.cpp  histogram of contour-enclosed pixels
						//  automatictrace.cpp class to keep track of multi-contour auto-trace
						// USER INTERFACE OPERATIONS
#include "operations.h"	// Bodies of the operation functions:
						//  init.cpp		program initialization
						//  end.cpp			program termination
						//  threads.cpp		multithreading
						//  program_menu.cpp program menu operations
						//  series_menu.cpp	series menu operations
						//  section_menu.cpp section menu operations
						//  domain_menu.cpp	domain menu operations
						//  trace_menu.cpp	trace menu operations
						//  object_menu.cpp	object menu operations
						//  help_menu.cpp	help menu operations
						//  tools.cpp		toolbar and tools operations
						//  thumbnails.cpp	thumbnail window 
						//  opengl.cpp		OpenGL 3D scene display
						//  palette.cpp		trace palette window
						//  keyboard.cpp	keyboard input operations
						//  mouse.cpp		mouse clicks and cursor movements
						//  utility.cpp		utility functions

						// GLOBAL STATE VARIABLES
#include "globals.h"	//	the actual variables are declared in "globals.cpp"

// the application main (WinMain) and windows procedure is located in
// reconstruct.cpp

