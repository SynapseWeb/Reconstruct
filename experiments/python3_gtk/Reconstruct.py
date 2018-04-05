#!/usr/bin/python3

### Patterned after:  https://developer.gnome.org/gnome-devel-demos/stable/menubar.py.html.en

from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import GdkPixbuf
from gi.repository import GLib
from gi.repository import Gio
from gi.repository import cairo
import sys
import math

# This would typically be its own file
MENU_XML = """
<?xml version="1.0" encoding="UTF-8"?>
<interface>

  <menu id="menubar">

    <submenu>
      <attribute name="label">_Program</attribute>
      <submenu>
        <attribute name="label">Windows</attribute>
          <section>
            <item>
              <attribute name="label">Tools Window</attribute>
              <attribute name="action">app.tools_window</attribute>
            </item>
            <item>
              <attribute name="label">Status Bar</attribute>
              <attribute name="action">app.status_bar</attribute>
            </item>
          </section>
      </submenu>
      <section>
        <submenu>
          <attribute name="label">Debug</attribute>
          <section>
            <item>
              <attribute name="label">Console</attribute>
              <attribute name="action">win.console</attribute>
            </item>
            <item>
              <attribute name="label">Choose File...</attribute>
              <attribute name="action">win.open</attribute>
            </item>
            <item>
              <attribute name="label">Logging</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Times</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
          </section>
          <section>

            <submenu>
              <attribute name="label">Set _Angle</attribute>
              <section>
                <item>
                  <attribute name="label">a=0</attribute>
                  <attribute name="action">win.float_angle</attribute>
                  <attribute name="target">0.0</attribute>
                </item>
                <item>
                  <attribute name="label">a=45</attribute>
                  <attribute name="action">win.float_angle</attribute>
                  <attribute name="target">45.0</attribute>
                </item>
                <item>
                  <attribute name="label">a=90</attribute>
                  <attribute name="action">win.float_angle</attribute>
                  <attribute name="target">90.0</attribute>
                </item>
                <item>
                  <attribute name="label">a=180</attribute>
                  <attribute name="action">win.float_angle</attribute>
                  <attribute name="target">180.0</attribute>
                </item>
                <item>
                  <attribute name="label">a=270</attribute>
                  <attribute name="action">win.float_angle</attribute>
                  <attribute name="target">270.0</attribute>
                </item>
                <item>
                  <attribute name="label">a=360</attribute>
                  <attribute name="action">win.float_angle</attribute>
                  <attribute name="target">360.0</attribute>
                </item>
              </section>
            </submenu>

            <submenu>
              <attribute name="label">Set _Line Width</attribute>
              <section>
                <item>
                  <attribute name="label">w=0</attribute>
                  <attribute name="action">win.line_width</attribute>
                  <attribute name="target">0</attribute>
                </item>
                <item>
                  <attribute name="label">w=1</attribute>
                  <attribute name="action">win.line_width</attribute>
                  <attribute name="target">1</attribute>
                </item>
                <item>
                  <attribute name="label">w=2</attribute>
                  <attribute name="action">win.line_width</attribute>
                  <attribute name="target">2</attribute>
                </item>
                <item>
                  <attribute name="label">w=3</attribute>
                  <attribute name="action">win.line_width</attribute>
                  <attribute name="target">3</attribute>
                </item>
                <item>
                  <attribute name="label">w=5</attribute>
                  <attribute name="action">win.line_width</attribute>
                  <attribute name="target">5</attribute>
                </item>
                <item>
                  <attribute name="label">w=9</attribute>
                  <attribute name="action">win.line_width</attribute>
                  <attribute name="target">9</attribute>
                </item>
              </section>
            </submenu>

            <submenu>
              <attribute name="label">_Choice Testing</attribute>
              <submenu>
                <attribute name="label">_Shapes</attribute>
                  <section>
                    <item>
                      <attribute name="label">_Line</attribute>
                      <attribute name="action">win.shape</attribute>
                      <attribute name="target">line</attribute>
                    </item>
                    <item>
                      <attribute name="label">_Triangle</attribute>
                      <attribute name="action">win.shape</attribute>
                      <attribute name="target">triangle</attribute>
                    </item>
                    <item>
                      <attribute name="label">_Square</attribute>
                      <attribute name="action">win.shape</attribute>
                      <attribute name="target">square</attribute>
                    </item>
                    <item>
                      <attribute name="label">_Polygon</attribute>
                      <attribute name="action">win.shape</attribute>
                      <attribute name="target">polygon</attribute>
                    </item>
                    <item>
                      <attribute name="label">_Circle</attribute>
                      <attribute name="action">win.shape</attribute>
                      <attribute name="target">circle</attribute>
                    </item>
                  </section>
              </submenu>
              <section>
                <item>
                  <attribute name="label">_On</attribute>
                  <attribute name="action">app.state</attribute>
                  <attribute name="target">on</attribute>
                </item>
                <item>
                  <attribute name="label">_Off</attribute>
                  <attribute name="action">app.state</attribute>
                  <attribute name="target">off</attribute>
                </item>
              </section>
              <section>
                <item>
                  <attribute name="label">_Check 1</attribute>
                  <attribute name="action">app.tools_window</attribute>
                </item>
                <item>
                  <attribute name="label">_Check 2</attribute>
                  <attribute name="action">app.status_bar</attribute>
                </item>
              </section>
            </submenu>
          </section>
        </submenu>
        <item>
          <attribute name="label">Exit</attribute>
          <attribute name="action">app.quit</attribute>
        </item>
      </section>
    </submenu>

    <submenu>
      <attribute name="label">Series</attribute>
      <section>
        <item>
          <attribute name="label">Open...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Close</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">New...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Save</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Options... Ctrl+o</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
      <section>
        <submenu>
          <attribute name="label">Export</attribute>
          <section>
            <item>
              <attribute name="label">Images...</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Lines...</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Trace lists...</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
          </section>
        </submenu>
        <submenu>
          <attribute name="label">Import</attribute>
          <section>
            <item>
              <attribute name="label">Images...</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Lines...</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Series...</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
          </section>
        </submenu>
      </section>
    </submenu>

    <submenu>
      <attribute name="label">Section</attribute>
      <section>
        <item>
          <attribute name="label">List sections...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Thumbnails...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">New...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Save</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Thickness...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
      <section>
        <submenu>
          <attribute name="label">Zoom</attribute>
          <section>
            <item>
              <attribute name="label">Center</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Last</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Actual pixels</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Magnification...</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
          </section>
        </submenu>
        <submenu>
          <attribute name="label">Movement</attribute>
          <section>
            <item>
              <attribute name="label">Unlock</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <submenu>
              <attribute name="label">Flip</attribute>
              <item>
                <attribute name="label">Horizontally</attribute>
                <attribute name="action">win.generic_window</attribute>
              </item>
              <item>
                <attribute name="label">Vertically</attribute>
                <attribute name="action">win.generic_window</attribute>
              </item>
            </submenu>
            <submenu>
              <attribute name="label">Rotate</attribute>
              <item>
                <attribute name="label">90&#176; clockwise</attribute>
                <attribute name="action">win.generic_window</attribute>
              </item>
              <item>
                <attribute name="label">90&#176; counterclockwise</attribute>
                <attribute name="action">win.generic_window</attribute>
              </item>
              <item>
                <attribute name="label">180&#176;</attribute>
                <attribute name="action">win.generic_window</attribute>
              </item>
            </submenu>
            <item>
              <attribute name="label">Type in...</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
          </section>
          <section>
            <item>
              <attribute name="label">By correlation</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
          </section>
          <section>
            <item>
              <attribute name="label">Repeat</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Propagate...</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
          </section>
          <section>
            <submenu>
              <attribute name="label">Record</attribute>
              <item>
                <attribute name="label">Start</attribute>
                <attribute name="action">win.generic_window</attribute>
              </item>
              <item>
                <attribute name="label">from selected</attribute>
                <attribute name="action">win.generic_window</attribute>
              </item>
            </submenu>
          </section>
        </submenu>
      </section>
    </submenu>

    <submenu>
      <attribute name="label">Domain</attribute>
      <section>
        <item>
          <attribute name="label">List image domains...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Import image...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">Merge front</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Merge rear</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Attributes...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <submenu>
          <attribute name="label">Reinitialize</attribute>
          <section>
            <item>
              <attribute name="label">Reinitialize Unknown_1</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Reinitialize Unknown_2</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
          </section>
        </submenu>
      </section>
      <section>
        <item>
          <attribute name="label">Delete Del</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
    </submenu>

    <submenu>
      <attribute name="label">Trace</attribute>
      <section>
        <item>
          <attribute name="label">List traces...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Find...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Select all</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Deselect all</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Zoom to</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">Attributes...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Palette...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">Cut</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Copy</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Paste</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Paste attributes</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Delete</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
      <section>
        <submenu>
          <attribute name="label">Align Section</attribute>
          <section>
            <item>
              <attribute name="label">Align Section Unknown_1</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
            <item>
              <attribute name="label">Align Section Unknown_2</attribute>
              <attribute name="action">win.generic_window</attribute>
            </item>
          </section>
        </submenu>
        <item>
          <attribute name="label">Calibrate...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Merge</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Reverse</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Simplify</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Smooth</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
    </submenu>

    <submenu>
      <attribute name="label">Object</attribute>
      <section>
        <item>
          <attribute name="label">List objects...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">3D Scene...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Z-Traces...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
        <item>
          <attribute name="label">Distances...</attribute>
          <attribute name="action">win.generic_window</attribute>
        </item>
      </section>
    </submenu>

    <submenu>
      <attribute name="label">Help</attribute>
      <section>
        <item>
          <attribute name="label">Manual...</attribute>
          <attribute name="action">win.about</attribute>
        </item>
        <item>
          <attribute name="label">Key commands...</attribute>
          <attribute name="action">win.about</attribute>
        </item>
        <item>
          <attribute name="label">Mouse clicks...</attribute>
          <attribute name="action">win.about</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">License...</attribute>
          <attribute name="action">win.about</attribute>
        </item>
        <item>
          <attribute name="label">Version...</attribute>
          <attribute name="action">win.about</attribute>
        </item>
      </section>
    </submenu>

  </menu>

  <menu id="appmenu"> <!-- Application Menu is a special menu (typically first in a menu bar) -->
    <section>
      <item>
        <attribute name="label">New</attribute>
        <attribute name="action">app.new</attribute>
      </item>
      <item>
        <attribute name="label">Quit</attribute>
        <attribute name="action">app.quit</attribute>
      </item>
    </section>
  </menu>

</interface>
"""


class MyWindow(Gtk.ApplicationWindow):

    def __init__(self, app):
        Gtk.Window.__init__(self, title="Reconstruct Python3 Gtk+", application=app)
        self.set_default_size(720, 540)
        self.set_border_width ( 10 )

        self.draw_area = Gtk.DrawingArea()
        self.draw_area.connect ( "draw", self.draw_callback )
        self.draw_area.set_events ( Gdk.EventMask.BUTTON_PRESS_MASK )
        self.add ( self.draw_area )

        self.angle = 270.0
        self.line_width = 3

        generic_window_action = Gio.SimpleAction.new("generic_window", None)        # action without a state created (name, parameter type)
        generic_window_action.connect("activate", self.generic_window_callback)     # connected with the callback function
        self.add_action(generic_window_action)                                      # added to the window

        shape_action = Gio.SimpleAction.new_stateful("shape", GLib.VariantType.new('s'), GLib.Variant.new_string('triangle')) # action with a state created (name, parameter type, initial state)
        shape_action.connect("activate", self.shape_callback)        # connected to the callback function
        self.add_action(shape_action)        # added to the window

        float_angle_action = Gio.SimpleAction.new_stateful("float_angle", GLib.VariantType.new('s'), GLib.Variant.new_string(str(self.angle))) # action with a state created (name, parameter type, initial state)
        float_angle_action.connect("activate", self.float_angle_callback)        # connected to the callback function
        self.add_action(float_angle_action)        # added to the window

        line_width_action = Gio.SimpleAction.new_stateful("line_width", GLib.VariantType.new('s'), GLib.Variant.new_string(str(self.line_width))) # action with a state created (name, parameter type, initial state)
        line_width_action.connect("activate", self.line_width_callback)        # connected to the callback function
        self.add_action(line_width_action)        # added to the window

        about_action = Gio.SimpleAction.new("about", None)        # action with a state created
        about_action.connect("activate", self.about_callback)        # action connected to the callback function
        self.add_action(about_action)        # action added to the application

        open_action = Gio.SimpleAction.new("open", None)        # action with a state created
        open_action.connect("activate", self.open_callback)        # action connected to the callback function
        self.add_action(open_action)        # action added to the application

        console_action = Gio.SimpleAction.new("console", None)        # action with a state created
        console_action.connect("activate", self.console_callback)        # action connected to the callback function
        self.add_action(console_action)        # action added to the application

        self.image_name = None

        self.image_frame = None  # = GdkPixbuf.Pixbuf.new_from_file ( "test.png" )

    def get_angle ( self, event ):
        self.angle = self.spin.get_value_as_int()
        self.draw_area.queue_draw()

    def draw_callback ( self, draw_area, cr ):

        if self.image_frame != None:

            # This seems to lock up the window manager:

            #__import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})

            #drawable.draw_pixbuf ( gc, scaled_image, 0, 0, zpa.wxi(0), zpa.wyi(0), -1, -1, gtk.gdk.RGB_DITHER_NONE )

            # From https://www.cairographics.org/cookbook/

            #cr.set_source ( self.image_frame )
            #cr.paint()
            pass

        else:
            cr.set_line_width(self.line_width)
            cr.set_source_rgba ( 0.5, 0.0, 0.0, 1.0 )
            w = self.draw_area.get_allocated_width()
            h = self.draw_area.get_allocated_height()
            cr.translate ( w/2, h/2 )
            cr.line_to ( 50, 0 )
            cr.line_to (  0, 0 )
            cr.arc ( 0, 0, 50, 0, self.angle * ( math.pi / 180 ) )
            cr.line_to (  0, 0 )
            cr.stroke_preserve()
            cr.set_source_rgba ( 0.0, 0.5, 0.5, 1.0 )
            cr.fill()

            # From http://www.tortall.net/mu/wiki/CairoTutorial

            """
            cr.set_line_width(5)
            cr.set_source_rgb(0, 0, 0)
            cr.rectangle(10, 10, 50, 50)
            cr.fill()
            """


        #__import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})


    def generic_window_callback(self, action, parameter):
        print ("\"Generic Window\" action activated")
        print ( "  action = " + str(action) )
        print ( "  parameter = " + str(parameter) )
        #__import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})

    def shape_callback(self, action, parameter):
        print("Shape is set to", parameter.get_string())
        #__import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})

        # Note that we set the state of the action!
        action.set_state(parameter)

    def float_angle_callback(self, action, parameter):
        if action.get_name() == "float_angle":
            print("Angle is set to", parameter.get_string())
            #__import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})
            self.angle = float(parameter.get_string())
            self.draw_area.queue_draw()

        # Note that we set the state of the action!
        action.set_state(parameter)


    def line_width_callback(self, action, parameter):
        if action.get_name() == "line_width":
            print("Line width is set to", parameter.get_string())
            #__import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})
            self.line_width = int(parameter.get_string())
            self.draw_area.queue_draw()

        # Note that we set the state of the action!
        action.set_state(parameter)


    def open_callback(self, action, parameter):
        dialog = Gtk.FileChooserDialog("Open Image", self.get_toplevel(), Gtk.FileChooserAction.OPEN)
        dialog.add_button ( Gtk.STOCK_CANCEL, 0 )
        dialog.add_button ( Gtk.STOCK_OK, 1 )
        dialog.set_default_response(1)
        filefilter = Gtk.FileFilter()
        filefilter.add_pixbuf_formats()
        dialog.set_filter(filefilter)
        if dialog.run() == 1:
            self.image_frame = GdkPixbuf.Pixbuf.new_from_file ( dialog.get_filename() )
        dialog.destroy()


    def console_callback(self, action, parameter):
        __import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})


    def about_callback(self, action, parameter):
        aboutdialog = Gtk.AboutDialog()

        authors = ["SynapseWeb Team"]
        documenters = ["SynapseWeb Team"]

        aboutdialog.set_program_name("Reconstruct Python3 GTK Demo")
        aboutdialog.set_copyright("Copyright \u00a9 2018")
        aboutdialog.set_authors(authors)
        aboutdialog.set_documenters(documenters)
        aboutdialog.set_website("http://sites.cns.utexas.edu/synapseweb")
        aboutdialog.set_website_label("SynapseWeb Portal")

        # to close the aboutdialog when "close" is clicked we connect the
        # "response" signal to on_close
        aboutdialog.connect("response", self.on_close)
        # show the aboutdialog
        aboutdialog.show()

    # Callback function to destroy the aboutdialog
    def on_close(self, action, parameter):
        action.destroy()


class MyApplication(Gtk.Application):

    def __init__(self):
        Gtk.Application.__init__(self)

    def do_activate(self):
        win = MyWindow(self)
        win.show_all()

    def do_startup(self):
        # FIRST THING TO DO: do_startup()
        Gtk.Application.do_startup(self)

        new_action = Gio.SimpleAction.new("new", None)     # action without a state created
        new_action.connect("activate", self.new_callback)  # action connected to the callback function
        self.add_action(new_action)                        # action added to the application

        quit_action = Gio.SimpleAction.new("quit", None)        # action without a state created
        quit_action.connect("activate", self.quit_callback)     # action connected to the callback function
        self.add_action(quit_action)                            # action added to the application

        state_action = Gio.SimpleAction.new_stateful("state", GLib.VariantType.new('s'), GLib.Variant.new_string('off'))  # action with a state created
        state_action.connect("activate", self.state_callback)   # action connected to the callback function
        self.add_action(state_action)                           # action added to the application

        tools_window_action = Gio.SimpleAction.new_stateful("tools_window", None, GLib.Variant.new_boolean(False))        # action with a state created
        tools_window_action.connect("activate", self.tools_window_callback)     # action connected to the callback function
        self.add_action(tools_window_action)                                    # action added to the application

        status_bar_action = Gio.SimpleAction.new_stateful("status_bar", None, GLib.Variant.new_boolean(False))  # action with a state created
        status_bar_action.connect("activate", self.status_bar_callback)         # action connected to the callback function
        self.add_action(status_bar_action)                                      # action added to the application


        builder = Gtk.Builder()  # Builder can read XML to build the application. The XML can be hand coded or come from Glade
        # get the file (if it is there)
        try:
            # builder.add_from_file("menubar.ui")
            builder.add_from_string(MENU_XML)
        except:
            print("Unable to load the Menu Definition XML")
            sys.exit()

        # Use Gtk.Application.set_menubar(menubar) to add the menubar
        # and the menu to the application (Note: NOT the window!)
        self.set_menubar(builder.get_object("menubar"))
        
        # self.set_app_menu(builder.get_object("appmenu"))  # This forces an "Application" menu (typically the first item in a menu bar)
        self.set_app_menu(None) # No "Application" menu item

    def new_callback(self, action, parameter):
        print("You clicked \"New\"")

    def quit_callback(self, action, parameter):
        print("You clicked \"Quit\"")
        sys.exit()

    def state_callback(self, action, parameter):
        print("State is set to", parameter.get_string())
        action.set_state(parameter)

    def tools_window_callback(self, action, parameter):
        action.set_state(GLib.Variant.new_boolean(not action.get_state()))
        if action.get_state().get_boolean() is True:
            print("You checked \"Tools Window\"")
        else:
            print("You unchecked \"Tools Window\"")

    def status_bar_callback(self, action, parameter):
        action.set_state(GLib.Variant.new_boolean(not action.get_state()))
        if action.get_state().get_boolean() is True:
            print("You checked \"Status Bar\"")
        else:
            print("You unchecked \"Status Bar\"")


app = MyApplication()
exit_status = app.run(sys.argv)
sys.exit(exit_status)
