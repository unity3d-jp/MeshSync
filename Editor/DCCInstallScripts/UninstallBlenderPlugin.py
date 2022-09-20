import bpy

bpy.ops.preferences.addon_remove(module='unity_mesh_sync')
bpy.ops.wm.save_userpref()

bpy.ops.preferences.addon_disable(module='MeshSyncClientBlender')
bpy.ops.wm.save_userpref()

bpy.ops.preferences.addon_remove(module='MeshSyncClientBlender')
bpy.ops.wm.save_userpref()

