# Reconstruct Java Notes and Log
## This document is intended to collect notes on the Java implementation and document its progress.

# Notes:

The first tests have demonstrated that Java can handle the loading, zooming, and panning of large images very well!

# Log:

**March 13th, 2018** - Began experimenting with the Java XML classes for reading existing Reconstruct XML files. It's not clear (at this time) whether the current XML representation should be used as it is or not. But in any event, it seems likely that we'll want to have the ability to open previous XML files to import. The exact format of the XML files is still being deduced.

Notes on Reconstruct Files (from reconstruct_intro_for_beginners_parker_2017.pdf):
* There are 3 types of Reconstruct files: traces, images, and .ser.
* The trace files contain the Reconstruct traces for each individual section. The .ser file (.ser is short for series) contains traces/information which spans across many of the sections (such as Z-traces, the trace palette, settings, etc.). The image files are, of course, the images that you will be looking at and tracing.
* In order to be opened in Reconstruct, the image files, trace files, and .ser file must be in the same folder.
* When you open a reconstruct series, you select the .ser file to open.
* Series in the Harris Lab are named with a randomly generated 5-letter code, such as BBCHZ.
