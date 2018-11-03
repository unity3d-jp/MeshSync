bl_info = {
    "name": "Unity Mesh Sync",
    "author": "Unity Technologies",
    "version": (2018, 9, 18),
    "blender": (2, 80, 0),
    "description": "Sync Meshes with Unity",
    "location": "View3D > Mesh Sync",
    "tracker_url": "https://github.com/unity3d-jp/MeshSync",
    "support": "COMMUNITY",
    "category": "Import-Export",
}

import bpy
from bpy.app.handlers import persistent
import MeshSyncClientBlender as ms
from unity_mesh_sync_common import *


class MESHSYNC_PT_Server(bpy.types.Panel):
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_label = "Server"

    def draw(self, context):
        scene = bpy.context.scene
        self.layout.prop(scene, 'meshsync_server_address')
        self.layout.prop(scene, 'meshsync_server_port')


class MESHSYNC_PT_Scene(bpy.types.Panel):
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_label = "Scene"

    def draw(self, context):
        scene = bpy.context.scene
        self.layout.prop(scene, 'meshsync_scale_factor')
        self.layout.prop(scene, 'meshsync_sync_meshes')
        if scene.meshsync_sync_meshes:
            b = self.layout.box()
            b.prop(scene, 'meshsync_sync_normals')
            b.prop(scene, 'meshsync_sync_uvs')
            b.prop(scene, 'meshsync_sync_colors')
            b.prop(scene, 'meshsync_bake_modifiers')
        self.layout.prop(scene, 'meshsync_sync_bones')
        self.layout.prop(scene, 'meshsync_sync_blendshapes')
        self.layout.prop(scene, 'meshsync_sync_textures')
        self.layout.prop(scene, 'meshsync_sync_cameras')
        self.layout.prop(scene, 'meshsync_sync_lights')
        self.layout.separator()
        if MESHSYNC_OT_AutoSync._timer:
            self.layout.operator("meshsync.auto_sync", "Auto Sync", icon="PAUSE")
        else:
            self.layout.operator("meshsync.auto_sync", "Auto Sync", icon="PLAY")
        self.layout.operator("meshsync.sync_scene", text="Manual Sync")
        self.layout.separator()

class MESHSYNC_PT_Animation(bpy.types.Panel):
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_label = "Animation"

    def draw(self, context):
        scene = bpy.context.scene
        self.layout.prop(scene, 'meshsync_animation_ts')
        self.layout.prop(scene, 'meshsync_animation_fi')
        self.layout.operator("meshsync.sync_animations", text="Sync")


class MESHSYNC_OT_SyncScene(bpy.types.Operator):
    bl_idname = "meshsync.sync_scene"
    bl_label = "Sync Scene"
    def execute(self, context):
        msb_apply_scene_settings()
        msb_context.setup(bpy.context);
        msb_context.sendSceneAll(True)
        return{'FINISHED'}


class MESHSYNC_OT_SyncAnimations(bpy.types.Operator):
    bl_idname = "meshsync.sync_animations"
    bl_label = "Sync Animations"
    def execute(self, context):
        msb_apply_animation_settings()
        msb_context.setup(bpy.context);
        msb_context.sendAnimationsAll()
        return{'FINISHED'}


class MESHSYNC_OT_AutoSync(bpy.types.Operator):
    bl_idname = "meshsync.auto_sync"
    bl_label = "Auto Sync"
    _timer = None
    
    def invoke(self, context, event):
        print('invoke')
        if context.area.type != "VIEW_3D":
            return
        if not MESHSYNC_OT_AutoSync._timer:
            MESHSYNC_OT_AutoSync._timer = context.window_manager.event_timer_add(1.0 / 3.0, context.window)
            context.window_manager.modal_handler_add(self)
            return {'RUNNING_MODAL'}
        else:
            context.window_manager.event_timer_remove(MESHSYNC_OT_AutoSync._timer)
            MESHSYNC_OT_AutoSync._timer = None
            return {'FINISHED'}

    def modal(self, context, event):
        if event.type == 'TIMER':
            print('timer event fired')
            self.update()
        return {'PASS_THROUGH'}

    def update(self):
        print('update')
        msb_context.flushPendingList();
        if(bpy.context.scene.meshsync_auto_sync):
            msb_apply_scene_settings()
            msb_context.setup(bpy.context);
            msb_context.sendSceneUpdated()


classes = (
    MESHSYNC_PT_Server,
    MESHSYNC_PT_Scene,
    MESHSYNC_PT_Animation,
    MESHSYNC_OT_SyncScene,
    MESHSYNC_OT_SyncAnimations,
    MESHSYNC_OT_AutoSync,
)

def register():
    msb_initialize_properties()
    for c in classes:
        bpy.utils.register_class(c)

def unregister():
    for c in classes:
        bpy.utils.unregister_class(c)
    bpy.app.handlers.scene_update_post.remove(on_scene_update)

if __name__ == "__main__":
    register()
