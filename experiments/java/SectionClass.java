/* This Class represents a Reconstruct Section. */

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.util.Set;

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


public class SectionClass {

  BufferedImage image_frames[] = null;
  String image_frame_names[] = null;

  public SectionClass ( Document section_doc ) {

    BufferedImage image_frames[] = null;
    String image_frame_names[] = null;

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

	public void paint_section (Graphics g, Reconstruct r) {
	  Dimension win_s = r.getSize();
	  int win_w = win_s.width;
	  int win_h = win_s.height;
	  if (r.recalculate) {
      r.set_scale_to_fit ( -100, 100, -100, 100, win_w, win_h );
	    r.recalculate = false;
	  }
		if (r.image_frame == null) {
		  System.out.println ( "Image is null" );
		} else {
		  // System.out.println ( "Image is NOT null" );
		  int img_w = r.image_frame.getWidth();
		  int img_h = r.image_frame.getHeight();
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
      g.drawImage ( r.image_frame, draw_x, draw_y, draw_w, draw_h, r );
      //g.drawImage ( r.image_frame, (win_w-img_w)/2, (win_h-img_h)/2, img_w, img_h, this );
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

