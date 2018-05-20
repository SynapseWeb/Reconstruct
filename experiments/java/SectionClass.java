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


public class SectionClass {

  String path_name = null;
  String file_name = null;
  Document section_doc = null;

  BufferedImage section_image = null;
  String image_file_names[] = new String[0];
  ArrayList<String> bad_image_file_names = new ArrayList<String>();

  int highest_xform_dim = 0;
  double image_magnification = 1.0; // This is in units per pixel, equivalent to 1 / (pixels per unit)

	// ArrayList<ArrayList<double[]>> strokes = new ArrayList<ArrayList<double[]>>();  // Argument (if any) specifies initial capacity (default 10)
	ArrayList<ContourClass> contours = new ArrayList<ContourClass>();  // Argument (if any) specifies initial capacity (default 10)

	static void priority_println ( int thresh, String s ) {
		if (thresh >= 90) {
			System.out.println ( s );
		}
	}

	static void write_section_file ( String image_file_name, int width, int height ) {
		// This is an alternate place to write a section file from parameters.
		// This is currently being done in the SeriesClass.
	}

  public SectionClass ( String p_name, String f_name ) {
    this.path_name = p_name;
    this.file_name = f_name;
		this.highest_xform_dim = 0;
		this.image_magnification = 1.0;

    File section_file = new File ( this.path_name + File.separator + this.file_name );

    this.section_doc = XML_Parser.parse_xml_file_to_doc ( section_file );

    Element section_element = this.section_doc.getDocumentElement();

    priority_println ( 50, "SectionClass: This section is index " + section_element.getAttribute("index") );
    if (section_element.hasChildNodes()) {
      priority_println ( 50, "  SectionClass: This section has child nodes" );
      NodeList child_nodes = section_element.getChildNodes();
      for (int cn=0; cn<child_nodes.getLength(); cn++) {
        Node child = child_nodes.item(cn);
        if (child.getNodeName() == "Transform") {
          priority_println ( 50, "    SectionClass: Node " + cn + " is a transform" );
          int xform_dim = Integer.parseInt( ((Element)child).getAttribute("dim") );
          String xform_xcoef = ((Element)child).getAttribute("xcoef");
          String xform_ycoef = ((Element)child).getAttribute("ycoef");
          priority_println ( 50, "      dim = \"" + xform_dim + "\"" );
          priority_println ( 50, "      xcoef = " + xform_xcoef );
          priority_println ( 50, "      ycoef = " + xform_ycoef );
					if ( (!(xform_dim == 0)) && (!(xform_dim == 1)) )  {
						priority_println ( 100, "Transforms must be 0 or 1 dimension in this version." );
						JOptionPane.showMessageDialog(null, "Error: Dim=" + xform_dim + ", transforms must be 0 or 1 dimension in this version.", "SectionClass: Dim error", JOptionPane.WARNING_MESSAGE);
					}
					if (xform_dim > highest_xform_dim) {
						highest_xform_dim = xform_dim;
					}

          if (child.hasChildNodes()) {
            NodeList grandchild_nodes = child.getChildNodes();
            boolean is_image = false;
            for (int gn=0; gn<grandchild_nodes.getLength(); gn++) {
              if (grandchild_nodes.item(gn).getNodeName() == "Image") {
                is_image = true;
                break;
              }
            }
            if (is_image) {
              // This transform should contain an "Image" node and ONE "Contour" node
              for (int gn=0; gn<grandchild_nodes.getLength(); gn++) {
                Node grandchild = grandchild_nodes.item(gn);
                if (grandchild.getNodeName() == "Image") {
                  priority_println ( 40, "      SectionClass: Grandchild " + gn + " is an image" );
                  priority_println ( 40, "         SectionClass: Image name is: " + ((Element)grandchild).getAttribute("src") );
                  String new_names[] = new String[image_file_names.length + 1];
                  for (int i=0; i<image_file_names.length; i++) {
                    new_names[i] = image_file_names[i];
                  }
                  try {
                    new_names[image_file_names.length] = new File ( this.path_name + File.separator + ((Element)grandchild).getAttribute("src") ).getCanonicalPath();
                  } catch (Exception e) {
                    priority_println ( 20, "SectionClass: Error getting path for " + ((Element)grandchild).getAttribute("src") );
                    System.exit(1);
                  }
                  try {
                    this.image_magnification = Double.parseDouble ( ((Element)grandchild).getAttribute("mag") );
                  } catch (Exception e) {
                    priority_println ( 20, "SectionClass: Error getting image magnification for " + ((Element)grandchild).getAttribute("src") );
                    this.image_magnification = 1.0;
                  }
                  image_file_names = new_names;
                } else if (grandchild.getNodeName() == "Contour") {
                  priority_println ( 40, "      SectionClass: Grandchild " + gn + " is an image perimeter contour" );
                  priority_println ( 40, "         SectionClass: Contour name is: " + ((Element)grandchild).getAttribute("name") );
                }
              }
            } else {
              // This transform should contain one or more "Contour" nodes
              for (int gn=0; gn<grandchild_nodes.getLength(); gn++) {
                Node grandchild = grandchild_nodes.item(gn);
                if (grandchild.getNodeName() == "Contour") {
                  priority_println ( 40, "      SectionClass: Grandchild " + gn + " is a trace contour" );
                  priority_println ( 40, "         SectionClass: Contour name is: " + ((Element)grandchild).getAttribute("name") );
                  String points_str = ((Element)grandchild).getAttribute("points");
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
                  contours.add ( new ContourClass ( stroke, ((Element)grandchild).getAttribute("border"), ((Element)grandchild).getAttribute("closed").trim().equals("true") ) );
                  priority_println ( 40, "         SectionClass: Contour points: " + ((Element)grandchild).getAttribute("points") );
                }
              }
            }
          }
        }
      }
    }

    priority_println ( 50, "============== Section File " + this.file_name + " ==============" );
    // priority_println ( 50, "This section is index " + this.section_docs[i].getDocumentElement().getAttributes().getNamedItem("index").getNodeValue() );
    priority_println ( 50, "SectionClass: This section is index " + section_element.getAttribute("index") );
    priority_println ( 50, "===========================================" );

  }


  public void dump_strokes() {
		System.out.println ( "Dumping Contours for a Section:" );
		/*
    for (int i=0; i<strokes.size(); i++) {
      priority_println ( 150, " Stroke " + i );
      ArrayList<double[]> s = strokes.get(i);
	    for (int j=0; j<s.size(); j++) {
	      double p[] = s.get(j);
	      priority_println ( 150, "   Point " + j + " = [" + p[0] + "," + p[1] + "]" );
	    }
    }
    */
    for (int i=0; i<contours.size(); i++) {
      priority_println ( 150, " Contour " + i );
      ContourClass contour = contours.get(i);
      ArrayList<double[]> s = contour.stroke_points;
	    for (int j=0; j<s.size(); j++) {
	      double p[] = s.get(j);
	      priority_println ( 150, "   Point " + j + " = [" + p[0] + "," + p[1] + "]" );
	    }
    }
  }

  public void clear_strokes() {
    // strokes = new ArrayList<ArrayList<double[]>>();
  }

  public void add_stroke (	ArrayList<double[]> stroke ) {
    // strokes.add ( stroke );
  }


  public BufferedImage get_image() throws OutOfMemoryError {
    if (section_image == null) {
      try {
        priority_println ( 50, " SectionClass: Opening ... " + this.image_file_names[0] );
        File image_file = new File ( this.image_file_names[0] );
        this.section_image = ImageIO.read(image_file);
      } catch (OutOfMemoryError mem_err) {
        // this.series.image_frame = null;
        priority_println ( 100, "SectionClass: **** Out of Memory Error while opening an image file:\n   " + this.image_file_names[0] );
        throw ( mem_err );
      } catch (Exception oe) {
        // this.series.image_frame = null;
        boolean found = false;
				for (int i=0; i<bad_image_file_names.size(); i++) {
				  String s = bad_image_file_names.get(i);
				  if (this.image_file_names[0].equals(s)) {
						found = true;
						break;
				  }
				}
				if (!found) {
					// Notify of missing file and put in list to be ignored in the future
					priority_println ( 100, "SectionClass: Error while opening an image file:\n   " + this.image_file_names[0] );
					JOptionPane.showMessageDialog(null, "Cannot open " + this.image_file_names[0], "SectionClass: File Error", JOptionPane.WARNING_MESSAGE);
					bad_image_file_names.add ( this.image_file_names[0] );
	      }
      }
    }
    return ( section_image );
  }

	public boolean purge_images() {
		if (section_image == null) {
			return ( false );
		} else {
			section_image = null;
			return ( true );
		}
	}

	public void draw_scaled_line ( Graphics g, Reconstruct r, int xoffset, int yoffset, double x0, double y0, double x1, double y1 ) {
	  g.drawLine ( xoffset+r.x_to_pxi(x0),   yoffset+r.y_to_pyi(-y0),  xoffset+r.x_to_pxi(x1),  yoffset+r.y_to_pyi(-y1) );
	}

  public void draw_stroke ( Graphics g, ArrayList<double[]> s, Reconstruct r, boolean closed ) {
    if (s.size() > 0) {
      int line_padding = r.line_padding;
      double p0[] = null;
      double p1[] = null;
      if (line_padding >= 0) {
		    for (int xoffset=-line_padding; xoffset<=line_padding; xoffset++) {
		      for (int yoffset=-line_padding; yoffset<=line_padding; yoffset++) {
		        p0 = s.get(0);
		        for (int j=1; j<s.size(); j++) {
		          p1 = s.get(j);
		          draw_scaled_line ( g, r, xoffset, yoffset, p0[0], p0[1], p1[0], p1[1] );
		          p0 = new double[2];
		          p0[0] = p1[0];
		          p0[1] = p1[1];
		        }
		        if (closed) {
							p1 = s.get(0);
		          draw_scaled_line ( g, r, xoffset, yoffset, p0[0], p0[1], p1[0], p1[1] );
		        }
		      }
		    }
		  }
    }
  }


	public void paint_section (Graphics g, Reconstruct r, SeriesClass series) throws OutOfMemoryError {
	  BufferedImage image_frame = get_image();
		if (image_frame == null) {
		  priority_println ( 50, "Image is null" );
		} else {
		  // priority_println ( 50, "Image is NOT null" );
		  int img_w = image_frame.getWidth();
		  int img_h = image_frame.getHeight();
		  double img_wf = img_w * image_magnification;
		  double img_hf = img_h * image_magnification;
		  /*
		  if (img_w >= img_h) {
		    // Make the image wider to fit
		    img_wf = img_w * img_wf / img_h;
		  } else {
		    // Make the height shorter to fit
		    img_hf = img_h * img_hf / img_w;
		  }
		  */
		  /*
		  int draw_x = r.x_to_pxi(-img_wf/2.0);
		  int draw_y = r.y_to_pyi(-img_hf/2.0);
		  int draw_w = r.x_to_pxi(img_wf/2.0) - draw_x;
		  int draw_h = r.y_to_pyi(img_hf/2.0) - draw_y;
		  */

		  int draw_x = r.x_to_pxi(0);
		  int draw_y = r.y_to_pyi(0);
		  int draw_w = r.x_to_pxi(img_wf) - draw_x;
		  int draw_h = r.y_to_pyi(img_hf) - draw_y;

      g.drawImage ( image_frame, draw_x, draw_y-draw_h, draw_w, draw_h, r );
      //g.drawImage ( image_frame, (win_w-img_w)/2, (win_h-img_h)/2, img_w, img_h, this );
    }

    g.setColor ( new Color ( 200, 0, 0 ) );
    /*
    for (int i=0; i<strokes.size(); i++) {
      // priority_println ( 50, " Stroke " + i );
      ArrayList<double[]> s = strokes.get(i);
      draw_stroke ( g, s, r, true );
    }
    */
    for (int i=0; i<contours.size(); i++) {
      // priority_println ( 50, " Stroke " + i );
      ContourClass c = contours.get(i);
      g.setColor ( new Color ( (int)(255*c.r), (int)(255*c.g), (int)(255*c.b) ) );
      ArrayList<double[]> s = c.stroke_points;
      draw_stroke ( g, s, r, c.closed );
    }
    if (r.stroke != null) {
      g.setColor ( new Color ( 255, 0, 0 ) );
      draw_stroke ( g, r.stroke, r, false );
    }
    if (r.center_draw) {
      g.setColor ( new Color ( 255, 255, 255 ) );
      int cx = r.getSize().width / 2;
      int cy = r.getSize().height / 2;
      g.drawLine ( cx-10, cy, cx+10, cy );
      g.drawLine ( cx, cy-10, cx, cy+10 );
    }
    // if (highest_xform_dim > 0) {
      g.setColor ( new Color ( 255, 255, 255 ) );
			g.drawString ( "Transform Dimension = " + highest_xform_dim, 10, 24 );
			g.drawString ( "Image Magnification = " + image_magnification, 10, 44 );
    // }
	}




	public static void main ( String[] args ) {
		priority_println ( 50, "Testing SectionClass.java ..." );
		File sf = new File ("data/organelle_series/organelle_3_slice.ser");
		SeriesClass sc = new SeriesClass(sf);
		Element sec0 = sc.section_docs[0].getDocumentElement();
		priority_println ( 50, "NodeName = " + sec0.getNodeName() );
		priority_println ( 50, "thickness = " + sec0.getAttribute("thickness") );
		priority_println ( 50, "index = " + sec0.getAttribute("index") );
		String sfnames[] = sc.get_section_file_names(sf.getParent(), sf.getName().substring(0,sf.getName().length()-4));
		priority_println ( 50, "sfnames:" );
		for (int i=0; i<sfnames.length; i++) {
			priority_println ( 50, "  " + sfnames[i] );
		}
		Element se = sc.section_docs[0].getDocumentElement();
	}

}

