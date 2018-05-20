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

import java.awt.geom.GeneralPath;

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
	int mode=0;

  public ContourClass ( ArrayList<double[]> stroke, String color_string, boolean closed ) {
		stroke_points = stroke;
		this.closed = closed;
  	// System.out.println ( "Setting color to " + color_string );
  	String color_part_strings[] = color_string.trim().split(" ");
  	try { r = Double.parseDouble ( color_part_strings[0].trim() ); } catch (Exception e) { r = 0.5; }
  	try { g = Double.parseDouble ( color_part_strings[1].trim() ); } catch (Exception e) { g = 0.5; }
  	try { b = Double.parseDouble ( color_part_strings[2].trim() ); } catch (Exception e) { b = 0.5; }
  }

	public void draw_scaled_line ( Graphics g, Reconstruct r, int xoffset, int yoffset, double x0, double y0, double x1, double y1 ) {
	  g.drawLine ( xoffset+r.x_to_pxi(x0),   yoffset+r.y_to_pyi(-y0),  xoffset+r.x_to_pxi(x1),  yoffset+r.y_to_pyi(-y1) );
	}

  public void draw ( Graphics g, Reconstruct r ) {
    Graphics2D g2 = (Graphics2D)g;

    if (stroke_points.size() > 0) {

	    double p0[] = null;
	    double p1[] = null;

	    int line_padding = r.line_padding;

	    if (line_padding >= 0) {

			  g.setColor ( new Color ( (int)(255*this.r), (int)(255*this.g), (int)(255*this.b) ) );

			  if (this.mode < 0) {

					// System.out.println ( "Fill this contour when mode == " + this.mode + " ?" );
					GeneralPath path = new GeneralPath();
					p0 = stroke_points.get(0);
					path.moveTo ( r.x_to_pxi(p0[0]), r.y_to_pyi(-p0[1]) );
					for (int j=1; j<stroke_points.size(); j++) {
						p0 = stroke_points.get(j);
						path.lineTo ( r.x_to_pxi(p0[0]), r.y_to_pyi(-p0[1]) );
					}
					if (closed) {
						path.closePath();
					}
					g2.fill ( path );

			  } else {

				  for (int xoffset=-line_padding; xoffset<=line_padding; xoffset++) {
				    for (int yoffset=-line_padding; yoffset<=line_padding; yoffset++) {
				      p0 = stroke_points.get(0);
				      for (int j=1; j<stroke_points.size(); j++) {
				        p1 = stroke_points.get(j);
				        draw_scaled_line ( g, r, xoffset, yoffset, p0[0], p0[1], p1[0], p1[1] );
				        p0 = new double[2];
				        p0[0] = p1[0];
				        p0[1] = p1[1];
				      }
				      if (closed) {
								p1 = stroke_points.get(0);
				        draw_scaled_line ( g, r, xoffset, yoffset, p0[0], p0[1], p1[0], p1[1] );
				      }
				    }
				  }
				}

			}

    }
  }


  public void clear_strokes() {
    stroke_points = new ArrayList<double[]>();
  }

  public void add_point (	double[] point ) {
    stroke_points.add ( point );
  }

	public void set_mode ( int mode ) {
		this.mode = mode;
	}

	static void priority_println ( int thresh, String s ) {
		if (thresh >= 90) {
			System.out.println ( s );
		}
	}

	public static void main ( String[] args ) {
		priority_println ( 50, "Testing ContourClass.java ..." );
	}

}

