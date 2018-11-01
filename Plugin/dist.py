#!/usr/bin/env python
import os
import glob
import shutil

dist_dir = os.path.dirname(os.path.realpath(__file__)) + "/dist"
os.chdir(dist_dir)

# make Blender addon archives
blender_dirs = glob.glob('UnityMeshSync_Blender*')
for blender_dir in blender_dirs:
    if not os.path.isdir(blender_dir):
        continue
    os.chdir(blender_dir)
    for t in os.listdir('.'):
        if os.path.isdir(t):
            shutil.make_archive(t, 'zip', root_dir=t)
            shutil.rmtree(t)
            print(t + '.zip')
    os.chdir('..')

# make plugin archives
targets = []
targets += glob.glob('UnityMeshSync_3dsMax*')
targets += glob.glob('UnityMeshSync_Blender*')
targets += glob.glob('UnityMeshSync_Maya*')
targets += glob.glob('UnityMeshSync_MotionBuilder*')
targets += glob.glob('UnityMeshSync_Metasequoia*')
targets += glob.glob('UnityMeshSync_xismo*')
for t in targets:
    if not os.path.isdir(t):
        continue
    shutil.make_archive(t, 'zip', root_dir='.', base_dir=t)
    print(t + '.zip')