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

import java.awt.geom.*;


public class ContourClass {

  String contour_name = null;
	ArrayList<double[]> stroke_points = new ArrayList<double[]>();  // Argument (if any) specifies initial capacity (default 10)
	TransformClass xform = null;
	boolean closed = true;
	boolean hidden = false;
	double r=1.0, g=0.0, b=0.0;
	int mode=0;
	public boolean is_bezier=false;

  public ContourClass ( ArrayList<double[]> stroke, String color_string, boolean closed ) {
		stroke_points = stroke;
		this.closed = closed;
  	// System.out.println ( "Setting color to " + color_string );
  	String color_part_strings[] = color_string.trim().split(" ");
  	try { r = Double.parseDouble ( color_part_strings[0].trim() ); } catch (Exception e) { r = 0.5; }
  	try { g = Double.parseDouble ( color_part_strings[1].trim() ); } catch (Exception e) { g = 0.5; }
  	try { b = Double.parseDouble ( color_part_strings[2].trim() ); } catch (Exception e) { b = 0.5; }
  }

  public ContourClass ( ArrayList<double[]> stroke, int trace_color, boolean closed ) {
		stroke_points = stroke;
		this.closed = closed;
		r = ( (trace_color & 0x00ff0000) >> 16 ) / 255.0;
		g = ( (trace_color & 0x0000ff00) >>  8 ) / 255.0;
		b = ( (trace_color & 0x000000ff)       ) / 255.0;
  }

	public void draw_scaled_line ( Graphics g, Reconstruct r, int xoffset, int yoffset, double x0, double y0, double x1, double y1 ) {
	  g.drawLine ( xoffset+r.x_to_pxi(x0),   yoffset+r.y_to_pyi(-y0),  xoffset+r.x_to_pxi(x1),  yoffset+r.y_to_pyi(-y1) );
	}

	double [] translate_to_screen ( double p[], Reconstruct r ) {
		double[] t = new double[2];
		double dx=0;
		double dy=0;
		if (this.xform != null) {
			if (this.xform.dim > 0) {
				dx = this.xform.xcoef[0];
				dy = this.xform.ycoef[0];
			}
		}
		t[0] = r.x_to_pxi(p[0]-dx);
		t[1] = r.y_to_pyi(dy-p[1]);
		return ( t );
	}

	CubicCurve2D.Double default_curve ( double p0[], double p1[] ) {
    double mid_x, mid_y;
    mid_x = p0[0] + ((p1[0]-p0[0])/3);
    mid_y = p0[1] + ((p1[1]-p0[1])/3);
    double h0[] = { mid_x, mid_y };
    //this.h0 = new CurvePoint ( mid_x, mid_y );
    mid_x = p0[0] + (2*(p1[0]-p0[0])/3);
    mid_y = p0[1] + (2*(p1[1]-p0[1])/3);
    double h1[] = { mid_x, mid_y };
    //this.h1 = new CurvePoint ( mid_x, mid_y );

		return ( new CubicCurve2D.Double ( p0[0], p0[1], h0[0], h0[1], h1[0], h1[1], p1[0], p1[1] ) );
	}

  public void draw ( Graphics g, Reconstruct r ) {

		if ( !hidden ) {
			Graphics2D g2 = (Graphics2D)g;
			double dx=0;
			double dy=0;
			if (this.xform != null) {
				if (this.xform.dim > 0) {
					dx = this.xform.xcoef[0];
					dy = this.xform.ycoef[0];
				}
			}

			if (stroke_points.size() > 0) {

				double p0[] = null;
				double p1[] = null;

				int line_padding = r.line_padding;

				if (line_padding >= 0) {

					g.setColor ( new Color ( (int)(255*this.r), (int)(255*this.g), (int)(255*this.b) ) );

					if (is_bezier) {
						double factor = 0.2;

						ArrayList<CubicCurve2D.Double> curves = new ArrayList<CubicCurve2D.Double>();  // Argument (if any) specifies initial capacity (default 10)

						p0 = translate_to_screen ( stroke_points.get(0), r );

						// path.moveTo ( r.x_to_pxi(p0[0]-dx), r.y_to_pyi(dy-p0[1]) );
						for (int j=1; j<stroke_points.size(); j++) {
							p1 = translate_to_screen ( stroke_points.get(j), r );
							curves.add ( default_curve ( p0, p1 ) );
							p0 = p1;
						}
						if (closed) {
							p1 = translate_to_screen ( stroke_points.get(0), r );
							curves.add ( default_curve ( p0, p1 ) );
						}

						// Smooth the handles on all of the curves
						int n = curves.size();
						for (int i=0; i<n; i++) {
							CubicCurve2D.Double seg_to_adjust = curves.get(i);
							// Adjust the h0 handle
							if (closed || (i > 0)) {
								// System.out.println ( " Adjusting h0 for segment " + i + ", with previous = " + ((n+i-1)%n) );
								// These notes map the variables from BezierTracing.java to the CubicCurve2D members:
								// p0.x = x1
								// p0.y = y1
								// p1.x = x2
								// p1.y = y2
								// h0.x = ctrlx1
								// h0.y = ctrly1
								// h1.x = ctrlx2
								// h1.y = ctrly2
								CubicCurve2D.Double prev_seg = curves.get((n+i-1)%n);
								double cdx = seg_to_adjust.x2 - prev_seg.x1;
								double cdy = seg_to_adjust.y2 - prev_seg.y1;
								seg_to_adjust.ctrlx1 = seg_to_adjust.x1 + (factor * cdx);
								seg_to_adjust.ctrly1 = seg_to_adjust.y1 + (factor * cdy);
							}
							// Adjust the h1 handle
							if (closed || (i < n-1)) {
								// System.out.println ( " Adjusting h1 for segment " + i + ", with next = " + ((n+i+1)%n) );
								CubicCurve2D.Double next_seg = curves.get((n+i+1)%n);
								double cdx = next_seg.x2 - seg_to_adjust.x1;
								double cdy = next_seg.y2 - seg_to_adjust.y1;
								seg_to_adjust.ctrlx2 = seg_to_adjust.x2 - (factor * cdx);
								seg_to_adjust.ctrly2 = seg_to_adjust.y2 - (factor * cdy);
							}
						}

						Stroke previous_stroke = g2.getStroke();
						if (line_padding >= 1) {
							g2.setStroke ( new BasicStroke(1 + (2*line_padding)) );
						}

						for (int j=0; j<curves.size(); j++) {
							g2.draw ( curves.get(j) );
						}

						// Draw the control points
						for (int j=0; j<stroke_points.size(); j++) {
							p0 = stroke_points.get(j);
							int l = 4;
							int x = r.x_to_pxi(p0[0]-dx);
							int y = r.y_to_pyi(dy-p0[1]);
							g.drawOval ( x-l, y-l, 2*l, 2*l );
						}

						g2.setStroke(previous_stroke);


					} else {

						// System.out.println ( "Fill this contour when mode == " + this.mode + " ?" );
						GeneralPath path = new GeneralPath();
						p0 = stroke_points.get(0);
						path.moveTo ( r.x_to_pxi(p0[0]-dx), r.y_to_pyi(dy-p0[1]) );
						for (int j=1; j<stroke_points.size(); j++) {
							p0 = stroke_points.get(j);
							path.lineTo ( r.x_to_pxi(p0[0]-dx), r.y_to_pyi(dy-p0[1]) );
						}
						if (closed) {
							path.closePath();
						}

						// It's not clear what the "mode" means, but -13 seems to match objects to fill
						if (this.mode == -13) {
							g2.fill ( path );
						} else {
							Stroke previous_stroke = g2.getStroke();
							if (line_padding >= 1) {
								g2.setStroke ( new BasicStroke(1 + (2*line_padding)) );
							}
							g2.draw ( path );
							g2.setStroke(previous_stroke);
						}
					}

				}

			}
		}
  }


  public void clear_strokes() {
    stroke_points = new ArrayList<double[]>();
  }

  public void close() {
    closed = true;
  }

  public void add_point (	double[] point ) {
    stroke_points.add ( point );
  }

	public void set_mode ( int mode ) {
		this.mode = mode;
	}

	public void set_hidden ( boolean hidden ) {
		this.hidden = hidden;
	}

	public void set_transform ( TransformClass xform ) {
		this.xform = xform;
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

