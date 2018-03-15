/*

gbmtga.c - Truevision Targa/Vista support

Reads any uncompressed or runlength compresed type as 8 bit or 24 bit.
Writes out only as 8 bit or 24 bit.
Output options: 16,24,32, ydown,yup (default: 24 yup)

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

/*...suseful:0:*/
#define	low_u8(w)	((gbm_u8)  (          (w)&0x00ff)    )
#define	high_u8(w)	((gbm_u8) (((unsigned)(w)&0xff00)>>8))
#define	make_u16(a,b)	(((gbm_u16)a) + (((gbm_u16)b) << 8))
/*...e*/

static GBMFT tga_gbmft =
	{
	"Targa",
	"Truevision Targa/Vista bitmap",
	"TGA VST AFI",
	GBM_FT_R8|GBM_FT_R24|
	GBM_FT_W8|GBM_FT_W24,
	};

#define	GBM_ERR_TGA_BAD_BPP	((GBM_ERR) 400)
#define	GBM_ERR_TGA_BAD_TYPE	((GBM_ERR) 401)
#define	GBM_ERR_TGA_BAD_PAL	((GBM_ERR) 402)

/*...starga file header definition:0:*/
/*

A Targa file is a header, followed by an identification string, followed by
a color map, followed by the data for the image. Both the identification
string and the color-map can be of zero length.

*/

#define	TGA_NO_IMAGE	0		/* No image data included in file    */
#define	TGA_UNCOMP_CM	1		/* Uncompressed, Color-Mapped (VDA/D */
					/* and Targa M-8 images)             */
#define	TGA_UNCOMP_RGB	2		/* Uncompressed, RGB images (eg: ICB */
					/* Targa 16, 24 and 32)              */
#define	TGA_UNCOMP_BW	3		/* Uncompressed, B/W images (eg:     */
					/* Targa 8 and Targa M-8 images)     */
#define	TGA_RL_CM	9		/* Run-length, Color-Mapped (VDA/D   */
					/* and Targa M-8 images)             */
#define	TGA_RL_RGB	10		/* Run-length, RGB images (eg: ICB   */
					/* Targa 16, 24 and 32)              */
#define	TGA_RL_BW	11		/* Run-length, B/W images (eg: Targa */
					/* 8 and Targa M-8)                  */
#define	TGA_COMP_CM	32		/* Compressed Color-Mapped (VDA/D)   */
					/* data using Huffman, Delta, and    */
					/* run length encoding               */
#define	TGA_COMP_CM_4	33		/* Compressed Color-Mapped (VDA/D)   */
					/* data using Huffman, Delta, and    */
					/* run length encoding in 4 passes   */

#define	IDB_ATTRIBUTES	0x0f		/* How many attrib bits per pixel    */
					/* ie: 1 for T16, 8 for T32          */
#define	IDB_ORIGIN	0x20		/* Origin in top left corner bit     */
					/* else its in bottom left corner    */
#define	IDB_INTERLEAVE	0xc0		/* Interleave bits as defined below  */
#define	IDB_NON_INT	0x00		/* Non-Interlaced                    */
#define	IDB_2_WAY	0x40		/* 2 way (even/odd) interleaving     */
#define	IDB_4_WAY	0x80		/* 4 way interleaving (eg: AT&T PC)  */

typedef struct
	{
	gbm_u8 n_chars_in_id;		/* Length of identification text     */
	gbm_u8 color_map_present;	/* 0 means no, 1 yes                 */
	gbm_u8 image_type;		/* Type of image file, one of TGA_   */
	gbm_u8 color_map_start_low;	/* These 5 gbm_u8s are only valid if   */
	gbm_u8 color_map_start_high;	/* color_map_present is 1. They      */
	gbm_u8 color_map_length_low;	/* Specify the size of the color map */
	gbm_u8 color_map_length_high;	/* and where it starts from          */
	gbm_u8 color_map_entry_bits;	/* Bits per color map entry          */
					/* Typically 15, 16, 24 or 32        */
	gbm_u8 x_origin_low;
	gbm_u8 x_origin_high;
	gbm_u8 y_origin_low;
	gbm_u8 y_origin_high;
	gbm_u8 width_low;
	gbm_u8 width_high;
	gbm_u8 height_low;
	gbm_u8 height_high;
	gbm_u8 bpp;			/* Typically 16, 24 or 32            */
	gbm_u8 image_descriptor;	/* Split into IDB_ bits              */
	} TGA_HEADER;

#define	SIZEOF_TGA_HEADER 18
/*...e*/
/*...sconverters:0:*/
/*...st24_t32:0:*/
static void t24_t32(gbm_u8 *dest, const gbm_u8 *src, int n)
	{
	while ( n-- )
		{
		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;
		dest++;
		}
	}
/*...e*/
/*...st32_t24:0:*/
static void t32_t24(gbm_u8 *dest, const gbm_u8 *src, int n)
	{
	while ( n-- )
		{
		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;
		src++;
		}
	}
/*...e*/
/*...st24_t16:0:*/
static void t24_t16(gbm_u8 *dest, const gbm_u8 *src, int n)
	{
	while ( n-- )
		{
		gbm_u16	b = (gbm_u16) (*src++ & 0xf8);
		gbm_u16	g = (gbm_u16) (*src++ & 0xf8);
		gbm_u16	r = (gbm_u16) (*src++ & 0xf8);
		gbm_u16	w = ((r << 7) | (g << 2) | (b >> 3));
		*dest++ = (gbm_u8)  w;
		*dest++ = (gbm_u8) (w >> 8);
		}
	}
/*...e*/
/*...st16_t24:0:*/
static void t16_t24(gbm_u8 *dest, const gbm_u8 *src, int n)
	{
	while ( n-- )
		{
		gbm_u16 l = *src++;
		gbm_u16 h = *src++;
		gbm_u16 w = l + (h << 8);
		*dest++ = (gbm_u8) ((w & 0x001fU) << 3);
		*dest++ = (gbm_u8) ((w & 0x03e0U) >> 2);
		*dest++ = (gbm_u8) ((w & 0x7c00U) >> 7);
		}
	}
/*...e*/
/*...e*/
typedef struct
	{
	TGA_HEADER header;
	} TGA_PRIV;

/*...stga_qft:0:*/
GBM_ERR tga_qft(GBMFT *gbmft)
	{
	*gbmft = tga_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...stga_rhdr:0:*/
GBM_ERR tga_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	TGA_PRIV *priv = (TGA_PRIV *) gbm->priv;

	fn=fn; opt=opt; /* Suppress 'unref arg' compiler warnings */

	gbm_file_read(fd, (char *) &(priv->header), SIZEOF_TGA_HEADER);

	switch ( priv->header.image_type )
		{
		case TGA_RL_BW:
		case TGA_UNCOMP_BW:
			if ( priv->header.bpp != 8 )
				return GBM_ERR_TGA_BAD_BPP;
			break;
		case TGA_RL_CM:
		case TGA_UNCOMP_CM:
			if ( priv->header.bpp != 8 )
				return GBM_ERR_TGA_BAD_BPP;
			if ( priv->header.color_map_entry_bits != 24 )
				return GBM_ERR_TGA_BAD_PAL;
			break;
		case TGA_RL_RGB:
		case TGA_UNCOMP_RGB:
			if ( priv->header.bpp != 16 &&
			     priv->header.bpp != 24 &&
			     priv->header.bpp != 32 )
				return GBM_ERR_TGA_BAD_BPP;
			break;
		default:
			return GBM_ERR_TGA_BAD_TYPE;
		}

	gbm->w = make_u16(priv->header.width_low , priv->header.width_high );
	gbm->h = make_u16(priv->header.height_low, priv->header.height_high);

	if ( gbm->w <= 0 || gbm->h <= 0 )
		return GBM_ERR_BAD_SIZE;

	gbm->bpp = ( priv->header.bpp == 8 ) ? 8 : 24;

	return GBM_ERR_OK;
	}
/*...e*/
/*...stga_rpal:0:*/
GBM_ERR tga_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	TGA_PRIV *priv = (TGA_PRIV *) gbm->priv;
	switch ( priv->header.image_type )
		{
/*...sTGA_UNCOMP_BW\44\ TGA_RL_BW:16:*/
case TGA_UNCOMP_BW:
case TGA_RL_BW:
	{
	int i;
	for (  i = 0; i < 0x100; i++ )
		gbmrgb[i].b =
		gbmrgb[i].g =
		gbmrgb[i].r = (gbm_u8) i;
	}
	break;
/*...e*/
/*...sTGA_UNCOMP_CM\44\ TGA_RL_CM:16:*/
case TGA_UNCOMP_CM:
case TGA_RL_CM:
	{
	int color_map_start, color_map_length, i;
	gbm_file_lseek(fd, (long) (SIZEOF_TGA_HEADER+priv->header.n_chars_in_id), SEEK_SET);
	color_map_start  = make_u16(priv->header.color_map_start_low , priv->header.color_map_start_high );
	color_map_length = make_u16(priv->header.color_map_length_low, priv->header.color_map_length_high);
	memset(gbmrgb, 0, 0x100 * sizeof(GBMRGB));
	for ( i = color_map_start; i < color_map_start + color_map_length; i++ )
		{
		gbm_u8 b[3];
		if ( gbm_file_read(fd, (char *) b, 3) != 3 )
			return GBM_ERR_READ;
		gbmrgb[i].b = b[0];
		gbmrgb[i].g = b[1];
		gbmrgb[i].r = b[2];
		}
	}
	break;
/*...e*/
		}
	return GBM_ERR_OK;
	}
/*...e*/
/*...stga_rdata:0:*/
GBM_ERR tga_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	TGA_PRIV *priv = (TGA_PRIV *) gbm->priv;
	gbm_u8 *p;
	int i, stride;

	gbm_file_lseek(fd, (long) (SIZEOF_TGA_HEADER + priv->header.n_chars_in_id), SEEK_SET);

	if ( priv->header.color_map_present )
		{
		int color_map_length;
		int bpp_pal = priv->header.color_map_entry_bits;
			/* Valid values are 32, 24, 16 and sometimes 15 */

		if ( bpp_pal == 15 ) bpp_pal = 16;

		color_map_length = make_u16(priv->header.color_map_length_low, priv->header.color_map_length_high);
		gbm_file_lseek(fd, (long) ((color_map_length * bpp_pal) / 8L), SEEK_CUR);
		}

	stride = ((gbm->w*gbm->bpp + 31) / 32) * 4;
	p = data;
	if ( (priv->header.image_descriptor & IDB_ORIGIN) != 0 )
		{
		p += (gbm->h-1) * stride;
		stride = -stride;
		}

	switch ( priv->header.image_type )
		{
/*...sTGA_RL_BW\44\ TGA_RL_CM\44\ TGA_RL_RGB:16:*/
case TGA_RL_CM:
case TGA_RL_BW:
case TGA_RL_RGB:
	{
	AHEAD *ahead;
	int x = 0, y = 0;
	if ( (ahead = gbm_create_ahead(fd)) == NULL )
		return GBM_ERR_MEM;
	switch ( priv->header.bpp )
		{
/*...s8:32:*/
case 8:
	while ( y < gbm->h )
		{
		int i, cnt = gbm_read_ahead(ahead);
		if ( cnt == -1 )
			{
			gbm_destroy_ahead(ahead);
			return GBM_ERR_READ;
			}
		if ( cnt & 0x80 )
			{
			gbm_u8 v = (gbm_u8) gbm_read_ahead(ahead);
			for ( i = 0x80; i <= cnt; i++ )
				{
				data[x++] = v;
				if ( x == gbm->w )
					{ x = 0; y++; data += stride; }
				}
			}
		else
			for ( i = 0; i <= cnt; i++ )
				{
				data[x++] = (gbm_u8) gbm_read_ahead(ahead);
				if ( x == gbm->w )
					{ x = 0; y++; data += stride; }
				}
		}
	break;
/*...e*/
/*...s16:32:*/
/*
We have 3 bytes per pixel in the data array, indexed by p.
We will read 2 bytes per pixel into the right hand 2/3 of each line.
Then we will expand leftwards to fill the full width afterwards.
*/

case 16:

	p += gbm->w;

	while ( y < gbm->h )
		{
		int i, cnt = gbm_read_ahead(ahead);
		if ( cnt == -1 )
			{
			gbm_destroy_ahead(ahead);
			return GBM_ERR_READ;
			}
		if ( cnt & 0x80 )
			{
			gbm_u8 v1 = (gbm_u8) gbm_read_ahead(ahead);
			gbm_u8 v2 = (gbm_u8) gbm_read_ahead(ahead);
			for ( i = 0x80; i <= cnt; i++ )
				{
				p[x++] = v1;
				p[x++] = v2;
				if ( x == gbm->w*2 )
					{ x = 0; y++; p += stride; }
				}
			}
		else
			for ( i = 0; i <= cnt; i++ )
				{
				p[x++] = (gbm_u8) gbm_read_ahead(ahead);
				p[x++] = (gbm_u8) gbm_read_ahead(ahead);
				if ( x == gbm->w*2 )
					{ x = 0; y++; p += stride; }
				}
		}

	if ( stride < 0 )
		stride = -stride;
	for ( y = 0, p = data; y < gbm->h; y++, p += stride )
		t16_t24(p, p+gbm->w, gbm->w);

	break;
/*...e*/
/*...s24:32:*/
case 24:
	while ( y < gbm->h )
		{
		int i, cnt = gbm_read_ahead(ahead);
		if ( cnt == -1 )
			{
			gbm_destroy_ahead(ahead);
			return GBM_ERR_READ;
			}
		if ( cnt & 0x80 )
			{
			gbm_u8 v1 = (gbm_u8) gbm_read_ahead(ahead);
			gbm_u8 v2 = (gbm_u8) gbm_read_ahead(ahead);
			gbm_u8 v3 = (gbm_u8) gbm_read_ahead(ahead);
			for ( i = 0x80; i <= cnt; i++ )
				{
				p[x++] = v1;
				p[x++] = v2;
				p[x++] = v3;
				if ( x == gbm->w*3 )
					{ x = 0; y++; p += stride; }
				}
			}
		else
			for ( i = 0; i <= cnt; i++ )
				{
				p[x++] = (gbm_u8) gbm_read_ahead(ahead);
				p[x++] = (gbm_u8) gbm_read_ahead(ahead);
				p[x++] = (gbm_u8) gbm_read_ahead(ahead);
				if ( x == gbm->w*3 )
					{ x = 0; y++; p += stride; }
				}
		}
	break;
/*...e*/
/*...s32:32:*/
case 32:
	while ( y < gbm->h )
		{
		int i, cnt = gbm_read_ahead(ahead);
		if ( cnt == -1 )
			{
			gbm_destroy_ahead(ahead);
			return GBM_ERR_READ;
			}
		if ( cnt & 0x80 )
			{
			gbm_u8 v1 = (gbm_u8) gbm_read_ahead(ahead);
			gbm_u8 v2 = (gbm_u8) gbm_read_ahead(ahead);
			gbm_u8 v3 = (gbm_u8) gbm_read_ahead(ahead);
			gbm_read_ahead(ahead); /* Discard alpha channel */
			for ( i = 0x80; i <= cnt; i++ )
				{
				p[x++] = v1;
				p[x++] = v2;
				p[x++] = v3;
				if ( x == gbm->w*3 )
					{ x = 0; y++; p += stride; }
				}
			}
		else
			for ( i = 0; i <= cnt; i++ )
				{
				p[x++] = (gbm_u8) gbm_read_ahead(ahead);
				p[x++] = (gbm_u8) gbm_read_ahead(ahead);
				p[x++] = (gbm_u8) gbm_read_ahead(ahead);
				gbm_read_ahead(ahead); /* Discard alpha channel */
				if ( x == gbm->w*3 )
					{ x = 0; y++; p += stride; }
				}
		}
	break;
/*...e*/
		}
	gbm_destroy_ahead(ahead);
	}
	break;
/*...e*/
/*...sTGA_UNCOMP_BW\44\ TGA_UNCOMP_CM\44\ TGA_UNCOMP_RGB:16:*/
case TGA_UNCOMP_BW:
case TGA_UNCOMP_CM:
case TGA_UNCOMP_RGB:
	switch ( priv->header.bpp )
		{
/*...s8:32:*/
case 8:
	for ( i = 0; i < gbm->h; i++, p += stride )
		if ( gbm_file_read(fd, p, gbm->w) != gbm->w )
			return GBM_ERR_READ;
	break;
/*...e*/
/*...s16:32:*/
case 16:
	for ( i = 0; i < gbm->h; i++, p += stride )
		{
		if ( gbm_file_read(fd, p+gbm->w, gbm->w * 2) != gbm->w * 2 )
			return GBM_ERR_READ;
		t16_t24(p, p+gbm->w, gbm->w);
		}
	break;
/*...e*/
/*...s24:32:*/
case 24:
	for ( i = 0; i < gbm->h; i++, p += stride )
		if ( gbm_file_read(fd, p, gbm->w * 3) != gbm->w * 3 )
			return GBM_ERR_READ;
	break;
/*...e*/
/*...s32:32:*/
case 32:
	{
	gbm_u8 *linebuf;
	if ( (linebuf = malloc((size_t) (gbm->w * 4))) == NULL )
		return GBM_ERR_MEM;
	for ( i = 0; i < gbm->h; i++, p += stride )
		{
		if ( gbm_file_read(fd, linebuf, gbm->w * 4) != gbm->w * 4 )
			{
			free(linebuf);
			return GBM_ERR_READ;
			}
		t32_t24(p, linebuf, gbm->w);
		}
	free(linebuf);
	}
	break;
/*...e*/
		}
	break;
/*...e*/
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...stga_w:0:*/
#define	SW3(a,b,c)	((a)*4+(b)*2+(c))

GBM_ERR tga_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	TGA_HEADER tga_header;
	int i, stride, obpp;
	const gbm_u8 *p;
	gbm_boolean o16   = ( gbm_find_word(opt, "16"   ) != NULL );
	gbm_boolean o24   = ( gbm_find_word(opt, "24"   ) != NULL );
	gbm_boolean o32   = ( gbm_find_word(opt, "32"   ) != NULL );
	gbm_boolean yup   = ( gbm_find_word(opt, "yup"  ) != NULL );
	gbm_boolean ydown = ( gbm_find_word(opt, "ydown") != NULL );

	fn=fn; /* Suppress 'unref arg' compiler warning */

	if ( gbm->bpp == 24 )
		switch ( SW3(o16,o24,o32) )
			{
			case SW3(1,0,0):	obpp = 16;	break;
			case SW3(0,0,0):
			case SW3(0,1,0):	obpp = 24;	break;
			case SW3(0,0,1):	obpp = 32;	break;
			default:		return GBM_ERR_BAD_OPTION;
			}
	else
		{
		if ( o16 || o24 || o32 )
			return GBM_ERR_BAD_OPTION;
		obpp = 8;
		}

	if ( yup && ydown )
		return GBM_ERR_BAD_OPTION;

	tga_header.n_chars_in_id         = 0;
	tga_header.image_type            = ( obpp == 8 ) ? TGA_UNCOMP_CM : TGA_UNCOMP_RGB;
	tga_header.x_origin_low          = low_u8(0);
	tga_header.x_origin_high         = high_u8(0);
	tga_header.y_origin_low          = low_u8(0);
	tga_header.y_origin_high         = high_u8(0);
	tga_header.color_map_start_low   = low_u8(0);
	tga_header.color_map_start_high  = high_u8(0);
	if ( gbm->bpp == 8 )
		{
		tga_header.color_map_present     = (gbm_u8) 1;
		tga_header.color_map_length_low  = low_u8(0x100);
		tga_header.color_map_length_high = high_u8(0x100);
		tga_header.color_map_entry_bits  = 24;
		}
	else
		{
		tga_header.color_map_present     = (gbm_u8) 0;
		tga_header.color_map_length_low  = low_u8(0);
		tga_header.color_map_length_high = high_u8(0);
		tga_header.color_map_entry_bits  = 0;
		}
	tga_header.width_low             = low_u8(gbm->w);
	tga_header.width_high            = high_u8(gbm->w);
	tga_header.height_low            = low_u8(gbm->h);
	tga_header.height_high           = high_u8(gbm->h);
	tga_header.bpp                   = (gbm_u8) obpp;
	tga_header.image_descriptor      = IDB_NON_INT;

	if ( ydown )
		tga_header.image_descriptor |= IDB_ORIGIN;

	gbm_file_write(fd, (char *) &tga_header, SIZEOF_TGA_HEADER);

	switch ( obpp )
		{
/*...s8:16:*/
case 8:
	for ( i = 0; i < 0x100; i++ )
		{
		gbm_u8	b[3];
		b[0] = gbmrgb[i].b;
		b[1] = gbmrgb[i].g;
		b[2] = gbmrgb[i].r;
		gbm_file_write(fd, b, 3);
		}
	stride = ((gbm->w + 3) & ~3);
	p = data;
	if ( ydown )
		{
		p += ((gbm->h - 1) * stride);
		stride = -stride;
		}
	for ( i = 0; i < gbm->h; i++ )
		{
		gbm_file_write(fd, p, gbm->w);
		p += stride;
		}
	break;
/*...e*/
/*...s16:16:*/
case 16:
	{
	gbm_u8 *linebuf;
	if ( (linebuf = (gbm_u8*) malloc((size_t) (gbm->w * 2))) == NULL )
		return GBM_ERR_MEM;
	stride = ((gbm->w * 3 + 3) & ~3);
	p = data;
	if ( ydown )
		{
		p += ((gbm->h - 1) * stride);
		stride = -stride;
		}
	for ( i = 0; i < gbm->h; i++ )
		{
		t24_t16(linebuf, p, gbm->w);
		gbm_file_write(fd, linebuf, gbm->w * 2);
		p += stride;
		}
	free(linebuf);
	}
	break;
/*...e*/
/*...s24:16:*/
case 24:
	stride = ((gbm->w * 3 + 3) & ~3);
	p = data;
	if ( ydown )
		{
		p += ((gbm->h - 1) * stride);
		stride = -stride;
		}
	for ( i = 0; i < gbm->h; i++ )
		{
		gbm_file_write(fd, p, gbm->w * 3);
		p += stride;
		}
	break;
/*...e*/
/*...s32:16:*/
case 32:
	{
	gbm_u8	*linebuf;
	if ( (linebuf = (gbm_u8 *) malloc((size_t) (gbm->w * 4))) == NULL )
		return GBM_ERR_MEM;
	stride = ((gbm->w * 3 + 3) & ~3);
	p = data;
	if ( ydown )
		{
		p += ((gbm->h - 1) * stride);
		stride = -stride;
		}
	for ( i = 0; i < gbm->h; i++ )
		{
		t24_t32(linebuf, p, gbm->w);
		gbm_file_write(fd, linebuf, gbm->w * 4);
		p += stride;
		}
	free(linebuf);
	}
	break;
/*...e*/
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...stga_err:0:*/
const char *tga_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_TGA_BAD_BPP:
			return "bits per pixel not 8, 16, 24 or 32";
		case GBM_ERR_TGA_BAD_TYPE:
			return "unsupported compression type for bits per pixel";
		case GBM_ERR_TGA_BAD_PAL:
			return "color map entry size not 24 bits per pixel";
		}
	return NULL;
	}
/*...e*/
