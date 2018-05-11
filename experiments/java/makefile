all: ZoomPanLib.class Reconstruct.jar BezierTracing.jar ZoomPanDraw.jar sha1sum.txt

ZoomPanLib.class: ZoomPanLib.java makefile
	javac -nowarn -source 1.4 ZoomPanLib.java

XML_Parser.class: XML_Parser.java makefile
	javac -nowarn -source 1.5 XML_Parser.java

SeriesClass.class: SeriesClass.java makefile
	javac -nowarn -source 1.5 SeriesClass.java

SectionClass.class: SectionClass.java makefile
	javac -nowarn -source 1.5 SectionClass.java

ReconstructDefaults.class: ReconstructDefaults.java makefile
	javac -nowarn -source 1.5 ReconstructDefaults.java

Reconstruct.jar: Reconstruct.java ZoomPanLib.class XML_Parser.class SeriesClass.class SectionClass.class ReconstructDefaults.class makefile
	javac Reconstruct.java
	jar -cfe Reconstruct.jar Reconstruct *.class

ZoomPanDraw.jar: ZoomPanDraw.java ZoomPanLib.java makefile
	javac -nowarn -source 1.4 ZoomPanDraw.java
	jar -cfe ZoomPanDraw.jar ZoomPanDraw *.class

BezierTracing.jar: BezierTracing.java makefile
	rm -f *.class
	javac -nowarn -source 1.2 BezierTracing.java
	jar -cfe BezierTracing.jar BezierTracing *.class

sha1sum.txt: Reconstruct.jar BezierTracing.jar
	sha1sum Reconstruct.jar > sha1sum.txt
	sha1sum BezierTracing.jar >> sha1sum.txt

clean:
	rm -f *.jar
	rm -f *.class
	rm -f *~

