# Reconstruct Java Experiments

## Reconstruct.jar can write XML data compatible with Reconstruct.exe (June 20th, 2018)

![SeriesFromJavaAndExe](images/2018_06_20_2004.gif?raw=true "Series with objects traced in both Java and Exe Versions")

## Reconstruct.jar can read XML data and display images and color traces (May 11th, 2018):

![ColorTracesFromXML](images/2018_05_11_2303.gif?raw=true "Color Traces from XML")

This image shows a short series of traces (with color) viewed with the current Reconstruct.jar.

## Comparison of Reconstruct.jar and Reconstruct.exe (May 11th, 2018):

![CompareJavaExe](Compare_Java_Exe_2018_05_11.gif?raw=true "Compare Java and Original Exe")

This image shows a snapshot from the Windows version of Reconstruct 1.1.0.0 and a snapshot of
the current Java version (as of this commit). The snapshots were taken from two different
versions (1.1.0.0 and Java) running on two different computers (Windows and Linux) using the
same image files and XML files. The image snapshots were not perfectly registered (but close).
The Java version shows traces as wide red strokes. The 1.1.0.0 version shows the same traces
as thin multi-colored strokes. The images were scaled down for size and this reduced the already
thin multi-colored strokes. Note that this Java version used manual trace scaling to verify that
the XML data is being correctly read. The application of the XML transforms has yet to be completed
in the code.

## Prototype Bezier Tracing Interface with BezierTracing.jar (April 16th, 2018):

![Bezier Tracing](Trace_Round.gif?raw=true "Bezier Tracing")

## Use:

* Obtain a copy of **BezierTracing.jar** (either by downloading, or by building with make).
* Double click on BezierTracing.jar (or run it with: java -jar BezierTracing.jar).
* Start a trace by **RIGHT** clicking. A white dot should appear as your starting point.
* Move (not drag) the mouse to the next point and **LEFT** click to add the second point.
* With only two points, you'll get a line with two endpoints and two "handles" between them.
* Again move (not drag) the mouse to the next point and click again. Another segment will appear.
* **NOTE:** Each new point (click) will affect the previous segment. That's OK.
* Complete the trace with a **RIGHT** click on either the start point or the end point (see following).
* If you complete a trace by **RIGHT** clicking on the first (start) point, the trace will be closed to that point.
* If you complete a trace by **RIGHT** clicking on the last (end) point, the trace will remain open.

## Typical workflow:

#### Creating a trace:

* Right click to start a trace (white dot appears).
* Left click to add sequential points (no dragging needed).
* Right click to end a trace.

#### Modifying a trace:

* When not creating a trace, the left button performs "click and drag" on all points and handles.

![Bezier Point Adjustment](Bezier_Point_Adjustment.gif?raw=true "Bezier Point Adjustment")

![Bezier Slope Adjustment](Single_Cubic_Bezier_Control.gif?raw=true "Bezier Slope Adjustment")


#### Comparison with Polygon Tool:

![Bezier v Polygon](Bezier_Polygon_Image.gif?raw=true "Bezier v Polygon")
