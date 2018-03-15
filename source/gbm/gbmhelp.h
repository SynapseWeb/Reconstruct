/*

gbmhelp.h - Internal helpers for GBM file I/O stuff

*/

#ifndef GBMHELP_H
#define	GBMHELP_H

/*...sincludes:0:*/
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
/*...e*/

extern gbm_boolean gbm_same(const char *s1, const char *s2, int n);
extern const char *gbm_find_word(const char *str, const char *substr);
extern const char *gbm_find_word_prefix(const char *str, const char *substr);

extern int  (*gbm_file_open  )(const char *fn, int mode);
extern int  (*gbm_file_create)(const char *fn, int mode);
extern void (*gbm_file_close )(int fd);
extern long (*gbm_file_lseek )(int fd, long pos, int whence);
extern int  (*gbm_file_read  )(int fd, void *buf, int len);
extern int  (*gbm_file_write )(int fd, const void *buf, int len);

typedef void AHEAD;
extern AHEAD *gbm_create_ahead(int fd);
extern void   gbm_destroy_ahead(AHEAD *ahead);
extern int    gbm_read_ahead(AHEAD *ahead);

#endif
