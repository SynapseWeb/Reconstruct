# Provide an interface to a user application that ...
#
#  - Allows direct use of all GTK drawing commands
#  - Allows most code to work with user coordinates
#  - Provides simple coordinate transform functions
#  - Transparent zoom and pan
#  - Application controlled zoom and pan requests
#  - Application controlled window sizing requests
#  - Easy menu interface (may be a separate module)
#  - Easy status bar interface (may be a separate module)
#
#  - Define initial scaling and window size:
#      win.set_x_scale ( 0.0,   0,  1.0, 640 )
#      win.set_y_scale ( 0.0, 480,  1.0,   0 )
#      win.set_fixed_aspect ( True )
#      win.set_scroll_factor ( 1.1 )
#  - Query current window size:
#      win.get_x_min ()
#      win.get_x_max ()
#      win.get_y_min ()
#      win.get_y_max ()
#  - Request refit
#      win.fitwidth  ( -3.1, 7.6, keep_aspect=False ) # implies left to right
#      win.fitheight (  200, 0.0, keep_aspect=False ) # implies top to bottom
#  - Convert user coordinates to window coordinates for drawing
#      win.wx ( user_x )
#      win.wy ( user_y )
#      win.ww ( user_w )
#      win.wh ( user_h )
#  - Convert window coordinates to user coordinates for callbacks
#      win.x ( win_x )
#      win.y ( win_y )
#      win.w ( win_w )
#      win.h ( win_h )

#
#  - Drawing callback
#      pixmap.draw_rectangle ( gc, True, win.x(0.33), win.y(1.2), win.w(0.5), win.h(0.1) )


#__import__('code').interact(local = locals())



import math

import pygtk
pygtk.require('2.0')
import gobject
import gtk

# There was no way to pass the zoom/pan window
# into the mouse scroll callback. So it was made
# global here.  : (
global_zpa = None

class zoom_pan_area:

  def __init__( self, window, win_width, win_height, name="" ):
    # These are defined to move from user space to graphics space
    self.window = window
    self.name = name
    self.set_defaults()
    self.drawing_area = gtk.DrawingArea()
    self.drawing_area.set_flags ( gtk.CAN_FOCUS )
    self.drawing_area.set_size_request(win_width,win_height)

    # self.drawing_area.connect ( "expose_event", expose_callback, self )
    self.drawing_area.connect ( "scroll_event", mouse_scroll_callback )
    self.drawing_area.connect ( "key_press_event", key_press_callback )
    self.drawing_area.connect ( "button_press_event", button_press_callback )
    self.drawing_area.connect ( "button_release_event", button_release_callback )
    self.drawing_area.connect ( "motion_notify_event", mouse_motion_callback )

    self.drawing_area.set_events ( gtk.gdk.EXPOSURE_MASK
                                 | gtk.gdk.ENTER_NOTIFY_MASK
                                 | gtk.gdk.LEAVE_NOTIFY_MASK
                                 | gtk.gdk.KEY_PRESS_MASK
                                 | gtk.gdk.BUTTON_PRESS_MASK
                                 | gtk.gdk.BUTTON_RELEASE_MASK
                                 | gtk.gdk.POINTER_MOTION_MASK
                                 | gtk.gdk.POINTER_MOTION_HINT_MASK )

    self.accel_group = gtk.AccelGroup()
    self.window.add_accel_group(self.accel_group)
    
    self.user_data = None

    global global_zpa
    global_zpa = self

  def set_defaults ( self ):
    self.x_offset = self.reset_x_offset = 0.0
    self.y_offset = self.reset_y_offset = 0.0
    self.x_scale = self.reset_x_scale = 1.0
    self.y_scale = self.reset_y_scale = 1.0
    self.aspect_fixed = True
    self.scroll_count = 0
    self.scroll_factor = 1.25
    self.zoom_scale = 1.0
    self.dragging = False
    self.last_x = 0
    self.last_y = 0

  def reset_view ( self ):
    #self.set_defaults()
    self.x_offset = self.reset_x_offset
    self.y_offset = self.reset_y_offset
    self.x_scale = self.reset_x_scale
    self.y_scale = self.reset_y_scale
    self.scroll_count = 0
    self.zoom_scale = 1.0
    self.drawing_area.queue_draw()

  def set_x_scale ( self, user_x1, win_x1, user_x2, win_x2 ):
    self.x_scale  = self.reset_x_scale = float(win_x2 - win_x1) / (user_x2 - user_x1)
    self.x_offset = self.reset_x_offset = win_x1 - ( user_x1 * self.x_scale )
    
  def set_y_scale ( self, user_y1, win_y1, user_y2, win_y2 ):
    self.y_scale  = self.reset_y_scale = float(win_y2 - win_y1) / (user_y2 - user_y1)
    self.y_offset = self.reset_y_offset = win_y1 - ( user_y1 * self.y_scale )


  def get_drawing_area ( self ):
    return ( self.drawing_area )


  def queue_draw ( self ):
    return ( self.drawing_area.queue_draw() )

  
  def wx ( self, user_x ):
    return ( self.x_offset + (user_x * self.x_scale * self.zoom_scale ) )
  def wy ( self, user_y ):
    return ( self.y_offset + (user_y * self.y_scale * self.zoom_scale ) )
  def ww ( self, user_w ):
    return ( user_w * self.x_scale * self.zoom_scale )
  def wh ( self, user_h ):
    return ( user_h * self.y_scale * self.zoom_scale )

  def wxi ( self, user_x ):
    return ( int(round(self.wx(user_x))) )
  def wyi ( self, user_y ):
    return ( int(round(self.wy(user_y))) )
  def wwi ( self, user_w ):
    return ( int(round(self.ww(user_w))) )
  def whi ( self, user_h ):
    return ( int(round(self.wh(user_h))) )

  def x ( self, win_x ):
    return ( (win_x - self.x_offset) / (self.x_scale * self.zoom_scale) )
  def y ( self, win_y ):
    return ( (win_y - self.y_offset) / (self.y_scale * self.zoom_scale) )
  def w ( self, win_w ):
    return ( win_w / (self.x_scale * self.zoom_scale) )
  def h ( self, win_h ):
    return ( win_h / (self.y_scale * self.zoom_scale) )
    
  def set_fixed_aspect ( self, fixed_aspect ):
    pass
  def fitwidth  ( self, user_xmin, user_xmax, keep_aspect=False ):
    pass  # implies left to right
  def fitheight ( self, user_ymin, user_ymax, keep_aspect=False ):
    pass # implies top to bottom

  def set_scroll_factor ( self, scroll_factor ):
    pass

  def zoom_at_point ( self, zoom_delta, at_x, at_y ):
    # First save the mouse location in user space before the zoom
    user_x_at_zoom = self.x(at_x)
    user_y_at_zoom = self.y(at_y)
    # Perform the zoom by changing the zoom scale
    self.scroll_count += zoom_delta
    self.zoom_scale = pow (self.scroll_factor, self.scroll_count)
    # Get the new window coordinates of the previously saved user space location
    win_x_after_zoom = self.wx ( user_x_at_zoom )
    win_y_after_zoom = self.wy ( user_y_at_zoom )
    # Adjust the offsets (window coordinates) to keep user point at same location
    self.x_offset += at_x - win_x_after_zoom
    self.y_offset += at_y - win_y_after_zoom

  def set_cursor ( self, gtk_gdk_cursor ):
    self.drawing_area.window.set_cursor ( gtk.gdk.Cursor(gtk_gdk_cursor) )  # gtk.gdk.HAND2 DRAFT_SMALL TARGET HAND1 SB_UP_ARROW CROSS CROSSHAIR CENTER_PTR CIRCLE DIAMOND_CROSS IRON_CROSS PLUS CROSS_REVERSE DOT DOTBOX FLEUR


  def add_menu ( self, label ):
    menu = gtk.Menu()
    item = gtk.MenuItem(label)
    item.set_submenu ( menu )
    item.show()
    return (menu, item)

  def add_menu_item ( self, parent, callback, label, data, key=None, mask=gtk.gdk.CONTROL_MASK, ):
    item = gtk.MenuItem(label=label)
    item.connect ( "activate", callback, data )
    if key != None:
      item.add_accelerator("activate", self.accel_group, ord(key), mask, gtk.ACCEL_VISIBLE)
    parent.append ( item )
    item.show()

  def add_menu_sep ( self, parent ):
    item = gtk.SeparatorMenuItem()
    parent.append ( item )
    item.show()



def mouse_scroll_callback ( canvas, event ):
  global global_zpa
  # print ( "Mouse Scroll: " + str(canvas) + " event at (" + str() + "," + str(event.y) + ") : " + str(event) )
  #__import__('code').interact(local = locals())
  if event.direction == gtk.gdk.SCROLL_UP:
    global_zpa.zoom_at_point (  1, event.x, event.y )
    # print ( "Mouse scrolled up = zoom in (make everything larger)" )
  elif event.direction == gtk.gdk.SCROLL_DOWN:
    global_zpa.zoom_at_point ( -1, event.x, event.y )
    # print ( "Mouse scrolled down = zoom out (make everything smaller)" )
  elif event.direction == gtk.gdk.SCROLL_LEFT:
    pass
    # print ( "Mouse scrolled left" )
  elif event.direction == gtk.gdk.SCROLL_RIGHT:
    pass
    # print ( "Mouse scrolled right" )
  else:
    pass
    # print ( "Mouse scrolled other?" )

  global_zpa.drawing_area.queue_draw()

  return True  # Event has been handled, do not propagate further


def button_press_callback ( widget, event ):
  global global_zpa
  # print ( "A mouse button was pressed at x = " + str(event.x) + ", y = " + str(event.y) + "  state = " + str(event.state) )
  if event.button == 1:
    global_zpa.last_x = event.x
    global_zpa.last_y = event.y
    global_zpa.dragging = True
  widget.queue_draw()
  return True  # Event has been handled, do not propagate further


def button_release_callback ( widget, event ):
  global global_zpa
  # print ( "A mouse button was released at x = " + str(event.x) + ", y = " + str(event.y) + "  state = " + str(event.state) )
  if event.button == 1:
    global_zpa.x_offset += (event.x - global_zpa.last_x)
    global_zpa.y_offset += (event.y - global_zpa.last_y)
    global_zpa.last_x = event.x
    global_zpa.last_y = event.y
    global_zpa.dragging = False
  widget.queue_draw()
  return True  # Event has been handled, do not propagate further


def mouse_motion_callback ( canvas, event ):
  # width, height = canvas.window.get_size()
  if event.state == 0:
    #print ( "Hover: x = " + str(event.x) + ", y = " + str(event.y) + "  state = " + str(event.state) )
    pass
  elif event.state & gtk.gdk.BUTTON1_MASK:
    #print ( "Drag:  x = " + str(event.x) + ", y = " + str(event.y) + "  state = " + str(event.state) )
    #__import__('code').interact(local = locals())
    global_zpa.x_offset += (event.x - global_zpa.last_x)
    global_zpa.y_offset += (event.y - global_zpa.last_y)
    global_zpa.last_x = event.x
    global_zpa.last_y = event.y
    canvas.queue_draw()

  return False  # Event has been handled, do not propagate further


def key_press_callback ( widget, event ):
  print ( "Key press event: " + str(event.keyval) + " = " + str(event) )
  handled = False
  if event.type == gtk.gdk.KEY_PRESS:
    # __import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})
    global global_zpa
    if event.keyval == 65363:  # Right arrow: increase the x offset
      # print ("increasing offset from " + str(global_zpa.x_offset) )
      global_zpa.x_offset += -10
      # print ("                    to " + str(global_zpa.x_offset) )
    if event.keyval == 65361:  # Left arrow: decrease the x offset
      # print ("decreasing offset from " + str(global_zpa.x_offset) )
      global_zpa.x_offset += 10
      # print ("                    to " + str(global_zpa.x_offset) )
    if event.keyval == 65362:  # Up arrow: increase the y offset
      # print ("increasing offset from " + str(global_zpa.y_offset) )
      global_zpa.y_offset += 10
      # print ("                    to " + str(global_zpa.y_offset) )
    if event.keyval == 65364:  # Down arrow: decrease the y offset
      # print ("decreasing offset from " + str(global_zpa.y_offset) )
      global_zpa.y_offset += -10
      # print ("                    to " + str(global_zpa.y_offset) )
    widget.queue_draw()
    handled = True  # Event has been handled, do not propagate further
  return handled

"""
def reset_callback(zpa):
  zpa.set_defaults()
  zpa.set_x_scale ( -1.0,   0, 1.0, 400 )
  zpa.set_y_scale (  0.0, 300, 1.0,   0 )
  drawing_area = zpa.get_drawing_area()
  drawing_area.queue_draw()
  return True
"""


