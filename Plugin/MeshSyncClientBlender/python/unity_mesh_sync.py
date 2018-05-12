import bpy
from bpy.app.handlers import persistent
import MeshSyncClientBlender as ms

bl_info = {
    "name": "Unity Mesh Sync",
    "author": "Unity Technologies",
    "version": (2018, 5, 14),
    "blender": (2, 79),
    "description": "Sync Meshes with Unity",
    "location": "View3D > Mesh Sync",
    "tracker_url": "https://github.com/unity3d-jp/MeshSync",
    "support": "COMMUNITY",
    "category": "Import-Export",
}


msb_context = ms.Context()

def msb_apply_settings(self = None, context = None):
    ctx = msb_context
    scene = bpy.context.scene
    ctx.server_address = scene.meshsync_server_address
    ctx.server_port = scene.meshsync_server_port
    ctx.scale_factor = scene.meshsync_scale_factor
    ctx.sync_meshes = scene.meshsync_sync_meshes
    ctx.sync_normals = scene.meshsync_sync_normals
    ctx.sync_uvs = scene.meshsync_sync_uvs
    ctx.sync_colors = scene.meshsync_sync_colors
    ctx.sync_bones = scene.meshsync_sync_bones
    ctx.sync_blendshapes = scene.meshsync_sync_blendshapes
    ctx.sync_cameras = scene.meshsync_sync_cameras
    ctx.sync_lights = scene.meshsync_sync_lights
    ctx.animation_ts = scene.meshsync_animation_ts
    ctx.animation_interval = scene.meshsync_animation_fi
    return None


def msb_initialize_properties():
    bpy.types.Scene.meshsync_server_address = bpy.props.StringProperty(default = "127.0.0.1", name = "Server Address")
    bpy.types.Scene.meshsync_server_port = bpy.props.IntProperty(default = 8080, name = "Server Port")
    bpy.types.Scene.meshsync_scale_factor = bpy.props.FloatProperty(default = 1.0, name = "Scale Factor")
    bpy.types.Scene.meshsync_sync_meshes = bpy.props.BoolProperty(default = True, name = "Sync Meshes")
    bpy.types.Scene.meshsync_sync_normals = bpy.props.BoolProperty(default = True, name = "Normals")
    bpy.types.Scene.meshsync_sync_uvs = bpy.props.BoolProperty(default = True, name = "UVs")
    bpy.types.Scene.meshsync_sync_colors = bpy.props.BoolProperty(default = False, name = "Colors")
    bpy.types.Scene.meshsync_sync_bones = bpy.props.BoolProperty(default = True, name = "Bones")
    bpy.types.Scene.meshsync_sync_blendshapes = bpy.props.BoolProperty(default = True, name = "Blend Shapes")
    bpy.types.Scene.meshsync_apply_modifiers = bpy.props.BoolProperty(default = False, name = "Apply Modifiers")
    bpy.types.Scene.meshsync_sync_cameras = bpy.props.BoolProperty(default = True, name = "Sync Cameras")
    bpy.types.Scene.meshsync_sync_lights = bpy.props.BoolProperty(default = True, name = "Sync Lights")
    bpy.types.Scene.meshsync_auto_sync = bpy.props.BoolProperty(default = False, name = "Auto Sync")
    bpy.types.Scene.meshsync_animation_ts = bpy.props.FloatProperty(default = 1, name = "Time Scale")
    bpy.types.Scene.meshsync_animation_fi = bpy.props.IntProperty(default = 10, name = "Frame Interval")


class MeshSyncPanel(bpy.types.Panel):
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_category = "Mesh Sync"
    bl_label = "Mesh Sync"

    def draw(self, context):
        scene = bpy.context.scene
        self.layout.prop(context.scene, 'meshsync_server_address')
        self.layout.prop(context.scene, 'meshsync_server_port')
        self.layout.separator()

        self.layout.label("Scene")
        self.layout.prop(context.scene, 'meshsync_scale_factor')
        self.layout.prop(context.scene, 'meshsync_sync_meshes')
        if scene.meshsync_sync_meshes:
            b = self.layout.box()
            b.prop(context.scene, 'meshsync_sync_normals')
            b.prop(context.scene, 'meshsync_sync_uvs')
            b.prop(context.scene, 'meshsync_sync_colors')
            b.prop(context.scene, 'meshsync_sync_bones')
            b.prop(context.scene, 'meshsync_sync_blendshapes')
            b.prop(context.scene, 'meshsync_apply_modifiers')
        self.layout.prop(context.scene, 'meshsync_sync_cameras')
        self.layout.prop(context.scene, 'meshsync_sync_lights')
        self.layout.separator()
        self.layout.prop(context.scene, 'meshsync_auto_sync')
        self.layout.operator("meshsync.sync_scene", text="Manual Sync")
        self.layout.separator()

        self.layout.label("Animation")
        self.layout.prop(context.scene, 'meshsync_animation_ts')
        self.layout.prop(context.scene, 'meshsync_animation_fi')
        self.layout.operator("meshsync.sync_animations", text="Sync")


class MeshSync_OpSyncScene(bpy.types.Operator):
    bl_idname = "meshsync.sync_scene"
    bl_label = "Sync Scene"
    def execute(self, context):
        msb_apply_settings()
        msb_context.sendSceneAll()
        return{'FINISHED'}
    

class MeshSync_OpSyncAnimations(bpy.types.Operator):
    bl_idname = "meshsync.sync_animations"
    bl_label = "Sync Animations"
    def execute(self, context):
        msb_apply_settings()
        msb_context.sendAnimationsAll()
        return{'FINISHED'}


@persistent
def on_scene_update(context):
    msb_context.flushPendingList();
    if(bpy.context.scene.meshsync_auto_sync):
        msb_apply_settings()
        msb_context.sendSceneUpdated()

def register():
    msb_initialize_properties()
    bpy.utils.register_module(__name__)
    bpy.app.handlers.scene_update_post.append(on_scene_update)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.app.handlers.scene_update_post.remove(on_scene_update)

if __name__ == "__main__":
    register()
