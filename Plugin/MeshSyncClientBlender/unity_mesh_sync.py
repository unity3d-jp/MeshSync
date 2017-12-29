from time import time
import bpy
from bpy.app.handlers import persistent
import MeshSync as ms

bl_info = {
    "name": "Unity Mesh Sync",
    "author": "Unity Technologies",
    "version": (2018, 1, 1),
    "blender": (2, 65, 4),
    "description": "Sync Meshes with Unity",
}


def msb_sync():
    print("msb_sync:", bpy.data.filepath)
    objects = bpy.data.objects


def msb_add_mesh(ctx, obj):
    pass


def MeshSync_InitProperties():
    bpy.types.Scene.meshsync_server_addr = bpy.props.StringProperty(name = "Server Address")
    bpy.types.Scene.meshsync_server_port = bpy.props.IntProperty(name = "Server Port")
    bpy.types.Scene.meshsync_scale_factor = bpy.props.FloatProperty(name = "Scale Factor")
    bpy.types.Scene.meshsync_sync_normals = bpy.props.BoolProperty(name = "Sync Normals")
    bpy.types.Scene.meshsync_sync_colors = bpy.props.BoolProperty(name = "Sync Vertex Colors")
    bpy.types.Scene.meshsync_sync_bones = bpy.props.BoolProperty(name = "Sync Bones")
    bpy.types.Scene.meshsync_sync_cameras = bpy.props.BoolProperty(name = "Sync Cameras")
    bpy.types.Scene.meshsync_camera_path = bpy.props.StringProperty(name = "Camera Path")
    bpy.types.Scene.meshsync_sync_animations = bpy.props.BoolProperty(name = "Sync Animations")
    bpy.types.Scene.meshsync_auto_sync = bpy.props.BoolProperty(name = "Auto Sync")
    bpy.types.Scene.meshsync_interval = bpy.props.FloatProperty(name = "Interval (Sec)")
    bpy.types.Scene.meshsync_last_sent = bpy.props.FloatProperty(name = "Last Sent")
    scene = bpy.context.scene
    scene['meshsync_server_addr'] = "localhost"
    scene['meshsync_server_port'] = 8080
    scene['meshsync_scale_factor'] = 100.0
    scene['meshsync_sync_normals'] = True
    scene['meshsync_sync_colors'] = False
    scene['meshsync_sync_bones'] = True
    scene['meshsync_sync_cameras'] = True
    scene['meshsync_camera_path'] = "/Main Camera"
    scene['meshsync_sync_animations'] = True
    scene['meshsync_auto_sync'] = False
    scene['meshsync_interval'] = 2.0
    scene['meshsync_last_sent'] = 0.0


class MeshSyncPanel(bpy.types.Panel):
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_category = "Mesh Sync"
    bl_label = "Mesh Sync"

    def draw(self, context):
        scene = bpy.context.scene
        self.layout.prop(context.scene, 'meshsync_server_addr')
        self.layout.prop(context.scene, 'meshsync_server_port')
        self.layout.separator()
        self.layout.prop(context.scene, 'meshsync_scale_factor')
        self.layout.prop(context.scene, 'meshsync_sync_normals')
        self.layout.prop(context.scene, 'meshsync_sync_colors')
        self.layout.prop(context.scene, 'meshsync_sync_bones')
        self.layout.prop(context.scene, 'meshsync_sync_cameras')
        if(scene['meshsync_sync_cameras']):
            self.layout.prop(context.scene, 'meshsync_camera_path')
        self.layout.prop(context.scene, 'meshsync_sync_animations')
        self.layout.prop(context.scene, 'meshsync_auto_sync')
        if(scene['meshsync_auto_sync']):
            self.layout.prop(context.scene, 'meshsync_interval')
        self.layout.separator()
        self.layout.operator("meshsync.sync", text="Manual Sync")


class MeshSync_OpSync(bpy.types.Operator):
    bl_idname = "meshsync.sync"
    bl_label = "Sync"

    def execute(self, context):
        msb_sync()
        return{'FINISHED'}



@persistent
def on_scene_update(dummy):
    scene = bpy.context.scene
    if(scene['meshsync_auto_sync']):
        if(time() - scene['meshsync_last_sent'] > scene['meshsync_interval']):
            msb_sync()
            scene['meshsync_last_sent'] = time()

def register():
    MeshSync_InitProperties()
    bpy.utils.register_module(__name__)
    bpy.app.handlers.scene_update_post.append(on_scene_update)
    bpy.app.handlers.frame_change_post.append(on_scene_update)
    bpy.app.handlers.save_post.append(on_scene_update)
    bpy.app.handlers.load_post.append(on_scene_update)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.app.handlers.scene_update_post.remove(on_scene_update)
    bpy.app.handlers.frame_change_post.remove(on_scene_update)
    bpy.app.handlers.save_post.remove(on_scene_update)
    bpy.app.handlers.load_post.remove(on_scene_update)

if __name__ == "__main__":
    register()
