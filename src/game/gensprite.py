#!/usr/bin/env python3
# Copyright 2014 Dietrich Epp.
# This file is part of Oubliette.  Oubliette is licensed under the terms
# of the 2-clause BSD license.  For more information, see LICENSE.txt.
import os
import subprocess
import collections
os.chdir(os.path.dirname(os.path.realpath(__file__)))
root = '../../data/sprite'
Image = collections.namedtuple('Image', 'name width height')
images = []
AUTOGEN = '// This file is automatically generated.'
for fname in os.listdir(root):
    if fname.startswith('.') or not fname.endswith('.png'):
        continue
    path = os.path.join(root, fname)
    info = subprocess.check_output(
        ['gm', 'identify', '-format', '%w %h', path])
    width, height = [int(x) for x in info.split()]
    images.append(Image(os.path.splitext(fname)[0], width, height))
images.sort(key=lambda x: x.name)
with open('sprite_enum.hpp', 'w') as fp:
    print(AUTOGEN, file=fp)
    print('enum class sprite {', file=fp);
    for image in images[:-1]:
        print('    {},'.format(image.name.upper()), file=fp)
    print('    {}'.format(images[-1].name.upper()), file=fp)
    print('};', file=fp)
with open('sprite_array.hpp', 'w') as fp:
    print(AUTOGEN, file=fp)
    for image in images:
        print('{{ "{}", 0, 0, {}, {} }},'
              .format(image.name, image.width, image.height), file=fp)