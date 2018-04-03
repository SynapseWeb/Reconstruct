#!/usr/bin/python3

### Patterned after:  https://developer.gnome.org/gnome-devel-demos/stable/menubar.py.html.en

from gi.repository import Gtk, Gdk
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
              <attribute name="action">app.awesome</attribute>
            </item>
            <item>
              <attribute name="label">Status Bar</attribute>
              <attribute name="action">app.cool</attribute>
            </item>
          </section>
      </submenu>
      <section>
        <submenu>
          <attribute name="label">Debug</attribute>
          <section>
            <item>
              <attribute name="label">Logging</attribute>
            </item>
            <item>
              <attribute name="label">Times</attribute>
            </item>
          </section>
          <section>
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
                  <attribute name="action">app.awesome</attribute>
                </item>
                <item>
                  <attribute name="label">_Check 2</attribute>
                  <attribute name="action">app.cool</attribute>
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
          <attribute name="action">app.new</attribute>
        </item>
        <item>
          <attribute name="label">Close</attribute>
          <attribute name="action">app.new</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">New...</attribute>
          <attribute name="action">app.new</attribute>
        </item>
        <item>
          <attribute name="label">Save</attribute>
          <attribute name="action">app.new</attribute>
        </item>
        <item>
          <attribute name="label">Options... Ctrl+o</attribute>
          <attribute name="action">app.new</attribute>
        </item>
      </section>
      <section>
        <submenu>
          <attribute name="label">Export</attribute>
          <section>
            <item>
              <attribute name="label">Images...</attribute>
            </item>
            <item>
              <attribute name="label">Lines...</attribute>
            </item>
            <item>
              <attribute name="label">Trace lists...</attribute>
            </item>
          </section>
        </submenu>
        <submenu>
          <attribute name="label">Import</attribute>
          <section>
            <item>
              <attribute name="label">Images...</attribute>
            </item>
            <item>
              <attribute name="label">Lines...</attribute>
            </item>
            <item>
              <attribute name="label">Series...</attribute>
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
          <attribute name="action">app.new</attribute>
        </item>
        <item>
          <attribute name="label">Thumbnails...</attribute>
          <attribute name="action">app.new</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">New...</attribute>
          <attribute name="action">app.new</attribute>
        </item>
        <item>
          <attribute name="label">Save</attribute>
          <attribute name="action">app.new</attribute>
        </item>
        <item>
          <attribute name="label">Thickness...</attribute>
          <attribute name="action">app.new</attribute>
        </item>
      </section>
      <section>
        <submenu>
          <attribute name="label">Zoom</attribute>
          <section>
            <item>
              <attribute name="label">Center</attribute>
            </item>
            <item>
              <attribute name="label">Last</attribute>
            </item>
            <item>
              <attribute name="label">Actual pixels</attribute>
            </item>
            <item>
              <attribute name="label">Magnification...</attribute>
            </item>
          </section>
        </submenu>
        <submenu>
          <attribute name="label">Movement</attribute>
          <section>
            <item>
              <attribute name="label">Unlock</attribute>
            </item>
            <submenu>
              <attribute name="label">Flip</attribute>
              <item>
                <attribute name="label">Horizontally</attribute>
              </item>
              <item>
                <attribute name="label">Vertically</attribute>
              </item>
            </submenu>
            <submenu>
              <attribute name="label">Rotate</attribute>
              <item>
                <attribute name="label">90&#176; clockwise</attribute>
              </item>
              <item>
                <attribute name="label">90&#176; counterclockwise</attribute>
              </item>
              <item>
                <attribute name="label">180&#176;</attribute>
              </item>
            </submenu>
            <item>
              <attribute name="label">Type in...</attribute>
            </item>
          </section>
          <section>
            <item>
              <attribute name="label">By correlation</attribute>
            </item>
          </section>
          <section>
            <item>
              <attribute name="label">Repeat</attribute>
            </item>
            <item>
              <attribute name="label">Propagate...</attribute>
            </item>
          </section>
          <section>
            <submenu>
              <attribute name="label">Record</attribute>
              <item>
                <attribute name="label">Start</attribute>
              </item>
              <item>
                <attribute name="label">from selected</attribute>
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
        </item>
        <item>
          <attribute name="label">Import image...</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">Merge front</attribute>
        </item>
        <item>
          <attribute name="label">Merge rear</attribute>
        </item>
        <item>
          <attribute name="label">Attributes...</attribute>
        </item>
        <submenu>
          <attribute name="label">Reinitialize</attribute>
          <section>
            <item>
              <attribute name="label">Reinitialize Unknown_1</attribute>
            </item>
            <item>
              <attribute name="label">Reinitialize Unknown_2</attribute>
            </item>
          </section>
        </submenu>
      </section>
      <section>
        <item>
          <attribute name="label">Delete Del</attribute>
        </item>
      </section>
    </submenu>

    <submenu>
      <attribute name="label">Trace</attribute>
      <section>
        <item>
          <attribute name="label">List traces...</attribute>
        </item>
        <item>
          <attribute name="label">Find...</attribute>
        </item>
        <item>
          <attribute name="label">Select all</attribute>
        </item>
        <item>
          <attribute name="label">Deselect all</attribute>
        </item>
        <item>
          <attribute name="label">Zoom to</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">Attributes...</attribute>
        </item>
        <item>
          <attribute name="label">Palette...</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">Cut</attribute>
        </item>
        <item>
          <attribute name="label">Copy</attribute>
        </item>
        <item>
          <attribute name="label">Paste</attribute>
        </item>
        <item>
          <attribute name="label">Paste attributes</attribute>
        </item>
        <item>
          <attribute name="label">Delete</attribute>
        </item>
      </section>
      <section>
        <submenu>
          <attribute name="label">Align Section</attribute>
          <section>
            <item>
              <attribute name="label">Align Section Unknown_1</attribute>
            </item>
            <item>
              <attribute name="label">Align Section Unknown_2</attribute>
            </item>
          </section>
        </submenu>
        <item>
          <attribute name="label">Calibrate...</attribute>
        </item>
        <item>
          <attribute name="label">Merge</attribute>
        </item>
        <item>
          <attribute name="label">Reverse</attribute>
        </item>
        <item>
          <attribute name="label">Simplify</attribute>
        </item>
        <item>
          <attribute name="label">Smooth</attribute>
        </item>
      </section>
    </submenu>

    <submenu>
      <attribute name="label">Object</attribute>
      <section>
        <item>
          <attribute name="label">List objects...</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">3D Scene...</attribute>
        </item>
        <item>
          <attribute name="label">Z-Traces...</attribute>
        </item>
        <item>
          <attribute name="label">Distances...</attribute>
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

  <menu id="appmenu"> <!-- Application Menu is special -->
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
        Gtk.Window.__init__(self, title="MenuBar Example", application=app)
        self.set_default_size(720, 540)
        self.set_border_width ( 10 )

        self.draw_area = Gtk.DrawingArea()
        self.draw_area.connect ( "draw", self.draw_callback )
        self.draw_area.set_events ( Gdk.EventMask.BUTTON_PRESS_MASK )
        self.add ( self.draw_area )

        self.angle = 270

        copy_action = Gio.SimpleAction.new("copy", None)        # action without a state created (name, parameter type)
        copy_action.connect("activate", self.copy_callback)        # connected with the callback function
        self.add_action(copy_action)        # added to the window

        paste_action = Gio.SimpleAction.new("paste", None)        # action without a state created (name, parameter type)
        paste_action.connect("activate", self.paste_callback)        # connected with the callback function
        self.add_action(paste_action)        # added to the window

        shape_action = Gio.SimpleAction.new_stateful("shape", GLib.VariantType.new('s'), GLib.Variant.new_string('triangle')) # action with a state created (name, parameter type, initial state)
        shape_action.connect("activate", self.shape_callback)        # connected to the callback function
        self.add_action(shape_action)        # added to the window

        about_action = Gio.SimpleAction.new("about", None)        # action with a state created
        about_action.connect("activate", self.about_callback)        # action connected to the callback function
        self.add_action(about_action)        # action added to the application


    def get_angle ( self, event ):
        self.angle = self.spin.get_value_as_int()
        self.draw_area.queue_draw()

    def draw_callback ( self, draw_area, cr ):
        cr.set_line_width(10)
        cr.set_source_rgba ( 0.5, 0.0, 0.0, 1.0 )
        w = self.draw_area.get_allocated_width()
        h = self.draw_area.get_allocated_height()
        cr.translate ( w/2, h/2 )
        cr.line_to ( 55, 0 )
        cr.line_to (  0, 0 )
        cr.arc ( 0, 0, 50, 0, self.angle * ( math.pi / 180 ) )
        cr.line_to (  0, 0 )
        cr.stroke_preserve()
        cr.set_source_rgba ( 0.0, 0.5, 0.5, 1.0 )
        cr.fill()


    def copy_callback(self, action, parameter):
        print("\"Copy\" activated")

    def paste_callback(self, action, parameter):
        print("\"Paste\" activated")

    def shape_callback(self, action, parameter):
        print("Shape is set to", parameter.get_string())
        # Note that we set the state of the action!
        action.set_state(parameter)

    def about_callback(self, action, parameter):
        aboutdialog = Gtk.AboutDialog()

        authors = ["SynapseWeb Team"]
        documenters = ["SynapseWeb Team"]

        aboutdialog.set_program_name("Reconstruct Python3 GTK Demo")
        aboutdialog.set_copyright("Copyright \xa9 2018")
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

        awesome_action = Gio.SimpleAction.new_stateful("awesome", None, GLib.Variant.new_boolean(False))        # action with a state created
        awesome_action.connect("activate", self.awesome_callback)     # action connected to the callback function
        self.add_action(awesome_action)                               # action added to the application

        cool_action = Gio.SimpleAction.new_stateful("cool", None, GLib.Variant.new_boolean(False))  # action with a state created
        cool_action.connect("activate", self.cool_callback)        # action connected to the callback function
        self.add_action(cool_action)                               # action added to the application


        builder = Gtk.Builder()        # a builder to add the UI designed with Glade to the grid:
        # get the file (if it is there)
        try:
            # builder.add_from_file("menubar.ui")
            builder.add_from_string(MENU_XML)
        except:
            print("Unable to load the Menu Definition XML")
            sys.exit()

        # we use the method Gtk.Application.set_menubar(menubar) to add the menubar
        # and the menu to the application (Note: NOT the window!)
        self.set_menubar(builder.get_object("menubar"))
        
        # self.set_app_menu(builder.get_object("appmenu"))  # This forces an "Application" menu item
        self.set_app_menu(None) # No "Application" menu item

    def new_callback(self, action, parameter):
        print("You clicked \"New\"")

    def quit_callback(self, action, parameter):
        print("You clicked \"Quit\"")
        sys.exit()

    def state_callback(self, action, parameter):
        print("State is set to", parameter.get_string())
        action.set_state(parameter)

    def awesome_callback(self, action, parameter):
        action.set_state(GLib.Variant.new_boolean(not action.get_state()))
        if action.get_state().get_boolean() is True:
            print("You checked \"Awesome\" or \"Cool\"")
        else:
            print("You unchecked \"Awesome\" or \"Cool\"")

    def cool_callback(self, action, parameter):
        action.set_state(GLib.Variant.new_boolean(not action.get_state()))
        if action.get_state().get_boolean() is True:
            print("You checked \"Cool\"")
        else:
            print("You unchecked \"Cool\"")


app = MyApplication()
exit_status = app.run(sys.argv)
sys.exit(exit_status)
