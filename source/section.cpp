////////////////////////////////////////////////////////////////////////
//	This file contains the methods for the Section class
//
//    Copyright (C) 2002-2007  John Fiala (fiala@bu.edu)
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
// modified 4/29/05 by JCF (fiala@bu.edu)
// -+- change: Added method AddDXFLines() to read polylines and circles from DXF file.
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change: Added methods HasImage, HasContour, MinPixelsize, and ContourSize; modified ImageSize method.
// modified 7/15/05 by JCF (fiala@bu.edu)
// -+- change: Changed text of save section warning message to remove term "deleting".
// modified 2/28/06 by JCF (fiala@bu.edu)
// -+- change: Modified SetDefaultName params.
// modified 3/15/06 by JCF (fiala@bu.edu)
// -+- change: Modified to prevent crash when loading corrupted XML file.
// modified 5/11/06 by JCF (fiala@bu.edu)
// -+- change: Modified AddDXFLines to fix bug, recognize more entities.
// modified 5/23/06 by JCF (fiala@bu.edu)
// -+- change: Added TEXT points and color recognition to AddDXFLines.
// modified 11/14/06 by JCF (fiala@bu.edu)
// -+- change: Added new method AddViewPortContour() to replace JU's AddContour global function.
// modified 2/5/07 by JCF (fiala@bu.edu)
// -+- change: Added some debug logging.
// modified 3/15/07 by JCF (fiala@bu.edu)
// -+- change: Added color channel mask input/output to Images.
// modified 4/2/07 by JCF (fiala@bu.edu)
// -+- change: Modified TestGBMfile to also return the number of images in an image stack
// modified 4/5/07 by JCF (fiala@bu.edu)
// -+- change: Corrected initial domain boundary extent in AddNewImage()
//

#include "reconstruct.h"

Section::Section()					// empty constructor
{
	index = 1;
	if ( CurrSeries ) thickness = CurrSeries->defaultThickness;
	else thickness = 0.1;
	transforms = NULL;
	active = NULL;
	undos = NULL;					// create undo lifo only as needed
	redo = NULL;
	hasChanged = false;
	alignLocked = false;
}

Section::Section( Section &copyfrom )							// copy constructor
{
	index = copyfrom.index;
	thickness = copyfrom.thickness;
	alignLocked = copyfrom.alignLocked;
	if ( copyfrom.transforms ) transforms = new Transforms( *(copyfrom.transforms) );
	else transforms = NULL;
	active = NULL;
	undos = NULL;
	redo = NULL;
	hasChanged = false;
}

Section::~Section()												// destructor frees memory of transforms
{
	if ( transforms ) delete transforms;
	if ( undos ) delete undos;
	if ( redo ) delete redo;
}
																// the following routines are local routines
																// designed to simplify loading from file

char * LoadSectionContour( XML_DATA *xml, char *begin_contour, Contour *contour )
{
	Point *p;													// load contour from XML data
	double x, y;
	char *begin_attr, *end_contour;

	begin_attr = xml->findString(begin_contour,"name=","/>");
	if ( begin_attr ) xml->getString( contour->name, begin_attr, MAX_CONTOUR_NAME );
	begin_attr = xml->findString(begin_contour,"comment=","/>");
	if ( begin_attr ) xml->getString( contour->comment, begin_attr, MAX_COMMENT );
	begin_attr = xml->findString(begin_contour,"hidden=",">");
	if ( begin_attr ) contour->hidden = xml->getBool( begin_attr );
	begin_attr = xml->findString(begin_contour,"closed=",">");
	if ( begin_attr ) contour->closed = xml->getBool( begin_attr );
	begin_attr = xml->findString(begin_contour,"simplified=",">");
	if ( begin_attr ) contour->simplified = xml->getBool( begin_attr );
	begin_attr = xml->findString(begin_contour,"border=",">");
	if ( begin_attr ) {
		begin_attr = xml->openQuote( begin_attr );
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->border.r = x;
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->border.g = x;
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->border.b = x;
		}
	begin_attr = xml->findString(begin_contour,"fill=",">");
	if ( begin_attr ) {
		begin_attr = xml->openQuote( begin_attr );
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->fill.r = x;
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->fill.g = x;
		if ( begin_attr ) begin_attr = xml->getNextdouble( &x, begin_attr );
		if ( begin_attr ) contour->fill.b = x;
		}
	begin_attr = xml->findString(begin_contour,"mode=",">");
	if ( begin_attr ) contour->mode = xml->getInt( begin_attr );
	begin_attr = xml->findString(begin_contour,"points=","/>");
	if ( begin_attr )
		{
		contour->points = new Points();
		begin_attr = xml->openQuote( begin_attr );	// read and store contour points
		while ( begin_attr )
			{
			begin_attr = xml->getNextdouble( &x, begin_attr );
			if ( begin_attr )
				{
				begin_attr = xml->getNextdouble( &y, begin_attr );
				if ( begin_attr )
					{
					p = new Point();
					p->x = x;
					p->y = y;
					//p->z = (double)section->index;
					contour->points->Add(p);
					}
				}
			}										// until reach end of list
		}
	end_contour = xml->findString(begin_contour,"/>","<Contour");
	return( end_contour );
}

char * LoadImage( XML_DATA *xml, char *begin_image, Image *image )
{																// load image from XML data
	char *begin_attr, *end_image;
	
	begin_attr = xml->findString(begin_image,"mag=","/>");
	if ( begin_attr ) image->mag = xml->getdouble( begin_attr );
	begin_attr = xml->findString(begin_image,"contrast=","/>");
	if ( begin_attr ) image->contrast = xml->getdouble( begin_attr );
	else image->contrast = 1.0;
	begin_attr = xml->findString(begin_image,"brightness=","/>");
	if ( begin_attr ) image->brightness = xml->getdouble( begin_attr );
	else image->brightness = 0.0;
	begin_attr = xml->findString(begin_image,"red=",">");
	if ( begin_attr ) image->red = xml->getBool( begin_attr );
	begin_attr = xml->findString(begin_image,"green=",">");
	if ( begin_attr ) image->green = xml->getBool( begin_attr );
	begin_attr = xml->findString(begin_image,"blue=",">");
	if ( begin_attr ) image->blue = xml->getBool( begin_attr );
	begin_attr = xml->findString(begin_image,"src=","/>");
	if ( begin_attr ) xml->getString( image->src, begin_attr, MAX_PATH );
	begin_attr = xml->findString(begin_image,"proxy_src=","/>");
	if ( begin_attr ) xml->getString( image->proxySrc, begin_attr, MAX_PATH );
	begin_attr = xml->findString(begin_image,"proxy_scale=","/>");
	if ( begin_attr ) image->proxyScale = xml->getdouble( begin_attr );
	end_image = xml->findString(begin_image,"/>","</Transform");
	return( end_image );
}

char * LoadTransform( XML_DATA *xml, char *begin_transform, Transform *transform, bool &hasChanged )
{
	int i;														// load transform from XML data
	double f;
	Contour *contour;
	char *begin_attr, *end_transform, *begin_element, *end_element;

	begin_attr = xml->findString(begin_transform,"dim=",">");	// read number of significant terms
	if ( begin_attr ) transform->nform->dim = xml->getInt( begin_attr );
	else transform->nform->dim = DIM;

	begin_attr = xml->findString(begin_transform,"xcoef=",">");	// read xcoef if present
	if ( begin_attr )
		{
		begin_attr = xml->openQuote( begin_attr );
		if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
		i = 0;
		while ( begin_attr && (i < DIM) )
			{
			transform->nform->a[i] = f;
			i++;
			begin_attr = xml->getNextdouble( &f, begin_attr );
			}
		}
	begin_attr = xml->findString(begin_transform,"ycoef=",">");	// read ycoef if present
	if ( begin_attr )
		{
		begin_attr = xml->openQuote( begin_attr );
		if ( begin_attr ) begin_attr = xml->getNextdouble( &f, begin_attr );
		i = 0;
		while ( begin_attr && (i < DIM) )
			{
			transform->nform->b[i] = f;
			i++;
			begin_attr = xml->getNextdouble( &f, begin_attr );
			}
		}
																			// find image if present
	begin_element = xml->findString(begin_transform,"<Image","</Transform");
	if ( begin_element )
		{
		transform->image = new Image();											// create image
		LoadImage( xml, begin_element, transform->image );		// and load it
		}

	if ( transform->image )													// if Image, then get single domain contour
		{
		begin_element = xml->findString(begin_transform,"<Contour","</Transform");
		if ( begin_element )
			{
			transform->domain = new Contour();									// create domain
			LoadSectionContour( xml, begin_element, transform->domain );
			if ( CurrSeries->hideDomains && !transform->domain->hidden )		// compare hidden state with onLoad flag
				{
				transform->domain->hidden = true;								// if needed mark as hidden
				hasChanged = true;												// and flag change so user can autosave it
				}
			else if ( CurrSeries->unhideDomains && transform->domain->hidden )	// both switches will toggle all states
				{
				transform->domain->hidden = false;								// unhide domain
				hasChanged = true;												// and mark as changed
				}
			}
		}
	else																	// else get set of contours when no image
		{
		begin_element = xml->findString(begin_transform,"<Contour","</Transform");
		if ( begin_element )
			{
			transform->contours = new Contours();
			while ( begin_element )												// for each contour found
				{
				contour = new Contour();										// create contour and read from file
				end_element = LoadSectionContour( xml, begin_element, contour );
				if ( CurrSeries->hideTraces && !contour->hidden )				// compare hidden state with onLoad flag
					{
					contour->hidden = true;										// if needed mark as hidden
					hasChanged = true;											// and flag change so user can autosave it
					}
				else if ( CurrSeries->unhideTraces && contour->hidden )			// both switches will toggle all states
					{
					contour->hidden = false;									// unhide contour
					hasChanged = true;											// and mark as changed
					}
				transform->contours->Add(contour);								// add contour to Transform's list
				begin_element = xml->findString(end_element,"<Contour","</Transform");
				}
			}
		}
																			// find end of transform
	end_transform = xml->findString(begin_transform,"</Transform","</Section");
	return( end_transform );
}

Section::Section( char * sectionfile )									// Constructor for Section from file!
{														
	XML_DATA *xml;
	Transform *transform;
	char *begin_element, *begin_attr, *end_element;
// begin debug logging...
	DWORD byteswritten;
	char line[MAX_PATH];
	if ( debugLogFile != INVALID_HANDLE_VALUE )
		{
		sprintf(line,"Entered new Section( %s )\r\n",sectionfile);
		WriteFile(debugLogFile, line, strlen(line), &byteswritten, NULL);
		}
// ...end debug logging
	index = -1;															// initial default values
	if ( CurrSeries ) thickness = CurrSeries->defaultThickness;
	else thickness = 0.1;
	transforms = NULL;
	active = NULL;
	undos = NULL;
	redo = NULL;
	hasChanged = false;
	alignLocked = false;

	xml = new XML_DATA( sectionfile );											// read entire file into memory
	if ( xml->data == NULL ) 
		{
		delete xml;
		return;											// if no file data, then return empty
		}

	begin_element = xml->findString(xml->data,"<Section","</Section");			// search for start of section
	if ( begin_element )	
		{																		// fill section from attributes
		begin_attr = xml->findString(begin_element,"index=",">");
		if ( begin_attr ) this->index = xml->getInt( begin_attr );
		begin_attr = xml->findString(begin_element,"thickness=",">");
		if ( begin_attr ) this->thickness = xml->getdouble( begin_attr );
		begin_attr = xml->findString(begin_element,"alignLocked=",">");
		if ( begin_attr ) this->alignLocked = xml->getBool( begin_attr );
		end_element = xml->findString(begin_element,">","</Section");			// advance ptr to section '>'

		begin_element = xml->findString(end_element,"<Transform","</Section");
		if ( begin_element )													// add all transforms for section
			{
			this->transforms = new Transforms();
			while ( begin_element )
				{
				transform = new Transform();											// create transform
				end_element = LoadTransform( xml, begin_element, transform, hasChanged );
				this->transforms->Add(transform);										// add transform to Section list
				begin_element = xml->findString(end_element,"<Transform","</Section");
				}
			}
		}
	else ErrMsgOK( ERRMSG_READ_FAILED, sectionfile );

	delete xml;																		// free xml memory
}

bool SaveSectionContour( HANDLE hFile, Contour *contour )		// save a section contour
{
	Point *p;
	DWORD byteswritten;
	char line[MAX_PATH];
	bool error = false;

	sprintf(line,"<Contour name=\"%s\"", contour->name);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
	if ( strlen(contour->comment) )
		{
		sprintf(line," comment=\"%s\"", contour->comment);
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
		}
	if ( contour->hidden ) sprintf(line," hidden=\"true\"");
	else sprintf(line," hidden=\"false\"");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
	if ( contour->closed ) sprintf(line," closed=\"true\"");
	else sprintf(line," closed=\"false\"");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
	if ( contour->simplified ) sprintf(line," simplified=\"true\"");
	else sprintf(line," simplified=\"false\"");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
	sprintf(line," border=\"%g %g %g\" fill=\"%g %g %g\" mode=\"%d\"",
					contour->border.r, contour->border.g, contour->border.b,
						contour->fill.r, contour->fill.g, contour->fill.b, contour->mode );
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
	if ( contour->points )
		{
		sprintf(line,"\r\n points=\"");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
		p = contour->points->first;
		while ( p != NULL )
			{														// output at most 'precision' significant digits
			sprintf(line,"%1.*g %1.*g,\r\n\t",Precision,p->x,Precision,p->y);
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
			p = p->next;
			}
		sprintf(line,"\"");
		if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;
		}
	sprintf(line,"/>\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) error = true;

	return error;
}

void Section::Save( char * sectionfile )
{													
	Transform *transform;
	Image *image;
	Contour *contour;
	int i, j;
	HANDLE hFile;
	DWORD byteswritten;
	bool ErrorOccurred = false;
	bool SaveIt;
	char line[MAX_PATH], errmsg[1024];
																	// attempt to open file using Win32 call
 
	hFile = CreateFile( sectionfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		{											// fail, format error string
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							errmsg, 512, NULL );
		strcat(errmsg, sectionfile );
		ErrMsgOK( ERRMSG_WRITE_FAILED, errmsg );
		return;
		}															// write file line by line...
	
	sprintf(line,"<?xml version=\"1.0\"?>\r\n<!DOCTYPE Section SYSTEM \"section.dtd\">\r\n\r\n");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	if ( alignLocked )
		sprintf(line,"<Section index=\"%d\" thickness=\"%1.*g\" alignLocked=\"true\">\r\n",this->index,Precision,this->thickness);
	else sprintf(line,"<Section index=\"%d\" thickness=\"%1.*g\" alignLocked=\"false\">\r\n",this->index,Precision,this->thickness);
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	if ( this->transforms )
		{
		transform = this->transforms->first;
		while ( transform != NULL )									// output transform parameters
		  {
		  SaveIt = false;											// only write out transforms with data
		  if ( transform->image ) SaveIt = true;
		  else if ( transform->contours ) if ( transform->contours->Number() > 0 ) SaveIt = true;
		  if ( SaveIt )
			{
			sprintf(line,"<Transform");
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
			if ( transform->nform ) {
				sprintf(line," dim=\"%d\"\r\n xcoef=\"",transform->nform->dim);
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
				for (i=0; i<DIM; i++) {
					sprintf(line," %1.*g",MAX_SIGDIGITS,transform->nform->a[i]);
					if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
					}
				sprintf(line,"\"\r\n ycoef=\"");
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
				for (i=0; i<DIM; i++) {
					sprintf(line," %1.*g",MAX_SIGDIGITS,transform->nform->b[i]);
					if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
					}
				sprintf(line,"\"");
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
				}
			sprintf(line,">\r\n");
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
			
			if ( transform->image )									// output image element
				{
				sprintf(line,"<Image mag=\"%1.*g\" contrast=\"%g\" brightness=\"%g\"",
										Precision,transform->image->mag,transform->image->contrast,transform->image->brightness);			
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
				if ( transform->image->red ) sprintf(line," red=\"true\"");
				else sprintf(line," red=\"false\"");
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
				if ( transform->image->green ) sprintf(line," green=\"true\"");
				else sprintf(line," green=\"false\"");
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
				if ( transform->image->blue ) sprintf(line," blue=\"true\"");
				else sprintf(line," blue=\"false\"");
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
				sprintf(line,"\r\n src=\"%s\"", transform->image->src);			
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

				if ( strlen(transform->image->proxySrc) )			// if proxy, output its attributes
					{
					sprintf(line," proxy_src=\"%s\"", transform->image->proxySrc);			
					if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
					sprintf(line," proxy_scale=\"%1.*g\"", Precision,transform->image->proxyScale);			
					if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
					}
																		// close element attributes
				sprintf(line," />\r\n");
				if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

				if ( transform->domain )							// output image domain contour
					ErrorOccurred = SaveSectionContour( hFile, transform->domain );
				}
			else {													// output contours
				if ( transform->contours )
					{
					contour = transform->contours->first;
					while ( contour != NULL )				// PROBLEM: Won't know until write output
						{									// whether transform contains any contours!
						ErrorOccurred = SaveSectionContour( hFile, contour );
						contour = contour->next;
						}
					}
				}													// close transform
			sprintf(line,"</Transform>\r\n\r\n");
			if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;
			}
		  transform = transform->next;
		  }
		}
																	// close section
	sprintf(line,"</Section>");
	if ( !WriteFile(hFile, line, strlen(line), &byteswritten, NULL) ) ErrorOccurred = true;

	CloseHandle( hFile );

	if ( ErrorOccurred ) {
		ErrMsgOK( ERRMSG_WRITE_FAILED, sectionfile );
		}
	else hasChanged = false;						// signal that file is up to date
}

bool Section::SaveIfNeeded( void )					// check to save Series before deleting
{
	int status;
	char filename[MAX_PATH], msgtxt[MAX_PATH];

	sprintf(filename,"%s%s.%d",WorkingPath,BaseName,index);
																// save if hasChanged and autosave
	if ( hasChanged )
		if ( CurrSeries->autoSaveSection ) Save( filename );
		else if ( CurrSeries->warnSaveSection )					// or if user wants to save
			{
			sprintf(msgtxt,"Save changes to section %d?", index);
			status = MessageBox(appWnd,msgtxt,"WARNING",MB_YESNOCANCEL|MB_ICONWARNING);

			if ( status == IDCANCEL ) return false;				// fail if user cancels operation
			if ( status == IDYES ) Save( filename );
			}
	return true;												// if no cancel, return true
}

void Section::AddNewContour( Contour *contour )		// add a newly created contour in section units to active transform
{
	Point *p;
	this->PushUndoState();							// save section undo before modifying

	if ( !active )									// if there already is an active transform, use it
		{
		active = new Transform();					// otherwise create a new one and add it to list
		if ( !transforms ) transforms = new Transforms();
		transforms->Add( active );
		}											// (if new one created, it will not be a domain)
	active->isActive = true;						// set state flags
	hasChanged = true;

	p = contour->points->first;						// clear any z-values from 2D section representation
	while ( p )										// (mouse routines use z to track sections while tracing)
		{
		p->z = 0.0;
		p = p->next;
		}

	contour->FwdNform( active->nform );				// put contour into transform coordinates

	if ( active->domain && active->image )			// this is a domain, replace domain contour
		{
		contour->Scale( 1.0/active->image->mag );	// scale contour into image pixels
		delete active->domain;
		active->domain = contour;					// set as domain
		contour->closed = true;						// a domain contour is closed (but not selected)
		contour->mode = R2_NOP;						// a domain is never filled!
		}											// otherwise, just add it to active contours list
	else
		{											// create active contours list if it doesn't exist
		if ( !active->contours ) active->contours = new Contours();
		active->contours->Add( contour );
		}
}
													// update defname w/tokens and output next name w/o tokens

void Section::SetDefaultName( char *name, char *defname, bool isForDomain )
{
	Transform *transform;
	Contour *contour, *max_contour;
	char insert[MAX_CONTOUR_NAME], modified[MAX_CONTOUR_NAME];
	int i, j, k, maxi, n, max_n, s;
	int token;										// tokens are sets of special chars: "+", "$", "$+"

	if ( CurrSeries )
		{											// and modify it if necessary
		for ( j=0; j<MAX_CONTOUR_NAME; j++ )
			{
			name[j] = '\0';							// clear output string
			modified[j] = '\0';						// and modified defaultName
			}
		i = 0; j = 0; k = 0;
		maxi = strlen(defname);
		while ( i < maxi )							// search for special char sequences
			{										// determine type of sequence
			token = 0;
			while ( (defname[i] == '$') || (defname[i] == '+') )
				{
				if ( (defname[i] == '$') && (token == 0) ) token = 1;
				if ( (defname[i] == '+') && (token == 0) ) token = 2;
				if ( (defname[i] == '$') && (token == 2) ) token = 3;
				if ( (defname[i] == '+') && (token == 1) ) token = 3;
				i++;
				}									// move to end of token in default name

			switch (token) {
			  case 1:								// replace token with section number
				s = -1;									// determine if there is a preceding integer
				while ( (s+j) >= 0 )					// will use number of digits to zero pad output
					if ( isdigit(name[s+j]) ) s--;		// of immediately preceding number
					else break;
				s++;
				sprintf(insert,"%0*d",abs(s),this->index);	// form zero-padded string of index
				sprintf(name+j+s,"%s",insert);			// tack onto result overwriting digits
				j += s + strlen(insert);				// incr ptr into string
				strcat(modified,"$");					// output token to modified name string
				k++;
				break;

			  case 2:								// increment preceding integer
				s = -1;									// determine offset to start of number
				while ( ((s+j) >= 0) && (s>-10) )		// limit to max of 10 digits
					if ( isdigit(name[s+j]) ) s--;
					else break;
				s++;
				n = atoi( name+j+s );					// extract integer
				sprintf(insert,"%0*d",abs(s),n+1);		// left pad with zeros to make at least abs(s) # of chars
				sprintf(name+j+s,"%s",insert);			// tack onto result
				j += s + strlen(insert);				// incr ptr into string
				sprintf(modified+k+s,"%s+",insert);		// replace also in defaultName
				k += s + strlen(insert) + 1;
				break;

			  case 3:								// increment within the section
				s = -1;									// determine starting point if none in section
				while ( ((s+j) >= 0) && (s>-10) )		// limit to 10 digits
					if ( isdigit(name[s+j]) ) s--;
					else break;
				s++;
				max_n = atoi( name+j+s );				// extract integer
				if ( transforms )	// now search every transform in section
					{
					transform =  transforms->first;
					while ( transform )	
					  {									// for each transform search domain and contours...
					  if ( transform->domain && isForDomain )	// first, check domain if it exists
						{
						contour = transform->domain;
						if ( strncmp( contour->name, name, j+s) == 0 )		// found matching name
							{
							n = atoi(contour->name+j+s);					// compare integer values
							if ( n > max_n ) max_n = n;
							}
						}
					  if ( transform->contours && !isForDomain ) // next, check contours if they exist
						{
						contour = transform->contours->first;
						while ( contour )							
							{
							if ( strncmp( contour->name, name, j+s) == 0 )	// found matching name
								{
								n = atoi(contour->name+j+s);				// compare integer values
								if ( n > max_n ) max_n = n;
								}
							contour = contour->next;	// go to next contour
							}
						}
					  transform = transform->next;		// do next transform
					  }
					}
				sprintf(insert,"%0*d",abs(s),max_n+1);	// pad w/zeros for at least abs(s) # of chars
				sprintf(name+j+s,"%s",insert);			// tack onto result
				j += s + strlen(insert);				// incr ptr into string
				strcat(modified,"$+");					// put token also in defaultName
				k += 2;
				break;

			  case 0:								// just copy from default string
				name[j] = defname[i];
				modified[k] = defname[i];
				i++; j++; k++;							// move to next chars
			  }
			}
		strcpy(defname,modified);					// replace defaultName with modified default		
		}
}

Transform * Section::AddNewImage( char * imagefile, double mag ) // add a new image to this section
{
	Transform *transform;
	int ft, w, h, bpp, n;
	Point *p;

	ft = TestGBMfile( imagefile, &w, &h, &bpp, &n );	// attempt read image file
	if ( ft ) 											// filetype was recognized
		{
		transform = new Transform();					// create a new transform & add it to list
		if ( !transforms ) transforms = new Transforms();
		transforms->Add( transform );
														// create new image and set components
		transform->image = new Image();
		strcpy( transform->image->src, imagefile );
		transform->image->mag = mag;	
		transform->image->adib = NULL;					// only read when regenerate view
		
		transform->domain = new Contour();				// create domain contour that corresponds to image
		CurrSeries->SetDefaultAttributes( transform->domain );	// use current defaults
		this->SetDefaultName(transform->domain->name,CurrSeries->defaultName,true);
		transform->domain->mode = R2_NOP;				// don't fill a domain
		transform->domain->closed = true;				// but use a closed boundary
		transform->domain->points = new Points(); 
		p = new Point(0.0,0.0,0.0);	
		transform->domain->points->Add(p);
		p = new Point((double)(w-1),0.0,0.0);
		transform->domain->points->Add(p);
		p = new Point((double)(w-1),(double)(h-1),0.0);
		transform->domain->points->Add(p);
		p = new Point(0.0,(double)(h-1),0.0);
		transform->domain->points->Add(p);
		hasChanged = true;
		return( transform );
		}
	
	return( NULL );								
}

void Section::AddViewPortContour( Contour *contour )	// add contour in view's pixel coordinates to section
{												
	if ( contour && CurrSeries )
		{
		CurrSeries->SetDefaultAttributes( contour );		// use current defaults
		this->SetDefaultName(contour->name,CurrSeries->defaultName,FrontView==DomainView);
		contour->Shift( 0.5, 0.5 );							// shift onto pixel centers before rescaling
		contour->Scale( CurrSeries->pixel_size );			// convert pixels into section units
		contour->Shift( CurrSeries->offset_x, CurrSeries->offset_y );
		this->AddNewContour( contour );						// put into active contours
		}
}


Transform * Section::ExtractActiveTransform( void )
{
	Transform *extracted;								// extract active transform from transforms list

	extracted = NULL;

	if ( active && transforms )							// do only if there is active AND transforms list
		{
		extracted = active;
		active = NULL;									// clear active transform
		extracted->isActive = false;
		transforms->Extract( extracted );
		hasChanged = true;
		}

	return( extracted );
}


void Section::SelectAll( void )						// make all contours active and selected
{
	Transform *transform, *toremove;
	Contour *contour, *toactive;

	if ( transforms )								// now move every contour in section
		{
		transform = transforms->first;				// for each transform move contours
		while ( transform )							// but don't move active or image ones
			if ( (transform != active) && (transform->image == NULL) && transform->contours )
				{
				contour = transform->contours->first;		// retrieve first contour
				while ( contour )
				  if ( !contour->hidden )							
					{
					if ( !active )							// if there already is an active transform, add to it
						{
						active = new Transform();			// otherwise create a new one and add it to list
						if ( !transforms ) transforms = new Transforms();
						transforms->Add( active );
						}									// create active contours list if it doesn't exist
					if ( !active->contours ) active->contours = new Contours();
					active->isActive = true;
					toactive = contour;						// will move toactive to active transform
					contour = contour->next;				// but first get next one in list
					transform->contours->Extract( toactive );
					toactive->InvNform( transform->nform );	// transform contour into section coord.
					toactive->FwdNform( active->nform );	// then into active coordinates
					active->contours->Add( toactive );		// move contour to the active transform
					}
				  else contour = contour->next;

				if ( transform->contours->Number() )		// all contours selected?
					transform = transform->next;			// if yes, delete transform!
				else {
					toremove = transform;					// remove transform since its empty...
					transform = transform->next;			// first determine next transform
					transforms->Extract( toremove );		// extract old transform from list
					delete toremove;						// and then delete it from memory
					}
				}
			else transform = transform->next;
		}
}

void Section::UnSelectAll( void )				// remove all active contours
{												// and clear active transform pointer
	if ( active )
		active->isActive = false;
	active = NULL;								// note: active is an item in transforms list so don't delete
}

Contour * Section::FindClosestContour( double x, double y )
{
	Transform *transform;							// locate and return closest contour
	Contour *contour, *nearest_contour;
	double d, min_d, transX, transY;

	min_d = MAX_FLOAT;
	nearest_contour = NULL;

	if ( this->transforms )
		{
		transform = this->transforms->first;
		while ( transform )							// check each transform for contours
			{
			if ( transform->contours )
				{
				transX = transform->nform->X( x, y );	// put x,y into transform coord
				transY = transform->nform->Y( x, y );
														// test each contour
				contour = transform->contours->first;
				while ( contour )
					{
					if ( !contour->hidden )
					  {
					  d = contour->Distance( transX, transY );						
					  if ( d < min_d )
						{
						min_d = d;
						nearest_contour = contour;
						}
					  }
					contour = contour->next;			// do next contour in list
					}
				}
			transform = transform->next;
			}
		}

	return nearest_contour;
}

bool Section::SelectClosestContour( double x, double y )
{
	Transform *transform, *nearest_transform;		// locate closest contour and put in active transform
	Contour *contour, *nearest_contour;
	double d, min_d, transX, transY;

	min_d = MAX_FLOAT;
	nearest_contour = NULL;
	nearest_transform = NULL;

	if ( this->transforms )
		{
		transform = this->transforms->first;
		while ( transform )							// check each transform for contours
			{
			if ( transform->contours )
				{
				transX = transform->nform->X( x, y );		// put x,y into transform coord
				transY = transform->nform->Y( x, y );
															// test each contour
				contour = transform->contours->first;
				while ( contour )
					{
					if ( !contour->hidden )
					  {
					  d = contour->Distance( transX, transY );						
					  if ( d < min_d )
						{
						min_d = d;
						nearest_contour = contour;
						nearest_transform = transform;
						}
					  }
					contour = contour->next;				// do next contour in list
					}
				}
			transform = transform->next;
			}
		}

	if ( nearest_transform )							// found a nearest one (there might not be any!)
		{
		if ( !active )
			{											// if no active transform, create it
			active = new Transform();
			if ( !transforms ) transforms = new Transforms();
			transforms->Add( active );
			}
		active->isActive = true;
														// create active contours list if it doesn't exist
		if ( !active->contours ) active->contours = new Contours();

		nearest_transform->contours->Extract( nearest_contour );	// extract near contour
		nearest_contour->InvNform( nearest_transform->nform );		// transform out of nearest_transform
		nearest_contour->FwdNform( active->nform );					// and in to active transform
		active->contours->Add( nearest_contour );					// then add to active list

		return( true );
		}

	return( false );
}

Contour * Section::ExtractClosestContour( double x, double y )
{
	Transform *transform, *nearest_transform;		// locate closest contour and put in active transform
	Contour *contour, *nearest_contour;
	double d, min_d, transX, transY;

	min_d = MAX_FLOAT;
	nearest_contour = NULL;
	nearest_transform = NULL;

	if ( this->transforms )
		{
		transform = this->transforms->first;
		while ( transform )							// check each transform for contours
			{
			if ( transform->contours )
				{
				transX = transform->nform->X( x, y );		// put x,y into transform coord
				transY = transform->nform->Y( x, y );
															// test each contour
				contour = transform->contours->first;
				while ( contour )
					{
					if ( !contour->hidden )
					  {
					  d = contour->Distance( transX, transY );						
					  if ( (d < min_d) )
						{
						min_d = d;
						nearest_contour = contour;
						nearest_transform = transform;
						}
					  }
					contour = contour->next;				// do next contour in list
					}
				}
			transform = transform->next;
			}
		}

	if ( nearest_transform )							// found a nearest one (there might not be any!)
		{
		nearest_transform->contours->Extract( nearest_contour );	// extract near contour
		nearest_contour->InvNform( nearest_transform->nform );		// transform out of nearest_transform
		}

	return nearest_contour;
}

bool Section::SelectContoursInRegion( double left, double top, double right, double bottom )
{
	Transform *transform, *nearest_transform;		// locate contours in region and put in active transform
	Contour *contour, *incontour;
	double transLeft, transTop, transRight, transBottom;
	bool foundone = false;
	
	if ( transforms )
		{
		transform = transforms->first;
		while ( transform )		// check each transform (except active!) for contours
			{
			if ( (transform != active) && transform->contours )
				{
				transLeft = transform->nform->X( left, top );			// put region into transform coord
				transTop = transform->nform->Y( left, top );
				transRight = transform->nform->X( right, bottom );
				transBottom = transform->nform->Y( right, bottom );
																		// test each contour
				contour = transform->contours->first;
				while ( contour )
					{
					if ( contour->IsInRegion( transLeft, transTop, transRight, transBottom ) && !contour->hidden )
						{
						foundone = true;
						if ( !active )
							{											// if no active transform, create it
							active = new Transform();
							transforms->Add( active );					// likewise create a contours list as needed
							}
						if ( !active->contours ) active->contours = new Contours();
						active->isActive = true;
						incontour = contour;
						contour = contour->next;
						transform->contours->Extract( incontour );		// extract contour from transform
						incontour->InvNform( transform->nform );		// transform out of transform coord.
						incontour->FwdNform( active->nform );			// and into active transform coord.
						active->contours->Add( incontour );				// then add to active list
						}
					else contour = contour->next;						// do next contour in list
					}
				}
			transform = transform->next;
			}
		}

	return( foundone );
}

Contour * Section::SelectContourId( int id )			// locate contour by Id and put in active transform
{														// return ptr to identified contour
	Transform *transform;
	Contour *contour;
	bool found;

	found = false;
	contour = NULL;
	transform = this->transforms->first;
	while ( transform && !found )						// check each transform for contours
		{
		if ( (transform != active) && transform->contours )
			{											// test each contour
			contour = transform->contours->first;
			while ( contour && !found )					// until found					
				if ( contour->Id == id )
					{
					if ( !active )
						{								// if no active transform, create it
						active = new Transform();
						if ( !transforms ) transforms = new Transforms();
						transforms->Add( active );
						}								// create active contours list if it doesn't exist
					if ( !active->contours ) active->contours = new Contours();
					active->isActive = true;
					transform->contours->Extract( contour );	// extract id contour
					contour->InvNform( transform->nform );		// transform out of transform
					contour->FwdNform( active->nform );			// and in to active transform
					active->contours->Add( contour );			// then add to active list
					found = true;								// stop searching
					}
				else contour = contour->next;				// do next contour in list
			}
		transform = transform->next;
		}
	return contour;
}

Contour * Section::FindContourId( int id )			// locate contour by Id 
{
	Transform *transform;
	Contour *contour;
	bool found = false;

	contour = NULL;
	transform = this->transforms->first;
	while ( transform && !found )						// check each transform for contours
		{
		if ( transform->contours )
			{											// test each contour
			contour = transform->contours->first;
			while ( contour && !found )					// until found					
				if ( contour->Id == id ) found = true;
				else contour = contour->next;			// otherwise do next contour in list
			}
		transform = transform->next;
		}
	return contour;
}

Transform * Section::FindDomainId( int id )			// return the transform for domain with id
{
	Transform *transform;
	bool found = false;

	transform = this->transforms->first;
	while ( transform && !found )						// check each transform for domain
		if (  transform->image && transform->domain )			
			if ( transform->domain->Id == id ) found = true;// stop searching
			else transform = transform->next;
		else transform = transform->next;

	return transform;									// returns NULL if not found
}

void Section::UnSelectClosestActive( double x, double y )
{
	Transform *transform;							// remove closest contour from active transform
	Contour *contour, *nearest_contour;
	double d, min_d, transX, transY;

	min_d = MAX_FLOAT;
	nearest_contour = NULL;

	if ( active )									// search only active contours
		{
		if ( active->contours )
			{
			transX = active->nform->X( x, y );			// put x,y into transform coord
			transY = active->nform->Y( x, y );
														// test each contour
			contour = active->contours->first;
			while ( contour )
				{
				if ( !contour->hidden )
				  {
				  d = contour->Distance( transX, transY );						
				  if ( d < min_d )
					{
					min_d = d;
					nearest_contour = contour;
					}
				  }
				contour = contour->next;				// do next contour in list
				}
			}
		}

	if ( active && nearest_contour )				// found nearest contour in active!
		{
		active->contours->Extract(nearest_contour);				// extract it from active
		transform = new Transform();							// create a new transform for it
		delete transform->nform;								// without an initial nform
		transform->nform = new Nform( *(active->nform) );		// so can copy same nform as active
		transform->contours = new Contours();					// start with empty contours list
		transform->contours->Add( nearest_contour );			// add this unselected contour
		this->transforms->Add( transform );						// add the new transform to section

		if ( active->contours->Number() < 1 )				// delete empty active transform from section
			{
			transforms->Extract( active );
			delete active;
			active = NULL;
			}
		}
}

void Section::UnselectHidden( void )		// remove hidden traces from active list
{
	Transform *transform;
	Contour *contour, *acontour;

	if ( active )						// do only if there are active contours
		if ( active->contours )
			{
			transform = NULL;				// placeholder for new Transform
			acontour = active->contours->first;
			while ( acontour )
				{
				if ( acontour->hidden )		// check all active contours for hidden ones
					{
					contour = acontour;
					acontour = acontour->next;
					active->contours->Extract( contour );		// extract it from active
					if ( transform == NULL )
						{
						transform = new Transform();			// create a new transform for it
						delete transform->nform;				// without an initial nform
						transform->nform = new Nform( *(active->nform) );// copy nform from active
						transform->contours = new Contours();	// start with empty contours list
						transforms->Add( transform );			// add the new transform to section
						}
					transform->contours->Add( contour );		// add the hidden contour
					}
				else acontour = acontour->next;	
				}

			if ( active->contours->Number() < 1 )	// delete active transform if empty
				{
				transforms->Extract( active );
				delete active;
				active = NULL;
				}
			}
}

bool Section::HasImage( void )						// return false if none found
{
	Transform *transform;
	bool found = false;

	if ( this->transforms )								// do only if transforms present
		{
		transform = this->transforms->first;			// check all images in section
		while ( transform )								
			{			
			if ( transform->image ) found = true;
			transform = transform->next;				// do next transform in list
			}
		}

	return found;
}

double Section::MinPixelsize( void )				// get smallest mag in section
{
	Transform *transform;
	double min_mag;

	min_mag = MAX_FLOAT;
	if ( this->transforms )								// do only if transforms present
		{
		transform = this->transforms->first;			// check all images in section
		while ( transform )								
			{			
			if ( transform->image )
				if ( transform->image->mag < min_mag )
					min_mag = transform->image->mag;	// remember smallest value
			transform = transform->next;				// do next transform in list
			}
		}

	return min_mag;				// return value will be MAX_FLOAT if no images in section!
}

void Section::ImageSize( Point &min, Point &max )	// determine extent of all image domains in section units
{
	Transform *transform;
	Point *p;
	Contour *domain;										// will return these extreme values if no images!

	min.x = MAX_FLOAT; min.y = MAX_FLOAT; max.x = -MAX_FLOAT; max.y = -MAX_FLOAT;
	if ( transforms )
		{
		transform = transforms->first;						// find all images in section
		while ( transform )								
			{			
			if ( transform->image ) 						// found image
				{
				if ( transform->domain )					// use domain boundary to determine area...
					{	
					domain = new Contour( *(transform->domain));// create a copy of domain
					domain->Scale( transform->image->mag );		// go from pixels to units
					domain->InvNform( transform->nform );		// transform into section
					if ( domain->points )						// do only if have points
						{
						p = domain->points->first;				//  find extremes...
						while ( p )
							{					
							if ( p->x < min.x ) min.x = p->x;
							if ( p->x > max.x ) max.x = p->x;
							if ( p->y < min.y ) min.y = p->y;
							if ( p->y > max.y ) max.y = p->y;		
							p = p->next;
							}
						}										// delete transformed domain
					delete domain;								
					}
				}
			transform = transform->next;	// do next transform in list
			}
		}
}

bool Section::HasContour( void )					// return false if none found
{
	Transform *transform;
	bool found = false;

	if ( this->transforms )								// do only if transforms present
		{
		transform = this->transforms->first;			// check all images in section
		while ( transform )								
			{			
			if ( transform->contours )
				if ( transform->contours->first ) found = true;
			transform = transform->next;				// do next transform in list
			}
		}

	return found;
}

void Section::ContourSize( Point &min, Point &max )	// determine extent of all contours in section units
{
	Transform *transform;
	Contour *c, *contour;
	Point cmin, cmax;								// will return these extreme values if no contours!

	min.x = MAX_FLOAT; min.y = MAX_FLOAT; max.x = -MAX_FLOAT; max.y = -MAX_FLOAT;
	if ( transforms )
		{
		transform = transforms->first;						// find all images in section
		while ( transform )								
			{			
			if ( transform->contours )					// contours are selected
				{
				contour = transform->contours->first;
				while ( contour )
					{
					c = new Contour( *contour );				// create a copy of contour
					c->InvNform( transform->nform );			// transform into section
					c->Extent( &cmin, &cmax );					//  find extremes...
					delete c;									// delete transformed contour
					if ( cmin.x < min.x ) min.x = cmin.x;		// accummulate min, max
					if ( cmin.y < min.y ) min.y = cmin.y;
					if ( cmax.x > max.x ) max.x = cmax.x;
					if ( cmax.y > max.y ) max.y = cmax.y;
					contour = contour->next;						// use all active contours
					}
				}
			transform = transform->next;	// do next transform in list
			}
		}
}

void Section::PushUndoState( void )
{													// remember current section state for later undo
	if ( transforms )
		{
		if ( active ) active->isActive = true;		// make sure active transform in list is marked
		if ( !undos ) undos = new TformsLIFO();		// create undo LIFO if needed
		undos->Push( transforms );					// save state of section
		if ( redo ) delete redo;					// forget any redo state
		redo = NULL;
		}
}

bool Section::PushRedoState( void )
{												// restore section state prior to last undo
	Transform *transform;

	if ( redo )
		{
		if ( active ) active->isActive = true;		// make sure active transform in list is marked
		if ( !undos ) undos = new TformsLIFO();		// create undo LIFO if needed
		undos->Push( transforms );					// save state of section
		transforms = redo;							// recall redo state
		redo = NULL;
		active = NULL;
		transform = transforms->first;				// look for marked active transform (if any)
		while ( transform )
			if ( transform->isActive ) { active = transform; transform = NULL; }
			else transform = transform->next;
		return true;
		}
	return false;
}

bool Section::PopUndoState( void )
{													// return to last pushed state (and remove it from LIFO)
	Transforms *tforms;
	Transform *transform;
	bool undone = false;

	if ( undos )									// only valid if undos exist (Push has occurred)
		{
		tforms = undos->Pop();
		if ( tforms )								// if have a remaining undo state, apply it
			{
			undone = true;
			if ( redo ) delete redo;				// get rid of any previous redo state
			redo = transforms;						// remember current transforms for redo
			if ( active ) active->isActive = true;	// remember which transform was active in redo
			transforms = tforms;
			active = NULL;
			transform = transforms->first;			// look for marked active transform (if any)
			while ( transform )
				if ( transform->isActive ) { active = transform; transform = NULL; }
				else transform = transform->next;
			}
		}

	return( undone );
}

bool Section::ResetUndoState( void )				// jump back to first undo state
{													// return to last pushed state (and remove it from LIFO)
	Transforms *tforms;
	Transform *transform;
	bool reset = false;

	if ( undos )									// only valid if undos exist (Push has occurred)
		{
		tforms = undos->Reset();
		if ( tforms )								// if have a remaining undo state, apply it
			{
			reset = true;
			if ( redo ) delete redo;
			redo = transforms;						// save current transforms for possible redo
			if ( active ) active->isActive = true;	// remember which transform was active in redo
			transforms = tforms;
			active = NULL;
			transform = transforms->first;			// look for marked active transform (if any)
			while ( transform )
				if ( transform->isActive ) { active = transform; transform = NULL; }
				else transform = transform->next;
			}
		}

	return( reset );
}

bool Section::HasUndos( void )						// report whether a section undo is available
{
	bool present = false;
	
	if ( undos ) if ( undos->last >= 0 ) present = true;

	return( present );
}

bool Section::HasRedo( void )						// report whether a section undo is available
{
	if ( redo ) return(true);
	return( false );
}

bool GetNextCodeValue(char *data, int size, int &index, int &code, char *value)
{															// grab next code-value pait from data string
															// and update index to reflect new position in data
	if ( index < size ) sscanf( data+index, "%d", &code );  // read integer code line
	while ( (index < size) && (data[index] != '\n') ) index++;// find next line
	index++;												// move to start of field
	if ( index < size ) sscanf( data+index, "%s", value );	// read value string			
	while ( (index < size) && (data[index] != '\n') ) index++;// find next line
	index++;												// leave index at start of field
    if ( index > size) return false;						// return false when file ends
	return true;
}

void Section::AddDXFLines( char *filename )			// read DXF polylines from filename and add to section as active
{
	Contour *contour;
	Point *point;
	HANDLE hFile;
	DWORD didRead, dwSize;
	bool valid_code;
	double x, y, r, step, fx, fy;
	int code, index, c;
	char value[256];
	char errmsg[1024];
	char *data, *nextItem;														// bytes of raw ascii
	unsigned int size;
									
	if ( active )													// unselect active contours
		active->isActive = false;									// will put imported line there
	active = NULL;													// if fail, this will be NULL
	
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
	
	index = 0;
	contour = NULL;
	if ( GetNextCodeValue(data,size,index,code,value) )				// find first zero (entity) code
		valid_code = true;
	else valid_code = false;
	
	while ( valid_code )
		{
		valid_code = false;											// terminate loop if no more entities
		if ( !code && !strcmp(value,"LINE") )						// begin LINE entitiy
			{
			if ( contour ) AddNewContour( contour );				// first, end the last contour
			contour = new Contour();								// start a new contour to hold the POLYLINE
			contour->points = new Points();
			contour->closed = false;
			contour->mode = R2_NOP;
			contour->simplified = true;
			strcpy(contour->name,"dxf_line");
			while ( GetNextCodeValue(data,size,index,code,value) )
				{
				if ( code == 0 ) { valid_code = true; break; }		// if new entity, break to process it
				else if ( code == 62 ) 
					{
					strcat(contour->name,value);	// add color number to contour name
					c = atoi(value);				// use value to create a border color
					contour->border.r = (float)(c >> 5)/7.0;
					contour->border.g = (float)((c & 31) >> 2)/7.0;
					contour->border.b = (float)(c & 3)/3.0;
					contour->fill = contour->border;
					}
				else if ( (code == 10) || (code == 11) || (code == 12) || (code == 13) )
						{
						x = atof(value);
						if ( GetNextCodeValue(data,size,index,code,value) )
							{
							if ( code == 0 ) { valid_code = true; break; }	// if new entity, break
							else if ( (code == 20) || (code == 21) || (code == 22) || (code == 23) )
								{
								point = new Point();				// create new point
								point->y = atof(value);
								point->x = x;
								contour->points->Add(point);		// search for x,y values
								}
							}
						}
				}
				
			}
		
		if ( !code && !strcmp(value,"POLYLINE") )					// begin POLYLINE entitiy
			{
			if ( contour ) AddNewContour( contour );				// first, end the last contour
			contour = new Contour();								// start a new contour to hold the POLYLINE
			contour->points = new Points();
			contour->closed = true;
			contour->mode = R2_NOP;
			contour->simplified = true;
			strcpy(contour->name,"dxf_polyline");
			while ( GetNextCodeValue(data,size,index,code,value) )
				if ( code == 0 ) { valid_code = true; break; }		// if new entity, break to process it
				else if ( code == 62 ) 
					{
					strcat(contour->name,value);	// add color number to contour name
					c = atoi(value);				// use value to create a border color
					contour->border.r = (float)(c >> 5)/7.0;
					contour->border.g = (float)((c & 31) >> 2)/7.0;
					contour->border.b = (float)(c & 3)/3.0;
					contour->fill = contour->border;
					}
				else if ( code == 70 )
					if ( value[0] == '0' ) contour->closed = false;	// check for open contour flag
			}
		else if ( !code && !strcmp(value,"VERTEX") )				// get a VERTEX
			{
			if ( contour )											// only process VERTEX if have POLYLINE
				{
				point = new Point();								// create new point
				contour->points->Add(point);						// search for x,y values
				while ( GetNextCodeValue(data,size,index,code,value) )
					if ( code == 0 ) { valid_code = true; break; }	// bail on new entity
					else if ( code == 10 ) point->x = atof(value);
					else if ( code == 20 ) point->y = atof(value);
				}
			}
		else if ( !code && !strcmp(value,"SEQEND") )				// end POLYLINE entity
			{
			if ( contour )  AddNewContour( contour );				// add contour to this section
			contour = NULL;
			if ( GetNextCodeValue(data,size,index,code,value) )		// go on to next code
				valid_code = true;
			}
																	// get CIRCLE entity
		else if ( !code && !strcmp(value,"CIRCLE") )
			{
			if ( contour ) AddNewContour( contour );				// first, end the last contour
			contour = new Contour();								// start a new contour
			contour->points = new Points();
			contour->closed = true;
			contour->mode = R2_NOP;
			contour->simplified = true;
			strcpy(contour->name,"dxf_circle");
			x = 0.0; y = 0.0; r = 0.0;
			while ( GetNextCodeValue(data,size,index,code,value) )
				if ( code == 0 ) { valid_code = true; break; }		// if new entity, break to process it
				else if ( code == 62 ) 
					{
					strcat(contour->name,value);	// add color number to contour name
					c = atoi(value);				// use value to create a border color
					contour->border.r = (float)(c >> 5)/7.0;
					contour->border.g = (float)((c & 31) >> 2)/7.0;
					contour->border.b = (float)(c & 3)/3.0;
					contour->fill = contour->border;
					}
				else if ( code == 10 ) x = atof(value);
				else if ( code == 20 ) y = atof(value);
				else if ( code == 40 ) r = atof(value);
			if ( valid_code && (r > 0.0) )
				{
				step = 2.0*PI/r;
				if ( step > PI/10.0 ) step = PI/10.0;
				for (double p=-PI; p<PI-step; p=p+step)
					{													// create ellipse
					fx = r*cos(p) + x;
					fy = r*sin(p) + y;
					point = new Point( fx, fy, 0.0 );
					contour->points->Add(point);
					}
				}
			else { delete contour; contour = NULL; }				// didn't get info needed for circle
			}

		else if ( !code && (!strcmp(value,"MTEXT")					// get TEXT entity
							|| !strcmp(value,"RTEXT") || !strcmp(value,"TEXT")) ) 
			{
			if ( contour ) AddNewContour( contour );				// first, end the last contour
			contour = new Contour();								// start a new contour
			contour->points = new Points();
			contour->closed = true;
			contour->mode = R2_NOP;
			contour->simplified = true;
			strcpy(contour->name,"dxf_text");
			x = 0.0; y = 0.0; r = 0.0;
			while ( GetNextCodeValue(data,size,index,code,value) )
				if ( code == 0 ) { valid_code = true; break; }		// if new entity, break to process it
				else if ( code == 62 ) 
					{
					strcat(contour->name,value);	// add color number to contour name
					c = atoi(value);				// use value to create a border color
					contour->border.r = (float)(c >> 5)/7.0;
					contour->border.g = (float)((c & 31) >> 2)/7.0;
					contour->border.b = (float)(c & 3)/3.0;
					contour->fill = contour->border;
					}
				else if ( code == 10 ) x = atof(value);
				else if ( code == 20 ) y = atof(value);
				else if ( code == 40 ) r = atof(value);
			if ( valid_code && (r > 0.0) )
				{													// make contour a square = height of text
				point = new Point( x, y+r, 0.0 );
				contour->points->Add(point);
				point = new Point( x+r, y+r, 0.0 );
				contour->points->Add(point);
				point = new Point( x+r, y, 0.0 );
				contour->points->Add(point);
				point = new Point( x, y, 0.0 );
				contour->points->Add(point);
				}
			else { delete contour; contour = NULL; }				// didn't get info needed for circle
			}

		else if ( !code && !strcmp(value,"SOLID") )				// get a SOLID
			{
			if ( contour ) AddNewContour( contour );				// first, end the last contour
			contour = new Contour();								// start a new contour
			contour->points = new Points();
			contour->closed = true;
			contour->mode = R2_COPYPEN;
			contour->simplified = true;
			strcpy(contour->name,"dxf_solid");
			while ( GetNextCodeValue(data,size,index,code,value) )
				{
				if ( code == 0 ) { valid_code = true; break; }		// if new entity, break to process it
				else if ( code == 62 ) 
					{
					strcat(contour->name,value);	// add color number to contour name
					c = atoi(value);				// use value to create a border color
					contour->border.r = (float)(c >> 5)/7.0;
					contour->border.g = (float)((c & 31) >> 2)/7.0;
					contour->border.b = (float)(c & 3)/3.0;
					contour->fill = contour->border;
					}
				else if ( (code == 10) || (code == 11) || (code == 12) || (code == 13) )
						{
						x = atof(value);
						if ( GetNextCodeValue(data,size,index,code,value) )
							{
							if ( code == 0 ) { valid_code = true; break; }	// if new entity, break
							else if ( (code == 20) || (code == 21) || (code == 22) || (code == 23) )
								{
								point = new Point();				// create new point
								point->y = atof(value);
								point->x = x;
								contour->points->Add(point);		// search for x,y values
								}
							}
						}
				}
			}
		else if ( !code && !strcmp(value,"EOF") )
			valid_code = false;										// let loop terminate!
		else
			if ( GetNextCodeValue(data,size,index,code,value) )		// otherwise, find next entity
				valid_code = true;
		}															// end while valid_code

	if ( contour ) AddNewContour( contour );				// don't leave the last contour hanging
	free( data );	
							
}

												// create grid traces at (fx,fy) on section
void Section::CreateGridTraces( double fx, double fy )	
{
	int i, j;
	double cx, cy, phi, step;
	Point *p, min, max, allmin, allmax;
	Contour *contour, *econtour;
	char name[MAX_CONTOUR_NAME];
	Contours *element;							// place elements on section starting at (fx,fy)

	element = new Contours();					// one element of grid can be a set of contours	

	switch ( CurrSeries->gridType )
		{
		case POINT_GRID:								// a grid of points
			econtour = new Contour();
			element->Add( econtour );
			econtour->closed = false;
			econtour->simplified = true;
			CurrSeries->SetDefaultAttributes( econtour );	// use default colors/fill
			econtour->points = new Points();				// crossed lines shape
			p = new Point(-CurrSeries->gridXSize/2.0,0.0,0.0);
			econtour->points->Add(p);
			p = new Point(CurrSeries->gridXSize/2.0,0.0,0.0);
			econtour->points->Add(p);
			econtour = new Contour();						// second contour of cross
			element->Add( econtour );
			econtour->closed = false;
			econtour->simplified = true;
			CurrSeries->SetDefaultAttributes( econtour );	// use default colors/fill
			econtour->points = new Points();
			p = new Point(0.0,-CurrSeries->gridYSize/2.0,0.0);
			econtour->points->Add(p);
			p = new Point(0.0,CurrSeries->gridYSize/2.0,0.0);
			econtour->points->Add(p);
			break;

		case FRAME_GRID:								// a grid of sampling frames
			econtour = new Contour();
			element->Add( econtour );
			econtour->closed = false;
			econtour->simplified = true;
			econtour->border = Color(1.0,0.0,0.0);		// make exclusion line red
			econtour->points = new Points();
			p = new Point(0.0,CurrSeries->gridYSize/2.0,0.0);
			econtour->points->Add(p);
			p = new Point(0.0,0.0,0.0);					// include all corners of frame
			econtour->points->Add(p);
			p = new Point(0.0,-CurrSeries->gridYSize,0.0);
			econtour->points->Add(p);
			p = new Point(CurrSeries->gridXSize,-CurrSeries->gridYSize,0.0);
			econtour->points->Add(p);
			p = new Point(CurrSeries->gridXSize,-3.0*CurrSeries->gridYSize/2.0,0.0);
			econtour->points->Add(p);
			econtour = new Contour();					// second contour is inclusion line
			element->Add( econtour );
			econtour->closed = false;
			econtour->simplified = true;
			econtour->border = Color(0.0,1.0,0.0);		// make inclusion line green
			econtour->points = new Points();
			p = new Point(0.0,0.0,0.0);
			econtour->points->Add(p);
			p = new Point(CurrSeries->gridXSize,0.0,0.0);
			econtour->points->Add(p);
			p = new Point(CurrSeries->gridXSize,-CurrSeries->gridYSize,0.0);
			econtour->points->Add(p);
			break;

		case STAMP_GRID:
			econtour = new Contour( *StampContour );		// copy shape of stamp contour
			CurrSeries->SetDefaultAttributes( econtour );	// use current defaults
			econtour->Scale( CurrSeries->pixel_size );		// put into section untis
			econtour->Extent( &min, &max );
			cx = (max.x - min.x);
			cy = (max.y - min.y);
			if ( (CurrSeries->gridXSize > 0.0) && (cx > 0.0) ) 
				econtour->ScaleXY( CurrSeries->gridXSize/cx, 1.0 );
			if ( (CurrSeries->gridYSize > 0.0) && (cy > 0.0) ) 
				econtour->ScaleXY( 1.0, CurrSeries->gridYSize/cy );
			element->Add( econtour );
			break;

		case CLIPBOARD_GRID:
			if ( ClipboardTransform )
			  if ( ClipboardTransform->contours )
				{
				allmin.x = MAX_FLOAT;						// allmin, allmax will hold final extents
				allmin.y = MAX_FLOAT;
				allmax.x = -MAX_FLOAT;
				allmax.y = -MAX_FLOAT;
				contour = ClipboardTransform->contours->first;
				while ( contour )
					{												// add copies from clipboard contours
					econtour = new Contour( *contour );
					econtour->InvNform( ClipboardTransform->nform );// transform out of clip transform
					element->Add( econtour );
					econtour->Extent( &min, &max );
					if ( min.x < allmin.x ) allmin.x = min.x;		// accummulate min, max
					if ( min.y < allmin.y ) allmin.y = min.y;
					if ( max.x > allmax.x ) allmax.x = max.x;
					if ( max.y > allmax.y ) allmax.y = max.y;
					contour = contour->next;
					}
				cx = (allmin.x + allmax.x)/2.0;						// computer center of group
				cy = (allmin.y + allmax.y)/2.0;
				econtour = element->first;
				while ( econtour )									// translate group to origin
					{
					econtour->Shift(-cx,-cy);
					econtour = econtour->next;
					}
				}
			break;

		case CYCLOID_GRID:								// a cycloidal element used in stereology
			econtour = new Contour();
			element->Add( econtour );
			econtour->closed = false;
			econtour->simplified = true;
			CurrSeries->SetDefaultAttributes( econtour );	// use default colors/fill
			econtour->points = new Points();
			step = 2.0*PI/20.0;								// create shape using 20 steps
			for (i=0; i<=20; i++)							// of parameter phi (0,2PI)
				{											// cx will be gridXSize at 2PI
				phi = i*step;
				cx = CurrSeries->gridXSize*(phi-sin(phi))/(4.0*PI);
				cy = CurrSeries->gridXSize*(1.0-cos(phi))/(4.0*PI);
				p = new Point( cx, cy, 0.0 );
				econtour->points->Add(p);
				}
			econtour = new Contour( *econtour );			// for second cycle, just flip previous one
			econtour->ScaleXY( 1.0, -1.0 );
			econtour->Shift( CurrSeries->gridXSize/2.0, 0.0 );
			element->Add( econtour );
			break;

		case ELLIPSE_GRID:								// an elliptical element 
			econtour = new Contour();
			element->Add( econtour );
			econtour->closed = true;
			econtour->simplified = true;
			CurrSeries->SetDefaultAttributes( econtour );	// use default colors/fill
			econtour->points = new Points();
			step = 2.0*PI/32.0;								// create shape using 32 steps
			for (i=0; i<32; i++)
				{
				phi = i*step;								// create ellipse
				cx = CurrSeries->gridXSize*cos(phi)/2.0;
				cy = CurrSeries->gridYSize*sin(phi)/2.0;
				p = new Point( cx, cy, 0.0 );
				econtour->points->Add(p);
				}
			break;

		default:									// a grid of rectangles 
			econtour = new Contour();
			element->Add( econtour );
			econtour->closed = true;
			econtour->simplified = true;
			CurrSeries->SetDefaultAttributes( econtour );	// usedefault colors/fill
			econtour->points = new Points();				// and rectangle shape
			p = new Point(0.0,-CurrSeries->gridYSize,0.0);
			econtour->points->Add(p);
			p = new Point(CurrSeries->gridXSize,-CurrSeries->gridYSize,0.0);
			econtour->points->Add(p);
			p = new Point(CurrSeries->gridXSize,0.0,0.0);
			econtour->points->Add(p);
			p = new Point(0.0,0.0,0.0);
			econtour->points->Add(p);
		}
			
	for (j=0; j>-CurrSeries->gridYNumber; j--)			// repeat rectangular array pattern
		for (i=0; i<CurrSeries->gridXNumber; i++)		// cursor is at upper left of array
			{
			this->SetDefaultName(name,CurrSeries->defaultName,false);
			econtour = element->first;
			while ( econtour )							// add all contours that make up one grid element
				{
				contour = new Contour( *econtour );				// copy shape of contour
				strcpy(contour->name,name);						// use name for all parts of grid element
				contour->Shift(fx,fy);							// shift to cursor then to grid position
				contour->Shift( (double)i*CurrSeries->gridXDistance, (double)j*CurrSeries->gridYDistance );
				this->AddNewContour( contour );	// add contour to section
				econtour = econtour->next;
				}
			}

	delete element;
}
