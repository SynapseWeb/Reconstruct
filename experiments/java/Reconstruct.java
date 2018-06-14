/* This is a prototype of Reconstruct functionality. */

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
// Deprecated: import java.io.StringBufferInputStream;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;


class MyFileChooser extends JFileChooser {
  private static final long serialVersionUID = 1L;
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


public class Reconstruct extends ZoomPanLib implements ActionListener, MouseListener, MouseMotionListener, MouseWheelListener, KeyListener {
  private static final long serialVersionUID = 1L;

	JFrame parent_frame = null;
	static int w=800, h=600;

	boolean show_points = true;
	boolean drawing_mode = false;
  boolean center_draw = false;
  boolean segment_draw = true;
  boolean bezier_draw = false;
	boolean stroke_started = false;
  String current_directory = "";
  MyFileChooser file_chooser = null;
  
	int line_padding = 1;
	int new_trace_color = 0xff0000;
  
  SeriesClass series = null;
	ContourClass active_contour = null;

  String current_trace_name = null;


  void dump_strokes() {
    if (series != null) {
      series.dump_strokes();
    }
  }

	void draw_scaled_line ( Graphics g, int xoffset, int yoffset, double x0, double y0, double x1, double y1 ) {
	  g.drawLine ( xoffset+this.x_to_pxi(x0),   yoffset+this.y_to_pyi(y0),  xoffset+this.x_to_pxi(x1),  yoffset+this.y_to_pyi(y1) );
	}

  public void paint_frame (Graphics g) {
	  Dimension win_s = getSize();
	  int win_w = win_s.width;
	  int win_h = win_s.height;
	  if (recalculate) {
      set_scale_to_fit ( -10, 10, -10, 10, win_w, win_h );
	    recalculate = false;
	  }
    if (this.series != null) {
			try {
				this.series.paint_section(g, this);
			} catch (OutOfMemoryError mem_err) {
				System.out.println ( "Reconstruct.paint_frame: **** Out of Memory Error" );
			}
    }

    if (active_contour != null) {
    	active_contour.draw ( g, this );
    }
    if (this.center_draw) {
      // Draw the white center cross on top of everything
      g.setColor ( new Color ( 255, 255, 255 ) );
      int cx = this.getSize().width / 2;
      int cy = this.getSize().height / 2;
      g.drawLine ( cx-10, cy, cx+10, cy );
      g.drawLine ( cx, cy-10, cx, cy+10 );
    }

  }


  public void set_cursor() {
    if (drawing_mode) {
      current_cursor = Cursor.getPredefinedCursor ( Cursor.CROSSHAIR_CURSOR );
      if (center_draw) {
        current_cursor = Cursor.getPredefinedCursor ( Cursor.HAND_CURSOR );
      }
      if (segment_draw) {
        current_cursor = Cursor.getPredefinedCursor ( Cursor.HAND_CURSOR );
      }
      if (bezier_draw) {
        current_cursor = Cursor.getPredefinedCursor ( Cursor.CROSSHAIR_CURSOR );
      }
      setCursor ( current_cursor );
      if (draw_menu_item != null) {
        draw_menu_item.setSelected(true);
      }
    } else {
      current_cursor = b_cursor;
      setCursor ( current_cursor );
      if (move_menu_item != null) {
        move_menu_item.setSelected(true);
      }
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
      int cal = (int)(w/5);   // Arrow head length

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
      // current_cursor = b_cursor;
      // current_cursor = Cursor.getPredefinedCursor ( Cursor.MOVE_CURSOR );
      // current_cursor = Cursor.getPredefinedCursor ( Cursor.CROSSHAIR_CURSOR );
      this.set_cursor();
    }
    setCursor ( current_cursor );
  }

  public void mouseExited ( MouseEvent e ) {
    // System.out.println ( "Mouse exited" );
    super.mouseExited(e);
  }

  public void mouseClicked ( MouseEvent e ) {
    // System.out.println ( "Mouse clicked: " + e );
    if (e.getButton() == MouseEvent.BUTTON3) {
			if (segment_draw || bezier_draw) {
				if (active_contour != null) {
					active_contour.close();
					series.add_contour ( active_contour );
				}
				active_contour = null;
				// segment_draw = false;
			}
      drawing_mode = !drawing_mode;
      set_cursor();
      repaint();
    } else {
      super.mouseClicked(e);
    }
  }

  public void mousePressed ( MouseEvent e ) {
    // System.out.println ( "Mouse pressed with drawing_mode = " + drawing_mode );
    super.mousePressed(e);
    if (e.getButton() == MouseEvent.BUTTON1) {
      if (drawing_mode == true) {
				if (segment_draw) {
				} else if (bezier_draw) {
				} else {
		      if (active_contour != null) {
		        // System.out.println ( "Saving previous stroke" );
		        if (series != null) {
							active_contour.close();
							series.add_contour ( active_contour );
		        }
		      }
		    }
		    if (active_contour == null) {
	        // System.out.println ( "Making new stroke" );
	        active_contour = new ContourClass ( new ArrayList<double[]>(100), new_trace_color, false, bezier_draw );
	        active_contour.contour_name = current_trace_name;
	        active_contour.is_bezier = bezier_draw;
	      }
        double p[] = { px_to_x(e.getX()), py_to_y(e.getY()) };
        if (center_draw) {
          p[0] = px_to_x(getSize().width / 2);
          p[1] = py_to_y(getSize().height / 2);
        }
        // System.out.println ( "Adding point " + p[0] + "," + p[1] );
        double contour_point[] = { p[0], -p[1] };
        active_contour.add_point ( contour_point );
        repaint();
      }
    }
  }

  public void mouseReleased ( MouseEvent e ) {
    // System.out.println ( "Mouse released" );
    if (drawing_mode == false) {
      super.mouseReleased(e);
    } else {
      if (active_contour != null) {
        double p[] = { px_to_x(e.getX()), py_to_y(e.getY()) };
        if (segment_draw) {
		      // ?? active_contour.add_point ( p );
		    } else if (bezier_draw) {
        } else {
		      if (center_draw) {
		        p[0] = px_to_x(getSize().width / 2);
		        p[1] = py_to_y(getSize().height / 2);
		      }
	        double contour_point[] = { p[0], -p[1] };
		      active_contour.add_point ( contour_point );
		      if (series != null) {
						active_contour.close();
						series.add_contour ( active_contour );
			    }
		      active_contour = null;
		    }
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
      if (segment_draw) {
				// Ignore mouse drags
			} else if (bezier_draw) {
      } else {
		    if (center_draw) {
		      super.mouseDragged(e);
		    }
		    if (active_contour == null) {
		      active_contour  = new ContourClass ( new ArrayList<double[]>(100), new_trace_color, false, bezier_draw );
	        active_contour.is_bezier = bezier_draw;
		    }
		    if (active_contour != null) {
		      double p[] = { px_to_x(e.getX()), py_to_y(e.getY()) };
		      if (center_draw) {
		        p[0] = px_to_x(getSize().width / 2);
		        p[1] = py_to_y(getSize().height / 2);
		      }
	        double contour_point[] = { p[0], -p[1] };
		      active_contour.add_point ( contour_point );
		      repaint();
		    }
		  }
    }
  }

  public void mouseMoved ( MouseEvent e ) {
    super.mouseMoved ( e );
  }

  // MouseWheelListener methods:
	public void mouseWheelMoved ( MouseWheelEvent e ) {
		if (this.series == null) {
			// Don't change the section index in this case
		} else {
		  if (drawing_mode == true) {
		    // scroll_wheel_position += e.getWheelRotation();
		    int scroll_wheel_delta = -e.getWheelRotation();
		    int section_index = this.series.position_by_n_sections ( scroll_wheel_delta );
		    if (this.parent_frame != null) {
		      this.parent_frame.setTitle ( "Series " + this.series.get_short_name() + ", Section " + (section_index+1) );
			  }
			} else {
			  super.mouseWheelMoved ( e );
			}
		}
		repaint();
	}


  JMenuItem move_menu_item = null;
  JMenuItem draw_menu_item = null;
  JMenuItem center_draw_menu_item = null;
  JMenuItem segment_draw_menu_item = null;
  JMenuItem bezier_draw_menu_item = null;

	JMenuItem new_series_menu_item=null;
	JMenuItem open_series_menu_item=null;
	JMenuItem save_series_menu_item=null;
	JMenuItem import_images_menu_item=null;
	JMenuItem list_sections_menu_item=null;

	JMenuItem series_options_menu_item=null;

  JMenuItem line_menu_none_item = null;
  JMenuItem line_menu_0_item = null;
  JMenuItem line_menu_1_item = null;
  JMenuItem line_menu_2_item = null;

  JMenuItem color_menu_red_item = null;
  JMenuItem color_menu_green_item = null;
  JMenuItem color_menu_blue_item = null;
  JMenuItem color_menu_yellow_item = null;
  JMenuItem color_menu_magenta_item = null;
  JMenuItem color_menu_cyan_item = null;
  JMenuItem color_menu_black_item = null;
  JMenuItem color_menu_white_item = null;

  JMenuItem color_menu_half_item = null;

  JMenuItem color_menu_light_item = null;
  JMenuItem color_menu_dark_item = null;


  JMenuItem purge_images_menu_item = null;
	JMenuItem dump_menu_item=null;
	JMenuItem clear_menu_item=null;

	JMenuItem exit_menu_item=null;

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
    if ( (e.getKeyCode() == KeyEvent.VK_PAGE_DOWN) || (e.getKeyCode() == KeyEvent.VK_PAGE_UP  ) ) {
      // Figure out if there's anything to do
      if (this.series != null) {
        int delta = 0;
        if (e.getKeyCode() == KeyEvent.VK_PAGE_UP) {
          //System.out.println ( "Page Up" );
          delta = 1;
        } else if (e.getKeyCode() == KeyEvent.VK_PAGE_DOWN) {
          //System.out.println ( "Page Down" );
          delta = -1;
        }
        if (delta != 0) {
          int section_index = this.series.position_by_n_sections ( delta );
		      if (this.parent_frame != null) {
			      this.parent_frame.setTitle ( "Series " + this.series.get_short_name() + ", Section " + (section_index+1) );
			    }
          repaint();
        }
      }
    } else if ( e.getKeyCode() == KeyEvent.VK_HOME ) {
      this.recalculate = true;
      repaint();
    } else {
      System.out.println ( "Key Pressed, e = " + e );
    }
    //super.keyPressed ( e );
  }
  public void keyReleased ( KeyEvent e ) {
    // System.out.println ( "Key Released" );
    //super.keyReleased ( e );
  }


	String new_series_file_name = null;

  // ActionPerformed methods (mostly menu responses):

	public void actionPerformed(ActionEvent e) {
		Object action_source = e.getSource();

		String cmd = e.getActionCommand();
		// System.out.println ( "ActionPerformed got \"" + cmd + "\"" );
		// System.out.println ( "ActionPerformed got \"" + action_source + "\"" );

		if ( action_source == move_menu_item ) {
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
		} else if ( action_source == draw_menu_item ) {
      current_cursor = Cursor.getPredefinedCursor ( Cursor.CROSSHAIR_CURSOR );
      if (center_draw) {
        current_cursor = Cursor.getPredefinedCursor ( Cursor.HAND_CURSOR );
      }
      setCursor ( current_cursor );
		  // center_draw = false;
		  drawing_mode = true;
		  stroke_started = false;
		  repaint();
		} else if ( action_source == center_draw_menu_item ) {
		  JCheckBoxMenuItem item = (JCheckBoxMenuItem)e.getSource();
		  center_draw = item.getState();
		  if (drawing_mode) {
        current_cursor = Cursor.getPredefinedCursor ( Cursor.HAND_CURSOR );
      }
		  repaint();
		} else if ( action_source == segment_draw_menu_item ) {
		  JCheckBoxMenuItem item = (JCheckBoxMenuItem)e.getSource();
		  segment_draw = item.getState();
		  if (segment_draw) {
        current_cursor = Cursor.getPredefinedCursor ( Cursor.HAND_CURSOR );
      }
		  repaint();
		} else if ( action_source == bezier_draw_menu_item ) {
		  JCheckBoxMenuItem item = (JCheckBoxMenuItem)e.getSource();
		  bezier_draw = item.getState();
		  if (bezier_draw) {
        current_cursor = Cursor.getPredefinedCursor ( Cursor.HAND_CURSOR );
      }
		  repaint();
		} else if ( action_source == new_series_menu_item ) {
		  file_chooser.setMultiSelectionEnabled(false);
		  FileNameExtensionFilter filter = new FileNameExtensionFilter("Series Files", "ser");
		  file_chooser.setFileFilter(filter);
		  if (file_chooser.getName() == null) {
			  // Try to set the default file name of newSeries.ser
		  }
		  int returnVal = file_chooser.showDialog(this, "New Series");
		  if ( returnVal == JFileChooser.APPROVE_OPTION ) {
		    File series_file = file_chooser.getSelectedFile();
        System.out.println ( "You chose to create this file: " /* + chooser.getCurrentDirectory() + " / " */ + series_file );
				try {
		      DataOutputStream f = new DataOutputStream ( new FileOutputStream ( series_file ) );
		      f.writeBytes ( ReconstructDefaults.default_series_file_string );
		      f.close();
		      this.new_series_file_name = series_file.getPath();
		      this.series = new SeriesClass();
				} catch (Exception oe) {
					System.out.println ( "Error while writing a series file:\n" + oe );
					oe.printStackTrace();
					JOptionPane.showMessageDialog(null, "File write error", "File Write Error", JOptionPane.WARNING_MESSAGE);
				}
				repaint();
		  }
		} else if ( action_source == open_series_menu_item ) {
		  file_chooser.setMultiSelectionEnabled(false);
		  FileNameExtensionFilter filter = new FileNameExtensionFilter("Series Files", "ser");
		  file_chooser.setFileFilter(filter);
		  int returnVal = file_chooser.showDialog(this, "Open Series");
		  if ( returnVal == JFileChooser.APPROVE_OPTION ) {
		    File series_file = file_chooser.getSelectedFile();
        System.out.println ( "You chose to open this file: " /* + chooser.getCurrentDirectory() + " / " */ + series_file );

				try {
					this.series = new SeriesClass ( series_file );
				} catch (Exception oe) {
					// this.series.image_frame = null;
					System.out.println ( "Error while opening a series file:\n" + oe );
					oe.printStackTrace();
					JOptionPane.showMessageDialog(null, "File error", "File Path Error", JOptionPane.WARNING_MESSAGE);
				}
				repaint();
		  }
		} else if ( action_source == save_series_menu_item ) {
		  file_chooser.setMultiSelectionEnabled(false);
		  FileNameExtensionFilter filter = new FileNameExtensionFilter("Series Files", "ser");
		  file_chooser.setFileFilter(filter);
		  int returnVal = file_chooser.showDialog(this, "Save Series");
		  if ( returnVal == JFileChooser.APPROVE_OPTION ) {
		    File series_file = file_chooser.getSelectedFile();
        System.out.println ( "You chose to save to this file: " /* + chooser.getCurrentDirectory() + " / " */ + series_file );
				try {
					this.series.write_as_xml ( series_file );
				} catch (Exception oe) {
					// this.series.image_frame = null;
					System.out.println ( "Error while saving a series file:\n" + oe );
					oe.printStackTrace();
					JOptionPane.showMessageDialog(null, "File error", "File Path Error", JOptionPane.WARNING_MESSAGE);
				}
				repaint();
		  }
		} else if ( action_source == series_options_menu_item ) {
			// Open the Series Options Dialog
			//JOptionPane.showMessageDialog(null, "Give new traces this name:", "Series Options", JOptionPane.WARNING_MESSAGE);
			String response = JOptionPane.showInputDialog(null, "Give new traces this name:", "Series Options", JOptionPane.QUESTION_MESSAGE);
			current_trace_name = null;
			if (response != null) {
				if (response.length() > 0) {
					current_trace_name = response;
					System.out.println ( "Series Options Current Trace Name = " + response );
				}
			}
	    repaint();
		} else if ( action_source == line_menu_none_item ) {
			line_padding = -1; // This signals to not draw at all
	    repaint();
		} else if ( action_source == line_menu_0_item ) {
			line_padding = 0;
	    repaint();
		} else if ( action_source == line_menu_1_item ) {
			line_padding = 1;
	    repaint();
		} else if ( action_source == line_menu_2_item ) {
			line_padding = 2;
	    repaint();
		} else if ( action_source == dump_menu_item ) {
      if (this.series != null) {
				System.out.println ( ">>>>>>>>>>>>>>>>>>> Action: dump_menu_item" );
				System.out.println ( ">>>>>>>>>>>>>>>>>>> dump_xml" );
				this.series.dump_xml();
				System.out.println ( ">>>>>>>>>>>>>>>>>>> dump_strokes" );
				this.series.dump_strokes();
      }

		} else if ( action_source == color_menu_red_item ) {
		  new_trace_color=0xff0000;
	    repaint();
		} else if ( action_source == color_menu_green_item ) {
		  new_trace_color=0x00ff00;
	    repaint();
		} else if ( action_source == color_menu_blue_item ) {
		  new_trace_color=0x0000ff;
	    repaint();
		} else if ( action_source == color_menu_yellow_item ) {
		  new_trace_color=0xffff00;
	    repaint();
		} else if ( action_source == color_menu_magenta_item ) {
		  new_trace_color=0xff00ff;
	    repaint();
		} else if ( action_source == color_menu_cyan_item ) {
		  new_trace_color=0x00ffff;
	    repaint();
		} else if ( action_source == color_menu_black_item ) {
		  new_trace_color=0x000000;
	    repaint();
		} else if ( action_source == color_menu_white_item ) {
		  new_trace_color=0xffffff;
	    repaint();
		} else if ( action_source == color_menu_half_item ) {
			// System.out.println ( "color_menu_half_item = " + color_menu_half_item );
		  new_trace_color=0x00777777 & new_trace_color;
	    repaint();
		} else if ( action_source == color_menu_light_item ) {
			int r = (new_trace_color & 0xff0000) >> 16;
			int g = (new_trace_color & 0x00ff00) >>  8;
			int b = (new_trace_color & 0x0000ff);
			r = 4 * r / 3; if (r > 255) r = 255;
			g = 4 * g / 3; if (g > 255) g = 255;
			b = 4 * b / 3; if (b > 255) b = 255;
		  new_trace_color = (r << 16) + (g << 8) + b;
	    repaint();
		} else if ( action_source == color_menu_dark_item ) {
			int r = (new_trace_color & 0xff0000) >> 16;
			int g = (new_trace_color & 0x00ff00) >>  8;
			int b = (new_trace_color & 0x0000ff);
			r = 3 * r / 4; if (r < 0) r = 0;
			g = 3 * g / 4; if (g < 0) g = 0;
			b = 3 * b / 4; if (b < 0) b = 0;
		  new_trace_color = (r << 16) + (g << 8) + b;
	    repaint();
		} else if ( action_source == purge_images_menu_item ) {
      if (this.series != null) {
        this.series.purge_images();
      }
	    active_contour = null;
	    repaint();
		} else if ( action_source == clear_menu_item ) {
      if (this.series != null) {
        this.series.clear_strokes();
      }
	    active_contour = null;
	    repaint();
		} else if ( action_source == import_images_menu_item ) {
		  file_chooser.setMultiSelectionEnabled(true);
		  FileNameExtensionFilter filter = new FileNameExtensionFilter("Image Files", "jpg", "gif", "png", "tiff");
		  file_chooser.setFileFilter(filter);
		  int returnVal = file_chooser.showDialog(this, "Import Images");
		  if ( returnVal == JFileChooser.APPROVE_OPTION ) {
		    File image_files[] = file_chooser.getSelectedFiles();
        System.out.println ( "You chose to import these " + image_files.length + " images:" );
        for (int i=0; i<image_files.length; i++) {
	        System.out.println ( "  " + image_files[i] );
	      }
	      // Reconstruct.exe creates the section files as soon as the images have been imported so do it now
	      this.series = new SeriesClass ( this.new_series_file_name );
				this.series.import_images ( image_files );
		  }
		} else if ( action_source == list_sections_menu_item ) {
		  System.out.println ( "Sections: ..." );
		} else if ( action_source == exit_menu_item ) {
			System.exit ( 0 );
		} else {
	    JOptionPane.showMessageDialog(null, "Option Not Implemented", "Option Not Implemented", JOptionPane.WARNING_MESSAGE);
	  }
    if ( (this.parent_frame != null) && (this.series != null) ) {
      this.parent_frame.setTitle ( "Series " + this.series.get_short_name() + ", Section " + (this.series.get_position()+1) );
	  }

  }


	public static void main ( String[] args ) {
	  for (int i=0; i<args.length; i++) {
		  System.out.println ( "Arg[" + i + "] = \"" + args[i] + "\"" );
		}
		System.out.println ( "Reconstruct: Use the mouse wheel to zoom, and drag to pan." );
		javax.swing.SwingUtilities.invokeLater ( new Runnable() {
			public void run() {
			  JFrame f = new JFrame("Reconstruct - No Active Series");
				f.setDefaultCloseOperation ( JFrame.EXIT_ON_CLOSE );
				
        Reconstruct zp = new Reconstruct();
        zp.parent_frame = f;
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
              debug_menu.add ( zp.dump_menu_item = mi = new JMenuItem("Dump") );
              mi.addActionListener(zp);
              debug_menu.add ( zp.clear_menu_item = mi = new JMenuItem("Clear") );
              mi.addActionListener(zp);
            program_menu.add ( debug_menu );

            program_menu.add ( zp.exit_menu_item = mi = new JMenuItem("Exit") );
            mi.addActionListener(zp);
            menu_bar.add ( program_menu );

          JMenu series_menu = new JMenu("Series");
            series_menu.add ( mi = new JMenuItem("Open...") );
            zp.open_series_menu_item = mi;
            mi.addActionListener(zp);
            series_menu.add ( mi = new JMenuItem("Close") );
            mi.addActionListener(zp);

            series_menu.addSeparator();

            series_menu.add ( mi = new JMenuItem("New...") );
            zp.new_series_menu_item = mi;
            mi.addActionListener(zp);
            series_menu.add ( mi = new JMenuItem("Save") );
            zp.save_series_menu_item = mi;
            mi.addActionListener(zp);
            series_menu.add ( zp.series_options_menu_item = mi = new JMenuItem("Options...") );
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
              import_menu.add ( zp.import_images_menu_item = mi = new JMenuItem("Images...") );
              mi.addActionListener(zp);
              import_menu.add ( mi = new JMenuItem("Lines...") );
              mi.addActionListener(zp);
              import_menu.add ( mi = new JMenuItem("Trace lists...") );
              mi.addActionListener(zp);
            series_menu.add ( import_menu );

            menu_bar.add ( series_menu );

          JMenu section_menu = new JMenu("Section");
            section_menu.add ( zp.list_sections_menu_item = mi = new JMenuItem("List Sections...") );
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

          JMenu extras_menu = new JMenu("Extras");

		        JMenu color_menu = new JMenu("Color");
		          bg = new ButtonGroup();
		          color_menu.add ( zp.color_menu_red_item = mi = new JRadioButtonMenuItem("Red", zp.new_trace_color==0xff0000) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          color_menu.add ( zp.color_menu_green_item = mi = new JRadioButtonMenuItem("Green", zp.new_trace_color==0x00ff00) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          color_menu.add ( zp.color_menu_blue_item = mi = new JRadioButtonMenuItem("Blue", zp.new_trace_color==0x0000ff) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          color_menu.add ( zp.color_menu_yellow_item = mi = new JRadioButtonMenuItem("Yellow", zp.new_trace_color==0xffff00) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          color_menu.add ( zp.color_menu_magenta_item = mi = new JRadioButtonMenuItem("Magenta", zp.new_trace_color==0xff00ff) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          color_menu.add ( zp.color_menu_cyan_item = mi = new JRadioButtonMenuItem("Cyan", zp.new_trace_color==0x00ffff) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          color_menu.add ( zp.color_menu_black_item = mi = new JRadioButtonMenuItem("Black", zp.new_trace_color==0x000000) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          color_menu.add ( zp.color_menu_white_item = mi = new JRadioButtonMenuItem("White", zp.new_trace_color==0xffffff) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
              color_menu.addSeparator();
		          color_menu.add ( zp.color_menu_half_item = mi = new JMenuItem("Half") );
		          mi.addActionListener(zp);
		          bg.add ( mi );
              color_menu.addSeparator();
		          color_menu.add ( zp.color_menu_light_item = mi = new JMenuItem("Lighter") );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          color_menu.add ( zp.color_menu_dark_item = mi = new JMenuItem("Darker") );
		          mi.addActionListener(zp);
		          bg.add ( mi );
            extras_menu.add ( color_menu );

		        JMenu line_menu = new JMenu("Line");
		          bg = new ButtonGroup();
		          line_menu.add ( zp.line_menu_none_item = mi = new JRadioButtonMenuItem("None", zp.line_padding==-1) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          line_menu.add ( zp.line_menu_0_item = mi = new JRadioButtonMenuItem("Width = 1", zp.line_padding==0) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          line_menu.add ( zp.line_menu_1_item = mi = new JRadioButtonMenuItem("Width = 3", zp.line_padding==1) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          line_menu.add ( zp.line_menu_2_item = mi = new JRadioButtonMenuItem("Width = 5", zp.line_padding==2) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
            extras_menu.add ( line_menu );

            extras_menu.addSeparator();

		        JMenu mode_menu = new JMenu("Mode");
		          bg = new ButtonGroup();
		          mode_menu.add ( zp.move_menu_item = mi = new JRadioButtonMenuItem("Move", !zp.drawing_mode) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          mode_menu.add ( zp.draw_menu_item = mi = new JRadioButtonMenuItem("Draw", zp.drawing_mode) );
		          mi.addActionListener(zp);
		          bg.add ( mi );
		          mode_menu.add ( zp.segment_draw_menu_item = mi = new JCheckBoxMenuItem("Segment Drawing", zp.segment_draw) );
		          mi.addActionListener(zp);
		          mode_menu.add ( zp.bezier_draw_menu_item = mi = new JCheckBoxMenuItem("Bezier Drawing", zp.bezier_draw) );
		          mi.addActionListener(zp);
		          mode_menu.add ( zp.center_draw_menu_item = mi = new JCheckBoxMenuItem("Center Drawing", zp.center_draw) );
		          mi.addActionListener(zp);
		          // mode_menu.add ( zp.dump_menu_item );
		          // mi.addActionListener(zp);
		          // mode_menu.add ( mi = new JMenuItem("Clear") );
		          // mi.addActionListener(zp);
            extras_menu.add ( mode_menu );

            extras_menu.addSeparator();

            extras_menu.add ( zp.purge_images_menu_item = mi = new JMenuItem("Purge Images") );
            mi.addActionListener(zp);

	          menu_bar.add ( extras_menu );

				f.setJMenuBar ( menu_bar );
				
				zp.setBackground ( new Color (0,0,0) );
		    zp.file_chooser = new MyFileChooser ( zp.current_directory );

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
