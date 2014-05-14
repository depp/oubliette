#!/bin/sh
set -x
set -e
test -d Oubliette && rm -r Oubliette
for platform in Windows OSX ; do
    unzip Oubliette-${platform}.zip
done
cp README.txt Oubliette
cp LICENSE.txt Oubliette
cp -R data Oubliette/Data
cd Oubliette/Data
find . -type f -name '.*' -delete
rm quant.py
cd ..
now=$(date '+%Y%m%d%H%M')
find -L . -type f -exec touch -t ${now} '{}' +
cd ..
rm -f Oubliette.zip
zip -r -y -D Oubliette.zip Oubliette
