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

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after sha1sum.txt

sha1sum.txt: $(BIN)
	sha1sum Reconstruct.exe > sha1sum.txt

clean: clean-custom
	${RM} $(OBJ) $(BIN) $(GBMLIB)
	${RM} sha1sum.txt
	${RM} gbm/*.o
# */  Just a way to end the "comment" for broken syntax highlighting editors

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

$(GBMLIB): subdirs

#$(GBMLIB): $(GBMOBJ) subdirs
#	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

$(BIN): $(OBJ) $(GBMLIB) makefile
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS) $(GBMLIB)

adib.o: adib.cpp
	$(CPP) -c adib.cpp -o adib.o $(CXXFLAGS)

boissonnat.o: boissonnat.cpp
	$(CPP) -c boissonnat.cpp -o boissonnat.o $(CXXFLAGS)

contour.o: contour.cpp
	$(CPP) -c contour.cpp -o contour.o $(CXXFLAGS)

domain_menu.o: domain_menu.cpp
	$(CPP) -c domain_menu.cpp -o domain_menu.o $(CXXFLAGS)

end.o: end.cpp
	$(CPP) -c end.cpp -o end.o $(CXXFLAGS)

globals.o: globals.cpp
	$(CPP) -c globals.cpp -o globals.o $(CXXFLAGS)

help_menu.o: help_menu.cpp
	$(CPP) -c help_menu.cpp -o help_menu.o $(CXXFLAGS)

histogram.o: histogram.cpp
	$(CPP) -c histogram.cpp -o histogram.o $(CXXFLAGS)

image.o: image.cpp
	$(CPP) -c image.cpp -o image.o $(CXXFLAGS)

init.o: init.cpp
	$(CPP) -c init.cpp -o init.o $(CXXFLAGS)

keyboard.o: keyboard.cpp
	$(CPP) -c keyboard.cpp -o keyboard.o $(CXXFLAGS)

kube-gustavson-fft.o: kube-gustavson-fft.cpp
	$(CPP) -c kube-gustavson-fft.cpp -o kube-gustavson-fft.o $(CXXFLAGS)

mouse.o: mouse.cpp
	$(CPP) -c mouse.cpp -o mouse.o $(CXXFLAGS)

nform.o: nform.cpp
	$(CPP) -c nform.cpp -o nform.o $(CXXFLAGS)

objects_menu.o: objects_menu.cpp
	$(CPP) -c objects_menu.cpp -o objects_menu.o $(CXXFLAGS)

opengl.o: opengl.cpp
	$(CPP) -c opengl.cpp -o opengl.o $(CXXFLAGS)

palette.o: palette.cpp
	$(CPP) -c palette.cpp -o palette.o $(CXXFLAGS)

program_menu.o: program_menu.cpp
	$(CPP) -c program_menu.cpp -o program_menu.o $(CXXFLAGS)

reconstruct.o: reconstruct.cpp
	$(CPP) -c reconstruct.cpp -o reconstruct.o $(CXXFLAGS)

section.o: section.cpp
	$(CPP) -c section.cpp -o section.o $(CXXFLAGS)

section_menu.o: section_menu.cpp
	$(CPP) -c section_menu.cpp -o section_menu.o $(CXXFLAGS)

sectionsinfo.o: sectionsinfo.cpp
	$(CPP) -c sectionsinfo.cpp -o sectionsinfo.o $(CXXFLAGS)

series.o: series.cpp
	$(CPP) -c series.cpp -o series.o $(CXXFLAGS)

series_menu.o: series_menu.cpp
	$(CPP) -c series_menu.cpp -o series_menu.o $(CXXFLAGS)

tformslifo.o: tformslifo.cpp
	$(CPP) -c tformslifo.cpp -o tformslifo.o $(CXXFLAGS)

threads.o: threads.cpp
	$(CPP) -c threads.cpp -o threads.o $(CXXFLAGS)

thumbnails.o: thumbnails.cpp
	$(CPP) -c thumbnails.cpp -o thumbnails.o $(CXXFLAGS)

tools.o: tools.cpp
	$(CPP) -c tools.cpp -o tools.o $(CXXFLAGS)

trace_menu.o: trace_menu.cpp
	$(CPP) -c trace_menu.cpp -o trace_menu.o $(CXXFLAGS)

utility.o: utility.cpp
	$(CPP) -c utility.cpp -o utility.o $(CXXFLAGS)

viewport.o: viewport.cpp
	$(CPP) -c viewport.cpp -o viewport.o $(CXXFLAGS)

vrmlobject.o: vrmlobject.cpp
	$(CPP) -c vrmlobject.cpp -o vrmlobject.o $(CXXFLAGS)

vrmlobjects.o: vrmlobjects.cpp
	$(CPP) -c vrmlobjects.cpp -o vrmlobjects.o $(CXXFLAGS)

vrmlscene.o: vrmlscene.cpp
	$(CPP) -c vrmlscene.cpp -o vrmlscene.o $(CXXFLAGS)

xmldata.o: xmldata.cpp
	$(CPP) -c xmldata.cpp -o xmldata.o $(CXXFLAGS)

reconstruct.res: reconstruct.rc
	$(WINDRES) -i reconstruct.rc --input-format=rc -o reconstruct.res -O coff
