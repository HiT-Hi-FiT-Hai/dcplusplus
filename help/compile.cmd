@echo off
REM  this isn't pretty. - Todd
if exist "C:\Program Files\HTML Help Workshop\hhc.exe" goto compile
echo.
echo HTML Help Workshop not detected.  Please install it from:
echo http://msdn.microsoft.com/library/default.asp?url=/library/en-us/htmlhelp/html/hwMicrosoftHTMLHelpDownloads.asp
echo.
exit 1

:compile
"C:\Program Files\HTML Help Workshop\hhc.exe" DCPlusPlus.hhp
if errorlevel 1 goto okay

:bad
exit 1

:okay
exit 0
