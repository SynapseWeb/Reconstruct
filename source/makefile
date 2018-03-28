# Project: Reconstruct
# Makefile created by Dev-C++ 5.11 and modified manually

SUBDIRS = gbm

CPP      = g++.exe
CC       = gcc.exe
WINDRES  = windres.exe
RES      = reconstruct.res
# GBMOBJ   = gbm.o gbmbmp.o gbmcvp.o gbmgem.o gbmgif.o gbmhelp.o gbmiax.o gbmjpg.o gbmkps.o gbmlbm.o gbmpcx.o gbmpgm.o gbmppm.o gbmpsg.o gbmspr.o gbmtga.o gbmtif.o gbmtifh.o gbmvid.o gbmxbm.o
GBMLIB   = gbm/gbm.a
OBJ      = adib.o boissonnat.o contour.o domain_menu.o end.o globals.o help_menu.o histogram.o image.o init.o keyboard.o kube-gustavson-fft.o mouse.o nform.o objects_menu.o opengl.o palette.o program_menu.o reconstruct.o section.o section_menu.o sectionsinfo.o series.o series_menu.o tformslifo.o threads.o thumbnails.o tools.o trace_menu.o utility.o viewport.o vrmlobject.o vrmlobjects.o vrmlscene.o xmldata.o $(RES)
LINKOBJ  = adib.o boissonnat.o contour.o domain_menu.o end.o globals.o help_menu.o histogram.o image.o init.o keyboard.o kube-gustavson-fft.o mouse.o nform.o objects_menu.o opengl.o palette.o program_menu.o reconstruct.o section.o section_menu.o sectionsinfo.o series.o series_menu.o tformslifo.o threads.o thumbnails.o tools.o trace_menu.o utility.o viewport.o vrmlobject.o vrmlobjects.o vrmlscene.o xmldata.o $(RES)
LIBS     = -L"C:/devcpp/MinGW64/lib" -L"C:/devcpp/MinGW64/x86_64-w64-mingw32/lib" -static-libgcc -mwindows -lopengl32 "C:/devcpp/MinGW64/x86_64-w64-mingw32/lib/libcomctl32.a" "C:/devcpp/MinGW64/x86_64-w64-mingw32/lib/libversion.a" "htmlhelp.lib"
INCS     = -I"C:/devcpp/MinGW64/include" -I"C:/devcpp/MinGW64/x86_64-w64-mingw32/include" -I"C:/devcpp/MinGW64/lib/gcc/x86_64-w64-mingw32/4.9.2/include"
CXXINCS  = -I"C:/devcpp/MinGW64/include" -I"C:/devcpp/MinGW64/x86_64-w64-mingw32/include" -I"C:/devcpp/MinGW64/lib/gcc/x86_64-w64-mingw32/4.9.2/include" -I"C:/devcpp/MinGW64/lib/gcc/x86_64-w64-mingw32/4.9.2/include/c++"
BIN      = Reconstruct.exe
CXXFLAGS = $(CXXINCS) -w -fpermissive
CFLAGS   = $(INCS) -w -fpermissive
RM       = rm.exe -f

.PHONY: all all-before all-after clean clean-custom makefile

all: all-before $(BIN) all-after sha1sum.txt makefile

sha1sum.txt: $(BIN)
	sha1sum Reconstruct.exe > sha1sum.txt

clean: clean-custom
	${RM} $(OBJ) $(BIN) $(GBMLIB)
	${RM} sha1sum.txt
	${RM} gbm/*.o
# */  Just a way to end the "comment" for broken syntax highlighting editors

.PHONY: subdirs $(SUBDIRS)
subdirs: makefile $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

$(GBMLIB): subdirs

#$(GBMLIB): $(GBMOBJ) subdirs
#	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

$(BIN): $(OBJ) $(GBMLIB) makefile
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS) $(GBMLIB)

adib.o: adib.cpp makefile
	$(CPP) -c adib.cpp -o adib.o $(CXXFLAGS)

boissonnat.o: boissonnat.cpp makefile
	$(CPP) -c boissonnat.cpp -o boissonnat.o $(CXXFLAGS)

contour.o: contour.cpp makefile
	$(CPP) -c contour.cpp -o contour.o $(CXXFLAGS)

domain_menu.o: domain_menu.cpp makefile
	$(CPP) -c domain_menu.cpp -o domain_menu.o $(CXXFLAGS)

end.o: end.cpp makefile
	$(CPP) -c end.cpp -o end.o $(CXXFLAGS)

globals.o: globals.cpp makefile
	$(CPP) -c globals.cpp -o globals.o $(CXXFLAGS)

help_menu.o: help_menu.cpp makefile
	$(CPP) -c help_menu.cpp -o help_menu.o $(CXXFLAGS)

histogram.o: histogram.cpp makefile
	$(CPP) -c histogram.cpp -o histogram.o $(CXXFLAGS)

image.o: image.cpp makefile
	$(CPP) -c image.cpp -o image.o $(CXXFLAGS)

init.o: init.cpp makefile
	$(CPP) -c init.cpp -o init.o $(CXXFLAGS)

keyboard.o: keyboard.cpp makefile
	$(CPP) -c keyboard.cpp -o keyboard.o $(CXXFLAGS)

kube-gustavson-fft.o: kube-gustavson-fft.cpp makefile
	$(CPP) -c kube-gustavson-fft.cpp -o kube-gustavson-fft.o $(CXXFLAGS)

mouse.o: mouse.cpp makefile
	$(CPP) -c mouse.cpp -o mouse.o $(CXXFLAGS)

nform.o: nform.cpp makefile
	$(CPP) -c nform.cpp -o nform.o $(CXXFLAGS)

objects_menu.o: objects_menu.cpp makefile
	$(CPP) -c objects_menu.cpp -o objects_menu.o $(CXXFLAGS)

opengl.o: opengl.cpp makefile
	$(CPP) -c opengl.cpp -o opengl.o $(CXXFLAGS)

palette.o: palette.cpp makefile
	$(CPP) -c palette.cpp -o palette.o $(CXXFLAGS)

program_menu.o: program_menu.cpp makefile
	$(CPP) -c program_menu.cpp -o program_menu.o $(CXXFLAGS)

reconstruct.o: reconstruct.cpp makefile
	$(CPP) -c reconstruct.cpp -o reconstruct.o $(CXXFLAGS)

section.o: section.cpp makefile
	$(CPP) -c section.cpp -o section.o $(CXXFLAGS)

section_menu.o: section_menu.cpp makefile
	$(CPP) -c section_menu.cpp -o section_menu.o $(CXXFLAGS)

sectionsinfo.o: sectionsinfo.cpp makefile
	$(CPP) -c sectionsinfo.cpp -o sectionsinfo.o $(CXXFLAGS)

series.o: series.cpp makefile
	$(CPP) -c series.cpp -o series.o $(CXXFLAGS)

series_menu.o: series_menu.cpp makefile
	$(CPP) -c series_menu.cpp -o series_menu.o $(CXXFLAGS)

tformslifo.o: tformslifo.cpp makefile
	$(CPP) -c tformslifo.cpp -o tformslifo.o $(CXXFLAGS)

threads.o: threads.cpp makefile
	$(CPP) -c threads.cpp -o threads.o $(CXXFLAGS)

thumbnails.o: thumbnails.cpp makefile
	$(CPP) -c thumbnails.cpp -o thumbnails.o $(CXXFLAGS)

tools.o: tools.cpp makefile
	$(CPP) -c tools.cpp -o tools.o $(CXXFLAGS)

trace_menu.o: trace_menu.cpp makefile
	$(CPP) -c trace_menu.cpp -o trace_menu.o $(CXXFLAGS)

utility.o: utility.cpp makefile
	$(CPP) -c utility.cpp -o utility.o $(CXXFLAGS)

viewport.o: viewport.cpp makefile
	$(CPP) -c viewport.cpp -o viewport.o $(CXXFLAGS)

vrmlobject.o: vrmlobject.cpp makefile
	$(CPP) -c vrmlobject.cpp -o vrmlobject.o $(CXXFLAGS)

vrmlobjects.o: vrmlobjects.cpp makefile
	$(CPP) -c vrmlobjects.cpp -o vrmlobjects.o $(CXXFLAGS)

vrmlscene.o: vrmlscene.cpp makefile
	$(CPP) -c vrmlscene.cpp -o vrmlscene.o $(CXXFLAGS)

xmldata.o: xmldata.cpp makefile
	$(CPP) -c xmldata.cpp -o xmldata.o $(CXXFLAGS)

reconstruct.res: reconstruct_min_ok.rc makefile
	$(WINDRES) -i reconstruct_min_ok.rc --input-format=rc -o reconstruct.res -O coff
