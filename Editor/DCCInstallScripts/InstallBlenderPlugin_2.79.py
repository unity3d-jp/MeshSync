import bpy

bpy.ops.wm.addon_install(overwrite=True, filepath="{0}")
bpy.ops.wm.addon_enable(module='unity_mesh_sync')
bpy.ops.wm.save_userpref()
