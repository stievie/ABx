@echo off

rem These do not change
..\Tools\PackageTool.exe ..\bin\CoreData\ CoreData.pak -c
..\Tools\PackageTool.exe ..\bin\Data\ Data.pak -c
..\Tools\PackageTool.exe ..\bin\Autoload\ Autoload.pak -c

..\Tools\PackageTool.exe ..\bin\AbData\ AbData.pak -c
..\Tools\PackageTool.exe ..\bin\SoundData\ SoundData.pak -c
