#!/bin/sh
set -e
for dir in level sprite ; do
    cd $dir
    rm -f *-fs8.png
    for file in *.png ; do
        base=${file%.png}
        pngquant ${base}.png
        mv ${base}-fs8.png ${base}.png
    done
    cd ..
done
