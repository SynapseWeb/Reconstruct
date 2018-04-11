/* This Class represents a Reconstruct Series. */

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.util.Set;



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



public class XML_Parser {

  public static String depth_string ( int depth ) {
    String s = "";
    for (int i=0; i<depth; i++) {
      s = "  " + s;
    }
    return ( s );
  }

  public static String ntn ( short node_type_code ) {
    if (node_type_code == Node.ATTRIBUTE_NODE) return ( "ATTRIBUTE_NODE" );
    if (node_type_code == Node.CDATA_SECTION_NODE) return ( "CDATA_SECTION_NODE" );
    if (node_type_code == Node.COMMENT_NODE) return ( "COMMENT_NODE" );
    if (node_type_code == Node. 	DOCUMENT_FRAGMENT_NODE) return ( " 	DOCUMENT_FRAGMENT_NODE" );
    if (node_type_code == Node.DOCUMENT_NODE) return ( "DOCUMENT_NODE" );
    if (node_type_code == Node.DOCUMENT_NODE) return ( "DOCUMENT_NODE" );
    if (node_type_code == Node.ELEMENT_NODE) return ( "ELEMENT_NODE" );
    if (node_type_code == Node.ENTITY_NODE) return ( "ENTITY_NODE" );
    if (node_type_code == Node.ENTITY_REFERENCE_NODE) return ( "ENTITY_REFERENCE_NODE" );
    if (node_type_code == Node.NOTATION_NODE) return ( "NOTATION_NODE" );
    if (node_type_code == Node.PROCESSING_INSTRUCTION_NODE) return ( "PROCESSING_INSTRUCTION_NODE" );
    if (node_type_code == Node.TEXT_NODE) return ( "TEXT_NODE" );

    if (node_type_code == Node.DOCUMENT_POSITION_CONTAINS) return ( "DOCUMENT_POSITION_CONTAINS" );
    if (node_type_code == Node.DOCUMENT_POSITION_DISCONNECTED) return ( "DOCUMENT_POSITION_DISCONNECTED" );
    if (node_type_code == Node.DOCUMENT_POSITION_FOLLOWING) return ( "DOCUMENT_POSITION_FOLLOWING" );
    if (node_type_code == Node.DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC) return ( "DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC" );
    if (node_type_code == Node.DOCUMENT_POSITION_PRECEDING) return ( "DOCUMENT_POSITION_PRECEDING" );
    if (node_type_code == Node.DOCUMENT_POSITION_PRECEDING) return ( "DOCUMENT_POSITION_PRECEDING" );

    return ( "Unknown?" );
  }

  public static String firstfew ( String full ) {
    int num = 40;
    if (full.length() < num) return ( full );
    return ( full.substring(0,num) );
  }

  public static void dump_nodes_and_atts ( Node parent, int depth ) {

    NamedNodeMap attr_map = parent.getAttributes();
    for (int index=0; index<attr_map.getLength(); index++) {
      Node node = attr_map.item(index);
      System.out.println ( depth_string(depth) + "Attr: " + ntn(node.getNodeType()) + " " + node.getNodeName() + " = " + firstfew(node.getNodeValue()) );
    }

    NodeList nodes = parent.getChildNodes();
    // System.out.println ( "Calling parent.getChildNodes() returns " + nodes.getLength() + " nodes" );
    for (int index=0; index<nodes.getLength(); index++) {
      Node node = nodes.item(index);
      System.out.println ( depth_string(depth) + "Node: " + ntn(node.getNodeType()) + " " + node.getNodeName() );
      if (node.getNodeType() == Node.ELEMENT_NODE) {
        dump_nodes_and_atts ( (Element)node, depth+1 );
      }
    }

  }

  public static void dump_doc(Document doc) {
    // Process the document tree to pull out the data for this series

    if (doc != null) {

      doc.getDocumentElement().normalize();

      String doc_name = doc.getDocumentElement().getNodeName();

      if ( ! ( ( doc_name.equalsIgnoreCase ( "Series" ) ) || ( doc_name.equalsIgnoreCase ( "Section" ) ) ) ) {
        System.out.println ( "Error: Series XML files must contain either a series or a section root element" );
      } else {

        System.out.println("Root element: " + doc.getDocumentElement().getNodeName());

        dump_nodes_and_atts ( doc.getDocumentElement(), 1 );

      }

    }

  }

  public static Document parse_xml_file_to_doc ( File f ) {
    System.out.println ( "Parsing " + f );

    Document doc = null;

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

      doc = null;

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

        String doc_name = doc.getDocumentElement().getNodeName();

        if ( ! ( ( doc_name.equalsIgnoreCase ( "Series" ) ) || ( doc_name.equalsIgnoreCase ( "Section" ) ) ) ) {
          System.out.println ( "Error: Series XML files must contain either a series or a section element" );
        } else {

          System.out.println("Root element: " + doc.getDocumentElement().getNodeName());

        }
      }
    } catch ( Exception e ) {
      System.out.println("Parsing error: " + e );
    }
    return ( doc );
  }


	public static void main ( String[] args ) {
		System.out.println ( "Testing XML_Parser.java ..." );
	}

}
