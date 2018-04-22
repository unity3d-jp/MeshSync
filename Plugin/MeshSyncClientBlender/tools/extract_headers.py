#!/usr/bin/env python
import sys
import os
import re
import shutil

if (len(sys.argv) != 2):
    print 'Usage: %s blender_source_dir' % sys.argv[0]
    quit()

src_dir = sys.argv[1]
dst_dir = os.getcwd()

os.chdir(src_dir + '/source/blender')
print(os.getcwd())

for target in [
    'blenlib',
    'makesdna',
    'makesrna',
    'python'
    ]:
    shutil.copytree(target, dst_dir+'/'+target,
        ignore = lambda dir, list: [f for f in list if os.path.isfile(dir + '/' + f) and not re.search('\.h$', f)])
