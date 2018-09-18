#!/usr/bin/env python
import os
import glob
import shutil

dist_dir = os.path.dirname(os.path.realpath(__file__)) + "/dist"
os.chdir(dist_dir)

targets = []
targets += glob.glob('UnityMeshSync_3dsMax*')
targets += glob.glob('UnityMeshSync_Blender*')
targets += glob.glob('UnityMeshSync_Maya*')
targets += glob.glob('UnityMeshSync_MotionBuilder*')
targets += glob.glob('UnityMeshSync_Metasequoia*')
targets += glob.glob('UnityMeshSync_xismo*')
for target in targets:
    if os.path.isdir(target):
        shutil.make_archive(target, 'zip', root_dir=target)
        print(target)