// Reconstruct: version 1.1.0.0 //
--------------------------------------------------------------------------------

 Files of the distribution:

	readme.txt	This file
	reconstruct.exe	Reconstruct application
	gpl.txt		GNU General Public License, version 2.
	keycmds.chm	Keyboard Commands help file
	mousecmds.chm	Mouse commands help file
	(manual.chm	Online Users Manual help file in full version download only)
	

// Reconstruct: Authorship, Copyright and License //
--------------------------------------------------------------------------------------------

 Reconstruct copyright (c) 1996-2007 John C. Fiala.
 Additional credits and copyrights: Andy Key, Thomas G. Lane, Ju Lu.

 Development of Reconstruct was funded, in part, by the Human Brain Project
 and the National Institutes of Health under grants (P30 HD 18655,
 R01 MH/DA 57351, R01 EB 002170, RO1 NS024760 and RO1 MH057414).

 Permission to use, copy, and redistribute Reconstruct is granted without fee
 under the terms of the GNU General Public License as published by the Free
 Software Foundation. You should have received a copy of the GNU General
 Public License version 2 along with this program. This program is distributed
 in the hope that it will be useful, but WITHOUT ANY WARRANTY,
 including the implied warranty of MERCHANTABILITY or FITNESS FOR A
 PARTICULAR PURPOSE.

// Reconstruct: About Software //
--------------------------------------------------------------------------------------

v1.0.0.0 First public release version.

v1.0.0.5	Added Distances List to Object Menu and Lists tab of Options.
	Fixed wildcards in .csv filenames.

v1.0.1.0    Removed potential of background thread hanging, e.g. when filling Object List.
	Fixed erroneous detecion of intersections in Distances calculations.
	Added Undo push before Scissors Tool use.

v1.0.1.2	Added DXF output to 3D Scene export.

v1.0.1.6    Added Movement By Correlation.
	Fixed minor bug in Simplify algorithm.
	Added whole object Simplify operation.
	Fixed crashing after XML file read errors.
	Fixed open of a bad series file.
	Added Movement tab options to Series file.
	Added precision cursor option for drawing tools.
	Added ability to import components from another series.
	Updated help files.

v1.0.1.9	Added Export of all Trace Lists to one .csv file.
	Fixed no z-Contours bug in Import Series and improved dialog.
	Minor bug fixes in Simplify, Fill, and Merge algorithms.

v1.0.2.0	Added ` keystroke as additional flicker command.
	Modified Scene Attributes dialog.
	Bug fixes in maintaining path information for series and images, and
	finding a missing image file will now reset the images path to that folder.

v1.0.2.1	Added significantDigits attribute to series.
	Fixed bug in saving z-trace comments.
	Added comments to tools palette selection.
	Fixed formula on import domain dialog.

v1.0.2.4	Added alternative method for z-distance calculation
	Added .bmp or .jpg extension to exported images

v1.0.2.6	Fixed bug in Trace List when Area column is absent
	Added user adjustment of significant digits and output precision.
	Added Brightness/Contrast adjustment to Modify menu of Section List.

v1.0.2.9	Added wildfire region growing tool and associated series options.
	Fixed Section List double click with nothing selected.

v1.0.3.0	Switched Wildfire stop criteria to HSB from RGB.
	Added Spin speed control in 3D Scene window and simplified menus.

v1.0.3.1	Fixed inaccuracy in Align (Section or Traces) > Rigid.
	Added example.ser to installation.

v1.0.3.2	Fixed bug in palette name updating in series options tab.

v1.0.3.6	Modified behavior of Tools window title text and initial size of window.
	Added preliminary Scalpel tool for slicing traces.
	Added Copy, Rename to Objects Modify menu. Rename is same as Modify>Attributes>Of Traces...
	Added Message at end of Of Traces... operation to let user know Object list should be repoened.
	Added Shrink back option to autotracing to eliminate trace expansion.
	Added Info item to 3D Scene menu to allow evaluation of scene complexity.

v1.0.3.8	Made Palette and Tools window title text fixed by button press.
	Added section number to Trace Lists export.
	Mouse Wheel in main window pages sections.

v1.0.3.9	Fixed bug in view pan/zoom when page after domain selected.
	Fixed renaming of z-traces when there are duplicate names.
	Added select with right mouse to Scalpel Tool.
	Fixed slowness when 3D Scene not rotating in spin mode.

v1.0.4.0	Added ellipse element to grid tool.
	Fixed problems with scalpel tool when break line runs along trace.
	Added "Nothing to Simplify" message when no traces are simplified.

v1.0.4.1	Modified spin rate dialog to provide single rotations.

v1.0.4.2	Modified upper and lower face 3D Boissonnat surface options to be restricted
		to beginning and end of object only, not intermediate sections.
	
v1.0.4.3	Fixed bug in 3D Scene Export->Bitmap that sometimes created boug images.
		Modified order of Export menu and added save as JPEG option.

v1.0.4.4        Added autotrace function to automatically trace an object across multiple sections
                using wildfire. Made contours generated by wildfire mutually exclusive.

v1.0.4.5	Fixed 1.0.4.4 Wildfire bug. Added DXF import of traces.
		Added option to stop Wildfire at traces. Added paste option for domain boundary.
		Fixed Status Bar message when editing with Scissor Tool.
		Modified Section>New... dialog to just ask for valid section number.

v1.0.4.6	Removed wildfire autotrace initialization from pencil tool.
		Fixed Ctrl-D causing autotrace.

v1.0.4.7	Added wildfire ability to trace everything in a rectangular region.
		Made wildfire autotrace key cmds only available when Wildfire Tool is selected.
		Modified some menu item text for clarity.

v1.0.4.8	Added wildfire autotracing option parameters for area.
		Fixed wildfire region tracing bug that generated Abnormal Program Termination.
		Increased default upper bounds on some section number option params.

v1.0.4.9	F2 keystroke changed to 90deg rotation and corrdinates of flips corrected.
		Flips and rotations added to Movement menu.
		Domain menu modified to include full reset of an image's transform.
		Import Domain now places image on section without selecting it.
		Fixed Domain Tool button press when select a Domain from list.

v1.0.5.0	Fixed bug in wildfire when region was single pixel and problem with
		regional wildfires that caused irregular crashes.

v1.0.5.1	Improved efficiency of changing multiple object attributes in long series.
		Added whole section brightness/contrast changes from keyboard.
		Adjusted 3D Scene reset to recalculate scene extent.

v1.0.5.2	Fixed bug in 3D Scene of traces of polyline just created in current section.
		Modification of Object List menu structure.

v1.0.5.3	Fixed bug in reseting/opening 3D Scene.
		Fixed bug that produces empty export images when images are too large.
		Modified z-trace list-scene interaction behavior, and behavior of Section list.

v1.0.5.4	Fixed file type text in JPEG export of 3D Scene.
		Fixed Export Images to not draw hidden traces when using Fill with border color.
		Fixed behavior of deleting and renumbering sections that are in memory.

v1.0.5.5	Added error message when Thumbnails run out of memory.
		Black and white cursors. Added precision cursor option to Magnify Tool.

v1.0.5.6	Unpushed palette button with change default contour settings in Options.
		
v1.0.5.7	Middle mouse button for trace selection on all tools. Scapel rt drag = pan.
		Deferred message of GDI memory error until try to use bitmap resource.
		Fixed scene problems after Export As... Bitmap/JPEG.

v1.0.5.8	Fixed too large section numbers on Import Images with -1 option.
		Fixed scalpel right click arrow cursor.
		Separated Rename objects from attributes to allow change over section range.
		Fixed fatal error when start 2 different 3D generation jobs at same time.
		Fixed fatal error in using pencil with domain selected.
		Modified behavior of Ctrl-Home to be less zoomed when centering trace.

v1.0.5.9	Added progress for object list reading.

v1.0.6.0	Modified RenameObject to delete old names from object list.

v1.0.6.1	Made automatic naming of domains and contours independent.
		Added save of memory sections before Exporting Images.

v1.0.6.2	Fixed multiple Object list dailogs bug.
		Updated DTD for series XML format.
		Fixed crashing when reading a corrupted (incomplete) XML data file.

v1.0.6.3	Deleted Objects from scene when renamed.

v1.0.6.4	Fixed missing brightStopValue read from Series file.
		Rewrote Ju Lu's autotracing stuff so it would work properly.

v1.0.6.5	Fixed atan2 DOMAIN error that caused crash during align Rigid.
		Added check and warning message for mismatched traces when aligning.
		Added 3D Trace Slabs output as generalization of Trace Areas.

v1.0.6.6	Added List Menu to lists with refresh and info commands.
		Added orthographic scene mode.

v1.0.6.7	Fixed animation of scene when resizing scene window.
		Added Create ZTrace from Midpoints to Object List.

v1.0.6.8	Added Z-trace smoothing, trace smoothing, and object smoothing.
		Added shift by section use difference of z-traces to 3D tab.

v1.0.6.9	Fixed DXF lines import bug and added LINE and SOLID entity recognition.

v1.0.7.0	Added Create Grid at Z-trace points to Z-trace List menu.

v1.0.7.2	Added pixel width to existing traces for wildfire stopping.
		Minor updates to menu text.

v1.0.7.3	Enhanced palette editing to all 20 entries.
		Fixed delay caused by counting proxies when open proxies tab.

v1.0.7.4	Modified DXF imports to include some color and other point entities.
		Added DXF Export of traces to Series Menu.

v1.0.7.5	Fixed fatal bug when added empty object to empty scene.
		Fixed object list refresh hanging after close z-trace list and other minor bugs.

v1.0.7.7	Adds debug items such as logging to the Program menu.
                Brightness adaptation parameter added to Wildfire autotracing.
		Made hue parameter wrap around for Wildfire stop criterion.

v1.0.7.8	Fixed a number of minor bugs in contour manipulation and compilation issues.

v1.0.8.0	Added HTML help manual.
		Included Ju Lu's undocumented adaptive, multi-autotrace feature.
		Used Microsoft compiler generated code.

v1.0.8.1	Multiautotrace working from any set of selected traces.
		WildfireRegions modified to fully trace all region in window whose initial points
		  lie in the rectangle.

v1.0.8.2	Ctrl+g to get clipboard attributes as default attributes for drawing.
		Home key resets 3D Scene.
		Find Trace... function added to Trace menu.
		Reverse Order function added to Section List menu.
		
v1.0.8.5	Fixed bug in image loading that caused repeated error messages.
		Fixed relative paths for domain images and proxy images, and when Importing Series.
		Fixed false error message when deleting sections.
		Switched back to Borland compiler for much faster section file loading.

v1.0.8.6	Added Copy command and Create menu to Domain List.

v1.0.8.7	Fixed bug in storing z-traces with section numbers greater than data precision.
		Added some debugging options.

v1.0.8.8	Added ability to limit domain R, G or B color channel display.
		Warn before closing series with changes when automatic Save is off.

v1.0.9.0	Fixed bug in counting proxies.
		Added GIF and TIFF stack types to domain attributes dialog.
		Fixed bug in image domain path when copy file while useAbsolutePaths mode is set.
		Fixed bug in trying to copy non-existent proxy images in Import Series.
		Added ability to import GIF and TIFF stack in Import Images dialog.
		Modified Import Images dialog for stack input and new default start section.

v1.0.9.1	Fixed bugs in displaying domains that sometimes left black lines.
		After Import Images, the first section is automatically displayed in main window.

v1.0.9.2	Fixed Series Option dialog text on 96dpi machines.
		Fixed Cancel of Rotation dialog in 3D Scene window.
		Fixed (?) Object List hanging at "Reading..." on some machines.
		Added 360 degree bitmap sequence output from 3D Scene.
		Added "Find 1st!" command to Object List to display first trace of object.

v1.0.9.3	Removed setting of hue to initial value when undefined in Wildfire. On pure grayscale hue is 255.
		Changed Status Bar to report HSB when wildfire tool in use.
		Changed Wildfire Stop Criterion from "equals" to "does not equal" and made test approximate.

v1.0.9.4	Fixed list hanging!?
v1.0.9.5	Added Magnification... dialog to Zoom menu.
v1.0.9.6	Display wait cursor during section list operations.

v1.0.9.8	Modified List windows, thambnail window, and 3D Scene to allow maximize/restore.
		Added color channel modify to multiple sections from Section List.

v1.0.9.9	Fixed bugs that crashed program when accessing menus with Domain Tool or Wildfire Tool.

v1.1.0.0	Minor textual corrections.


Visit the Reconstruct Users Group at  http://groups.yahoo.com/group/reconstruct_users/
for the latest support files, links, and information.
Bug reports can be emailed to reconstruct_developers@yahoogroups.com
