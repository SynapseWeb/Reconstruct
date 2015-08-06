///////////////////////////////////////////////////////////////////////////////
//	This file contains the methods for the section info list
//
//    Copyright (C) 2003-2005  John Fiala (fiala@bu.edu)
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
// modified 4/27/05 by JCF (fiala@bu.edu)
// -+- change: Added IsSection() to test if a sectnum is already in the list.
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change: Added pixelsize and min, max components to SectionInfo.
//

#include "reconstruct.h"

bool SectionsInfo::IsSection( int sectnum )
{
	SectionInfo *s;
	s = first;
	while ( s )
		{
		if ( sectnum == s->index ) return true;
		s = s->next;
		}
	return false;
}
												//	add section to list in correct order

void SectionsInfo::AddSection( Section *add, char *src )
{
	SectionInfo *s, *c;
	bool endoflist = false;

	if ( add )
		{
		s = first;									// find where sectnum would be (is) in list
		while ( s )
			{
			endoflist = false;
			if ( add->index == s->index ) break;
			if ( add->index > s->index ) s = s->next;
			else { s = s->prev;  break; }
			endoflist = true;
			}

		c = new SectionInfo();						// create info to be added
		c->index = add->index;
		c->thickness = add->thickness;
		c->alignLocked = add->alignLocked;
		strcpy(c->filename,src);
		if ( add->HasImage() )
			{
			c->pixelsize = add->MinPixelsize();
			add->ImageSize( c->min, c->max );	// find extremes of section
			}
		else
			{
			c->min.x = 0.0;							// section has no image, so set something reasonable
			c->max.x = 0.0;
			c->min.y = 0.0;
			c->max.y = 0.0;
			c->pixelsize = ImagePixelSize;
			}
		
		if ( !endoflist )							// add belongs at head or middle of list
			{
			Insert( c, s );							// insert after section
			if ( s )
			  if ( add->index == s->index )			// remove existing item from list
				{
				Extract( s );
				delete s;
				}
			}
		else Add( c );								// otherwise add to tail of list
		}

}

void SectionsInfo::DeleteSection( int sectnum )	//	delete section from list
{
	SectionInfo *s;

	s = first;									// find in list
	while ( s )
		{
		if ( sectnum == s->index ) break;
		else s = s->next;
		}

	if ( s )									// extract and delete it
		{
		Extract( s );
		delete s;
		}
}

double SectionsInfo::ZDistance( int sectnum, bool middle )	//	find total distance to sectnum,
{															// starting from first nonzero section
	SectionInfo *s;
	double distance;

	distance = 0.0;
	s = first;	
	if ( !s->index ) s = s->next;				// skip section zero
	while ( s )
		{
		if ( s->index == sectnum )
			if ( middle ) distance += s->thickness/2.0;	// add half of the thickness
			else distance += s->thickness;		// use whole thickness to get distance to top of section
		else if ( s->index < sectnum )
			distance += s->thickness;			// add in section thickness for a lower section
		else break;								// done when have passed sectnum
		s = s->next;
		}
	return( distance );
}

void SectionsInfo::SetThickness( int sectnum, double thickness )
{												// give sectnum a new thickness value
	SectionInfo *s;
	
	s = first;	
	while ( s )
		{
		if ( sectnum == s->index )
			{
			s->thickness = thickness;			// replace section thickness for this section
			break;
			}
		s = s->next;
		}
}

void SectionsInfo::SetLock( int sectnum, bool lock )
{												// give sectnum a new alignLocked state
	SectionInfo *s;
	
	s = first;	
	while ( s )
		{
		if ( sectnum == s->index )
			{
			s->alignLocked = lock;			// replace alignLocked for this section
			break;
			}
		s = s->next;
		}
}
