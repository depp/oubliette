#!/bin/sh
set -e
xcodebuild -target Oubliette -configuration Release
test -d Oubliette && rm -r Oubliette
mkdir Oubliette
cp -R Build/Release/Oubliette.app Oubliette
cd Oubliette/Oubliette.app/Contents
mkdir Frameworks
cd Frameworks
for framework in SDL2 SDL2_image SDL2_mixer ; do
    cp -R /Library/Frameworks/${framework}.framework .
    cd ${framework}.framework
    find . -name Headers -name Headers -print0 | xargs -0 rm -r
    cd ..
done
cd ../../../..
rm -f Oubliette.zip
zip -r -y -D Oubliette-OSX.zip Oubliette
