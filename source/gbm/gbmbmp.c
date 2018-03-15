/*

gbmbmp.c - OS/2 1.1, 1.2, 2.0 and Windows 3.0 support

Reads and writes any OS/2 1.x bitmap.
Will also read uncompressed, RLE4 and RLE8 Windows 3.x bitmaps too.
There are horrific file structure alignment considerations hence each
gbm_u16,gbm_u32 is read individually.
Input options: index=# (default: 0)

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

#ifndef min
#define	min(a,b)	(((a)<(b))?(a):(b))
#endif
/*...e*/

/*...suseful:0:*/
/*...sread_u16:0:*/
static gbm_boolean read_u16(int fd, gbm_u16 *w)
	{
	gbm_u8 low, high;
	if ( gbm_file_read(fd, (char *) &low, 1) != 1 )
		return GBM_FALSE;
	if ( gbm_file_read(fd, (char *) &high, 1) != 1 )
		return GBM_FALSE;
	*w = (gbm_u16) (low + ((gbm_u16) high << 8));
	return GBM_TRUE;
	}
/*...e*/
/*...sread_u32:0:*/
static gbm_boolean read_u32(int fd, gbm_u32 *d)
	{
	gbm_u16 low, high;
	if ( !read_u16(fd, &low) )
		return GBM_FALSE;
	if ( !read_u16(fd, &high) )
		return GBM_FALSE;
	*d = low + ((gbm_u32) high << 16);
	return GBM_TRUE;
	}
/*...e*/
/*...swrite_u16:0:*/
static gbm_boolean write_u16(int fd, gbm_u16 w)
	{
	gbm_u8 low  = (gbm_u8)  w      ;
	gbm_u8 high = (gbm_u8) (w >> 8);
	gbm_file_write(fd, &low , 1);
	gbm_file_write(fd, &high, 1);
	return GBM_TRUE;
	}
/*...e*/
/*...swrite_u32:0:*/
static gbm_boolean write_u32(int fd, gbm_u32 d)
	{
	write_u16(fd, (gbm_u16)  d       );
	write_u16(fd, (gbm_u16) (d >> 16));
	return GBM_TRUE;
	}
/*...e*/
/*...e*/

static GBMFT bmp_gbmft =
	{
	"Bitmap",
	"OS/2 1.1, 1.2, 2.0 / Windows 3.0 bitmap",
	"BMP VGA BGA RLE DIB RL4 RL8",
	GBM_FT_R1|GBM_FT_R4|GBM_FT_R8|GBM_FT_R24|
	GBM_FT_W1|GBM_FT_W4|GBM_FT_W8|GBM_FT_W24,
	};

#define	GBM_ERR_BMP_PLANES	((GBM_ERR) 300)
#define	GBM_ERR_BMP_BITCOUNT	((GBM_ERR) 301)
#define	GBM_ERR_BMP_CBFIX	((GBM_ERR) 302)
#define	GBM_ERR_BMP_COMP	((GBM_ERR) 303)
#define	GBM_ERR_BMP_OFFSET	((GBM_ERR) 304)

typedef struct
	{
	gbm_u32 base;
	gbm_boolean windows;
	gbm_u32 cbFix;
	gbm_u32 ulCompression;
	gbm_u32 cclrUsed;
	gbm_u32 offBits;
	gbm_boolean inv, invb;
	} BMP_PRIV;

#define	BFT_BMAP	0x4d42
#define	BFT_BITMAPARRAY	0x4142
#define	BCA_UNCOMP	0x00000000L
#define	BCA_RLE8	0x00000001L
#define	BCA_RLE4	0x00000002L
#define	BCA_HUFFFMAN1D	0x00000003L
#define	BCA_RLE24	0x00000004L
#define	MSWCC_EOL	0
#define	MSWCC_EOB	1
#define	MSWCC_DELTA	2

/*...sinvert:0:*/
static void invert(gbm_u8 *buffer, unsigned count)
	{
	while ( count-- )
		*buffer++ ^= (gbm_u8) 0xffU;
	}
/*...e*/
/*...sswap_pal:0:*/
static void swap_pal(GBMRGB *gbmrgb)
	{
	GBMRGB tmp = gbmrgb[0];
	gbmrgb[0] = gbmrgb[1];
	gbmrgb[1] = tmp;
	}
/*...e*/

/*...sbmp_qft:0:*/
GBM_ERR bmp_qft(GBMFT *gbmft)
	{
	*gbmft = bmp_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...sbmp_rhdr:0:*/
GBM_ERR bmp_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	gbm_u16 usType, xHotspot, yHotspot;
	gbm_u32 cbSize, offBits, cbFix;
	BMP_PRIV *bmp_priv = (BMP_PRIV *) gbm->priv;
	bmp_priv->inv  = ( gbm_find_word(opt, "inv" ) != NULL );
	bmp_priv->invb = ( gbm_find_word(opt, "invb") != NULL );

	fn=fn; /* Suppress 'unref arg' compiler warnings */

	if ( !read_u16(fd, &usType) )
		return GBM_ERR_READ;
	if ( usType == BFT_BITMAPARRAY )
/*...shandle bitmap arrays:16:*/
{
const char *index;
int i;

if ( (index = gbm_find_word_prefix(opt, "index=")) != NULL )
	sscanf(index + 6, "%d", &i);
else
	i = 0;

while ( i-- > 0 )
	{
	gbm_u32 cbSize2, offNext;

	if ( !read_u32(fd, &cbSize2) )
		return GBM_ERR_READ;
	if ( !read_u32(fd, &offNext) )
		return GBM_ERR_READ;
	if ( offNext == 0L )
		return GBM_ERR_BMP_OFFSET;
	gbm_file_lseek(fd, (long) offNext, SEEK_SET);
	if ( !read_u16(fd, &usType) )
		return GBM_ERR_READ;
	if ( usType != BFT_BITMAPARRAY )
		return GBM_ERR_BAD_MAGIC;
	}
gbm_file_lseek(fd, 4L + 4L + 2L + 2L, SEEK_CUR);
if ( !read_u16(fd, &usType) )
	return GBM_ERR_READ;
}
/*...e*/

	if ( usType != BFT_BMAP )
		return GBM_ERR_BAD_MAGIC;

	bmp_priv->base = (gbm_u32) ( gbm_file_lseek(fd, 0L, SEEK_CUR) - 2L );

	if ( !read_u32(fd, &cbSize) )
		return GBM_ERR_READ;
	if ( !read_u16(fd, &xHotspot) )
		return GBM_ERR_READ;
	if ( !read_u16(fd, &yHotspot) )
		return GBM_ERR_READ;
	if ( !read_u32(fd, &offBits) )
		return GBM_ERR_READ;
	if ( !read_u32(fd, &cbFix) )
		return GBM_ERR_READ;

	bmp_priv->offBits = offBits;

	if ( cbFix == 12 )
/*...sOS\47\2 1\46\1\44\ 1\46\2:16:*/
/* OS/2 1.x uncompressed bitmap */
{
gbm_u16 cx, cy, cPlanes, cBitCount;

if ( !read_u16(fd, &cx) )
	return GBM_ERR_READ;
if ( !read_u16(fd, &cy) )
	return GBM_ERR_READ;
if ( !read_u16(fd, &cPlanes) )
	return GBM_ERR_READ;
if ( !read_u16(fd, &cBitCount) )
	return GBM_ERR_READ;

if ( cx == 0 || cy == 0 )
	return GBM_ERR_BAD_SIZE;
if ( cPlanes != 1 )
	return GBM_ERR_BMP_PLANES;
if ( cBitCount != 1 && cBitCount != 4 && cBitCount != 8 && cBitCount != 24 )
	return GBM_ERR_BMP_BITCOUNT;

gbm->w   = (int) cx;
gbm->h   = (int) cy;
gbm->bpp = (int) cBitCount;

bmp_priv->windows = GBM_FALSE;
}
/*...e*/
	else if ( cbFix >= 16 && cbFix <= 64 &&
	          ((cbFix & 3) == 0 || cbFix == 42 || cbFix == 46) )
/*...sOS\47\2 2\46\0 and Windows 3\46\0:16:*/
{
gbm_u16 cPlanes, cBitCount, usUnits, usReserved, usRecording, usRendering;
gbm_u32 ulWidth, ulHeight, ulCompression;
gbm_u32 ulSizeImage, ulXPelsPerMeter, ulYPelsPerMeter;
gbm_u32 cclrUsed, cclrImportant, cSize1, cSize2, ulColorEncoding, ulIdentifier;
gbm_boolean ok;

ok  = read_u32(fd, &ulWidth);
ok &= read_u32(fd, &ulHeight);
ok &= read_u16(fd, &cPlanes);
ok &= read_u16(fd, &cBitCount);
if ( cbFix > 16 )
	ok &= read_u32(fd, &ulCompression);
else
	ulCompression = BCA_UNCOMP;
if ( cbFix > 20 )
	ok &= read_u32(fd, &ulSizeImage);
if ( cbFix > 24 )
	ok &= read_u32(fd, &ulXPelsPerMeter);
if ( cbFix > 28 )
	ok &= read_u32(fd, &ulYPelsPerMeter);
if ( cbFix > 32 )
	ok &= read_u32(fd, &cclrUsed);
else
	cclrUsed = ( (gbm_u32)1 << cBitCount );
if ( cBitCount != 24 && cclrUsed == 0 )
	cclrUsed = ( (gbm_u32)1 << cBitCount );

/* Protect against badly written bitmaps! */
if ( cclrUsed > ( (gbm_u32)1 << cBitCount ) )
	cclrUsed = ( (gbm_u32)1 << cBitCount );

if ( cbFix > 36 )
	ok &= read_u32(fd, &cclrImportant);
if ( cbFix > 40 )
	ok &= read_u16(fd, &usUnits);
if ( cbFix > 42 )
	ok &= read_u16(fd, &usReserved);
if ( cbFix > 44 )
	ok &= read_u16(fd, &usRecording);
if ( cbFix > 46 )
	ok &= read_u16(fd, &usRendering);
if ( cbFix > 48 )
	ok &= read_u32(fd, &cSize1);
if ( cbFix > 52 )
	ok &= read_u32(fd, &cSize2);
if ( cbFix > 56 )
	ok &= read_u32(fd, &ulColorEncoding);
if ( cbFix > 60 )
	ok &= read_u32(fd, &ulIdentifier);

if ( !ok )
	return GBM_ERR_READ;

if ( ulWidth == 0L || ulHeight == 0L )
	return GBM_ERR_BAD_SIZE;
if ( cPlanes != 1 )
	return GBM_ERR_BMP_PLANES;
if ( cBitCount != 1 && cBitCount != 4 && cBitCount != 8 && cBitCount != 24 )
	return GBM_ERR_BMP_BITCOUNT;

gbm->w   = (int) ulWidth;
gbm->h   = (int) ulHeight;
gbm->bpp = (int) cBitCount;

bmp_priv->windows       = GBM_TRUE;
bmp_priv->cbFix         = cbFix;
bmp_priv->ulCompression = ulCompression;
bmp_priv->cclrUsed      = cclrUsed;
}
/*...e*/
	else
		return GBM_ERR_BMP_CBFIX;

	return GBM_ERR_OK;
	}
/*...e*/
/*...sbmp_rpal:0:*/
GBM_ERR bmp_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	BMP_PRIV *bmp_priv = (BMP_PRIV *) gbm->priv;
	if ( gbm->bpp != 24 )
		{
		int i;
		gbm_u8 b[4];
		if ( bmp_priv->windows )
/*...sOS\47\2 2\46\0 and Windows 3\46\0:24:*/
{
gbm_file_lseek(fd, (long) (bmp_priv->base + 14L + bmp_priv->cbFix), SEEK_SET);
for ( i = 0; i < (1 << gbm->bpp); i++ )
/* Used to use bmp_priv->cclrUsed, but bitmaps have been found which have
   their used colours not at the start of the palette. */
	{
	gbm_file_read(fd, b, 4);
	gbmrgb[i].b = b[0];
	gbmrgb[i].g = b[1];
	gbmrgb[i].r = b[2];
	}
}
/*...e*/
		else
/*...sOS\47\2 1\46\1\44\ 1\46\2:24:*/
{
gbm_file_lseek(fd, (long) (bmp_priv->base + 26L), SEEK_SET);
for ( i = 0; i < (1 << gbm->bpp); i++ )
	{
	gbm_file_read(fd, b, 3);
	gbmrgb[i].b = b[0];
	gbmrgb[i].g = b[1];
	gbmrgb[i].r = b[2];
	}
}
/*...e*/
		}
	if ( gbm->bpp == 1 && !bmp_priv->inv )
		swap_pal(gbmrgb);
	return GBM_ERR_OK;
	}
/*...e*/
/*...sbmp_rdata:0:*/
GBM_ERR bmp_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	BMP_PRIV *bmp_priv = (BMP_PRIV *) gbm->priv;
	int cLinesWorth = ((gbm->bpp * gbm->w + 31) / 32) * 4;
	if ( bmp_priv->windows )
/*...sOS\47\2 2\46\0 and Windows 3\46\0:16:*/
{
gbm_file_lseek(fd, (long)bmp_priv->offBits, SEEK_SET);
switch ( (int) bmp_priv->ulCompression )
	{
/*...sBCA_UNCOMP:24:*/
case BCA_UNCOMP:
	gbm_file_read(fd, data, gbm->h * cLinesWorth);
	break;
/*...e*/
/*...sBCA_RLE8:24:*/
case BCA_RLE8:
	{
	AHEAD *ahead;
	int x = 0, y = 0;
	gbm_boolean eof8 = GBM_FALSE;

	if ( (ahead = gbm_create_ahead(fd)) == NULL )
		return GBM_ERR_MEM;

	while ( !eof8 )
		{
		gbm_u8 c = (gbm_u8) gbm_read_ahead(ahead);
		gbm_u8 d = (gbm_u8) gbm_read_ahead(ahead);
/* fprintf(stderr, "(%d,%d) c=%d,d=%d\n", x, y, c, d); debug*/

		if ( c )
			{
			memset(data, d, c);
			x += c;
			data += c;
			}
		else
			switch ( d )
				{
/*...sMSWCC_EOL:56:*/
case MSWCC_EOL:
	{
	int to_eol = cLinesWorth - x;

	memset(data, 0, (size_t) to_eol);
	data += to_eol;
	x = 0;
	if ( ++y == gbm->h )
		eof8 = GBM_TRUE;
	}
	break;
/*...e*/
/*...sMSWCC_EOB:56:*/
case MSWCC_EOB:
	if ( y < gbm->h )
		{
		int to_eol = cLinesWorth - x;

		memset(data, 0, (size_t) to_eol);
		x = 0; y++;
		data += to_eol;
		while ( y < gbm->h )
			{
			memset(data, 0, (size_t) cLinesWorth);
			data += cLinesWorth;
			y++;
			}
		}
	eof8 = GBM_TRUE;
	break;
/*...e*/
/*...sMSWCC_DELTA:56:*/
case MSWCC_DELTA:
	{
	gbm_u8 dx = (gbm_u8) gbm_read_ahead(ahead);
	gbm_u8 dy = (gbm_u8) gbm_read_ahead(ahead);
	int fill = dx + dy * cLinesWorth;
	memset(data, 0, (size_t) fill);
	data += fill;
	x += dx; y += dy;
	if ( y == gbm->h )
		eof8 = GBM_TRUE;
	}
	break;
/*...e*/
/*...sdefault:56:*/
default:
	{
	int n = (int) d;

	while ( n-- > 0 )
		*data++ = (gbm_u8) gbm_read_ahead(ahead);
	x += d;
if ( x > gbm->w ) { x = 0; ++y; } /* @@@AK */
	if ( d & 1 )
		gbm_read_ahead(ahead); /* Align */
	}
	break;
/*...e*/
				}
		}

	gbm_destroy_ahead(ahead);
	}
	break;
/*...e*/
/*...sBCA_RLE4:24:*/
case BCA_RLE4:
	{
	AHEAD *ahead;
	int x = 0, y = 0;
	gbm_boolean eof4 = GBM_FALSE;
	int inx = 0;

	if ( (ahead = gbm_create_ahead(fd)) == NULL )
		return GBM_ERR_MEM;

	memset(data, 0, (size_t) (gbm->h * cLinesWorth));

	while ( !eof4 )
		{
		gbm_u8 c = (gbm_u8) gbm_read_ahead(ahead);
		gbm_u8 d = (gbm_u8) gbm_read_ahead(ahead);

		if ( c )
			{
			gbm_u8 h, l;
			int i;
			if ( x & 1 )
				{ h = (gbm_u8) (d >> 4); l = (gbm_u8) (d << 4); }
			else
				{ h = (gbm_u8) (d&0xf0); l = (gbm_u8) (d&0x0f); }
			for ( i = 0; i < (int) c; i++, x++ )
				{
				if ( x & 1U )
					data[inx++] |= l;
				else
					data[inx]   |= h;
				}
			}
		else
			switch ( d )
				{
/*...sMSWCC_EOL:56:*/
case MSWCC_EOL:
	x = 0;
	if ( ++y == gbm->h )
		eof4 = GBM_TRUE;
	inx = cLinesWorth * y;
	break;
/*...e*/
/*...sMSWCC_EOB:56:*/
case MSWCC_EOB:
	eof4 = GBM_TRUE;
	break;
/*...e*/
/*...sMSWCC_DELTA:56:*/
case MSWCC_DELTA:
	{
	gbm_u8 dx = (gbm_u8) gbm_read_ahead(ahead);
	gbm_u8 dy = (gbm_u8) gbm_read_ahead(ahead);

	x += dx; y += dy;
	inx = y * cLinesWorth + (x/2);
		
	if ( y == gbm->h )
		eof4 = GBM_TRUE;
	}
	break;
/*...e*/
/*...sdefault:56:*/
default:
	{
	int i, nr = 0;

	if ( x & 1 )
		{
		for ( i = 0; i+2 <= (int) d; i += 2 )
			{
			gbm_u8 b = (gbm_u8) gbm_read_ahead(ahead);
			data[inx++] |= (b >> 4);
			data[inx  ] |= (b << 4);
			nr++;
			}
		if ( i < (int) d )
			{
			data[inx++] |= ((gbm_u8) gbm_read_ahead(ahead) >> 4);
			nr++;
			}
		}
	else
		{
		for ( i = 0; i+2 <= (int) d; i += 2 )
			{
			data[inx++] = (gbm_u8) gbm_read_ahead(ahead);
			nr++;
			}
		if ( i < (int) d )
			{
			data[inx] = (gbm_u8) gbm_read_ahead(ahead);
			nr++;
			}
		}
	x += d;

	if ( nr & 1 )
		gbm_read_ahead(ahead); /* Align input stream to next gbm_u16 */
	}
	break;
/*...e*/
				}
		}

	gbm_destroy_ahead(ahead);
	}
	break;
/*...e*/
/*...sdefault:24:*/
default:
	return GBM_ERR_BMP_COMP;
/*...e*/
	}
}
/*...e*/
	else
/*...sOS\47\2 1\46\1\44\ 1\46\2:16:*/
{
gbm_file_lseek(fd, (long) bmp_priv->offBits, SEEK_SET);
gbm_file_read(fd, data, cLinesWorth * gbm->h);
}
/*...e*/
	if ( bmp_priv->invb )
		invert(data, (unsigned) (cLinesWorth * gbm->h));
	return GBM_ERR_OK;
	}
/*...e*/
/*...sbmp_w:0:*/
/*...sbright:0:*/
static int bright(const GBMRGB *gbmrgb)
	{
	return gbmrgb->r*30+gbmrgb->g*60+gbmrgb->b*10;
	}
/*...e*/
/*...swrite_inv:0:*/
static int write_inv(int fd, const gbm_u8 *buffer, int count)
	{
	gbm_u8 small_buf[1024];
	int so_far = 0, this_go, written;

	while ( so_far < count )
		{
		this_go = min(count - so_far, 1024);
		memcpy(small_buf, buffer + so_far, (size_t) this_go);
		invert(small_buf, (unsigned) this_go);
		if ( (written = gbm_file_write(fd, small_buf, this_go)) != this_go )
			return so_far + written;
		so_far += written;
		}

	return so_far;
	}
/*...e*/

GBM_ERR bmp_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	gbm_boolean pm11    = ( gbm_find_word(opt, "1.1"    ) != NULL );
	gbm_boolean win     = ( gbm_find_word(opt, "win"    ) != NULL ||
	                        gbm_find_word(opt, "2.0"    ) != NULL );
	gbm_boolean inv     = ( gbm_find_word(opt, "inv"    ) != NULL );
	gbm_boolean invb    = ( gbm_find_word(opt, "invb"   ) != NULL );
	gbm_boolean darkfg  = ( gbm_find_word(opt, "darkfg" ) != NULL );
	gbm_boolean lightfg = ( gbm_find_word(opt, "lightfg") != NULL );
	int cRGB;
	GBMRGB gbmrgb_1bpp[2];

	if ( pm11 && win )
		return GBM_ERR_BAD_OPTION;

	fn=fn; /* Suppress 'unref arg' compiler warning */

	cRGB = ( (1 << gbm->bpp) & 0x1ff );
		/* 1->2, 4->16, 8->256, 24->0 */

	if ( cRGB == 2 )
/*...shandle messy 1bpp case:16:*/
{
/*
The palette entries inside a 1bpp PM bitmap are not honored, or handled
correctly by most programs. Current thinking is that they have no actual
meaning. Under OS/2 PM, bitmap 1's re fg and 0's are bg, and it is the job of
the displayer to pick fg and bg. We will pick fg=black, bg=white in the bitmap
file we save. If we do not write black and white, we find that most programs
will incorrectly honor these entries giving unpredicatable (and often black on
a black background!) results.
*/

gbmrgb_1bpp[0].r = gbmrgb_1bpp[0].g = gbmrgb_1bpp[0].b = 0xff;
gbmrgb_1bpp[1].r = gbmrgb_1bpp[1].g = gbmrgb_1bpp[1].b = 0x00;

/*
We observe these values must be the wrong way around to keep most PM
programs happy, such as WorkPlace Shell WPFolder backgrounds.
*/

if ( !inv )
	swap_pal(gbmrgb_1bpp);

/*
If the user has picked the darkfg option, then they intend that the darkest
colour in the image is to be the foreground. This is a very sensible option
because the foreground will appear to be black when the image is reloaded.
To acheive this we must invert the bitmap bits, if the palette dictates.
*/

if ( darkfg && bright(&gbmrgb[0]) < bright(&gbmrgb[1]) )
	invb = !invb;
if ( lightfg && bright(&gbmrgb[0]) >= bright(&gbmrgb[1]) )
	invb = !invb;

gbmrgb = gbmrgb_1bpp;
}
/*...e*/

	if ( pm11 )
/*...sOS\47\2 1\46\1:16:*/
{
gbm_u16 usType     = BFT_BMAP;
gbm_u16 xHotspot   = 0;
gbm_u16 yHotspot   = 0;
gbm_u32 cbFix     = (gbm_u32) 12;
gbm_u16 cx         = (gbm_u16) gbm->w;
gbm_u16 cy         = (gbm_u16) gbm->h;
gbm_u16 cPlanes    = (gbm_u16) 1;
gbm_u16 cBitCount  = (gbm_u16) gbm->bpp;
int cLinesWorth = (((cBitCount * cx + 31) / 32) * cPlanes) * 4;
gbm_u32 offBits   = (gbm_u32) 26 + cRGB * (gbm_u32) 3;
gbm_u32 cbSize    = offBits + (gbm_u32) cy * (gbm_u32) cLinesWorth;
int i, total, actual;

write_u16(fd, usType);
write_u32(fd, cbSize);
write_u16(fd, xHotspot);
write_u16(fd, yHotspot);
write_u32(fd, offBits);
write_u32(fd, cbFix);
write_u16(fd, cx);
write_u16(fd, cy);
write_u16(fd, cPlanes);
write_u16(fd, cBitCount);

for ( i = 0; i < cRGB; i++ )
	{
	gbm_u8 b[3];

	b[0] = gbmrgb[i].b;
	b[1] = gbmrgb[i].g;
	b[2] = gbmrgb[i].r;
	if ( gbm_file_write(fd, b, 3) != 3 )
		return GBM_ERR_WRITE;
	}

total = gbm->h * cLinesWorth;
if ( invb )
	actual = write_inv(fd, data, total);
else
	actual = gbm_file_write(fd, data, total);
if ( actual != total )
	return GBM_ERR_WRITE;
}
/*...e*/
	else
/*...sOS\47\2 2\46\0 and Windows 3\46\0:16:*/
{
gbm_u16 usType        = BFT_BMAP;
gbm_u16 xHotspot      = 0;
gbm_u16 yHotspot      = 0;
gbm_u32 cbFix         = (gbm_u32) 40;
gbm_u32 cx            = (gbm_u32) gbm->w;
gbm_u32 cy            = (gbm_u32) gbm->h;
gbm_u16 cPlanes       = (gbm_u16) 1;
gbm_u16 cBitCount     = (gbm_u16) gbm->bpp;
int cLinesWorth       = (((cBitCount * (int) cx + 31) / 32) * cPlanes) * 4;
gbm_u32 offBits       = (gbm_u32) 54 + cRGB * (gbm_u32) 4;
gbm_u32 cbSize        = offBits + (gbm_u32) cy * (gbm_u32) cLinesWorth;
gbm_u32 ulCompression = BCA_UNCOMP;
gbm_u32 cbImage       = (gbm_u32) cLinesWorth * (gbm_u32) gbm->h;
gbm_u32 cxResolution  = 0;
gbm_u32 cyResolution  = 0;
gbm_u32 cclrUsed      = 0;
gbm_u32 cclrImportant = 0;
int i, total, actual;

write_u16(fd, usType);
write_u32(fd, cbSize);
write_u16(fd, xHotspot);
write_u16(fd, yHotspot);
write_u32(fd, offBits);

write_u32(fd, cbFix);
write_u32(fd, cx);
write_u32(fd, cy);
write_u16(fd, cPlanes);
write_u16(fd, cBitCount);
write_u32(fd, ulCompression);
write_u32(fd, cbImage);
write_u32(fd, cxResolution);
write_u32(fd, cyResolution);
write_u32(fd, cclrUsed);
write_u32(fd, cclrImportant);

for ( i = 0; i < cRGB; i++ )
	{
	gbm_u8 b[4];
	b[0] = gbmrgb[i].b;
	b[1] = gbmrgb[i].g;
	b[2] = gbmrgb[i].r;
	b[3] = 0;
	if ( gbm_file_write(fd, b, 4) != 4 )
		return GBM_ERR_WRITE;
	}

total = gbm->h * cLinesWorth;
if ( invb )
	actual = write_inv(fd, data, total);
else
	actual = gbm_file_write(fd, data, total);
if ( actual != total )
	return GBM_ERR_WRITE;
}
/*...e*/

	return GBM_ERR_OK;
	}
/*...e*/
/*...sbmp_err:0:*/
const char *bmp_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_BMP_PLANES:
			return "number of bitmap planes is not 1";
		case GBM_ERR_BMP_BITCOUNT:
			return "bit count not 1, 4, 8 or 24";
		case GBM_ERR_BMP_CBFIX:
			return "cbFix bad";
		case GBM_ERR_BMP_COMP:
			return "compression type not uncompressed, RLE4 or RLE8";
		case GBM_ERR_BMP_OFFSET:
			return "less bitmaps in file than index requested";
		}
	return NULL;
	}
/*...e*/
