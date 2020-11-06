@echo off

if exist client_data.7z del client_data.7z

"%PROGRAMFILES%\7-Zip\7z.exe" a -t7z -mx client_data.7z AbData\** SoundData\**
