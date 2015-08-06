//////////////////////////////////////////////////////////////////////////////////////////
// A Histogram class to represent the image pixels inside a contour.
//
//    Copyright (C) 2006 Ju Lu (julu@fas.harvard.edu)
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
// modified 11/07/03 by JCF
// -+- change: Fixed copyright info to correctly credit Ju Lu
// modified 11/15/03 by JCF
// -+- change: Fixed decelaration of copy constructor to follow convention. 

#include "reconstruct.h"

Histogram::Histogram()
{
	ResetHistogram();
}

Histogram::Histogram(Histogram &copyfrom)
{
	for (int i=0; i<=RGBMAX; i++ )
	{
		count_red[i] = copyfrom.count_red[i];
		count_green[i] = copyfrom.count_green[i];
		count_blue[i] = copyfrom.count_blue[i];
	}
	for (int i=0; i<=HLSMAX; i++ )
	{
		count_hue[i] = copyfrom.count_hue[i];
		count_sat[i] = copyfrom.count_sat[i];
		count_bright[i] = copyfrom.count_bright[i];
	}
}

/* JCF 11/13/06 commented out since not used
Histogram& Histogram::operator=(const Histogram& rhs)
{
	if (&rhs != this)
	{
		for (int i=0; i<=RGBMAX; i++ )
		{
			count_red[i] = rhs.count_red[i];
			count_green[i] = rhs.count_green[i];
			count_blue[i] = rhs.count_blue[i];
		}
		for (int i=0; i<=HLSMAX; i++ )
		{
			count_hue[i] = rhs.count_hue[i];
			count_sat[i] = rhs.count_sat[i];
			count_bright[i] = rhs.count_bright[i];
		}
	}
	return *this;
}

void Histogram::AddRGBPixel(int r, int g, int b)
{
	if (r>=0 && r<=RGBMAX && g>=0 && g<=RGBMAX && b>=0 && b<=RGBMAX)
	{
		count_red[r]++;
		count_green[g]++;
		count_blue[b]++;
	}
}
*/
void Histogram::AddHSBPixel(int h, int s, int b)
{
	if (h>=0 && h<=HLSMAX) 
		count_hue[h]++;
	if (s>=0 && s<=HLSMAX) 
		count_sat[s]++;
	if (b>=0 && b<=HLSMAX)
		count_bright[b]++;

}

void Histogram::ResetHistogram()
{
	for (int i=0; i<=RGBMAX; i++ )
	{
		count_red[i] = 0;
		count_green[i] = 0;
		count_blue[i] = 0;
	}

	for (int i=0; i<=HLSMAX; i++ )
	{
	    count_hue[i] = 0;
	    count_sat[i] = 0;
	    count_bright[i] = 0;
	}
}


int Histogram::MeanValue(char choice)
{
	int total_val = 0, mean_val;
	int pixels = 0;
	int * p = 0;
	int max_val = 0;
	switch (choice)
	{
		case 'r':
			p = count_red;
			max_val = RGBMAX;
			break;
		case 'g':	
			p = count_green;
			max_val = RGBMAX;
			break;
		case 'b':
			p = count_blue;
			max_val = RGBMAX;
			break;
		case 'h':
			p = count_hue;
			max_val = HLSMAX;
			break;
		case 's':
			p = count_sat;
			max_val = HLSMAX;
			break;
		case 'v':
			p = count_bright;
			max_val = HLSMAX;
			break;
	}
	
	for (int i=0; i<=max_val ;i++ )
	{
		total_val += p[i]*i;
		pixels += p[i];
	}
	
	if (pixels)
		mean_val = int(double(total_val)/pixels);
	else
		mean_val = 0;
	return mean_val;
}


int Histogram::PercentileValue(char choice, double percentile)
{	
	// return the value at which the histogram reaches certain percentile.

	int total_pixels = 0;
	int * p = 0;
	int max_val = 0;
	
	switch (choice)
	{
		case 'r':
			p = count_red;
			max_val = RGBMAX;
			break;
		case 'g':	
			p = count_green;
			max_val = RGBMAX;
			break;
		case 'b':
			p = count_blue;
			max_val = RGBMAX;
			break;
		case 'h':
			p = count_hue;
			max_val = HLSMAX;
			break;
		case 's':
			p = count_sat;
			max_val = HLSMAX;
			break;
		case 'v':
			p = count_bright;
			max_val = HLSMAX;
			break;
	}

	for (int i=0; i<=max_val;i++ )
		total_pixels += p[i];
	
	int target = int(total_pixels*(1-percentile));
	int pixels = 0;
	for (int i=max_val; i>=0 ; i-- )
	{
		pixels += p[i];
		if (pixels >= target)
			return i;
	}

	return 0;
}