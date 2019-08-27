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


class MESHSYNC_PT_Cache(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Cache"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.operator("meshsync.export_cache", text="Export Cache")


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


class MESHSYNC_OT_ExportCache(bpy.types.Operator):
    bl_idname = "meshsync.export_cache"
    bl_label = "Export Cache"
    bl_description = "Export Cache"

    filepath: bpy.props.StringProperty(subtype = "FILE_PATH")
    filename: bpy.props.StringProperty()
    directory: bpy.props.StringProperty(subtype = "FILE_PATH")

    # cache properties
    object_scope: bpy.props.EnumProperty(
        name = "Object Scope",
        default = "0",
        items = {
            ("0", "All", "Export all objects"),
            ("1", "Selected", "Export selected objects"),
        })
    frame_range: bpy.props.EnumProperty(
        name = "Frame Range",
        default = "1",
        items = {
            ("0", "Current", "Export current frame"),
            ("1", "All", "Export all frames"),
            ("2", "Custom", "Export speficied frames"),
        })
    frame_begin: bpy.props.IntProperty(name = "Frame Begin", default = 1)
    frame_end: bpy.props.IntProperty(name = "Frame End", default = 100)
    frame_step: bpy.props.IntProperty(name = "Frame Step", default = 1, min = 1)
    material_frame_range: bpy.props.EnumProperty(
        name = "Material Range",
        default = "1",
        items = {
            ("0", "None", "Export no materials"),
            ("1", "One", "Export one frame of materials"),
            ("2", "All", "Export all frames of materials"),
        })
    zstd_compression_level: bpy.props.IntProperty(name = "ZSTD Compression", default = 3)
    make_double_sided: bpy.props.BoolProperty(name = "Make Double Sided", default = False)
    bake_modifiers: bpy.props.BoolProperty(name = "Bake Modifiers", default = True)
    bake_transform: bpy.props.BoolProperty(name = "Bake Transform", default = False)
    convert_to_mesh: bpy.props.BoolProperty(name = "Convert To Mesh", default = True)
    flatten_hierarchy: bpy.props.BoolProperty(name = "Flatten Hierarchy", default = False)
    merge_meshes: bpy.props.BoolProperty(name = "Merge Meshes", default = False)
    strip_normals: bpy.props.BoolProperty(name = "Strip Normals", default = False)
    strip_tangents: bpy.props.BoolProperty(name = "Strip Tangents", default = False)

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


classes = (
    MESHSYNC_PT_Main,
    MESHSYNC_PT_Server,
    MESHSYNC_PT_Scene,
    MESHSYNC_PT_Animation,
    MESHSYNC_PT_Cache,
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
