/*

gbmgamma.c - Gamma correct General Bitmap

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <math.h>
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

/*...vgbm\46\h:0:*/
/*...e*/

static char progname[] = "gbmgamma";

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

	fprintf(stderr, "usage: %s [-m map] [-g gamma] [-s shelf] fn1.ext{,opt} [--] [fn2.ext{,opt}]\n", progname);
	fprintf(stderr, "flags: -m map         mapping in the form ?_to_? (default: i_to_l), where ? is\n");
	fprintf(stderr, "                      i  physical intensitys (eg: raytracer output)\n");
	fprintf(stderr, "                      p  gamma corrected for a specific monitor\n");
	fprintf(stderr, "                      l  L* cyclometric linear perceived intensitys (as in PM)\n");
	fprintf(stderr, "       -g gamma       set monitor gamma (default 2.1)\n");
	fprintf(stderr, "       -s shelf       set monitor shelf (default 0.0)\n");
	fprintf(stderr, "                      gamma and shelf only used for mapping to or from p\n");
	fprintf(stderr, "                      for 8514 monitor gamma=2.3,shelf=0.136\n");
	fprintf(stderr, "                      for 8515 monitor gamma=2.1,shelf=0.0\n");
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
/*...sget_opt_double:0:*/
static double get_opt_double(const char *s, const char *name)
	{
	double v;

	if ( s == NULL )
		fatal("missing %s argument", name);
	sscanf(s, "%lf", &v);

	return v;
	}
/*...e*/
/*...ssame:0:*/
static gbm_boolean same(const char *s1, const char *s2, int n)
	{
	for ( ; n--; s1++, s2++ )
		if ( tolower(*s1) != tolower(*s2) )
			return GBM_FALSE;
	return GBM_TRUE;
	}
/*...e*/
/*...smain:0:*/
/*...smapinfos:0:*/
#define	CVT_NONE	0
#define	CVT_I_TO_P	1
#define	CVT_P_TO_I	2
#define	CVT_I_TO_L	3
#define	CVT_L_TO_I	4
#define	CVT_P_TO_L	5
#define	CVT_L_TO_P	6

typedef struct { char *name; int m; } MAPINFO;

static MAPINFO mapinfos[] =
	{
	"none",		CVT_NONE,
	"i_to_p",	CVT_I_TO_P,
	"p_to_i",	CVT_P_TO_I,
	"i_to_l",	CVT_I_TO_L,
	"l_to_i",	CVT_L_TO_I,
	"p_to_l",	CVT_P_TO_L,
	"l_to_p",	CVT_L_TO_P,
	};

#define	N_MAPINFOS	(sizeof(mapinfos)/sizeof(mapinfos[0]))
/*...e*/

/*...smap_compute:0:*/
/*...slstar_from_i:0:*/
static double lstar_from_i(double y)
	{
	y = pow(1.16 * y, 1.0/3.0) - 0.16;

	if ( y < 0.0 ) y = 0.0; else if ( y > 1.0 ) y = 1.0;

	return y;
	}
/*...e*/
/*...si_from_lstar:0:*/
static double i_from_lstar(double y)
	{
	y = pow(y + 0.16, 3.0) / 1.16;

	if ( y < 0.0 ) y = 0.0; else if ( y > 1.0 ) y = 1.0;

	return y;
	}
/*...e*/
/*...spal_from_i:0:*/
static double pal_from_i(double y, double gam, double shelf)
	{
	y = pow(y,1.0 / gam) * (1.0 - shelf) + shelf;

	if ( y < 0.0 ) y = 0.0; else if ( y > 1.0 ) y = 1.0;

	return y;
	}
/*...e*/
/*...si_from_pal:0:*/
static double i_from_pal(double y, double gam, double shelf)
	{
	if ( y >= shelf )
		y = pow((y - shelf) / (1.0 - shelf), gam);
	else
		y = 0.0;

	if ( y < 0.0 ) y = 0.0; else if ( y > 1.0 ) y = 1.0;

	return y;
	}
/*...e*/

static void map_compute(int m, gbm_u8 remap[], double gam, double shelf)
	{
	int	i;

	for ( i = 0; i < 0x100; i++ )
		{
		double y = (double) i / 255.0;

		switch ( m )
			{
			case CVT_I_TO_P: y = pal_from_i(y, gam, shelf); break;
			case CVT_P_TO_I: y = i_from_pal(y, gam, shelf); break;
			case CVT_I_TO_L: y = lstar_from_i(y); break;
			case CVT_L_TO_I: y = i_from_lstar(y); break;
			case CVT_P_TO_L: y = lstar_from_i(i_from_pal(y, gam, shelf)); break;
			case CVT_L_TO_P: y = pal_from_i(i_from_lstar(y), gam, shelf); break;
			}

		remap[i] = (gbm_u8) (y * 255.0);
		}
	}
/*...e*/
/*...smap_data:0:*/
static void map_data(gbm_u8 *data, int w, int h, const gbm_u8 remap[])
	{
	int stride = ((w * 3 + 3) & ~3);
	int x, y;

	for ( y = 0; y < h; y++, data += stride )
		for ( x = 0; x < w * 3; x++ )
			data[x] = remap[data[x]];
	}
/*...e*/
/*...smap_palette:0:*/
static void map_palette(GBMRGB *gbmrgb, int npals, const gbm_u8 remap[])
	{
	for ( ; npals--; gbmrgb++ )
		{
		gbmrgb->b = remap[gbmrgb->b];
		gbmrgb->g = remap[gbmrgb->g];
		gbmrgb->r = remap[gbmrgb->r];
		}
	}
/*...e*/

int main(int argc, char *argv[])
	{
	char	fn_src[500+1], fn_dst[500+1], *opt_src, *opt_dst;
	int	fd, ft_src, ft_dst, i, stride, bytes, flag, m;
	GBM_ERR	rc;
	GBMFT	gbmft;
	GBM	gbm;
	GBMRGB	gbmrgb[0x100];
	gbm_u8	*data;
	char	*map = "none";
	gbm_u8	remap[0x100];
	double gam = 2.1, shelf = 0.0;

/*...scommand line arguments:8:*/
for ( i = 1; i < argc; i++ )
	{
	if ( argv[i][0] != '-' )
		break;
	else if ( argv[i][1] == '-' )
		{ ++i; break; }
	switch ( argv[i][1] )
		{
		case 'm':
			if ( ++i == argc )
				fatal("expected map argument");
			map = argv[i];
			break;
		case 'g':
			if ( ++i == argc ) usage();
			gam = get_opt_double(argv[i], "gam");
			if ( gam < 0.1 || gam > 10.0 )
				fatal("only gammas in the range 0.1 to 10.0 are sensible");
			break;
		case 's':
			if ( ++i == argc ) usage();
			shelf = get_opt_double(argv[i], "shelf");
			break;
		default:
			usage();
			break;
		}
	}
/*...e*/

/*...sdeduce mapping and bits per pixel etc\46\:8:*/
{
int j;

for ( j = 0; j < N_MAPINFOS; j++ )
	if ( same(map, mapinfos[j].name, (int) strlen(map) + 1) )
		break;
if ( j == N_MAPINFOS )
	fatal("unrecognised mapping %s", map);
m = mapinfos[j].m;
}
/*...e*/

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

	stride = ( ((gbm.w * gbm.bpp + 31)/32) * 4 );
	bytes = stride * gbm.h;
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

	map_compute(m, remap, gam, shelf);

	if ( gbm.bpp == 24 )
		map_data(data, gbm.w, gbm.h, remap);
	else
		map_palette(gbmrgb, 1 << gbm.bpp, remap);

	if ( (fd = gbm_io_create(fn_dst, O_WRONLY|O_BINARY)) == -1 )
		fatal("can't create %s", fn_dst);

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
