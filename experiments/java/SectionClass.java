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

	public static void main ( String[] args ) {
		System.out.println ( "Testing SectionClass.java ..." );
	}

}

