#!/usr/bin/env python

#__import__('code').interact(local = locals())
import time

import pygtk
pygtk.require('2.0')
import gobject
import gtk

import app_window
import scheduler
import location
import sim_2d


def expose_callback ( widget, event, zpa ):
  diff_2d_sim = zpa.user_data['diff_2d_sim']
  state_history = zpa.user_data['state_history']
  display_time_index = zpa.user_data['display_time_index']
  x, y, width, height = event.area  # This is the area of the portion newly exposed
  width, height = widget.window.get_size()  # This is the area of the entire window
  x, y = widget.window.get_origin()
  drawable = widget.window
  colormap = widget.get_colormap()
  gc = widget.get_style().fg_gc[gtk.STATE_NORMAL]
  # Save the current color
  old_fg = gc.foreground
  # Clear the screen with black
  gc.foreground = colormap.alloc_color(0,0,0)
  drawable.draw_rectangle(gc, True, 0, 0, width, height)
  # Buffer the original value if the history is empty
  if len(state_history) == 0:
    buffer_state(zpa)
  # Draw the current state referenced by display_time_index
  t = 0
  if len(state_history) > 0:
    current_state = state_history[display_time_index]
    t = current_state['t']
    for m in current_state['mols']:
      gc.foreground = colormap.alloc_color(int(65535*m['c'][0]),int(65535*m['c'][1]),int(65535*m['c'][2]))
      px = m['x']
      py = m['y']
      drawable.draw_arc(gc, True, zpa.wxi(px)-m['r'], zpa.wyi(py)-m['r'], 2*m['r'], 2*m['r'], 0, 360*64)
    gc.foreground = colormap.alloc_color(60000, 60000, 60000)
    for coll in current_state['cols']:
      px = coll['x']
      py = coll['y']
      drawable.draw_rectangle(gc, True, zpa.wxi(px), zpa.wyi(py), 3, 3)
    for o in current_state['objs']:
      #print ( "Drawing object " + o['name'] )
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


# Buffer the state for redrawing when the simulation goes backward or foreward
def buffer_state(zpa):
  diff_2d_sim = zpa.user_data['diff_2d_sim']
  state_history = zpa.user_data['state_history']
  display_time_index = zpa.user_data['display_time_index']
  current_state = {}
  #print ( "Buffering state " )
  # Save the current molecule positions for this time
  current_state['mols'] = []
  for m in diff_2d_sim.mols:
    m_draw = {'name':m.species.name, 'x':m.pt.x, 'y':m.pt.y, 'r':2, 'c':m.species.color}
    if issubclass ( m.pt.__class__, location.point_radius ):
      m_draw['r'] = m.pt.r
    current_state['mols'].append ( m_draw )
  # Save the history of detected collisions for this time
  current_state['cols'] = []
  for coll in diff_2d_sim.collisions:
    current_state['cols'].append ( { 'x':coll[0], 'y':coll[1] } )
  current_state['objs'] = []
  for obj in diff_2d_sim.objects:
    #print ( "Buffering object " + obj.name )
    current_state['objs'].append ( { 'name':obj.name, 'x':obj.x, 'y':obj.y, 'c':obj.color, 'points':[p for p in obj.points], 'faces':[f for f in obj.faces] } )
  current_state['t'] = diff_2d_sim.t

  state_history.append ( current_state )

def step_callback(zpa):
  diff_2d_sim = zpa.user_data['diff_2d_sim']
  state_history = zpa.user_data['state_history']
  display_time_index = zpa.user_data['display_time_index']
  if len(state_history) == 0:
    buffer_state(zpa)  # Save the original state
  if display_time_index < -1:
    # The history is already buffered
    display_time_index += 1
    zpa.user_data['display_time_index'] = display_time_index
    print ( "Display buffered data for index " + str(display_time_index) )
  else:
    # Buffer the history and step
    if diff_2d_sim.scheduler.items_left() > 0:
      print ( "Display new data" )
      diff_2d_sim.step()
      buffer_state(zpa)
    else:
      print ( "No events scheduled, cannot advance time." )
      zpa.user_data['running'] = False
  zpa.get_drawing_area().queue_draw()
  return True

def back_callback(zpa):
  state_history = zpa.user_data['state_history']
  display_time_index = zpa.user_data['display_time_index']
  if ( display_time_index+len(state_history) ) > 0:
    # It's OK to step backwards
    display_time_index += -1
    zpa.user_data['display_time_index'] = display_time_index
    print ( "Display buffered data for index " + str(display_time_index) )
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
  zpa.user_data['diff_2d_sim'] = sim_2d.diff_2d_sim()
  zpa.user_data['state_history'] = []
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
    if command == "Spiral":
      zpa.user_data['option'] = command
      zpa.queue_draw()
    elif command == "Rect":
      zpa.user_data['option'] = command
      zpa.queue_draw()
    elif command == "Fast":
      zpa.user_data['frame_delay'] = 0.01
    elif command == "Med":
      zpa.user_data['frame_delay'] = 0.1
    elif command == "Slow":
      zpa.user_data['frame_delay'] = 1.0
    elif command == "ToggleLegend":
      zpa.user_data['show_legend'] = not zpa.user_data['show_legend']
      zpa.queue_draw()
  return True


# Create the window and connect the events
def main():

  # Create a top-level GTK window
  window = gtk.Window ( gtk.WINDOW_TOPLEVEL )
  window.set_title ( "2D Diffusion" )

  # Create a zoom/pan area to hold all of the drawing
  zpa = app_window.zoom_pan_area(window,600,500,"2D Diffusion")
  zpa.user_data = { 
                    'diff_2d_sim'        : sim_2d.diff_2d_sim(),
                    'state_history'      : [],
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

  back_button = gtk.Button("Back")
  hbox.pack_start ( back_button, True, True, 0 )
  back_button.connect_object ( "clicked", back_callback, zpa )
  back_button.show()

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

  """ # These bypass the buffering and shouldn't be used until fixed.
  step_in_button = gtk.Button("Step In")
  hbox.pack_start ( step_in_button, True, True, 0 )
  step_in_button.connect_object ( "clicked", step_in_callback, zpa )
  step_in_button.show()

  step_10_button = gtk.Button("Step 10")
  hbox.pack_start ( step_10_button, True, True, 0 )
  step_10_button.connect_object ( "clicked", step_10_callback, zpa )
  step_10_button.show()
  """

  reset_button = gtk.Button("Reset")
  hbox.pack_start ( reset_button, True, True, 0 )
  reset_button.connect_object ( "clicked", reset_callback, zpa )
  reset_button.show()

  # Show the main window
  window.show()

  zpa.set_cursor ( gtk.gdk.HAND2 )

  gtk.idle_add ( background_callback, zpa )

  # Turn control over to GTK to run everything from here onward.
  gtk.main()
  return 0


if __name__ == '__main__':
  main()
