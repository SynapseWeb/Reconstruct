/*

gbmpgm.c - Poskanzers PGM format

Reads as 8 bit grey image.
Writes grey equivelent of passed in 8 bit colour data (no palette written).
Output options: r,g,b,k (default: k)

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"

/*...vgbm\46\h:0:*/
/*...vgbmhelp\46\h:0:*/
/*...e*/

/*...smake_output_palette:0:*/
#define	SW4(a,b,c,d)	((a)*8+(b)*4+(c)*2+(d))

static gbm_boolean make_output_palette(const GBMRGB gbmrgb[], gbm_u8 grey[], const char *opt)
	{
	gbm_boolean	k = ( gbm_find_word(opt, "k") != NULL );
	gbm_boolean	r = ( gbm_find_word(opt, "r") != NULL );
	gbm_boolean	g = ( gbm_find_word(opt, "g") != NULL );
	gbm_boolean	b = ( gbm_find_word(opt, "b") != NULL );
	int i;
	switch ( SW4(k,r,g,b) )
		{
		case SW4(0,0,0,0):
			/* Default is the same as "k" */
		case SW4(1,0,0,0):
			for ( i = 0; i < 0x100; i++ )
				grey[i] = (gbm_u8) ( ((gbm_u16) gbmrgb[i].r *  77U +
						      (gbm_u16) gbmrgb[i].g * 150U +
						      (gbm_u16) gbmrgb[i].b *  29U) >> 8 );
			return GBM_TRUE;
		case SW4(0,1,0,0):
			for ( i = 0; i < 0x100; i++ )
				grey[i] = gbmrgb[i].r;
			return GBM_TRUE;
		case SW4(0,0,1,0):
			for ( i = 0; i < 0x100; i++ )
				grey[i] = gbmrgb[i].g;
			return GBM_TRUE;
		case SW4(0,0,0,1):
			for ( i = 0; i < 0x100; i++ )
				grey[i] = gbmrgb[i].b;
			return GBM_TRUE;
		}
	return GBM_FALSE;
	}
/*...e*/
/*...sposk stuff:0:*/
/*...sread_u8:0:*/
static gbm_u8 read_u8(int fd)
	{
	gbm_u8 b = 0;
	gbm_file_read(fd, (char *) &b, 1);
	return b;
	}
/*...e*/
/*...sread_char:0:*/
static char read_char(int fd)
	{
	char c;
	while ( (c = read_u8(fd)) == '#' )
		/* Discard to end of line */
		while ( (c = read_u8(fd)) != '\n' )
			;
	return c;
	}
/*...e*/
/*...sread_num:0:*/
static int read_num(int fd)
	{
	char c;
	int num;
	while ( isspace(c = read_char(fd)) )
		;
	num = c - '0';
	while ( isdigit(c = read_char(fd)) )
		num = num * 10 + (c - '0');
	return num;
	}
/*...e*/
/*...sread_posk_header:0:*/
static void read_posk_header(int fd, int *h1, int *h2, int *w, int *h, int *m)
	{
	gbm_file_lseek(fd, 0, SEEK_SET);
	*h1 = read_u8(fd);
	*h2 = read_u8(fd);
	*w  = read_num(fd);
	*h  = read_num(fd);
	*m  = read_num(fd);
	}
/*...e*/
/*...e*/

static GBMFT pgm_gbmft =
	{
	"Greymap",
	"Portable Greyscale-map (binary P5 type)",
	"PGM",
	GBM_FT_R8|
	GBM_FT_W8,
	};

#define	GBM_ERR_PGM_BAD_M	((GBM_ERR) 100)

/*...spgm_qft:0:*/
GBM_ERR pgm_qft(GBMFT *gbmft)
	{
	*gbmft = pgm_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...spgm_rhdr:0:*/
GBM_ERR pgm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	int h1, h2, w, h, m;

	fn=fn; opt=opt; /* Suppress 'unref arg' compiler warnings */

	read_posk_header(fd, &h1, &h2, &w, &h, &m);
	if ( h1 != 'P' || h2 != '5' )
		return GBM_ERR_BAD_MAGIC;

	if ( w <= 0 || h <= 0 )
		return GBM_ERR_BAD_SIZE;

	if ( m <= 1 || m >= 0x100 )
		return GBM_ERR_PGM_BAD_M;

	gbm->w   = w;
	gbm->h   = h;
	gbm->bpp = 8;

	return GBM_ERR_OK;
	}
/*...e*/
/*...spgm_rpal:0:*/
GBM_ERR pgm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	int h1, h2, w, h, m, i;

	gbm=gbm; /* Suppress 'unref arg' compiler warning */

	read_posk_header(fd, &h1, &h2, &w, &h, &m);
	for ( i = 0; i <= m; i++ )
		gbmrgb[i].r =
		gbmrgb[i].g =
		gbmrgb[i].b = (gbm_u8) (i * 0xff / m);

	return GBM_ERR_OK;
	}
/*...e*/
/*...spgm_rdata:0:*/
GBM_ERR pgm_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	int h1, h2, w, h, m, i, stride;
	gbm_u8 *p;
	
	read_posk_header(fd, &h1, &h2, &w, &h, &m);

	stride = ((gbm->w + 3) & ~3);
	p = data + ((gbm->h - 1) * stride);
	for ( i = gbm->h - 1; i >= 0; i-- )
		{
		gbm_file_read(fd, p, gbm->w);
		p -= stride;
		}
	return GBM_ERR_OK;
	}
/*...e*/
/*...spgm_w:0:*/
GBM_ERR pgm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	char s[100+1];
	int i, j, stride;
	gbm_u8 grey[0x100];
	const gbm_u8 *p;
	gbm_u8 *linebuf;

	fn=fn; opt=opt; /* Suppress 'unref arg' compiler warnings */

	if ( gbm->bpp != 8 )
		return GBM_ERR_NOT_SUPP;

	if ( !make_output_palette(gbmrgb, grey, opt) )
		return GBM_ERR_BAD_OPTION;

	if ( (linebuf = malloc((size_t) gbm->w)) == NULL )
		return GBM_ERR_MEM;

	sprintf(s, "P5\n%d %d\n255\n", gbm->w, gbm->h);
	gbm_file_write(fd, s, (int) strlen(s));

	stride = ((gbm->w + 3) & ~3);
	p = data + ((gbm->h - 1) * stride);
	for ( i = gbm->h - 1; i >= 0; i-- )
		{
		for ( j = 0; j < gbm->w; j++ )
			linebuf[j] = grey[p[j]];
		gbm_file_write(fd, linebuf, gbm->w);
		p -= stride;
		}

	free(linebuf);

	return GBM_ERR_OK;
	}
/*...e*/
/*...spgm_err:0:*/
const char *pgm_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_PGM_BAD_M:
			return "bad maximum pixel intensity";
		}
	return NULL;
	}
/*...e*/
