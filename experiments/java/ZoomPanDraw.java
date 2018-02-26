/* This is a demonstration program to show zooming(scrollwheel) and panning(mouse drag) functionality. */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

import java.io.*;
import java.awt.image.*;
import javax.imageio.ImageIO;

public class ZoomPanDraw extends ZoomPanLib implements ActionListener, MouseMotionListener {

	static int w=800, h=600;
	
	boolean drawing_mode = false;
	boolean stroke_started = false;
  BufferedImage image_frame = null;

	ArrayList strokes = new ArrayList();  // Argument (if any) specifies initial capacity (default 10)
	ArrayList stroke  = null;

  void dump_strokes() {
    for (int i=0; i<strokes.size(); i++) {
      System.out.println ( " Stroke " + i );
      ArrayList s = (ArrayList)(strokes.get(i));
	    for (int j=0; j<s.size(); j++) {
	      double p[] = (double[])(s.get(j));
	      System.out.println ( "   Point " + j + " = [" + p[0] + "," + p[1] + "]" );
	    }
    }
  }

  void draw_stroke ( Graphics g, ArrayList s ) {
    if (s.size() > 0) {
      int line_padding = 1;
      for (int xoffset=-line_padding; xoffset<=line_padding; xoffset++) {
        for (int yoffset=-line_padding; yoffset<=line_padding; yoffset++) {
          double p0[] = (double[])(s.get(0));
          for (int j=1; j<s.size(); j++) {
            double p1[] = (double[])(s.get(j));
      		  g.drawLine (  xoffset+x_to_pxi(p0[0]),   yoffset+y_to_pyi(p0[1]),  xoffset+x_to_pxi(p1[0]),  yoffset+y_to_pyi(p1[1]) );
            // System.out.println ( "   Line " + j + " = [" + p0[0] + "," + p0[1] + "] to [" + p1[0] + "," + p1[1] + "]" );
      		  p0 = new double[2];
      		  p0[0] = p1[0];
      		  p0[1] = p1[1];
          }
        }
      }
    }
  }

	public void paint_frame (Graphics g) {
	  Dimension win_s = getSize();
	  int win_w = win_s.width;
	  int win_h = win_s.height;
	  if (recalculate) {
      set_scale_to_fit ( -100, 100, -100, 100, win_w, win_h );
	    recalculate = false;
	  }
		if (image_frame == null) {
		  System.out.println ( "Image is null" );
		} else {
		  // System.out.println ( "Image is NOT null" );
		  int img_w = image_frame.getWidth();
		  int img_h = image_frame.getHeight();
		  double img_wf = 200;
		  double img_hf = 200;
		  if (img_w >= img_h) {
		    // Make the image wider to fit
		    img_wf = img_w * img_wf / img_h;
		  } else {
		    // Make the height shorter to fit
		    img_hf = img_h * img_hf / img_w;
		  }
		  int draw_x = x_to_pxi(-img_wf/2.0);
		  int draw_y = y_to_pyi(-img_hf/2.0);
		  int draw_w = x_to_pxi(img_wf/2.0) - draw_x;
		  int draw_h = y_to_pyi(img_hf/2.0) - draw_y;
  		g.drawImage ( image_frame, draw_x, draw_y, draw_w, draw_h, this );
  		//g.drawImage ( image_frame, (win_w-img_w)/2, (win_h-img_h)/2, img_w, img_h, this );
    }

		g.setColor ( new Color ( 200, 200, 90 ) );
    for (int i=0; i<strokes.size(); i++) {
      // System.out.println ( " Stroke " + i );
      ArrayList s = (ArrayList)(strokes.get(i));
      draw_stroke ( g, s );
    }
    if (stroke != null) {
  		g.setColor ( new Color ( 255, 255, 190 ) );
      draw_stroke ( g, stroke );
    }
	}


  //  MouseListener methods:
  
  public void mouseClicked ( MouseEvent e ) {
    // System.out.println ( "Mouse clicked" );
    super.mouseClicked(e);
  }

  public void mousePressed ( MouseEvent e ) {
    // System.out.println ( "Mouse pressed" );
    super.mousePressed(e);
    if (drawing_mode == true) {
      if (stroke != null) {
        // System.out.println ( "Saving previous stroke" );
        strokes.add ( stroke );
      }
      // System.out.println ( "Making new stroke" );
      stroke  = new ArrayList(100);
      double p[] = { px_to_x(e.getX()), py_to_y(e.getY()) };
      // System.out.println ( "Adding point " + p[0] + "," + p[1] );
      stroke.add ( p );
  	  repaint();
    }
  }

  public void mouseReleased ( MouseEvent e ) {
    // System.out.println ( "Mouse released" );
    if (drawing_mode == false) {
      super.mouseReleased(e);
    } else {
      if (stroke != null) {
        double p[] = { px_to_x(e.getX()), py_to_y(e.getY()) };
        // System.out.println ( "Adding point " + p[0] + "," + p[1] );
        stroke.add ( p );
        // System.out.println ( "Adding active stroke to strokes list" );
        strokes.add ( stroke );
        // System.out.println ( "Setting stroke to null" );
        stroke = null;
    	  repaint();
      }
    }
  }

  public void mouseEntered ( MouseEvent e ) {
    // System.out.println ( "Mouse entered" );
    super.mouseEntered(e);
  }

  public void mouseExited ( MouseEvent e ) {
    // System.out.println ( "Mouse exited" );
    super.mouseExited(e);
  }


  // MouseMotionListener methods:

  public void mouseDragged ( MouseEvent e ) {
    // System.out.println ( "Mouse dragged" );
    if (drawing_mode == false) {
      super.mouseDragged(e);
    } else {
      if (stroke == null) {
        // System.out.println ( "stroke was null, making new array" );
        stroke  = new ArrayList(100);
      }
      if (stroke != null) {
        double p[] = { px_to_x(e.getX()), py_to_y(e.getY()) };
        // System.out.println ( "Adding point " + p[0] + "," + p[1] );
        stroke.add ( p );
    	  repaint();
      }
    }
    /*
    return;
	  // System.out.println ( "Mouse dragged by " + e.getX() + "," + e.getY() );
	  if (!x_locked) {
	    mouse_dragging_offset_x = e.getX() - mouse_down_x;
	  }
	  if (!y_locked) {
  	  mouse_dragging_offset_y = e.getY() - mouse_down_y;
  	}
	  repaint();
	  */
  }

  public void mouseMoved ( MouseEvent e ) {
    super.mouseMoved ( e );
  }


	public void actionPerformed(ActionEvent e) {
		String cmd = e.getActionCommand();
		System.out.println ( "ActionPerformed got \"" + cmd + "\"" );
		
		if (cmd.equalsIgnoreCase("Move")) {
		  drawing_mode = false;
		  stroke_started = false;
		} else if (cmd.equalsIgnoreCase("Draw")) {
		  drawing_mode = true;
		  stroke_started = false;
		} else if (cmd.equalsIgnoreCase("Dump")) {
		  dump_strokes();
		} else if (cmd.equalsIgnoreCase("Clear")) {
	    strokes = new ArrayList();
	    stroke  = null;
   	  repaint();
		} else if (cmd.equalsIgnoreCase("Exit")) {
			System.exit ( 0 );
		}
  }

	public static void main ( String[] args ) {
		System.out.println ( "Use the mouse wheel to zoom, and drag to pan." );
		javax.swing.SwingUtilities.invokeLater ( new Runnable() {
			public void run() {
			  JFrame f = new JFrame("Zoom/Pan Drawing Demonstration");
				f.setDefaultCloseOperation ( JFrame.EXIT_ON_CLOSE );
				
				ZoomPanDraw zp = new ZoomPanDraw();

				JMenuBar menu_bar = new JMenuBar();
          JMenuItem mi;
          ButtonGroup bg;
          JMenu file_menu = new JMenu("File");
	        	file_menu.add ( mi = new JMenuItem("New") );
	        	mi.addActionListener(zp);
	        	file_menu.add ( mi = new JMenuItem("Exit") );
	        	mi.addActionListener(zp);
	      	  menu_bar.add ( file_menu );
          JMenu mode_menu = new JMenu("Mode");
            bg = new ButtonGroup();
	        	mode_menu.add ( mi = new JRadioButtonMenuItem("Move", true) );
	        	mi.addActionListener(zp);
	        	bg.add ( mi );
	        	mode_menu.add ( mi = new JRadioButtonMenuItem("Draw") );
	        	mi.addActionListener(zp);
	        	bg.add ( mi );
	        	mode_menu.add ( mi = new JMenuItem("Dump") );
	        	mi.addActionListener(zp);
	        	mode_menu.add ( mi = new JMenuItem("Clear") );
	        	mi.addActionListener(zp);
	      	  menu_bar.add ( mode_menu );
				f.setJMenuBar ( menu_bar );
				
				zp.setBackground ( new Color (0,0,0) );

        try {
			    zp.image_frame = ImageIO.read(new File("test.png"));
			    // int type = zp.image_frame.getType() == 0 ? BufferedImage.TYPE_INT_ARGB : zp.image_frame.getType();
		    } catch (IOException e) {
			    System.out.println( "\n\nI/O Error opening JPEG input file.\n" + e + "\n" );
		    }

				f.add ( zp );
				f.pack();
				f.setSize ( w, h );
				f.setVisible ( true );
			}
		} );
	}

}
