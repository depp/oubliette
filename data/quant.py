#!/usr/bin/env python3
import os
import subprocess
import pickle

os.chdir(os.path.dirname(os.path.abspath(__file__)))

mtimes = {}

def read_db():
    global mtimes
    try:
        with open('.quant.dat', 'rb') as fp:
            mtimes = pickle.load(fp)
    except FileNotFoundError:
        pass

def write_db():
    with open('.quant.dat.tmp', 'wb') as fp:
        pickle.dump(mtimes, fp, protocol=-1)
    os.rename('.quant.dat.tmp', '.quant.dat')

def rm_f(path):
    try:
        os.unlink(path)
    except FileNotFoundError:
        pass

def convert_root(root):
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [x for x in dirnames if not x.startswith('.')]
        for filename in filenames:
            if filename.startswith('.') or not filename.endswith('.png'):
                continue
            base, ext = os.path.splitext(filename)
            if base.endswith('-fs8'):
                continue
            filepath = os.path.join(dirpath, filename)
            st = os.stat(filepath)
            if st.st_mtime == mtimes.get(filepath, 0):
                continue
            temppath = os.path.join(dirpath, '{}-fs8.png'.format(base))
            subprocess.check_call(
                ['pngquant', '-o', temppath, filepath])
            os.rename(temppath, filepath)
            st2 = os.stat(filepath)
            mtimes[filepath] = st2.st_mtime
            print('{}: {} -> {}'.format(filepath, st.st_size, st2.st_size))

def main():
    read_db()
    try:
        for root in ['sprite', 'ui', 'level']:
            convert_root(root)
    finally:
        write_db()

main()
