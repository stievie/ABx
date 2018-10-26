@echo off

if exist abserv.zip del abserv.zip

"%PROGRAMFILES%\7-Zip\7z.exe" a -tzip abserv.zip abdata.exe abmsgs.exe ablogin.exe abserv.exe absmngr.exe abfile.exe ablb.exe absadmin.exe server.crt server.csr server.key abserver.dh nssm.exe 00run.bat *.dll *.lua config\* data\* file_root\* logs\*\.gitkeep recordings\.gitkeep ..\README.md
