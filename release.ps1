$sdl = "C:\SDL2-2.0.3"
$sdl_image = "C:\SDL2_image-2.0.0"
$glew = "C:\glew-1.10.0"
$7zip = "C:\Program Files (x86)\7-Zip\7z.exe"

$ErrorActionPreference = "Stop"
mkdir Oubliette
cp Release\Oubliette.exe Oubliette
cp README.txt Oubliette
cp LICENSE.txt Oubliette
cp -r data Oubliette\Data
rm Oubliette\Data\.gitignore, Oubliette\Data\quant.sh
cp 'C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.CRT\msvcr120.dll','C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.CRT\msvcp120.dll' Oubliette
cp $sdl\lib\x86\*.dll,$sdl_image\lib\x86\*.dll,$sdl_image\lib\x86\*.txt,$glew\bin\Release\Win32\glew32.dll Oubliette
& $7zip a Oubliette.zip Oubliette
