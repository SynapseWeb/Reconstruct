/* This Class represents a Reconstruct Section. */

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.util.Set;
import java.util.Arrays;

import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

import java.io.*;

import java.awt.image.*;
import javax.imageio.ImageIO;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;


public class ContourClass {

  String contour_name = null;
	ArrayList<double[]> stroke_points = new ArrayList<double[]>();  // Argument (if any) specifies initial capacity (default 10)
	boolean closed = true;
	double r=1.0, g=0.0, b=0.0;

  public ContourClass ( ArrayList<double[]> stroke, String color_string, boolean closed ) {
		stroke_points = stroke;
		this.closed = closed;
  	// System.out.println ( "Setting color to " + color_string );
  	String color_part_strings[] = color_string.trim().split(" ");
  	try { r = Double.parseDouble ( color_part_strings[0].trim() ); } catch (Exception e) { r = 0.5; }
  	try { g = Double.parseDouble ( color_part_strings[1].trim() ); } catch (Exception e) { g = 0.5; }
  	try { b = Double.parseDouble ( color_part_strings[2].trim() ); } catch (Exception e) { b = 0.5; }
  }

	static void priority_println ( int thresh, String s ) {
		if (thresh >= 90) {
			System.out.println ( s );
		}
	}

  public void clear_strokes() {
    stroke_points = new ArrayList<double[]>();
  }

  public void add_point (	double[] point ) {
    stroke_points.add ( point );
  }


	public static void main ( String[] args ) {
		priority_println ( 50, "Testing ContourClass.java ..." );
	}

}

