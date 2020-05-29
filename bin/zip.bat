@echo off

if exist abserv.zip del abserv.zip

"%PROGRAMFILES%\7-Zip\7z.exe" a -tzip abserv.zip abdata.exe abmsgs.exe abmatch.exe ablogin.exe abserv.exe abfile.exe ablb.exe absadmin.exe keygen.exe dbtool.exe server.crt server.csr server.key abserver.dh 00run.bat *.dll *.lua config\* data\* file_root\* admin\README.md admin\templates\* admin\root\*.ico admin\root\css\* admin\root\fonts\* admin\root\images\* admin\root\js\* admin\root\files\.gitkeep logs\*\.gitkeep recordings\.gitkeep ..\README.md

rem Not added to the ZIP archive:
rem - admin\root\vendors
