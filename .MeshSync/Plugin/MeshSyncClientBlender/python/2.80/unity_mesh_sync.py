bl_info = {
    "name": "Unity Mesh Sync",
    "author": "Unity Technologies",
    "version": (2018, 11, 30),
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

class MESHSYNC_PT:
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = ".workspace"


class MESHSYNC_PT_Main(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "MeshSync"

    def draw(self, context):
        pass


class MESHSYNC_PT_Server(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Server"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.prop(scene, 'meshsync_server_address')
        layout.prop(scene, 'meshsync_server_port')


class MESHSYNC_PT_Scene(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Scene"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.prop(scene, 'meshsync_scale_factor')
        layout.prop(scene, 'meshsync_sync_meshes')
        if scene.meshsync_sync_meshes:
            b = layout.box()
            b.prop(scene, 'meshsync_sync_normals')
            b.prop(scene, 'meshsync_sync_uvs')
            b.prop(scene, 'meshsync_sync_colors')
            b.prop(scene, 'meshsync_convert_to_mesh')
            b.prop(scene, 'meshsync_make_double_sided')
            b.prop(scene, 'meshsync_bake_modifiers')
        layout.prop(scene, 'meshsync_sync_bones')
        layout.prop(scene, 'meshsync_sync_blendshapes')
        layout.prop(scene, 'meshsync_sync_textures')
        layout.prop(scene, 'meshsync_sync_cameras')
        layout.prop(scene, 'meshsync_sync_lights')
        layout.separator()
        if MESHSYNC_OT_AutoSync._timer:
            layout.operator("meshsync.auto_sync", text="Auto Sync", icon="PAUSE")
        else:
            layout.operator("meshsync.auto_sync", text="Auto Sync", icon="PLAY")
        layout.operator("meshsync.sync_scene", text="Manual Sync")


class MESHSYNC_PT_Animation(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Animation"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.prop(scene, 'meshsync_animation_ts')
        layout.prop(scene, 'meshsync_animation_fi')
        layout.prop(scene, 'meshsync_animation_kfr')
        layout.operator("meshsync.sync_animations", text="Sync")


class MESHSYNC_PT_Version(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Plugin Version"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.label(text = msb_context.version)


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
        scene = bpy.context.scene
        if not MESHSYNC_OT_AutoSync._timer:
            scene.meshsync_auto_sync = True
            MESHSYNC_OT_AutoSync._timer = context.window_manager.event_timer_add(1.0 / 3.0, window=context.window)
            context.window_manager.modal_handler_add(self)
            return {'RUNNING_MODAL'}
        else:
            scene.meshsync_auto_sync = False
            context.window_manager.event_timer_remove(MESHSYNC_OT_AutoSync._timer)
            MESHSYNC_OT_AutoSync._timer = None
            return {'FINISHED'}

    def modal(self, context, event):
        if event.type == 'TIMER':
            self.update()
        return {'PASS_THROUGH'}

    def update(self):
        msb_context.flushPendingList();
        msb_apply_scene_settings()
        msb_context.setup(bpy.context);
        msb_context.sendSceneUpdated()


classes = (
    MESHSYNC_PT_Main,
    MESHSYNC_PT_Server,
    MESHSYNC_PT_Scene,
    MESHSYNC_PT_Animation,
    MESHSYNC_PT_Version,
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
