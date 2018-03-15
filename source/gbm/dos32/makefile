#
# Generalised Bitmap Module
#
# 32 bit DOS
# Watcom C/C++ 11.0b + CauseWay 1.3
#

# Modify this line to point to the Independant JPEG groups library 6a
# If you haven't got it, comment out the line instead.
IJG =		jpeg-6a

!ifdef IJG
CJPEG =		-DIJG -I$(IJG)
!else
#CJPEG =
!endif

CFLAGS =	-bt=DOS -dDOS32 -oit -4r -s -w3 -we -zp4 -mf -zq -fr $(CJPEG)
LIBFLAGS =	-q -n -b -c

.SUFFIXES:	.c .obj

.c.obj:
		wcc386 $(CFLAGS) $*.c

#

all:		ijg gbm.lib gbmhdr.exe \
		gbmmir.lib gbmref.exe \
		gbmrect.lib gbmsub.exe \
		gbmscale.lib gbmsize.exe \
		gbmerr.lib gbmtrunc.lib gbmht.lib gbmhist.lib gbmmcut.lib gbmbpp.exe \
		gbmcpal.exe \
		gbmgamma.exe

#

ijg:
!ifdef IJG
		@echo JPEG support, via Independent JPEG Group
		copy $(IJG)\jconfig.doc $(IJG)\jconfig.h
		copy makeijgo $(IJG)
		copy jmorecfg.h $(IJG)
		(cd $(IJG) && nmake -c -f makeijgo)
!else
		@echo No JPEG support
!endif

#

!ifdef IJG
IJGOBJ =	$(IJG)\jcapimin.obj $(IJG)\jcapistd.obj $(IJG)\jctrans.obj  \
		$(IJG)\jcparam.obj  $(IJG)\jdatadst.obj $(IJG)\jcinit.obj   \
		$(IJG)\jcmaster.obj $(IJG)\jcmarker.obj $(IJG)\jcmainct.obj \
		$(IJG)\jcprepct.obj $(IJG)\jccoefct.obj $(IJG)\jccolor.obj  \
		$(IJG)\jcsample.obj $(IJG)\jchuff.obj   $(IJG)\jcphuff.obj  \
		$(IJG)\jcdctmgr.obj $(IJG)\jfdctfst.obj $(IJG)\jfdctflt.obj \
		$(IJG)\jfdctint.obj $(IJG)\jdapimin.obj $(IJG)\jdapistd.obj \
		$(IJG)\jdtrans.obj  $(IJG)\jdatasrc.obj $(IJG)\jdmaster.obj \
		$(IJG)\jdinput.obj  $(IJG)\jdmarker.obj $(IJG)\jdhuff.obj   \
		$(IJG)\jdphuff.obj  $(IJG)\jdmainct.obj $(IJG)\jdcoefct.obj \
		$(IJG)\jdpostct.obj $(IJG)\jddctmgr.obj $(IJG)\jidctfst.obj \
		$(IJG)\jidctflt.obj $(IJG)\jidctint.obj $(IJG)\jidctred.obj \
		$(IJG)\jdsample.obj $(IJG)\jdcolor.obj  $(IJG)\jquant1.obj  \
		$(IJG)\jquant2.obj  $(IJG)\jdmerge.obj  $(IJG)\jcomapi.obj  \
		$(IJG)\jutils.obj   $(IJG)\jerror.obj   $(IJG)\jmemmgr.obj  \
		$(IJG)\jmemnobs.obj
IJGOBJP =	+$(IJG)\jcapimin.obj +$(IJG)\jcapistd.obj +$(IJG)\jctrans.obj  \
		+$(IJG)\jcparam.obj  +$(IJG)\jdatadst.obj +$(IJG)\jcinit.obj   \
		+$(IJG)\jcmaster.obj +$(IJG)\jcmarker.obj +$(IJG)\jcmainct.obj \
		+$(IJG)\jcprepct.obj +$(IJG)\jccoefct.obj +$(IJG)\jccolor.obj  \
		+$(IJG)\jcsample.obj +$(IJG)\jchuff.obj   +$(IJG)\jcphuff.obj  \
		+$(IJG)\jcdctmgr.obj +$(IJG)\jfdctfst.obj +$(IJG)\jfdctflt.obj \
		+$(IJG)\jfdctint.obj +$(IJG)\jdapimin.obj +$(IJG)\jdapistd.obj \
		+$(IJG)\jdtrans.obj  +$(IJG)\jdatasrc.obj +$(IJG)\jdmaster.obj \
		+$(IJG)\jdinput.obj  +$(IJG)\jdmarker.obj +$(IJG)\jdhuff.obj   \
		+$(IJG)\jdphuff.obj  +$(IJG)\jdmainct.obj +$(IJG)\jdcoefct.obj \
		+$(IJG)\jdpostct.obj +$(IJG)\jddctmgr.obj +$(IJG)\jidctfst.obj \
		+$(IJG)\jidctflt.obj +$(IJG)\jidctint.obj +$(IJG)\jidctred.obj \
		+$(IJG)\jdsample.obj +$(IJG)\jdcolor.obj  +$(IJG)\jquant1.obj  \
		+$(IJG)\jquant2.obj  +$(IJG)\jdmerge.obj  +$(IJG)\jcomapi.obj  \
		+$(IJG)\jutils.obj   +$(IJG)\jerror.obj   +$(IJG)\jmemmgr.obj  \
		+$(IJG)\jmemnobs.obj
!else
IJGOBJ =
IJGOBJP =
!endif

gbm.lib:	gbm.obj gbmpgm.obj gbmppm.obj gbmbmp.obj gbmtga.obj gbmkps.obj gbmiax.obj gbmpcx.obj gbmtif.obj gbmtifh.obj gbmlbm.obj gbmvid.obj gbmgif.obj gbmxbm.obj gbmspr.obj gbmpsg.obj gbmgem.obj gbmcvp.obj gbmjpg.obj gbmhelp.obj $(IJGOBJ)
		wlib $(LIBFLAGS) gbm.lib +gbm.obj +gbmpgm.obj +gbmppm.obj +gbmbmp.obj +gbmtga.obj +gbmkps.obj +gbmiax.obj +gbmpcx.obj +gbmtif.obj +gbmtifh.obj +gbmlbm.obj +gbmvid.obj +gbmgif.obj +gbmxbm.obj +gbmspr.obj +gbmpsg.obj +gbmgem.obj +gbmcvp.obj +gbmjpg.obj +gbmhelp.obj $(IJGOBJP)

gbm.obj:	gbm.c gbm.h gbmhelp.h gbmpgm.h gbmppm.h gbmbmp.h gbmtga.h gbmkps.h gbmiax.h gbmpcx.h gbmtif.h gbmlbm.h gbmvid.h gbmgif.h gbmxbm.h gbmspr.h gbmpsg.h gbmgem.h gbmcvp.h gbmjpg.h

gbmpgm.obj:	gbmpgm.c gbm.h gbmhelp.h

gbmppm.obj:	gbmppm.c gbm.h gbmhelp.h

gbmbmp.obj:	gbmbmp.c gbm.h gbmhelp.h

gbmtga.obj:	gbmtga.c gbm.h gbmhelp.h

gbmkps.obj:	gbmkps.c gbm.h gbmhelp.h

gbmiax.obj:	gbmiax.c gbm.h gbmhelp.h

gbmpcx.obj:	gbmpcx.c gbm.h gbmhelp.h

gbmtif.obj:	gbmtif.c gbm.h gbmhelp.h gbmtifh.h

gbmtifh.obj:	gbmtifh.c gbmtifh.h

gbmlbm.obj:	gbmlbm.c gbm.h gbmhelp.h

gbmvid.obj:	gbmvid.c gbm.h gbmhelp.h

gbmgif.obj:	gbmgif.c gbm.h gbmhelp.h

gbmxbm.obj:	gbmxbm.c gbm.h gbmhelp.h

gbmspr.obj:	gbmspr.c gbm.h gbmhelp.h

gbmpsg.obj:	gbmpsg.c gbm.h gbmhelp.h

gbmgem.obj:	gbmgem.c gbm.h gbmhelp.h

gbmcvp.obj:	gbmcvp.c gbm.h gbmhelp.h

gbmjpg.obj:	gbmjpg.c gbm.h gbmhelp.h

gbmhelp.obj:	gbmhelp.c gbm.h

#

gbmhdr.exe:	gbmhdr.obj gbm.lib
		wlink @<<
			System CauseWay
			Name $@
			File gbmhdr.obj
			Library gbm.lib
			Option Quiet
			Option Stack = 0x2000
<<

gbmhdr.obj:	gbmhdr.c gbm.h

#

gbmmir.lib:	gbmmir.obj
		wlib $(LIBFLAGS) gbmmir.lib +gbmmir.obj

gbmmir.obj:	gbmmir.c

gbmref.exe:	gbmref.obj gbm.lib gbmmir.lib
		wlink @<<
			System CauseWay
			Name $@
			File gbmref.obj
			Library gbm.lib,gbmmir.lib
			Option Quiet
			Option Stack = 0x2000
<<

gbmref.obj:	gbmref.c gbm.h gbmmir.h

#

gbmrect.lib:	gbmrect.obj
		wlib $(LIBFLAGS) gbmrect.lib +gbmrect.obj

gbmrect.obj:	gbmrect.c

gbmsub.exe:	gbmsub.obj gbm.lib gbmrect.lib
		wlink @<<
			System CauseWay
			Name $@
			File gbmsub.obj
			Library gbm.lib,gbmrect.lib
			Option Quiet
			Option Stack = 0x2000
<<

gbmsub.obj:	gbmsub.c gbm.h gbmrect.h

#

gbmscale.lib:	gbmscale.obj
		wlib $(LIBFLAGS) gbmscale.lib +gbmscale.obj

gbmscale.obj:	gbmscale.c

gbmsize.exe:	gbmsize.obj gbm.lib gbmscale.lib
		wlink @<<
			System CauseWay
			Name $@
			File gbmsize.obj
			Library gbm.lib,gbmscale.lib
			Option Quiet
			Option Stack = 0x2000
<<

gbmsize.obj:	gbmsize.c gbm.h gbmscale.h

#

gbmerr.lib:	gbmerr.obj
		wlib $(LIBFLAGS) gbmerr.lib +gbmerr.obj

gbmerr.obj:	gbmerr.c

#

gbmtrunc.lib:	gbmtrunc.obj
		wlib $(LIBFLAGS) gbmtrunc.lib +gbmtrunc.obj

gbmtrunc.obj:	gbmtrunc.c

#

gbmht.lib:	gbmht.obj
		wlib $(LIBFLAGS) gbmht.lib +gbmht.obj

gbmht.obj:	gbmht.c

#

gbmhist.lib:	gbmhist.obj
		wlib $(LIBFLAGS) gbmhist.lib +gbmhist.obj

gbmhist.obj:	gbmhist.c

#

gbmmcut.lib:	gbmmcut.obj
		wlib $(LIBFLAGS) gbmmcut.lib +gbmmcut.obj

gbmmcut.obj:	gbmmcut.c

#

gbmbpp.exe:	gbmbpp.obj gbm.lib gbmerr.lib gbmtrunc.lib gbmht.lib gbmhist.lib gbmmcut.lib
		wlink @<<
			System CauseWay
			Name $@
			File gbmbpp.obj
			Library gbm.lib,gbmerr.lib,gbmtrunc.lib,gbmht.lib
			Library gbmhist.lib,gbmmcut.lib
			Option Quiet
			Option Stack = 0x2000
<<

gbmbpp.obj:	gbmbpp.c gbm.h gbmerr.h gbmtrunc.h gbmht.h gbmhist.h gbmmcut.h

#

gbmcpal.exe:	gbmcpal.obj gbm.lib gbmhist.lib gbmmcut.lib
		wlink @<<
			System CauseWay
			Name $@
			File gbmcpal.obj
			Library gbm.lib,gbmhist.lib,gbmmcut.lib
			Option Quiet
			Option Stack = 0x2000
<<

gbmcpal.obj:	gbmcpal.c gbm.h gbmhist.h gbmmcut.h

#

gbmgamma.exe:	gbmgamma.obj gbm.lib
		wlink @<<
			System CauseWay
			Name $@
			File gbmgamma.obj
			Library gbm.lib
			Option Quiet
			Option Stack = 0x2000
<<

gbmgamma.obj:	gbmgamma.c gbm.h

#

clean:
		-del *.obj 2> nul

#

package:
		zip -q gbmdos32 gbm.htm gbm.h gbm.lib *.exe
