@echo off

if exist fwclient.zip del fwclient.zip

..\Tools\PackageTool.exe ..\bin\AbData\ AbData.pak

"%PROGRAMFILES%\7-Zip\7z.exe" a -tzip fwclient.zip ..\bin\fw.exe ..\bin\d3dcompiler_47.dll ..\bin\libeay32.dll ..\bin\ssleay32.dll *.pak GameData\* config.xml ..\README.md ..\LICENSE.txt ..\..\CREDITS.md
