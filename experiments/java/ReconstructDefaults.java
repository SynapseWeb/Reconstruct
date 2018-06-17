class ReconstructDefaults {

  public static String current_newline_string = "\n";  // For Windows this may be set to: "\r\n"

  public static String convert_newlines ( String s ) {
    return ( s.replaceAll ( "\n", current_newline_string ) );
  }


	public static String default_series_file_string =
		"<?xml version=\"1.0\"?>\n" +
		"<!DOCTYPE Series SYSTEM \"series.dtd\">\n" +
		"\n" +
		"<Series index=\"1\" viewport=\"0 0 0.00275105\"\n" +
		"	units=\"microns\"\n" +
		"	autoSaveSeries=\"true\"\n" +
		"	autoSaveSection=\"true\"\n" +
		"	warnSaveSection=\"true\"\n" +
		"	beepDeleting=\"true\"\n" +
		"	beepPaging=\"true\"\n" +
		"	hideTraces=\"false\"\n" +
		"	unhideTraces=\"false\"\n" +
		"	hideDomains=\"false\"\n" +
		"	unhideDomains=\"false\"\n" +
		"	useAbsolutePaths=\"false\"\n" +
		"	defaultThickness=\"0.05\"\n" +
		"	zMidSection=\"false\"\n" +
		"	thumbWidth=\"128\"\n" +
		"	thumbHeight=\"96\"\n" +
		"	fitThumbSections=\"false\"\n" +
		"	firstThumbSection=\"1\"\n" +
		"	lastThumbSection=\"2147483647\"\n" +
		"	skipSections=\"1\"\n" +
		"	displayThumbContours=\"true\"\n" +
		"	useFlipbookStyle=\"false\"\n" +
		"	flipRate=\"5\"\n" +
		"	useProxies=\"true\"\n" +
		"	widthUseProxies=\"2048\"\n" +
		"	heightUseProxies=\"1536\"\n" +
		"	scaleProxies=\"0.25\"\n" +
		"	significantDigits=\"6\"\n" +
		"	defaultBorder=\"1.000 0.000 1.000\"\n" +
		"	defaultFill=\"1.000 0.000 1.000\"\n" +
		"	defaultMode=\"9\"\n" +
		"	defaultName=\"domain$+\"\n" +
		"	defaultComment=\"\"\n" +
		"	listSectionThickness=\"true\"\n" +
		"	listDomainSource=\"true\"\n" +
		"	listDomainPixelsize=\"true\"\n" +
		"	listDomainLength=\"false\"\n" +
		"	listDomainArea=\"false\"\n" +
		"	listDomainMidpoint=\"false\"\n" +
		"	listTraceComment=\"true\"\n" +
		"	listTraceLength=\"false\"\n" +
		"	listTraceArea=\"true\"\n" +
		"	listTraceCentroid=\"false\"\n" +
		"	listTraceExtent=\"false\"\n" +
		"	listTraceZ=\"false\"\n" +
		"	listTraceThickness=\"false\"\n" +
		"	listObjectRange=\"true\"\n" +
		"	listObjectCount=\"true\"\n" +
		"	listObjectSurfarea=\"false\"\n" +
		"	listObjectFlatarea=\"false\"\n" +
		"	listObjectVolume=\"false\"\n" +
		"	listZTraceNote=\"true\"\n" +
		"	listZTraceRange=\"true\"\n" +
		"	listZTraceLength=\"true\"\n" +
		"	borderColors=\"0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		\"" +
		"	fillColors=\"0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		0.000 0.000 0.000,\n" +
		"		\"" +
		"	offset3D=\"0 0 0\"\n" +
		"	type3Dobject=\"0\"\n" +
		"	first3Dsection=\"1\"\n" +
		"	last3Dsection=\"2147483647\"\n" +
		"	max3Dconnection=\"-1\"\n" +
		"	upper3Dfaces=\"true\"\n" +
		"	lower3Dfaces=\"true\"\n" +
		"	faceNormals=\"false\"\n" +
		"	vertexNormals=\"true\"\n" +
		"	facets3D=\"8\"\n" +
		"	dim3D=\"-1 -1 -1\"\n" +
		"	gridType=\"0\"\n" +
		"	gridSize=\"1 1\"\n" +
		"	gridDistance=\"1 1\"\n" +
		"	gridNumber=\"1 1\"\n" +
		"	hueStopWhen=\"3\"\n" +
		"	hueStopValue=\"50\"\n" +
		"	satStopWhen=\"3\"\n" +
		"	satStopValue=\"50\"\n" +
		"	brightStopWhen=\"0\"\n" +
		"	brightStopValue=\"100\"\n" +
		"	tracesStopWhen=\"false\"\n" +
		"	areaStopPercent=\"999\"\n" +
		"	areaStopSize=\"0\"\n" +
		"	ContourMaskWidth=\"0\"\n" +
		"	smoothingLength=\"7\"\n" +
		"	mvmtIncrement=\"0.022 1 1 1.01 1.01 0.02 0.02 0.001 0.001\"\n" +
		"	ctrlIncrement=\"0.0044 0.01 0.01 1.002 1.002 0.004 0.004 0.0002 0.0002\"\n" +
		"	shiftIncrement=\"0.11 100 100 1.05 1.05 0.1 0.1 0.005 0.005\"\n" +
		"	>\n" +
		"<Contour name=\"a$+\" closed=\"true\" border=\"1.000 0.500 0.000\" fill=\"1.000 0.500 0.000\" mode=\"13\"\n" +
		" points=\"-3 1,\n" +
		"	-3 -1,\n" +
		"	-1 -3,\n" +
		"	1 -3,\n" +
		"	3 -1,\n" +
		"	3 1,\n" +
		"	1 3,\n" +
		"	-1 3,\n" +
		"	\"/>\n" +
		"<Contour name=\"b$+\" closed=\"true\" border=\"0.500 0.000 1.000\" fill=\"0.500 0.000 1.000\" mode=\"13\"\n" +
		" points=\"-2 1,\n" +
		"	-5 0,\n" +
		"	-2 -1,\n" +
		"	-4 -4,\n" +
		"	-1 -2,\n" +
		"	0 -5,\n" +
		"	1 -2,\n" +
		"	4 -4,\n" +
		"	2 -1,\n" +
		"	5 0,\n" +
		"	2 1,\n" +
		"	4 4,\n" +
		"	1 2,\n" +
		"	0 5,\n" +
		"	-1 2,\n" +
		"	-4 4,\n" +
		"	\"/>\n" +
		"<Contour name=\"pink$+\" closed=\"true\" border=\"1.000 0.000 0.500\" fill=\"1.000 0.000 0.500\" mode=\"-13\"\n" +
		" points=\"-6 -6,\n" +
		"	6 -6,\n" +
		"	0 5,\n" +
		"	\"/>\n" +
		"<Contour name=\"X$+\" closed=\"true\" border=\"1.000 0.000 0.000\" fill=\"1.000 0.000 0.000\" mode=\"-13\"\n" +
		" points=\"-7 7,\n" +
		"	-2 0,\n" +
		"	-7 -7,\n" +
		"	-4 -7,\n" +
		"	0 -1,\n" +
		"	4 -7,\n" +
		"	7 -7,\n" +
		"	2 0,\n" +
		"	7 7,\n" +
		"	4 7,\n" +
		"	0 1,\n" +
		"	-4 7,\n" +
		"	\"/>\n" +
		"<Contour name=\"yellow$+\" closed=\"true\" border=\"1.000 1.000 0.000\" fill=\"1.000 1.000 0.000\" mode=\"-13\"\n" +
		" points=\"8 8,\n" +
		"	8 -8,\n" +
		"	-8 -8,\n" +
		"	-8 6,\n" +
		"	-10 8,\n" +
		"	-10 -10,\n" +
		"	10 -10,\n" +
		"	10 10,\n" +
		"	-10 10,\n" +
		"	-8 8,\n" +
		"	\"/>\n" +
		"<Contour name=\"blue$+\" closed=\"true\" border=\"0.000 0.000 1.000\" fill=\"0.000 0.000 1.000\" mode=\"9\"\n" +
		" points=\"0 7,\n" +
		"	-7 0,\n" +
		"	0 -7,\n" +
		"	7 0,\n" +
		"	\"/>\n" +
		"<Contour name=\"magenta$+\" closed=\"true\" border=\"1.000 0.000 1.000\" fill=\"1.000 0.000 1.000\" mode=\"9\"\n" +
		" points=\"-6 2,\n" +
		"	-6 -2,\n" +
		"	-2 -6,\n" +
		"	2 -6,\n" +
		"	6 -2,\n" +
		"	6 2,\n" +
		"	2 6,\n" +
		"	-2 6,\n" +
		"	\"/>\n" +
		"<Contour name=\"red$+\" closed=\"true\" border=\"1.000 0.000 0.000\" fill=\"1.000 0.000 0.000\" mode=\"9\"\n" +
		" points=\"6 -6,\n" +
		"	0 -6,\n" +
		"	0 -3,\n" +
		"	3 0,\n" +
		"	12 3,\n" +
		"	6 6,\n" +
		"	3 12,\n" +
		"	-3 6,\n" +
		"	-6 0,\n" +
		"	-6 -6,\n" +
		"	-12 -6,\n" +
		"	-3 -12,\n" +
		"	\"/>\n" +
		"<Contour name=\"green$+\" closed=\"true\" border=\"0.000 1.000 0.000\" fill=\"0.000 1.000 0.000\" mode=\"9\"\n" +
		" points=\"-12 4,\n" +
		"	-12 -4,\n" +
		"	-4 -4,\n" +
		"	-4 -12,\n" +
		"	4 -12,\n" +
		"	4 -4,\n" +
		"	12 -4,\n" +
		"	12 4,\n" +
		"	4 4,\n" +
		"	4 12,\n" +
		"	-4 12,\n" +
		"	-4 4,\n" +
		"	\"/>\n" +
		"<Contour name=\"cyan$+\" closed=\"true\" border=\"0.000 1.000 1.000\" fill=\"0.000 1.000 1.000\" mode=\"9\"\n" +
		" points=\"0 12,\n" +
		"	4 8,\n" +
		"	-12 -8,\n" +
		"	-8 -12,\n" +
		"	8 4,\n" +
		"	12 0,\n" +
		"	12 12,\n" +
		"	\"/>\n" +
		"<Contour name=\"a$+\" closed=\"true\" border=\"1.000 0.500 0.000\" fill=\"1.000 0.500 0.000\" mode=\"13\"\n" +
		" points=\"-3 1,\n" +
		"	-3 -1,\n" +
		"	-1 -3,\n" +
		"	1 -3,\n" +
		"	3 -1,\n" +
		"	3 1,\n" +
		"	1 3,\n" +
		"	-1 3,\n" +
		"	\"/>\n" +
		"<Contour name=\"b$+\" closed=\"true\" border=\"0.500 0.000 1.000\" fill=\"0.500 0.000 1.000\" mode=\"13\"\n" +
		" points=\"-2 1,\n" +
		"	-5 0,\n" +
		"	-2 -1,\n" +
		"	-4 -4,\n" +
		"	-1 -2,\n" +
		"	0 -5,\n" +
		"	1 -2,\n" +
		"	4 -4,\n" +
		"	2 -1,\n" +
		"	5 0,\n" +
		"	2 1,\n" +
		"	4 4,\n" +
		"	1 2,\n" +
		"	0 5,\n" +
		"	-1 2,\n" +
		"	-4 4,\n" +
		"	\"/>\n" +
		"<Contour name=\"pink$+\" closed=\"true\" border=\"1.000 0.000 0.500\" fill=\"1.000 0.000 0.500\" mode=\"-13\"\n" +
		" points=\"-6 -6,\n" +
		"	6 -6,\n" +
		"	0 5,\n" +
		"	\"/>\n" +
		"<Contour name=\"X$+\" closed=\"true\" border=\"1.000 0.000 0.000\" fill=\"1.000 0.000 0.000\" mode=\"-13\"\n" +
		" points=\"-7 7,\n" +
		"	-2 0,\n" +
		"	-7 -7,\n" +
		"	-4 -7,\n" +
		"	0 -1,\n" +
		"	4 -7,\n" +
		"	7 -7,\n" +
		"	2 0,\n" +
		"	7 7,\n" +
		"	4 7,\n" +
		"	0 1,\n" +
		"	-4 7,\n" +
		"	\"/>\n" +
		"<Contour name=\"yellow$+\" closed=\"true\" border=\"1.000 1.000 0.000\" fill=\"1.000 1.000 0.000\" mode=\"-13\"\n" +
		" points=\"8 8,\n" +
		"	8 -8,\n" +
		"	-8 -8,\n" +
		"	-8 6,\n" +
		"	-10 8,\n" +
		"	-10 -10,\n" +
		"	10 -10,\n" +
		"	10 10,\n" +
		"	-10 10,\n" +
		"	-8 8,\n" +
		"	\"/>\n" +
		"<Contour name=\"blue$+\" closed=\"true\" border=\"0.000 0.000 1.000\" fill=\"0.000 0.000 1.000\" mode=\"9\"\n" +
		" points=\"0 7,\n" +
		"	-7 0,\n" +
		"	0 -7,\n" +
		"	7 0,\n" +
		"	\"/>\n" +
		"<Contour name=\"magenta$+\" closed=\"true\" border=\"1.000 0.000 1.000\" fill=\"1.000 0.000 1.000\" mode=\"9\"\n" +
		" points=\"-6 2,\n" +
		"	-6 -2,\n" +
		"	-2 -6,\n" +
		"	2 -6,\n" +
		"	6 -2,\n" +
		"	6 2,\n" +
		"	2 6,\n" +
		"	-2 6,\n" +
		"	\"/>\n" +
		"<Contour name=\"red$+\" closed=\"true\" border=\"1.000 0.000 0.000\" fill=\"1.000 0.000 0.000\" mode=\"9\"\n" +
		" points=\"6 -6,\n" +
		"	0 -6,\n" +
		"	0 -3,\n" +
		"	3 0,\n" +
		"	12 3,\n" +
		"	6 6,\n" +
		"	3 12,\n" +
		"	-3 6,\n" +
		"	-6 0,\n" +
		"	-6 -6,\n" +
		"	-12 -6,\n" +
		"	-3 -12,\n" +
		"	\"/>\n" +
		"<Contour name=\"green$+\" closed=\"true\" border=\"0.000 1.000 0.000\" fill=\"0.000 1.000 0.000\" mode=\"9\"\n" +
		" points=\"-12 4,\n" +
		"	-12 -4,\n" +
		"	-4 -4,\n" +
		"	-4 -12,\n" +
		"	4 -12,\n" +
		"	4 -4,\n" +
		"	12 -4,\n" +
		"	12 4,\n" +
		"	4 4,\n" +
		"	4 12,\n" +
		"	-4 12,\n" +
		"	-4 4,\n" +
		"	\"/>\n" +
		"<Contour name=\"cyan$+\" closed=\"true\" border=\"0.000 1.000 1.000\" fill=\"0.000 1.000 1.000\" mode=\"9\"\n" +
		" points=\"0 12,\n" +
		"	4 8,\n" +
		"	-12 -8,\n" +
		"	-8 -12,\n" +
		"	8 4,\n" +
		"	12 0,\n" +
		"	12 12,\n" +
		"	\"/>\n" +
		"</Series>";



	public static String default_section_file_string_1 =
		"<?xml version=\"1.0\"?>\n" +
		"<!DOCTYPE Section SYSTEM \"section.dtd\">\n" +
		"\n" +
		"<Section index=\"1\" thickness=\"0.05\" alignLocked=\"false\">\n" +
		"<Transform dim=\"0\"\n" +
		"  xcoef=\" 0 1 0 0 0 0\"\n" +
		"  ycoef=\" 0 0 1 0 0 0\">\n" +
		"  <Image mag=\"0.00254\" contrast=\"1\" brightness=\"0\" red=\"true\" green=\"true\" blue=\"true\"\n" +
		"   src=\"";
		
	public static String default_section_file_string_2 =
		"\" />\n" +
		"  <Contour name=\"domain1\" hidden=\"false\" closed=\"true\" simplified=\"false\" border=\"1 0 1\" fill=\"1 0 1\" mode=\"11\"\n" +
		"   points=\"";

	public static String default_section_file_string_3 =
		"	  \"/>\n" +
		"</Transform>\n" +
		"\n" +
		/*
		"<Transform dim=\"0\"\n" +
		" xcoef=\" 0 1 0 0 0 0\"\n" +
		" ycoef=\" 0 0 1 0 0 0\">\n" +
		"<Contour name=\"Cell\" hidden=\"false\" closed=\"true\" simplified=\"true\" border=\"1 0.5 0\" fill=\"1 0.5 0\" mode=\"13\"\n" +
		" points=\"0.5334 0.5715,\n" +
		"	0.54102 0.56134,\n" +
		"	0.54864 0.55626,\n" +
		"	0.55118 0.55118,\n" +
		"	0.59182 0.55118,\n" +
		"	0.59436 0.5461,\n" +
		"	0.6096 0.56134,\n" +
		"	0.6223 0.59944,\n" +
		"	0.61468 0.61976,\n" +
		"	0.59436 0.62738,\n" +
		"	0.57404 0.62738,\n" +
		"	0.55372 0.61722,\n" +
		"	0.53594 0.59944,\n" +
		"	0.5334 0.57912,\n" +
		"	0.5334 0.57404,\n" +
		"	\"/>\n" +
		"</Transform>\n" +
		"\n" +
		*/
		"</Section>\n";

}

