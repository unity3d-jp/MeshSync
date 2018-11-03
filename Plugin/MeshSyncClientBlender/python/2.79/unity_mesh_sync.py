bl_info = {
    "name": "Unity Mesh Sync",
    "author": "Unity Technologies",
    "version": (2018, 9, 18),
    "blender": (2, 79, 0),
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

class MESHSYNC_PT:
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_category = "Mesh Sync"


class MESHSYNC_PT_Server(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Server"

    def draw(self, context):
        scene = bpy.context.scene
        self.layout.prop(scene, 'meshsync_server_address')
        self.layout.prop(scene, 'meshsync_server_port')


class MESHSYNC_PT_Scene(MESHSYNC_PT, bpy.types.Panel):
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
        self.layout.prop(scene, 'meshsync_auto_sync')
        self.layout.operator("meshsync.sync_scene", text="Manual Sync")
        self.layout.separator()

class MESHSYNC_PT_Animation(MESHSYNC_PT, bpy.types.Panel):
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


@persistent
def on_scene_update(context):
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
)

def register():
    msb_initialize_properties()
    for c in classes:
        bpy.utils.register_class(c)
    bpy.app.handlers.scene_update_post.append(on_scene_update)

def unregister():
    for c in classes:
        bpy.utils.unregister_class(c)
    bpy.app.handlers.scene_update_post.remove(on_scene_update)

if __name__ == "__main__":
    register()
