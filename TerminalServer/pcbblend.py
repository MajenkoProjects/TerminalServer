import bpy
import csv
import sys
import os
from mathutils import Vector, Matrix, Quaternion
import time
from glob import glob
from math import radians
import bmesh
import random

# Useful colours

# Finishes
hasl = (0.6, 0.6, 0.6, 1.0)
enig = (0.865, 0.524, 0.0, 1.0)

# Silk colours
white = (1.0, 1.0, 1.0, 1.0)
black = (0.0, 0.0, 0.0, 1.0)

# Mask colours
red = (0.347, 0.000, 0.022, 1.0)
green = (0.0, 0.17, 0.015, 1.0)
blue = (0.0, 0.195, 0.828, 1.0)
purple = (0.174, 0.0, 0.574, 1.0)
yellow = (0.814, 0.429, 0.0, 1.0)
# black - use silk colour
# white - use silk colour

class PCBImport(bpy.types.Operator):

    # Location where all your converted files are stored
    file_root       = "/home/matt/git/TerminalServer/TerminalServer"

    # Base name of the files
    file_name       = "TerminalServer"

    # Colour you want for the mask: blue, red, green, purple, black, white, yellow
    color           = green

    # Finish for the board: hasl, enig
    finish          = hasl
    

    # Silk screen colour: black, white
    silk            = white

    camera = [ 52.5315, -159.018, 161.144 ]
    target = (2.6644, -9.21087, 0)    
    

    # Location where all the component library files are stored
    component_root = "/home/matt/git/3D_Components"

    # Whether or not to join everything into one single object
    doJoin          = False

    # List of manual rotations in the form of {"refdes": angle, "refdes": angle...}
    rotations = {
    }

    dnp = {

    }

    bl_label = "Import gEDA PCB Models"
    bl_idname = "wm.modal_timer_operator"
    bl_options = {'BLOCKING'}

    file_outline    = file_root + "/" + file_name + ".stl"
    file_csv        = file_root + "/Gerber/" + file_name + "-top-pos.csv"

    components = []

    txt = None    
    pcb = None
    objects = None
    

    def NormalInDirection(self, normal, direction, limit = 0.5 ):
        return direction.dot( normal ) > limit

    def GoingUp(self, normal, limit = 0.5 ):
        return self.NormalInDirection( normal, Vector( (0, 0, 1 ) ), limit )

    def GoingDown(self, normal, limit = 0.5 ):
        return self.NormalInDirection( normal, Vector( (0, 0, -1 ) ), limit )

    def GoingLeft(self, normal, limit = 0.5 ):
        return self.NormalInDirection( normal, Vector( (-1, 0, 0) ), limit)

    def GoingRight(self, normal, limit = 0.5 ):
        return self.NormalInDirection( normal, Vector( (1, 0, 0) ), limit)

    def GoingFore(self, normal, limit = 0.5 ):
        return self.NormalInDirection( normal, Vector( (0, 1, 0) ), limit)

    def GoingBack(self, normal, limit = 0.5 ):
        return self.NormalInDirection( normal, Vector( (0, -1, 0) ), limit)

    def GoingSide(self, normal, limit = 0.5 ):
        return self.GoingUp( normal, limit ) == False and self.GoingDown( normal, limit ) == False

    def clearMeshSelections(self):
        self.setMode('EDIT')
        bpy.ops.mesh.select_all(action='DESELECT')
        self.setMode('OBJECT')

    def meshSelectAll(self):
        self.setMode('EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        self.setMode('OBJECT')

    def setMode(self, newmode):
        bpy.ops.object.mode_set(mode=newmode, toggle=False)

    def deselectAll(self):
        bpy.ops.object.select_all(action='DESELECT')
        
    def selectAll(self):
        bpy.ops.object.select_all(action='SELECT')


    def openBuildReport(self):
        self.txt = bpy.data.texts.get("BuildReport.txt")
        if not self.txt:
            self.txt = bpy.data.texts.new("BuildReport.txt")
        self.txt.clear()

    def deleteExistingBoard(self):
        self.deselectAll()
        for ob in bpy.data.objects:
            if ob.name == self.file_name:
                self.setSelect(ob, True)
                bpy.ops.object.delete()
        self.deselectAll()        

    def deleteOrphans(self):           
        self.deselectAll()
        for m in bpy.data.materials:
            if (m.users == 0):
                bpy.data.materials.remove(m)

        for m in bpy.data.meshes:
            if (m.users == 0):
                bpy.data.meshes.remove(m)

        for m in bpy.data.images:
            if (m.users == 0):
                bpy.data.images.remove(m)

    def vabs(self, v):
        return (abs(v[0]), abs(v[1]), abs(v[2]))

    def version(self):
        return bpy.app.version[0] + (bpy.app.version[1]/100)    

    def setSelect(self, object, state):
        if self.version() < 2.80:
            object.select = state
        else:
            object.select_set(state)
            
    def getSelect(self, object):
        if self.version() < 2.80:
            return object.select
        else:
            return object.select_get()

    def setActiveObject(self, object):
        if self.version() < 2.80:
            bpy.context.scene.objects.active = object
        else:
            bpy.context.view_layer.objects.active = object
            
    def linkObject(self, object):
        if self.version() < 2.80:
            bpy.context.scene.objects.link(object)
        else:
            bpy.context.scene.collection.objects.link(object)
    
    def matchingVertices(self, f1, f2):
        count = 0
        for v1 in f1.vertices:
            for v2 in f2.vertices:
                if v1 == v2:
                    count += 1
        return count
        
    def selectOuterFaces(self, object):
        faces = object.data.polygons

        bpy.ops.mesh.select_all(action="DESELECT")
            
        sideFaces = [f for f in faces if self.GoingSide(f.normal)]

        print(sideFaces)

        maxObject = None
        maxArea = 0

        linkedFaces = []
        for f in sideFaces:
            if (len(f.vertices) == 3):
                if f.area > maxArea:
                    maxArea = f.area
                    maxObject = f
        
        print("Max object:")
        print(maxObject.index)
        
        self.selectFace(object, maxObject.index)

        bpy.ops.mesh.select_linked(delimit={'UV'})
        
            
            
    def selectFace(self, object, faceid):
        bm = bmesh.from_edit_mesh(object.data)
        bm.faces.ensure_lookup_table()
        bm.faces[faceid].select = True
        bmesh.update_edit_mesh(object.data)
        
    def importOutline(self):
        self.deselectAll()
        bpy.ops.wm.stl_import(filepath = self.file_outline)
        outline = [c for c in bpy.context.scene.objects if self.getSelect(c)]
        self.pcb = outline[0]
        self.pcb["auto_created"] = True
        self.pcb.data["auto_created"] = True
        self.deselectAll()
        self.setSelect(self.pcb, True)
        self.setActiveObject(self.pcb)
        bpy.context.object.data.name = self.file_name
        bpy.context.object.name = self.file_name
        bpy.ops.object.origin_set(type='ORIGIN_CENTER_OF_MASS')
        self.deselectAll()

    def loadMaterials(self):
        with bpy.data.libraries.load(self.component_root + "/Base/materials.blend", link=False) as (data_from, data_to):
            data_to.materials = ["Metal", "PCB Texture", "PCB Substrate"]

        top = None
        try:
            top = bpy.data.materials['PCB Top']
        except:
            top = bpy.data.materials['PCB Texture'].copy();
            top.name = "PCB Top"

        btm = None
        try:
            btm = bpy.data.materials['PCB Bottom']
        except:
            btm = bpy.data.materials['PCB Texture'].copy();
            btm.name = "PCB Bottom"

            
        top.node_tree.nodes['copper'].image = bpy.data.images.load(filepath = self.file_root + "/Render/" + self.file_name + "-F_Cu.png")
        top.node_tree.nodes['soldermask'].image = bpy.data.images.load(filepath = self.file_root + "/Render/" + self.file_name + "-F_Mask.png")
        top.node_tree.nodes['silk'].image = bpy.data.images.load(filepath = self.file_root + "/Render/" + self.file_name + "-F_Silkscreen.png")
        top.node_tree.nodes['pcbtexture'].inputs['Color'].default_value = self.color
        top.node_tree.nodes['pcbtexture'].inputs['Finish'].default_value = self.finish
        top.node_tree.nodes['pcbtexture'].inputs['Silk Color'].default_value = self.silk
        top.node_tree.nodes['mapping'].inputs['Scale'].default_value = (1.000, 1.000, 1)
        top.node_tree.nodes['mapping'].inputs['Location'].default_value = (0, 0.00, 0)
        
        btm.node_tree.nodes['copper'].image = bpy.data.images.load(filepath = self.file_root + "/Render/" + self.file_name + "-B_Cu.png")
        btm.node_tree.nodes['soldermask'].image = bpy.data.images.load(filepath = self.file_root + "/Render/" + self.file_name + "-B_Mask.png")
        btm.node_tree.nodes['silk'].image = bpy.data.images.load(filepath = self.file_root + "/Render/" + self.file_name + "-B_Silkscreen.png")
        btm.node_tree.nodes['pcbtexture'].inputs['Color'].default_value = self.color
        btm.node_tree.nodes['pcbtexture'].inputs['Finish'].default_value = self.finish
        btm.node_tree.nodes['pcbtexture'].inputs['Silk Color'].default_value = self.silk
        btm.node_tree.nodes['mapping'].inputs['Scale'].default_value = (1.000, 1.000, 1)
        btm.node_tree.nodes['mapping'].inputs['Location'].default_value = (0, 0, 0)
                    
        self.pcb.data.materials.append(bpy.data.materials['Metal'])
        self.pcb.data.materials.append(top)
        self.pcb.data.materials.append(btm)
        self.pcb.data.materials.append(bpy.data.materials['PCB Substrate'])
        

    def facesTouching(self, one, two):
        return False
        
    def faceArea(self, f):
        return f.area

    def assignMaterials(self):
        # Metal
        self.clearMeshSelections()
        self.setMode('OBJECT')
        for face in self.pcb.data.polygons:
            face.select = self.GoingSide(face.normal)        
        self.pcb.active_material_index = 0
        self.setMode('EDIT')
        bpy.ops.object.material_slot_assign()
        self.setMode('OBJECT')

        ## PCBTop
        self.clearMeshSelections()
        for face in self.pcb.data.polygons:
            face.select = self.GoingUp(face.normal)        
        self.pcb.active_material_index = 1
        self.setMode('EDIT')
        bpy.ops.object.material_slot_assign()
        bpy.ops.uv.cube_project(scale_to_bounds=True)
        self.setMode('OBJECT')

        # PCBBottom
        self.clearMeshSelections()
        for face in self.pcb.data.polygons:
            face.select = self.GoingDown(face.normal)        
        self.pcb.active_material_index = 2
        self.setMode('EDIT')
        bpy.ops.object.material_slot_assign()
        bpy.ops.uv.cube_project(scale_to_bounds=True)
        self.setMode('OBJECT')

        # PCB Substrate

        # This one is a little harder. First
        # select all the side faces.
        # Second work out which is the biggest - 
        # that's guaranteed to be an outside face.
        # Then repeatedly look through all the side
        # faces finding any that touch that first one.
        # Add them to an array, then repeat looking
        # for any faces that touch any in the array.
        
        self.setMode('EDIT')
        self.selectOuterFaces(self.pcb)
        self.pcb.active_material_index = 3
        bpy.ops.object.material_slot_assign()
        self.setMode('OBJECT')

    def populate(self):
        self.setMode('OBJECT')
        with open(self.file_csv, newline='', encoding='ISO-8859-15') as fobj:
            reader = csv.reader(filter(lambda row: row[0] == '"', fobj))
            layout_table = list(reader)

        self.objects = []
        missing = []

        for refdes, value, fp, x, y, rot, side in layout_table:

            refdes = refdes.replace('"', '')
            value = value.replace('"', '')
            component = value + "/" + fp.replace('"', '')
            x = float(x.replace('"', ''))
            y = float(y.replace('"', ''))
            rot = float(rot.replace('"', ''))

            if refdes == "(unknown)":
                continue

            try:
                if self.dnp[refdes] == 1:
                    continue
            except:
                pass

            z = 0
            yrot = 0
            if side == "bottom":
                z = -1.5;
                yrot = 180 / 57.2957795
            else:
                z = 1.5;
            loc = tuple(float(val) for val in (x, y, z))
            frot = float(rot)
            try:
                if self.rotations[refdes]:
                    frot = self.rotations[refdes]
                    self.txt.write("Rotation " +refdes + " : " + frot + "\n")
            except:
                pass

            frot = frot / 57.2957795
            zrot = tuple(float(val) for val in (0, yrot, frot))
            
            found = os.path.isfile(self.component_root + "/" + component + ".blend")
            if (found and (component not in bpy.data.meshes.keys())):
                self.txt.write("Loading component " + component + "\n")
                with bpy.data.libraries.load(self.component_root + "/" + component + ".blend", link=False) as (data_from, data_to):
                    data_to.meshes = data_from.meshes
                    
                oname = refdes + " - " + component
                dupli = bpy.data.objects.new(oname, data_to.meshes[0])
                dupli["auto_created"] = True
                dupli.data["auto_created"] = True
                dupli["refdes"] = refdes
                dupli["value"] = value
                dupli["footprint"] = fp
                dupli.location = loc
                dupli.rotation_euler = zrot
                self.linkObject(dupli)
                self.objects.append(oname)
                    
                
            if found == False:
                if component not in missing:
                    missing.append(component)
                    
                    
        if len(missing) > 0:
            self.txt.write("\nMissing components:\n")
            for m in missing:
                self.txt.write("    " + m + ".blend\n")

    def createSolder(self):
        bpy.ops.mesh.primitive_plane_add(size=1)
        self.solder = bpy.context.object
        self.solder.dimensions = self.pcb.dimensions
        self.solder['auto_created'] = True
        self.solder.data['auto_created'] = True
        self.solder.name="Solder"
        self.solder.location.z = 0.5
        self.setMode("EDIT")
        bpy.ops.mesh.subdivide(number_cuts=100)
        bpy.ops.mesh.subdivide(number_cuts=14)
        self.setMode("OBJECT")
        
        with bpy.data.libraries.load(self.component_root + "/Base/materials.blend", link=False) as (data_from, data_to):
            data_to.materials = ["Solder"]
                
        sld = bpy.data.materials['Solder'].copy()
        sld.node_tree.nodes['Image Texture'].image = bpy.data.images.load(filepath = self.file_root + "/Render/" + self.file_name + "-F_Paste_bw.png")
        self.solder.data.materials.append(sld)

        bpy.ops.object.modifier_add(type="DISPLACE")
        self.solder_deform = self.solder.modifiers[0]
        self.solder_deform.direction = "Z"
        self.solder_deform.strength = 1
        
        bpy.data.textures.new("Solder", "IMAGE")
        tex = bpy.data.textures["Solder"]
        tex.image = bpy.data.images.load(filepath = self.file_root + "/Render/" + self.file_name + "-F_Paste.png")
        tex.use_color_ramp = True
        
        self.solder_deform.texture = tex
        self.solder_deform.texture_coords = "UV"
        
        for f in self.solder.data.polygons:
            f.use_smooth = True
        
        bpy.ops.object.modifier_apply(modifier=self.solder_deform.name)
        
        self.setMode('EDIT')
        bpy.ops.mesh.select_all(action='DESELECT')
        t = len(self.solder.data.polygons)
        
        found = False
        while found == False:
            r = random.randrange(t)
            poly = self.solder.data.polygons[r]
            found = True
            for v in poly.vertices:
                z = self.solder.data.vertices[v].co.z
                if (z != -0.5):
                    found = False
            if (found == True):
                self.selectFace(self.solder, poly.index)
                bpy.ops.mesh.faces_select_linked_flat()
                
        if (found == True):
            bpy.ops.mesh.delete(type="FACE")
            bpy.ops.mesh.select_all(action="SELECT")
            bpy.ops.mesh.dissolve_limited()
        
        self.setMode('OBJECT')

        
        
    def cleanup(self):
        self.deselectAll()

        if self.doJoin:            
            for ob in self.objects:
                self.setSelect(bpy.data.objects[ob], True)

            self.setSelect(bpy.data.objects[self.file_name], True)

            self.setActiveObject(bpy.data.objects[self.file_name])
            bpy.ops.object.join()
            self.setActiveObject(bpy.data.objects[self.file_name])
        else:
            for ob in self.objects:
                bpy.data.objects[ob].parent = self.pcb
                bpy.data.objects[ob].location[0] -= self.pcb.location[0]
                bpy.data.objects[ob].location[1] -= self.pcb.location[1]
                bpy.data.objects[ob].location[2] -= self.pcb.location[2]
                
        for ob in self.objects:
            self.setSelect(bpy.data.objects[ob], False)

        self.setSelect(self.pcb, True)
        self.setActiveObject(self.pcb)
        bpy.context.object.data.name = self.file_name
        bpy.context.object.name = self.file_name
        bpy.ops.object.origin_set(type='ORIGIN_CENTER_OF_MASS')
        self.pcb.location = [0, 0, 0]
        
            
        for mesh in bpy.data.meshes:
            mesh.make_local()
            
        for object in bpy.data.objects:
            object.make_local()
            
        for material in bpy.data.materials:
            material.make_local()
            
        for ng in bpy.data.node_groups:
            ng.make_local()
            
        bpy.ops.object.select_all(action="SELECT")
        bpy.ops.object.shade_auto_smooth()
        bpy.ops.object.select_all(action="DESELECT")
            
    def deleteExistingObjects(self):
        for x in bpy.data.objects:
            if (x.get("auto_created") is not None):
                bpy.data.objects.remove(x)
            
        for x in bpy.data.meshes:
            if (x.get("auto_created") is not None):
                bpy.data.meshes.remove(x)

        for x in bpy.data.materials:
            bpy.data.materials.remove(x)
                
        for x in bpy.data.node_groups:
            bpy.data.node_groups.remove(x)            

        bpy.data.orphans_purge()
        bpy.data.orphans_purge()
        bpy.data.orphans_purge()
        bpy.data.orphans_purge()
        bpy.data.orphans_purge()
        bpy.data.orphans_purge()
        bpy.data.orphans_purge()
            
    def createDefaultObjects(self):

        bpy.ops.object.camera_add(location=self.camera)
        cam = bpy.context.object
        cam["auto_created"] = True
        bpy.ops.object.empty_add(location=self.target)
        empty = bpy.context.object
        empty["auto_created"] = True

        con = cam.constraints.new("TRACK_TO")

        con.target = empty
        

#        sun = bpy.data.lights.new("Sun", 'SUN')
#        sun_obj = bpy.data.objects.new("Sun", sun)
#        self.linkObject(sun_obj)
#        sun_obj.rotation_euler = self.sun

    def run(self):
        self.openBuildReport()
        self.deleteExistingObjects()
        self.deleteOrphans()
        self.createDefaultObjects()
        self.importOutline()
        self.loadMaterials()
        self.assignMaterials()
        self.populate()
        self.deleteOrphans()
        self.cleanup()
        self.createSolder()

    def startTimer(self):
        self._timer = self.wm.event_timer_add(2, window=self.context.window)
        
    def stopTimer(self):
        self.wm.event_timer_remove(self._timer)

    def execute(self, context):
        self.run()
        return {'FINISHED'}

    def cancel(self, context):
        self.stopTimer()


def register():
    bpy.utils.register_class(PCBImport)
    bpy.ops.wm.modal_timer_operator()


def unregister():
    bpy.utils.unregister_class(PCBImport)


if __name__ == "__main__":
    register()


