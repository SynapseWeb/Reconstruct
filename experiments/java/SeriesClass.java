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
  String series_file_name = null;
  Document series_doc = null;

  SectionClass sections[] = null;
  int section_index = 0;

  public SeriesClass () {
    this.series_file_name = null;
    this.series_doc = null;
    this.section_index = 0;
  }

  public SeriesClass ( String series_file_name ) {
    this.series_file_name = series_file_name;
    this.series_doc = null;
    this.section_index = 0;
  }

  public SeriesClass ( File series_file ) {
		this.load_from_xml ( series_file );
  }

	public void load_from_xml ( File series_file ) {
    this.series_path = series_file.getParent();

    String series_file_name = series_file.getName();


    this.series_doc = XML_Parser.parse_xml_file_to_doc ( series_file );

    String section_file_names[] = get_section_file_names ( series_path, series_file_name.substring(0,series_file_name.length()-4) );


    Element series_element = this.series_doc.getDocumentElement();
    System.out.println ( "Series is currently viewing index: " + series_element.getAttribute("index") );

    sections = new SectionClass[section_file_names.length];

    for (int i=0; i<section_file_names.length; i++) {
      File section_file;

      section_file = new File ( series_path + File.separator + section_file_names[i] );
      sections[i] = new SectionClass ( series_path, section_file_names[i] );
    }

    section_index = 0;
	}

	String series_attr_names[] = {
		"index", "viewport", "units", "autoSaveSeries", "autoSaveSection", "warnSaveSection",
		"beepDeleting", "beepPaging", "hideTraces", "unhideTraces", "hideDomains", "unhideDomains",
		"useAbsolutePaths", "defaultThickness", "zMidSection", "thumbWidth", "thumbHeight",
		"fitThumbSections", "firstThumbSection", "lastThumbSection", "skipSections", "displayThumbContours",
		"useFlipbookStyle", "flipRate", "useProxies", "widthUseProxies", "heightUseProxies", "scaleProxies",
		"significantDigits", "defaultBorder", "defaultFill", "defaultMode", "defaultName", "defaultComment",
		"listSectionThickness", "listDomainSource", "listDomainPixelsize", "listDomainLength", "listDomainArea",
		"listDomainMidpoint", "listTraceComment", "listTraceLength", "listTraceArea", "listTraceCentroid",
		"listTraceExtent", "listTraceZ", "listTraceThickness", "listObjectRange", "listObjectCount",
		"listObjectSurfarea", "listObjectFlatarea", "listObjectVolume", "listZTraceNote", "listZTraceRange",
		"listZTraceLength", "borderColors", "fillColors", "offset3D", "type3Dobject", "first3Dsection",
		"last3Dsection", "max3Dconnection", "upper3Dfaces", "lower3Dfaces", "faceNormals", "vertexNormals",
		"facets3D", "dim3D", "gridType", "gridSize", "gridDistance", "gridNumber", "hueStopWhen", "hueStopValue",
		"satStopWhen", "satStopValue", "brightStopWhen", "brightStopValue", "tracesStopWhen", "areaStopPercent",
		"areaStopSize", "ContourMaskWidth", "smoothingLength", "mvmtIncrement", "ctrlIncrement", "shiftIncrement"
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
		// In order to guarantee repeatability, this version exports "by hand" rather than using XML library functions
		System.out.println ( "Writing XML to file " + series_file.getName() );
    try {
		  PrintStream sf = new PrintStream ( series_file );
		  sf.print ( "<?xml version=\"1.0\"?>\n" );
		  sf.print ( "<!DOCTYPE Series SYSTEM \"series.dtd\">\n\n" );
		  if (this.series_doc != null) {
				Element series_element = this.series_doc.getDocumentElement();
		    if ( series_element.getNodeName().equalsIgnoreCase ( "Series" ) ) {
					int sa = 0;
					sf.print ( "<" + series_element.getNodeName() );
					if ( series_attr_names[sa].equals("index") ) {  // Put the index on the opening line
						sf.print ( " " + series_attr_names[sa] + "=\"" + series_element.getAttribute(series_attr_names[sa]) + "\"" );
						sa += 1;
						if ( series_attr_names[sa].equals("viewport") ) {  // Put the viewport on the opening line
							sf.print ( " " + series_attr_names[sa] + "=\"" + series_element.getAttribute(series_attr_names[sa]) + "\"" );
							sa += 1;
						}
					}
					sf.print ( "\n" );
					// Handle the remaining attributes normally
					for ( /*int sa=0 */; sa<series_attr_names.length; sa++) {
						if (series_attr_names[sa].equals("borderColors") || series_attr_names[sa].equals("fillColors")) {
							sf.print ( "\t" + series_attr_names[sa] + "=\"" + format_comma_sep(series_element.getAttribute(series_attr_names[sa]),"\t\t") + "\"\n" );
						} else {
							sf.print ( "\t" + series_attr_names[sa] + "=\"" + series_element.getAttribute(series_attr_names[sa]) + "\"\n" );
						}
					}
					sf.print ( "\t>\n" );
					// Handle the child nodes
					if (series_element.hasChildNodes()) {
					  NodeList child_nodes = series_element.getChildNodes();
						for (int cn=0; cn<child_nodes.getLength(); cn++) {
						  Node child = child_nodes.item(cn);
						  if (child.getNodeName().equalsIgnoreCase ( "Contour")) {
								int ca = 0;
								sf.print ( "<" + child.getNodeName() );
								sf.print ( " name=\"" + ((Element)child).getAttribute("name") + "\"" );
								sf.print ( " closed=\"" + ((Element)child).getAttribute("closed") + "\"" );
								sf.print ( " border=\"" + ((Element)child).getAttribute("border") + "\"" );
								sf.print ( " fill=\"" + ((Element)child).getAttribute("fill") + "\"" );
								sf.print ( " mode=\"" + ((Element)child).getAttribute("mode") + "\"\n" );
								sf.print ( " points=\"" + format_comma_sep(((Element)child).getAttribute("points"),"\t") + "\"/>\n" );
						  }
						}
					}
					sf.print ( "</" + series_element.getNodeName() + ">\n" );
				}
		  }
		  sf.close();
		  if (this.sections != null) {
			  for (int i=0; i<sections.length; i++) {
		      this.sections[i].write_as_xml(series_file);
		    }
		  }
		} catch (Exception e) {
			System.out.println ( "Error writing to file " + series_file.getName() );
		}
	}

  public void import_images ( File image_files[] ) {
    // This version builds its own XML files to eventually use "load_from_xml".
		if (series_file_name != null) {
			System.out.println ( "Importing images into " + series_file_name );
			File series_file = new File(series_file_name);
			String series_prefix = series_file_name.substring(0,series_file_name.length()-3);  // Subtract off the "ser" at the end of "name.ser"

		  for (int i=0; i<image_files.length; i++) {
		    System.out.println ( "  Importing " + image_files[i] + " into " + series_prefix + (i+1) );
		    try {
				  DataOutputStream f = new DataOutputStream ( new FileOutputStream ( series_prefix + (i+1) ) );
				  f.writeBytes ( ReconstructDefaults.default_section_file_string_1 );
				  f.writeBytes ( image_files[i].getName() );
				  f.writeBytes ( ReconstructDefaults.default_section_file_string_2 );
				  f.writeBytes ( "0 0,\n" );
				  f.writeBytes ( "	  479 0,\n" );
				  f.writeBytes ( "	  479 479,\n" );
				  f.writeBytes ( "	  0 479,\n" );
				  f.writeBytes ( ReconstructDefaults.default_section_file_string_3 );
				  f.close();
				} catch (Exception e) {
					System.out.println ( "Error writing to file " + image_files[i] + " into " + series_prefix + (i+1) );
				}
		  }
			this.load_from_xml ( series_file );
		}
  }

  public void dump_xml() {
    System.out.println ( "============== Begin Series ================" );
    XML_Parser.dump_doc(this.series_doc);
    // XML_Parser.dump_doc(this.series_doc);
    System.out.println ( "============== End Series ================" );
    if (this.sections != null) {
	    for (int i=0; i<sections.length; i++) {
        System.out.println ( "============== Begin Section ================" );
        XML_Parser.dump_doc(this.sections[i].section_doc);
        System.out.println ( "============== End Section ================" );
      }
    }
  }

  public void reverse_all_strokes() {
		System.out.println ( "Reversing all Traces ..." );
	  if (sections != null) {
	    for (int i=0; i<sections.length; i++) {
		  	System.out.println ( "    Reversing Section " + i + " Traces ..." );
	      sections[i].reverse_all_strokes();
	    }
    }
  }

  public void dump_strokes() {
		System.out.println ( "Dumping Strokes:" );
	  if (sections != null) {
	    if (section_index < sections.length) {
		  	System.out.println ( "    Dumping Section Stroke:" );
	      sections[section_index].dump_strokes();
	    }
    }
  }

  public void purge_images() {
	  if (sections != null) {
	    for (int i=0; i<sections.length; i++) {
	      sections[i].purge_images();
	    }
    }
  }

  public void clear_strokes() {
	  if (sections != null) {
	    if (section_index < sections.length) {
	      sections[section_index].clear_strokes();
	    }
    }
  }

  public void add_contour (	ContourClass contour ) {
	  if (sections != null) {
	    if (section_index < sections.length) {
	      sections[section_index].add_contour ( contour );
	    }
	  }
  }

	public int get_position() {
    return ( section_index );
	}

	public String get_short_name() {
	  if (series_file_name != null) {
	    return ( new File(series_file_name).getName() );
	  } else {
			return ( "No Series" );
	  }
	}

  public int position_by_n_sections ( int n ) {
		if (sections == null) {
			section_index = 0;
		} else {
		  section_index += n;
		  if (section_index < 0) {
		    section_index = 0;
		  } else if (section_index >= sections.length) {
		    section_index = sections.length - 1;
		  }
		}
    return ( section_index );
  }

	public void paint_section (Graphics g, Reconstruct r) {
	  // BufferedImage image_frame = null;
	  if (sections != null) {
	    if (section_index < sections.length) {
				OutOfMemoryError last_mem_err = null;
				boolean section_painted = false;
				int fartherest_section_index = ( section_index + (sections.length/2) ) % sections.length;
				int delta = 0;
				int purge_1 = (fartherest_section_index+delta) % sections.length;
				int purge_2 = (fartherest_section_index-delta) % sections.length;
				do {
					try {
						sections[section_index].paint_section ( g, r, this );
						section_painted = true;
					} catch (OutOfMemoryError mem_err) {
						// Attempt to remove images fartherest away from this (assuming circular indexing)
						System.out.println ( "         SeriesClass.paint_section: **** Out of Memory Error, try purging images on sections " + purge_1 + " and " + purge_2 );
						sections[purge_1].purge_images();
						sections[purge_2].purge_images();
						if ( (purge_1 != section_index) && (purge_2 != section_index) ) {
							delta += 1;
						}
						last_mem_err = mem_err;
					}
				} while ( (section_painted == false) && (purge_1 != section_index) && (purge_2 != section_index) );
				if (section_painted == false) {
					System.out.println ( "SeriesClass.paint_section: **** Out of Memory Error" );
					throw ( last_mem_err );
				}
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
        while ( (s.length() > lastdot+2) && (s.charAt(lastdot+1) == '0') ) {
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
