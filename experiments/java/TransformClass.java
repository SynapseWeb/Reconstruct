/* This Class represents a Reconstruct Transform. */

import java.util.Arrays;
import java.io.*;


public class TransformClass {

	int dim = 0;
	double xcoef[] = null;
	double ycoef[] = null;

	public String toString() {
		String s = "dim=" + dim;
		s = s + "  xcoef=";
		for (int i=0; i<xcoef.length; i++) {
			s = s + xcoef[i] + " ";
		}
		s = s + "  ycoef=";
		for (int i=0; i<ycoef.length; i++) {
			s = s + ycoef[i] + " ";
		}
		return ( s );
	}

  public TransformClass () {
		dim = 0;
  }

  public TransformClass ( double x0, double y0 ) {
		dim = 1;
		xcoef = new double[1];
		ycoef = new double[1];
		xcoef[0] = x0;
		ycoef[0] = y0;
  }

	double[] get_doubles ( int dim, String s ) {
		String string_parts[] = s.trim().split(" ");
		if (string_parts.length <= 0) {
			return ( new double[0] );
		} else {
			double[] d = new double[dim];
			for (int i=0; i<dim; i++) {
				try {
					d[i] = Double.parseDouble(string_parts[i]);
				} catch (Exception e) {
					d[i] = 0;
				}
			}
			return ( d );
		}
	}

  public TransformClass ( int dim, String xc_string, String yc_string ) {
		// System.out.println ( "Creating a Transform from \"" + xc_string + "\", \"" + yc_string + "\"" );
		this.dim = dim;
		this.xcoef = get_doubles ( dim, xc_string );
		this.ycoef = get_doubles ( dim, yc_string );
		if ( (xcoef.length != dim) || (ycoef.length != dim) ) {
			System.out.println ( "Error with \"" + xc_string + "\" or \"" + yc_string + "\" not of length " + dim );
		}
		// System.out.println ( "  " + this.toString() );
  }


	static void priority_println ( int thresh, String s ) {
		if (thresh >= 90) {
			System.out.println ( s );
		}
	}

	public static void main ( String[] args ) {
		priority_println ( 50, "Testing TransformClass.java ..." );
	}

}

