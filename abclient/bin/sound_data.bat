@echo off

if exist sound_data.7z del sound_data.7z

"%PROGRAMFILES%\7-Zip\7z.exe" a -t7z -mx sound_data.7z SoundData\**
