from time import time
import bpy
import bmesh
from bpy.app.handlers import persistent
import MeshSyncClientBlender as ms

bl_info = {
    "name": "Unity Mesh Sync",
    "author": "Unity Technologies",
    "version": (2018, 4, 20),
    "blender": (2, 79),
    "description": "Sync Meshes with Unity",
    "location": "View3D > Mesh Sync",
    "tracker_url": "https://github.com/unity3d-jp/MeshSync",
    "support": "COMMUNITY",
    "category": "Import-Export",
}

msb_context = ms.Context()
msb_last_sent = 0.0
msb_updated = []
msb_added = set()


def msb_sync_all():
    msb_sync(bpy.data.objects)


def msb_sync_updated():
    if not bpy.data.objects.is_updated:
        return
    msb_sync([obj for obj in bpy.data.objects if obj.is_updated or obj.is_updated_data])
    

def msb_sync(targets):
    global msb_context
    ctx = msb_context
    if not ctx.prepare():
        return

    start_time = time()
    scene = bpy.context.scene

    # materials
    for mat in bpy.data.materials:
        ctx.addMaterial(mat)

    for obj in targets:
        if not (obj.name in bpy.data.objects):
            ctx.addDeleted(msb_get_path(obj))
        elif (obj.type == 'MESH' and scene.meshsync_sync_meshes and not msb_is_particle_system(obj)) or\
             (obj.type == 'CAMERA' and scene.meshsync_sync_cameras) or\
             (obj.type == 'LAMP' and scene.meshsync_sync_lights) or\
             (obj.dupli_group != None):
            msb_add_object(ctx, obj)

    ctx.send()
    msb_added.clear()
    end_time = time()
    msb_last_sent = end_time
    #print("msb_sync(): done (", end_time-start_time, "sec)")


def msb_add_object(ctx, obj):
    if obj in msb_added:
        return None
    msb_construct_tree(ctx, obj.parent)

    ret = None
    if obj.type == 'MESH':
        ret = msb_add_mesh(ctx, obj)
    elif obj.type == 'CAMERA':
        ret = msb_add_camera(ctx, obj)
    elif obj.type == 'LAMP':
        ret = msb_add_light(ctx, obj)
    else:
        ret = msb_add_transform(ctx, obj)

    if ret != None:
        msb_added.add(obj)
        #ret.index = obj.pass_index
    return ret


def msb_construct_tree(ctx, obj):
    if obj == None:
        return
    if obj.parent != None:
        msb_construct_tree(ctx, obj.parent)
    msb_add_object(ctx, obj)


def msb_get_path(obj):
    path = None
    if obj.parent != None:
        path = msb_get_path(obj.parent)
    else:
        path = ''
    path += '/'
    path += obj.name
    return path


def msb_add_mesh(ctx, obj):
    path = msb_get_path(obj)
    dst = ctx.addMesh(path)
    msb_handle_dupli_group(ctx, path, obj)
    if obj.hide:
        dst.visible = False
    else:
        scene = bpy.context.scene
        dst.visible = obj.is_visible(scene)
        dst.swap_faces = True

        data = None
        if scene.meshsync_apply_modifiers:
            data = obj.to_mesh(scene, True, 'PREVIEW')
        else:
            data = obj.data

        material_ids = []
        for mat in data.materials:
            material_ids.append(msb_get_material_id(mat))

        ctx.extractMeshData(dst, obj)
    return dst


def msb_add_camera(ctx, obj):
    path = msb_get_path(obj)
    dst = ctx.addCamera(msb_get_path(obj))
    ctx.extractCameraData(dst, obj)
    msb_handle_dupli_group(ctx, path, obj)
    return dst


def msb_add_light(ctx, obj):
    path = msb_get_path(obj)
    dst = ctx.addLight(msb_get_path(obj))
    ctx.extractLightData(dst, obj)
    msb_handle_dupli_group(ctx, path, obj)
    return dst


def msb_add_transform(ctx, obj):
    path = msb_get_path(obj)
    dst = ctx.addTransform(path)
    ctx.extractTransformData(dst, obj)
    msb_handle_dupli_group(ctx, path, obj)
    return dst


def msb_add_reference_nodes(ctx, base_path, obj):
    local_path = msb_get_path(obj)
    path = base_path + local_path
    dst = ctx.addTransform(path)
    ctx.extractTransformData(dst, obj)
    msb_handle_dupli_group(ctx, path, obj)
    dst.reference = local_path
    for c in obj.children:
        msb_add_reference_nodes(ctx, path, c)
    return dst


def msb_handle_dupli_group(ctx, base_path, obj):
    group = obj.dupli_group
    if group != None:
        o = group.dupli_offset
        for c in group.objects:
            cdst = msb_add_reference_nodes(ctx, base_path, c)
            if c.parent == None:
                pos = cdst.position
                pos[0] -= o.x
                pos[1] -= o.y
                pos[2] -= o.z
                cdst.position = pos


def msb_is_particle_system(obj):
    for mod in obj.modifiers:
        if mod.type == 'PARTICLE_SYSTEM':
            return True
    return False


def msb_get_material_id(material):
    i = 0
    for mat in bpy.data.materials:
        if material == mat:
            return i
        i += 1
    return 0


def msb_update_settings(self = None, context = None):
    global msb_context
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
    #print("msb_update_settings")
    return None


def msb_initialize_properties():
    bpy.types.Scene.meshsync_server_address = bpy.props.StringProperty(default = "127.0.0.1", name = "Server Address", update = msb_update_settings)
    bpy.types.Scene.meshsync_server_port = bpy.props.IntProperty(default = 8080, name = "Server Port", update = msb_update_settings)
    bpy.types.Scene.meshsync_scale_factor = bpy.props.FloatProperty(default = 1.0, name = "Scale Factor", update = msb_update_settings)
    bpy.types.Scene.meshsync_sync_meshes = bpy.props.BoolProperty(default = True, name = "Sync Meshes", update = msb_update_settings)
    bpy.types.Scene.meshsync_sync_normals = bpy.props.BoolProperty(default = True, name = "Normals", update = msb_update_settings)
    bpy.types.Scene.meshsync_sync_uvs = bpy.props.BoolProperty(default = True, name = "UVs", update = msb_update_settings)
    bpy.types.Scene.meshsync_sync_colors = bpy.props.BoolProperty(default = False, name = "Colors", update = msb_update_settings)
    bpy.types.Scene.meshsync_sync_bones = bpy.props.BoolProperty(default = True, name = "Bones", update = msb_update_settings)
    bpy.types.Scene.meshsync_sync_poses = bpy.props.BoolProperty(default = True, name = "Poses", update = msb_update_settings)
    bpy.types.Scene.meshsync_sync_blendshapes = bpy.props.BoolProperty(default = True, name = "Blend Shapes", update = msb_update_settings)
    bpy.types.Scene.meshsync_apply_modifiers = bpy.props.BoolProperty(default = False, name = "Apply Modifiers", update = msb_update_settings)
    bpy.types.Scene.meshsync_sync_cameras = bpy.props.BoolProperty(default = True, name = "Sync Cameras", update = msb_update_settings)
    bpy.types.Scene.meshsync_sync_lights = bpy.props.BoolProperty(default = True, name = "Sync Lights", update = msb_update_settings)
    bpy.types.Scene.meshsync_sync_animations = bpy.props.BoolProperty(default = False, name = "Sync Animations", update = msb_update_settings)
    bpy.types.Scene.meshsync_sample_animation = bpy.props.BoolProperty(default = True, name = "Sample Animations", update = msb_update_settings)
    bpy.types.Scene.meshsync_animation_sps = bpy.props.IntProperty(default = 5, name = "Samples", update = msb_update_settings)
    bpy.types.Scene.meshsync_auto_sync = bpy.props.BoolProperty(default = False, name = "Auto Sync", update = msb_update_settings)


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
        self.layout.operator("meshsync.fcurve", text="Debug")


class MeshSync_OpSyncAll(bpy.types.Operator):
    bl_idname = "meshsync.sync_all"
    bl_label = "Sync All"
    def execute(self, context):
        msb_context.setup()
        msb_sync_all()
        return{'FINISHED'}

class MeshSync_OpFCurve(bpy.types.Operator):
    bl_idname = "meshsync.fcurve"
    bl_label = "fcurve"
    def execute(self, context):
        obj = bpy.context.active_object
        print("object " + obj.name)
        for curve in obj.animation_data.action.fcurves:
            print("  curve " + curve.data_path + "[" + str(curve.array_index) + "]")
            curve.evaluate(0)
            print(curve.evaluate)
        return{'FINISHED'}


@persistent
def on_scene_update(context):
    msb_context.setup()
    msb_context.flushPendingList();
    if(bpy.context.scene.meshsync_auto_sync):
        msb_sync_updated()

def register():
    msb_initialize_properties()
    bpy.utils.register_module(__name__)
    bpy.app.handlers.scene_update_post.append(on_scene_update)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.app.handlers.scene_update_post.remove(on_scene_update)

if __name__ == "__main__":
    register()
