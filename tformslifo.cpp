///////////////////////////////////////////////////////////////////////////////
//	This file contains the methods for the undo LIFOs on Transforms lists
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

TformsLIFO::TformsLIFO()								// initial the index and lifo array to NULLs
{
	last = -1;
	for ( int i=0; i<MAX_UNDOS; i++ ) lifo[i] = NULL;
}

TformsLIFO::~TformsLIFO()								// delete all lifo entries
{
	for (int i=0; i<MAX_UNDOS; i++)
		if ( lifo[i] ) delete lifo[i];	
}

TformsLIFO::TformsLIFO( TformsLIFO &copyfrom )			// PROBABLY WONT USE THIS
{
	last = copyfrom.last;
	for ( int i=0; i<MAX_UNDOS; i++ )
		if ( copyfrom.lifo[i] ) lifo[i] = new Transforms( *(copyfrom.lifo[i]) );
		else lifo[i] = NULL;
}

void TformsLIFO::Push( Transforms *transforms )			// save transforms in lifo
{
	last++;												//  put copy of transforms into LIFO
	if ( last >= MAX_UNDOS ) last = 1;					// wrap around if lifo full, but dont overwrite 0
	if ( lifo[last] ) delete lifo[last];				// clear (second) oldest lifo entry
	lifo[last] = new Transforms( *transforms );			// put copy of trnasforms into LIFO
}

Transforms * TformsLIFO::Pop( void )					// recall last lifo entry
{
	Transforms *tmp;
	if ( last < 0 ) return( NULL );						// return NULL if lifo is empty

	tmp = lifo[last];									// get item to return
	lifo[last] = NULL;									// clear it from LIFO (but dont delete becuz will use it)
	last--;												// backup one LIFO item
	if ( last >= 0 )									// still have entries in lifo?
		{
		if ( last == 0 ) last = MAX_UNDOS-1;			// test wrapping around lifo
		if ( lifo[last] == NULL ) last = 0;				// if no items remain, go to first entry
		}
	return( tmp );
}

void TformsLIFO::Clear( void )							// clear all lifo entries and reset index
{
	last = -1;
	for (int i=0; i<MAX_UNDOS; i++)
		{
		if ( lifo[i] ) delete lifo[i];
		lifo[i] = NULL;
		}
}

Transforms * TformsLIFO::Reset( void )					// return the first lifo entry and clear
{
	Transforms *tmp;
	tmp = lifo[0];
	lifo[0] = NULL;										// by setting this to NULL, Clear() wont delete it!
	this->Clear();										// delete all other lifo entries
	return( tmp );	
}

Transforms * TformsLIFO::First( void )					// return the first lifo entry but don't remove anything
{
	return( lifo[0] );	
}
