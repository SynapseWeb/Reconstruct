/* This Class represents a Reconstruct Series. */

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.util.Set;
import java.util.Arrays;

import java.io.*;

import java.awt.image.*;
import javax.imageio.ImageIO;

import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

import java.awt.image.*;
import javax.imageio.ImageIO;


import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;


public class SeriesClass {

  HashMap<String, String> attributes = new HashMap<String, String>();
  String series_path = null;
  Document series_doc = null;
  Document section_docs[] = null;

  BufferedImage image_frame = null;
  String image_frame_names[] = null;
  BufferedImage image_frames[] = null;


  void draw_stroke ( Graphics g, ArrayList<double[]> s, Reconstruct r ) {
    if (s.size() > 0) {
      int line_padding = 1;
      for (int xoffset=-line_padding; xoffset<=line_padding; xoffset++) {
        for (int yoffset=-line_padding; yoffset<=line_padding; yoffset++) {
          double p0[] = s.get(0);
          for (int j=1; j<s.size(); j++) {
            double p1[] = s.get(j);
            g.drawLine (  xoffset+r.x_to_pxi(p0[0]),   yoffset+r.y_to_pyi(p0[1]),  xoffset+r.x_to_pxi(p1[0]),  yoffset+r.y_to_pyi(p1[1]) );
            // System.out.println ( "   Line " + j + " = [" + p0[0] + "," + p0[1] + "] to [" + p1[0] + "," + p1[1] + "]" );
            p0 = new double[2];
            p0[0] = p1[0];
            p0[1] = p1[1];
          }
        }
      }
    }
  }

	public void paint_section (Graphics g, Reconstruct r) {
	  Dimension win_s = r.getSize();
	  int win_w = win_s.width;
	  int win_h = win_s.height;
	  if (r.recalculate) {
      r.set_scale_to_fit ( -100, 100, -100, 100, win_w, win_h );
	    r.recalculate = false;
	  }
		if (this.image_frame == null) {
		  System.out.println ( "Image is null" );
		} else {
		  // System.out.println ( "Image is NOT null" );
		  int img_w = this.image_frame.getWidth();
		  int img_h = this.image_frame.getHeight();
		  double img_wf = 200;
		  double img_hf = 200;
		  if (img_w >= img_h) {
		    // Make the image wider to fit
		    img_wf = img_w * img_wf / img_h;
		  } else {
		    // Make the height shorter to fit
		    img_hf = img_h * img_hf / img_w;
		  }
		  int draw_x = r.x_to_pxi(-img_wf/2.0);
		  int draw_y = r.y_to_pyi(-img_hf/2.0);
		  int draw_w = r.x_to_pxi(img_wf/2.0) - draw_x;
		  int draw_h = r.y_to_pyi(img_hf/2.0) - draw_y;
      g.drawImage ( this.image_frame, draw_x, draw_y, draw_w, draw_h, r );
      //g.drawImage ( this.image_frame, (win_w-img_w)/2, (win_h-img_h)/2, img_w, img_h, this );
    }

    g.setColor ( new Color ( 200, 0, 0 ) );
    for (int i=0; i<r.strokes.size(); i++) {
      // System.out.println ( " Stroke " + i );
      ArrayList<double[]> s = r.strokes.get(i);
      draw_stroke ( g, s, r );
    }
    if (r.stroke != null) {
      g.setColor ( new Color ( 255, 0, 0 ) );
      draw_stroke ( g, r.stroke, r );
    }
    if (r.center_draw) {
      g.setColor ( new Color ( 255, 255, 255 ) );
      int cx = r.getSize().width / 2;
      int cy = r.getSize().height / 2;
      g.drawLine ( cx-10, cy, cx+10, cy );
      g.drawLine ( cx, cy-10, cx, cy+10 );
    }
	}



  public SeriesClass ( File series_file ) {

    this.series_path = series_file.getParent();
    String series_file_name = series_file.getName();
    String section_file_names[] = get_section_file_names ( series_path, series_file_name.substring(0,series_file_name.length()-4) );

    this.series_doc = XML_Parser.parse_xml_file_to_doc ( series_file );

    Element series_element = this.series_doc.getDocumentElement();

    // System.out.println ( "Series is currently viewing: " + this.series_doc.getDocumentElement().getAttributes().getNamedItem("index") );
    System.out.println ( "Series is currently viewing index: " + series_element.getAttribute("index") );

    this.section_docs = new Document[section_file_names.length];

    for (int i=0; i<section_file_names.length; i++) {
      File section_file;
      System.out.println ( "============== Section File " + section_file_names[i] + " ==============" );
      section_file = new File ( series_path + File.separator + section_file_names[i] );
      this.section_docs[i] = XML_Parser.parse_xml_file_to_doc ( section_file );

      Element section_element = this.section_docs[i].getDocumentElement();

      // System.out.println ( "This section is index " + this.section_docs[i].getDocumentElement().getAttributes().getNamedItem("index").getNodeValue() );
      System.out.println ( "This section is index " + section_element.getAttribute("index") );
      System.out.println ( "===========================================" );
    }

    image_frame_names = get_image_file_names();

    System.out.println ( "Stub Constructor for SeriesClass" );
    SectionClass one_section = new SectionClass ( series_doc );
  }

  public String[] get_image_file_names() {
    System.out.println ( "Getting image file names" );
    String image_file_names[] = new String[0];
    if (this.section_docs != null) {
      for (int sect_num=0; sect_num<this.section_docs.length; sect_num++) {
        Element section_element = this.section_docs[sect_num].getDocumentElement();
        System.out.println ( "This section is index " + section_element.getAttribute("index") );
        if (section_element.hasChildNodes()) {
          System.out.println ( "  This section has child nodes" );
          NodeList child_nodes = section_element.getChildNodes();
          for (int cn=0; cn<child_nodes.getLength(); cn++) {
            Node child = child_nodes.item(cn);
            if (child.getNodeName() == "Transform") {
              System.out.println ( "    Node " + cn + " is a transform" );
              if (child.hasChildNodes()) {
                NodeList grandchild_nodes = child.getChildNodes();
                for (int gn=0; gn<grandchild_nodes.getLength(); gn++) {
                  Node grandchild = grandchild_nodes.item(gn);
                  if (grandchild.getNodeName() == "Image") {
                    System.out.println ( "      Grandchild " + gn + " is an image" );
                    System.out.println ( "         Image name is: " + ((Element)grandchild).getAttribute("src") );
                    String new_names[] = new String[image_file_names.length + 1];
                    for (int i=0; i<image_file_names.length; i++) {
                      new_names[i] = image_file_names[i];
                    }
                    try {
                      new_names[image_file_names.length] = new File ( series_path + File.separator + ((Element)grandchild).getAttribute("src") ).getCanonicalPath();
                    } catch (Exception e) {
                      System.out.println ( "Error getting path for " + ((Element)grandchild).getAttribute("src") );
                      System.exit(1);
                    }
                    image_file_names = new_names;
                  }
                }
              }
            }
          }
        }
      }
    }
    return ( image_file_names );
  }

  public void dump() {
    System.out.println ( "============== Begin Series File  ================" );
    XML_Parser.dump_doc(this.series_doc);
    // XML_Parser.dump_doc(this.series_doc);
    System.out.println ( "============== End Series File  ================" );
    if (this.section_docs != null) {
      for (int i=0; i<this.section_docs.length; i++) {
        System.out.println ( "============== Begin Section File  ================" );
        XML_Parser.dump_doc(this.section_docs[i]);
        System.out.println ( "============== End Section File  ================" );
      }
    }
  }

  public String[] get_section_file_names ( String path, String root_name ) {
    // System.out.println ( "Looking for " + root_name + " in " + path );
    File all_files[] = new File (path).listFiles();

    if (all_files==null) {
      return ( new String[0] );
    }
    if (all_files.length <= 0) {
      return ( new String[0] );
    }
    String matched_files[] = new String[all_files.length];
    int num_matched = 0;
    for (int i=0; i<all_files.length; i++) {
      matched_files[i] = null;
      String fn = all_files[i].getName();
      if ( fn.startsWith(root_name+".") ) {
        // This is a file of the form root_name.[something]
        // Verify that the "something" matches properly
        if ( fn.matches(root_name+"\\.[0123456789]+") ) {
          matched_files[i] = fn;
          // System.out.println ( "Found match: " + matched_files[i] );
          num_matched += 1;
        }
      }
    }
    String matched_names[] = new String[num_matched];
    int next_match_index = 0;
    for (int i=0; i<matched_files.length; i++) {
      if (matched_files[i] != null) {
        matched_names[next_match_index] = matched_files[i];
        next_match_index += 1;
      }
    }
    
    if (matched_names.length > 1) {
      // Sort the names so the sections will be in order (easier now than later)
      
      // Unfortunately, Arrays.sort will be sort as strings to give 1, 10, 11, 12 ... 19, 2, 20, 21 ... 29, 3, 30
      // So pad each with zeros until they are all the same length.
      
      // Get the maximum length
      int max_length = 0;
      for (int i=0; i<matched_names.length; i++) {
        int this_length = matched_names[i].length();
        if (this_length > max_length) {
          max_length = this_length;
        }
      }
      
      // Pad each to be the maximum length
      for (int i=0; i<matched_names.length; i++) {
        String s = matched_names[i];
        int this_length = s.length();
        if (this_length < max_length) {
          // Pad with zeros
          String zeros = "";
          for (int j=0; j<(max_length-this_length); j++) {
            zeros = zeros + "0";
          }
          int lastdot = s.lastIndexOf('.');
          matched_names[i] = s.substring(0,lastdot+1) + zeros + s.substring(lastdot+1);
        }
      }

      // Then sort them as strings (leading zeros will make them all comparable)
      Arrays.sort ( matched_names );
      
      // Finally, remove the leading zeros
      for (int i=0; i<matched_names.length; i++) {
        String s = matched_names[i];
        int lastdot = s.lastIndexOf('.');
        while ( s.charAt(lastdot+1) == '0' ) {
          s = s.substring(0,lastdot+1) + s.substring(lastdot+2);
        }
        matched_names[i] = s;
      }

    }

    // Print them to verify during testing
    //for (int i=0; i<matched_names.length; i++) {
    //  System.out.println ( "Sorted name: " + matched_names[i] );
    //}

    return ( matched_names );
  }


	public static void main ( String[] args ) {
		System.out.println ( "Testing SeriesClass.java ..." );
	}

}
