@echo off
REM generate changelog.html
gen_changelog.py

copy ..\res\users.bmp .
mkdir ..\build\help
hhc.exe DCPlusPlus.hhp > compile.log

del users.bmp
