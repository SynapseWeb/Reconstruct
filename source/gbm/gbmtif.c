/*

gbmtif.c - Microsoft/Aldus Tagged Image File Format support

Reads and writes 1,4,8 and 24 bit colour files.
Supports uncompressed, Packbits and LZW compressed files only.
Input option: index=N (default is 0)
Output options: artist=,software=,make=,model=,host=,documentname=,pagename=,
		imagedescription=,pal1bpp

Added support to allow SamplePerPixel>=3 for RGB data (TIFF 6.0 new feature).
Added rejection test of FillOrder!=1.
Added rejection test of PlanarConfiguration!=1.
Added support for CMYK images.
Changed write of 1bpp data to Baseline black and white write.
Added pal1bpp output option meaning don't force Baseline write of 1bpp data.
Improved error messages substantially.
Fixed Packbits compression.
Added support for PlanarConfiguration==2 for RGB images only.
Added Predictor==2 support for bps==8, spp=1 case.
Added Predictor==2 support for bps==8, spp>=3 case.
Removed Too Many Strips limitation.
Faster LZW code.
Fixes to LZW compressor (and #ifdef DEBUG debug code).
Fixed some shorts to gbm_u16s.

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmtifh.h"

/*...vgbm\46\h:0:*/
/*...vgbmhelp\46\h:0:*/
/*...vgbmtifh\46\h:0:*/

#ifndef min
#define	min(a,b)	(((a)<(b))?(a):(b))
#endif
/*...e*/

/* #define DEBUG */

static GBMFT tif_gbmft =
	{
	"TIFF",
	"Tagged Image File Format support (almost TIFF 6.0 Baseline)",
	"TIF TIFF",
	GBM_FT_R1|GBM_FT_R4|GBM_FT_R8|GBM_FT_R24|
	GBM_FT_W1|GBM_FT_W4|GBM_FT_W8|GBM_FT_W24,
	};

/*...serror codes:0:*/
#define	GBM_ERR_TIF_VERSION		((GBM_ERR) 800)
#define	GBM_ERR_TIF_N_TAGS		((GBM_ERR) 801)
#define	GBM_ERR_TIF_TAG_TYPE		((GBM_ERR) 802)
#define	GBM_ERR_TIF_HEADER		((GBM_ERR) 803)
#define	GBM_ERR_TIF_MISSING_TAG		((GBM_ERR) 804)
#define	GBM_ERR_TIF_SPP_BIT		((GBM_ERR) 805)
#define	GBM_ERR_TIF_BPS_BIT		((GBM_ERR) 806)
#define	GBM_ERR_TIF_SPP_RGB		((GBM_ERR) 807)
#define	GBM_ERR_TIF_BPS_RGB		((GBM_ERR) 808)
#define	GBM_ERR_TIF_SPP_PAL		((GBM_ERR) 809)
#define	GBM_ERR_TIF_BPS_PAL		((GBM_ERR) 810)
#define	GBM_ERR_TIF_SPP_CMYK		((GBM_ERR) 811)
#define	GBM_ERR_TIF_BPS_CMYK		((GBM_ERR) 812)
#define	GBM_ERR_TIF_COMP_1D_MH		((GBM_ERR) 813)
#define	GBM_ERR_TIF_COMP_T4		((GBM_ERR) 814)
#define	GBM_ERR_TIF_COMP_T6		((GBM_ERR) 815)
#define	GBM_ERR_TIF_COMP		((GBM_ERR) 816)
#define	GBM_ERR_TIF_COLORMAP		((GBM_ERR) 817)
#define	GBM_ERR_TIF_CORRUPT		((GBM_ERR) 818)
#define	GBM_ERR_TIF_PREDICTOR		((GBM_ERR) 819)
#define	GBM_ERR_TIF_PHOTO_TRANS		((GBM_ERR) 821)
#define	GBM_ERR_TIF_PHOTO_Y_Cb_Cr	((GBM_ERR) 822)
#define	GBM_ERR_TIF_PHOTO		((GBM_ERR) 823)
#define	GBM_ERR_TIF_FILLORDER		((GBM_ERR) 824)
#define	GBM_ERR_TIF_PLANARCONFIG_1	((GBM_ERR) 825)
#define	GBM_ERR_TIF_PLANARCONFIG_12	((GBM_ERR) 826)
#define	GBM_ERR_TIF_INKSET		((GBM_ERR) 827)
#define	GBM_ERR_TIF_ORIENT		((GBM_ERR) 828)
#define	GBM_ERR_TIF_INDEX		((GBM_ERR) 829)
/*...e*/

/*...sdefns:0:*/
/*
#define MAX_STRIPS	200
*/
#define	MAX_STRIPS	((PRIV_SIZE-9*sizeof(int)-0x100*sizeof(GBMRGB))/sizeof(long))

typedef struct
	{
	GBMRGB gbmrgb[0x100];
	int rps, spp, bps, comp, photo, orient, planar, predictor, inx;
	long so[MAX_STRIPS];
	} TIF_PRIV;

#define	ENC_NONE	((int) 1)
#define	ENC_G3_1D_MH	((int) 2)
#define	ENC_T4		((int) 3)
#define	ENC_T6		((int) 4)
#define	ENC_LZW		((int) 5)
#define	ENC_PACKBITS	((int) 32773)

#define	PHOTO_BIT0	0
#define	PHOTO_BIT1	1
#define	PHOTO_RGB	2
#define	PHOTO_PAL	3
#define	PHOTO_TRANS	4
#define	PHOTO_CMYK	5
#define	PHOTO_Y_Cb_Cr	6

/* TIFF LZW definitions */

#define	INIT_CODE_SIZE	9
#define	CLEAR_CODE      0x100
#define	EOI_CODE        0x101
#define	FIRST_FREE_CODE 0x102
/*...e*/

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

typedef unsigned int cgbm_u16;

#ifdef DEBUG
#define LOG(args) printf args
#else
#define	LOG(args)
#endif

/*...stif_qft:0:*/
GBM_ERR tif_qft(GBMFT *gbmft)
	{
	*gbmft = tif_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...stif_rhdr:0:*/
/*...svalue_of_tag_def:0:*/
static long value_of_tag_def(IFD *ifd, gbm_u16 type, long def)
	{
	TAG *tag;
	if ( (tag = locate_tag(ifd, type)) != NULL )
		return value_of_tag(tag);
	else
		return def;
	}
/*...e*/

GBM_ERR tif_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	TIF_PRIV *tif_priv = (TIF_PRIV *) gbm->priv;
	TAG *tag_w, *tag_h, *tag_so;
	GBM_ERR rc;
	IFH *ifh;
	IFD *ifd;
	int inx = 0, strip, n_strips, fillorder;
	const char *index;

	if ( (index = gbm_find_word_prefix(opt, "index=")) != NULL )
		sscanf(index + 6, "%d", &inx);
	tif_priv->inx = inx;

	fn=fn; /* Suppress 'unref arg' compiler warnings */

	switch ( read_ifh_and_ifd(fd, inx, &ifh) )
		{
		case TE_OK:		rc = GBM_ERR_OK;		break;
		case TE_MEM:		rc = GBM_ERR_MEM;		break;
		case TE_VERSION:	rc = GBM_ERR_TIF_VERSION;	break;
		case TE_N_TAGS:		rc = GBM_ERR_TIF_N_TAGS;	break;
		case TE_TAG_TYPE:	rc = GBM_ERR_TIF_TAG_TYPE;	break;
		case TE_N_IFD:		rc = GBM_ERR_TIF_INDEX;		break;
		default:		rc = GBM_ERR_TIF_HEADER;	break;
		}

	if ( rc != GBM_ERR_OK )
		return rc;

	ifd = ifh->ifd;

	if ( (tag_w  = locate_tag(ifd, T_IMAGEWIDTH  )) == NULL ||
	     (tag_h  = locate_tag(ifd, T_IMAGELENGTH )) == NULL ||
	     (tag_so = locate_tag(ifd, T_STRIPOFFSETS)) == NULL )
		{
		free_ifh(ifh);
		return GBM_ERR_TIF_MISSING_TAG;
		}

	gbm->w           = (int) value_of_tag(tag_w);
	gbm->h           = (int) value_of_tag(tag_h);
	tif_priv->photo  = (int) value_of_tag_def(ifd, T_PHOTOMETRIC    , 1L);
	tif_priv->rps    = (int) value_of_tag_def(ifd, T_ROWSPERSTRIP   , (long) gbm->h);
	tif_priv->spp    = (int) value_of_tag_def(ifd, T_SAMPLESPERPIXEL, 1L);
	tif_priv->bps    = (int) value_of_tag_def(ifd, T_BITSPERSAMPLE  , 1L);
	tif_priv->comp   = (int) value_of_tag_def(ifd, T_COMPRESSION    , 1L);
	tif_priv->orient = (int) value_of_tag_def(ifd, T_ORIENTATION    , 1L);
	tif_priv->planar = (int) value_of_tag_def(ifd, T_PLANARCONFIG   , 1L);

	rc = GBM_ERR_OK;
	switch ( tif_priv->photo )
		{
/*...sPHOTO_BITx    \45\ bitmap or greyscale:16:*/
case PHOTO_BIT0:
case PHOTO_BIT1:
	if ( tif_priv->spp != 1 )
		rc = GBM_ERR_TIF_SPP_BIT;
	else if ( tif_priv->bps != 1 && tif_priv->bps != 4 && tif_priv->bps != 8 )
		rc = GBM_ERR_TIF_BPS_BIT;
	else
		{
		int	i, n_pal;

		n_pal = ( 1 << tif_priv->bps );
		for ( i = 0; i < n_pal; i++ )
			{
			tif_priv->gbmrgb[i].r =
			tif_priv->gbmrgb[i].g =
			tif_priv->gbmrgb[i].b = (gbm_u8) ((0xff * i) / (n_pal - 1));
			}
		if ( tif_priv->photo == PHOTO_BIT0 )
			for ( i = 0; i < n_pal; i++ )
				{
				tif_priv->gbmrgb[i].r ^= 0xff;
				tif_priv->gbmrgb[i].g ^= 0xff;
				tif_priv->gbmrgb[i].b ^= 0xff;
				}
		}
	gbm->bpp = tif_priv->bps;
	break;
/*...e*/
/*...sPHOTO_RGB     \45\ 24 bit data:16:*/
/* It is possible for sample per pixel to be greater than 3.
   This implies there are extra samples (which we will ignore).
   This is a new TIFF 6.0 feature. */

case PHOTO_RGB:
	if ( tif_priv->spp < 3 )
		rc = GBM_ERR_TIF_SPP_RGB;
	else if ( tif_priv->bps != 8 )
		rc = GBM_ERR_TIF_BPS_RGB;
	gbm->bpp = 24;
	break;
/*...e*/
/*...sPHOTO_PAL     \45\ palettised:16:*/
/*
There are 2 known bugs in commonly available TIFF files today.
UBU will only write a ColorMap tag with a length field of 256 (not 256 * 3).
This bug is fixed inside my TIFF library itself!
OS/2 Image Support will write all palette entrys as gbm_u8s!
*/

case PHOTO_PAL:
	if ( tif_priv->spp != 1 )
		rc = GBM_ERR_TIF_SPP_PAL;
	else if ( tif_priv->bps != 1 && tif_priv->bps != 4 && tif_priv->bps != 8 )
		rc = GBM_ERR_TIF_BPS_PAL;
	else
		{
		int	i, n_pal;
		TAG	*tag_cm;

		n_pal = (1 << tif_priv->bps);
		if ( (tag_cm = locate_tag(ifd, T_COLORMAP)) != NULL )
			{
			GBMRGB	*gbmrgb;

			for ( i = 0, gbmrgb = tif_priv->gbmrgb; i < n_pal; i++, gbmrgb++ )
				{
				gbmrgb->r = (gbm_u8) ((unsigned)value_of_tag_n(tag_cm,             i) >> 8);
				gbmrgb->g = (gbm_u8) ((unsigned)value_of_tag_n(tag_cm,     n_pal + i) >> 8);
				gbmrgb->b = (gbm_u8) ((unsigned)value_of_tag_n(tag_cm, 2 * n_pal + i) >> 8);
				}
/*...sfix for OS\47\2 Image Support \40\and others\41\:40:*/
{
gbm_u8 bugfix = 0;

for ( i = 0, gbmrgb = tif_priv->gbmrgb; i < n_pal; i++, gbmrgb++ )
	bugfix |= (gbmrgb->r | gbmrgb->g | gbmrgb->b);

if ( bugfix == 0 )
	for ( i = 0, gbmrgb = tif_priv->gbmrgb; i < n_pal; i++, gbmrgb++ )
		{
		gbmrgb->r = (gbm_u8) value_of_tag_n(tag_cm,             i);
		gbmrgb->g = (gbm_u8) value_of_tag_n(tag_cm,     n_pal + i);
		gbmrgb->b = (gbm_u8) value_of_tag_n(tag_cm, 2 * n_pal + i);
		}
}
/*...e*/
			}
		else
			rc = GBM_ERR_TIF_COLORMAP;
		}
	gbm->bpp = tif_priv->bps;
	break;
/*...e*/
/*...sPHOTO_TRANS   \45\ transparency mask:16:*/
case PHOTO_TRANS:
	rc = GBM_ERR_TIF_PHOTO_TRANS;
	break;
/*...e*/
/*...sPHOTO_CMYK    \45\ CMYK or other seperated image:16:*/
/* This is a colour seperated image.
   Typically expect 4 seperations, for CMYK.
   Can be other numbers, and possibly 4 with non standard ink colours.
   Ignore all but 4 seperations which are CMYK.
   Consider this a 24 bit RGB, mapping will occur from CMYK to RGB. */

case PHOTO_CMYK:
	if ( tif_priv->spp != 4 )
		rc = GBM_ERR_TIF_SPP_CMYK;
	else if ( tif_priv->bps != 8 )
		rc = GBM_ERR_TIF_BPS_CMYK;
	else if ( value_of_tag_def(ifd, T_INKSET, 1L) != 1 )
		rc = GBM_ERR_TIF_INKSET;
	else
		gbm->bpp = 24;
	break;
/*...e*/
/*...sPHOTO_Y_Cb_Cr \45\ Y\45\Cb\45\Cr colour space:16:*/
case PHOTO_Y_Cb_Cr:
	rc = GBM_ERR_TIF_PHOTO_Y_Cb_Cr;
	break;
/*...e*/
/*...sdefault       \45\ wierd PhotometricInterpretation:16:*/
default:
	rc = GBM_ERR_TIF_PHOTO;
	break;
/*...e*/
		}

	if ( rc != GBM_ERR_OK )
		{ free_ifh(ifh); return rc; }

	/* Remember where strips are, and how big they are */

	n_strips = (gbm->h + (tif_priv->rps - 1)) / tif_priv->rps;
	if ( tif_priv->photo == PHOTO_RGB && tif_priv->planar == 2 )
		n_strips *= 3;

	if ( n_strips <= MAX_STRIPS )
		for ( strip = 0; strip < n_strips; strip++ )
			tif_priv->so[strip] = value_of_tag_n(tag_so, strip);

	if ( tif_priv->comp != ENC_NONE     &&
	     tif_priv->comp != ENC_PACKBITS &&
	     tif_priv->comp != ENC_LZW      )
/*...sreject compression type:16:*/
{
free_ifh(ifh);
switch ( tif_priv->comp )
	{
	case ENC_G3_1D_MH:	return GBM_ERR_TIF_COMP_1D_MH;
	case ENC_T4:		return GBM_ERR_TIF_COMP_T4   ;
	case ENC_T6:		return GBM_ERR_TIF_COMP_T6   ;
	default:		return GBM_ERR_TIF_COMP      ;
	}
}
/*...e*/

	if ( tif_priv->orient != 1 && tif_priv->orient != 4 )
		{ free_ifh(ifh); return GBM_ERR_TIF_ORIENT; }

	fillorder = (int) value_of_tag_def(ifd, T_FILLORDER, 1L);
	if ( fillorder != 1 )
		{ free_ifh(ifh); return GBM_ERR_TIF_FILLORDER; }

	if ( tif_priv->photo == PHOTO_RGB )
		/* Allow photo of 1 or 2 */
		{
		if ( tif_priv->planar != 1 && tif_priv->planar != 2 )
			{ free_ifh(ifh); return GBM_ERR_TIF_PLANARCONFIG_12; }
		}
	else
		/* Allow photo of 1 only */
		{
		if ( tif_priv->planar != 1 )
			{ free_ifh(ifh); return GBM_ERR_TIF_PLANARCONFIG_1; }
		}

	tif_priv->predictor = (int) value_of_tag_def(ifd, T_PREDICTOR, 1L);

	/* Only allow predictor of 1, unless a special case we handle */
	if ( tif_priv->predictor != 1 &&
	     !(tif_priv->comp == ENC_LZW &&
	       tif_priv->bps == 8 &&
	       (tif_priv->spp == 1 || tif_priv->spp >= 3)) )
		{ free_ifh(ifh); return GBM_ERR_TIF_PREDICTOR; }

	free_ifh(ifh);

	return GBM_ERR_OK;
	}
/*...e*/
/*...stif_rpal:0:*/
GBM_ERR tif_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	TIF_PRIV *tif_priv = (TIF_PRIV *) gbm->priv;

	fd=fd; /* Suppress 'unref arg' compiler warning */

	if ( gbm->bpp != 24 )
		memcpy(gbmrgb, tif_priv->gbmrgb, (1 << gbm->bpp) * sizeof(GBMRGB));

	return GBM_ERR_OK;
	}
/*...e*/
/*...stif_rdata:0:*/
/*
TIFF data is usually stored top-left-origin based.
ie: We ignore the private "Orientation" tag.
Our data format in memory has bigger padding than TIFF.
It has 'istride' gbm_u8s per line, GBM uses/requires 'stride' gbm_u8s per line.
Therefore we read it to the part of the strip area and then expand downwards.
*/

/*...sget_strip_packbits \45\ get gbm_u8s with packbits decompression:0:*/
static GBM_ERR get_strip_packbits(int fd, gbm_u8 *dest, long n_gbm_u8s)
	{
	AHEAD *ahead;
	if ( (ahead = gbm_create_ahead(fd)) == NULL )
		return GBM_ERR_MEM;
	while ( n_gbm_u8s > 0 )
		{
		gbm_u8 b = (gbm_u8) gbm_read_ahead(ahead);
		if ( b < 0x80 )
			{
			unsigned int count = b + 1;
			do
				*dest++ = (gbm_u8) gbm_read_ahead(ahead);
			while ( --count > 0 );
			n_gbm_u8s -= (b + 1);
			}
		else if ( b > 0x80 )
			{
			unsigned int count = 0x101 - (unsigned int) b;
			gbm_u8 c = (gbm_u8) gbm_read_ahead(ahead);
			memset(dest, c, count);
			dest += count;
			n_gbm_u8s -= count;
			}
		}
	gbm_destroy_ahead(ahead);
	return GBM_ERR_OK;
	}
/*...e*/
/*...sget_strip_lzw      \45\ get gbm_u8s with TIFF style LZW decompression:0:*/
/*
This code runs on output of FotoTouch and some sample TIFFs from an afs
directory on the IBM IP-network.
*/

/*...sread context:0:*/
typedef struct
	{
	int fd;				/* File descriptor to read */
	int inx, size;			/* Index and size in bits */
	gbm_u8 buf[255+3];		/* Buffer holding bits */
	int code_size;			/* Number of bits to return at once */
	cgbm_u16 read_mask;		/* 2^code_size-1 */
	} READ_CONTEXT;

static cgbm_u16 read_code(READ_CONTEXT *c)
	{
	gbm_u32 raw_code; int byte_inx;

	while ( c->inx + c->code_size > c->size )
/*...snot enough bits in buffer\44\ refill it:16:*/
/* Not very efficient, but infrequently called */

{
int bytes_to_lose = ((unsigned)c->inx >> 3);
int bytes;

/* Note biggest code size is 12 bits */
/* And this can at worst span 3 bytes */
c->buf[0] = c->buf[bytes_to_lose  ];
c->buf[1] = c->buf[bytes_to_lose+1];
c->buf[2] = c->buf[bytes_to_lose+2];
(c->inx) &= 7;
(c->size) -= (bytes_to_lose << 3);
bytes = 255 - ( (unsigned)c->size >> 3 );
if ( (bytes = gbm_file_read(c->fd, c->buf + ((unsigned)c->size >> 3), bytes)) <= 0 )
	return 0xffff;
(c->size) += (bytes << 3);
}
/*...e*/

	byte_inx = ((unsigned)c->inx >> 3);
	raw_code = ((c->buf[byte_inx    ]) << 16) +
		   ((c->buf[byte_inx + 1]) <<  8) +
	           ( c->buf[byte_inx + 2]       ) ;
	raw_code <<= ((c->inx) & 7);
	(c->inx) += (gbm_u8) (c->code_size);
	raw_code >>= ( 24 - c->code_size );
	return (cgbm_u16) raw_code & c->read_mask;
	}
/*...e*/

static GBM_ERR get_strip_lzw(int fd, gbm_u8 *dest, long n_gbm_u8s)
	{
	cgbm_u16 max_code;		/* 1 << code_size */
	cgbm_u16 free_code;		/* Next available free code slot */
	int i, out_count = 0;
	cgbm_u16 code, cur_code, old_code, in_code, fin_char;
	cgbm_u16 *prefix, *suffix, *outcode;
	READ_CONTEXT c;
	gbm_boolean table_full = GBM_FALSE;
	gbm_u8 *limit = dest+n_gbm_u8s;

	if ( (prefix = (cgbm_u16 *) malloc((size_t) (4096 * sizeof(cgbm_u16)))) == NULL )
		return GBM_ERR_MEM;
	if ( (suffix = (cgbm_u16 *) malloc((size_t) (4096 * sizeof(cgbm_u16)))) == NULL )
		{
		free(prefix);
		return GBM_ERR_MEM;
		}
	if ( (outcode = (cgbm_u16 *) malloc((size_t) (4097 * sizeof(cgbm_u16)))) == NULL )
		{
		free(suffix);
		free(prefix);
		return GBM_ERR_MEM;
		}

	/* Initial read context */

	c.inx            = 0;
	c.size           = 0;
	c.fd             = fd;
	c.code_size      = INIT_CODE_SIZE;
	c.read_mask      = (cgbm_u16) ( (1 << INIT_CODE_SIZE) - 1 );

	/* 2^min_code size accounts for all colours in file */

	free_code = FIRST_FREE_CODE;
	max_code  = (cgbm_u16) ( 1 << INIT_CODE_SIZE );

	LOG(("STRIP\n"));
	while ( dest < limit && (code = read_code(&c)) != EOI_CODE && code != 0xffff )
		{
		if ( code == CLEAR_CODE )
			{
			LOG(("CLEARED\n"));
			c.code_size = INIT_CODE_SIZE;
			c.read_mask = (cgbm_u16) ( (1 << INIT_CODE_SIZE) - 1);
			max_code = (cgbm_u16) ( 1 << INIT_CODE_SIZE );
			free_code = FIRST_FREE_CODE;
			cur_code = old_code = code = read_code(&c);
			if ( code == EOI_CODE || code == 0xffff )
				break;
			fin_char = cur_code;
			if ( cur_code >= 0x100 )
				{
				free(outcode);
				free(suffix);
				free(prefix);
				LOG(("CORRUPT1\n"));
				return GBM_ERR_TIF_CORRUPT;
				}
			*dest++ = (gbm_u8) fin_char;
			LOG(("%02x ", (gbm_u8) fin_char));
			LOG(("%03x:%u\n", code, c.code_size));
			table_full = GBM_FALSE;
			}
		else
			{
			cur_code = in_code = code;
			if ( cur_code >= free_code )
				{
				cur_code = old_code;
				outcode[out_count++] = fin_char;
				}
			while ( cur_code > 0xff )
				{
				if ( out_count > 4096 )
					{
					free(outcode);
					free(suffix);
					free(prefix);
					LOG(("CORRUPT2\n"));
					return GBM_ERR_TIF_CORRUPT;
					}
				outcode[out_count++] = suffix[cur_code];
				cur_code = prefix[cur_code];
				}
			fin_char = cur_code;
			outcode[out_count++] = fin_char;
			for ( i = out_count - 1; i >= 0; i-- )
				{
				*dest++ = (gbm_u8) outcode[i];
				LOG(("%02x ", (gbm_u8) outcode[i]));
				}
			LOG(("%03x:%u\n", code, c.code_size));
			out_count = 0;

			/* Update dictionary */

			if ( !table_full )
				{
				prefix[free_code] = old_code;
				suffix[free_code] = fin_char;

				/* Advance to next free slot */

				if ( ++free_code >= max_code - 1 )
					{
					if ( c.code_size < 12 )
						{
						c.code_size++;
						max_code <<= 1;
						c.read_mask = (cgbm_u16) (( 1 << c.code_size ) - 1);
						}
					else
						{
						table_full = GBM_TRUE;
						LOG(("FULL\n"));
						}
					}
				}
			old_code = in_code;
			}
		}

	free(outcode);
	free(suffix);
	free(prefix);

	if ( dest < limit && code == 0xffff )
		return GBM_ERR_READ;

	return GBM_ERR_OK;
	}
/*...e*/
/*...sget_strip_lzw_pred \45\ get_strip_lzw with non 1 predictor fixup:0:*/
static GBM_ERR get_strip_lzw_pred(
	int fd,
	gbm_u8 *dest,
	long n_gbm_u8s,
	TIF_PRIV *tif_priv,
	int w, int h
	)
	{
	GBM_ERR rc;

	if ( (rc = get_strip_lzw(fd, dest, n_gbm_u8s)) != GBM_ERR_OK )
		return rc;

	if ( tif_priv->predictor == 2 )
		/* Note: This is only allowed if bps==8 */
		{
		int x, y, spp = tif_priv->spp;
		for ( y = 0; y < h; y++, dest += w * spp )
			for ( x = spp; x < w * spp; x++ )
				dest[x] += dest[x - spp];
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...sget_strip_comp     \45\ get strip\44\ dealing with any compression:0:*/
/* n_gbm_u8s is passed in as istride * n_lines */

static GBM_ERR get_strip_comp(
	int fd,
	gbm_u8 *dest,
	long so, long n_gbm_u8s,
	TIF_PRIV *tif_priv,
	int w, int h
	)
	{
	gbm_file_lseek(fd, so, SEEK_SET);
	switch ( tif_priv->comp )
		{
/*...sENC_NONE     \45\ no compression:16:*/
case ENC_NONE:
	return ( (int) n_gbm_u8s == gbm_file_read(fd, dest, (int) n_gbm_u8s) ) ?
			GBM_ERR_OK : GBM_ERR_READ;
/*...e*/
/*...sENC_PACKBITS \45\ packbits:16:*/
case ENC_PACKBITS:
	return get_strip_packbits(fd, dest, n_gbm_u8s);
/*...e*/
/*...sENC_LZW      \45\ lzw:16:*/
case ENC_LZW:
	return get_strip_lzw_pred(fd, dest, n_gbm_u8s, tif_priv, w, h);
/*...e*/
		}
	return GBM_ERR_NOT_SUPP;
	}
/*...e*/
/*...sget_strip          \45\ get strip\44\ discarding extra samples\44\ and CMYK mapping:0:*/
/*
If there are too many samples per pixel, this code can ignore the extra ones.
Also, if CMYK data is being read, it will read the CMYK, and convert.
This requires a temporary buffer, to read the original data in.
The original data is then 'converted'.
*/

static GBM_ERR get_strip(
	int fd,
	gbm_u8 *dest,
	long so, long n_gbm_u8s,
	TIF_PRIV *tif_priv,
	int w, int h
	)
	{
	gbm_u8 *buf = dest;
	GBM_ERR rc;

	if ( tif_priv->photo == PHOTO_CMYK )
/*...sallocate space for CMYK image:16:*/
{
n_gbm_u8s = (long) w * 4L * (long) h;
if ( (buf = malloc((size_t) n_gbm_u8s)) == NULL )
	return GBM_ERR_MEM;
}
/*...e*/
	else if ( tif_priv->photo == PHOTO_RGB && tif_priv->spp > 3 )
/*...sallocate space for image \43\ extra samples:16:*/
{
n_gbm_u8s = (long) w * tif_priv->spp * (long) h;
if ( (buf = malloc((size_t) n_gbm_u8s)) == NULL )
	return GBM_ERR_MEM;
}
/*...e*/

	if ( (rc = get_strip_comp(fd, buf, so, n_gbm_u8s, tif_priv, w, h)) != GBM_ERR_OK )
		{
		if ( buf != dest )
			free(buf);
		return rc;
		}

	if ( tif_priv->photo == PHOTO_CMYK )
/*...sconvert from CMYK to RGB:16:*/
{
int x, yy;
gbm_u8 *buf_p = buf, *dest_p = dest;
for ( yy = 0; yy < h; yy++ )
	for ( x = 0; x < w; x++ )
		{
		gbm_u16 c = *buf_p++;
		gbm_u16 m = *buf_p++;
		gbm_u16 y = *buf_p++;
		gbm_u16 k = *buf_p++;

		/* Exploit 8 bit modulo arithmetic by biasing by + 0x100 */

		gbm_u16 r = 0x1ff - (c + k);
		gbm_u16 g = 0x1ff - (m + k);
		gbm_u16 b = 0x1ff - (y + k);

		if ( r < 0x100 ) r = 0x100;
		if ( g < 0x100 ) g = 0x100;
		if ( b < 0x100 ) b = 0x100;

		*dest_p++ = (gbm_u8) r;
		*dest_p++ = (gbm_u8) g;
		*dest_p++ = (gbm_u8) b;
		}

free(buf);
}
/*...e*/
	else if ( tif_priv->photo == PHOTO_RGB && tif_priv->spp > 3 )
/*...sextract\44\ ignoring extra\45\samples:16:*/
{
int x, y, skip = tif_priv->spp - 2;
gbm_u8 *buf_p = buf, *dest_p = dest;
for ( y = 0; y < h; y++ )
	for ( x = 0; x < w; x++ )
		{
		*dest_p++ = *buf_p++;
		*dest_p++ = *buf_p++;
		*dest_p++ = *buf_p  ;
		buf_p += skip;
		}

free(buf);
}
/*...e*/

	return GBM_ERR_OK;
	}
/*...e*/
/*...sget_image          \45\ get all strips\44\ result is whole image:0:*/
/*
This routine calls get_strip() to get strips data one after another until it
has read the entire images worth of data. Of course, scan lines are aligned on
gbm_u8 boundaries, and the data is usually considered to be image top to bottom.
*/

static GBM_ERR get_image(
	int fd,
	gbm_u8 *dest,
	TIF_PRIV *tif_priv,
	long *so,
	GBM *gbm,
	int *strip
	)
	{
	GBM_ERR	rc;
	int y, istride = ((gbm->w * gbm->bpp + 7) / 8);

	for ( y = 0; y < gbm->h; y += tif_priv->rps, (*strip)++ )
		{
		int n_lines = min(tif_priv->rps, gbm->h - y);
		if ( (rc = get_strip(fd, dest + y * istride,
				     so[*strip],
				     (long) n_lines * (long) istride,
				     tif_priv,
				     gbm->w, n_lines)) != GBM_ERR_OK )
			return rc;
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...sget_image_planar   \45\ get all strips\44\ allowing for PlanarConfiguration:0:*/
/*
get_image() will assume the data is in PlanarConfiguration==1, ie: chunky
pixel mode. This is GBM_TRUE most of the time. But sometimes we will actually
allow PlanarConfiguration==2. In this case, use get_image() and then fix-up
the results.
*/

static GBM_ERR get_image_planar(
	int fd,
	gbm_u8 *dest,
	TIF_PRIV *tif_priv,
	long *so,
	GBM *gbm
	)
	{
	int strip = 0;

	if ( tif_priv->photo == PHOTO_RGB &&
	     tif_priv->planar == 2 )
/*...sread 3 seperate planes\44\ and combine them:16:*/
/*
If PhotometricInterpretation==RGB and
   SamplesPerPixel>=3 and
   BitsPerSample==8 then
	we allow PlanarConfiguration==2

I have successfully read in a PlanarConfiguration==2 RGB TIFF file by using
the "read 3 images" logic below. This image had RowsPerStrip==1, and so
technically either fold below would have worked. I think the read 3 images
logic is a better interpretation of the TIFF 6.0 spec., but until I find
some other images I can handle, I will keep the alternative peice of code.
*/

{
GBM_ERR rc;
GBM gbm_planar;
int saved_spp = tif_priv->spp;
int n_gbm_u8s = gbm->w * gbm->h;
int x, y;
gbm_u8 *buf, *p[3];

if ( (buf = malloc((size_t) (n_gbm_u8s * 3))) == NULL )
	return GBM_ERR_MEM;
p[0] = buf;
p[1] = p[0] + n_gbm_u8s;
p[2] = p[1] + n_gbm_u8s;

tif_priv->spp = 1;
/*...sread 3 images:16:*/
{
int i;

gbm_planar.w   = gbm->w;
gbm_planar.h   = gbm->h;
gbm_planar.bpp = 8;
for ( i = 0; i < 3; i++ )
	if ( (rc = get_image(fd, p[i], tif_priv, so, &gbm_planar, &strip)) != GBM_ERR_OK )
		{
		tif_priv->spp = saved_spp;
		free(buf);
		return rc;
		}
}
/*...e*/
#ifdef NEVER
/*...sread single image 3x too high:16:*/
gbm_planar.w   = gbm->w;
gbm_planar.h   = gbm->h * 3;
gbm_planar.bpp = 8;
if ( (rc = get_image(fd, buf, tif_priv, so, &gbm_planar, &strip)) != GBM_ERR_OK )
	{
	tif_priv->spp = saved_spp;
	free(buf);
	return rc;
	}
/*...e*/
#endif
tif_priv->spp = saved_spp;

for ( y = 0; y < gbm->h; y++ )
	for ( x = 0; x < gbm->w; x++ )
		{
		*dest++ = *(p[0])++;
		*dest++ = *(p[1])++;
		*dest++ = *(p[2])++;
		}
free(buf);
return GBM_ERR_OK;
}
/*...e*/
	else
		return get_image(fd, dest, tif_priv, so, gbm, &strip);
	}
/*...e*/
/*...sget_image_orient   \45\ get all strips\44\ correctly orientated:0:*/
static GBM_ERR get_image_orient(
	int fd,
	gbm_u8 *dest,
	TIF_PRIV *tif_priv,
	long *so,
	GBM *gbm
	)
	{
	switch ( tif_priv->orient )
		{
/*...s1 \45\ usual Baseline required case:16:*/
/*
File has array[scanlines_down] of array[pixels_across] of pixel.
GBMs bitmap data is array[scanlines_up] of array[pixels_across] of pixel.
So call get_image_planar(), and vertically reflect resulting data.
*/

case 1:
	{
	int istride = ((gbm->bpp * gbm->w + 7) / 8);
	gbm_u8 *p0, *p1, *p2;
	GBM_ERR rc;
	if ( (rc = get_image_planar(fd, dest, tif_priv, so, gbm)) != GBM_ERR_OK )
		return rc;
	if ( (p0 = malloc((size_t) istride)) == NULL )
		return GBM_ERR_MEM;
	for ( p1 = dest, p2 = p1 + (gbm->h - 1) * istride;
	      p1 < p2;
	      p1 += istride, p2 -= istride )
		{
		memcpy(p0, p1, istride);
		memcpy(p1, p2, istride);
		memcpy(p2, p0, istride);
		}
	free(p0);
	return GBM_ERR_OK;
	}
/*...e*/
/*...s4 \45\ vertically swapped case we can easily handle:16:*/
/*
File has array[scanlines_up] of array[pixels_across] of pixel.
Exactly matches GBMs layout of a bitmap.
So simply call get_image() and be done with.
*/

case 4:
	return get_image_planar(fd, dest, tif_priv, so, gbm);
/*...e*/
		}
	return GBM_ERR_NOT_SUPP; /* Shouldn't get here */
	}
/*...e*/
/*...sget_image_strippy  \45\ get all strips\44\ when there are loads of them:0:*/
static GBM_ERR get_image_strippy(
	int fd,
	gbm_u8 *dest,
	TIF_PRIV *tif_priv,
	GBM *gbm
	)
	{
	int n_strips = (gbm->h + (tif_priv->rps - 1)) / tif_priv->rps;
	long *so = tif_priv->so;

	if ( n_strips > MAX_STRIPS )
/*...sre\45\read TIFF file header:16:*/
{
GBM_ERR rc;
int strip;
IFH *ifh;
IFD *ifd;
TAG *tag_so;

if ( (so = malloc((size_t) (n_strips * sizeof(long)))) == NULL )
	return GBM_ERR_MEM;

gbm_file_lseek(fd, 0L, SEEK_SET);
if ( read_ifh_and_ifd(fd, tif_priv->inx, &ifh) != TE_OK )
	{
	free(so);
	return GBM_ERR_MEM;
	}
ifd = ifh->ifd;
tag_so = locate_tag(ifd, T_STRIPOFFSETS);
for ( strip = 0; strip < n_strips; strip++ )
	so[strip] = value_of_tag_n(tag_so, strip);
free_ifh(ifh);

rc = get_image_orient(fd, dest, tif_priv, so, gbm);

free(so);
return rc;
}
/*...e*/
	else
		return get_image_orient(fd, dest, tif_priv, so, gbm);
	}
/*...e*/

GBM_ERR tif_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	TIF_PRIV *tif_priv = (TIF_PRIV *) gbm->priv;
	int stride = ((gbm->bpp * gbm->w + 31) / 32) * 4;
	int istride = ((gbm->bpp * gbm->w + 7) / 8);
	int bias = gbm->h * (stride - istride);
	GBM_ERR	rc;

	/* Read in data, packed close, and upside down */

	if ( (rc = get_image_strippy(fd, data + bias, tif_priv, gbm)) != GBM_ERR_OK )
		return rc;

/*...snow expand out from byte padding to dword padding:8:*/
if ( bias )
	{
	int y;
	gbm_u8 *dest = data, *src = data + bias;

	for ( y = 0; y < gbm->h; y++, dest += stride, src += istride )
		memmove(dest, src, istride);
	}
/*...e*/
/*...snow RGB\45\\62\BGR if 24 bit data returned:8:*/
if ( gbm->bpp == 24 )
	{
	int y;
	gbm_u8 *p = data;

	for ( y = 0; y < gbm->h; y++, p += stride )
		rgb_bgr(p, p, gbm->w);
	}
/*...e*/

	return GBM_ERR_OK;
	}
/*...e*/
/*...stif_w:0:*/
/*
Write out data in a single large strip for now.
Note that the palette entrys are written as ((r << 8) | r).
This means they are 257/256 too big (insignificant).
Most programs only look at the top 8 bits (ie: no error).
A few (incorrectly) look at the bottom 8 bits.
Therefore we cater for all programs, with minimal fuss.
*/

/*...suser_tag:0:*/
static gbm_boolean user_tag(IFD *ifd, char *name, gbm_u16 type, const char *opt, char *def)
	{
	char buf[200+1];
	const char *s;

	if ( (s = gbm_find_word_prefix(opt, name)) != NULL )
		sscanf(s + strlen(name), "%s", buf);
	else
		strcpy(buf, def);

	if ( *buf == '\0' )
		return GBM_TRUE;

	return add_ascii_tag(ifd, type, buf);
	}
/*...e*/
/*...swrite_strip:0:*/
static GBM_ERR write_strip(int fd, int w, int h, int bpp, const gbm_u8 *data)
	{
	int stride = ((bpp * w + 31) / 32) * 4;
	int ostride = ((bpp * w + 7) / 8);
	int y;
	data += ((h - 1) * stride);
	if ( bpp == 24 )
/*...sreverse rgb\47\bgr ordering and write:16:*/
{
gbm_u8 *line;

if ( (line = malloc((size_t) ostride)) == NULL )
	return GBM_ERR_MEM;
for ( y = 0; y < h; y++, data -= stride )
	{
	rgb_bgr(data, line, w);
	if ( gbm_file_write(fd, line, ostride) != ostride )
		{
		free(line);
		return GBM_ERR_WRITE;
		}
	}
free(line);
}
/*...e*/
	else
/*...swrite:16:*/
for ( y = 0; y < h; y++, data -= stride )
	if ( gbm_file_write(fd, data, ostride) != ostride )
		return GBM_ERR_WRITE;
/*...e*/
	return GBM_ERR_OK;
	}
/*...e*/
/*...swrite_strip_lzw \45\ new fast tail\43\col lookup version:0:*/
/*
This is a tricky bit of code to get right.
This code blantantly copied and hacked from that in gbmgif.c!
hashvalue is calculated from a string of pixels cumulatively.
hashtable is searched starting at index hashvalue for to find the entry.
hashtable is big enough so that MAX_HASH > 4*MAX_DICT.
*/

/*...swrite context:0:*/
#define	L_BUF 1024

typedef struct
	{
	int fd;				/* Open file descriptor to write to */
	int inx;			/* Bit index into buf */
	int code_size;			/* Code size in bits */
	gbm_u8 buf[L_BUF+2];		/* Biggest block + overflow space */
	} WRITE_CONTEXT;

static gbm_boolean write_code(cgbm_u16 code, WRITE_CONTEXT *w)
	{
	gbm_u8 *buf = w->buf + ((unsigned)w->inx >> 3);

	LOG(("%03x:%u\n", code, w->code_size));
	code <<= (24-w->code_size);
	code >>= (w->inx&7U);
	*buf++ |= (gbm_u8) (code >> 16);
	*buf++  = (gbm_u8) (code >>  8);
	*buf    = (gbm_u8)  code       ;

	(w->inx) += (w->code_size);
	if ( w->inx >= L_BUF * 8 )
		/* Flush out full buffer */
		{
		if ( gbm_file_write(w->fd, w->buf, L_BUF) != L_BUF )
			return GBM_FALSE;
		memcpy(w->buf, w->buf + L_BUF, 2);
		memset(w->buf + 2, 0, L_BUF);
		(w->inx) -= (L_BUF * 8);
		}

	return GBM_TRUE;
	}

static gbm_boolean flush_code(WRITE_CONTEXT *w)
	{
	int gbm_u8s = (((unsigned)w->inx + 7) >> 3);

	if ( gbm_u8s )
		{
		if ( gbm_file_write(w->fd, w->buf, gbm_u8s) != gbm_u8s )
			return GBM_FALSE;
		}

	return GBM_TRUE;
	}
/*...e*/

#define	MAX_HASH	17777		/* Probably prime, and > 4096        */
#define	MAX_DICT	4096		/* Dictionary size                   */
#define	INIT_HASH(p)	(((p)+3)*301)	/* Initial hash value                */

typedef struct { cgbm_u16 tail; gbm_u8 col; } DICT;

static GBM_ERR write_strip_lzw(int fd, int w, int h, int bpp, const gbm_u8 *data)
	{
	int stride = ((bpp * w + 31) / 32) * 4;
	int ostride = ((bpp * w + 7) / 8);
	WRITE_CONTEXT wc;
	cgbm_u16 last_code, max_code, tail;
	int x, y;
	unsigned int hashvalue, lenstring, j;
	DICT *dict, **hashtable;

	if ( (dict = (DICT *) malloc((size_t) (MAX_DICT * sizeof(DICT)))) == NULL )
		return GBM_ERR_MEM;

	if ( (hashtable = (DICT **) malloc((size_t) (MAX_HASH * sizeof(DICT *)))) == NULL )
		{
		free(dict);
		return GBM_ERR_MEM;
		}

	/* Setup write context */

	wc.fd        = fd;
	wc.inx       = 0;
	wc.code_size = INIT_CODE_SIZE;
	memset(wc.buf, 0, sizeof(wc.buf));

	if ( !write_code(CLEAR_CODE, &wc) )
		{
		free(hashtable);
		free(dict);
		return GBM_ERR_WRITE;
		}

	last_code = EOI_CODE;
	max_code  = ( 1 << INIT_CODE_SIZE );
	lenstring = 0;

	for ( j = 0; j < MAX_HASH; j++ )
		hashtable[j] = NULL;

	data += ( (h - 1) * stride );
	for ( y = h - 1; y >= 0; y--, data -= stride )
		{
		int inx1 = 0, inx2 = 2;
		for ( x = 0; x < ostride; x++ )
			{
			gbm_u8 col;
/*...sget gbm_u8 to write to col:24:*/
if ( bpp == 24 )
	/* Have to handle rgb/bgr reverse as we go along */
	{
	col = data[inx1+inx2];
	if ( --inx2 < 0 )
		{
		inx1 += 3; inx2 = 2;
		}
	}
else
	col = data[x];
/*...e*/
/*...sLZW encode:24:*/
if ( ++lenstring == 1 )
	{
	tail      = col;
	hashvalue = INIT_HASH(col);
	}
else
	{
	hashvalue *= ( col + lenstring + 4 );
	j = ( hashvalue %= MAX_HASH );
	while ( hashtable[j] != NULL &&
		( hashtable[j]->tail != tail ||
		  hashtable[j]->col  != col  ) )
		if ( ++j >= MAX_HASH )
			j = 0;
	if ( hashtable[j] != NULL )
		/* Found in the strings table */
		{
		tail = (hashtable[j]-dict);
		LOG(("%02x ", col));
		}
	else
		/* Not found */
		{
		if ( !write_code(tail, &wc) )
			{
			free(hashtable);
			free(dict);
			return GBM_ERR_WRITE;
			}
		hashtable[j]       = dict + ++last_code;
		hashtable[j]->tail = tail;
		hashtable[j]->col  = col;
		LOG(("%02x ", col));
		tail               = col;
		hashvalue          = INIT_HASH(col);
		lenstring          = 1;

/* Note: Things change 1 earlier than in the GIF LZW case, hence -1. */
		if ( last_code >= max_code -1 )
			/* Next code will be written longer */
			{
			max_code <<= 1;
			wc.code_size++;
			}
		else if ( last_code >= MAX_DICT-2 )
			/* Reset tables */
			{
			if ( !write_code(tail      , &wc) ||
			     !write_code(CLEAR_CODE, &wc) )
				{
				free(hashtable);
				free(dict);
				return GBM_ERR_WRITE;
				}
			lenstring    = 0;
			last_code    = EOI_CODE;
			wc.code_size = INIT_CODE_SIZE;
			max_code     = ( 1 << INIT_CODE_SIZE );
			for ( j = 0; j < MAX_HASH; j++ )
				hashtable[j] = NULL;
			}
		}
	}
/*...e*/
			}
		}

	free(hashtable);
	free(dict);

	if ( lenstring != 0 )
		/* Only write tail if no code written for some input */
		{
		if ( !write_code(tail, &wc) )
			return GBM_ERR_WRITE;
		if ( ++last_code >= max_code -1 )
			/* Next code will be written longer */
			wc.code_size++;
		}

	if ( !write_code(EOI_CODE, &wc) ||
	     !flush_code(          &wc) )
		return GBM_ERR_WRITE;

	return GBM_ERR_OK;
	}
/*...e*/

GBM_ERR tif_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	gbm_boolean baseline = ( gbm_find_word(opt, "pal1bpp") == NULL );
	gbm_boolean lzw      = ( gbm_find_word(opt, "lzw"    ) != NULL );
	IFH	*ifh;
	IFD	*ifd;
	gbm_s32 w = gbm->w;
	gbm_s32 h = gbm->h;
	gbm_s32	stripoffset, stripbytecount;
	gbm_u16	samplesperpixel, bitspersample[3], photo, comp;
	gbm_u16	colormap[0x100+0x100+0x100];
	gbm_boolean ok;
	GBM_ERR	rc;

	fn=fn; /* Suppress 'unref arg' compiler warning */

	if ( (ifh = make_ifh()) == NULL )
		return GBM_ERR_MEM;

	ifd = ifh->ifd;

	if ( gbm->bpp == 1 && baseline )
		{
		gbm_u16 k0 = (gbm_u16) gbmrgb[0].r + (gbm_u16) gbmrgb[0].g + (gbm_u16) gbmrgb[0].b;
		gbm_u16 k1 = (gbm_u16) gbmrgb[1].r + (gbm_u16) gbmrgb[1].g + (gbm_u16) gbmrgb[1].b;
		samplesperpixel  = 1;
		bitspersample[0] = 1;
		photo = ( k0 < k1 ) ? PHOTO_BIT1 : PHOTO_BIT0; /* Black is zero : White is zero */
		}
	else if ( gbm->bpp == 24 )
		{
		samplesperpixel  = 3;
		bitspersample[0] = 
		bitspersample[1] = 
		bitspersample[2] = 8;
		photo = PHOTO_RGB;
		}
	else
		{
		samplesperpixel  = 1;
		bitspersample[0] = (gbm_u16) gbm->bpp;
		photo = PHOTO_PAL;
		}

	comp = ( lzw ) ? ENC_LZW : ENC_NONE;

	ok = add_long_tag(ifd, T_IMAGEWIDTH, (gbm_u32 *) &w, 1) &&
	     add_long_tag(ifd, T_IMAGELENGTH, (gbm_u32 *) &h, 1) &&
	     add_long_tag(ifd, T_STRIPOFFSETS, (gbm_u32 *) &stripoffset, 1) &&
	     add_long_tag(ifd, T_STRIPBYTECOUNTS, (gbm_u32 *) &stripbytecount, 1) &&
	     add_short_tag(ifd, T_SAMPLESPERPIXEL, (gbm_u16 *) &samplesperpixel, 1) &&
	     add_short_tag(ifd, T_BITSPERSAMPLE, (gbm_u16 *) bitspersample, samplesperpixel) &&
	     add_short_tag(ifd, T_PHOTOMETRIC, (gbm_u16 *) &photo, 1) &&
	     add_short_tag(ifd, T_COMPRESSION, (gbm_u16 *) &comp, 1) &&
	     user_tag(ifd, "artist=", T_ARTIST, opt, "") &&
	     user_tag(ifd, "software=", T_MAKE, opt, "") &&
	     user_tag(ifd, "make=", T_MAKE, opt, "") &&
	     user_tag(ifd, "model=", T_MODEL, opt, "") &&
	     user_tag(ifd, "hostcomputer=", T_HOSTCOMPUTER, opt, "") &&
	     user_tag(ifd, "documentname=", T_DOCNAME, opt, "") &&
	     user_tag(ifd, "pagename=", T_PAGENAME, opt, "") &&
	     user_tag(ifd, "imagedescription=", T_DESCRIPTION, opt, "");

	if ( gbm->bpp != 24 )
		{
		int i, n_cols = (1 << gbm->bpp);

		for ( i = 0; i < n_cols; i++ )
			{
			gbm_u16 r = (gbm_u16) gbmrgb[i].r;
			gbm_u16 g = (gbm_u16) gbmrgb[i].g;
			gbm_u16 b = (gbm_u16) gbmrgb[i].b;
			colormap[             i] = ((r << 8) | r);
			colormap[    n_cols + i] = ((g << 8) | g);
			colormap[2 * n_cols + i] = ((b << 8) | b);
			}
		if ( gbm->bpp != 1 || !baseline )
			ok &= add_short_tag(ifd, T_COLORMAP, colormap, n_cols * 3);
		}

	if ( !ok )
		{
		free_ifh(ifh);
		return GBM_ERR_MEM;
		}

	if ( !write_ifh_and_ifd(ifh, fd) )
		{
		free_ifh(ifh);
		return GBM_ERR_WRITE;
		}

	stripoffset = gbm_file_lseek(fd, 0L, SEEK_CUR);

	if ( lzw )
		rc = write_strip_lzw(fd, gbm->w, gbm->h, gbm->bpp, data);
	else
		rc = write_strip(fd, gbm->w, gbm->h, gbm->bpp, data);
	
	if ( rc != GBM_ERR_OK )
		{
		free_ifh(ifh);
		return rc;
		}

	stripbytecount = gbm_file_lseek(fd, 0L, SEEK_CUR) - stripoffset;

	update_long_tag(ifd, T_STRIPOFFSETS, (gbm_u32 *) &stripoffset);
	update_long_tag(ifd, T_STRIPBYTECOUNTS, (gbm_u32 *) &stripbytecount);

	if ( !update_ifd(ifd, fd) )
		{
		free_ifh(ifh);
		return GBM_ERR_WRITE;
		}

	free_ifh(ifh);

	return GBM_ERR_OK;
	}
/*...e*/
/*...stif_err:0:*/
const char *tif_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_TIF_VERSION:
			return "version number not 42";
		case GBM_ERR_TIF_N_TAGS:
			return "too many tags in file";
		case GBM_ERR_TIF_TAG_TYPE:
			return "bad tag type";
		case GBM_ERR_TIF_HEADER:
			return "corrupt header";
		case GBM_ERR_TIF_MISSING_TAG:
			return "ImageWidth, ImageLength or StripOffsets tag missing";
		case GBM_ERR_TIF_SPP_BIT:
			return "SamplesPerPixel tag must be 1 for bitmap or greyscale file";
		case GBM_ERR_TIF_BPS_BIT:
			return "BitsPerSample tag must be 1,4 or 8 for bitmap or greyscale file";
		case GBM_ERR_TIF_SPP_RGB:
			return "SamplesPerPixel tag must be 3 or more for RGB file";
		case GBM_ERR_TIF_BPS_RGB:
			return "BitsPerSample tag must be 8 for RGB file";
		case GBM_ERR_TIF_SPP_PAL:
			return "SamplesPerPixel tag must be 1 for palettised file";
		case GBM_ERR_TIF_BPS_PAL:
			return "BitsPerSample tag must be 1,4 or 8 for palettised file";
		case GBM_ERR_TIF_SPP_CMYK:
			return "SamplesPerPixel tag must be 4 for CMYK file";
		case GBM_ERR_TIF_BPS_CMYK:
			return "BitsPerSample tag must be 8 for CMYK file";
		case GBM_ERR_TIF_COMP_1D_MH:
			return "Compression tag is CCITT 1D Modified Huffman, not supported";
		case GBM_ERR_TIF_COMP_T4:
			return "Compression tag is CCITT T.4 G3 Facsimile, not supported";
		case GBM_ERR_TIF_COMP_T6:
			return "Compression tag is CCITT T.6 G4 Facsimile, not supported";
		case GBM_ERR_TIF_COMP:
			return "Compression tag not uncompressed, PackBits or LZW, not supported";
		case GBM_ERR_TIF_COLORMAP:
			return "ColorMap tag missing";
		case GBM_ERR_TIF_CORRUPT:
			return "encoded data is corrupt";
		case GBM_ERR_TIF_PREDICTOR:
			return "Predictor tag bad";
		case GBM_ERR_TIF_PHOTO_TRANS:
			return "PhotometricInterpretation tag is transparency mask, not supported";
		case GBM_ERR_TIF_PHOTO_Y_Cb_Cr:
			return "PhotometricInterpreation tag is Y-Cb-Cr colour space, not supported";
		case GBM_ERR_TIF_PHOTO:
			return "PhotometricInterpretation tag unsupported/bad";
		case GBM_ERR_TIF_FILLORDER:
			return "FillOrder tag must be 1";
		case GBM_ERR_TIF_PLANARCONFIG_1:
			return "PlanarConfiguration tag must be 1 for non RGB files";
		case GBM_ERR_TIF_PLANARCONFIG_12:
			return "PlanarConfiguration tag must be 1 or 2 for RGB files";
		case GBM_ERR_TIF_INKSET:
			return "InkSet tag indicates non-CMYK colour seperations";
		case GBM_ERR_TIF_ORIENT:
			return "Orientation tag must be 1 or 4";
		case GBM_ERR_TIF_INDEX:
			return "less bitmaps in file than index requested";
		}
	return NULL;
	}
/*...e*/
