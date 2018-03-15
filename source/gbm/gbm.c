/*

gbm.c - Generalised Bitmap Module

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#if defined(AIX) || defined(LINUX) || defined(SUN) || defined(MACOSX) || defined(IPHONE)
#include <unistd.h>
#else
#include <io.h>
#endif
#include <fcntl.h>
#ifdef MAC
#include <types.h>
#include <stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include "gbm.h"
#include "gbmhelp.h"

/*...vgbm\46\h:0:*/
/*...vgbmhelp\46\h:0:*/
/*...e*/

/*...sentrypoints:0:*/
#include "gbmpgm.h"
#include "gbmppm.h"
#include "gbmbmp.h"
#include "gbmtga.h"
#include "gbmkps.h"
#include "gbmiax.h"
#include "gbmpcx.h"
#include "gbmtif.h"
#include "gbmlbm.h"
#include "gbmvid.h"
#include "gbmgif.h"
#include "gbmxbm.h"
#include "gbmspr.h"
#include "gbmpsg.h"
#include "gbmgem.h"
#include "gbmcvp.h"
#include "gbmjpg.h"

/*...vgbmpgm\46\h:0:*/
/*...vgbmppm\46\h:0:*/
/*...vgbmbmp\46\h:0:*/
/*...vgbmtga\46\h:0:*/
/*...vgbmkps\46\h:0:*/
/*...vgbmiax\46\h:0:*/
/*...vgbmpcx\46\h:0:*/
/*...vgbmtif\46\h:0:*/
/*...vgbmlbm\46\h:0:*/
/*...vgbmvid\46\h:0:*/
/*...vgbmgif\46\h:0:*/
/*...vgbmxbm\46\h:0:*/
/*...vgbmspr\46\h:0:*/
/*...vgbmpsg\46\h:0:*/
/*...vgbmgem\46\h:0:*/
/*...vgbmcvp\46\h:0:*/
/*...vgbmjpg\46\h:0:*/

typedef struct
	{
	GBM_ERR      (*query_filetype)(GBMFT *gbmft);
	GBM_ERR      (*read_header )(const char *fn, int fd, GBM *gbm, const char *opt);
	GBM_ERR      (*read_palette)(int fd, GBM *gbm, GBMRGB *gbmrgb);
	GBM_ERR      (*read_data   )(int fd, GBM *gbm, gbm_u8 *data);
	GBM_ERR      (*write       )(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt);
	const char * (*err         )(GBM_ERR rc);
	} FT;

static FT fts[] =
	{
	bmp_qft, bmp_rhdr, bmp_rpal, bmp_rdata, bmp_w, bmp_err,
	gif_qft, gif_rhdr, gif_rpal, gif_rdata, gif_w, gif_err,
	pcx_qft, pcx_rhdr, pcx_rpal, pcx_rdata, pcx_w, pcx_err,
	tif_qft, tif_rhdr, tif_rpal, tif_rdata, tif_w, tif_err,
	tga_qft, tga_rhdr, tga_rpal, tga_rdata, tga_w, tga_err,
	lbm_qft, lbm_rhdr, lbm_rpal, lbm_rdata, lbm_w, lbm_err,
	vid_qft, vid_rhdr, vid_rpal, vid_rdata, vid_w, vid_err,
	pgm_qft, pgm_rhdr, pgm_rpal, pgm_rdata, pgm_w, pgm_err,
	ppm_qft, ppm_rhdr, ppm_rpal, ppm_rdata, ppm_w, ppm_err,
	kps_qft, kps_rhdr, kps_rpal, kps_rdata, kps_w, kps_err,
	iax_qft, iax_rhdr, iax_rpal, iax_rdata, iax_w, iax_err,
	xbm_qft, xbm_rhdr, xbm_rpal, xbm_rdata, xbm_w, xbm_err,
	spr_qft, spr_rhdr, spr_rpal, spr_rdata, spr_w, spr_err,
	psg_qft, psg_rhdr, psg_rpal, psg_rdata, psg_w, psg_err,
	gem_qft, gem_rhdr, gem_rpal, gem_rdata, gem_w, gem_err,
	cvp_qft, cvp_rhdr, cvp_rpal, cvp_rdata, cvp_w, cvp_err,
#ifdef IJG
	jpg_qft, jpg_rhdr, jpg_rpal, jpg_rdata, jpg_w, jpg_err,
#endif
	};

#define	N_FT (sizeof(fts) / sizeof(fts[0]))
/*...e*/
/*...sextension:0:*/
static const char *extension(const char *fn)
	{
	const char *dot, *slash;

	if ( (dot = strrchr(fn, '.')) == NULL )
		return NULL;

	if ( (slash = strpbrk(fn, "/\\")) == NULL )
		return dot + 1;

	return ( slash < dot ) ? dot + 1 : NULL;
	}
/*...e*/

/*...sgbm_init:0:*/
GBMEXPORT GBM_ERR GBMENTRY gbm_init(void)
	{
	return GBM_ERR_OK;
	}
/*...e*/
/*...sgbm_deinit:0:*/
GBMEXPORT GBM_ERR GBMENTRY gbm_deinit(void)
	{
	return GBM_ERR_OK;
	}
/*...e*/
/*...sgbm_io_\42\:0:*/
GBMEXPORT GBM_ERR GBMENTRY gbm_io_setup(
	int  (*open  )(const char *fn, int mode),
	int  (*create)(const char *fn, int mode),
	void (*close )(int fd),
	long (*lseek )(int fd, long pos, int whence),
	int  (*read  )(int fd, void *buf, int len),
	int  (*write )(int fd, const void *buf, int len)
	)
	{
	gbm_file_open   = open  ;
	gbm_file_create = create;
	gbm_file_close  = close ;
	gbm_file_lseek  = lseek ;
	gbm_file_read   = read  ;
	gbm_file_write  = write ;
	return GBM_ERR_OK;
	}

GBMEXPORT int  GBMENTRY gbm_io_open(const char *fn, int mode)
	{ return gbm_file_open(fn, mode); }
GBMEXPORT int  GBMENTRY gbm_io_create(const char *fn, int mode)
	{ return gbm_file_create(fn, mode); }
GBMEXPORT void GBMENTRY gbm_io_close(int fd)
	{ gbm_file_close(fd); }
GBMEXPORT long GBMENTRY gbm_io_lseek(int fd, long pos, int whence)
	{ return gbm_file_lseek(fd, pos, whence); }
GBMEXPORT int  GBMENTRY gbm_io_read (int fd, void *buf, int len)
	{ return gbm_file_read(fd, buf, len); }
GBMEXPORT int  GBMENTRY gbm_io_write(int fd, const void *buf, int len)
	{ return gbm_file_write(fd, buf, len); }
/*...e*/
/*...sgbm_query_n_filetypes:0:*/
GBMEXPORT GBM_ERR GBMENTRY gbm_query_n_filetypes(int *n_ft)
	{
	if ( n_ft == NULL )
		return GBM_ERR_BAD_ARG;
	*n_ft = N_FT;
	return GBM_ERR_OK;
	}
/*...e*/
/*...sgbm_guess_filetype:0:*/
GBMEXPORT GBM_ERR GBMENTRY gbm_guess_filetype(const char *fn, int *ft)
	{
	int i;
	const char *ext;

	if ( fn == NULL || ft == NULL )
		return GBM_ERR_BAD_ARG;

	if ( (ext = extension(fn)) == NULL )
		ext = "";

	for ( i = 0; i < N_FT; i++ )
		{
		GBMFT	gbmft;
		char	buf[100+1], *s;

		fts[i].query_filetype(&gbmft);
		for ( s  = strtok(strcpy(buf, gbmft.extensions), " \t,");
		      s != NULL;
		      s  = strtok(NULL, " \t,") )
			if ( gbm_same(s, ext, (int) strlen(ext) + 1) )
				{
				*ft = i;
				return GBM_ERR_OK;
				}
		}
	return GBM_ERR_NOT_FOUND;
	}
/*...e*/
/*...sgbm_query_filetype:0:*/
GBMEXPORT GBM_ERR GBMENTRY gbm_query_filetype(int ft, GBMFT *gbmft)
	{
	if ( gbmft == NULL )
		return GBM_ERR_BAD_ARG;
	return (*fts[ft].query_filetype)(gbmft);
	}
/*...e*/
/*...sgbm_read_header:0:*/
GBMEXPORT GBM_ERR GBMENTRY gbm_read_header(const char *fn, int fd, int ft, GBM *gbm, const char *opt)
	{
	if ( fn == NULL || opt == NULL || gbm == NULL )
		return GBM_ERR_BAD_ARG;
	gbm_file_lseek(fd, 0L, SEEK_SET);
	return (*fts[ft].read_header)(fn, fd, gbm, opt);
	}
/*...e*/
/*...sgbm_read_palette:0:*/
GBMEXPORT GBM_ERR GBMENTRY gbm_read_palette(int fd, int ft, GBM *gbm, GBMRGB *gbmrgb)
	{
	if ( gbm == NULL || gbmrgb == NULL )
		return GBM_ERR_BAD_ARG;
	return (*fts[ft].read_palette)(fd, gbm, gbmrgb);
	}
/*...e*/
/*...sgbm_read_data:0:*/
GBMEXPORT GBM_ERR GBMENTRY gbm_read_data(int fd, int ft, GBM *gbm, gbm_u8 *data)
	{
	if ( gbm == NULL || data == NULL )
		return GBM_ERR_BAD_ARG;
	return (*fts[ft].read_data)(fd, gbm, data);
	}
/*...e*/
/*...sgbm_write:0:*/
GBMEXPORT GBM_ERR GBMENTRY gbm_write(const char *fn, int fd, int ft, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	if ( fn == NULL || opt == NULL )
		return GBM_ERR_BAD_ARG;
	return (*fts[ft].write)(fn, fd, gbm, gbmrgb, data, opt);
	}
/*...e*/
/*...sgbm_err:0:*/
GBMEXPORT const char * GBMENTRY gbm_err(GBM_ERR rc)
	{
	int ft;

	switch ( (int) rc )
		{
		case GBM_ERR_OK:
			return "ok";
		case GBM_ERR_MEM:
			return "out of memory";
		case GBM_ERR_NOT_SUPP:
			return "not supported";
		case GBM_ERR_BAD_OPTION:
			return "bad option(s)";
		case GBM_ERR_NOT_FOUND:
			return "not found";
		case GBM_ERR_BAD_MAGIC:
			return "bad magic number / signiture block";
		case GBM_ERR_BAD_SIZE:
			return "bad bitmap size";
		case GBM_ERR_READ:
			return "can't read file";
		case GBM_ERR_WRITE:
			return "can't write file";
		case GBM_ERR_BAD_ARG:
			return "bad argument to gbm function";
		}

	for ( ft = 0; ft < N_FT; ft++ )
		{
		const char *s;
		if ( (s = (*fts[ft].err)(rc)) != NULL )
			return s;
		}

	return "general error";
	}
/*...e*/
/*...sgbm_version:0:*/
GBMEXPORT int GBMENTRY gbm_version(void)
	{
	return 113; /* 1.13 */
	}
/*...e*/

#ifdef OS2
/*...s_System entrypoints:0:*/
/* For GBM.DLL to be callable from IBM Smalltalk under OS/2, the entrypoints
   must be of _System calling convention. These veneers help out here.
   I can't just change the usual entrypoints because people depend on them. */

GBM_ERR _System Gbm_init(void)
	{ return gbm_init(); }

GBM_ERR _System Gbm_deinit(void)
	{ return gbm_deinit(); }

/* Gbm_io_setup omitted for now...
   Implies GBM.DLL must call out to non-_Optlink callback routines. */

int _System Gbm_io_open(const char *fn, int mode)
	{ return gbm_io_open(fn, mode); }

int _System Gbm_io_create(const char *fn, int mode)
	{ return gbm_io_create(fn, mode); }

void _System Gbm_io_close(int fd)
	{ gbm_io_close(fd); }

long _System Gbm_io_lseek(int fd, long pos, int whence)
	{ return gbm_file_lseek(fd, pos, whence); }

int _System Gbm_io_read (int fd, void *buf, int len)
	{ return gbm_file_read(fd, buf, len); }

int _System Gbm_io_write(int fd, const void *buf, int len)
	{ return gbm_file_write(fd, buf, len); }

GBM_ERR _System Gbm_query_n_filetypes(int *n_ft)
	{ return gbm_query_n_filetypes(n_ft); }

GBM_ERR _System Gbm_guess_filetype(const char *fn, int *ft)
	{ return gbm_guess_filetype(fn, ft); }

GBM_ERR	_System Gbm_query_filetype(int ft, GBMFT *gbmft)
	{ return gbm_query_filetype(ft, gbmft); }

GBM_ERR _System Gbm_read_header(const char *fn, int fd, int ft, GBM *gbm, const char *opt)
	{ return gbm_read_header(fn, fd, ft, gbm, opt); }

GBM_ERR _System Gbm_read_palette(int fd, int ft, GBM *gbm, GBMRGB *gbmrgb)
	{ return gbm_read_palette(fd, ft, gbm, gbmrgb); }

GBM_ERR _System Gbm_read_data(int fd, int ft, GBM *gbm, gbm_u8 *data)
	{ return gbm_read_data(fd, ft, gbm, data); }

GBM_ERR _System Gbm_write(const char *fn, int fd, int ft, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{ return gbm_write(fn, fd, ft, gbm, gbmrgb, data, opt); }

const char * _System Gbm_err(GBM_ERR rc)
	{ return gbm_err(rc); }

int _System Gbm_version(void)
	{ return gbm_version(); }
/*...e*/
#endif
