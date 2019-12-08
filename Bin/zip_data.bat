@echo off

if exist data.zip del data.zip

"%PROGRAMFILES%\7-Zip\7z.exe" a -tzip data.zip  data/maps/** data/Models/**
