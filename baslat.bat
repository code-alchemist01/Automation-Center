@echo off
title Sistem Otomasyon Merkezi
echo.
echo ========================================
echo   Sistem Otomasyon Merkezi Başlatılıyor
echo ========================================
echo.
echo UTF-8 kodlama ayarlanıyor...
chcp 65001 > nul
echo Kodlama ayarlandı: UTF-8 (65001)
echo.
echo Program başlatılıyor...
echo.
automation_center.exe
echo.
echo Program sonlandı.
pause