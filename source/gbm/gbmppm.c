/*

gbmppm.c - Poskanzers PPM format

Reads and writes 24 bit RGB.

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

static GBMFT ppm_gbmft =
	{
	"Pixmap",
	"Portable Pixel-map (binary P6 type)",
	"PPM",
	GBM_FT_R24|
	GBM_FT_W24,
	};

#define	GBM_ERR_PPM_BAD_M	((GBM_ERR) 200)

/*...srgb_bgr:0:*/
static void rgb_bgr(const gbm_u8 *p, gbm_u8 *q, int n)
	{
	while ( n-- )
		{
		gbm_u8 r = *p++;
		gbm_u8 g = *p++;
		gbm_u8 b = *p++;
		*q++ = b;
		*q++ = g;
		*q++ = r;
		}
	}
/*...e*/

/*...sppm_qft:0:*/
GBM_ERR ppm_qft(GBMFT *gbmft)
	{
	*gbmft = ppm_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...sppm_rhdr:0:*/
GBM_ERR ppm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	int h1, h2, w, h, m;

	fn=fn; opt=opt; /* Suppress 'unref arg' compiler warnings */

	read_posk_header(fd, &h1, &h2, &w, &h, &m);
	if ( h1 != 'P' || h2 != '6' )
		return GBM_ERR_BAD_MAGIC;

	if ( w <= 0 || h <= 0 )
		return GBM_ERR_BAD_SIZE;

	if ( m <= 1 || m >= 0x100 )
		return GBM_ERR_PPM_BAD_M;

	gbm->w   = w;
	gbm->h   = h;
	gbm->bpp = 24;

	return GBM_ERR_OK;
	}
/*...e*/
/*...sppm_rpal:0:*/
GBM_ERR ppm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	fd=fd; gbm=gbm; gbmrgb=gbmrgb; /* Suppress 'unref arg' compiler warnings */

	return GBM_ERR_OK;
	}
/*...e*/
/*...sppm_rdata:0:*/
GBM_ERR ppm_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	int h1, h2, w, h, m, i, stride;
	gbm_u8 *p;

	read_posk_header(fd, &h1, &h2, &w, &h, &m);

	stride = ((gbm->w * 3 + 3) & ~3);
	p = data + ((gbm->h - 1) * stride);
	for ( i = gbm->h - 1; i >= 0; i-- )
		{
		gbm_file_read(fd, p, gbm->w * 3);
		rgb_bgr(p, p, gbm->w);
		p -= stride;
		}
	if ( m < 255 )
		for ( i = 0; i < stride*h; i++ )
			p[i] = (gbm_u8) ((p[i]*255U)/(unsigned)m);
	return GBM_ERR_OK;
	}
/*...e*/
/*...sppm_w:0:*/
GBM_ERR ppm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	char s[100+1];
	int i, stride;
	const gbm_u8 *p;
	gbm_u8 *linebuf;

	fn=fn; gbmrgb=gbmrgb; opt=opt; /* Suppress 'unref arg' compiler warnings */

	if ( gbm->bpp != 24 )
		return GBM_ERR_NOT_SUPP;

	if ( (linebuf = malloc((size_t) (gbm->w * 3))) == NULL )
		return GBM_ERR_MEM;

	sprintf(s, "P6\n%d %d\n255\n", gbm->w, gbm->h);
	gbm_file_write(fd, s, (int) strlen(s));

	stride = ((gbm->w * 3 + 3) & ~3);
	p = data + ((gbm->h - 1) * stride);
	for ( i = gbm->h - 1; i >= 0; i-- )
		{
		rgb_bgr(p, linebuf, gbm->w);
		gbm_file_write(fd, linebuf, gbm->w * 3);
		p -= stride;
		}

	free(linebuf);

	return GBM_ERR_OK;
	}
/*...e*/
/*...sppm_err:0:*/
const char *ppm_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_PPM_BAD_M:
			return "bad maximum pixel intensity";
		}
	return NULL;
	}
/*...e*/
