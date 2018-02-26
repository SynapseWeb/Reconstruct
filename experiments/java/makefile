all: ZoomPanLib.class ZoomPanDraw.jar

ZoomPanLib.class: ZoomPanLib.java makefile
	javac ZoomPanLib.java

ZoomPanDraw.jar: ZoomPanDraw.java ZoomPanLib.class makefile
	javac ZoomPanDraw.java
	jar -cvfe ZoomPanDraw.jar ZoomPanDraw *.class *.java

clean:
	rm -f *.jar
	mv ZoomPanLib.class ZoomPanLib.class_saved
	rm -f *.class
	mv ZoomPanLib.class_saved ZoomPanLib.class
	rm -f *~

