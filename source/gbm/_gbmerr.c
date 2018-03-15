/*

_gbmerr.c - Calculate the quick 'closest-colour' lookup table for gbmerr.c

*/

#include <stdio.h>
#include "gbm.h"

/*...sVGA palette:0:*/
static GBMRGB gbmrgb_vga[] =
	{
	  0,  0,  0,
	128,  0,  0,
	  0,128,  0,
	128,128,  0,
	  0,  0,128,
	128,  0,128,
	  0,128,128,
	128,128,128,
	204,204,204,
	255,  0,  0,
	  0,255,  0,
	255,255,  0,
	  0,  0,255,
	255,  0,255,
	  0,255,255,
	255,255,255,
	};
/*...e*/
/*...scalc_nearest:0:*/
/*
This function, when given am RGB colour, finds the VGA palette entry closest
to it. We deliberately bias away from the two grey palette entries.
*/

static byte calc_nearest(byte r, byte g, byte b)
	{
	long min_dist = 3L * 256L * 256L * 10L;
	byte bi, bi_min;

	for ( bi = 0; bi < 0x10; bi++ )
		{
		long b_dist = ((long) b - (long) gbmrgb_vga[bi].b);
		long g_dist = ((long) g - (long) gbmrgb_vga[bi].g);
		long r_dist = ((long) r - (long) gbmrgb_vga[bi].r);
		long dist = r_dist * r_dist + g_dist * g_dist + b_dist * b_dist;

		if ( bi == 7 || bi == 8 )
			/* Bias away from this colour */
			dist <<= 3;

		if ( dist < min_dist )
			{
			min_dist = dist;
			bi_min = bi;
			}
		}
	return ( bi_min );
	}
/*...e*/

static char *dw_casings[] =
	{
	"\t%d,", "%d,", "%d,", "%d,", "%d,", "%d,", "%d,", "%d,",
	"%d,", "%d,", "%d,", "%d,", "%d,", "%d,", "%d,", "%d,\n",
	};

int main(void)
	{
	byte r, r0, r1, g, g0, g1, b, b0, b1, i = 0;

	printf("static byte quick_tab[16][16][16] =\n\t{\n");

	for ( r = 0, r0 = 0, r1 = 15; r < 16; r++, r0 += 16, r1 += 16 )
		for ( g = 0, g0 = 0, g1 = 15; g < 16; g++, g0 += 16, g1 += 16 )
			for ( b = 0, b0 = 0, b1 = 15; b < 16; b++, b0 += 16, b1 += 16 )
/*...sanalyse cube:32:*/
{
byte n = calc_nearest(r0, g0, b0);
byte inx;

if ( n == calc_nearest(r0, g0, b1) &&
     n == calc_nearest(r0, g1, b0) &&
     n == calc_nearest(r0, g1, b1) &&
     n == calc_nearest(r1, g0, b0) &&
     n == calc_nearest(r1, g0, b1) &&
     n == calc_nearest(r1, g1, b0) &&
     n == calc_nearest(r1, g1, b1) )
	inx = n;
else
	inx = (byte) 0xff;
printf(dw_casings[i++ & 15], (int) inx);
}
/*...e*/

	printf("\t};\n");
	return 0;
	}
