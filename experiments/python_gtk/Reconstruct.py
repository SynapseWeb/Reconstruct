#!/usr/bin/env python

#__import__('code').interact(local = locals())
import time

import pygtk
pygtk.require('2.0')
import gobject
import gtk

import app_window


def expose_callback ( drawing_area, event, zpa ):
  diff_2d_sim = zpa.user_data['diff_2d_sim']
  display_time_index = zpa.user_data['display_time_index']
  x, y, width, height = event.area  # This is the area of the portion newly exposed
  width, height = drawing_area.window.get_size()  # This is the area of the entire window
  x, y = drawing_area.window.get_origin()
  drawable = drawing_area.window
  colormap = drawing_area.get_colormap()
  gc = drawing_area.get_style().fg_gc[gtk.STATE_NORMAL]
  # Save the current color
  old_fg = gc.foreground
  # Clear the screen with black
  gc.foreground = colormap.alloc_color(0,0,0)
  drawable.draw_rectangle(gc, True, 0, 0, width, height)
  # Draw the current state referenced by display_time_index
  t = 0

  if zpa.user_data['image_frame']:

    """
    def draw_pixbuf(gc, pixbuf, src_x, src_y, dest_x, dest_y, width=-1, height=-1, dither=gtk.gdk.RGB_DITHER_NORMAL, x_dither=0, y_dither=0)

      gc : a gtk.gdk.GC, used for clipping, or None
      pixbuf : a gtk.gdk.Pixbuf
      src_x : Source X coordinate within pixbuf.
      src_y : Source Y coordinate within pixbuf.
      dest_x : Destination X coordinate within drawable.
      dest_y : Destination Y coordinate within drawable.
      width : Width of region to render, in pixels, or -1 to use pixbuf width. Must be specified in PyGTK 2.2.
      height : Height of region to render, in pixels, or -1 to use pixbuf height. Must be specified in PyGTK 2.2
      dither : Dithering mode for GdkRGB.
      x_dither : X offset for dither.
      y_dither : Y offset for dither.

    The draw_pixbuf() method renders a rectangular portion of a gtk.gdk.Pixbuf specified by pixbuf
    to the drawable using the gtk.gdk.GC specified by gc. The portion of pixbuf that is rendered is
    specified by the origin point (src_x src_y) and the width and height arguments. pixbuf is rendered
    to the location in the drawable specified by (dest_x dest_y). dither specifies the dithering mode a
    s one of:

      gtk.gdk.RGB_DITHER_NONE 	  Never use dithering.

      gtk.gdk.RGB_DITHER_NORMAL   Use dithering in 8 bits per pixel (and below) only.

      gtk.gdk.RGB_DITHER_MAX	    Use dithering in 16 bits per pixel and below.

    The destination drawable must have a colormap. All windows have a colormap, however, pixmaps only have
    colormap by default if they were created with a non-None window argument. Otherwise a colormap must be
    set on them with the gtk.gdk.Drawable.set_colormap() method.

    On older X servers, rendering pixbufs with an alpha channel involves round trips to the X server, and
    may be somewhat slow. The clip mask of gc is ignored, but clip rectangles and clip regions work fine.

    ========
    https://developer.gnome.org/gdk-pixbuf/stable/gdk-pixbuf-Scaling.html
    C:
    void gdk_pixbuf_scale (const GdkPixbuf *src,
                  GdkPixbuf *dest,
                  int dest_x,
                  int dest_y,
                  int dest_width,
                  int dest_height,
                  double offset_x,
                  double offset_y,
                  double scale_x,
                  double scale_y,
                  GdkInterpType interp_type);
    Python:
       src.scale ( dest, dest_x, dest_y, dest_width, dest_height, offset_x, offset_y, scale_x, scale_y, interp_type )
          src          a GdkPixbuf
          dest         the GdkPixbuf into which to render the results
          dest_x       the left coordinate for region to render
          dest_y       the top coordinate for region to render
          dest_width   the width of the region to render
          dest_height  the height of the region to render
          offset_x     the offset in the X direction (currently rounded to an integer)
          offset_y     the offset in the Y direction (currently rounded to an integer)
          scale_x      the scale factor in the X direction
          scale_y      the scale factor in the Y direction
          interp_type  the interpolation type for the transformation.
    """
    pix_buf = zpa.user_data['image_frame']
    pbw = pix_buf.get_width()
    pbh = pix_buf.get_height()
    scale_w = zpa.ww(pbw) / pbw
    scale_h = zpa.wh(pbh) / pbh
    ##scaled_image = pix_buf.scale_simple( int(pbw*scale_w), int(pbh*scale_h), gtk.gdk.INTERP_BILINEAR )
    #scaled_image = pix_buf.scale_simple( int(pbw*scale_w), int(pbh*scale_h), gtk.gdk.INTERP_NEAREST )
    #drawable.draw_pixbuf ( gc, scaled_image, 0, 0, zpa.wxi(0), zpa.wyi(0), -1, -1, gtk.gdk.RGB_DITHER_NONE )

    #(dw,dh) = drawable.get_size()
    #pbcs = pix_buf.get_colorspace()
    # dest_pm = gtk.gdk.Pixmap ( drawable, dw, dh )
    #dest = gtk.gdk.Pixbuf ( pbcs, False, drawable.get_depth(), dw, dh )
    #dest = gtk.gdk.Pixbuf ( pbcs, False, 8, dw, dh )  # For some reason the depth seems to have to be 8
    #pix_buf.scale(dest, 0, 0, 10, 10, 0, 0, 1, 1, gtk.gdk.INTERP_NEAREST)
    #drawable.draw_pixbuf ( gc, scaled_image, 0, 0, zpa.wxi(0), zpa.wyi(0), -1, -1, gtk.gdk.RGB_DITHER_NONE )

    #gtk.gdk

    #__import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})
    scaled_image = pix_buf.scale_simple( int(pbw*scale_w), int(pbh*scale_h), gtk.gdk.INTERP_NEAREST )
    drawable.draw_pixbuf ( gc, scaled_image, 0, 0, zpa.wxi(0), zpa.wyi(0), -1, -1, gtk.gdk.RGB_DITHER_NONE )

    # print ( "Zoom Scale = " + str(zpa.zoom_scale) )

    # __import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})

  # Draw objects (from the original example)
  for obj in zpa.user_data['diff_2d_sim'].objects:
    #print ( "Drawing object " + o['name'] )
    o = { 'name':obj.name, 'x':obj.x, 'y':obj.y, 'c':obj.color, 'points':[p for p in obj.points], 'faces':[f for f in obj.faces] }
    gc.foreground = colormap.alloc_color(int(65535*o['c'][0]),int(65535*o['c'][1]),int(65535*o['c'][2]))
    cx = o['x']
    cy = o['y']
    for f in o['faces']:
      p1 = ( cx+o['x']+o['points'][f[0]][0], cy+o['y']+o['points'][f[0]][1] )
      p2 = ( cx+o['x']+o['points'][f[1]][0], cy+o['y']+o['points'][f[1]][1] )
      drawable.draw_line ( gc, zpa.wxi(p1[0]), zpa.wyi(p1[1]), zpa.wxi(p2[0]), zpa.wyi(p2[1]) )

  if zpa.user_data['show_legend']:
    # Draw the current time (seconds) to show something that doesn't zoom or pan
    cr = drawable.cairo_create()
    cr.set_source_rgb (1,1,1)
    cr.move_to(15,21)
    legend = "Sim Time: " + str(diff_2d_sim.t) + ", Showing: t-" + str(abs(display_time_index+1)) + " = " + str(t)
    cr.show_text ( legend )
    # Underline the text (uses a crude estimate since text is variable width)
    gc.foreground = colormap.alloc_color(65535,0,0)
    drawable.draw_rectangle(gc, True, 10, 25, 12+int(round(145*len(legend)/28.0)), 2)

  # Restore the previous color
  gc.foreground = old_fg
  return False


def step_callback(zpa):
  diff_2d_sim = zpa.user_data['diff_2d_sim']
  display_time_index = zpa.user_data['display_time_index']
  diff_2d_sim.step()
  zpa.get_drawing_area().queue_draw()
  return True


def step_in_callback(zpa):
  diff_2d_sim = zpa.user_data['diff_2d_sim']
  diff_2d_sim.step_in()
  zpa.get_drawing_area().queue_draw()
  return True

def step_10_callback(zpa):
  for i in range(10):
    step_callback(drawing_area)
  return True

def dump_callback(zpa):
  diff_2d_sim = zpa.user_data['diff_2d_sim']
  diff_2d_sim.print_self()
  return True

def reset_callback(zpa):
  # This creates a new simulation
  zpa.user_data['diff_2d_sim'] = diff_2d_sim()
  zpa.user_data['display_time_index'] = -1
  zpa.get_drawing_area().queue_draw()


def background_callback ( zpa ):
  if zpa.user_data['running']:
    t = time.time()
    if t - zpa.user_data['last_update'] > zpa.user_data['frame_delay']:
      zpa.user_data['last_update'] = t
      step_callback(zpa)
      print ( "  Running at time = " + str(t) )
      zpa.queue_draw()
  return True

def run_callback ( zpa ):
  # print ( "Run " )
  zpa.user_data['running'] = True
  return True

def stop_callback ( zpa ):
  # print ( "Stop " )
  zpa.user_data['running'] = False
  return True


def menu_callback ( widget, data=None ):
  # Menu items will trigger this call
  # The menu items are set up to pass either a tuple:
  #  (command_string, zpa)
  # or a plain string:
  #  command_string
  # Checking the type() of data will determine which
  if type(data) == type((True,False)):
    # Any tuple passed is assumed to be: (command, zpa)
    command = data[0]
    zpa = data[1]
    if command == "Fast":
      zpa.user_data['frame_delay'] = 0.01
    elif command == "Med":
      zpa.user_data['frame_delay'] = 0.1
    elif command == "Slow":
      zpa.user_data['frame_delay'] = 1.0
    elif command == "ToggleLegend":
      zpa.user_data['show_legend'] = not zpa.user_data['show_legend']
      zpa.queue_draw()
    elif command == "Debug":
      __import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})
      zpa.queue_draw()
    else:
      print ( "Menu option \"" + command + "\" is not handled yet." )
  return True

# Minimized stub of the previous 2D Simulation
class point_face_object:
  def __init__( self, name="", x=0, y=0, color=(32000,32000,32000), points=[], faces=[] ):
    self.name = name
    self.color = color
    self.points = [ p for p in points ]
    self.faces = [ f for f in faces ]
    self.x = x
    self.y = y
    if (len(points) > 0) and (len(faces) == 0):
      # Make faces for the points
      for i in range(len(self.points)):
        print ( "  Making face with " + str(i) + "," + str((i+1)%len(self.points)) )
        self.faces.append ( (i, (i+1)%len(self.points)) )
  def print_self ( self ):
    print ( "Object " + self.name + ": x,y = (" + str(self.x) + "," + str(self.y) )
    for point in self.points:
      print ( "  " + str(point) )
    for face in self.faces:
      print ( "  " + str(face) )

class diff_2d_sim:
  def __init__ ( self ):
    print ( " Constructing a new minimal simulation" )
    self.objects = [
        point_face_object ( "Triangle 1", x=80, y=0, points=[[0,100], [50,-10], [-50,-10]] ),
        point_face_object ( "Square 1", x=0, y=0, points=[[-90,-90], [-90,90], [90,90], [90,-90]] )
      ]
    # Set some simulation values
    self.t = 0
    self.dt = 2

  def diffuse ( self ):
    print ( "Start diffuse" )
    print ( "End diffuse" )

  def step ( self ):
    print ( "Before run(1): t=" + str(self.t) )
    self.t += self.dt
    print ( "After run(1): t=" + str(self.t) )

  def step_in ( self ):
    print ( "Before step_in(): t=" + str(self.t) )
    self.t += self.dt
    print ( "After step_in(): t=" + str(self.t) )

  def print_self ( self ):
    print ( "t = " + str(self.t) )



# Create the window and connect the events
def main():

  # Create a top-level GTK window
  window = gtk.Window ( gtk.WINDOW_TOPLEVEL )
  window.set_title ( "Reconstruct Python GTK Demonstration" )

  # Create a zoom/pan area to hold all of the drawing
  zpa = app_window.zoom_pan_area(window,720,540,"Reconstruct Python GTK Demonstration")
  zpa.user_data = { 
                    'image_frame'        : None,
                    'image_frames'       : [],
                    'diff_2d_sim'        : diff_2d_sim(),
                    'display_time_index' : -1,
                    'running'            : False,
                    'last_update'        : -1,
                    'show_legend'        : True,
                    'frame_delay'        : 0.1,
                    'size'               : 1.0
                  }

  # Set the relationship between "user" coordinates and "screen" coordinates
  zpa.set_x_scale ( 0.0, 300, 100.0, 400 )
  zpa.set_y_scale ( 0.0, 250 ,100.0, 350 )

  # Create a vertical box to hold the menu, drawing area, and buttons
  vbox = gtk.VBox ( homogeneous=False, spacing=0 )
  window.add(vbox)
  vbox.show()

  # Connect GTK's "main_quit" function to the window's "destroy" event
  window.connect ( "destroy", lambda w: gtk.main_quit() )

  # Create a menu bar and add it to the vertical box
  menu_bar = gtk.MenuBar()
  vbox.pack_start(menu_bar, expand=False, fill=False, padding=0)

  # Create a "Program" menu
  (program_menu, program_item) = zpa.add_menu ( "_Program" )
  if True: # An easy way to indent and still be legal Python
    zpa.add_menu_item ( program_menu, menu_callback, "Windows >",  ("Windows", zpa ) )
    zpa.add_menu_sep  ( program_menu )
    zpa.add_menu_item ( program_menu, menu_callback, "Debug >",    ("Debug", zpa ) )
    zpa.add_menu_item ( program_menu, menu_callback, "Exit",       ("Exit", zpa ) )

  # Create a "Series" menu
  (series_menu, series_item) = zpa.add_menu ( "_Series" )
  if True: # An easy way to indent and still be legal Python
    zpa.add_menu_item ( series_menu, menu_callback, "Open...",   ("Open", zpa ) )
    zpa.add_menu_item ( series_menu, menu_callback, "Close",     ("Close", zpa ) )
    zpa.add_menu_sep  ( series_menu )
    zpa.add_menu_item ( series_menu, menu_callback, "New...",     ("New Series", zpa ) )
    zpa.add_menu_item ( series_menu, menu_callback, "Save",       ("Save Series", zpa ) )
    zpa.add_menu_item ( series_menu, menu_callback, "Options...", ("Options", zpa ) )
    zpa.add_menu_sep  ( series_menu )
    zpa.add_menu_item ( series_menu, menu_callback, "Export >",     ("Export", zpa ) )
    zpa.add_menu_item ( series_menu, menu_callback, "Import >",     ("Import", zpa ) )

  # Create a "Section" menu
  (section_menu, section_item) = zpa.add_menu ( "_Section" )
  if True: # An easy way to indent and still be legal Python
    zpa.add_menu_item ( section_menu, menu_callback, "List Sections...",   ("List Sections", zpa ) )
    zpa.add_menu_item ( section_menu, menu_callback, "Thumbnails...",      ("Thumbnails", zpa ) )
    zpa.add_menu_sep  ( section_menu )
    zpa.add_menu_item ( section_menu, menu_callback, "New...",   ("New Section", zpa ) )
    zpa.add_menu_item ( section_menu, menu_callback, "Save",   ("Save Section", zpa ) )
    zpa.add_menu_item ( section_menu, menu_callback, "Thickness...",   ("Thickness", zpa ) )
    zpa.add_menu_sep  ( section_menu )
    zpa.add_menu_item ( section_menu, menu_callback, "Undo",   ("Undo", zpa ) )
    zpa.add_menu_item ( section_menu, menu_callback, "Redo",   ("Redo", zpa ) )
    zpa.add_menu_item ( section_menu, menu_callback, "Reset",   ("Reset", zpa ) )
    zpa.add_menu_item ( section_menu, menu_callback, "Blend",   ("Blend", zpa ) )
    zpa.add_menu_item ( section_menu, menu_callback, "Zoom >",   ("Zoom", zpa ) )
    zpa.add_menu_item ( section_menu, menu_callback, "Movement >",   ("Movement", zpa ) )

  # Create a "Domain" menu
  (domain_menu, domain_item) = zpa.add_menu ( "_Domain" )
  if True: # An easy way to indent and still be legal Python
    zpa.add_menu_item ( domain_menu, menu_callback, "List image domains...",   ("List Domains", zpa ) )
    zpa.add_menu_item ( domain_menu, menu_callback, "Import image...",   ("Import Image", zpa ) )
    zpa.add_menu_sep  ( domain_menu )
    zpa.add_menu_item ( domain_menu, menu_callback, "Merge front",   ("Merge Front", zpa ) )
    zpa.add_menu_item ( domain_menu, menu_callback, "Merge rear",   ("Merge Rear", zpa ) )
    zpa.add_menu_item ( domain_menu, menu_callback, "Attributes...",   ("Attributes", zpa ) )
    zpa.add_menu_item ( domain_menu, menu_callback, "Reinitialize >",   ("Reinitialize", zpa ) )
    zpa.add_menu_sep  ( domain_menu )
    zpa.add_menu_item ( domain_menu, menu_callback, "Delete",   ("Delete Domain", zpa ) )

  # Create a "Trace" menu
  (trace_menu, trace_item) = zpa.add_menu ( "_Trace" )
  if True: # An easy way to indent and still be legal Python
    zpa.add_menu_item ( trace_menu, menu_callback, "List traces...",   ("List traces", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Find...",   ("Find Trace", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Select all",   ("Select All Traces", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Deselect all",   ("Deselect All Traces", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Zoom to",   ("Zoom to", zpa ) )
    zpa.add_menu_sep  ( trace_menu )
    zpa.add_menu_item ( trace_menu, menu_callback, "Attributes...",   ("Trace Attributes", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Palette...",   ("Trace Palette", zpa ) )
    zpa.add_menu_sep  ( trace_menu )
    zpa.add_menu_item ( trace_menu, menu_callback, "Cut",   ("Cut", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Copy",   ("Copy", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Paste",   ("Past", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Paste attributes",   ("Paste Attributes", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Delete",   ("Delete Trace", zpa ) )
    zpa.add_menu_sep  ( trace_menu )
    zpa.add_menu_item ( trace_menu, menu_callback, "Align Section",   ("Align Section", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Calibrate...",   ("Calibrate", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Merge",   ("Merge Trace", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Reverse",   ("Reverse Trace", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Simplify",   ("Simplify Trace", zpa ) )
    zpa.add_menu_item ( trace_menu, menu_callback, "Smooth",   ("Smooth Trace", zpa ) )

  # Create a "Object" menu
  (object_menu, object_item) = zpa.add_menu ( "_Object" )
  if True: # An easy way to indent and still be legal Python
    zpa.add_menu_item ( object_menu, menu_callback, "List Objects...",   ("List Objects", zpa ) )
    zpa.add_menu_sep  ( object_menu )
    zpa.add_menu_item ( object_menu, menu_callback, "3D Scene...",   ("3D Scene", zpa ) )
    zpa.add_menu_item ( object_menu, menu_callback, "Z-Traces...",   ("Z-Traces", zpa ) )
    zpa.add_menu_item ( object_menu, menu_callback, "Distances...",   ("Distances", zpa ) )

  # Create a "Help" menu
  (help_menu, help_item) = zpa.add_menu ( "_Help" )
  if True: # An easy way to indent and still be legal Python
    zpa.add_menu_item ( help_menu, menu_callback, "Manual...",   ("Manual", zpa ) )
    zpa.add_menu_item ( help_menu, menu_callback, "Key commands...",   ("Key Commands", zpa ) )
    zpa.add_menu_item ( help_menu, menu_callback, "Mouse clicks...",   ("Mouse Clicks", zpa ) )
    zpa.add_menu_sep  ( help_menu )
    zpa.add_menu_item ( help_menu, menu_callback, "License...",   ("License", zpa ) )
    zpa.add_menu_item ( help_menu, menu_callback, "Version...",   ("Version", zpa ) )


  # Create a "Mode" menu
  (mode_menu, mode_item) = zpa.add_menu ( "_Mode" )

  # Create an "Options" menu
  (options_menu, options_item) = zpa.add_menu ( "_Options" )
  if True: # An easy way to indent and still be legal Python
    zpa.add_menu_item ( options_menu, menu_callback, "Toggle Legend", ("ToggleLegend",zpa) )


  # Create a "Speed" menu
  (speed_menu, speed_item) = zpa.add_menu ( "_Speed" )
  if True: # An easy way to indent and still be legal Python
    zpa.add_menu_item ( speed_menu, menu_callback, "Fast",   ("Fast",zpa ) )
    zpa.add_menu_item ( speed_menu, menu_callback, "Medium", ("Med", zpa ) )
    zpa.add_menu_item ( speed_menu, menu_callback, "Slow",   ("Slow",zpa ) )

  # Append the menus to the menu bar itself
  menu_bar.append ( program_item )
  menu_bar.append ( series_item )
  menu_bar.append ( section_item )
  menu_bar.append ( domain_item )
  menu_bar.append ( trace_item )
  menu_bar.append ( object_item )
  menu_bar.append ( help_item )
  menu_bar.append ( mode_item )

  menu_bar.append ( options_item )
  menu_bar.append ( speed_item )

  # Show the menu bar itself (everything must be shown!!)
  menu_bar.show()

  # The zoom/pan area has its own drawing area (that it zooms and pans)
  drawing_area = zpa.get_drawing_area()

  # Add the zoom/pan area to the vertical box (becomes the main area)
  vbox.pack_start(drawing_area, True, True, 0)
  drawing_area.show()

  # The zoom/pan area doesn't draw anything, so add our custom expose callback
  drawing_area.connect ( "expose_event", expose_callback, zpa )

  # Set the events that the zoom/pan area must respond to
  #  Note that zooming and panning requires button press and pointer motion
  #  Other events can be set and handled by user code as well
  drawing_area.set_events ( gtk.gdk.EXPOSURE_MASK
                          | gtk.gdk.LEAVE_NOTIFY_MASK
                          | gtk.gdk.BUTTON_PRESS_MASK
                          | gtk.gdk.POINTER_MOTION_MASK
                          | gtk.gdk.POINTER_MOTION_HINT_MASK )

  # Create a horizontal box to hold application buttons
  hbox = gtk.HBox ( True, 0 )
  hbox.show()
  vbox.pack_start ( hbox, False, False, 0 )

  # Add some application specific buttons and their callbacks

  step_button = gtk.Button("Step")
  hbox.pack_start ( step_button, True, True, 0 )
  step_button.connect_object ( "clicked", step_callback, zpa )
  step_button.show()

  run_button = gtk.Button("Run")
  hbox.pack_start ( run_button, True, True, 0 )
  run_button.connect_object ( "clicked", run_callback, zpa )
  run_button.show()

  stop_button = gtk.Button("Stop")
  hbox.pack_start ( stop_button, True, True, 0 )
  stop_button.connect_object ( "clicked", stop_callback, zpa )
  stop_button.show()

  dump_button = gtk.Button("Dump")
  hbox.pack_start ( dump_button, True, True, 0 )
  dump_button.connect_object ( "clicked", dump_callback, zpa )
  dump_button.show()

  reset_button = gtk.Button("Reset")
  hbox.pack_start ( reset_button, True, True, 0 )
  reset_button.connect_object ( "clicked", reset_callback, zpa )
  reset_button.show()



  zpa.user_data['image_frame'] = gtk.gdk.pixbuf_new_from_file ( "test.png" )


  # Show the main window
  window.show()

  zpa.set_cursor ( gtk.gdk.HAND2 )

  gtk.idle_add ( background_callback, zpa )

  # Turn control over to GTK to run everything from here onward.
  gtk.main()
  return 0


if __name__ == '__main__':
  main()
