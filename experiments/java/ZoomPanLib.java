/* This Class provides zooming and panning for subclasses. */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

public class ZoomPanLib extends Panel implements MouseListener, MouseMotionListener, MouseWheelListener {

  // The x and y variables in this demo are from an arbitrary user coordinate space
  // The xs and py variables are in screen space

	static int w=800, h=600;

	int scroll_wheel_position = 0; // Counts the amount of "rolling" that's happened so far
	int px_offset = 0; // Amount to offset the screen coordinates in x
	int py_offset = 0; // Amount to offset the screen coordinates in y
	
	double zoom_base = 1.3;  // This controls how fast the screen zooms
	double zoom_exp = 0;     // This will increment and decrement with the mouse wheel
	double scale = 1.0;      // scale = zoom_base ^ zoom_exp;
	double mx = 1.0;
	double my = 1.0;

	int mouse_dragging_offset_x = 0;
	int mouse_dragging_offset_y = 0;

	boolean x_locked = false;
	boolean y_locked = false;

  public ZoomPanLib () {
    super();
		// this.setBackground ( bg );
		this.addMouseListener ( this );
		this.addMouseWheelListener ( this );
		this.addMouseMotionListener ( this );
  }

	public void lock_x ( boolean lock ) {
	  x_locked = lock;
	}

	public void lock_y ( boolean lock ) {
	  y_locked = lock;
	}

  public void set_scale_to_fit ( double x_min, double x_max, double y_min, double y_max, int w, int h ) {
    mx = w / (x_max - x_min);
    // px_offset = (int)( -mx * x_min );
    my = h / (y_max - y_min);
    // py_offset = (int)( -my * y_min );
    scale = Math.min(mx,my);
		scroll_wheel_position = - (int) Math.floor ( Math.log(scale) / Math.log(zoom_base) );
		// System.out.println ( "Scroll wheel = " + scroll_wheel_position );
	  zoom_exp = -scroll_wheel_position;
    scale = Math.pow(zoom_base, zoom_exp);
    mx = my = scale;
    
    int width_of_points = (int) ( (x_max * mx) - (x_min * mx) );
    int height_of_points = (int) ( (y_max * my) - (y_min * my) );

    // For centering, start with offsets = 0 and work back
    px_offset = 0;
    py_offset = 0;
    
    px_offset = - x_to_pxi(x_min);
    py_offset = - y_to_pyi(y_min);
    
    px_offset += (w - width_of_points) / 2;
    py_offset += (h - height_of_points) / 2;
  }

	double x_to_px ( double x ) {
	  return (mx * x) + px_offset + mouse_dragging_offset_x;
	}

	double y_to_py ( double y ) {
	  return (my * y) + py_offset + mouse_dragging_offset_y;
	}

	double px_to_x ( double xs ) {
	  return (xs - (px_offset + mouse_dragging_offset_x)) / mx;
	}

	double py_to_y ( double py ) {
	  return (py - (py_offset + mouse_dragging_offset_y)) / my;
	}


	public int x_to_pxi ( double x ) {
	  return (int)(x_to_px(x));
	}

	public int y_to_pyi ( double y ) {
	  return (int)(y_to_py(y));
	}


  // static Color bg = new Color (0,0,0);

  Image img_buffer = null;
  Graphics g_buffer = null;
  boolean recalculate = true;

  void init_gbuffer() {
    if ( (img_buffer == null) || (g_buffer == null) || (size().width != img_buffer.getWidth(this)) || (size().height != img_buffer.getHeight(this)) ) {
      img_buffer = createImage ( size().width, size().height );
      g_buffer = img_buffer.getGraphics();
      g_buffer.setColor ( this.getBackground() );
      g_buffer.fillRect (0,0,size().width, size().height);
      recalculate = true;
    }
  }

  public void update(Graphics g) {
    init_gbuffer();
    g_buffer.setColor ( this.getBackground() );
    g_buffer.fillRect (0,0,size().width, size().height);
    paint ( g );
  }

  public void paint(Graphics g) {
    init_gbuffer();
    paint_frame ( g_buffer );
    g.drawImage ( img_buffer, 0, 0, this );
  }

	public void paint_frame (Graphics g) {
	  if (recalculate) {
      set_scale_to_fit ( -100, 100, -100, 100, getSize().width, getSize().height );
	    recalculate = false;
	  }
    for (int r=0; r<100; r++) {
  		g.setColor ( new Color ( 255*(r&0x04)/4, 255*(r&0x02)/2, 255*(r&0x01) ) );
		  g.drawLine (  x_to_pxi(-r),  y_to_pyi(-r),  x_to_pxi(-r),  y_to_pyi(r) );
		  g.drawLine (  x_to_pxi(-r),  y_to_pyi(r),   x_to_pxi(r),   y_to_pyi(r) );
		  g.drawLine (  x_to_pxi(r),   y_to_pyi(r),   x_to_pxi(r),   y_to_pyi(-r) );
		  g.drawLine (  x_to_pxi(r),   y_to_pyi(-r),  x_to_pxi(-r),  y_to_pyi(-r) );
		}
	}


  // MouseWheelListener methods:

	public void mouseWheelMoved ( MouseWheelEvent e ) {
	  // System.out.println ( "Before moving, wheel = " + scroll_wheel_position );
	  scroll_wheel_position += e.getWheelRotation();
	  int scrolled_x = e.getX();
	  int scrolled_y = e.getY();

	  double old_scale = scale;

	  zoom_exp = -scroll_wheel_position;
	  scale = Math.pow(zoom_base, zoom_exp);

	  // System.out.println ( "Wheel moved by " + e.getWheelRotation() + " at " + scrolled_x + "," + scrolled_y + ", new position is " + scroll_wheel_position + ", scale = " + scale );
	  mx = my = scale;
	  if (!x_locked) {
	    px_offset = (int)(scrolled_x - ( (scale/old_scale) * (scrolled_x-px_offset) ));
	  }
	  if (!y_locked) {
  	  py_offset = (int)(scrolled_y - ( (scale/old_scale) * (scrolled_y-py_offset) ));
  	}

		repaint();
	}

  //  MouseListener methods:
  
  protected int mouse_down_x = 0;
  protected int mouse_down_y = 0;

  public void mouseClicked ( MouseEvent e ) {
    // System.out.println ( "Mouse clicked" );
  }

  public void mousePressed ( MouseEvent e ) {
    // System.out.println ( "Mouse pressed" );
    mouse_down_x = e.getX();
    mouse_down_y = e.getY();
  }

  public void mouseReleased ( MouseEvent e ) {
    // System.out.println ( "Mouse released" );
    if (!x_locked) {
      px_offset += mouse_dragging_offset_x;
    }
    if (!y_locked) {
      py_offset += mouse_dragging_offset_y;
    }
    mouse_dragging_offset_x = 0;
    mouse_dragging_offset_y = 0;
  }

  public void mouseEntered ( MouseEvent e ) {
    // System.out.println ( "Mouse entered" );
  }

  public void mouseExited ( MouseEvent e ) {
    // System.out.println ( "Mouse exited" );
  }

  // MouseMotionListener methods:

  public void mouseDragged ( MouseEvent e ) {
	  // System.out.println ( "Mouse dragged by " + e.getX() + "," + e.getY() );
	  if (!x_locked) {
	    mouse_dragging_offset_x = e.getX() - mouse_down_x;
	  }
	  if (!y_locked) {
  	  mouse_dragging_offset_y = e.getY() - mouse_down_y;
  	}
	  repaint();
  }

  public void mouseMoved ( MouseEvent e ) {
  }

	public static void main ( String[] args ) {
		System.out.println ( "Use the mouse wheel to zoom, and drag to pan." );
		javax.swing.SwingUtilities.invokeLater ( new Runnable() {
			public void run() {
			  JFrame f = new JFrame("Zoom and Pan Demonstration");
				f.setDefaultCloseOperation ( JFrame.EXIT_ON_CLOSE );
				
				ZoomPanLib zp_top = new ZoomPanLib();
				zp_top.setBackground ( new Color (0,0,0) );
				zp_top.addMouseListener ( zp_top );
				zp_top.addMouseWheelListener ( zp_top );
				zp_top.addMouseMotionListener ( zp_top );

				ZoomPanLib zp_bot = new ZoomPanLib();
				zp_bot.lock_x(true);
				zp_bot.lock_y(true);
				zp_bot.setBackground ( new Color (64,64,64) );
				zp_bot.addMouseListener ( zp_bot );
				zp_bot.addMouseWheelListener ( zp_bot );
				zp_bot.addMouseMotionListener ( zp_bot );

				JSplitPane split_pane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, true, zp_top, zp_bot );
				split_pane.setResizeWeight ( 0.8 );

				f.add ( split_pane );
				f.pack();
				f.setSize ( w, h );
				f.setVisible ( true );
			}
		} );
	}

}
