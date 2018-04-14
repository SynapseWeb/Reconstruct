all: ZoomPanLib.class Reconstruct.jar ZoomPanDraw.jar sha1sum.txt

ZoomPanLib.class: ZoomPanLib.java makefile
	javac -nowarn -source 1.4 ZoomPanLib.java

XML_Parser.class: XML_Parser.java makefile
	javac -nowarn -source 1.5 XML_Parser.java

SeriesClass.class: SeriesClass.java makefile
	javac -nowarn -source 1.5 SeriesClass.java

SectionClass.class: SectionClass.java makefile
	javac -nowarn -source 1.5 SectionClass.java

Reconstruct.jar: Reconstruct.java ZoomPanLib.class XML_Parser.class SeriesClass.class SectionClass.class makefile
	javac Reconstruct.java
	jar -cfe Reconstruct.jar Reconstruct *.class *.java

ZoomPanDraw.jar: ZoomPanDraw.java ZoomPanLib.java makefile
	javac -nowarn -source 1.4 ZoomPanDraw.java
	jar -cfe ZoomPanDraw.jar ZoomPanDraw *.class *.java

sha1sum.txt: Reconstruct.jar
	sha1sum Reconstruct.jar > sha1sum.txt

clean:
	rm -f *.jar
	rm -f *.class
	rm -f *~

