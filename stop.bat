@echo off

taskkill /IM abserv.exe
taskkill /IM abmatch.exe
taskkill /IM ablogin.exe
timeout /T 1 /nobreak >nul
taskkill /IM abfile.exe
timeout /T 1 /nobreak >nul
taskkill /IM abmsgs.exe
timeout /T 1 /nobreak >nul
taskkill /IM abdata.exe
