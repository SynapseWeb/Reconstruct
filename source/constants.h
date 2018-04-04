/////////////////////////////////////////////////////////////////////////////////////////////
// Constants for Reconstruct application
//
//    Copyright (C) 1999-2007  John Fiala (fiala@bu.edu)
//
//    This is free software created with funding from the NIH. You may
//    redistribute it and/or modify it under the terms of the GNU General
//    Public License published by the Free Software Foundation (www.gnu.org).
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License version 2 for more details.
//
//
// modified 10/29/04 by JCF (fiala@bu.edu)
// -+- change: Added cursors, bitmap, and tool constants for Wildfire region growing.
// -+- change: Added control IDs for AutoTracing Series Options dialog.
// modified 11/11/04 by JCF (fiala@bu.edu)
// -+- change: Modified constants related to spin control in 3D Scene.
// -+- change: Switched wildfire stop criteria to HSB instead of RGB.
// modified 2/3/05 by JCF (fiala@bu.edu)
// -+- change: Added constants for SCALPEL tool.
// -+- change: Added ID for ShrinkBack option in Autotracing tab.
// modified 2/8/05 by JCF (fiala@bu.edu)
// -+- change: Added CM_COPYOBJECTS to Object List Modify menu.
// -+- change: Added CM_INFOSCENE to 3D Scene main menu.
// modified 2/16/05 by JCF (fiala@bu.edu)
// -+- change: Removed Palette and tools window timer constants.
// modified 3/10/05 by JCF (fiala@bu.edu)
// -+- change: Added GRID_ELLIPSE element type to Grid Tool.
// -+- change: Added ERRMSG_NOTSIMPLIFIED
// modified 3/16/05 by JCF (fiala@bu.edu)
// -+- change: Added ID_SPINRATEMSG for SpinRate dialog.
// modified 4/11/05 by JCF (fiala@bu.edu)
// -+- change: Added Save Scene as JPEG.
// modified 4/21/05 by Ju Lu  (julu@fas.harvard.edu)
// -+- change: Added automatic tracing; mutual avoidance between wildfire-generated contours.
// modified 4/27/05 by JCF (fiala@bu.edu)
// -+- change: Added ERRMSG constants to support modified CmNewSection().
// -+- change: Added CM_IMPORTLINES and ID_STOPATTRACES constants.
// modified 6/17/05 by JCF (fiala@bu.edu)
// -+- change: Added control ID constants for Autotracing tab.
// modified 6/23/05 by JCF (fiala@bu.edu)
// -+- change: Added constants for rotation menu operations, and ClearDomainTform.
// modified 7/8/05 by JCF (fiala@bu.edu)
// -+- change: Added constants for modified Object List menu.
// modified 7/12/05 by JCF (fiala@bu.edu)
// -+- change:  Added constants for number of columns in refillable lists and ERRMSG_GDIFAILED.
// modified 11/1/05 by JCF (fiala@bu.edu)
// -+- change:  Added constant for Rename Objects menu item.
// modified 3/29/06 by JCF (fiala@bu.edu)
// -+- change: Added AUTOTRACE_TIMER and removed UM_DOAUTOTRACE.
// modified 4/20/06 by JCF (fiala@bu.edu)
// -+- change: Added error message for mismatched trace names.
// modified 4/24/06 by JCF (fiala@bu.edu)
// -+- change: Added CM_REFRESHLIST and CMINFOLIST for new list menu commands; CM_ORTHOGRAPHIC
// modified 5/4/06 by JCF (fiala@bu.edu)
// -+- change: Added CM_SMOOTHSELECTED for smoothing traces, and CM_ZTRACEOBJECTS, CM_SMOOTHZTRACES,
//   CM_COPYZTRACES, CM_SMOOTHOBJECTS, and ID_SMOOTHINGLENGTH, ID_ZTRACEOFFSET1, ID_ZTRACEOFFSET2.
// modified 5/11/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added ID_CONTOURMASKWIDTH for masking a strip along contours
// modified 5/17/06 by JCF (fiala@bu.edu)
// -+- change: Added ID_COUNTPROXIES for button in proxies tab, and ID_SWAPPALETTEROWS for Names/Colors.
// modified 5/24/06 by JCF (fiala@bu.edu)
// -+- change: Added CM_EXPORTLINES and ID_SECTIONRANGEMSG to support DXF export from Series Menu.
// modified 6/16/06 by JCF (fiala@bu.edu)
// -+- change: Moved debug menu items to Program menu. Added CM_DEBUGLOG.
// modified 6/19/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added ID_AUTOADJUSTTHRESHOLD for automatic adjustment of brightness threshold
// modified 6/24/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Moved HLSMAX, HALFHUE, RGBMAX from Viewport.cpp
// modified 7/3/06 by Ju Lu (julu@fas.harvard.edu)
// -+- change: Added MULTIAUTOTRACE
// modified 11/15/06 by JCF (fiala@bu.edu)
// -+- change: Removed MultiAutotrace
// -+- change: Added CM_FINDTRACE and ERRMSG_TRACENOTFOUND.
// -+- change: Added CM_REVERSESECTIONS to list menu.
// modified 12/12/06 by JCF (fiala@bu.edu)
// -+- change: Added CM_COPYLISTDOMAIN command for Copy within Domain List
// modified 2/5/07 by JCF (fiala@bu.edu)
// -+- change: Added CM_DEBUGTIMES to allow more debug submenu items.
// modified 3/15/07 by JCF (fiala@bu.edu)
// -+- change: Added color channel options to domain image Attributes dialog.
// modified 4/23/07 by JCF (fiala@bu.edu)
// -+- change: Added CM_FINDOBJECTNAMED and CM_SAVE360BITMAP.
// modified 4/25/07 by JCF (fiala@bu.edu)
// -+- change: Moved UM_ message numbers to 1900's instead of WM_USER to eliminate list hanging problems.
// modified 5/8/07 by JCF (fiala@bu.edu)
// -+- change: OK, that didn't work. But this time I figured it out so I need more UM_ messages
// modified 5/11/07 by JCF (fiala@bu.edu)
// -+- change: Added CM_MAGNIFICATION for Zoom menu.
// modified 4/3/18 by BK (bobkuczewski@salk.edu)
// -+- change: Added CM_HIDEALLTRACES.


#define MAX_CONTOUR_NAME 64			// max length of Contour names
#define MAX_COMMENT		128			// max length of a trace comment string
#define MAX_VRML_COMMENT 512		// max length for VRML comment string
#define MAX_UNITS_STRING 32			// max length of units comment string
#define MAX_BASE		256			// max length of BaseName of series
#define MAX_INT32		2147483647	// max value for a (signed) 32-bit integer
#define MAX_DIGITS		22			// max string size for double
#define MAX_SIGDIGITS	15			// max digits for double significand
#define MAX_FLOAT		3.4E38		// max value for a 32-bit floating point
#define MIN_FLOAT		1E-14		// min value before zero in distance calc.
#define MIN_SCALE		0.001		// min allowed scale value
#define PI      3.1415926535897932
#define DIM				6			// dimension of the nonlinear transform basis
#define IRGB_PIXELS		32			// 1 index byte and 3 8bit values of color per pixel
#define MAX_DOMAINS		255			// actual domains allowed is only 254
#define MAX_UNDOS		16			// size of undo LIFO queues
#define NUM_WINDOWS		11			// total number of floating child windows
#define NUM_TOOLS		16			// number of toolbar buttons
#define MAX_SECTIONLISTCOLS 2		// number of potential columns in sectionList
#define MAX_DOMAINLISTCOLS 7		// number of potential columns in domainList
#define MAX_TRACELISTCOLS 12		// number of potential columns in traceList
#define MAX_OBJECTLISTCOLS 7		// number of potential columns in traceList
#define MAX_ZTRACELISTCOLS 5		// number of potential columns in ztraceList

#define  HLSMAX   252	// H,L, and S vary over 0-HLSMAX
#define  HALFHUE  126	// one half of the maximum hue value for wrap around of hue values
#define  RGBMAX   255   // R,G, and B vary over 0-RGBMAX

////////////////////////////////////// menu + command constants:
									// program menu
#define CM_TOOLBAR			1010
#define CM_STATUSBAR		1020
#define CM_DEBUGLOG			1030
#define CM_DEBUGTIMES		1035
#define CM_DEBUG			1040
#define CM_EXIT				1090
									// series menu
#define CM_OPEN				1100
#define CM_NEWSERIES		1105
#define CM_CLOSE			1110
#define CM_SERIESSRC		1120
#define CM_SAVESERIES		1130
#define CM_EXPORTLINES		1140
#define CM_RENDERSECTIONS	1150
#define CM_EXPORTTRACELISTS 1155
#define CM_IMPORTIMAGES		1160
#define CM_IMPORTLINES		1162
#define CM_IMPORTSERIES		1165
#define CM_SETOPTIONS		1170
									// section menu
#define CM_SECTIONLIST		1200
#define CM_PREVIOUS			1202
#define CM_PREDECESSOR		1204
#define CM_SUCCESSOR		1206
#define CM_THUMBNAILS		1210
#define CM_NEWSECTION		1212
#define CM_BLEND			1214
#define CM_CENTER			1216
#define CM_LASTZOOM			1218
#define CM_ACTUALPIXELS		1220
#define CM_MAGNIFICATION	1221
#define CM_SECTIONATTRIBUTES 1222
#define CM_BYCORRELATION	1224
#define CM_RELOAD			1226
#define CM_SECTIONSRC		1228
#define CM_SAVESECTION		1230
#define CM_TYPESECTIONTFORM	1232
#define CM_LOCKSECTION		1234
#define CM_UNDOSECTION		1236
#define CM_REDOSECTION		1238
#define CM_RESETSECTION		1240
#define CM_REPEATADJUSTMENT	1242
#define CM_PROPAGATEADJUST	1244
#define CM_RECORD			1246
#define CM_COPYACTIVENFORM	1248
#define CM_CALIBRATESECTIONS 1250
#define CM_90CLKWISE		1254
#define CM_90COUNTERCLKWISE 1256
#define CM_ROTATE180		1258
#define CM_FLIPHORZ			1260
#define CM_FLIPVERT			1262
									// domain and domain list menu
#define CM_NEWDOMAINFILE	1300
#define CM_MERGEREAR		1305
#define CM_MERGEFRONT		1310
#define CM_DOMAINLIST		1315
#define CM_RESTORECONTRAST	1320
#define CM_DELETEDOMAIN		1325
#define CM_DEFAULTDOMAIN	1330
#define CM_EDITDOMAIN		1350
#define CM_DELETELISTDOMAIN	1352
#define CM_SELECTDOMAIN		1355
#define CM_DOMAINATTRIBUTES	1365
#define CM_COPYLISTDOMAIN	1368
#define CM_HIDELISTDOMAIN	1370
#define CM_UNHIDELISTDOMAIN	1375
#define CM_CLEARDOMAINTFORM	1380
									// trace and trace list menu
#define CM_CUTSELECTED		1400
#define CM_COPYSELECTED		1402
#define CM_PASTESELECTED	1405
#define CM_PASTEATTRIBUTES	1406
#define CM_DELETESELECTED	1407
#define CM_ZOOMSELECTED		1410
#define CM_HIDETRACES		1415
#define CM_UNHIDETRACES		1417
#define CM_TRACEATTRIBUTES	1420
#define CM_SELECTTRACES		1425
#define CM_MERGESELECTED	1432
#define CM_REVERSESELECTED	1434
#define CM_SIMPLIFYSELECTED	1436
#define CM_SMOOTHSELECTED	1438
#define CM_UNSELECTALL		1442
#define CM_SELECTALL		1445
#define CM_LISTTRACES		1450
#define CM_ALIGNLINEAR		1452
#define CM_ALIGNDEFORM		1454
#define CM_ALIGNQUADRATIC	1456
#define CM_ALIGNRIGID		1458
#define CM_TRACEPALETTE		1460
#define CM_CALIBRATETRACES	1465
#define CM_SAVETRACELIST	1470
#define CM_DELETETRACES		1475
#define CM_EDITTRACES		1480
#define CM_FINDTRACE		1482
									// 3D object menu and list menus
#define CM_OBJECTLIST		1500
#define CM_3DSCENE			1501
#define CM_OBJECTTOSCENE	1505
#define CM_REMOVEFROMSCENE	1507
#define CM_OBJECTATTRIBUTES	1510
#define CM_OBJECTRENAME		1512
#define CM_SCENEATTRIBUTES	1515
#define CM_DELETEOBJECTS	1517
#define CM_SIMPLIFYOBJECTS	1518
#define CM_COPYOBJECTS		1519
#define CM_HIDEOBJECTS		1520
#define CM_UNHIDEOBJECTS	1522
#define CM_ZTRACEOBJECTS	1524
#define CM_SMOOTHOBJECTS	1525
#define CM_FINDOBJECTNAMED	1526
#define CM_SAVELIST			1530
#define CM_REFRESHLIST		1532
#define CM_INFOLIST			1533
#define CM_DISTANCELIST		1535
#define CM_ZLIST			1540
#define CM_ZTRACETOSCENE	1542
#define CM_ZTRACENAME		1544
#define CM_ZTRACECOLOR		1546
#define CM_DELETEZTRACES	1548
#define CM_ZTRACECOMMENT	1550
#define CM_COPYZTRACES		1552
#define CM_SMOOTHZTRACES	1554
#define CM_GRIDATZTRACES	1556
									// Section list menu
#define CM_DELETESECTIONS	1580
#define CM_MODIFYTHICKNESS	1582
#define CM_LOCKSECTIONS		1584
#define CM_UNLOCKSECTIONS	1586
#define CM_MODIFYCONTRAST	1587
#define CM_MODIFYPIXELSIZE	1588
#define CM_RENUMBERSECTIONS	1590
#define CM_REVERSESECTIONS	1592
									// 3D menu for OpenGL Window
#define CM_SAVEVRML1		1600
#define CM_SAVEVRML2		1602
#define CM_SAVEJPEG			1605
#define CM_SAVEBITMAP		1606
#define CM_SAVEDXF			1607
#define CM_SAVE360BMP		1608
#define CM_RESETSCENE		1610
#define CM_ORTHOGRAPHIC		1615
#define CM_BACKGROUND		1620
#define CM_ANIMATE			1635
#define CM_CLOSESCENE		1645
#define CM_CLEAROBJECTS		1650
#define CM_SPINSPEED		1655
#define CM_INFOSCENE		1660
									// help menu
#define CM_HELPRECON		1710
#define CM_HELPABOUT		1720
#define CM_HELPKEYTABLE		1730
#define CM_HELPMOUSE		1740
#define CM_HELPLICENSE		1750
									// keystroke commands
#define CM_TOGGLEVIEWS		1800
#define CM_CLKWISE			1802
#define CM_SLOWCLKWISE		1803
#define CM_FASTCLKWISE		1804
#define CM_COUNTERCLKWISE	1806
#define CM_SLOWCOUNTERCLKWISE 1807
#define CM_FASTCOUNTERCLKWISE 1808
#define CM_LEFT				1810
#define CM_SLOWLEFT			1811
#define CM_FASTLEFT			1812
#define CM_RIGHT			1814
#define CM_SLOWRIGHT		1815
#define CM_FASTRIGHT		1816
#define CM_UP				1820
#define CM_SLOWUP			1821
#define CM_FASTUP			1822
#define CM_DOWN				1824
#define CM_SLOWDOWN			1825
#define CM_FASTDOWN			1826
#define CM_XLARGER			1830
#define CM_SLOWXLARGER		1831
#define CM_FASTXLARGER		1832
#define CM_XSMALLER			1834
#define CM_SLOWXSMALLER		1835
#define CM_FASTXSMALLER		1836
#define CM_YLARGER			1838
#define CM_SLOWYLARGER		1839
#define CM_FASTYLARGER		1840
#define CM_YSMALLER			1842
#define CM_SLOWYSMALLER		1843
#define CM_FASTYSMALLER		1844
#define CM_SLANTXLEFT		1848
#define CM_SLOWSLANTXLEFT	1849
#define CM_FASTSLANTXLEFT	1850
#define CM_SLANTXRIGHT		1852
#define CM_SLOWSLANTXRIGHT	1853
#define CM_FASTSLANTXRIGHT	1854
#define CM_SLANTYDOWN		1856
#define CM_SLOWSLANTYDOWN	1857
#define CM_FASTSLANTYDOWN	1858
#define CM_SLANTYUP			1860
#define CM_SLOWSLANTYUP		1861
#define CM_FASTSLANTYUP		1862
#define CM_DEFORMXPLUS		1864
#define CM_SLOWDEFORMXPLUS	1865
#define CM_FASTDEFORMXPLUS	1866
#define CM_DEFORMXMINUS		1868
#define CM_SLOWDEFORMXMINUS	1869
#define CM_FASTDEFORMXMINUS	1870
#define CM_DEFORMYPLUS		1872
#define CM_SLOWDEFORMYPLUS	1873
#define CM_FASTDEFORMYPLUS	1874
#define CM_DEFORMYMINUS		1876
#define CM_SLOWDEFORMYMINUS	1877
#define CM_FASTDEFORMYMINUS	1878
#define CM_BACKSPACE		1884
#define CM_ESCAPECURRENTTOOL 1886
									// contrast changes
#define CM_BRIGHTEN			1900
#define CM_BRIGHTENFAST		1901
#define CM_DARKEN			1902
#define CM_DARKENFAST		1903
#define CM_MORECONTRAST		1904
#define CM_MORECONTRASTFAST	1905
#define CM_LESSCONTRAST		1906
#define CM_LESSCONTRASTFAST	1907

#define CM_PIXELSIZE		1910	// used in sEMToReconstruct only

//////////////////////////////////////additional control + message IDs
#define UM_UPDATESECTION	1971
#define UM_CONVERTNEXT		1972
#define UM_UPDATETEXT		1973
#define UM_STOPAUTOTRACE    1975
									// thread progress messages
#define UM_THUMBSPROGRESS	1984		// Sergey you may need this, follow logic of object thread
#define UM_OBJECTPROGRESS	1985
#define UM_DISTANCEPROGRESS	1987
									// thread termination messages
#define UM_RENDERDONE		1993
#define UM_THUMBSDONE		1994
#define UM_OBJECTDONE		1995
#define UM_GENERATE3DDONE	1996
#define UM_DISTANCEDONE		1997

//////////////  adding controls to the windows that use UM_ messages might cause conflicts

									// Resource IDs
#define XYCUR			2002		// cursors
#define ZOOMCUR			2004
#define MAGNIFYCUR		2006
#define DOMAINCUR		2008
#define POINTCUR		2010
#define ELLIPSECUR		2012
#define RECTCUR			2014
#define LINECUR			2016
#define MULTILINECUR	2018
#define PENCILCUR		2020
#define CULTILINECUR	2022
#define SCISSORCUR		2024
#define ZLINECUR		2026
#define WILDFIRECUR		2028
#define SCALPELCUR		2030
#define ERASERCUR		2032
#define ROTATECUR		2036
#define GRIDCUR			2038
#define PRECISECUR		2040
#define OUTFIRECUR		2042
									// tool IDs: These must be sequential numbers...
#define ARROW_TOOL			0
#define ZOOM_TOOL			1
#define MAGNIFY_TOOL		2
#define DOMAIN_TOOL			3
#define POINT_TOOL			4
#define ELLIPSE_TOOL		5
#define RECTANGLE_TOOL		6
#define LINE_TOOL			7
#define MULTILINE_TOOL		8
#define PENCIL_TOOL			9
#define CULTILINE_TOOL		10
#define SCISSOR_TOOL		11
#define ZLINE_TOOL			12
#define GRID_TOOL			13
#define WILDFIRE_TOOL		14
#define SCALPEL_TOOL		15
									// ...with no gaps or missing numbers for correct toolbar creation
									// tool bitmap/command IDs: these should equal 2100+(tool ID)
#define TB_ARROW			2100
#define TB_ZOOM				2101
#define TB_MAGNIFY			2102
#define TB_DOMAIN			2103
#define TB_POINT			2104
#define TB_ELLIPSE			2105
#define TB_RECTANGLE		2106
#define TB_LINE				2107
#define TB_MULTILINE		2108
#define TB_PENCIL			2109
#define TB_CULTILINE		2110
#define TB_SCISSOR			2111
#define TB_ZLINE			2112
#define TB_GRID				2113
#define TB_WILDFIRE			2114
#define TB_SCALPEL			2115

									// initial params of button and child windows
#define THUMB_WIDTH			128
#define THUMB_HEIGHT		96
#define GL_WIDTH			512
#define GL_HEIGHT			512
#define BUTTON_SIZE			32		// size of tool buttons (fixed)
#define BUTTON_SPACE		33
									// timer IDs
#define STATUS_TIMER		1
#define SPLASH_TIMER		2
#define THUMBS_TIMER		4
#define OPENGL_TIMER		8
#define SCROLL_TIMER		16
#define AUTOTRACE_TIMER		32
									// state flags for Image loaded state
#define NONE				0		// NONE must be zero for logical tests
#define SKIPPED				1
#define PROXY				2
#define SOURCE				4
									// states for Calibrate scope
#define APPLY_TRACES		0
#define APPLY_DOMAIN		1
#define APPLY_SECTION		2
#define APPLY_SERIES		4
									// grid types
#define RECTANGLE_GRID		0
#define POINT_GRID			1
#define FRAME_GRID			2
#define STAMP_GRID			3
#define CLIPBOARD_GRID		4
#define CYCLOID_GRID		5
#define ELLIPSE_GRID		6
									// 3D object types
#define TRACES_OBJECT		0
#define SURFACE_OBJECT		1
#define POINTSET_OBJECT		2
#define AREAS_OBJECT		3
#define BOX_OBJECT			4
#define CYLINDER_OBJECT		5
#define ELLIPSOID_OBJECT	6
#define SPHERE_OBJECT		7
									// dialog control IDs
									// transform type-in dialog
#define ID_CENTER			4000
#define ID_XCENTER			4002
#define ID_YCENTER			4004
#define ID_THETA			4006
#define ID_TRANSX			4008
#define ID_TRANSY			4010
#define ID_SCALEX			4012
#define ID_SCALEY			4014
#define ID_SLANTX			4016
#define ID_SLANTY			4018
#define ID_DEFORMX			4020
#define ID_DEFORMY			4022
									// movement increment options
#define ID_INTRINSIC		4025
#define ID_EXTRINSIC		4030
#define ID_DEFORMKEYS		4035
#define ID_THETACTRL		4040
#define ID_TRANSXCTRL		4042
#define ID_TRANSYCTRL		4044
#define ID_SCALEXCTRL		4046
#define ID_SCALEYCTRL		4048
#define ID_SLANTXCTRL		4050
#define ID_SLANTYCTRL		4052
#define ID_DEFORMXCTRL		4054
#define ID_DEFORMYCTRL		4056
#define ID_THETASHFT		4070
#define ID_TRANSXSHFT		4072
#define ID_TRANSYSHFT		4074
#define ID_SCALEXSHFT		4076
#define ID_SCALEYSHFT		4078
#define ID_SLANTXSHFT		4080
#define ID_SLANTYSHFT		4082
#define ID_DEFORMXSHFT		4084
#define ID_DEFORMYSHFT		4086
									// some ids used in multiple dialogs
#define ID_UNITS			4100
#define ID_UNITS1			4101
#define ID_UNITS2			4102
#define ID_UNITS3			4103
#define ID_UNITS4			4104
#define ID_SECTIONRANGEMSG	4108
#define ID_FIRSTSECTION		4110
#define ID_LASTSECTION		4112
#define ID_DELETEFILES		4115
#define ID_INFO				4120
#define ID_DESTNAME			4130
#define ID_WIDTH			4140
#define ID_HEIGHT			4142
#define ID_SCALE			4144
#define ID_PATH				4150
#define ID_BROWSE			4152
#define ID_INPUTVALUE		4160
#define ID_INPUTNAME		4161
#define ID_INPUTSTRING		4162
#define ID_SOURCE			4165
#define ID_DONE				4170
									// section list dialog
#define ID_SECTIONLIST		4180
#define CM_SOURCE			4184	// list submenu
#define CM_LISTGOTO			4186
									// series dialogs
#define ID_PIXELSIZE		4200
#define ID_THICKNESS		4205
#define ID_SETTHICKNESS		4206
#define ID_SETPIXELSIZE		4207
#define ID_24BIT			4210
#define ID_TRACES			4215
#define ID_8BIT				4220
#define ID_NOTRACES			4225
#define ID_COLORIZE			4228
#define ID_GRAYSCALE		4230
#define ID_JPEG				4232
#define ID_JPEGQUALITY		4234
#define ID_BITMAP			4236
#define ID_WINDOW			4240
#define ID_FULL				4242
#define ID_OTHER			4244
#define ID_FILESIZE			4270
#define ID_WIDTHHEIGHT		4280
#define ID_INCREMENTBY		4285

#define ID_LOCKED			4300
#define ID_UNLOCKED			4301
									// multi-section image import dialog
#define ID_XOFFSET			4370
#define ID_YOFFSET			4371
#define ID_FILELIST			4373
#define ID_IMPORTFILES		4375
#define ID_COPYFILES		4380
#define ID_SORTFILES		4382
#define ID_RANDOMFILES		4383
#define ID_BROWSESRC		4385
									// series import dialog
#define ID_IMPORTOPTIONS	4400
#define ID_IMPORTZTRACES	4405
#define ID_IMPORTPALETTE	4407
#define ID_IMPORTSECTIONS	4410
#define ID_IMPORTDOMAINS	4415
#define ID_IMPORTTRACES		4417
#define ID_ZTRACENAME		4420
									// missing image file dialog
#define ID_SKIPFILE			4490
#define ID_SKIPALL			4495
#define ID_MISSINGMSG		4496
									// domain menu dialoga
#define ID_DOMAINNAME		4500
#define ID_DOMAINFORMAT		4501
#define ID_DOMAINBPP		4502
#define ID_DOMAINDIMENSIONS	4503
#define ID_DOMAINSIZE		4504
#define ID_DOMAINPROXY		4505
#define ID_DOMAINFILE		4510
#define ID_CONTRAST			4515
#define ID_BRIGHTNESS		4520
#define ID_REDCHANNEL		4522
#define ID_GREENCHANNEL		4524
#define ID_BLUECHANNEL		4526
									// general options tab dialogs
#define ID_SIGDIGITS		5000
#define ID_INCREASEDIGITS	5005
#define ID_AUTOSAVESERIES	5010
#define ID_AUTOSAVESECTION	5011
#define ID_WARNSAVESECTION	5015
#define ID_BEEPDELETING		5020
#define ID_BEEPPAGING		5022
#define ID_HIDETRACES		5024
#define ID_UNHIDETRACES		5026
#define ID_HIDEDOMAINS		5030
#define ID_UNHIDEDOMAINS	5035
#define ID_USEABSOLUTEPATHS	5040
#define ID_ZMIDSECTION		5045
									// proxies
#define ID_PERCENTPROXIES	5100
#define ID_COUNTPROXIES		5101
#define ID_CREATEPROXIES	5102
#define ID_DELETEPROXIES	5104
#define ID_USEPROXIES		5106
									// thumbnails
#define ID_SKIPSECTIONS		5215
#define ID_THUMBVIEW		5220
#define ID_THUMBFIT			5225
#define ID_THUMBCONTOURS	5230
#define ID_FLIPBOOKSTYLE	5235
#define ID_FLIPRATE			5240
									// names/colors
#define ID_BORDERCOLOR		5300
#define ID_HIDDEN			5302
#define ID_FILLCOLOR		5305
#define ID_COMMENT			5307
#define ID_CONTOURNAME		5310
#define ID_CLOSED			5312
#define ID_SIMPLIFIED		5313
#define ID_FILLSELECTED		5315
#define ID_MERGEFILL		5320
#define ID_MASKFILL			5321
#define ID_NOFILL			5322
#define ID_COPYFILL			5323
#define ID_SWAPPALETTEROWS	5324
#define ID_PALETTE			5325
									// autotracing
#define ID_AUTOSIMPLIFY		5326
#define ID_SIMPLIFYRESOLUTION 5327
#define ID_SHRINKBACK		5328
#define ID_SMOOTHINGLENGTH	5329
#define ID_HUESTOPTYPE		5330
#define ID_HUESTOPCHANGE	5332
#define ID_HUESTOPVALUE		5334
#define ID_SATSTOPTYPE		5340
#define ID_SATSTOPCHANGE	5342
#define ID_SATSTOPVALUE		5344
#define ID_BRIGHTSTOPTYPE	5350
#define ID_BRIGHTSTOPCHANGE	5352
#define ID_BRIGHTSTOPVALUE	5354
#define ID_STOPATTRACES		5355
#define ID_AREASTOPPERCENT	5360
#define ID_AREASTOPSIZE		5365
#define ID_CONTOURMASKWIDTH 5366
#define ID_AUTOADJUSTTHRESHOLD 5367

									// list options
#define ID_LISTSECTIONTHICKNESS	5400
#define ID_LIMITSECTIONLIST		5401
#define ID_LIMITDOMAINLIST		5410
#define ID_LISTDOMAINSOURCE		5415
#define ID_LISTDOMAINPIXELSIZE	5416
#define ID_LISTDOMAINLENGTH		5417
#define ID_LISTDOMAINAREA		5418
#define ID_LISTDOMAINMIDPOINT	5419
#define ID_LIMITTRACELIST		5425
#define ID_LISTTRACELENGTH		5430
#define ID_LISTTRACEAREA		5431
#define ID_LISTTRACECENTROID	5432
#define ID_LISTTRACEEXTENT		5433
#define ID_LISTTRACEZ			5434
#define ID_LISTTRACETHICKNESS	5435
#define ID_LISTTRACECOMMENT		5436
#define ID_LIMITZTRACELIST		5440
#define ID_LISTZTRACENOTE		5442
#define ID_LISTZTRACERANGE		5444
#define ID_LISTZTRACELENGTH		5446
#define ID_LIMITOBJECTLIST		5450
#define ID_LISTOBJECTRANGE		5452
#define ID_LISTOBJECTCOUNT		5454
#define ID_LISTOBJECTSURFACEAREA 5456
#define ID_LISTOBJECTFLATAREA	5458
#define ID_LISTOBJECTVOLUME		5460
#define ID_LEFTDISTANCELIST		5470
#define ID_RIGHTDISTANCELIST	5475
									// 3D
#define ID_ZOFFSET			5550	// XOFFSET and YOFFSET defined above
#define ID_3DTRACES			5552
#define ID_3DSURFACE		5554
#define ID_3DCYLINDER		5555
#define ID_3DSPHERE			5556
#define ID_3DELLIPSOID		5557
#define ID_3DBOX			5558
#define ID_3DAREAS			5559
#define ID_3DPOINTSET		5560
#define ID_MAXCONNECTION	5565
#define ID_UPPERFACES		5567
#define ID_LOWERFACES		5568
#define ID_NONORMALS		5570
#define ID_FACENORMALS		5574
#define ID_VERTEXNORMALS	5576
#define ID_4FACETS			5580
#define ID_8FACETS			5581
#define ID_16FACETS			5582
#define ID_32FACETS			5583
#define ID_ASIZE			5586
#define ID_BSIZE			5587
#define ID_CSIZE			5588
#define ID_ZTRACEOFFSET1	5590
#define ID_ZTRACEOFFSET2	5592
									// Grids tab
#define ID_GRIDRECTANGLE	5600
#define ID_GRIDPOINT		5602
#define ID_GRIDFRAME		5604
#define ID_GRIDSTAMP		5606
#define ID_GRIDCLIPBOARD	5608
#define ID_GRIDCYCLOID		5610
#define ID_GRIDELLIPSE		5612
#define ID_SIZEX			5620
#define ID_SIZEY			5625
#define ID_DISTANCEX		5630
#define ID_DISTANCEY		5635
#define ID_NUMBERX			5640
#define ID_NUMBERY			5645
									// trace attribute dialogs
#define ID_NUMTRACES		5700
#define ID_GETDEFAULTS		5701
#define ID_GETCLIPBOARD		5702
									// trace calibration dialog
#define ID_APPLYTRACES		5710
#define ID_APPLYDOMAIN		5712
#define ID_APPLYSECTION		5714
#define ID_APPLYSERIES		5716
									// scene attribute dialog
#define ID_DIFFUSECOLOR		5850
#define ID_EMISSIVECOLOR	5852
#define ID_SHININESS		5854
#define ID_SPECULARCOLOR	5856
#define ID_AMBIENTINTENSITY	5858
#define ID_TRANSPARENCY		5860
#define ID_FRONTFILL		5865
#define ID_BACKFILL			5867
									// scene rotation dialog
#define ID_HORZSPINRATE		5870
#define ID_VERTSPINRATE		5875
#define ID_SPINRATEMSG		5877

#define FRAMETIME			33		// time in ms for 3D animation frames

									// status bar id
#define ID_STATUSBAR		5900
									// modeless splash dialog text controls
#define ID_ERRMSG			5950
#define ID_LICENSE			5955
#define ID_MSGTXT			5960
#define ID_MSGICON			5970
#define ID_VERSION			5980
#define ID_COPYRIGHT		5990
#define ID_DESCRIPTION		5995

////////////////////////////////////// General messages
#define MSG_TITLE			6000
#define MSG_COPYRIGHT		6010
#define MSG_AUTHORSHIP		6020
#define MSG_VERSION			6030
#define MSG_PURPOSE			6040
									// Error message numbers
#define ERRMSG_READ_FAILED		6500
#define ERRMSG_WRITE_FAILED		6505
#define ERRMSG_DELETE_FAILED	6510
#define ERRMSG_COPY_FAILED		6515
#define ERRMSG_RUN_FAILED		6520
#define ERRMSG_ALLOC_FAILED		6525
#define ERRMSG_CONVERGENCE		6530
#define ERRMSG_INVALID_FORMAT	6535
#define ERRMSG_JPEG_ERROR		6540
#define ERRMSG_OPEN_SERIES		6545
#define ERRMSG_NEW_SERIES		6547
#define ERRMSG_ABORT			6550
#define ERRMSG_SUCCESS			6555
#define ERRMSG_FILEWRITTEN		6557
#define ERRMSG_NOSECTIONS		6560
#define ERRMSG_NOOBJECTS		6562
#define ERRMSG_FILELIST			6565
#define ERRMSG_NEEDSECTION		6570
#define ERRMSG_NEEDTRACE		6572
#define ERRMSG_NEEDLINETRACE	6573
#define ERRMSG_NEEDTOALIGN		6575
#define ERRMSG_MISMATCHEDTRACES 6576
#define ERRMSG_BADPIXELFORMAT	6580
#define ERRMSG_NOZTRACES		6585
#define ERRMSG_RENUMBER			6590
#define ERRMSG_PRECISION		6595
#define ERRMSG_NOTSIMPLIFIED	6596
#define ERRMSG_SECTIONINVALID	6597
#define ERRMSG_SECTIIONEXISTS	6598
#define ERRMSG_GDIFAILED		6599
#define ERRMSG_TRACENOTFOUND	6600

#define CM_HIDEALLTRACES    	7100
