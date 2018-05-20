# Reconstruct Wish List

## Top Suggestions

* **1. Add a key to hide/unhide all traces without picking a domain (Preliminary fix in 1.2.0.9)**

* **2. Add support for an Object Comment**

* **3. Add versioning support for traces (date/ID)**

* **4. Add support for viewing a section/location from 3D representation**


## Report of bugs found and features requested in Reconstruct 

## Please use the following format:

**Name of bug/request:**
< Whatever you want to call this >

**Description of bug:** 
< A few sentences describing the bug or feature you want fixed and the results of the bug. > 

**Series where you have experienced bug:** 
< List at least one example series where you can demonstrate the bug or need for feature if asked. >

**Date of bug:** 
< When did you first experience bug? Can be date of this entry if if continuous. >

**Workaround:** 
< If you have figured out a workaround and have time to write it up, please do so. Otherwise, put “none” or “Contact Your Name” if you have one but don’t want to write it up. >

**System specs:** 
< Operating system, version of reconstruct, tifs or jpegs, hardware (ie dell optiplex 790 or microsoft surface pro 3 i5 4gb RAM.) >

**Point of contact:** 
< Name of person requesting this, fastest way of reaching you (email, text, etc.) >

**Endorsements:** 
< If you have experienced this bug but did not make the first report, add your name here and workarounds above in the same color as your name. >

*************************

## **Name of bug/request:** ‘Select Domain’ Tool scrolling issues

**Description of bug:** When using the select domain tool, a fast way to hide all traces on a single section, you cannot 
scroll back and forth between sections. If this is done, the domain is deleted from the series, and if the series is 
the saved, the domain must be re added later, causing load errors.

**Series where you have experienced bug:** KSGRS, JHHZS, HLWLQ, etc.

**Date of bug:** 9/6/2017 (constant)

**Workaround:** Use the hide traces option in the trace list rather than the select domain tool. Ctrl-S + Ctrl-H to hide 
all traces on that section. Or just don’t go to a different section before right clicking, and unselecting the domain.

**System specs:** Dell optiplex 790, windows 7, reconstruct x32 version 1.1.0.1 last modified 8/2/2008

**Point of contact:** Patrick

**Endorsements:** Lyndsey, etc

*************************

## **Name of request:** Calibrate first warning.

**Description:** If you don’t calibrate first, all your traces are ruined. If there was some flashing warning 
that the pixel size hasn’t been changed from the default when you make your first trace, that would help save 
people time. The program should not let you make such a catastrophic mistake.

**Workaround:** Inelegant kludge.

**Point of contact:** Patrick

**Endorsements:** Lyndsey

*************************

## **Name of request:** manual control for setting the number of sections Reconstruct loads into RAM

**Description of request:** By default reconstruct only loads 2 sections into RAM. This made sense 
when the program was created, but now that most computers we would use in the lab have at least 8GB 
of (preferably DDR4 or DDR5) RAM, a simple menu option to allow the user to specify the number of 
sections loaded would be very useful. 

**Series where you have experienced bug:** All series, especially tSEM series.

**Date of bug:** till the end of time.

**Workaround:** use hard drives with faster IOPS: SATA SSD<M.2<PCIE (USB-C, fast enough for 
external storage if your computer has a port for it)

**System specs:** Any hardware created after 2008.

**Point of contact:** Seth

**Endorsements:** Zeus, Ares, Heimdall, Odin

*************************

## **Name of bug/request:** Ability to highlight and copy object lists

**Description of bug:** Currently all object lists have to be saved as a CSV file, then opened in 
excel, using comma as delimiter. This is very time consuming and leads to many saved “object lists” 
when it would be easier to just copy the relevant data over into a master excel spreadsheet.

**Series where you have experienced bug:** all

**Date of bug:** always

**Workaround:** 

**System specs:**

**Point of contact:** Lyndsey

**Endorsements:** 


*************************

## **October 14, 2008 Wish List**

**Refactoring RECONSTRUCT TM  -- Wish list**

Modify the Simplify function to prevent trace expansion

Approach for smoothing the 3D representation (splines or NURBS)

Program the zoom feature as a shortcut with the mouse rather than as a framed region of 
interest to zoom. Also, consider a shortcut for unzooming.

Programming of auto-segmentation features


**Reconstruct Users’ Group 
Unsolicited suggestions for enhancements to RECONSTRUCTTM program:**

Consider programming so mouse can be used to align sections.  Also, color code 
adjacent sections to facilitate alignment (User Group Message #270).

Import files from photoshop so traces made prior to import could be used in Reconstruct (User Group Message #253).

Numbering sections in thumbnails (User Group Message #155).

Feature to allow export of 3D objects to known scale (User Group Message #139).

Selective renaming of traces (#111/112). KS notes this is already possible

Create an application to make surfaces smoother (#94). 

Modify the WildFire tool to allow color coding within an object group (#65).

Modify point tool to draw a box around the items one wants to select, and 
then choose to hide or show the traces for the boxed cluster. (#22).

Mouse or keyboard shortcuts for selecting traces, while using a drawing 
tool (#164). KS notes this is already possible

Consider making hue scale a circular scale.

To have the Autotracing window constantly open, so that one can
quickly change and test new Wildfire parameters without having to
open-close-open this window all the time

One extremely useful implementation would be: from a manual trace, to
have the program read RGB values of all the pixel and feed them into
the Widfire parameters limits

For manual tracing: to have a tool that could select the outside of a
pixel when you click on that pixel at high zoom.  (#213).

 


*************************

**On Sun, Feb 18, 2018 at 12:30 PM, Kristen Harris <kmh2249@gmail.com> wrote:**

Dear Bob,

Upon realizing that you could make the 3D as you go in Blender, it occurs to me that a way to retain all of our 
PSD surface area calculations without having to redraw them would be to take our cfa traces and expand/close 
them as the basis of a 'uniform' capsule around pre and post.  Cailey can show you how we do it (very inefficiently) 
now in reconstruct.

One y'all have the retro understanding of Reconstruct, I suspect that you could 'close' the open CFA trace, expand 
it relative to the plasma membrane so that they don't have to be 'retraced' manually in reconstruct.

Currently the cfa traces accurately delinate the surface of the PSD, whereas the c traces made to give a bouissanant 
surface are too big...

Eagerly awaiting your visit.

Patrick could you add this idea to the Reconstruct wish list wiki.

Thanks
Kristen

PS - triggered by a conversation with Corey -- we are tracing side-by-side right now to curate some postnatal 
day 10 data, and listening to classical guitar!

*************************

**Name of bug/request:**

**Description of bug:** 

**Series where you have experienced bug:** 

**Date of bug:** 

**Workaround:** 

**System specs:**

**Point of contact:** 

**Endorsements:** 

