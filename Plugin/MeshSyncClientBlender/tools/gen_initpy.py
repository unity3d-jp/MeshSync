#!/usr/bin/env python
import sys

if (len(sys.argv) != 3):
    print 'Usage: %s [file] [version]' % sys.argv[0]
    quit()

with open(sys.argv[1], 'w') as f:
    content = "from MeshSyncClientBlender%s.MeshSyncClientBlender import *" % sys.argv[2]
    f.write(content)
