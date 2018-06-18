all: ZoomPanLib.class Reconstruct.jar BezierTracing.jar ZoomPanDraw.jar sha1sum.txt

ZoomPanLib.class: ZoomPanLib.java makefile
	javac -nowarn -source 1.6 ZoomPanLib.java

XML_Parser.class: XML_Parser.java makefile
	javac -nowarn -source 1.6 XML_Parser.java

TransformClass.class: TransformClass.java makefile
	javac -nowarn -source 1.6 TransformClass.java

SeriesClass.class: SeriesClass.java makefile
	javac -nowarn -source 1.6 SeriesClass.java

SectionClass.class: SectionClass.java makefile
	javac -nowarn -source 1.6 SectionClass.java

ContourClass.class: ContourClass.java makefile
	javac -nowarn -source 1.6 ContourClass.java

ReconstructDefaults.class: ReconstructDefaults.java makefile
	javac -nowarn -source 1.6 ReconstructDefaults.java

Reconstruct.jar: Reconstruct.java ZoomPanLib.class XML_Parser.class TransformClass.class SeriesClass.class SectionClass.class ContourClass.class ReconstructDefaults.class makefile
	javac Reconstruct.java
	jar -cfe Reconstruct.jar Reconstruct *.class

ZoomPanDraw.jar: ZoomPanDraw.java ZoomPanLib.java makefile
	javac -nowarn -source 1.6 ZoomPanDraw.java
	jar -cfe ZoomPanDraw.jar ZoomPanDraw *.class

BezierTracing.jar: BezierTracing.java makefile
	rm -f *.class
	javac -nowarn -source 1.6 BezierTracing.java
	jar -cfe BezierTracing.jar BezierTracing *.class

sha1sum.txt: Reconstruct.jar BezierTracing.jar
	sha1sum Reconstruct.jar > sha1sum.txt
	sha1sum BezierTracing.jar >> sha1sum.txt

clean:
	rm -f *.jar
	rm -f *.class
	rm -f *~

