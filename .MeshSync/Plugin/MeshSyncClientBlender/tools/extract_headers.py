#!/usr/bin/env python
import sys
import os
import re
import shutil

if (len(sys.argv) != 2):
    print('Usage: %s blender_source_dir' % sys.argv[0])
    quit()

src_dir = sys.argv[1]
dst_dir = os.getcwd()

os.chdir(src_dir + '/source/blender')
print(os.getcwd())

def cond(f):
    return re.search('\.h$', f) or\
        f == 'math_base_inline.c' or\
        f == 'math_bits_inline.c' or\
        f == 'math_vector_inline.c' or\
        f == 'math_color_inline.c' or\
        f == 'math_geom_inline.c' or\
        f == 'math_vector.c' or\
        f == 'math_matrix.c' or\
        f == 'math_rotation.c' or\
        f == 'math_geom.c'

for target in [
    'blenkernel',
    'blenlib',
    'bmesh',
    'makesdna',
    'makesrna',
    'python'
    ]:
    shutil.copytree(target, dst_dir+'/'+target,
        ignore = lambda dir, list: [f for f in list if os.path.isfile(dir + '/' + f) and not cond(f)])
