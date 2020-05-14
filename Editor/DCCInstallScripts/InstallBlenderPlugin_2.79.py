import bpy

bpy.ops.preferences.addon_install(overwrite=True, filepath="{0}")
bpy.ops.preferences.addon_enable(module='unity_mesh_sync')
bpy.ops.wm.save_userpref()
