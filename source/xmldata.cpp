///////////////////////////////////////////////////////////////////////////
//	This file contains the methods for XML_DATA class for parsing ASCII XML
//
//    Copyright (C) 2002-2006  John Fiala (fiala@bu.edu)
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
// modified 3/15/06 by JCF (fiala@bu.edu)
// -+- change: Added limit to loops in getString and getNextDouble.
//             Guard against reaching end of file unexpectedly.
//

#include "reconstruct.h"


XML_DATA::XML_DATA( char * filename )								// intialize XML_DATA from file
{
	HANDLE hFile;
	DWORD didRead, dwSize;
	char errmsg[1024];

	data = NULL; size = 0;											// if fail, this will be NULL
																	// attempt to open file using Win32 call
 
	hFile = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL,
							OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		{															// if fail, format error string
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							errmsg, 512, NULL );
		strcat(errmsg,filename);
		ErrMsgOK( ERRMSG_READ_FAILED, errmsg );
		return;
		}
																	// get size (in bytes) of file
	dwSize = GetFileSize (hFile, NULL) ; 
	if ( dwSize == 0xFFFFFFFF ) { 
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							errmsg, 512, NULL );
		strcat(errmsg,filename);
		ErrMsgOK( ERRMSG_READ_FAILED, errmsg );
		CloseHandle( hFile );
		return;

		}

	data = (char *)GlobalAllocOrDie(dwSize);						// alloc memory for whole file

	ReadFile( hFile, data, dwSize, &didRead, NULL );				// read entire file into memory
	size = (unsigned int)didRead;
	CloseHandle( hFile );
}

XML_DATA::~XML_DATA()
{
	if ( data ) free( data );
}

char * XML_DATA::findString( char *start, char *pattern, char *stopper )
{															// search sequentially from start for "pattern"
	int	j, k;												// but fail if encounter "stopper" string first
	char *i;												// return 0 when "pattern" not found
	j = 0; k = 0; i = start;
	if ( i )
	  while ( i < (data+size) )								// never search past end of data
		{
		if ( *i == pattern[j] ) { j++; if ( pattern[j] == '\0' ) return(i+1); }	// pattern found!
		else  j = 0;															// restart pattern search
		if ( *i == stopper[k] ) { k++; if ( stopper[k] == '\0' ) return(0); }	// stopper found!
		else  k = 0;															// restart stopper search
		i++;
		}
    return 0;												// neither pattern nor stopper found
}

char * XML_DATA::openQuote( char *src )
{
	if ( src )
		{
		while ( (*src != '\"') && (src < (data+size)) ) src++;	// find open quote
		if ( src < (data+size) ) return( src+1 );
		}
	return( 0 );
}

void XML_DATA::getString( char *dest, char *src, int max_len ) // retrieve attribute string from quotes
{
	char *i, *l;
	int j, k;
	i = openQuote(src);
	if ( i )
		{
		j = 0;
		l = i;
		while ( ( *l != '\"') && ( *l != '>' ) && (l < (data+size)))	// find close quote
			{ j++; l++; }
		if ( j >= max_len ) j = max_len-1;				// limit to max length
		for ( k=0; k<j; k++ ) dest[k] = i[k];			// copy the characters
		dest[k] = '\0';
		}
}

double XML_DATA::getdouble( char *src )		// retrieve doubleing point attribute from quotes
{
	src = openQuote( src );
	if ( src ) return( (double)atof(src) );						// convert to double
	return( 0.0 );
}

int XML_DATA::getInt( char *src )			// retrieve integer attribute from quotes
{
	src = openQuote( src );
	if ( src ) return( atoi(src) );							// convert to int
	return( 0 );
}

bool XML_DATA::getBool( char *src )			// retrieve integer attribute from quotes
{
	src = openQuote( src );
	if ( src )
	  while ( *src != '\"' ) 						// a 't' or 'T' before close quote is true
		{
		if ( (*src == 't') || ( *src == 'T' ) ) return(true);
		src++;
		}
	return false;									// otherwise false
}

char * XML_DATA::getNextdouble( double *f, char *src )	// get next doubleing point attribute from quotes
{
	if ( src )
	  {
	  while ( src < (data+size) )
		{											// skip non-double stuff
		if ( *src == '\"' ) return 0;
		if ( isdigit(*src) || (*src == '+') || (*src == '-') || (*src == '.') ) break;
		src++;
		}
	  if ( src < (data+size) ) *f = (double)atof(src); // scan over this double to advance to next one
	  while ( (src < (data+size)) && ( isdigit(*src) || (*src == '+') || (*src == '-') 
										|| (*src == '.') || (*src == 'e') || (*src == 'E') ) ) src++;
	  if ( src < (data+size) ) return src;
	  }
	return (0);
}
