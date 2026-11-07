#!/usr/bin/env python3
"""vst.json "skin_post": run by gen_vst.py as `python3 skin_post.py <plugin folder>` on the built skin.

1. The knobs: the builder composes each knob size's strip from art/knob.png over art/knob_base.png. Each is replaced by
   art/knob_r<r>.png (art/make_art.py), the same size, which adds the level ring around the knob.
2. The toolkit writes 1.0.0.0 into every skin: put this plugin's own version there (vst.json 1030 = 1.0.3.0)."""
import glob
import json
import os
import re
import shutil
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SKINS = os.path.join(sys.argv[1], "Plugin Skins")


def size(path):
    with open(path, "rb") as f:
        return struct.unpack(">II", f.read(24)[16:24])


n = 0
for found in glob.glob(os.path.join(SKINS, "sh_knob_r*_*.png")):
    r = int(re.search(r"sh_knob_r(\d+)_", os.path.basename(found)).group(1))
    src = os.path.join(HERE, "art", "knob_r%d.png" % r)
    if not os.path.exists(src):
        sys.exit("skin_post: no art/knob_r%d.png for %s (add %d to RADII in art/make_art.py)" % (r, os.path.basename(found), r))
    if size(src) != size(found):
        sys.exit("skin_post: art/knob_r%d.png is %s, the skin wants %s" % (r, size(src), size(found)))
    shutil.copy(src, found)
    n += 1
v = json.load(open(os.path.join(HERE, "vst.json")))["version"]
path = os.path.join(sys.argv[1], "version.xml")
text = open(path).read()
open(path, "w").write(re.sub(r"<version>[^<]*</version>", "<version>%d.%d.%d.0</version>" % (v // 1000, v // 100 % 10, v // 10 % 10), text))
print("skin_post: %d knob strips with level rings, version %d.%d.%d" % (n, v // 1000, v // 100 % 10, v // 10 % 10))
