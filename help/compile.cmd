@echo off
REM generate changelog.html
gen_changelog.py

copy ..\res\users.bmp .
mkdir ..\build\help
hhc.exe DCPlusPlus.hhp

del users.bmp

pause
