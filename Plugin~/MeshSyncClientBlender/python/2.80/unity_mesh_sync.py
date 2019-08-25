bl_info = {
    "name": "Unity Mesh Sync",
    "author": "Unity Technologies",
    "version": (2019, 8, 19),
    "blender": (2, 80, 57),
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
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tool"


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
        layout.prop(scene, "meshsync_server_address")
        layout.prop(scene, "meshsync_server_port")


class MESHSYNC_PT_Scene(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Scene"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.prop(scene, "meshsync_scale_factor")
        layout.prop(scene, "meshsync_sync_meshes")
        if scene.meshsync_sync_meshes:
            b = layout.box()
            b.prop(scene, "meshsync_curves_as_mesh")
            b.prop(scene, "meshsync_make_double_sided")
            b.prop(scene, "meshsync_bake_modifiers")
            b.prop(scene, "meshsync_bake_transform")
        layout.prop(scene, "meshsync_sync_bones")
        layout.prop(scene, "meshsync_sync_blendshapes")
        layout.prop(scene, "meshsync_sync_textures")
        layout.prop(scene, "meshsync_sync_cameras")
        layout.prop(scene, "meshsync_sync_lights")
        layout.separator()
        if MESHSYNC_OT_AutoSync._timer:
            layout.operator("meshsync.auto_sync", text="Auto Sync", icon="PAUSE")
        else:
            layout.operator("meshsync.auto_sync", text="Auto Sync", icon="PLAY")
        layout.operator("meshsync.send_objects", text="Manual Sync")
        layout.operator("meshsync.export_cache", text="Export Cache")


class MESHSYNC_PT_Animation(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Animation"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.prop(scene, "meshsync_frame_step")
        layout.operator("meshsync.send_animations", text="Sync")


class MESHSYNC_PT_Version(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Plugin Version"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.label(text = msb_context.PLUGIN_VERSION)


class MESHSYNC_OT_AutoSync(bpy.types.Operator):
    bl_idname = "meshsync.auto_sync"
    bl_label = "Auto Sync"
    _timer = None

    def invoke(self, context, event):
        scene = bpy.context.scene
        if not MESHSYNC_OT_AutoSync._timer:
            scene.meshsync_auto_sync = True
            if not scene.meshsync_auto_sync:
                # server not available
                return {'FINISHED'}
            MESHSYNC_OT_AutoSync._timer = context.window_manager.event_timer_add(1.0 / 3.0, window=context.window)
            context.window_manager.modal_handler_add(self)
            return {'RUNNING_MODAL'}
        else:
            scene.meshsync_auto_sync = False
            context.window_manager.event_timer_remove(MESHSYNC_OT_AutoSync._timer)
            MESHSYNC_OT_AutoSync._timer = None
            return {'FINISHED'}

    def modal(self, context, event):
        if event.type == "TIMER":
            self.update()
        return {'PASS_THROUGH'}

    def update(self):
        msb_context.flushPendingList();
        msb_apply_scene_settings()
        msb_context.setup(bpy.context);
        msb_context.exportUpdatedObjects()


classes = (
    MESHSYNC_PT_Main,
    MESHSYNC_PT_Server,
    MESHSYNC_PT_Scene,
    MESHSYNC_PT_Animation,
    MESHSYNC_PT_Version,
    MESHSYNC_OT_SendObjects,
    MESHSYNC_OT_SendAnimations,
    MESHSYNC_OT_AutoSync,
    MESHSYNC_OT_ExportCache,
)

def register():
    msb_initialize_properties()
    for c in classes:
        bpy.utils.register_class(c)

def unregister():
    for c in classes:
        bpy.utils.unregister_class(c)

if __name__ == "__main__":
    register()
