/* This Class represents a Reconstruct Series. */

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.util.Set;

import java.io.*;

import java.awt.image.*;
import javax.imageio.ImageIO;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;


public class SeriesClass {

  HashMap<String, String> attributes = new HashMap<String, String>();
  Document series_doc = null;
  Document section_docs[] = null;

  public SeriesClass ( File series_file ) {

    String series_file_name = series_file.getName();
    String section_file_names[] = get_section_file_names ( series_file.getParent(), series_file_name.substring(0,series_file_name.length()-4) );

    this.series_doc = XML_Parser.parse_xml_file_to_doc ( series_file );

    Element series_element = this.series_doc.getDocumentElement();

    // System.out.println ( "Series is currently viewing: " + this.series_doc.getDocumentElement().getAttributes().getNamedItem("index") );
    System.out.println ( "Series is currently viewing index: " + series_element.getAttribute("index") );

    this.section_docs = new Document[section_file_names.length];

    for (int i=0; i<section_file_names.length; i++) {
      File section_file;
      System.out.println ( "============== Section File " + section_file_names[i] + " ==============" );
      section_file = new File ( series_file.getParent() + File.separator + section_file_names[i] );
      this.section_docs[i] = XML_Parser.parse_xml_file_to_doc ( section_file );

      Element section_element = this.section_docs[i].getDocumentElement();

      // System.out.println ( "This section is index " + this.section_docs[i].getDocumentElement().getAttributes().getNamedItem("index").getNodeValue() );
      System.out.println ( "This section is index " + section_element.getAttribute("index") );
      System.out.println ( "===========================================" );
    }

    System.out.println ( "Stub Constructor for SeriesClass" );
    SectionClass one_section = new SectionClass ( series_doc );

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
          System.out.println ( "Found match: " + matched_files[i] );
          num_matched += 1;
        }
      }
    }
    String matched_names[] = new String[num_matched];
    int j = 0;
    for (int i=0; i<matched_files.length; i++) {
      if (matched_files[i] != null) {
        matched_names[j] = matched_files[i];
        j += 1;
      }
    }
    return ( matched_names );
  }


	public static void main ( String[] args ) {
		System.out.println ( "Testing SeriesClass.java ..." );
	}

}
