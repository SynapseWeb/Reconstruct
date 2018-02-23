bl_info = {
	"name": "Object Tracer",
	"author": "Bob Kuczewski",
	"version": (1,0,0),
	"blender": (2,5,6),
	"location": "View 3D > Tool Shelf",
	"description": "Object Tracer",
	"warning" : "",
	"category": "Blender Experiments",
}


import bpy
from bpy.props import *


class DataBrowserStringProperty(bpy.types.PropertyGroup):
    v = StringProperty(name="DBString")

class DataBrowserIntProperty(bpy.types.PropertyGroup):
    v = IntProperty(name="DBInt")

class DataBrowserFloatProperty(bpy.types.PropertyGroup):
    v = FloatProperty(name="DBFloat")

class DataBrowserBoolProperty(bpy.types.PropertyGroup):
    v = BoolProperty(name="DBBool")



class ObjectTracerPropertyGroup(bpy.types.PropertyGroup):
    internal_file_name = StringProperty ( name = "Text File Name" )
    image_file_name = StringProperty ( name = "Image File Name", subtype='FILE_PATH', default="" )

    string_list = CollectionProperty(type=DataBrowserStringProperty, name="String List")
    int_list    = CollectionProperty(type=DataBrowserIntProperty,    name="Int List")
    float_list  = CollectionProperty(type=DataBrowserFloatProperty,  name="Float List")
    bool_list   = CollectionProperty(type=DataBrowserBoolProperty,   name="Bool List")

    def draw_layout ( self, context, layout ):

        row = layout.row()
        row.prop ( self, "image_file_name" )

        #row = layout.row()
        #col = row.column()
        #col.operator ( "browse.from_file" )
        #col = row.column()
        #col.operator ( "browse.to_file" )
        row = layout.row()
        row.operator ( "tracer.tk_draw_tracer" )
        


class ToFileOperator(bpy.types.Operator):
    bl_idname = "browse.to_file"
    bl_label = "To File"

    def invoke(self, context, event):
        return{'FINISHED'}


class FromFileOperator(bpy.types.Operator):
    bl_idname = "browse.from_file"
    bl_label = "From File"

    def invoke(self, context, event):
        db = context.scene.object_tracer
        return{'FINISHED'}




# Set up to run tkinter code in another thread
canvas = None
last_x = -99999
last_y = -99999
image_name = None
line_segments = []

def paint_canvas ( event ):
    global last_x
    global last_y
    global line_segments
    if canvas != None:
        draw_color = "#ff0000"
        #x1, y1 = ( event.x - 2 ), ( event.y - 2 )
        #x2, y2 = ( event.x + 2 ), ( event.y + 2 )
        #canvas.create_oval( x1, y1, x2, y2, fill=draw_color, outline=draw_color )
        if last_x != -99999:
            l = canvas.create_line ( last_x, last_y, event.x, event.y, fill=draw_color, width=3, tag='line' )
            line_segments.append ( ((last_x,last_y),(event.x,event.y)) )
        last_x = event.x
        last_y = event.y

try:
    import tkinter as tk
    from tkinter import messagebox
    import threading

    import bpy

    class ObjectTracer(threading.Thread):

        depth = 0

        copy_with_pretty_print = True

        w = 800
        h = 800

        def __init__(self):
            threading.Thread.__init__(self)
            self.start()


        browser_data_model = None

        def clear_drawing(self):
            global last_x
            global last_y
            global line_segments
            last_x = -99999
            last_y = -99999
            self.canvas.delete('line')
            line_segments = []

        def print_trace(self):
            global line_segments
            print ( "Segments:" )
            for s in line_segments:
              print ( "  " + str(s) )

        def load_data_model(self):
            self.browser_data_model = { 'mcell' : {'mols': ['a', 'b', 'c'], 'iters': 10 } }


        def set_copy_as_one_line(self):
            self.copy_with_pretty_print = False

        def set_copy_as_pretty_print(self):
            self.copy_with_pretty_print = True

        def resize_window(self, event):
            if str(event.widget) == '.':
                self.w = event.width

        def copy_selected(self):
            if len(self.tree.selection()) > 0:
                selected_item = self.tree.item(self.tree.selection()[0])
                self.root.clipboard_clear()
                self.root.clipboard_append(str(selected_item['text']))

        def destroy(self):
            #if messagebox.askyesno("Exit", "Do you want to close the Tracer?"):
            #    # print ( "Destroying Tk" )
            #    self.root.destroy()
            self.root.destroy()

        def debug(self):
            __import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})

        def run(self):
            self.root = tk.Tk()
            self.root.wm_title("CellBlender Image Tracer")
            self.root.protocol("WM_DELETE_WINDOW", self.destroy)

            self.top = self.root.winfo_toplevel()
            self.menuBar = tk.Menu(self.top)
            self.top['menu'] = self.menuBar

            self.trace_menu = tk.Menu ( self.menuBar )
            self.trace_menu.add_command ( label='Start Trace', command = self.clear_drawing )
            self.trace_menu.add_command ( label='End Trace', command = self.clear_drawing )
            self.trace_menu.add_command ( label='Add Trace', command = self.clear_drawing )
            self.trace_menu.add_command ( label='Clear Trace', command = self.clear_drawing )
            self.trace_menu.add_command ( label='Print Trace', command = self.print_trace )
            #self.trace_menu.add_command ( label='Reload', command = self.load_data_model )
            #self.trace_menu.add_command ( label='Copy with Pretty Print', command = self.set_copy_as_pretty_print )
            #self.trace_menu.add_command ( label='Copy as One Line', command = self.set_copy_as_one_line )
            self.menuBar.add_cascade(label="Traces", menu=self.trace_menu)

            self.frame_menu = tk.Menu ( self.menuBar )
            self.frame_menu.add_command ( label='Next Frame', command = self.clear_drawing )
            self.frame_menu.add_command ( label='Prev Frame', command = self.clear_drawing )
            self.frame_menu.add_command ( label='First Frame', command = self.clear_drawing )
            self.frame_menu.add_command ( label='Last Frame', command = self.clear_drawing )
            #self.frame_menu.add_command ( label='Reload', command = self.load_data_model )
            #self.frame_menu.add_command ( label='Copy with Pretty Print', command = self.set_copy_as_pretty_print )
            #self.frame_menu.add_command ( label='Copy as One Line', command = self.set_copy_as_one_line )
            self.menuBar.add_cascade(label="Frames", menu=self.frame_menu)

            self.root.bind_all ( '<Configure>', self.resize_window )
            
            self.canvas = tk.Canvas(self.root, width=800, height=600)
            self.canvas.pack(expand=tk.YES, fill=tk.BOTH)
            self.canvas.bind( "<Button>", paint_canvas )
            self.canvas.bind( "<B1-Motion>", paint_canvas )

            message = tk.Label( self.root, text = "Draw with click-and-drag or click-and-click" )
            message.pack( side = tk.BOTTOM )
    
            global canvas
            global image_file
            canvas = self.canvas

            print ( "Image file is " + str(image_name) )
            if len(image_name) > 0:
                #__import__('code').interact(local={k: v for ns in (globals(), locals()) for k, v in ns.items()})
                img = tk.PhotoImage(file=image_name)
                canvas.create_image(0,0,image=img,anchor=tk.NW)

            #self.tree = ttk.Treeview(self.root, show='tree', selectmode='browse' )
            #self.tree.bind ( '<<TreeviewSelect>>', self.item_select )
            #vscroll = ttk.Scrollbar(self.root, orient='vertical', command=self.tree.yview)
            #self.tree.configure(yscroll=vscroll.set)

            #self.tree.pack(fill=tk.BOTH,expand=1)

            #self.load_data_model()

            self.root.mainloop()

except ( ImportError ):
    # Unable to import needed libraries so don't draw
    print ( "Unable to import libraries needed for Image Tracer ... most likely tkinter" )
    pass


try:

    class TkDrawBounds(bpy.types.Operator):
        '''Trace Bounds with a Tk Application (requires tkinter installation)'''
        bl_idname = "tracer.tk_draw_tracer"
        bl_label = "Launch Tracer with Tk"
        bl_description = "Trace Bounds with a Tk Application (requires tkinter installation)"

        def execute(self, context):
            print ( "Launch Object Tracer" )
            global last_x
            global last_y
            global image_name
            last_x = -99999
            last_y = -99999
            image_name = context.scene.object_tracer.image_file_name
            app = ObjectTracer()
            return {'FINISHED'}


except:
    # Unable to import Blender classses
    print ( "Unable to import Blender classes ... running outside of Blender." )
    pass




class Data_Browser_Panel(bpy.types.Panel):
  bl_label = "Object Tracer"
  bl_space_type = "VIEW_3D"
  bl_region_type = "TOOLS"
  bl_category = "Object Tracer"
  bl_options = {'DEFAULT_CLOSED'}

  def draw(self, context):
    db = context.scene.object_tracer
    db.draw_layout(context, self.layout)


def register():
    bpy.utils.register_module(__name__)
    bpy.types.Scene.object_tracer = bpy.props.PointerProperty(type=ObjectTracerPropertyGroup)

def unregister():
    del bpy.types.Scene.object_tracer
    bpy.utils.unregister_module(__name__)

if __name__ == "__main__":
    register()



