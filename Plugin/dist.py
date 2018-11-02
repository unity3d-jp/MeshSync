#!/usr/bin/env python
import os
import glob
import shutil
import re

dist_dir = os.path.dirname(os.path.realpath(__file__)) + "/dist"
os.chdir(dist_dir)

is_blender = re.compile(r"_Blender.*")
for t in os.listdir('.'):
    if not os.path.isdir(t):
        continue
    if is_blender.search(t):
        shutil.make_archive(t, 'zip', root_dir=t)
    else:
        shutil.make_archive(t, 'zip', root_dir='.', base_dir=t)
    print(t + '.zip')