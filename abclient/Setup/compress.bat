@echo off

if exist fwclient.7z del fwclient.7z

..\Tools\PackageTool.exe ..\bin\AbData\ AbData.pak -c
..\Tools\PackageTool.exe ..\bin\SoundData\ SoundData.pak -c

"%PROGRAMFILES%\7-Zip\7z.exe" a -t7z -mx fwclient.7z ..\bin\fw.exe ..\bin\abupdate.exe ..\bin\d3dcompiler_47.dll ..\bin\libcrypto-1_1-x64.dll ..\bin\libssl-1_1-x64.dll *.pak config.xml ..\README.md ..\LICENSE.txt ..\..\CREDITS.md
