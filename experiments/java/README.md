# Reconstruct Java Experiments

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

## Early Java prototype of basic functionality (February 27th, 2018):

![Early Demo](Screenshot_02272018_105320PM.png?raw=true "Early Demo")

This version uses the familiar menus from the earlier Reconstruct. Most of the menus are non-functional.

This version supports importing a series file with the **Series/Open...** option.

This version supports switching between **Move Mode** and **Draw Mode** via the space bar or right mouse button. These modes are:

* **Move Mode:** The image can be zoomed with the mouse scrollwheel and panned with mouse click and drag.
* **Draw Mode:** The series can be viewed with the scrollwheel. Multiple outlines can be drawn with click and drag.

## Use:

* Obtain a copy of **Reconstruct.jar** (either by downloading, or by building with make).
* Double click on Reconstruct.jar (or run it with: java -jar Reconstruct.jar).
* Use the menu option "**Series / Open...**" to open an existing series file (.ser).
* The mouse wheel will zoom in and out. The image will zoom about the mouse location (that point remains fixed).
* The default mode will be **Move Mode** (cursor will show 4 arrows). Click and drag to move the image.
* The mode can be toggled between **Move Mode** and **Draw Mode** with the space bar or right mouse button (the mouse will change shape).
* Traces can be drawn with **Draw Mode** ("crosshair" cursor).
* Current traces are single segments with no editing capabilities.
* All traces on each section can be printed to parent console with "**Program / Debug / Dump**".
* All traces on each section can be cleared with "**Program / Debug / Clear**".
* The "Mode" menu item can also be used to switch modes, print traces, and clear traces.
* The "Center Drawing" mode acts like a sewing machine or jigsaw where the "work" is moved
  under the center drawing point (cross hair).

