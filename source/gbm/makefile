#
# Generalised Bitmap Module
#
# UNIX
#

JPEG = 1

ifdef JPEG
IJG =		jpeg-6a
CJPEG =		-DIJG -I$(IJG)
else
CJPEG =
endif

USE_STRIP =	strip
USE_AR =	ar

ifdef AIXxlc
CC =		cc
CFLAGS =	-c -O -DUNIX -DAIX   $(CJPEG)
else
ifdef AIX
CC =		gcc
CFLAGS =	-c -O -DUNIX -DAIX   $(CJPEG)
else
ifdef HP
CC =		cc
CFLAGS =	-c -O -DUNIX -DHP    $(CJPEG) -Aa
else
ifdef SUN
CC =		gcc
CFLAGS =	-c -O -DUNIX -DSUN   $(CJPEG)
else
ifdef SUNcc
CC =		cc
CFLAGS =	-c -O -DUNIX -DSUN   $(CJPEG) -Xa
else
ifdef XNX
CC =		cc
CFLAGS =	-c -O -DUNIX -DXNX   $(CJPEG)
else
ifdef MACOSX
CC =		cc
CFLAGS =	-c -O -DUNIX -DMACOSX $(CJPEG)
USE_RANLIB =	ranlib
else
ifdef MINGW
CC =		gcc
CFLAGS =	-c -O -DUNIX -DMINGW $(CJPEG)
USE_RANLIB =
else
ifdef IPHONE
CC =		arm-apple-darwin-gcc
CFLAGS =	-c -O -DUNIX -DIPHONE $(CJPEG)
USE_AR =	arm-apple-darwin-ar
USE_RANLIB =	arm-apple-darwin-ranlib
USE_STRIP =	arm-apple-darwin-strip
else
CC =		gcc
CFLAGS =	-c -O -DUNIX -DLINUX $(CJPEG)
endif
endif
endif
endif
endif
endif
endif
endif
endif

.SUFFIXES:	.c .o

.c.o:
		$(CC) $(CFLAGS) $*.c

#

gbm.a:		gbm.o gbmpgm.o gbmppm.o gbmbmp.o gbmtga.o gbmkps.o gbmiax.o gbmpcx.o gbmtif.o gbmtifh.o gbmlbm.o gbmvid.o gbmgif.o gbmxbm.o gbmspr.o gbmpsg.o gbmgem.o gbmcvp.o gbmjpg.o gbmhelp.o $(IJGOBJ)
		$(USE_AR) ru gbm.a gbm.o gbmpgm.o gbmppm.o gbmbmp.o gbmtga.o gbmkps.o gbmiax.o gbmpcx.o gbmtif.o gbmtifh.o gbmlbm.o gbmvid.o gbmgif.o gbmxbm.o gbmspr.o gbmpsg.o gbmgem.o gbmcvp.o gbmjpg.o gbmhelp.o $(IJGOBJ)

all:		ijg gbm.a gbmhdr \
		gbmmir.a gbmref \
		gbmrect.a gbmsub \
		gbmscale.a gbmsize \
		gbmerr.a gbmtrunc.a gbmht.a gbmhist.a gbmmcut.a gbmbpp \
		gbmcpal \
		gbmgamma

#

ijg:
ifdef JPEG
		@echo JPEG support, via Independent JPEG Group
		cp $(IJG)/jconfig.doc $(IJG)/jconfig.h
		cp makeijgo $(IJG)
		cp jmorecfg.h $(IJG)
		(cd $(IJG) && mingw32-make CC=$(CC) -f makeijgo)
else
		@echo No JPEG support
endif

#

ifdef JPEG
IJGOBJ =	$(IJG)/jcapimin.o $(IJG)/jcapistd.o $(IJG)/jctrans.o  \
		$(IJG)/jcparam.o  $(IJG)/jdatadst.o $(IJG)/jcinit.o   \
		$(IJG)/jcmaster.o $(IJG)/jcmarker.o $(IJG)/jcmainct.o \
		$(IJG)/jcprepct.o $(IJG)/jccoefct.o $(IJG)/jccolor.o  \
		$(IJG)/jcsample.o $(IJG)/jchuff.o   $(IJG)/jcphuff.o  \
		$(IJG)/jcdctmgr.o $(IJG)/jfdctfst.o $(IJG)/jfdctflt.o \
		$(IJG)/jfdctint.o $(IJG)/jdapimin.o $(IJG)/jdapistd.o \
		$(IJG)/jdtrans.o  $(IJG)/jdatasrc.o $(IJG)/jdmaster.o \
		$(IJG)/jdinput.o  $(IJG)/jdmarker.o $(IJG)/jdhuff.o   \
		$(IJG)/jdphuff.o  $(IJG)/jdmainct.o $(IJG)/jdcoefct.o \
		$(IJG)/jdpostct.o $(IJG)/jddctmgr.o $(IJG)/jidctfst.o \
		$(IJG)/jidctflt.o $(IJG)/jidctint.o $(IJG)/jidctred.o \
		$(IJG)/jdsample.o $(IJG)/jdcolor.o  $(IJG)/jquant1.o  \
		$(IJG)/jquant2.o  $(IJG)/jdmerge.o  $(IJG)/jcomapi.o  \
		$(IJG)/jutils.o   $(IJG)/jerror.o   $(IJG)/jmemmgr.o  \
		$(IJG)/jmemnobs.o
else
IJGOBJ =
endif

ifdef USE_RANLIB
		$(USE_RANLIB) gbm.a
endif

gbm.o:		gbm.c gbm.h gbmhelp.h gbmpgm.h gbmppm.h gbmbmp.h gbmtga.h gbmkps.h gbmiax.h gbmpcx.h gbmtif.h gbmlbm.h gbmvid.h gbmgif.h gbmxbm.h gbmspr.h gbmpsg.h gbmgem.h gbmcvp.h gbmjpg.h

gbmpgm.o:	gbmpgm.c gbm.h gbmhelp.h

gbmppm.o:	gbmppm.c gbm.h gbmhelp.h

gbmbmp.o:	gbmbmp.c gbm.h gbmhelp.h

gbmtga.o:	gbmtga.c gbm.h gbmhelp.h

gbmkps.o:	gbmkps.c gbm.h gbmhelp.h

gbmiax.o:	gbmiax.c gbm.h gbmhelp.h

gbmpcx.o:	gbmpcx.c gbm.h gbmhelp.h

gbmtif.o:	gbmtif.c gbm.h gbmhelp.h gbmtifh.h

gbmtifh.o:	gbmtifh.c gbmtifh.h

gbmlbm.o:	gbmlbm.c gbm.h gbmhelp.h

gbmvid.o:	gbmvid.c gbm.h gbmhelp.h

gbmgif.o:	gbmgif.c gbm.h gbmhelp.h

gbmxbm.o:	gbmxbm.c gbm.h gbmhelp.h

gbmspr.o:	gbmspr.c gbm.h gbmhelp.h

gbmpsg.o:	gbmpsg.c gbm.h gbmhelp.h

gbmgem.o:	gbmgem.c gbm.h gbmhelp.h

gbmcvp.o:	gbmcvp.c gbm.h gbmhelp.h

gbmjpg.o:	gbmjpg.c gbm.h gbmhelp.h

gbmhelp.o:	gbmhelp.c gbm.h

#

gbmhdr:		gbmhdr.o gbm.a
		$(CC) -o gbmhdr gbmhdr.o gbm.a
		$(USE_STRIP) gbmhdr

gbmhdr.o:	gbmhdr.c gbm.h

#

gbmmir.a:	gbmmir.o
		$(USE_AR) ru gbmmir.a gbmmir.o
ifdef USE_RANLIB
		$(USE_RANLIB) gbmmir.a
endif

gbmmir.o:	gbmmir.c

gbmref:		gbmref.o gbm.a gbmmir.a
		$(CC) -o gbmref gbmref.o gbm.a gbmmir.a
		$(USE_STRIP) gbmref

gbmref.o:	gbmref.c gbm.h gbmmir.h

#

gbmrect.a:	gbmrect.o
		$(USE_AR) ru gbmrect.a gbmrect.o
ifdef USE_RANLIB
		$(USE_RANLIB) gbmrect.a
endif

gbmrect.o:	gbmrect.c

gbmsub:		gbmsub.o gbm.a gbmrect.a
		$(CC) -o gbmsub gbmsub.o gbm.a gbmrect.a
		$(USE_STRIP) gbmsub

gbmsub.o:	gbmsub.c gbm.h gbmrect.h

#

gbmscale.a:	gbmscale.o
		$(USE_AR) ru gbmscale.a gbmscale.o
ifdef USE_RANLIB
		$(USE_RANLIB) gbmscale.a
endif

gbmscale.o:	gbmscale.c

gbmsize:	gbmsize.o gbm.a gbmscale.a
		$(CC) -o gbmsize gbmsize.o gbm.a gbmscale.a
		$(USE_STRIP) gbmsize

gbmsize.o:	gbmsize.c gbm.h gbmscale.h

#

gbmerr.a:	gbmerr.o
		$(USE_AR) ru gbmerr.a gbmerr.o
ifdef USE_RANLIB
		$(USE_RANLIB) gbmerr.a
endif

gbmerr.o:	gbmerr.c

#

gbmtrunc.a:	gbmtrunc.o
		$(USE_AR) ru gbmtrunc.a gbmtrunc.o
ifdef USE_RANLIB
		$(USE_RANLIB) gbmtrunc.a
endif

gbmtrunc.o:	gbmtrunc.c

#

gbmht.a:	gbmht.o
		$(USE_AR) ru gbmht.a gbmht.o
ifdef USE_RANLIB
		$(USE_RANLIB) gbmht.a
endif

gbmht.o:	gbmht.c

#

gbmhist.a:	gbmhist.o
		$(USE_AR) ru gbmhist.a gbmhist.o
ifdef USE_RANLIB
		$(USE_RANLIB) gbmhist.a
endif

gbmhist.o:	gbmhist.c

#

gbmmcut.a:	gbmmcut.o
		$(USE_AR) ru gbmmcut.a gbmmcut.o
ifdef USE_RANLIB)
		$(USE_RANLIB) gbmmcut.a
endif

gbmmcut.o:	gbmmcut.c

#

gbmbpp:		gbmbpp.o gbm.a gbmerr.a gbmtrunc.a gbmht.a gbmhist.a gbmmcut.a
		$(CC) -o gbmbpp gbmbpp.o gbm.a gbmerr.a gbmtrunc.a gbmht.a gbmhist.a gbmmcut.a
		$(USE_STRIP) gbmbpp

gbmbpp.o:	gbmbpp.c gbm.h gbmerr.h gbmtrunc.h gbmht.h gbmhist.h gbmmcut.h

#

gbmcpal:	gbmcpal.o gbm.a gbmhist.a gbmmcut.a
		$(CC) -o gbmcpal gbmcpal.o gbm.a gbmhist.a gbmmcut.a
		$(USE_STRIP) gbmcpal

gbmcpal.o:	gbmcpal.c gbm.h gbmhist.h gbmmcut.h

#

gbmgamma:	gbmgamma.o gbm.a
		$(CC) -o gbmgamma gbmgamma.o gbm.a -lm
		$(USE_STRIP) gbmgamma

gbmgamma.o:	gbmgamma.c gbm.h

#

clean:
		@rm -f *.o

#

IFILES =	gbmhdr gbmref gbmsub gbmsize gbmbpp gbmcpal gbmgamma
IDIR =		/usr/local/bin

install:
		cp $(IFILES) $(IDIR)
		( cd $(IDIR) && chmod 755 $(IFILES) )

#

package:
		zip -q -r gbm *
		zip -q -d gbm *.bak *.o jpeg-6a/*.o *.a *.zip

website:	package
		mv *.zip /pub/gbm
		cp *.htm /pub/gbm
