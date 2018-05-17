@echo off

"%PROGRAMFILES%\7-Zip\7z.exe" a -tzip abserv.zip abdata.exe abserv.exe abfile.exe server.crt server.csr server.key nssm.exe *.dll *.lua config\* data\* logs\*\.gitkeep ..\README.md
