import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

import app_window

class MyWindow(Gtk.Window):

    def __init__(self):
        Gtk.Window.__init__(self)
        self.set_title ( "Reconstruct Python3 GTK+ GObj Demonstration" )
        self.button = Gtk.Button(label="Click Here")
        self.button.connect("clicked", self.on_button_clicked)
        self.add(self.button)

    def on_button_clicked(self, widget):
        print("Hello World")

win = MyWindow()
win.connect("destroy", Gtk.main_quit)
win.show_all()
Gtk.main()

