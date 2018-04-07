/* This is a demonstration program to show zooming(scrollwheel) and panning(mouse drag) functionality. */

import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

import java.io.*;
import java.awt.image.*;
import javax.imageio.ImageIO;

import org.w3c.dom.*;
import org.xml.sax.*;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringBufferInputStream;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;


class MyFileChooser extends JFileChooser {
  MyFileChooser ( String path ) {
    super(path);
  }
  protected JDialog createDialog(Component parent) throws HeadlessException {
    JDialog dialog = super.createDialog(parent);
    dialog.setLocation(300, 200);
    dialog.setSize ( 1024, 768 );
    // dialog.setResizable(false);
    return dialog;
  }
}


public class Reconstruct extends ZoomPanLib implements ActionListener, MouseMotionListener, MouseListener, KeyListener {

	static int w=800, h=600;
	
	boolean drawing_mode = false;
  boolean center_draw = false;
	boolean stroke_started = false;
  BufferedImage image_frame = null;
  BufferedImage image_frames[] = null;
  String current_directory = "";
  MyFileChooser file_chooser = null;

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

    g.setColor ( new Color ( 200, 0, 0 ) );
    for (int i=0; i<strokes.size(); i++) {
      // System.out.println ( " Stroke " + i );
      ArrayList s = (ArrayList)(strokes.get(i));
      draw_stroke ( g, s );
    }
    if (stroke != null) {
      g.setColor ( new Color ( 255, 0, 0 ) );
      draw_stroke ( g, stroke );
    }
    if (center_draw) {
      g.setColor ( new Color ( 255, 255, 255 ) );
      int cx = getSize().width / 2;
      int cy = getSize().height / 2;
      g.drawLine ( cx-10, cy, cx+10, cy );
      g.drawLine ( cx, cy-10, cx, cy+10 );
    }
	}


  //  MouseListener methods:
  

	Cursor current_cursor = null;
	Cursor h_cursor = null;
	Cursor v_cursor = null;
	Cursor b_cursor = null;
	int cursor_size = 33;

  public void mouseEntered ( MouseEvent e ) {
    if ( (h_cursor == null) || (v_cursor == null) || (b_cursor == null) ) {
      Toolkit tk = Toolkit.getDefaultToolkit();
      Graphics2D cg = null;
      BufferedImage cursor_image = null;
      Polygon p = null;
      int h = cursor_size;
      int w = cursor_size;

      // Create the horizontal cursor
      p = new Polygon();
      p.addPoint ( 0, h/2 );
      p.addPoint ( w/4, (h/2)-(h/4) );
      p.addPoint ( w/4, (h/2)-(h/8) );
      p.addPoint ( 3*w/4, (h/2)-(h/8) );
      p.addPoint ( 3*w/4, (h/2)-(h/4) );
      p.addPoint ( w-1, h/2 );
      p.addPoint ( 3*w/4, (h/2)+(h/4) );
      p.addPoint ( 3*w/4, (h/2)+(h/8) );
      p.addPoint ( w/4, (h/2)+(h/8) );
      p.addPoint ( w/4, (h/2)+(h/4) );

      cursor_image = new BufferedImage(cursor_size,cursor_size,BufferedImage.TYPE_4BYTE_ABGR);
      cg = cursor_image.createGraphics();
      cg.setColor ( new Color(255,255,255) );
      cg.fillPolygon ( p );
      cg.setColor ( new Color(0,0,0) );
      cg.drawPolygon ( p );

      h_cursor = tk.createCustomCursor ( cursor_image, new Point(cursor_size/2,cursor_size/2), "Horizontal" );

      // Create the vertical cursor
      p = new Polygon();
      p.addPoint ( w/2, 0 );
      p.addPoint ( (w/2)+(w/4), h/4 );
      p.addPoint ( (w/2)+(w/8), h/4 );
      p.addPoint ( (w/2)+(w/8), 3*h/4 );
      p.addPoint ( (w/2)+(w/4), 3*h/4 );
      p.addPoint ( w/2, h-1 );
      p.addPoint ( (w/2)-(w/4), 3*h/4 );
      p.addPoint ( (w/2)-(w/8), 3*h/4 );
      p.addPoint ( (w/2)-(w/8), h/4 );
      p.addPoint ( (w/2)-(w/4), h/4 );

      cursor_image = new BufferedImage(cursor_size,cursor_size,BufferedImage.TYPE_4BYTE_ABGR);
      cg = cursor_image.createGraphics();
      cg.setColor ( new Color(255,255,255) );
      cg.fillPolygon ( p );
      cg.setColor ( new Color(0,0,0) );
      cg.drawPolygon ( p );

      v_cursor = tk.createCustomCursor ( cursor_image, new Point(cursor_size/2,cursor_size/2), "Vertical" );

      // Create the both cursor

      int cw2 = w/2;   // Cursor width / 2
      int ch2 = h/2;   // Cursor height / 2

      int clw = w/12;  // Arrow line width / 2
      int caw = w/6;   // Arrow head width / 2
      int cal = (int)(w/3.5);   // Arrow head length

      p = new Polygon();
      p.addPoint (       -cw2,           0 );  // Left Point

      p.addPoint ( -(cw2-cal),         caw );  // Left arrow lower outside corner
      p.addPoint ( -(cw2-cal),         clw );  // Left arrow lower inside corner

      p.addPoint (       -clw,         clw );  // Left/Bottom corner

      p.addPoint (       -clw,     ch2-cal );  // Bottom arrow left inside corner
      p.addPoint (       -caw,     ch2-cal );  // Bottom arrow left outside corner

      p.addPoint (          0,         ch2 );  // Bottom Point

      p.addPoint (        caw,     ch2-cal );  // Bottom arrow right outside corner
      p.addPoint (        clw,     ch2-cal );  // Bottom arrow right inside corner

      p.addPoint (        clw,         clw );  // Right/Bottom corner

      p.addPoint (  (cw2-cal),         clw );  // Right arrow lower inside corner
      p.addPoint (  (cw2-cal),         caw );  // Right arrow lower outside corner

      p.addPoint (        cw2,           0 );  // Right Point

      p.addPoint (  (cw2-cal),        -caw );  // Right arrow upper outside corner
      p.addPoint (  (cw2-cal),        -clw );  // Right arrow upper inside corner

      p.addPoint (        clw,        -clw );  // Right/Top corner

      p.addPoint (        clw,  -(ch2-cal) );  // Top arrow right inside corner
      p.addPoint (        caw,  -(ch2-cal) );  // Top arrow right outside corner

      p.addPoint (          0,        -ch2 );  // Top Point

      p.addPoint (       -caw,  -(ch2-cal) );  // Top arrow left outside corner
      p.addPoint (       -clw,  -(ch2-cal) );  // Top arrow left inside corner

      p.addPoint (       -clw,        -clw );  // Left/Top corner

      p.addPoint ( -(cw2-cal),        -clw );  // Left arrow upper inside corner
      p.addPoint ( -(cw2-cal),        -caw );  // Left arrow upper outside corner

      p.addPoint (       -cw2,           0 );  // Left Point


      p.translate ( w/2, h/2 );

      cursor_image = new BufferedImage(cursor_size,cursor_size,BufferedImage.TYPE_4BYTE_ABGR);
      cg = cursor_image.createGraphics();
      cg.setColor ( new Color(255,255,255) );
      cg.fillPolygon ( p );
      cg.setColor ( new Color(0,0,0) );
      cg.drawPolygon ( p );

      b_cursor = tk.createCustomCursor ( cursor_image, new Point(cursor_size/2,cursor_size/2), "Both" );

    }
    if (current_cursor == null) {
      current_cursor = b_cursor;
      // current_cursor = Cursor.getPredefinedCursor ( Cursor.MOVE_CURSOR );
    }
    setCursor ( current_cursor );
  }

  public void mouseExited ( MouseEvent e ) {
    // System.out.println ( "Mouse exited" );
    super.mouseExited(e);
  }

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
      if (center_draw) {
        p[0] = px_to_x(getSize().width / 2);
        p[1] = py_to_y(getSize().height / 2);
      }
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
        if (center_draw) {
          p[0] = px_to_x(getSize().width / 2);
          p[1] = py_to_y(getSize().height / 2);
        }
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


  // MouseMotionListener methods:

  public void mouseDragged ( MouseEvent e ) {
    // System.out.println ( "Mouse dragged" );
    if (drawing_mode == false) {
      super.mouseDragged(e);
    } else {
      if (center_draw) {
        super.mouseDragged(e);
      }
      if (stroke == null) {
        // System.out.println ( "stroke was null, making new array" );
        stroke  = new ArrayList(100);
      }
      if (stroke != null) {
        double p[] = { px_to_x(e.getX()), py_to_y(e.getY()) };
        if (center_draw) {
          p[0] = px_to_x(getSize().width / 2);
          p[1] = py_to_y(getSize().height / 2);
        }
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

  JMenuItem move_menu_item = null;
  JMenuItem draw_menu_item = null;
  JMenuItem center_draw_menu_item = null;

  // KeyListener methods:

  public void keyTyped ( KeyEvent e ) {
    // System.out.println ( "Key: " + e );
    if (Character.toUpperCase(e.getKeyChar()) == ' ') {
      // Space bar toggles between drawing mode and move mode
      drawing_mode = !drawing_mode;
      if (drawing_mode) {
        current_cursor = Cursor.getPredefinedCursor ( Cursor.CROSSHAIR_CURSOR );
        if (center_draw) {
          current_cursor = Cursor.getPredefinedCursor ( Cursor.HAND_CURSOR );
        }
        setCursor ( current_cursor );
        stroke_started = false;
        if (draw_menu_item != null) {
          draw_menu_item.setSelected(true);
        }
      } else {
        current_cursor = b_cursor;
        setCursor ( current_cursor );
        stroke_started = false;
        if (move_menu_item != null) {
          move_menu_item.setSelected(true);
        }
      }
    } else if (Character.toUpperCase(e.getKeyChar()) == 'P') {
    } else {
      // System.out.println ( "KeyEvent = " + e );
    }
    repaint();
  }
  public void keyPressed ( KeyEvent e ) {
    // System.out.println ( "Key Pressed, e = " + e );
    if ( (e.getKeyCode() == 33) || (e.getKeyCode() == 34) ) {
      // Figure out if there's anything to do
      if (this.image_frames.length > 1) {
        // Find the current index
        int current_index = -1;
        for (int i=0; i<this.image_frames.length; i++) {
          if (this.image_frame == this.image_frames[i]) {
            current_index = i;
            break;
          }
        }
        // System.out.println ( "Current image is " + current_index );
        int delta = 0;
        if (e.getKeyCode() == 33) {
          // System.out.println ( "Page Up with " + this.image_frames.length + " frames" );
          delta = 1;
        } else if (e.getKeyCode() == 34) {
          // System.out.println ( "Page Down with " + this.image_frames.length + " frames" );
          delta = -1;
        }
        if ((current_index+delta >= 0) && (current_index+delta < this.image_frames.length)) {
          this.image_frame = this.image_frames[current_index+delta];
          repaint();
        }
      }
    }
    //super.keyPressed ( e );
  }
  public void keyReleased ( KeyEvent e ) {
    // System.out.println ( "Key Released" );
    //super.keyReleased ( e );
  }

  public String depth_string ( int depth ) {
    String s = "";
    for (int i=0; i<depth; i++) {
      s = "  " + s;
    }
    return ( s );
  }

  public void dump_nodes_and_attrs ( Element parent, int depth ) {

    NamedNodeMap attr_map = parent.getAttributes();
    for (int index=0; index<attr_map.getLength(); index++) {
      Node node = attr_map.item(index);
      System.out.println ( depth_string(depth) + "Attr: " + node );
    }

    NodeList nodes = parent.getElementsByTagName ( "*" );
    for (int index=0; index<nodes.getLength(); index++) {
      Node node = nodes.item(index);
      System.out.println ( depth_string(depth) + "Node: " + node );
      dump_nodes_and_attrs ( (Element)node, depth+1 );
    }

  }

  public void parse_series_xml_file ( File f ) {
    System.out.println ( "Parsing " + f );

    // Read the file and convert into a string buffer while removing the "<!DOCTYPE" line
    StringBuilder lines = new StringBuilder();
    try {
      BufferedReader fr = new BufferedReader ( new FileReader ( f ) );
      String line;
      do {
        line = fr.readLine();
        if ( ! line.trim().startsWith ( "<!DOCTYPE" ) ) {
          lines.append ( line );
          lines.append ( '\n' );
        }
      } while (line != null);
    } catch ( Exception e ) {
    }

    // Parse the resulting string buffer
    try {

      // Set up the parser
      DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
      dbFactory.setValidating ( false );
      DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();

      Document doc = null;

      try {
          doc = dBuilder.parse(new StringBufferInputStream(lines.toString()));
      } catch (SAXException e) {
          e.printStackTrace();
          doc = null;
      } catch (IOException e) {
          e.printStackTrace();
          doc = null;
      }

      if (doc != null) {

        // Process the document tree to pull out the data for this series

        doc.getDocumentElement().normalize();

        if ( ! doc.getDocumentElement().getNodeName().equalsIgnoreCase ( "series" ) ) {
          System.out.println ( "Error: Series XML files must contain a series element" );
        } else {

          System.out.println("Root element: " + doc.getDocumentElement().getNodeName());

          dump_nodes_and_attrs ( doc.getDocumentElement(), 1 );

          NamedNodeMap attr_map = doc.getDocumentElement().getAttributes();
          for (int index=0; index<attr_map.getLength(); index++) {
            Node node = attr_map.item(index);
            System.out.println ( "   Attr: " + node );
          }

          NodeList nodes = doc.getDocumentElement().getElementsByTagName ( "*" );
          for (int index=0; index<nodes.getLength(); index++) {
            Node node = nodes.item(index);
            System.out.println ( "   Node: " + node );
          }

        }
      }
    } catch ( Exception e ) {
      System.out.println("Parsing error: " + e );
    }
  }

  // ActionPerformed methods (mostly menu responses):

	public void actionPerformed(ActionEvent e) {
		String cmd = e.getActionCommand();
		System.out.println ( "ActionPerformed got \"" + cmd + "\"" );
		
		if (cmd.equalsIgnoreCase("Move")) {
      current_cursor = b_cursor;
      /*
        Alternative predefined cursors from java.awt.Cursor:
        current_cursor = Cursor.getPredefinedCursor ( Cursor.CROSSHAIR_CURSOR );
        current_cursor = Cursor.getPredefinedCursor ( Cursor.HAND_CURSOR );
        current_cursor = Cursor.getPredefinedCursor ( Cursor.MOVE_CURSOR );
      */
      setCursor ( current_cursor );
		  //center_draw = false;
		  drawing_mode = false;
		  stroke_started = false;
		  repaint();
		} else if (cmd.equalsIgnoreCase("Draw")) {
      current_cursor = Cursor.getPredefinedCursor ( Cursor.CROSSHAIR_CURSOR );
      if (center_draw) {
        current_cursor = Cursor.getPredefinedCursor ( Cursor.HAND_CURSOR );
      }
      setCursor ( current_cursor );
		  // center_draw = false;
		  drawing_mode = true;
		  stroke_started = false;
		  repaint();
		} else if (cmd.equalsIgnoreCase("Center Drawing")) {
		  JCheckBoxMenuItem item = (JCheckBoxMenuItem)e.getSource();
		  center_draw = item.getState();
		  if (drawing_mode) {
        current_cursor = Cursor.getPredefinedCursor ( Cursor.HAND_CURSOR );
      }
		  repaint();
		} else if (cmd.equalsIgnoreCase("Open...")) {
		  file_chooser.setMultiSelectionEnabled(false);
		  FileNameExtensionFilter filter = new FileNameExtensionFilter("Series Files", "ser");
		  file_chooser.setFileFilter(filter);
		  int returnVal = file_chooser.showDialog(this, "Open Series");
		  if ( returnVal == JFileChooser.APPROVE_OPTION ) {
        System.out.println ( "You chose to open this file: " /* + chooser.getCurrentDirectory() + " / " */ + file_chooser.getSelectedFile() );
        parse_series_xml_file ( file_chooser.getSelectedFile() );

        String file_path_and_name = "?Unknown?";
        try {
          file_path_and_name = "" + file_chooser.getSelectedFile();
          repaint();
        } catch (Exception oe) {
	        // this.image_frame = null;
	        JOptionPane.showMessageDialog(null, "File error for: " + file_path_and_name, "File Path Error", JOptionPane.WARNING_MESSAGE);
	        repaint();
        }
		  }

		} else if (cmd.equalsIgnoreCase("Dump")) {
		  dump_strokes();
		} else if (cmd.equalsIgnoreCase("Clear")) {
	    strokes = new ArrayList();
	    stroke  = null;
	    repaint();
		} else if ( cmd.equalsIgnoreCase("Add") || cmd.equalsIgnoreCase("Images...") ) {

			System.out.println ( "Opening new file ..." );
			// String old_path = current_base_path;

		  // MyFileChooser file_chooser = new MyFileChooser ( this.current_directory );
		  //FileNameExtensionFilter filter = new FileNameExtensionFilter("Text Files", "txt");
		  // FileNameExtensionFilter filter = new FileNameExtensionFilter("JPG & GIF Images", "jpg", "gif");
		  // file_chooser.setFileFilter(filter);
		  // data_set_chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		  file_chooser.setMultiSelectionEnabled(true);
		  int returnVal = file_chooser.showDialog(this, "Image Files to Add");
		  if ( returnVal == JFileChooser.APPROVE_OPTION ) {
		    File selected_files[] = file_chooser.getSelectedFiles();
		    if (selected_files.length > 0) {
		      for (int i=0; i<selected_files.length; i++) {
            System.out.println ( "You chose this file: " + selected_files[i] );
		      }
		    }
        System.out.println ( "You chose to open this file: " /* + chooser.getCurrentDirectory() + " / " */ + file_chooser.getSelectedFile() );
        String file_path_and_name = "?Unknown?";
        try {
          file_path_and_name = "" + file_chooser.getSelectedFile();
          File image_file = new File ( file_path_and_name );
          BufferedImage new_image;
          new_image = ImageIO.read(image_file);
          this.image_frame = new_image;

          if (this.image_frames == null) {
            this.image_frames = new BufferedImage[1];
          } else {
            BufferedImage temp[] = new BufferedImage[this.image_frames.length+1];
            for (int i=0; i<this.image_frames.length; i++) {
              temp[i] = this.image_frames[i];
            }
            this.image_frames = temp;
          }
          this.image_frames[this.image_frames.length-1] = this.image_frame;
          repaint();
        } catch (Exception oe) {
	        // this.image_frame = null;
	        JOptionPane.showMessageDialog(null, "File error for: " + file_path_and_name, "File Path Error", JOptionPane.WARNING_MESSAGE);
	        repaint();
        }
		  }
		} else if (cmd.equalsIgnoreCase("List Sections...")) {
		  System.out.println ( "Sections: ..." );
		} else if (cmd.equalsIgnoreCase("Exit")) {
			System.exit ( 0 );
		}
  }

	public static void main ( String[] args ) {
	  for (int i=0; i<args.length; i++) {
		  System.out.println ( "Arg[" + i + "] = \"" + args[i] + "\"" );
		}
		System.out.println ( "Reconstruct: Use the mouse wheel to zoom, and drag to pan." );
		javax.swing.SwingUtilities.invokeLater ( new Runnable() {
			public void run() {
			  JFrame f = new JFrame("Reconstruct Java Demonstration");
				f.setDefaultCloseOperation ( JFrame.EXIT_ON_CLOSE );
				
        Reconstruct zp = new Reconstruct();
        /* Can't use args in here
        if (args.length > 0) {
          zp.current_directory = args[0];
        } else {
	        zp.current_directory = System.getProperty("user.dir");
        }
        */
        zp.current_directory = System.getProperty("user.dir");

				JMenuBar menu_bar = new JMenuBar();
          JMenuItem mi;
          ButtonGroup bg;

          JMenu program_menu = new JMenu("Program");
            JMenu windows_menu = new JMenu("Windows");
              windows_menu.add ( mi = new JCheckBoxMenuItem("Tools window", true) );
              mi.addActionListener(zp);
              windows_menu.add ( mi = new JCheckBoxMenuItem("Status bar", true) );
              mi.addActionListener(zp);
            program_menu.add ( windows_menu );

            program_menu.addSeparator();

            JMenu debug_menu = new JMenu("Debug");
              debug_menu.add ( mi = new JCheckBoxMenuItem("Logging", true) );
              mi.addActionListener(zp);
              debug_menu.add ( mi = new JMenuItem("Times") );
              mi.addActionListener(zp);
              debug_menu.addSeparator();
              debug_menu.add ( mi = new JMenuItem("Dump") );
              mi.addActionListener(zp);
              debug_menu.add ( mi = new JMenuItem("Clear") );
              mi.addActionListener(zp);
            program_menu.add ( debug_menu );

            program_menu.add ( mi = new JMenuItem("Exit") );
            mi.addActionListener(zp);
            menu_bar.add ( program_menu );

          JMenu series_menu = new JMenu("Series");
            series_menu.add ( mi = new JMenuItem("Open...") );
            mi.addActionListener(zp);
            series_menu.add ( mi = new JMenuItem("Close") );
            mi.addActionListener(zp);

            series_menu.addSeparator();

            series_menu.add ( mi = new JMenuItem("New...") );
            mi.addActionListener(zp);
            series_menu.add ( mi = new JMenuItem("Save") );
            mi.addActionListener(zp);
            series_menu.add ( mi = new JMenuItem("Options...") );
            mi.addActionListener(zp);

            series_menu.addSeparator();

            JMenu export_menu = new JMenu("Export");
              export_menu.add ( mi = new JMenuItem("Images... ") );
              mi.addActionListener(zp);
              export_menu.add ( mi = new JMenuItem("Lines... ") );
              mi.addActionListener(zp);
              export_menu.add ( mi = new JMenuItem("Trace lists... ") );
              mi.addActionListener(zp);
            series_menu.add ( export_menu );

            JMenu import_menu = new JMenu("Import");
              import_menu.add ( mi = new JMenuItem("Images...") );
              mi.addActionListener(zp);
              import_menu.add ( mi = new JMenuItem("Lines...") );
              mi.addActionListener(zp);
              import_menu.add ( mi = new JMenuItem("Trace lists...") );
              mi.addActionListener(zp);
            series_menu.add ( import_menu );

            menu_bar.add ( series_menu );

          JMenu section_menu = new JMenu("Section");
            section_menu.add ( mi = new JMenuItem("List Sections...") );
            mi.addActionListener(zp);
            section_menu.add ( mi = new JMenuItem("Thumbnails...") );
            mi.addActionListener(zp);

            section_menu.addSeparator();

            section_menu.add ( mi = new JMenuItem("New...") );
            mi.addActionListener(zp);
            section_menu.add ( mi = new JMenuItem("Save") );
            mi.addActionListener(zp);
            section_menu.add ( mi = new JMenuItem("Thickness...") );
            mi.addActionListener(zp);

            section_menu.addSeparator();

            section_menu.add ( mi = new JMenuItem("Undo") );
            mi.addActionListener(zp);
            section_menu.add ( mi = new JMenuItem("Redo") );
            mi.addActionListener(zp);
            section_menu.add ( mi = new JMenuItem("Reset") );
            mi.addActionListener(zp);
            section_menu.add ( mi = new JMenuItem("Blend") );
            mi.addActionListener(zp);

            JMenu zoom_menu = new JMenu("Zoom");
              zoom_menu.add ( mi = new JMenuItem("Center") );
              mi.addActionListener(zp);
              zoom_menu.add ( mi = new JMenuItem("Last") );
              mi.addActionListener(zp);
              zoom_menu.add ( mi = new JMenuItem("Actual pixels") );
              mi.addActionListener(zp);
              zoom_menu.add ( mi = new JMenuItem("Magnification...") );
              mi.addActionListener(zp);
            section_menu.add ( zoom_menu );

            JMenu movement_menu = new JMenu("Movement");
              movement_menu.add ( mi = new JMenuItem("Unlock") );
              mi.addActionListener(zp);
              JMenu flip_menu = new JMenu("Flip");
                flip_menu.add ( mi = new JMenuItem("Horizontally") );
                mi.addActionListener(zp);
                flip_menu.add ( mi = new JMenuItem("Vertically") );
                mi.addActionListener(zp);
              movement_menu.add ( flip_menu );
              JMenu rotate_menu = new JMenu("Rotate");
                rotate_menu.add ( mi = new JMenuItem("90 clockwise") );
                mi.addActionListener(zp);
                rotate_menu.add ( mi = new JMenuItem("90 counterclockwise") );
                mi.addActionListener(zp);
                rotate_menu.add ( mi = new JMenuItem("180") );
                mi.addActionListener(zp);
              movement_menu.add ( rotate_menu );
              movement_menu.add ( mi = new JMenuItem("Type in...") );
              mi.addActionListener(zp);
              movement_menu.addSeparator();
              movement_menu.add ( mi = new JMenuItem("By correlation") );
              mi.addActionListener(zp);
              movement_menu.addSeparator();
              movement_menu.add ( mi = new JMenuItem("Repeat") );
              mi.addActionListener(zp);
              movement_menu.add ( mi = new JMenuItem("Propagate...") );
              mi.addActionListener(zp);
              movement_menu.addSeparator();
              JMenu record_menu = new JMenu("Record");
                record_menu.add ( mi = new JMenuItem("Start") );
                mi.addActionListener(zp);
                record_menu.addSeparator();
                record_menu.add ( mi = new JMenuItem("from selected") );
                mi.addActionListener(zp);
              movement_menu.add ( record_menu );
            section_menu.add ( movement_menu );

            menu_bar.add ( section_menu );

          JMenu domain_menu = new JMenu("Domain");
            domain_menu.add ( mi = new JMenuItem("List image domains...") );
            mi.addActionListener(zp);
            domain_menu.add ( mi = new JMenuItem("Import image...") );
            mi.addActionListener(zp);
            domain_menu.addSeparator();
            domain_menu.add ( mi = new JMenuItem("Merge front") );
            mi.addActionListener(zp);
            domain_menu.add ( mi = new JMenuItem("Merge rear") );
            mi.addActionListener(zp);
            domain_menu.add ( mi = new JMenuItem("Attributes...") );
            mi.addActionListener(zp);
            domain_menu.add ( mi = new JMenuItem("Reinitialize ->") );
            mi.addActionListener(zp);
            domain_menu.addSeparator();
            domain_menu.add ( mi = new JMenuItem("Delete") );
            mi.addActionListener(zp);
          menu_bar.add ( domain_menu );


          JMenu trace_menu = new JMenu("Trace");
            trace_menu.add ( mi = new JMenuItem("List traces...") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Find...") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Select all") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Deselect all") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Zoom to") );
            mi.addActionListener(zp);
            trace_menu.addSeparator();
            trace_menu.add ( mi = new JMenuItem("Attributes...") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Palette...") );
            mi.addActionListener(zp);
            trace_menu.addSeparator();
            trace_menu.add ( mi = new JMenuItem("Cut") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Copy") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Paste") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Paste attributes") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Delete") );
            mi.addActionListener(zp);
            trace_menu.addSeparator();
            trace_menu.add ( mi = new JMenuItem("Align Section") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Calibrate...") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Merge") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Reverse") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Simplify") );
            mi.addActionListener(zp);
            trace_menu.add ( mi = new JMenuItem("Smooth") );
            mi.addActionListener(zp);
          menu_bar.add ( trace_menu );

          JMenu object_menu = new JMenu("Object");
            object_menu.add ( mi = new JMenuItem("List Objects...") );
            mi.addActionListener(zp);
            object_menu.addSeparator();
            object_menu.add ( mi = new JMenuItem("3D Scene...") );
            mi.addActionListener(zp);
            object_menu.add ( mi = new JMenuItem("Z-Traces...") );
            mi.addActionListener(zp);
            object_menu.add ( mi = new JMenuItem("Distances...") );
            mi.addActionListener(zp);
            menu_bar.add ( object_menu );

          JMenu help_menu = new JMenu("Help");
            help_menu.add ( mi = new JMenuItem("Manual...") );
            mi.addActionListener(zp);
            help_menu.add ( mi = new JMenuItem("Key commands...") );
            mi.addActionListener(zp);
            help_menu.add ( mi = new JMenuItem("Mouse clicks...") );
            mi.addActionListener(zp);
            help_menu.addSeparator();
            help_menu.add ( mi = new JMenuItem("License...") );
            mi.addActionListener(zp);
            help_menu.add ( mi = new JMenuItem("Version...") );
            mi.addActionListener(zp);
            menu_bar.add ( help_menu );


          /*
          JMenu file_menu = new JMenu("File");
            file_menu.add ( mi = new JMenuItem("Next") );
            mi.addActionListener(zp);
            file_menu.add ( mi = new JMenuItem("Previous") );
            mi.addActionListener(zp);
            file_menu.add ( mi = new JMenuItem("Add") );
            mi.addActionListener(zp);
            file_menu.add ( mi = new JMenuItem("Remove") );
            mi.addActionListener(zp);
            file_menu.add ( mi = new JMenuItem("Exit") );
            mi.addActionListener(zp);
            menu_bar.add ( file_menu );
          */

          JMenu mode_menu = new JMenu("Mode");
            bg = new ButtonGroup();
            System.out.println ( "Initializing Drawing Mode with drawing_mode: "  + zp.drawing_mode );
            mode_menu.add ( zp.move_menu_item = mi = new JRadioButtonMenuItem("Move", !zp.drawing_mode) );
            mi.addActionListener(zp);
            bg.add ( mi );
            mode_menu.add ( zp.draw_menu_item = mi = new JRadioButtonMenuItem("Draw", zp.drawing_mode) );
            mi.addActionListener(zp);
            bg.add ( mi );
            System.out.println ( "Initializing Center Drawing check box with: "  + zp.center_draw );
            mode_menu.add ( zp.center_draw_menu_item = mi = new JCheckBoxMenuItem("Center Drawing", zp.center_draw) );
            mi.addActionListener(zp);
            mode_menu.add ( mi = new JMenuItem("Dump") );
            mi.addActionListener(zp);
            mode_menu.add ( mi = new JMenuItem("Clear") );
            mi.addActionListener(zp);
            menu_bar.add ( mode_menu );

				f.setJMenuBar ( menu_bar );
				
				zp.setBackground ( new Color (0,0,0) );
		    zp.file_chooser = new MyFileChooser ( zp.current_directory );

        try {
			    zp.image_frame = ImageIO.read(new File("test.png"));
			    // int type = zp.image_frame.getType() == 0 ? BufferedImage.TYPE_INT_ARGB : zp.image_frame.getType();
		    } catch (IOException e) {
			    System.out.println( "\n\nCannot open default \"test.png\" file:\n" + e + "\n" );
		    }

				f.add ( zp );
        zp.addKeyListener ( zp );
				f.pack();
				f.setSize ( w, h );
				f.setVisible ( true );
			  // Request the focus to make the drawing window responsive to keyboard commands without any clicking required
				zp.requestFocus();
			}
		} );
	}

}
