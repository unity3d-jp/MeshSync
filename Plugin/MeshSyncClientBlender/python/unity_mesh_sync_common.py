import bpy
from bpy.app.handlers import persistent
import MeshSyncClientBlender as ms

msb_context = ms.Context()

def msb_apply_scene_settings(self = None, context = None):
    ctx = msb_context
    scene = bpy.context.scene
    ctx.server_address = scene.meshsync_server_address
    ctx.server_port = scene.meshsync_server_port
    ctx.scale_factor = scene.meshsync_scale_factor
    ctx.sync_meshes = scene.meshsync_sync_meshes
    ctx.sync_normals = scene.meshsync_sync_normals
    ctx.sync_uvs = scene.meshsync_sync_uvs
    ctx.sync_colors = scene.meshsync_sync_colors
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
    return None

def msb_on_scene_settings_updated(self = None, context = None):
    msb_apply_scene_settings()
    if bpy.context.scene.meshsync_auto_sync:
        msb_context.setup(bpy.context)
        msb_context.sendSceneAll(False)
    return None

def msb_on_animation_settings_updated(self = None, context = None):
    # nothing to do for now
    return None

def msb_initialize_properties():
    bpy.types.Scene.meshsync_server_address = bpy.props.StringProperty(default = "127.0.0.1", name = "Address", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_server_port = bpy.props.IntProperty(default = 8080, name = "Port", min = 0, max = 65535, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_scale_factor = bpy.props.FloatProperty(default = 1.0, name = "Scale Factor", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_meshes = bpy.props.BoolProperty(default = True, name = "Sync Meshes", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_normals = bpy.props.BoolProperty(default = True, name = "Normals", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_uvs = bpy.props.BoolProperty(default = True, name = "UVs", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_colors = bpy.props.BoolProperty(default = True, name = "Colors", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_make_double_sided = bpy.props.BoolProperty(default = False, name = "Double Sided", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_bake_modifiers = bpy.props.BoolProperty(default = False, name = "Bake Modifiers", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_convert_to_mesh = bpy.props.BoolProperty(default = True, name = "Convert To Mesh", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_bones = bpy.props.BoolProperty(default = True, name = "Sync Bones", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_blendshapes = bpy.props.BoolProperty(default = True, name = "Sync Blend Shapes", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_textures = bpy.props.BoolProperty(default = True, name = "Sync Textures", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_cameras = bpy.props.BoolProperty(default = True, name = "Sync Cameras", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_lights = bpy.props.BoolProperty(default = True, name = "Sync Lights", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_auto_sync = bpy.props.BoolProperty(default = False, name = "Auto Sync", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_animation_ts = bpy.props.FloatProperty(default = 1, name = "Time Scale", min = 0.01, update = msb_on_animation_settings_updated)
    bpy.types.Scene.meshsync_animation_fi = bpy.props.IntProperty(default = 10, name = "Frame Step", min = 1, update = msb_on_animation_settings_updated)


@persistent
def on_scene_load(context):
    msb_context.clear()

@persistent
def on_scene_update(context):
    msb_context.flushPendingList()
    if(bpy.context.scene.meshsync_auto_sync):
        msb_apply_scene_settings()
        msb_context.setup(bpy.context)
        msb_context.sendSceneUpdated()
