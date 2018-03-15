/*

gbmsub.c - Subrectangle of General Bitmap

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
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
#ifndef O_BINARY
#define	O_BINARY	0
#endif
#include "gbm.h"
#include "gbmrect.h"

/*...vgbm\46\h:0:*/
/*...vgbmrect\46\h:0:*/
/*...e*/

static char progname[] = "gbmsub";

/*...sfatal:0:*/
static void fatal(const char *fmt, ...)
	{
	va_list	vars;
	char s[256+1];

	va_start(vars, fmt);
	vsprintf(s, fmt, vars);
	va_end(vars);
	fprintf(stderr, "%s: %s\n", progname, s);
	exit(1);
	}
/*...e*/
/*...susage:0:*/
static void usage(void)
	{
	int ft, n_ft;

	fprintf(stderr, "usage: %s [-x x] [-y y] [-w w] [-h h] [--] fn1.ext{,opt} [fn2.ext{,opt}]\n", progname);
	fprintf(stderr, "flags: -x x           left edge of rectangle (default 0)\n");
	fprintf(stderr, "       -y y           bottom edge of rectangle (default 0)\n");
	fprintf(stderr, "       -w w           width of rectangle (default width of image - x)\n");
	fprintf(stderr, "       -h h           height of rectangle (default height of image - y)\n");
	fprintf(stderr, "       fn1.ext{,opt}  input filename (with any format specific options)\n");
	fprintf(stderr, "       fn2.ext{,opt}  optional output filename (or will use fn1 if not present)\n");
	fprintf(stderr, "                      ext's are used to deduce desired bitmap file formats\n");

	gbm_init();
	gbm_query_n_filetypes(&n_ft);
	for ( ft = 0; ft < n_ft; ft++ )
		{
		GBMFT gbmft;

		gbm_query_filetype(ft, &gbmft);
		fprintf(stderr, "                      %s when ext in [%s]\n",
			gbmft.short_name, gbmft.extensions);
		}
	gbm_deinit();

	fprintf(stderr, "       opt's          bitmap format specific options\n");

	exit(1);
	}
/*...e*/
/*...sget_opt_value:0:*/
static int get_opt_value(const char *s, const char *name)
	{
	int v;

	if ( s == NULL )
		fatal("missing %s argument", name);
	if ( !isdigit(s[0]) )
		fatal("bad %s argument", name);
	if ( s[0] == '0' && tolower(s[1]) == 'x' )
		sscanf(s + 2, "%x", &v);
	else
		v = atoi(s);

	return v;
	}
/*...e*/
/*...sget_opt_value_pos:0:*/
static int get_opt_value_pos(const char *s, const char *name)
	{
	int n;

	if ( (n = get_opt_value(s, name)) < 0 )
		fatal("%s should not be negative", name);
	return n;
	}
/*...e*/
/*...smain:0:*/
int main(int argc, char *argv[])
	{
	int	x = 0, y = 0, w = -1, h = -1;
	char	fn_src[500+1], fn_dst[500+1], *opt_src, *opt_dst;
	int	fd, ft_src, ft_dst, i, stride_src, flag, bytes;
	GBM_ERR	rc;
	GBMFT	gbmft;
	GBM	gbm;
	GBMRGB	gbmrgb[0x100];
	gbm_u8	*data;

	for ( i = 1; i < argc; i++ )
		{
		if ( argv[i][0] != '-' )
			break;
		else if ( argv[i][1] == '-' )
			{ ++i; break; }
		switch ( argv[i][1] )
			{
			case 'x':
				if ( ++i == argc ) usage();
				x = get_opt_value_pos(argv[i], "x");
				break;
			case 'y':
				if ( ++i == argc ) usage();
				y = get_opt_value_pos(argv[i], "y");
				break;
			case 'w':
				if ( ++i == argc ) usage();
				w = get_opt_value_pos(argv[i], "w");
				break;
			case 'h':
				if ( ++i == argc ) usage();
				h = get_opt_value_pos(argv[i], "h");
				break;
			default:
				usage();
				break;
			}
		}

	if ( i == argc )
		usage();
	strcpy(fn_src, argv[i++]);
	strcpy(fn_dst, ( i == argc ) ? fn_src : argv[i++]);
	if ( i < argc )
		usage();

	if ( (opt_src = strchr(fn_src, ',')) != NULL )
		*opt_src++ = '\0';
	else
		opt_src = "";

	if ( (opt_dst = strchr(fn_dst, ',')) != NULL )
		*opt_dst++ = '\0';
	else
		opt_dst = "";

	gbm_init();

	if ( gbm_guess_filetype(fn_src, &ft_src) != GBM_ERR_OK )
		fatal("can't guess bitmap file format for %s", fn_src);

	if ( gbm_guess_filetype(fn_dst, &ft_dst) != GBM_ERR_OK )
		fatal("can't guess bitmap file format for %s", fn_dst);

	if ( (fd = gbm_io_open(fn_src, O_RDONLY|O_BINARY)) == -1 )
		fatal("can't open %s", fn_src);

	if ( (rc = gbm_read_header(fn_src, fd, ft_src, &gbm, opt_src)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read header of %s: %s", fn_src, gbm_err(rc));
		}

	if ( w == -1 )
		w = gbm.w - x;

	if ( h == -1 )
		h = gbm.h - y;

	if ( w     == 0     ) { gbm_io_close(fd); fatal("w = 0"); }
	if ( h     == 0     ) { gbm_io_close(fd); fatal("h = 0"); }
	if ( x     >= gbm.w ) { gbm_io_close(fd); fatal("x >= bitmap width"); }
	if ( y     >= gbm.h ) { gbm_io_close(fd); fatal("y >= bitmap height"); }
	if ( x + w >  gbm.w ) { gbm_io_close(fd); fatal("x+w > bitmap width"); }
	if ( y + h >  gbm.h ) { gbm_io_close(fd); fatal("y+h > bitmap height"); }

	gbm_query_filetype(ft_dst, &gbmft);
	switch ( gbm.bpp )
		{
		case 24:	flag = GBM_FT_W24;	break;
		case 8:		flag = GBM_FT_W8;	break;
		case 4:		flag = GBM_FT_W4;	break;
		case 1:		flag = GBM_FT_W1;	break;
		}
	if ( (gbmft.flags & flag) == 0 )
		{
		gbm_io_close(fd);
		fatal("output bitmap format %s does not support writing %d bpp data",
			gbmft.short_name, gbm.bpp);
		}

	if ( (rc = gbm_read_palette(fd, ft_src, &gbm, gbmrgb)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read palette of %s: %s", fn_src, gbm_err(rc));
		}

	stride_src = ( ((gbm.w * gbm.bpp + 31)/32) * 4 );
	bytes = stride_src * gbm.h;
	if ( (data = malloc((size_t) bytes)) == NULL )
		{
		gbm_io_close(fd);
		fatal("out of memory allocating %d bytes for bitmap", bytes);
		}

	if ( (rc = gbm_read_data(fd, ft_src, &gbm, data)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read bitmap data of %s: %s", fn_src, gbm_err(rc));
		}

	gbm_io_close(fd);

	gbm_subrectangle(&gbm, x, y, w, h, data, data);

	if ( (fd = gbm_io_create(fn_dst, O_WRONLY|O_BINARY)) == -1 )
		fatal("can't create %s", fn_dst);

	gbm.w = w;
	gbm.h = h;

	if ( (rc = gbm_write(fn_dst, fd, ft_dst, &gbm, gbmrgb, data, opt_dst)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		remove(fn_dst);
		fatal("can't write %s: %s", fn_dst, gbm_err(rc));
		}

	gbm_io_close(fd);

	free(data);

	gbm_deinit();

	return 0;
	}
/*...e*/
