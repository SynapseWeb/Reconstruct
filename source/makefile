# makefile (from Borland C++ 5.5 to MinGW) for Reconstruct application

EXEFILE  = reconstruct.exe
HDRFILES = constants.h gbm.h classes.h operations.h globals.h reconstruct.h kube-gustavson-fft.h
SRCFILES = contour.cpp nform.cpp adib.cpp image.cpp xmldata.cpp viewport.cpp section.cpp series.cpp kube-gustavson-fft.cpp\
 vrmlobject.cpp vrmlobjects.cpp vrmlscene.cpp boissonnat.cpp sectionsinfo.cpp tformslifo.cpp threads.cpp palette.cpp\
 program_menu.cpp series_menu.cpp section_menu.cpp domain_menu.cpp trace_menu.cpp objects_menu.cpp help_menu.cpp\
 init.cpp end.cpp utility.cpp tools.cpp thumbnails.cpp keyboard.cpp mouse.cpp opengl.cpp globals.cpp reconstruct.cpp histogram.cpp
OBJFILES = contour.obj nform.obj adib.obj image.obj xmldata.obj viewport.obj section.obj series.obj kube-gustavson-fft.obj\
 vrmlobject.obj vrmlobjects.obj vrmlscene.obj boissonnat.obj sectionsinfo.obj tformslifo.obj threads.obj palette.obj\
 program_menu.obj series_menu.obj section_menu.obj domain_menu.obj trace_menu.obj objects_menu.obj help_menu.obj\
 init.obj end.obj utility.obj tools.obj thumbnails.obj keyboard.obj mouse.obj opengl.obj globals.obj reconstruct.obj histogram.obj

RCFILES = reconstruct.rc
RESFILES = reconstruct.res
LIBFILES = gbm.lib

# the following definitions serve only to document all of files used by the application
ICONS = large.ico section.ico small.ico
CURSORS = cultiline.cur domain.cur ellipse.cur eraser.cur line.cur magnify.cur multiline.cur pencil.cur point.cur\
 rectangle.cur xy.cur zoom.cur
BITMAPS = arrow.bmp cultiline.bmp domain.bmp ellipse.bmp exclamation.bmp line.bmp magnify.bmp multiline.bmp pencil.bmp point.bmp\
 reconstruct.bmp rectangle.bmp xyzoom.bmp
HELPFILES = manual.chm keycmds.chm mousecmds.chm

# Define environment variables RECONSTRUCT_COMPILER, _LINKER, _RC, _LIB, _INCLUDE
# to specify the location on your system of these files (as a quoted string),
# e.g. define RECONSTRUCT_COMPILER="C:\Program Files\Borland\Bcc55\bin\bcc32" etc.
# via Control Panel->System->Advanced->Environment Variables on a Windows XP machine
# (You may need to logout before your environment variables will take effect.)

CFLAGS  = -c -tWM -tW -O2 -H -I$(RECONSTRUCT_INCLUDE) -L$(RECONSTRUCT_LIB) -DWINVER=0x0400 -D_WIN32_WINNT=0x0400
# -c   compile only, do not link
# -tWM multithreading
# -tW  Windows application
# -Od  no optimizations, for debug but in final use -O2
# -H   generate and use precompiled headers
# -I   location of include files
# -L   location of libraries
# -D   override value in Windows API include files to make compatible with NT 4.0

LFLAGS  = -aa -c -x -Gn -L$(RECONSTRUCT_LIB)
# -aa  Win32 application
# -c   case sensitive symbols
# -x   suppress map file creation
# -Gn  disable incremental linking
# -L   location of libraries

RFLAGS  = /r /nw
# for GoRC Resource Compiler:
# /r	generate .res output
# /nw	suppress warning messages
# RFLAGS  = -r -x -32
# -r   do not link resources
# -x   ignore INCLUDE environment variable
# -32  build 32-bit resources

STDOBJS = c0w32.obj
# c0w32.obj   startup code for Win32 GUI .exe

STDLIBS = import32.lib cw32mt.lib htmlhelp.lib
# import32.lib   Windows API 
# cw32mt.lib	 static multithreaded run-time libraries
# htmlhelp.lib created in the BCC55\LIB folder by using the command:
#    ..\bin\implib htmlhelp.lib C:\Windows\System32\hhctrl.ocx


# primary target is linking .exe application, dependent on .obj and .res
$(EXEFILE) : $(OBJFILES) $(RESFILES)
   $(RECONSTRUCT_LINKER) $(LFLAGS) $(OBJFILES) $(STDOBJS), $(EXEFILE), , $(LIBFILES) $(STDLIBS), , $(RESFILES)

# if any source files are newer than header files, recompile all sources
$(OBJFILES) : $(HDRFILES)
#  echo Using $(RECONSTRUCT_COMPILER) to compile
   $(RECONSTRUCT_COMPILER) $(CFLAGS) $(SRCFILES)

# if any source file is newer than its object file, recompile just that one source
.cpp.obj :
   $(RECONSTRUCT_COMPILER) $(CFLAGS) $(SRCFILES)

# if any .rc or header files are newer than .res files, recompile resources
$(RESFILES) : $(RCFILES) $(HDRFILES)
   $(RECONSTRUCT_RC) $(RFLAGS) $(RCFILES)

clean :
   del *.obj *.res *.tds *.bak
