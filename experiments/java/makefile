all: ZoomPanLib.class Reconstruct.jar ZoomPanDraw.jar

ZoomPanLib.class: ZoomPanLib.java makefile
	javac -nowarn -source 1.4 ZoomPanLib.java

Reconstruct.jar: Reconstruct.java ZoomPanLib.class XML_Parser.java SeriesClass.java SectionClass.java makefile
	javac Reconstruct.java
	jar -cfe Reconstruct.jar Reconstruct *.class *.java

ZoomPanDraw.jar: ZoomPanDraw.java ZoomPanLib.java makefile
	javac -nowarn -source 1.4 ZoomPanDraw.java
	jar -cfe ZoomPanDraw.jar ZoomPanDraw *.class *.java

clean:
	rm -f *.jar
	rm -f *.class
	rm -f *~

