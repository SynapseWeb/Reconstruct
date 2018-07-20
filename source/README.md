# Reconstruct 1.2.0.0

## This version has been modified to be compiled with MinGW 4.9.2
### This version may be compiled as follows:
1. Change directory (cd) to gbm/jpeg-6a
2. mingw32-make -f makeijgo
3. Change directory back to top level (cd ../..)
4. mingw32-make
### The help system is currently disabled.

-----------

Here at GitHub is the original Source code for Reconstruct - which was last compiled in 2007 and the
Executible is available at: https://sites.cns.utexas.edu/synapseweb/software-0
Software

Reconstruct Download

Reconstruct Version 1.1.0.0
 with Help Manual and example.ser (7.38MB)
 [August 20, 2007]


System Requirements:
Microsoft Windows (XP/Vista/7/8/10)
Pentium processor
10 Mb of disk space for program files
at least 128 Mb of RAM
Further reading:
Quick Start -- to get started using the software (User Manual Ch 1).
User Manual -- revised by Dr. Karin Sorra and Dr. Kristen Harris (2009).
Fiala JC (2005) Reconstruct: A free editor for serial section microscopy. J Microscopy.
Fiala JC, Harris KM (2001) Cylindrical diameters method for calibrating section thickness in serial electron microscopy. J Microscopy.
Reconstruct was written by John C. Fiala, Ph.D.
For more info go to Reconstruct History and Citations.
For help with specific questions, contact the Reconstruct Yahoo User's Group.

Reconstruct was created with funding, in part, from the National Institutes of Health and the Human Brain Project under grants (P30 HD18655, R01 MH/DA 57351, and R01 EB 002170).

Permission to use, copy, and redistribute Reconstruct is granted without fee under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.

(This software is distributed in the hope that it will be useful, but without any warranty, including the implied warranty of merchantibility or fitness for any particular purpose. For details read the terms of the GNU General Public License when installing the software.)

---------

Classes:

$ grep class *.h

classes.h:class EXCEPTION { }; // empty exception class for aborting operations
classes.h:template <class Element> class List { // template for doubly-linked, ordered list of Elements
classes.h:class XYPoint { // integer point list
classes.h:class XYPoints : public List<XYPoint> { }; // used in ADib.cpp for keeping track of zero equivalencies
classes.h:class Point { // 3D points in real coordinates
classes.h:class Points : public List<Point> {}; // a list of Points
classes.h:class Color {
classes.h:class Histogram {
classes.h:class ADib; // define temporary placeholders so can add pointers as needed
classes.h:class Nform;
classes.h:class Series;
classes.h:class Contour { // the actual contour definition
classes.h:class Contours : public List<Contour> {
classes.h:class Image {
classes.h:class Mvmt { // set of movement parameters for user input
classes.h:class Nform { // nonlinear transform by linear comb. of 2nd order poly.
classes.h:class Transform {
classes.h:class Transforms : public List<Transform> { };
classes.h:class TformsLIFO { // a limited LIFO for undos of transform lists
classes.h:class ADib {
classes.h:class Section {
classes.h:class Sections : public List<Section> { }; // for undo stacks when editing sections
classes.h:class SectionInfo {
classes.h:class SectionsInfo : public List<SectionInfo> {// for list of sections in series
classes.h:class ViewPort {
classes.h:class Series {
classes.h:class XML_DATA { // Data Object for reading and parsing XML
classes.h:class Button { // class for bitmapped buttons
classes.h:class Buttons : public List<Button> { }; // list of buttons
classes.h:class ZoomState {// structure for holding zoom params
classes.h:class VertexSet {// class for point coordinates of object
classes.h:class Line { // a line segment
classes.h:class LineSet { // an array of indexed line segments
classes.h:class Face { // a trianglular face class
classes.h:class FaceSet { // an array of indexed faces
classes.h:class VRMLObject { // a VRML-like 3D mesh object
classes.h:class VRMLObjects : public List<VRMLObject> { // a list of 3D objects
classes.h:class VRMLScene {// current 3D scene
classes.h:class InterObject { // relationships between objects
classes.h: InterObject *prev, *next; // all list element class need these
classes.h:class InterObjects : public List<InterObject> { }; // list of relationships
reconstruct.h:template <class T> const T& max (const T& a, const T& b) {
reconstruct.h:template <class T> const T& min (const T& a, const T& b) {
