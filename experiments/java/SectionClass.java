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

  public SectionClass ( String p_name, String f_name ) {
    this.path_name = p_name;
    this.file_name = f_name;
		this.highest_xform_dim = 0;
		this.image_magnification = 1.0;
		TransformClass current_transform = null;

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
					current_transform = new TransformClass ( xform_dim, xform_xcoef, xform_ycoef );

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
                  String type_str = ((Element)grandchild).getAttribute("type");
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
                  boolean is_bezier = false;
                  if (type_str != null) {
										if (type_str.equals("bezier")) {
											is_bezier = true;
										}
                  }
                  ContourClass cc = new ContourClass ( stroke, ((Element)grandchild).getAttribute("border"), ((Element)grandchild).getAttribute("closed").trim().equals("true"), is_bezier );
                  cc.set_mode ( Integer.parseInt ( ((Element)grandchild).getAttribute("mode").trim() ) );
                  cc.set_hidden ( ((Element)grandchild).getAttribute("hidden").trim().equals("true") );
                  cc.set_transform ( current_transform );
                  cc.modified = false; // This is currently a way to keep contours read from XML from being duplicated.
                  contours.add ( cc );
                  priority_println ( 40, "         SectionClass: Contour points: " + ((Element)grandchild).getAttribute("points") );
                }
              }
            }
          }

        }
      } // end for (int cn=0; cn<child_nodes.getLength(); cn++)
    }

    priority_println ( 50, "============== Section File " + this.file_name + " ==============" );
    priority_println ( 50, "SectionClass: This section is index " + section_element.getAttribute("index") );
    priority_println ( 50, "===========================================" );

  }

	static void write_section_file ( String image_file_name, int width, int height ) {
		// This is an alternate place to write a section file from parameters.
		// This is currently being done in the SeriesClass.
	}

	String section_attr_names[] = {
		"index", "thickness", "alignLocked"
	};

	String transform_attr_names[] = {
		"dim", "xcoef", "ycoef"
	};

	String image_attr_names[] = {
		"mag", "contrast", "brightness", "red", "green", "blue", "src"
	};

	String contour_attr_names[] = {
		"name", "hidden", "closed", "simplified", "border", "fill", "mode", "points"
	};

	public String format_comma_sep ( String comma_sep_string, String indent_with ) {
		String comma_sep_terms[] = comma_sep_string.trim().split(",");
		String formatted = "";
		for (int i=0; i<comma_sep_terms.length; i++) {
			formatted += comma_sep_terms[i].trim() + ",\n" + indent_with;
		}
		return ( formatted );
	}

	public void write_as_xml ( File series_file ) {
		// Use the path and file name from the series file, but append the index (number) from the section file
		try {
			String new_path_name = series_file.getParentFile().getCanonicalPath();
			String ser_file_name = series_file.getName();
			String new_file_name = ser_file_name.substring(0,ser_file_name.length()-4) + file_name.substring(file_name.lastIndexOf("."),file_name.length());
			// At this point, there should be no more exceptions, so change the actual member data for this object
			this.path_name = new_path_name;
			this.file_name = new_file_name;
		  priority_println ( 100, " Writing to Section file " + this.path_name + " / " + this.file_name );

	    File section_file = new File ( this.path_name + File.separator + this.file_name );

		  PrintStream sf = new PrintStream ( section_file );
		  sf.print ( "<?xml version=\"1.0\"?>\n" );
		  sf.print ( "<!DOCTYPE Section SYSTEM \"section.dtd\">\n\n" );
		  
		  if (this.section_doc != null) {
				Element section_element = this.section_doc.getDocumentElement();
		    if ( section_element.getNodeName().equalsIgnoreCase ( "Section" ) ) {
					int seca = 0;
					sf.print ( "<" + section_element.getNodeName() );
					// Write section attributes in line
					for ( /*int seca=0 */; seca<section_attr_names.length; seca++) {
						sf.print ( " " + section_attr_names[seca] + "=\"" + section_element.getAttribute(section_attr_names[seca]) + "\"" );
					}
					sf.print ( ">\n" );

					// Handle the child nodes
					if (section_element.hasChildNodes()) {
					  NodeList child_nodes = section_element.getChildNodes();
						for (int cn=0; cn<child_nodes.getLength(); cn++) {
						  Node child = child_nodes.item(cn);
						  if (child.getNodeName().equalsIgnoreCase ( "Transform")) {
						  	Element transform_element = (Element)child;
						  	int tfa = 0;
								sf.print ( "<" + child.getNodeName() );
								for ( /*int tfa=0 */; tfa<transform_attr_names.length; tfa++) {
								  sf.print ( " " + transform_attr_names[tfa] + "=\"" + transform_element.getAttribute(transform_attr_names[tfa]) + "\"" );
									if (transform_attr_names[tfa].equals("dim") || transform_attr_names[tfa].equals("xcoef")) {
									  sf.print ( "\n" );
									}
								}
								sf.print ( ">\n" );
								if (transform_element.hasChildNodes()) {
									NodeList transform_child_nodes = transform_element.getChildNodes();
									for (int gcn=0; gcn<transform_child_nodes.getLength(); gcn++) {
										Node grandchild = transform_child_nodes.item(gcn);
										if (grandchild.getNodeName().equalsIgnoreCase ( "Image")) {
											Element image_element = (Element)grandchild;
											int ia = 0;
											sf.print ( "<" + image_element.getNodeName() );
											for ( /*int ia=0 */; ia<image_attr_names.length; ia++) {
												sf.print ( " " + image_attr_names[ia] + "=\"" + image_element.getAttribute(image_attr_names[ia]) + "\"" );
												if (image_attr_names[ia].equals("blue")) {
													sf.print ( "\n" );
												}
											}
											sf.print ( " />\n" );
										} else if (grandchild.getNodeName().equalsIgnoreCase ( "Contour")) {
											Element contour_element = (Element)grandchild;
											int ca = 0;
											sf.print ( "<" + contour_element.getNodeName() );
											for ( /*int ca=0 */; ca<contour_attr_names.length; ca++) {
												if (contour_attr_names[ca].equals("points")) {
													sf.print ( " " + contour_attr_names[ca] + "=\"" + format_comma_sep(contour_element.getAttribute(contour_attr_names[ca]),"\t") + "\"" );
												} else {
													sf.print ( " " + contour_attr_names[ca] + "=\"" + contour_element.getAttribute(contour_attr_names[ca]) + "\"" );
													if (contour_attr_names[ca].equals("mode")) {
														sf.print ( "\n" );
													}
												}
											}
											sf.print ( "/>\n" );
										}
									}
								}
								sf.print ( "</" + child.getNodeName() + ">\n\n" );
							}
						}
					}

					// Also write out any new contours created by drawing

					for (int i=0; i<contours.size(); i++) {
						ContourClass contour = contours.get(i);
						ArrayList<double[]> s = contour.stroke_points;
						if (s.size() > 0) {
							if (contour.modified) {
								if (contour.contour_name == null) {
									contour.contour_name = "RGB_";
									if (contour.r > 0.5) { contour.contour_name += "1"; } else { contour.contour_name += "0"; }
									if (contour.g > 0.5) { contour.contour_name += "1"; } else { contour.contour_name += "0"; }
									if (contour.b > 0.5) { contour.contour_name += "1"; } else { contour.contour_name += "0"; }
								}
								sf.print ( "<Transform dim=\"0\"\n" );
								sf.print ( " xcoef=\" 0 1 0 0 0 0\"\n" );
								sf.print ( " ycoef=\" 0 0 1 0 0 0\">\n" );
								String contour_color = "\"" + contour.r + " " + contour.g + " " + contour.b + "\"";
								sf.print ( "<Contour name=\"" + contour.contour_name + "\" " );
								if (contour.is_bezier) {
									sf.print ( "type=\"bezier\" " );
								}
								sf.print ( "hidden=\"false\" closed=\"true\" simplified=\"false\" border=" + contour_color + " fill=" + contour_color + " mode=\"13\"\n" );
								sf.print ( " points=\"" );
								for (int j=0; j<s.size(); j++) {
									double p[] = s.get(j);
									sf.print ( "  " + p[0] + " " + p[1] + ",\n" );
								}
								sf.print ( "  \"/>\n" );
								sf.print ( "</Transform>\n\n" );
							}
						}
					}

					sf.print ( "</" + section_element.getNodeName() + ">" );
				}
		  }
		  sf.close();

		} catch (Exception e) {
		}
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

		  int draw_x = r.x_to_pxi(0);
		  int draw_y = r.y_to_pyi(0);
		  int draw_w = r.x_to_pxi(img_wf) - draw_x;
		  int draw_h = r.y_to_pyi(img_hf) - draw_y;

      g.drawImage ( image_frame, draw_x, draw_y-draw_h, draw_w, draw_h, r );
      //g.drawImage ( image_frame, (win_w-img_w)/2, (win_h-img_h)/2, img_w, img_h, this );
    }

    g.setColor ( new Color ( 200, 0, 0 ) );

    for (int i=0; i<contours.size(); i++) {
      // priority_println ( 50, " Stroke " + i );
      ContourClass c = contours.get(i);
      c.draw ( g, r );
    }
    // if (highest_xform_dim > 0) {
      g.setColor ( new Color ( 255, 255, 255 ) );
			g.drawString ( "Transform Dimension = " + highest_xform_dim, 10, 24 );
			g.drawString ( "Image Magnification = " + image_magnification, 10, 44 );
    // }
	}


  public void add_contour ( ContourClass contour ) {
		contours.add ( contour );
  }


	public static void main ( String[] args ) {
		priority_println ( 50, "Testing SectionClass.java ..." );
		File sf = new File ("data/organelle_series/organelle_3_slice.ser");
		SeriesClass sc = new SeriesClass(sf);
	}

}

