from time import time
import bpy
import bmesh
from bpy.app.handlers import persistent
import MeshSync as ms

bl_info = {
    "name": "Unity Mesh Sync",
    "author": "Unity Technologies",
    "version": (2018, 1, 1),
    "blender": (2, 79),
    "description": "Sync Meshes with Unity",
}

msb_context = ms.Context()
msb_context.handedness = 2
msb_last_sent = 0.0
msb_updated = []
msb_added = set()


def msb_sync_all():
    msb_sync(bpy.data.objects)


def msb_sync_updated():
    if not bpy.data.objects.is_updated:
        return
    msb_sync([obj for obj in bpy.data.objects if obj.is_updated])


def msb_sync(targets):
    global msb_context
    ctx = msb_context
    if ctx.isSending():
        return

    scene = bpy.context.scene
    for obj in targets:
        if obj.type == 'MESH' or\
          (obj.type == 'ARMATURE' and scene['meshsync_sync_bones']) or\
          (obj.type == 'CAMERA' and scene['meshsync_sync_cameras']):
            msb_add_object(ctx, obj)
    ctx.send()
    msb_added.clear()
    msb_last_sent = time()
    print("msb_sync(): done")


def msb_add_object(ctx, obj):
    if obj in msb_added:
        return None
    msb_construct_tree(ctx, obj.parent)

    ret = None
    if obj.type == 'MESH':
        ret = msb_add_mesh(ctx, obj)
    elif obj.type == 'CAMERA':
        ret = msb_add_camera(ctx, obj)
    else:
        ret = msb_add_transform(ctx, obj)
    msb_added.add(obj)
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


def msb_extract_transform(dst, obj):
    t = obj.location
    s = obj.scale
    dst.position = [t.x, t.y, t.z]
    dst.scale = [s.x, s.y, s.z]

    rmode = obj.rotation_mode
    if rmode == 'QUATERNION':
        r = obj.rotation_quaternion
        dst.rotation_quaternion = [r.x, r.y, r.z, r.w]
    elif rmode == 'AXIS_ANGLE':
        r = obj.rotation_axis_angle
        dst.rotation_axis_angle = [r[0], r[1], r[2], r[3]]
    elif rmode == 'XYZ':
        r = obj.rotation_euler
        dst.rotation_xyz = [r.x, r.y, r.z]
    elif rmode == 'XZY':
        r = obj.rotation_euler
        dst.rotation_xzy = [r.x, r.y, r.z]
    elif rmode == 'YXZ':
        r = obj.rotation_euler
        dst.rotation_yxz = [r.x, r.y, r.z]
    elif rmode == 'YZX':
        r = obj.rotation_euler
        dst.rotation_yzx = [r.x, r.y, r.z]
    elif rmode == 'ZXY':
        r = obj.rotation_euler
        dst.rotation_zxy = [r.x, r.y, r.z]
    elif rmode == 'ZYX':
        r = obj.rotation_euler
        dst.rotation_zyx = [r.x, r.y, r.z]


def msb_add_mesh(ctx, obj):
    dst = ctx.addMesh(msb_get_path(obj))
    msb_extract_transform(dst, obj)

    scene = bpy.context.scene
    if not obj.is_visible(scene):
        dst.visible = False
    else:
        dst.visible = True
        dst.swap_faces = True

        data = None
        if scene['meshsync_apply_modifiers']:
            data = obj.to_mesh(scene, True, 'PREVIEW')
        else:
            data = obj.data

        for vtx in data.vertices:
            p = vtx.co
            dst.addVertex([p.x, p.y, p.z])

        data.calc_normals_split()
        for loop in data.loops:
            n = loop.normal
            dst.addNormal([n.x, n.y, n.z])
        if len(data.uv_layers) > 0:
            for v in data.uv_layers.active.data:
                dst.addUV([v.uv.x, v.uv.y])
        if scene['meshsync_sync_colors'] and len(data.vertex_colors) > 0:
            for c in data.vertex_colors.active.data:
                dst.addColor([c.r, c.g, c.b, c.a])
        for poly in data.polygons:
            dst.addMaterialID(poly.material_index)
            dst.addCount(len(poly.vertices));
            for idx in poly.vertices:
                dst.addIndex(idx)

        if scene['meshsync_sync_bones'] and len(obj.vertex_groups) > 0:
            arm = bpy.data.objects['Armature']
            group_names = [g.name for g in obj.vertex_groups]
            for bone in arm.pose.bones:
                if bone.name not in group_names:
                    continue
                gidx = obj.vertex_groups[bone.name].index
                bone_verts = [v for v in data.vertices if gidx in [g.group for g in v.groups]]
                weights = [0.0] * len(data.vertices)
                for v in bone_verts:
                    weights[v.index] = v.groups[gidx].weight

                bdst = dst.addBone(msb_get_path(bone))
                for w in weights:
                    bdst.addWeight(w)
    return dst


def msb_add_camera(ctx, obj):
    dst = ctx.addCamera(msb_get_path(obj))
    msb_extract_transform(dst, obj)
    # todo
    return dst


def msb_add_transform(ctx, obj):
    dst = ctx.addTransform(msb_get_path(obj))
    msb_extract_transform(dst, obj)
    # todo
    return dst


def MeshSync_InitProperties():
    bpy.types.Scene.meshsync_server_addr = bpy.props.StringProperty(name = "Server Address")
    bpy.types.Scene.meshsync_server_port = bpy.props.IntProperty(name = "Server Port")
    bpy.types.Scene.meshsync_scale_factor = bpy.props.FloatProperty(name = "Scale Factor")
    bpy.types.Scene.meshsync_apply_modifiers = bpy.props.BoolProperty(name = "Apply Modifiers")
    bpy.types.Scene.meshsync_sync_colors = bpy.props.BoolProperty(name = "Sync Vertex Colors")
    bpy.types.Scene.meshsync_sync_bones = bpy.props.BoolProperty(name = "Sync Bones")
    bpy.types.Scene.meshsync_sync_cameras = bpy.props.BoolProperty(name = "Sync Cameras")
    bpy.types.Scene.meshsync_sync_animations = bpy.props.BoolProperty(name = "Sync Animations")
    bpy.types.Scene.meshsync_auto_sync = bpy.props.BoolProperty(name = "Auto Sync")
    bpy.types.Scene.meshsync_interval = bpy.props.FloatProperty(name = "Interval (Sec)")
    scene = bpy.context.scene
    scene['meshsync_server_addr'] = "localhost"
    scene['meshsync_server_port'] = 8080
    scene['meshsync_scale_factor'] = 1.0
    scene['meshsync_apply_modifiers'] = True
    scene['meshsync_sync_colors'] = False
    scene['meshsync_sync_bones'] = True
    scene['meshsync_sync_cameras'] = True
    scene['meshsync_sync_animations'] = True
    scene['meshsync_auto_sync'] = False
    scene['meshsync_interval'] = 1.0


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
        self.layout.prop(context.scene, 'meshsync_apply_modifiers')
        self.layout.prop(context.scene, 'meshsync_sync_colors')
        self.layout.prop(context.scene, 'meshsync_sync_bones')
        self.layout.prop(context.scene, 'meshsync_sync_cameras')
        self.layout.prop(context.scene, 'meshsync_sync_animations')
        self.layout.prop(context.scene, 'meshsync_auto_sync')
        if(scene['meshsync_auto_sync']):
            self.layout.prop(context.scene, 'meshsync_interval')
        self.layout.separator()
        self.layout.operator("meshsync.sync_all", text="Manual Sync")


class MeshSync_OpSyncAll(bpy.types.Operator):
    bl_idname = "meshsync.sync_all"
    bl_label = "Sync All"
    def execute(self, context):
        msb_sync_all()
        return{'FINISHED'}


@persistent
def on_scene_update(context):
    if(bpy.context.scene['meshsync_auto_sync']):
        msb_sync_updated()

def register():
    MeshSync_InitProperties()
    bpy.utils.register_module(__name__)
    bpy.app.handlers.scene_update_post.append(on_scene_update)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.app.handlers.scene_update_post.remove(on_scene_update)

if __name__ == "__main__":
    register()
