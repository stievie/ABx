@echo off

rem These do not change
..\Tools\PackageTool.exe ..\bin\CoreData\ CoreData.pak
..\Tools\PackageTool.exe ..\bin\Data\ Data.pak
..\Tools\PackageTool.exe ..\bin\Autoload\ Autoload.pak

..\Tools\PackageTool.exe ..\bin\AbData\ AbData.pak
