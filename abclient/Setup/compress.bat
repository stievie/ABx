@echo off

call createpak.bat
"%PROGRAMFILES%\7-Zip\7z.exe" a -tzip fwclient.zip ..\bin\fw.exe ..\bin\d3dcompiler_47.dll *.pak config.xml ..\README.md ..\LICENSE.txt ..\..\CREDITS.md
