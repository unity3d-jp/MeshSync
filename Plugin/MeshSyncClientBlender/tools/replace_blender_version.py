#!/usr/bin/env python
import sys
import os
import re

if (len(sys.argv) != 3):
    print 'Usage: %s [file_to_modify] [version_string]' % sys.argv[0]
    quit()

content = ''
with open(sys.argv[1], 'r') as f:
    content = f.read();

r = re.compile(r"\"blender\": \((.+?)\)")
content = r.sub("\"blender\": (" + sys.argv[2] + ")", content)

with open(sys.argv[1], 'w') as f:
    f.write(content)
