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
	ArrayList<double[][]> handle_points = null;
	TransformClass xform = null;
	boolean closed = true;
	boolean hidden = false;
	double r=1.0, g=0.0, b=0.0;
	int mode=0;
	public boolean modified=true; // Any new contour can be considered "modified" because it needs to be saved
	public boolean is_bezier=false;
	Element contour_element=null;

  public ContourClass ( Element element, TransformClass current_transform ) {
    this.contour_name = element.getAttribute("name");
    String type_str = element.getAttribute("type");
    String points_str = element.getAttribute("points");
    String xy_str[] = points_str.trim().split(",");
    // Allocate an ArrayList to hold the double points
    ArrayList<double[]> stroke = new ArrayList<double[]>(xy_str.length);
    for (int xyi=0; xyi<xy_str.length; xyi++) {
      String xy[] = xy_str[xyi].trim().split(" ");
      // double p[] = { (Double.parseDouble(xy[0])*165)-100, (-Double.parseDouble(xy[1])*165)+100 };
      double p[] = { Double.parseDouble(xy[0]), Double.parseDouble(xy[1]) };
      stroke.add ( p );
      priority_println ( 20, "              " + xy_str[xyi].trim() + " = " + p[0] + "," + p[1] );
    }
    // strokes.add ( stroke );
    boolean is_bezier = false;
    if (type_str != null) {
      if (type_str.equals("bezier")) {
        is_bezier = true;
      }
    }
    // ContourClass cc = new ContourClass ( stroke, element.getAttribute("border"), element.getAttribute("closed").trim().equals("true"), is_bezier );

		this.init_stroke_and_closed ( stroke, element.getAttribute("closed").trim().equals("true") );
		this.init_color ( element.getAttribute("border") );
		this.init_bezier ( is_bezier );

    set_mode ( Integer.parseInt ( element.getAttribute("mode").trim() ) );
    set_hidden ( element.getAttribute("hidden").trim().equals("true") );
    set_transform ( current_transform );
    modified = false; // This is currently a way to keep contours read from XML from being duplicated.
  }

  public ContourClass ( ArrayList<double[]> stroke, String color_string, boolean closed ) {
		this.init_stroke_and_closed ( stroke, closed );
		this.init_color ( color_string );
  }

  public ContourClass ( ArrayList<double[]> stroke, String color_string, boolean closed, boolean bezier ) {
		this.init_stroke_and_closed ( stroke, closed );
		this.init_color ( color_string );
		this.init_bezier ( bezier );
  }

  public ContourClass ( ArrayList<double[]> stroke, int trace_color, boolean closed ) {
		this.init_stroke_and_closed ( stroke, closed );
		this.init_color ( trace_color );
  }

  public ContourClass ( ArrayList<double[]> stroke, int trace_color, boolean closed, boolean bezier ) {
		this.init_stroke_and_closed ( stroke, closed );
		this.init_color ( trace_color );
		this.init_bezier ( bezier );
  }

	public void init_stroke_and_closed ( ArrayList<double[]> stroke, boolean closed ) {
		this.stroke_points = stroke;
		this.closed = closed;
	}

	public void init_color ( String color_string ) {
  	String color_part_strings[] = color_string.trim().split(" ");
  	try { r = Double.parseDouble ( color_part_strings[0].trim() ); } catch (Exception e) { r = 0.5; }
  	try { g = Double.parseDouble ( color_part_strings[1].trim() ); } catch (Exception e) { g = 0.5; }
  	try { b = Double.parseDouble ( color_part_strings[2].trim() ); } catch (Exception e) { b = 0.5; }
	}

	public void init_color ( int trace_color ) {
		r = ( (trace_color & 0x00ff0000) >> 16 ) / 255.0;
		g = ( (trace_color & 0x0000ff00) >>  8 ) / 255.0;
		b = ( (trace_color & 0x000000ff)       ) / 255.0;
	}


	public double twice_area() {
		double ha = 0.0;
	  if (stroke_points != null) {
			int n = stroke_points.size();
			if (n >= 3) {
				// Area = 2 * Sum over each segment of (x1-x0)(y1+y0)
				double p0[] = null;
				double p1[] = null;
				p0 = stroke_points.get(0);
				for (int i=0; i<n; i++) {
					p1 = stroke_points.get((i+1)%n);
					ha += ( p1[0] - p0[0] ) * ( p1[1] + p0[1] );
					p0 = p1;
				}
			}
		}
		return ( ha );
	}

	public void dump_stroke() {
    for (int j=0; j<stroke_points.size(); j++) {
      double p[] = stroke_points.get(j);
      priority_println ( 150, "   Contour Point " + j + " = [" + p[0] + "," + p[1] + "]" );
    }
    priority_println ( 150, "   Contour Area = " + (0.5 * twice_area()) );
	}

	public void dump_area() {
	  String name = "";
	  if (this.contour_name != null) {
	    name = " " + this.contour_name;
	  }
    priority_println ( 150, "   Contour" + name + ": Area = " + (0.5 * twice_area()) );
	}

	double[][] default_handle_points ( double p0[], double p1[] ) {
    double mid_x, mid_y;

    mid_x = p0[0] + ((p1[0]-p0[0])/3);
    mid_y = p0[1] + ((p1[1]-p0[1])/3);
    double h0[] = { mid_x, mid_y };

    mid_x = p0[0] + (2*(p1[0]-p0[0])/3);
    mid_y = p0[1] + (2*(p1[1]-p0[1])/3);
    double h1[] = { mid_x, mid_y };

    double h[][] = { h0, h1 };
    return ( h );
	}


	public void init_bezier ( boolean bezier ) {
		// Create the default Bezier handles for the current set of points

		this.is_bezier = bezier;

		if ( (bezier) && (stroke_points.size() > 1) ) {
			// Compute the default handles for the points so far
			int n = stroke_points.size();
			handle_points = new ArrayList<double[][]>();

			double p0[] = null;
			double p1[] = null;

			p0 = stroke_points.get(0);
			for (int stroke_point_index=1; stroke_point_index<n; stroke_point_index++) {
				p1 = stroke_points.get(stroke_point_index);
				handle_points.add ( default_handle_points ( p0, p1 ) );
				p0 = p1;
			}

			if (closed) {
				p1 = stroke_points.get(0);
				handle_points.add ( default_handle_points ( p0, p1 ) );
			}

			// Smooth the handles on all of the curves
			double factor = 0.2;
			int prev = 0;
			int next = 0;
			for (int i=0; i<n; i++) {
				prev = (n+i-1)%n;
				next = (n+i+1)%n;
				// double seg_to_adjust[][] = 
				//// CubicCurve2D.Double seg_to_adjust = curves.get(i);
				// Adjust the h0 handle
				if (closed || (i > 0)) {
					System.out.println ( " Adjusting h0 for segment " + i + ", with previous = " + prev );

					double[] current_point = stroke_points.get(i);
					double[] next_point = stroke_points.get(next);
					double[] prev_point = stroke_points.get(prev);
					double cdx = next_point[0] - prev_point[0];
					double cdy = next_point[1] - prev_point[1];

					double[][]handles = handle_points.get(i);
					handles[0][0] = current_point[0] - (factor * cdx);
					handles[0][1] = current_point[1] - (factor * cdy);

					// These notes map the variables from BezierTracing.java to the CubicCurve2D members:
					// p0.x = x1
					// p0.y = y1
					// p1.x = x2
					// p1.y = y2
					// h0.x = ctrlx1
					// h0.y = ctrly1
					// h1.x = ctrlx2
					// h1.y = ctrly2
					// CubicCurve2D.Double(double x1, double y1, double ctrlx1, double ctrly1, double ctrlx2, double ctrly2, double x2, double y2) 
					/*
					CubicCurve2D.Double prev_seg = curves.get((n+i-1)%n);
					double cdx = seg_to_adjust.x2 - prev_seg.x1;
					double cdy = seg_to_adjust.y2 - prev_seg.y1;
					seg_to_adjust.ctrlx1 = seg_to_adjust.x1 + (factor * cdx);
					seg_to_adjust.ctrly1 = seg_to_adjust.y1 + (factor * cdy);
					*/
				}
				// Adjust the h1 handle
				if (closed || (i < n-1)) {
					System.out.println ( " Adjusting h1 for segment " + i + ", with next = " + next );

					double[] current_point = stroke_points.get(i);
					double[] next_point = stroke_points.get(next);
					double[] prev_point = stroke_points.get(prev);
					double cdx = next_point[0] - prev_point[0];
					double cdy = next_point[1] - prev_point[1];

					double[][]handles = handle_points.get(i);
					handles[1][0] = current_point[0] + (factor * cdx);
					handles[1][1] = current_point[1] + (factor * cdy);


/*
					double[] current_point = stroke_points.get(i);
					double[] next_point = stroke_points.get(next);
					double cdx = next_point[0] - current_point[0];
					double cdy = next_point[1] - current_point[1];

					double[][]handles = handle_points.get(i);
					handles[1][0] = current_point[0] - (factor * cdx);
					handles[1][1] = current_point[1] - (factor * cdy);
*/
					/*
					CubicCurve2D.Double next_seg = curves.get((n+i+1)%n);
					double cdx = next_seg.x2 - seg_to_adjust.x1;
					double cdy = next_seg.y2 - seg_to_adjust.y1;
					seg_to_adjust.ctrlx2 = seg_to_adjust.x2 - (factor * cdx);
					seg_to_adjust.ctrly2 = seg_to_adjust.y2 - (factor * cdy);
					*/
				}
			}



		} else {
			handle_points = null;
		}
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
				double h0[][] = null;
				double h1[][] = null;
				double h[][] = null;
				double p[] = null;

				int line_padding = r.line_padding;

				if (line_padding >= 0) {

					if ( (is_bezier) && (handle_points != null) ) {

						Stroke previous_stroke = g2.getStroke();

						g.setColor ( new Color ( 100, 100, 100 ) );

						// Draw the control handle lines
						for (int j=0; j<handle_points.size(); j++) {
							int x, y, hx, hy;
							h = handle_points.get(j);
							p = stroke_points.get(j);

							x = r.x_to_pxi(p[0]-dx);
							y = r.y_to_pyi(dy-p[1]);
							hx = r.x_to_pxi(h[0][0]-dx);
							hy = r.y_to_pyi(dy-h[0][1]);
							g.setColor ( new Color ( 100, 0, 0 ) );
							g.drawLine ( x, y, hx, hy );

							x = r.x_to_pxi(p[0]-dx);
							y = r.y_to_pyi(dy-p[1]);
							hx = r.x_to_pxi(h[1][0]-dx);
							hy = r.y_to_pyi(dy-h[1][1]);
							g.setColor ( new Color ( 0, 100, 0 ) );
							g.drawLine ( x, y, hx, hy );
						}

						g.setColor ( new Color ( 150, 150, 150 ) );

						// Draw the control handle points
						for (int j=0; j<handle_points.size(); j++) {
							h = handle_points.get(j);
							int l = 4;
							int x, y;
							x = r.x_to_pxi(h[0][0]-dx);
							y = r.y_to_pyi(dy-h[0][1]);
							g.setColor ( new Color ( 150, 0, 0 ) );
							g.drawOval ( x-l, y-l, 2*l, 2*l );
							x = r.x_to_pxi(h[1][0]-dx);
							y = r.y_to_pyi(dy-h[1][1]);
							g.setColor ( new Color ( 0, 150, 0 ) );
							g.drawOval ( x-l, y-l, 2*l, 2*l );
						}

						g.setColor ( new Color ( (int)(255*this.r), (int)(255*this.g), (int)(255*this.b) ) );

						// Draw the end points for the curves
						for (int j=0; j<stroke_points.size(); j++) {
							if (j == 0) {
								g.setColor ( new Color ( 255, 255, 255 ) );
							} else {
								g.setColor ( new Color ( (int)(255*this.r), (int)(255*this.g), (int)(255*this.b) ) );
							}
							p0 = stroke_points.get(j);
							int l = 4;
							int x = r.x_to_pxi(p0[0]-dx);
							int y = r.y_to_pyi(dy-p0[1]);
							g.drawOval ( x-l, y-l, 2*l, 2*l );
						}
/*
						// Draw the curve itself

						ArrayList<CubicCurve2D.Double> curves = new ArrayList<CubicCurve2D.Double>();  // Argument (if any) specifies initial capacity (default 10)

						// Draw the curves themselves
						for (int j=1; j<stroke_points.size(); j++) {
							int x, y, hx, hy;
							p0 = stroke_points.get(j-1);
							p1 = stroke_points.get(j);
							h0 = handle_points.get(j-1);
							h1 = handle_points.get(j);

							double p0x = r.x_to_px(p0[0]-dx);
							double p0y = r.y_to_py(dy-p0[1]);
							double h0x = r.x_to_px(h0[1][0]-dx);
							double h0y = r.y_to_py(dy-h0[1][0]);
							double h1x = r.x_to_px(h1[0][0]-dx);
							double h1y = r.y_to_py(dy-h1[0][1]);
							double p1x = r.x_to_px(p1[0]-dx);
							double p1y = r.y_to_py(dy-p1[1]);


							h = handle_points.get(j);
							h1 = handle_points.get(j-1);
							h0x = r.x_to_px(h[0][0]-dx);
							h0y = r.y_to_py(dy-h[0][1]);
							h1x = r.x_to_px(h1[1][0]-dx);
							h1y = r.y_to_py(dy-h1[1][1]);



							// CubicCurve2D.Double(double x1, double y1, double ctrlx1, double ctrly1, double ctrlx2, double ctrly2, double x2, double y2) 
							curves.add ( new CubicCurve2D.Double ( p0x, p0y, h0x, h0y, h1x, h1y, p1x, p1y ) );

							// curves.add ( new CubicCurve2D.Double ( p0[0], p0[1], h0[1][0], h0[1][1], h1[0][0], h1[0][1], p1[0], p1[1] ) );

						}


						for (int j=0; j<curves.size(); j++) {
							g2.draw ( curves.get(j) );
						}

						g2.setStroke(previous_stroke);

*/

/*
						// path.moveTo ( r.x_to_pxi(p0[0]-dx), r.y_to_pyi(dy-p0[1]) );
						p0 = translate_to_screen ( stroke_points.get(0), r );
						for (int j=1; j<stroke_points.size(); j++) {
							p1 = translate_to_screen ( stroke_points.get(j), r );
							curves.add ( new CubicCurve2D.Double ( p0[0], p0[1], h0[0], h0[1], h1[0], h1[1], p1[0], p1[1] ) );
default_curve ( p0, p1 ) );
							p0 = p1;
						}
						if (closed) {
							p1 = translate_to_screen ( stroke_points.get(0), r );
							curves.add ( default_curve ( p0, p1 ) );
						}

						for (int j=0; j<curves.size(); j++) {
							g2.draw ( curves.get(j) );
						}

						g2.setStroke(previous_stroke);
*/

/*
						// Draw the curve itself
						ArrayList<CubicCurve2D.Double> curves = new ArrayList<CubicCurve2D.Double>();  // Argument (if any) specifies initial capacity (default 10)

						// CubicCurve2D.Double(double x1, double y1, double ctrlx1, double ctrly1, double ctrlx2, double ctrly2, double x2, double y2) 

						// path.moveTo ( r.x_to_pxi(p0[0]-dx), r.y_to_pyi(dy-p0[1]) );
						p0 = translate_to_screen ( stroke_points.get(0), r );
						for (int j=1; j<stroke_points.size(); j++) {
							p1 = translate_to_screen ( stroke_points.get(j), r );
							curves.add ( new CubicCurve2D.Double ( p0[0], p0[1], h0[0], h0[1], h1[0], h1[1], p1[0], p1[1] ) );
default_curve ( p0, p1 ) );
							p0 = p1;
						}
						if (closed) {
							p1 = translate_to_screen ( stroke_points.get(0), r );
							curves.add ( default_curve ( p0, p1 ) );
						}

						for (int j=0; j<curves.size(); j++) {
							g2.draw ( curves.get(j) );
						}

						g2.setStroke(previous_stroke);
*/




						double factor = 0.2;

						ArrayList<CubicCurve2D.Double> curves = new ArrayList<CubicCurve2D.Double>();  // Argument (if any) specifies initial capacity (default 10)
						// curves = new ArrayList<CubicCurve2D.Double>();  // Argument (if any) specifies initial capacity (default 10)

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

						previous_stroke = g2.getStroke();
						if (line_padding >= 1) {
							g2.setStroke ( new BasicStroke(1 + (2*line_padding)) );
						}

						for (int j=0; j<curves.size(); j++) {
							g2.draw ( curves.get(j) );
						}

/*
						// Draw the control points
						for (int j=0; j<stroke_points.size(); j++) {
							p0 = stroke_points.get(j);
							int l = 4;
							int x = r.x_to_pxi(p0[0]-dx);
							int y = r.y_to_pyi(dy-p0[1]);
							g.drawOval ( x-l, y-l, 2*l, 2*l );
						}
*/

						g2.setStroke(previous_stroke);



					} else {

						g.setColor ( new Color ( (int)(255*this.r), (int)(255*this.g), (int)(255*this.b) ) );

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

						if (r.show_points) {
							int l = 4;
							int x, y;
							for (int j=0; j<stroke_points.size(); j++) {
								p0 = stroke_points.get(j);
								p0 = stroke_points.get(j);
								x = r.x_to_pxi(p0[0]-dx);
								y = r.y_to_pyi(dy-p0[1]);
								g.fillOval ( x-l, y-l, 2*l, 2*l );
							}
						}
					}

				}

			}
		}
  }


  public void reverse_stroke() {
		System.out.println ( "   Contour reversing stroke" );
		ArrayList<double[]> reversed_stroke_points = new ArrayList<double[]>();
	  for (int j=stroke_points.size()-1; j>=0; j--) {
	    double p[] = stroke_points.get(j);
	    reversed_stroke_points.add ( p );
	  }
	  stroke_points = reversed_stroke_points;
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

