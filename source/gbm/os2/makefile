#
# Generalised Bitmap Module
#
# OS/2
# IBM VisualAge C++, with CTC306, CTD302 and CTU304
#
# Builds GBM.DLL which is compiled multithreaded
# Therefore all users should also be multithreaded
#
# Note problems have been observed when optimisation allows the instruction
# scheduler to do its thing. Bug present beleived present in IBM C-Set++, with
# CSD 12, and in particular affects inner loops in gbmht.c and gbmerr.c.
# Bug suspected also present in VisualAge C++, even with CTC305.
#

# Modify this line to point to the Independant JPEG groups library 6a
# If you haven't got it, comment out the line instead.
IJG = jpeg-6a

!ifdef IJG
CJPEG =		/DIJG
!else
CJPEG =
!endif

CWARNS =	/W3 /Wcnd+cns+dcl+ord+par+pro+rea+ret+use+
CFLAGS =	/C+ $(CWARNS) /DOS2 /G4 /Gd-m+ /Q+ /O+ /Os- $(CJPEG)
CFLAGS_EXE =	$(CFLAGS) /Ge+
CFLAGS_DLL =	$(CFLAGS) /Ge-
LFLAGS =	/NOI /ALIGN:4 /NOLOGO
LFLAGS_EXE =	$(LFLAGS) /BASE:0x10000 /STACK:0x6000 /PMTYPE:VIO
LFLAGS_DLL =	$(LFLAGS)

.SUFFIXES:	.c .obj

.c.obj:
		icc $(CFLAGS_EXE) $*.c

#

all:		ijg gbm.dll gbm.lib gbmhdr.exe \
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
		@copy $(IJG)\jconfig.doc $(IJG)\jconfig.h
		@copy makeijgo $(IJG)
		@copy jmorecfg.h $(IJG)
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
!else
IJGOBJ =
!endif

gbm.dll:	gbm.obj gbmpgm.obj gbmppm.obj gbmbmp.obj gbmtga.obj \
		gbmkps.obj gbmiax.obj gbmpcx.obj gbmtif.obj gbmtifh.obj \
		gbmlbm.obj gbmvid.obj gbmgif.obj gbmxbm.obj gbmspr.obj \
		gbmpsg.obj gbmgem.obj gbmcvp.obj gbmjpg.obj gbmhelp.obj \
		gbm.def
		ilink $(LFLAGS_DLL) /OUT:$@ @<<
			$** $(IJGOBJ)
<<

gbm.lib:	gbm.def
		implib /NOLOGO /NOI $@ $**

gbm.obj:	gbm.c gbm.h gbmhelp.h gbmpgm.h gbmppm.h gbmbmp.h gbmtga.h gbmkps.h gbmiax.h gbmpcx.h gbmtif.h gbmlbm.h gbmvid.h gbmgif.h gbmxbm.h gbmspr.h gbmpsg.h gbmgem.h gbmcvp.h gbmjpg.h
		icc $(CFLAGS_DLL) $*.c

gbmpgm.obj:	gbmpgm.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmppm.obj:	gbmppm.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmbmp.obj:	gbmbmp.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmtga.obj:	gbmtga.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmkps.obj:	gbmkps.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmiax.obj:	gbmiax.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmpcx.obj:	gbmpcx.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmtif.obj:	gbmtif.c gbm.h gbmhelp.h gbmtifh.h
		icc $(CFLAGS_DLL) $*.c

gbmtifh.obj:	gbmtifh.c gbmtifh.h
		icc $(CFLAGS_DLL) $*.c

gbmlbm.obj:	gbmlbm.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmvid.obj:	gbmvid.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmgif.obj:	gbmgif.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmxbm.obj:	gbmxbm.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmspr.obj:	gbmspr.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmpsg.obj:	gbmpsg.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmgem.obj:	gbmgem.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

gbmcvp.obj:	gbmcvp.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) $*.c

!ifdef IJG
gbmjpg.obj:	gbmjpg.c gbm.h gbmhelp.h
		icc $(CFLAGS_DLL) /I$(IJG) $*.c
!endif

gbmhelp.obj:	gbmhelp.c gbm.h
		icc $(CFLAGS_DLL) $*.c

#

gbmhdr.exe:	gbmhdr.obj gbm.lib
		ilink $(LFLAGS_EXE) /OUT:$@ /NOE setargv.obj $**

gbmhdr.obj:	gbmhdr.c gbm.h

#

gbmmir.lib:	gbmmir.obj
		-del $@ 2> nul
		ilib /NOLOGO $@ $**;

gbmmir.obj:	gbmmir.c

gbmref.exe:	gbmref.obj gbm.lib gbmmir.lib
		ilink $(LFLAGS_EXE) /OUT:$@ $**

gbmref.obj:	gbmref.c gbm.h gbmmir.h

#

gbmrect.lib:	gbmrect.obj
		-del $@ 2> nul
		ilib /NOLOGO $@ $**;

gbmrect.obj:	gbmrect.c

gbmsub.exe:	gbmsub.obj gbm.lib gbmrect.lib
		ilink $(LFLAGS_EXE) /OUT:$@ $**

gbmsub.obj:	gbmsub.c gbm.h gbmrect.h

#

gbmerr.lib:	gbmerr.obj
		-del $@ 2> nul
		ilib /NOLOGO $@ $**;

gbmerr.obj:	gbmerr.c

#

gbmscale.lib:	gbmscale.obj
		-del $@ 2> nul
		ilib /NOLOGO $@ $**;

gbmscale.obj:	gbmscale.c

gbmsize.exe:	gbmsize.obj gbm.lib gbmscale.lib
		ilink $(LFLAGS_EXE) /OUT:$@ $**

gbmsize.obj:	gbmsize.c gbm.h gbmscale.h

#

gbmtrunc.lib:	gbmtrunc.obj
		-del $@ 2> nul
		ilib /NOLOGO $@ $**;

gbmtrunc.obj:	gbmtrunc.c

#

gbmht.lib:	gbmht.obj
		-del $@ 2> nul
		ilib /NOLOGO $@ $**;

gbmht.obj:	gbmht.c

#

gbmhist.lib:	gbmhist.obj
		-del $@ 2> nul
		ilib /NOLOGO $@ $**;

gbmhist.obj:	gbmhist.c

#

gbmmcut.lib:	gbmmcut.obj
		-del $@ 2> nul
		ilib /NOLOGO $@ $**;

gbmmcut.obj:	gbmmcut.c

#

gbmbpp.exe:	gbmbpp.obj gbm.lib gbmerr.lib gbmtrunc.lib gbmht.lib gbmhist.lib gbmmcut.lib
		ilink $(LFLAGS_EXE) /OUT:$@ $**

gbmbpp.obj:	gbmbpp.c gbm.h gbmerr.h gbmtrunc.h gbmht.h gbmhist.h gbmmcut.h

#

gbmcpal.exe:	gbmcpal.obj gbm.lib gbmhist.lib gbmmcut.lib
		ilink $(LFLAGS_EXE) /OUT:$@ $**

gbmcpal.obj:	gbmcpal.c gbm.h gbmhist.h gbmmcut.h

#

gbmgamma.exe:	gbmgamma.obj gbm.lib
		ilink $(LFLAGS_EXE) /OUT:$@ $**

gbmgamma.obj:	gbmgamma.c gbm.h

#

clean:
		-del *.obj 2> nul

#

IDIR =		d:\gbm

install:
		copy *.exe $(IDIR)
		copy *.dll $(IDIR)

#

package:
		zip -q gbmos2 gbm.htm gbm.h gbm.lib gbm.dll *.exe
