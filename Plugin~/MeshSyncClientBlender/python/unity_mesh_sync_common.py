import os
import re
import bpy
from bpy.app.handlers import persistent
import MeshSyncClientBlender as ms

msb_context = ms.Context()
msb_cache = ms.Cache()

def msb_apply_scene_settings(self = None, context = None):
    ctx = msb_context
    scene = bpy.context.scene
    ctx.server_address = scene.meshsync_server_address
    ctx.server_port = scene.meshsync_server_port
    ctx.scale_factor = scene.meshsync_scale_factor
    ctx.sync_meshes = scene.meshsync_sync_meshes
    ctx.make_double_sided = scene.meshsync_make_double_sided
    ctx.bake_modifiers = scene.meshsync_bake_modifiers
    ctx.bake_transform = scene.meshsync_bake_transform
    ctx.curves_as_mesh = scene.meshsync_curves_as_mesh
    ctx.sync_bones = scene.meshsync_sync_bones
    ctx.sync_blendshapes = scene.meshsync_sync_blendshapes
    ctx.sync_textures = scene.meshsync_sync_textures
    ctx.sync_cameras = scene.meshsync_sync_cameras
    ctx.sync_lights = scene.meshsync_sync_lights
    return None

def msb_apply_animation_settings(self = None, context = None):
    ctx = msb_context
    scene = bpy.context.scene
    ctx.frame_step = scene.meshsync_frame_step
    return None


def msb_on_scene_settings_updated(self = None, context = None):
    msb_apply_scene_settings()
    if bpy.context.scene.meshsync_auto_sync:
        msb_context.setup(bpy.context)
        msb_context.export(msb_context.TARGET_OBJECTS)
    return None

def msb_on_toggle_auto_sync(self = None, context = None):
    msb_apply_scene_settings()
    if bpy.context.scene.meshsync_auto_sync:
        if not msb_context.is_server_available:
            print("MeshSync: " + msb_context.error_message)
            bpy.context.scene.meshsync_auto_sync = False
    if bpy.context.scene.meshsync_auto_sync:
        msb_context.setup(bpy.context)
        msb_context.export(msb_context.TARGET_OBJECTS)
    return None

def msb_on_animation_settings_updated(self = None, context = None):
    # nothing to do for now
    return None

def msb_initialize_properties():
    # sync settings
    bpy.types.Scene.meshsync_server_address = bpy.props.StringProperty(name = "Address", default = "127.0.0.1", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_server_port = bpy.props.IntProperty(name = "Port", default = 8080, min = 0, max = 65535, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_scale_factor = bpy.props.FloatProperty(name = "Scale Factor", default = 1.0, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_meshes = bpy.props.BoolProperty(name = "Sync Meshes", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_make_double_sided = bpy.props.BoolProperty(name = "Make Double Sided", default = False, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_bake_modifiers = bpy.props.BoolProperty(name = "Bake Modifiers", default = False, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_bake_transform  = bpy.props.BoolProperty(name = "Bake Transform", default = False, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_curves_as_mesh = bpy.props.BoolProperty(name = "Curves as Mesh", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_bones = bpy.props.BoolProperty(name = "Sync Bones", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_blendshapes = bpy.props.BoolProperty(name = "Sync Blend Shapes", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_textures = bpy.props.BoolProperty(name = "Sync Textures", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_cameras = bpy.props.BoolProperty(name = "Sync Cameras", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_lights = bpy.props.BoolProperty(name = "Sync Lights", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_auto_sync = bpy.props.BoolProperty(name = "Auto Sync", default = False, update = msb_on_toggle_auto_sync)
    bpy.types.Scene.meshsync_frame_step = bpy.props.IntProperty(name = "Frame Step", default = 1, min = 1, update = msb_on_animation_settings_updated)


@persistent
def on_scene_load(context):
    msb_context.clear()

@persistent
def on_scene_update(context):
    msb_context.flushPendingList()
    if(bpy.context.scene.meshsync_auto_sync):
        msb_apply_scene_settings()
        msb_context.setup(bpy.context)
        msb_context.exportUpdatedObjects()


class MESHSYNC_OT_SendObjects(bpy.types.Operator):
    bl_idname = "meshsync.send_objects"
    bl_label = "Export Objects"
    def execute(self, context):
        msb_apply_scene_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_OBJECTS)
        return{'FINISHED'}


class MESHSYNC_OT_SendAnimations(bpy.types.Operator):
    bl_idname = "meshsync.send_animations"
    bl_label = "Export Animations"
    def execute(self, context):
        msb_apply_animation_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_ANIMATIONS)
        return{'FINISHED'}


class MESHSYNC_OT_ExportCache(bpy.types.Operator):
    bl_idname = "meshsync.export_cache"
    bl_label = "Export Cache"
    bl_description = "Export Cache"

    filepath = bpy.props.StringProperty(subtype = "FILE_PATH")
    filename = bpy.props.StringProperty()
    directory = bpy.props.StringProperty(subtype = "FILE_PATH")

    # cache properties
    object_scope = bpy.props.EnumProperty(
        name = "Object Scope",
        default = "0",
        items = {
            ("0", "All", "Export all objects"),
            ("1", "Selected", "Export selected objects"),
        })
    frame_range = bpy.props.EnumProperty(
        name = "Frame Range",
        default = "1",
        items = {
            ("0", "Current", "Export current frame"),
            ("1", "All", "Export all frames"),
            ("2", "Custom", "Export speficied frames"),
        })
    frame_begin = bpy.props.IntProperty(name = "Frame Begin", default = 1)
    frame_end = bpy.props.IntProperty(name = "Frame End", default = 100)
    frame_step = bpy.props.IntProperty(name = "Frame Step", default = 1, min = 1)
    material_frame_range = bpy.props.EnumProperty(
        name = "Material Range",
        default = "1",
        items = {
            ("0", "None", "Export no materials"),
            ("1", "One", "Export one frame of materials"),
            ("2", "All", "Export all frames of materials"),
        })
    zstd_compression_level = bpy.props.IntProperty(name = "ZSTD Compression", default = 3)
    make_double_sided = bpy.props.BoolProperty(name = "Make Double Sided", default = False)
    bake_modifiers = bpy.props.BoolProperty(name = "Bake Modifiers", default = True)
    bake_transform = bpy.props.BoolProperty(name = "Bake Transform", default = False)
    convert_to_mesh = bpy.props.BoolProperty(name = "Convert To Mesh", default = True)
    flatten_hierarchy = bpy.props.BoolProperty(name = "Flatten Hierarchy", default = False)
    merge_meshes = bpy.props.BoolProperty(name = "Merge Meshes", default = False)
    strip_normals = bpy.props.BoolProperty(name = "Strip Normals", default = False)
    strip_tangents = bpy.props.BoolProperty(name = "Strip Tangents", default = False)

    def execute(self, context):
        ctx = msb_cache
        ctx.object_scope = int(self.object_scope)
        ctx.frame_range = int(self.frame_range)
        ctx.frame_begin = self.frame_begin
        ctx.frame_end = self.frame_end
        ctx.material_frame_range = int(self.material_frame_range)
        ctx.zstd_compression_level = self.zstd_compression_level
        ctx.frame_step = self.frame_step
        ctx.make_double_sided = self.make_double_sided
        ctx.bake_modifiers = self.bake_modifiers
        ctx.bake_transform = self.bake_transform
        ctx.convert_to_mesh = self.convert_to_mesh
        ctx.flatten_hierarchy = self.flatten_hierarchy
        ctx.merge_meshes = self.merge_meshes
        ctx.strip_normals = self.strip_normals
        ctx.strip_tangents = self.strip_tangents
        ctx.export(self.filepath)
        return {'FINISHED'}

    def invoke(self, context, event):
        msb_context.setup(bpy.context)
        ctx = msb_cache
        self.object_scope = str(ctx.object_scope);
        self.frame_range = str(ctx.frame_range);
        self.frame_begin = ctx.frame_begin;
        self.frame_end = ctx.frame_end;
        self.material_frame_range = str(ctx.material_frame_range);
        self.frame_end = ctx.frame_end;
        self.zstd_compression_level = ctx.zstd_compression_level;
        self.frame_step = ctx.frame_step;
        self.make_double_sided = ctx.make_double_sided;
        self.bake_modifiers = ctx.bake_modifiers;
        self.bake_transform = ctx.bake_transform;
        self.convert_to_mesh = ctx.convert_to_mesh;
        self.flatten_hierarchy = ctx.flatten_hierarchy;
        self.merge_meshes = ctx.merge_meshes;
        self.strip_normals = ctx.strip_normals;
        self.strip_tangents = ctx.strip_tangents;

        path = bpy.data.filepath
        if len(path) != 0:
            tmp = os.path.split(path)
            self.directory = tmp[0]
            self.filename = re.sub(r"\.[^.]+$", ".sc", tmp[1])
        else:
            self.directory = ""
            self.filename = "Untitled.sc";
        wm = bpy.context.window_manager
        wm.fileselect_add(self)
        return {'RUNNING_MODAL'}

    def draw(self, context):
        layout = self.layout
        if hasattr(layout, "use_property_split"): # false on 2.79
            layout.use_property_split = True
        layout.prop(self, "object_scope")
        layout.prop(self, "frame_range")
        if self.frame_range == "2":
            b = layout.box()
            b.prop(self, "frame_begin")
            b.prop(self, "frame_end")
        layout.prop(self, "material_frame_range")
        layout.prop(self, "zstd_compression_level")
        layout.prop(self, "frame_step")
        layout.prop(self, "make_double_sided")
        layout.prop(self, "bake_modifiers")
        layout.prop(self, "bake_transform")
        layout.prop(self, "convert_to_mesh")
        layout.prop(self, "flatten_hierarchy")
        layout.prop(self, "merge_meshes")
        layout.prop(self, "strip_normals")
        layout.prop(self, "strip_tangents")
