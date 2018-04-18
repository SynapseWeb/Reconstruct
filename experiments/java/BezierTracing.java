import java.awt.*;
import java.util.*;

import java.io.*;
import java.awt.image.*;
import javax.imageio.ImageIO;

/*
  A "point" is 2 values (x and y).
  A "segment" contains 2 end points.
  A "bezier segment" contains 2 end points and 2 handle points.
  A "curve" is a series of N points.
  A "open curve" is a curve with N-1 segments.
  A "closed curve" is a curve with N segments.
*/


class DrawParameters {
  boolean show_text = false;
  int symbol_size = 10;
  boolean round_points = true;
  boolean round_handles = true;
  boolean fill_current = true;
  boolean align_handles = true;
}


interface CanDraw {
  void draw (Graphics g, DrawParameters draw_pars);
  void draw (Graphics g);
}

interface PointSelect {
  PointValue closest_to ( double x, double y );
}

class PointValue {
  CurvePoint p = null;
  double v = 0.0;
  public PointValue ( CurvePoint p, double v ) {
    this.p = p; this.v = v;
  }
}

class Point implements CanDraw {
  public double x, y;
  public Point ( double x, double y ) {
    this.x = x; this.y = y;
  }
  public void draw ( Graphics g, DrawParameters draw_pars ) {
    int l = draw_pars.symbol_size;
    if (l > 0) {
      if (draw_pars.round_points && draw_pars.round_handles) {
        if (draw_pars.fill_current) {
          g.fillOval ( (int)(x-l), (int)(y-l), (int)(2*l), (int)(2*l) );
        } else {
          g.drawOval ( (int)(x-l), (int)(y-l), (int)(2*l), (int)(2*l) );
        }
      } else {
     		g.drawLine ((int)(x-l), (int)(y-l), (int)(x+l), (int)(y+l));
     		g.drawLine ((int)(x-l), (int)(y+l), (int)(x+l), (int)(y-l));
      }
    }
  }
  public void draw ( Graphics g ) {
    this.draw(g, new DrawParameters());
  }
}

class CurvePoint extends Point implements CanDraw {
  public CurveSegment segment;
  public CurvePoint ( double x, double y ) {
    super(x, y);
    // System.out.println ( "Called CurvePoint.super()" );
    this.segment = null;
  }
  public PointValue closest_to ( double x, double y ) {
    return ( new PointValue ( this, ((x-this.x)*(x-this.x)) + ((y-this.y)*(y-this.y)) ) );
  }
  public CurvePoint ( double x, double y, CurveSegment segment ) {
    super(x, y);
    this.segment = segment;
  }
}

class CurveSegment implements CanDraw, PointSelect {
  public CurvePoint p0, p1;
  public CurveSegment ( CurvePoint p0, CurvePoint p1 ) {
    //System.out.println ( "p0 = " + p0 );
    //System.out.println ( "this.p0 = " + this.p0 );
    //System.out.println ( "this = " + this );
    // System.out.println ( "this.p0.segment = " + this.p0.segment );
    this.p0 = p0; 
    this.p0.segment = this;
    this.p1 = p1; 
    this.p1.segment = this;
  }
  public PointValue closest_to ( double x, double y ) {
    PointValue pv0 = this.p0.closest_to ( x, y );
    PointValue pv1 = this.p1.closest_to ( x, y );
    if (pv0.v <= pv1.v) {
      return ( pv0 );
    } else {
      return ( pv1 );
    }
  }
  public void draw ( Graphics g, DrawParameters draw_pars ) {
    p0.draw ( g, draw_pars );
    p1.draw ( g, draw_pars );
 		g.drawLine ((int)(p0.x), (int)(p0.y), (int)(p1.x), (int)(p1.y));
  }
  public void draw ( Graphics g ) {
    this.draw(g, new DrawParameters());
  }
}

class BezierSegment extends CurveSegment implements CanDraw, PointSelect {
  public CurvePoint h0, h1;
	double ax, bx, cx, dx;
	double ay, by, cy, dy;

  public BezierSegment ( CurvePoint p0, CurvePoint p1 ) {
    super ( p0, p1 );
    double mid_x, mid_y;
    mid_x = p0.x + ((p1.x-p0.x)/3);
    mid_y = p0.y + ((p1.y-p0.y)/3);
    this.h0 = new CurvePoint ( mid_x, mid_y );
    mid_x = p0.x + (2*(p1.x-p0.x)/3);
    mid_y = p0.y + (2*(p1.y-p0.y)/3);
    this.h1 = new CurvePoint ( mid_x, mid_y );
  }

  public BezierSegment ( CurvePoint p0, CurvePoint p1, CurvePoint h0, CurvePoint h1 ) {
    super ( p0, p1 );
    this.h0 = h0;
    this.h1 = h1;
  }

  public PointValue closest_to ( double x, double y ) {
    CurvePoint p[] = { this.p0, this.h0, this.h1, this.p1 };
    PointValue pvclosest = null;
    for (int i=0; i<4; i++) {
      PointValue pv = p[i].closest_to ( x, y );
      if (i == 0) {
        pvclosest = pv;
      } else if (pv.v < pvclosest.v) {
        pvclosest = pv;
      }
    }
    return ( pvclosest );
  }

	public void update_equations() {
		dx = p0.x;
		cx = (h0.x - p0.x) * 3;
		bx = ((h1.x - h0.x) * 3) - cx;
		ax = ((p1.x - p0.x) - bx) - cx;

		dy = p0.y;
		cy = (h0.y - p0.y) * 3;
		by = ((h1.y - h0.y) * 3) - cy;
		ay = ((p1.y - p0.y) - by) - cy;
	}

	int last_x = 0;
	int last_y = 0;
	public void drawTo ( Graphics g, int x, int y ) {
		g.drawLine (last_x, last_y, x, y );
		last_x = x;
		last_y = y;
	}

  public void draw ( Graphics g, DrawParameters draw_pars ) {
    update_equations();
 		//g.drawLine ((int)(x-l), (int)(y-l), (int)(x+l), (int)(y+l));
 		//g.drawLine ((int)(x-l), (int)(y+l), (int)(x+l), (int)(y-l));

		double t, dt, t2, t3, x, y, dtdx, dtdy;

		// Draw the control lines along with the points and their labels
		String s = "";

		// Start with the control lines to put them in the background
		g.setColor(new Color(64,64,64));
		g.drawLine ((int)p0.x, (int)p0.y, (int)h0.x, (int)h0.y);
		g.drawLine ((int)h1.x, (int)h1.y, (int)p1.x, (int)p1.y);

		// Now draw each point and label it
  	Color colors[] = { new Color(255,20,20), new Color(20,255,20), new Color(100,100,255), new Color(240,240,0) };
		CurvePoint points[] = { p0, h0, h1, p1 };

		for (int i=0; i<4; i++) {
			g.setColor ( colors[i] );
			if (draw_pars.show_text) {
			  s = "P" + i + "=" + Integer.toString((int)(points[i].x)) + ", " + Integer.toString((int)(points[i].y));
			  int text_y_offset=0;
			  if (points[i].y < 30) {
			    text_y_offset = 30;
			  }
			  g.drawString ( s, (int)points[i].x, text_y_offset+(int)points[i].y );
			}
      int l = draw_pars.symbol_size;
      if (l > 0) {
        if (draw_pars.round_points && draw_pars.round_handles) {
          g.drawOval ( (int)(points[i].x-l), (int)(points[i].y-l), (int)(2*l), (int)(2*l) );
        } else {
			    g.drawLine ((int)points[i].x-l, (int)points[i].y+l, (int)points[i].x+l, (int)points[i].y-l);
			    g.drawLine ((int)points[i].x-l, (int)points[i].y-l, (int)points[i].x+l, (int)points[i].y+l);
        }
      }
		}

		// Finally, draw the curve itself

		g.setColor ( new Color(255,255,255) );

		dt = 0.001;

		t = 0;
		last_x = (int)p0.x;
		last_y = (int)p0.y;

		do {

			// Calculate t squared and t cubed for faster drawing
			t2 = t * t;
			t3 = t2 * t;

			// Calculate the next coordinate based on the parameter t
			x = (ax * t3) + (bx * t2) + (cx * t) + dx;
			y = (ay * t3) + (by * t2) + (cy * t) + dy;

			// Draw the segment (skipping the first when t==0)
			if (t == 0) {
				last_x = (int)x;
				last_y = (int)y;
			} else {
				drawTo (g, (int)x, (int)y); //, colr
			}

			// Calculate the maximum change in t that will change a single pixel in x or y
			dtdx = Math.abs(1 / ((3 * ax * t2) + (2 * bx * t) + cx));
			dtdy = Math.abs(1 / ((3 * ay * t2) + (2 * by * t) + cy));
			if (dtdx < dtdy) {
				dt = dtdx;
			} else {
				dt = dtdy;
			}
			if (dt < 0.5) {
				t = t + dt;
			} else {
			  // Handle the case where p[0]==p[1] (they lie on top of one another)
				t = t + 0.001;
			}

		} while ( t < 1 );
  }

}

class BezierCurve implements PointSelect {
  Vector segments = new Vector();

  public void draw ( Graphics g, DrawParameters draw_pars ) {
    for (int i=0; i<segments.size(); i++) {
      BezierSegment seg = (BezierSegment)(segments.elementAt(i));
      seg.draw ( g, draw_pars );
    }
  }

  public void add_segment ( BezierSegment new_segment ) {
    this.segments.add ( new_segment );
    this.smooth_segment_handles ( -1, 0.2 );
  }

  public void smooth_segment_handles ( int segment_num, double factor ) {
    int n = segments.size();
    boolean closed = ( ((BezierSegment)(segments.elementAt(0))).p0 == ((BezierSegment)(segments.elementAt(n-1))).p1 );
    for (int i=0; i<n; i++) {
      BezierSegment seg_to_adjust = (BezierSegment)(segments.elementAt(i));
      // Adjust the h0 handle
      if (closed || (i > 0)) {
        // System.out.println ( " Adjusting h0 for segment " + i + ", with previous = " + ((n+i-1)%n) );
        BezierSegment prev_seg = (BezierSegment)(segments.elementAt((n+i-1)%n));
        double dx = seg_to_adjust.p1.x - prev_seg.p0.x;
        double dy = seg_to_adjust.p1.y - prev_seg.p0.y;
        seg_to_adjust.h0.x = seg_to_adjust.p0.x + (factor * dx);
        seg_to_adjust.h0.y = seg_to_adjust.p0.y + (factor * dy);
      }
      // Adjust the h1 handle
      if (closed || (i < n-1)) {
        // System.out.println ( " Adjusting h1 for segment " + i + ", with next = " + ((n+i+1)%n) );
        BezierSegment next_seg = (BezierSegment)(segments.elementAt((n+i+1)%n));
        double dx = next_seg.p1.x - seg_to_adjust.p0.x;
        double dy = next_seg.p1.y - seg_to_adjust.p0.y;
        seg_to_adjust.h1.x = seg_to_adjust.p1.x - (factor * dx);
        seg_to_adjust.h1.y = seg_to_adjust.p1.y - (factor * dy);
      }
    }
  }

  public PointValue closest_to ( double x, double y ) {
    // CurvePoint p[] = { this.p0, this.h0, this.h1, this.p1 };
    PointValue pvclosest = null;
    for (int i=0; i<segments.size(); i++) {
      BezierSegment seg = (BezierSegment)(segments.elementAt(i));
      PointValue pv = seg.closest_to ( x, y );
      if (i == 0) {
        pvclosest = pv;
      } else if (pv.v < pvclosest.v) {
        pvclosest = pv;
      }
    }
    return ( pvclosest );
  }
}


class BezierCurves implements PointSelect, CanDraw {
  Vector curves = new Vector();
  BezierCurve current_curve = null;
  BezierSegment current_segment = null;
  CurvePoint current_point = null;
  
  CurvePoint closest_point = null;
  
  DrawParameters draw_pars = null;

  BezierCurves() {
    // System.out.println ( "Creating a new BezierCurves object" );
    draw_pars = new DrawParameters();
    draw_pars.show_text = false;
    draw_pars.symbol_size = 5;
  }
  
  public void dump () {
    System.out.println ( "======= Bezier Curves =======" );
    System.out.println ( "  Number of curves = " + curves.size() );
    System.out.println ( "  Current curve = " + current_curve );
    System.out.println ( "  Current segment = " + current_segment );
    System.out.println ( "  Current point = " + current_point );
    System.out.println ( "  Stored curves:" );
    for (int i=0; i<curves.size(); i++) {
      BezierCurve curve = (BezierCurve)(curves.elementAt(i));
      System.out.println ( "    Curve #" + i + " has " + curve.segments.size() + " segments:" );
      for (int j=0; j<curve.segments.size(); j++) {
        BezierSegment seg = (BezierSegment)(curve.segments.elementAt(j));
        System.out.println ( "      Segment #" + j + " goes from " + seg.p0.x + "," + seg.p0.y + " to " + seg.p1.x + "," + seg.p1.y  );
      }
    }
  }

  public void draw ( Graphics g, DrawParameters draw_pars ) {
    for (int i=0; i<curves.size(); i++) {
      BezierCurve curve = (BezierCurve)(curves.elementAt(i));
      curve.draw ( g, draw_pars );
    }
    if (current_curve != null) {
      current_curve.draw ( g, draw_pars );
    }
    if (closest_point != null) {
   		g.setColor(new Color(255,255,255));
   		draw_pars.fill_current = true;
      closest_point.draw ( g, draw_pars );
   		draw_pars.fill_current = false;
    }
    if ( (current_curve != null) && (current_curve.segments != null) ) {
      if (current_curve.segments.size() == 0) {
        if (current_point != null) {
          // System.out.println ( "Draw current point" );
          g.setColor(new Color(255,255,255));
          current_point.draw ( g, draw_pars );
        }
      }
    }
  }
  public void draw ( Graphics g ) {
    this.draw(g, draw_pars);
  }

  public void start_curve ( int x, int y ) {
    // System.out.println ( "Start a new curve" );
    if (current_curve != null) {
      curves.add ( current_curve );
      current_curve = null;
    }
    current_curve = new BezierCurve();
    current_point = new CurvePoint(x, y);
    // System.out.println ( "Started new curve with current_point = " + current_point );
    // dump();
  }

  public void add_point ( int x, int y ) {
    // System.out.println ( "Add a point at: " + x + "," + y + " to " + current_point );
    CurvePoint new_point = new CurvePoint(x, y);
    current_curve.add_segment ( new BezierSegment ( current_point, new_point ) );
    current_point = new_point;
    // dump();
  }

  public void end_curve ( int end_x, int end_y ) {
    if (current_curve != null) {
      if (current_curve.segments != null) {
        if (current_curve.segments.size() > 0) {
          // System.out.println ( "End the new curve at " + end_x + "," + end_y );
          // System.out.println ( "The most recent point is " + current_point.x + "," + current_point.y );
          CurvePoint start_point = ((BezierSegment)(current_curve.segments.elementAt(0))).p0;
          // System.out.println ( "The starting point was " + start_point.x + "," + start_point.y );
          // If the ending point is closer to the first point than to the current last point, close the loop
          double d_to_prev_sq = ((end_x-current_point.x)*(end_x-current_point.x)) + ((end_y-current_point.y)*(end_y-current_point.y));
          double d_to_start_sq = ((end_x-start_point.x)*(end_x-start_point.x)) + ((end_y-start_point.y)*(end_y-start_point.y));
          if (d_to_start_sq < d_to_prev_sq) {
            current_curve.add_segment ( new BezierSegment ( current_point, start_point ) );
          }
          curves.add ( current_curve );
        }
      }
      current_curve = null;
    }
    // dump();
  }

  public void select_closest_point ( int x, int y ) {
    // System.out.println ( "Select closest point to: " + x + "," + y );
    PointValue closest = closest_to ( x, y );
    if ( closest != null) {
      if (closest.v < (2*draw_pars.symbol_size*draw_pars.symbol_size)) {
        this.closest_point = closest.p;
      } else {
        this.closest_point = null;
      }
      // System.out.println ( "  Closest = " + closest.p.x + "," + closest.p.y );
    }
  }

  public PointValue closest_to ( double x, double y ) {
    // CurvePoint p[] = { this.p0, this.h0, this.h1, this.p1 };
    PointValue pvclosest = null;
    for (int i=0; i<curves.size(); i++) {
      BezierCurve curve = (BezierCurve)(curves.elementAt(i));
      PointValue pv = curve.closest_to ( x, y );
      if (i == 0) {
        pvclosest = pv;
      } else if (pv.v < pvclosest.v) {
        pvclosest = pv;
      }
    }
    return ( pvclosest );
  }

  public void key_press ( Event evt ) {
    if (evt.key == (int)'a') {
      draw_pars.align_handles = ! draw_pars.align_handles;
      System.out.println ( "Align Handles = " + draw_pars.align_handles );
    }
    if (evt.key == (int)'d') {
      System.out.println ( "Dumping data to bezier_traces.txt" );
    }
    if (evt.key == (int)'l') {
      System.out.println ( "Loading data from bezier_traces.txt" );
    }
    if (evt.key == (int)'r') {
      draw_pars.round_points = ! draw_pars.round_points;
      System.out.println ( "Set Round = " + draw_pars.round_points );
      draw_pars.round_handles = draw_pars.round_points;
    }
    if (evt.key == (int)'s') {
      if (draw_pars.symbol_size >= 40) {
        draw_pars.symbol_size = 0;
      } else if (draw_pars.symbol_size >= 20) {
        draw_pars.symbol_size = 40;
      } else if (draw_pars.symbol_size >= 12) {
        draw_pars.symbol_size = 20;
      } else if (draw_pars.symbol_size >= 8) {
        draw_pars.symbol_size = 12;
      } else if (draw_pars.symbol_size >= 5) {
        draw_pars.symbol_size = 8;
      } else if (draw_pars.symbol_size >= 3) {
        draw_pars.symbol_size = 5;
      } else if (draw_pars.symbol_size >= 0) {
        draw_pars.symbol_size = 3;
      } else {
        draw_pars.symbol_size = 8;
      }
      System.out.println ( "Symbol Size = " + draw_pars.symbol_size );
    }
    if (evt.key == (int)'t') {
      draw_pars.show_text = ! draw_pars.show_text;
      System.out.println ( "Show Text = " + draw_pars.show_text );
    }
    if (evt.key == (int)'x') {
      System.out.println ( "Deleting all traces" );
      current_curve = null;
      current_segment = null;
      current_point = null;
      closest_point = null;
      this.curves = new Vector();
    }
  }

  public void key_release ( Event evt ) {
    // System.out.println( "Key Press Event:" + evt.key );
  }

  public void mouse_drag ( BezierTracing window, Event evt ) {
    if (closest_point != null) {
      double dx = evt.x - this.closest_point.x;
      double dy = evt.y - this.closest_point.y;
      this.closest_point.x = evt.x;
      this.closest_point.y = evt.y;
      // Translate all handles attached to this point:
      for (int i=0; i<this.curves.size(); i++) {
        BezierCurve curve = (BezierCurve)(this.curves.elementAt(i));
        for (int j=0; j<curve.segments.size(); j++) {
          BezierSegment seg = (BezierSegment)(curve.segments.elementAt(j));
          if (seg.p0 == this.closest_point) {
            seg.h0.x += dx;
            seg.h0.y += dy;
          }
          if (seg.p1 == this.closest_point) {
            seg.h1.x += dx;
            seg.h1.y += dy;
          }
        }
      }
      if (this.draw_pars.align_handles) {
        // Rotate all handles attached to this point:
        for (int i=0; i<this.curves.size(); i++) {
          BezierCurve curve = (BezierCurve)(this.curves.elementAt(i));
          int nsegs = curve.segments.size();
          for (int j=0; j<nsegs; j++) {
            BezierSegment seg = (BezierSegment)(curve.segments.elementAt(j));
            if (seg.h0 == this.closest_point) {
              BezierSegment other_seg = (BezierSegment)(curve.segments.elementAt((j+nsegs-1)%nsegs));
              System.out.println ( "Seg " + j + ", h0: Update seg " + ((j+nsegs-1)%nsegs) + ", h1, loc = " + seg.h0.x + "," + seg.h0.y );
              double lp1h1 = Math.sqrt ( (seg.h1.x * seg.h1.x) + (seg.p1.y * seg.p1.y) );
              double lp0h0 = Math.sqrt ( (other_seg.h0.x * other_seg.h0.x) + (other_seg.p0.y * other_seg.p0.y) );
              other_seg.h1.x = other_seg.p1.x + ( lp1h1 * (seg.p0.x - seg.h0.x) / lp0h0 );
              other_seg.h1.y = other_seg.p1.y + ( lp1h1 * (seg.p0.y - seg.h0.y) / lp0h0 );
            }
            if (seg.h1 == this.closest_point) {
              BezierSegment other_seg = (BezierSegment)(curve.segments.elementAt((j+nsegs+1)%nsegs));
              System.out.println ( "Seg " + j + ", h1: Update seg " + ((j+nsegs+1)%nsegs) + ", h0, loc = " + seg.h1.x + "," + seg.h1.y );
              double lp1h1 = Math.sqrt ( (seg.h1.x * seg.h1.x) + (seg.p1.y * seg.p1.y) );
              double lp0h0 = Math.sqrt ( (other_seg.h0.x * other_seg.h0.x) + (other_seg.p0.y * other_seg.p0.y) );
              other_seg.h0.x = other_seg.p0.x + ( lp0h0 * (seg.p1.x - seg.h1.x) / lp1h1 );
              other_seg.h0.y = other_seg.p0.y + ( lp0h0 * (seg.p1.y - seg.h1.y) / lp1h1 );
            }
          }
       }
      }
      window.repaint();
    }
  }

}


class AppletFrame extends Frame {
	AppletFrame( String title ) {
		super(title);
	}
  public boolean handleEvent(Event evt) {
    // System.out.println( "Event:" + evt );
    if (evt.id == Event.WINDOW_DESTROY) {
      hide();
      System.exit(0);
    }
    return super.handleEvent(evt);
  }
}

public class BezierTracing extends java.applet.Applet {


  BezierCurves curves = new BezierCurves();

	static BezierTracing static_bezier = new BezierTracing();

	int w=600, h=400;
	
	static String default_file_name = null;

	void setUp ( int w, int h ) {

		this.w = w;
		this.h = h;

		setBackground(Color.black);
	}

	public void init() {
		int w, h;
		w = size().width;
		h = size().height;
		String s=null;

		s = getParameter("Width");
		if (s != null) {
			w = Integer.parseInt(s);
		}
		s = getParameter("Height");
		if (s != null) {
			h = Integer.parseInt(s);
		}
		setUp ( w, h );
	}

	BufferedImage bg = null;

  public void paint(Graphics g) {
		int x, y;
		w = size().width;
		h = size().height;
		
		if ( (bg == null) && (default_file_name != null) ) {
		  try {
		    bg = ImageIO.read(new File(default_file_name));
		  } catch (IOException e) {
			  System.out.println( "\n\nI/O Error opening image file.\n" + e + "\n" );
		  }
		}
		if (bg != null) {
  		g.drawImage ( bg, 0, 0, this );
    }
    // System.out.println ( "Calling curves.draw" );
    curves.draw(g);
  }

  boolean drawing = false;

  public boolean handleEvent(Event evt) {
  	if (evt.id == Event.MOUSE_DOWN) {
  	  if (evt.metaDown()) {
  	    // This is a right click
  	    if (drawing) {
  	      // Finish up the trace.
  	      // If right click was closest to ending point, then leave open
  	      // If right click was closest to starting point, then close
  	      // System.out.println ( "Finish trace" );
  	      this.curves.end_curve( evt.x, evt.y );
  	    } else {
  	      // Start a new trace
  	      // System.out.println ( "Start new trace" );
  	      if (this.curves == null) {
  	        this.curves = new BezierCurves();
  	      }
  	      this.curves.start_curve( evt.x, evt.y );
  	    }
  	    drawing = !drawing;
  	  } else {
  	    // This is a left click
  	    if (drawing) {
  	      // Add this point to the list
  	      // System.out.println ( "Add point to the trace" );
  	      this.curves.add_point( evt.x, evt.y );
  	    } else {
  	      // Select the closest point
  	      // System.out.println ( "Select closest point" );
  	      if (this.curves != null) {
    	      this.curves.select_closest_point ( evt.x, evt.y );
    	    }
  	    }
  	  }
  		//curve.select_point ( evt.x, evt.y );
 			repaint();
  	} else if (evt.id == Event.MOUSE_UP) {
 			//curve.select_none();
 			repaint();
  	} else if (evt.id == Event.MOUSE_DRAG) {
      if (this.curves != null) {
        this.curves.mouse_drag ( this, evt );
    	}
 		} else if (evt.id == Event.WINDOW_DESTROY) {
      hide();
      System.exit(0);
    } else if (evt.id == Event.MOUSE_MOVE) {
    } else if (evt.id == Event.KEY_PRESS) {
      if (this.curves != null) {
        this.curves.key_press ( evt );
      }
			repaint();
    } else if (evt.id == Event.KEY_RELEASE) {
      if (this.curves != null) {
        this.curves.key_release ( evt );
      }
		} else {
      // System.out.println( "Subwindow Event:" + evt );
    }
    return super.handleEvent(evt);
  }

	public static void main ( String args[] ) {
	  System.out.println ( "a: align handles" );
	  System.out.println ( "d: dump data to bezier_traces.txt" );
	  System.out.println ( "l: load data from bezier_traces.txt" );
	  System.out.println ( "r: round handles" );
	  System.out.println ( "s: change symbol size" );
	  System.out.println ( "t: show/hide text" );
	  System.out.println ( "x: delete all traces" );

		//Application Args   w h  #r (-)#c  diam
		int w=800, h=600, num_rows=20, num_cols=30, diam=20;

		int num_args = args.length;

    if (num_args > 0) default_file_name = args[0];

		// if (num_args > 0) w = Integer.parseInt(args[0]);
		// if (num_args > 1) h = Integer.parseInt(args[1]);

		// BezierTracing static_bezier = new BezierTracing();
		static_bezier.setUp ( w, h /* , num_rows, num_cols, diam */ );

		AppletFrame f = new AppletFrame ( "Bezier Tracing" );

		f.resize ( w, h );
		f.add("Center", static_bezier);
		f.show();
	}

}

