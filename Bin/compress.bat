@echo off

if exist abserv.zip del abserv.zip

"%PROGRAMFILES%\7-Zip\7z.exe" a -tzip abserv.zip abdata.exe ablogin.exe abserv.exe absmngr.exe abfile.exe server.crt server.csr server.key nssm.exe *.dll *.lua config\* data\* file_root\* logs\*\.gitkeep ..\README.md
