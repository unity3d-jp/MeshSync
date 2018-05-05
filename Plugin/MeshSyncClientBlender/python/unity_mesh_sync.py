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
    ctx.sync_normals = 2 if scene.meshsync_sync_normals else 0
    ctx.sync_uvs = scene.meshsync_sync_uvs
    ctx.sync_colors = scene.meshsync_sync_colors
    ctx.sync_bones = scene.meshsync_sync_bones
    ctx.sync_poses = scene.meshsync_sync_poses
    ctx.sync_blendshapes = scene.meshsync_sync_blendshapes
    ctx.sync_cameras = scene.meshsync_sync_cameras
    ctx.sync_lights = scene.meshsync_sync_lights
    ctx.sync_animations = scene.meshsync_sync_animations
    ctx.sample_animation = scene.meshsync_sample_animation
    ctx.animation_sps = scene.meshsync_animation_sps
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
    bpy.types.Scene.meshsync_sync_poses = bpy.props.BoolProperty(default = True, name = "Poses")
    bpy.types.Scene.meshsync_sync_blendshapes = bpy.props.BoolProperty(default = True, name = "Blend Shapes")
    bpy.types.Scene.meshsync_apply_modifiers = bpy.props.BoolProperty(default = False, name = "Apply Modifiers")
    bpy.types.Scene.meshsync_sync_cameras = bpy.props.BoolProperty(default = True, name = "Sync Cameras")
    bpy.types.Scene.meshsync_sync_lights = bpy.props.BoolProperty(default = True, name = "Sync Lights")
    bpy.types.Scene.meshsync_sync_animations = bpy.props.BoolProperty(default = False, name = "Sync Animations")
    bpy.types.Scene.meshsync_sample_animation = bpy.props.BoolProperty(default = True, name = "Sample Animations")
    bpy.types.Scene.meshsync_animation_sps = bpy.props.IntProperty(default = 5, name = "Samples")
    bpy.types.Scene.meshsync_auto_sync = bpy.props.BoolProperty(default = False, name = "Auto Sync")


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
        self.layout.prop(context.scene, 'meshsync_scale_factor')
        self.layout.prop(context.scene, 'meshsync_sync_meshes')
        if scene.meshsync_sync_meshes:
            b = self.layout.box()
            b.prop(context.scene, 'meshsync_sync_normals')
            b.prop(context.scene, 'meshsync_sync_uvs')
            b.prop(context.scene, 'meshsync_sync_colors')
            b.prop(context.scene, 'meshsync_sync_bones')
            if scene.meshsync_sync_bones:
                b2 = b.box()
                b2.prop(context.scene, 'meshsync_sync_poses')
            b.prop(context.scene, 'meshsync_sync_blendshapes')
            b.prop(context.scene, 'meshsync_apply_modifiers')
        self.layout.prop(context.scene, 'meshsync_sync_cameras')
        self.layout.prop(context.scene, 'meshsync_sync_lights')
        self.layout.prop(context.scene, 'meshsync_sync_animations')
        if scene.meshsync_sync_animations:
            b = self.layout.box()
            b.prop(context.scene, 'meshsync_sample_animation')
            if scene.meshsync_sample_animation:
                b.prop(context.scene, 'meshsync_animation_sps')
        self.layout.separator()
        self.layout.prop(context.scene, 'meshsync_auto_sync')
        self.layout.operator("meshsync.sync_all", text="Manual Sync")


class MeshSync_OpSyncAll(bpy.types.Operator):
    bl_idname = "meshsync.sync_all"
    bl_label = "Sync All"
    def execute(self, context):
        msb_apply_settings()
        msb_context.setup()
        msb_context.syncAll()
        return{'FINISHED'}


@persistent
def on_scene_update(context):
    msb_context.setup()
    msb_context.flushPendingList();
    if(bpy.context.scene.meshsync_auto_sync):
        msb_apply_settings()
        msb_context.syncUpdated()

def register():
    msb_initialize_properties()
    bpy.utils.register_module(__name__)
    bpy.app.handlers.scene_update_post.append(on_scene_update)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.app.handlers.scene_update_post.remove(on_scene_update)

if __name__ == "__main__":
    register()
