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
    ctx.convert_to_mesh = scene.meshsync_convert_to_mesh
    ctx.sync_bones = scene.meshsync_sync_bones
    ctx.sync_blendshapes = scene.meshsync_sync_blendshapes
    ctx.sync_textures = scene.meshsync_sync_textures
    ctx.sync_cameras = scene.meshsync_sync_cameras
    ctx.sync_lights = scene.meshsync_sync_lights
    return None

def msb_apply_animation_settings(self = None, context = None):
    ctx = msb_context
    scene = bpy.context.scene
    ctx.animation_ts = scene.meshsync_animation_ts
    ctx.animation_interval = scene.meshsync_animation_fi
    ctx.keyframe_reduction = scene.meshsync_animation_kfr
    ctx.keep_flat_curves = scene.meshsync_animation_kfc
    return None

def msb_export_cache(path):
    ctx = msb_cache
    scene = bpy.context.scene
    ctx.object_scope = int(scene.mscache_object_scope)
    ctx.frame_range = int(scene.mscache_frame_range)
    ctx.frame_begin = scene.mscache_frame_begin
    ctx.frame_end = scene.mscache_frame_end
    ctx.material_frame_range = int(scene.mscache_material_frame_range)
    ctx.zstd_compression_level = scene.mscache_zstd_compression_level
    ctx.frame_step = scene.mscache_frame_step
    ctx.make_double_sided = scene.mscache_make_double_sided
    ctx.bake_modifiers = scene.mscache_bake_modifiers
    ctx.convert_to_mesh = scene.mscache_convert_to_mesh
    ctx.flatten_hierarchy = scene.mscache_flatten_hierarchy
    ctx.merge_meshes = scene.mscache_merge_meshes
    ctx.strip_normals = scene.mscache_strip_normals
    ctx.strip_tangents = scene.mscache_strip_tangents
    ctx.export(path)
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
    bpy.types.Scene.meshsync_server_address = bpy.props.StringProperty(default = "127.0.0.1", name = "Address", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_server_port = bpy.props.IntProperty(default = 8080, name = "Port", min = 0, max = 65535, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_scale_factor = bpy.props.FloatProperty(default = 1.0, name = "Scale Factor", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_meshes = bpy.props.BoolProperty(default = True, name = "Sync Meshes", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_make_double_sided = bpy.props.BoolProperty(default = False, name = "Make Double Sided", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_bake_modifiers = bpy.props.BoolProperty(default = False, name = "Bake Modifiers", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_convert_to_mesh = bpy.props.BoolProperty(default = True, name = "Convert To Mesh", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_bones = bpy.props.BoolProperty(default = True, name = "Sync Bones", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_blendshapes = bpy.props.BoolProperty(default = True, name = "Sync Blend Shapes", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_textures = bpy.props.BoolProperty(default = True, name = "Sync Textures", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_cameras = bpy.props.BoolProperty(default = True, name = "Sync Cameras", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_lights = bpy.props.BoolProperty(default = True, name = "Sync Lights", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_auto_sync = bpy.props.BoolProperty(default = False, name = "Auto Sync", update = msb_on_toggle_auto_sync)
    bpy.types.Scene.meshsync_animation_ts = bpy.props.FloatProperty(default = 1, name = "Time Scale", min = 0.01, update = msb_on_animation_settings_updated)
    bpy.types.Scene.meshsync_animation_fi = bpy.props.IntProperty(default = 10, name = "Frame Step", min = 1, update = msb_on_animation_settings_updated)
    bpy.types.Scene.meshsync_animation_kfr = bpy.props.BoolProperty(default = True, name = "Keyframe Reduction", update = msb_on_animation_settings_updated)
    bpy.types.Scene.meshsync_animation_kfc = bpy.props.BoolProperty(default = False, name = "Keep Flat Curves", update = msb_on_animation_settings_updated)

    # cache settings
    bpy.types.Scene.mscache_object_scope = bpy.props.EnumProperty(
                    name = 'Object Scope',
                    default = '0',
                    items = {
                        ('0', 'All', ''),
                        ('1', 'Selected', ''),
                    })
    bpy.types.Scene.mscache_frame_range = bpy.props.EnumProperty(
                    name = 'Frame Range',
                    default = '0',
                    items = {
                        ('0', 'Current', ''),
                        ('1', 'All', ''),
                        ('2', 'Custom', ''),
                    })
    bpy.types.Scene.mscache_frame_begin = bpy.props.IntProperty(default = 1, name = "Frame Begin")
    bpy.types.Scene.mscache_frame_end = bpy.props.IntProperty(default = 100, name = "Frame End")
    bpy.types.Scene.mscache_material_frame_range = bpy.props.EnumProperty(
                    name = 'Material Range',
                    default = '0',
                    items = {
                        ('0', 'None', ''),
                        ('1', 'One', ''),
                        ('2', 'All', ''),
                    })
    bpy.types.Scene.mscache_zstd_compression_level = bpy.props.IntProperty(default = 3, name = "ZSTD Compression")
    bpy.types.Scene.mscache_frame_step = bpy.props.IntProperty(default = 1, name = "Frame Step")
    bpy.types.Scene.mscache_make_double_sided = bpy.props.BoolProperty(default = False, name = "Make Double Sided")
    bpy.types.Scene.mscache_bake_modifiers = bpy.props.BoolProperty(default = True, name = "Bake Modifiers")
    bpy.types.Scene.mscache_convert_to_mesh = bpy.props.BoolProperty(default = True, name = "Convert To Mesh")
    bpy.types.Scene.mscache_flatten_hierarchy = bpy.props.BoolProperty(default = False, name = "Flatten Hierarchy")
    bpy.types.Scene.mscache_merge_meshes = bpy.props.BoolProperty(default = False, name = "Merge Meshes")
    bpy.types.Scene.mscache_strip_normals = bpy.props.BoolProperty(default = False, name = "Strip Normals")
    bpy.types.Scene.mscache_strip_tangents = bpy.props.BoolProperty(default = False, name = "Strip Tangents")


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


class MESHSYNC_OT_ExportObjects(bpy.types.Operator):
    bl_idname = "meshsync.export_objects"
    bl_label = "Export Objects"
    def execute(self, context):
        msb_apply_scene_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_OBJECTS)
        return{'FINISHED'}


class MESHSYNC_OT_ExportMaterials(bpy.types.Operator):
    bl_idname = "meshsync.export_materials"
    bl_label = "Export Materials"
    def execute(self, context):
        msb_apply_scene_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_MATERIALS)
        return{'FINISHED'}


class MESHSYNC_OT_ExportAnimations(bpy.types.Operator):
    bl_idname = "meshsync.export_animations"
    bl_label = "Export Animations"
    def execute(self, context):
        msb_apply_animation_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_ANIMATIONS)
        return{'FINISHED'}


class MESHSYNC_OT_ExportEverything(bpy.types.Operator):
    bl_idname = "meshsync.export_everything"
    bl_label = "Export Everything"
    def execute(self, context):
        msb_apply_scene_settings()
        msb_apply_animation_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_EVERYTHING)
        return{'FINISHED'}


class MESHSYNC_OT_ExportCache(bpy.types.Operator):
    bl_idname = "meshsync.export_cache"
    bl_label = "Export Cache"
    bl_description = "Export Cache"

    filepath = bpy.props.StringProperty(subtype = "FILE_PATH")
    filename = bpy.props.StringProperty()
    directory = bpy.props.StringProperty(subtype = "FILE_PATH")

    def execute(self, context):
        msb_context.setup(bpy.context);
        msb_export_cache(self.filepath)
        return {'FINISHED'}

    def invoke(self, context, event):
        self.filename = "foo.sc";
        wm = bpy.context.window_manager
        wm.fileselect_add(self)
        return {'RUNNING_MODAL'}
