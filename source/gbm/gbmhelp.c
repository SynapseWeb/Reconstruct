/*

gbmhelp.c - Helpers for GBM file I/O stuff

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

/*...vgbm\46\h:0:*/
/*...e*/

/*...sgbm_same:0:*/
gbm_boolean gbm_same(const char *s1, const char *s2, int n)
	{
	for ( ; n--; s1++, s2++ )
		if ( tolower(*s1) != tolower(*s2) )
			return GBM_FALSE;
	return GBM_TRUE;
	}
/*...e*/
/*...sgbm_find_word:0:*/
const char *gbm_find_word(const char *str, const char *substr)
	{
	char buf[100+1], *s;
	int  len = strlen(substr);
	for ( s  = strtok(strcpy(buf, str), " \t,");
	      s != NULL;
	      s  = strtok(NULL, " \t,") )
		if ( gbm_same(s, substr, len) && s[len] == '\0' )
			{
			int inx = s - buf;
			return str + inx;
				/* Avoid referencing buf in the final return.
				   lcc and a Mac compiler see the buf, and then
				   warn about possibly returning the address
				   of an automatic variable! */
			}
	return NULL;
	}
/*...e*/
/*...sgbm_find_word_prefix:0:*/
const char *gbm_find_word_prefix(const char *str, const char *substr)
	{
	char buf[100+1], *s;
	int  len = strlen(substr);
	for ( s  = strtok(strcpy(buf, str), " \t,");
	      s != NULL;
	      s  = strtok(NULL, " \t,") )
		if ( gbm_same(s, substr, len) )
			{
			int inx = s - buf;
			return str + inx;
				/* Avoid referencing buf in the final return.
				   lcc and a Mac compiler see the buf, and then
				   warn about possibly returning the address
				   of an automatic variable! */
			}
	return NULL;
	}
/*...e*/
/*...sgbm_file_\42\:0:*/
/* Looking at this, you might think that the gbm_file_* function pointers
   could be made to point straight at the regular read,write etc..
   If we do this then we get into problems with different calling conventions
   (for example read is _Optlink under C-Set++ on OS/2), and also where
   function arguments differ (the length field to read is unsigned on OS/2).
   This simplest thing to do is simply to use the following veneers. */

#ifdef WIN32
/* Use ISO variants */
#define	fd_creat _creat
#define	fd_open  _open
#define	fd_close _close
#define	fd_read  _read
#define	fd_write _write
#define	fd_lseek _lseek
#else
/* Use POSIX variants */
#define	fd_creat creat
#define	fd_open  open
#define	fd_close close
#define	fd_read  read
#define	fd_write write
#define	fd_lseek lseek
#endif

static int def_open(const char *fn, int mode)
	{ return fd_open(fn, mode); }
static int def_create(const char *fn, int mode)
	{
#ifdef MAC
	return open(fn, O_CREAT|O_TRUNC|mode);
		/* S_IREAD and S_IWRITE won't exist on the Mac until MacOS/X */
#else
  #if defined(S_IRGRP) && defined(S_IROTH)
	/* Let umask remove some of these */
	return fd_open(fn, O_CREAT|O_TRUNC|mode, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
  #else
	/* A simple system without UNIX style permissions */
	return fd_open(fn, O_CREAT|O_TRUNC|mode, S_IREAD|S_IWRITE);
  #endif
#endif
	}
static void def_close(int fd)
	{ fd_close(fd); }
static long def_lseek(int fd, long pos, int whence)
	{ return fd_lseek(fd, pos, whence); }
static int def_read(int fd, void *buf, int len)
	{ return fd_read(fd, buf, len); }
static int def_write(int fd, const void *buf, int len)
	{
#ifdef MAC
	/* Prototype for write is missing a 'const' */
	return write(fd, (void *) buf, len);
#else
	return fd_write(fd, buf, len);
#endif
	}

int  (*gbm_file_open  )(const char *fn, int mode)         = def_open  ;
int  (*gbm_file_create)(const char *fn, int mode)         = def_create;
void (*gbm_file_close )(int fd)                           = def_close ;
long (*gbm_file_lseek )(int fd, long pos, int whence)     = def_lseek ;
int  (*gbm_file_read  )(int fd, void *buf, int len)       = def_read  ;
int  (*gbm_file_write )(int fd, const void *buf, int len) = def_write ;
/*...e*/
/*...sreading ahead:0:*/
#define	AHEAD_BUF 0x4000

typedef struct
	{
	gbm_u8 buf[AHEAD_BUF];
	int inx, cnt;
	int fd;
	} AHEAD;

AHEAD *gbm_create_ahead(int fd)
	{
	AHEAD *ahead;
	if ( (ahead = malloc((size_t) sizeof(AHEAD))) == NULL )
		return NULL;
	ahead->inx = 0;
	ahead->cnt = 0;
	ahead->fd  = fd;
	return ahead;
	}

void gbm_destroy_ahead(AHEAD *ahead)
	{
	free(ahead);
	}	

int gbm_read_ahead(AHEAD *ahead)
	{
	if ( ahead->inx >= ahead->cnt )
		{
		ahead->cnt = gbm_file_read(ahead->fd, (char *) ahead->buf, AHEAD_BUF);
		if ( ahead->cnt <= 0 )
			return -1;
		ahead->inx = 0;
		}
	return (int) (unsigned int) ahead->buf[ahead->inx++];
	}
/*...e*/
