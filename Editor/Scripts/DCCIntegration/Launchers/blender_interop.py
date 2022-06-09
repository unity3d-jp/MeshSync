import bpy
import time

bpy.ops.meshsync.auto_sync()

def run():
    time.sleep(10)

# Make sure blender doesn't shut down:
if bpy.app.background:
    while (True):
        run()
