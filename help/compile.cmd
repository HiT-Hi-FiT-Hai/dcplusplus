@echo off
REM generate changelog.html
gen_changelog.py
REM  this isn't pretty. - Todd
if exist "%ProgramFiles%\HTML Help Workshop\hhc.exe" goto compile
echo.
echo HTML Help Workshop not detected.  Please install it from:
echo http://msdn.microsoft.com/library/default.asp?url=/library/en-us/htmlhelp/html/hwMicrosoftHTMLHelpDownloads.asp
echo.
echo or.. enter the correct path in help/compile.cmd
echo.
exit 1

:compile
"%ProgramFiles%\HTML Help Workshop\hhc.exe" DCPlusPlus.hhp
if errorlevel 1 goto okay

:bad
exit 1

:okay
exit 0
