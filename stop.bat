@echo off

taskkill /IM abserv.exe /F
taskkill /IM abmatch.exe /F
taskkill /IM ablogin.exe /F
timeout /T 1 /nobreak >nul
taskkill /IM abfile.exe /F
timeout /T 1 /nobreak >nul
taskkill /IM abmsgs.exe /F
timeout /T 1 /nobreak >nul
taskkill /IM abdata.exe /F
