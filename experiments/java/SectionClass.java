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

  public SectionClass ( String p_name, String f_name ) {
    this.path_name = p_name;
    this.file_name = f_name;

    File section_file = new File ( this.path_name + File.separator + this.file_name );

    this.section_doc = XML_Parser.parse_xml_file_to_doc ( section_file );

    Element section_element = this.section_doc.getDocumentElement();

    System.out.println ( "SectionClass: This section is index " + section_element.getAttribute("index") );
    if (section_element.hasChildNodes()) {
      System.out.println ( "  SectionClass: This section has child nodes" );
      NodeList child_nodes = section_element.getChildNodes();
      for (int cn=0; cn<child_nodes.getLength(); cn++) {
        Node child = child_nodes.item(cn);
        if (child.getNodeName() == "Transform") {
          System.out.println ( "    SectionClass: Node " + cn + " is a transform" );
          if (child.hasChildNodes()) {
            NodeList grandchild_nodes = child.getChildNodes();
            for (int gn=0; gn<grandchild_nodes.getLength(); gn++) {
              Node grandchild = grandchild_nodes.item(gn);
              if (grandchild.getNodeName() == "Image") {
                System.out.println ( "      SectionClass: Grandchild " + gn + " is an image" );
                System.out.println ( "         SectionClass: Image name is: " + ((Element)grandchild).getAttribute("src") );
                String new_names[] = new String[image_file_names.length + 1];
                for (int i=0; i<image_file_names.length; i++) {
                  new_names[i] = image_file_names[i];
                }
                try {
                  new_names[image_file_names.length] = new File ( this.path_name + File.separator + ((Element)grandchild).getAttribute("src") ).getCanonicalPath();
                } catch (Exception e) {
                  System.out.println ( "SectionClass: Error getting path for " + ((Element)grandchild).getAttribute("src") );
                  System.exit(1);
                }
                image_file_names = new_names;
              }
            }
          }
        }
      }
    }

    System.out.println ( "============== Section File " + this.file_name + " ==============" );
    // System.out.println ( "This section is index " + this.section_docs[i].getDocumentElement().getAttributes().getNamedItem("index").getNodeValue() );
    System.out.println ( "SectionClass: This section is index " + section_element.getAttribute("index") );
    System.out.println ( "===========================================" );

  }

  public BufferedImage get_image() {
    if (section_image == null) {
      try {
        System.out.println ( " SectionClass: Opening ... " + this.image_file_names[0] );
        File image_file = new File ( this.image_file_names[0] );
        this.section_image = ImageIO.read(image_file);
      } catch (Exception oe) {
        // this.series.image_frame = null;
        System.out.println ( "SectionClass: Error while opening an image file:\n" + this.image_file_names[0] );
        JOptionPane.showMessageDialog(null, "SectionClass: File error", "SectionClass: File Path Error", JOptionPane.WARNING_MESSAGE);
      }
    }
    return ( section_image );
  }

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


	public void paint_section (Graphics g, Reconstruct r, SeriesClass series) {
	  BufferedImage image_frame = get_image();
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
		  int draw_x = r.x_to_pxi(-img_wf/2.0);
		  int draw_y = r.y_to_pyi(-img_hf/2.0);
		  int draw_w = r.x_to_pxi(img_wf/2.0) - draw_x;
		  int draw_h = r.y_to_pyi(img_hf/2.0) - draw_y;
      g.drawImage ( image_frame, draw_x, draw_y, draw_w, draw_h, r );
      //g.drawImage ( image_frame, (win_w-img_w)/2, (win_h-img_h)/2, img_w, img_h, this );
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




	public static void main ( String[] args ) {
		System.out.println ( "Testing SectionClass.java ..." );
	}

}

