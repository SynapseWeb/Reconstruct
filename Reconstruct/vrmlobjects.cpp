/////////////////////////////////////////////////////////////////////////////
//	This file contains the additional methods of the VRMLObject list class
//
//    Copyright (C) 2003-2004  John Fiala (fiala@bu.edu)
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
// modified xx/xx/xx by _______________________
// -+- change: 
//

#include "reconstruct.h"

VRMLObject * VRMLObjects::Match( char * name )
{
	VRMLObject *object;
	
	object = this->first;
	while ( object )
		{
		if ( (strcmp( object->name, name ) == 0) ) break;
		object = object->next;
		}

	return object;
}

