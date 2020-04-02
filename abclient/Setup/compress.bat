@echo off

if exist fwclient.7z del fwclient.7z

..\Tools\PackageTool.exe ..\bin\AbData\ AbData.pak
..\Tools\PackageTool.exe ..\bin\SoundData\ SoundData.pak

"%PROGRAMFILES%\7-Zip\7z.exe" a -t7z -mx fwclient.7z ..\bin\fw.exe ..\bin\d3dcompiler_47.dll ..\bin\libeay32.dll ..\bin\ssleay32.dll *.pak config.xml ..\README.md ..\LICENSE.txt ..\..\CREDITS.md
