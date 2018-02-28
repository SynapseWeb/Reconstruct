all: Reconstruct.jar ZoomPanDraw.jar

Reconstruct.jar: Reconstruct.java ZoomPanLib.java makefile
	javac Reconstruct.java
	jar -cvfe Reconstruct.jar Reconstruct *.class *.java

ZoomPanDraw.jar: ZoomPanDraw.java ZoomPanLib.java makefile
	javac ZoomPanDraw.java
	jar -cvfe ZoomPanDraw.jar ZoomPanDraw *.class *.java

clean:
	rm -f *.jar
	rm -f *.class
	rm -f *~

