; The name of the installer
Name "DC++"

ShowInstDetails show
ShowUninstDetails show

; The file to write
OutFile "DCPlusPlus.exe"

; The default installation directory
InstallDir $PROGRAMFILES\DC++
; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\DC++ "Install_Dir"

LicenseText "DC++ is licensed under the GPL, here's the full text!"
LicenseData "License.txt"

; The text to prompt the user to enter a directory
ComponentText "Welcome to the DC++ installer."
; The text to prompt the user to enter a directory
DirText "Choose a directory to install in to:"

; The stuff to install
Section "DC++ (required)"
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  File "/oname=DCPlusPlus.exe" "App\DCPlusPlus.exe"
  File "Readme.txt"
  File "ChangeLog.txt"
  File "Example.xml"
  File "License.txt"
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\DC++ "Install_Dir" "$INSTDIR"
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "DisplayName" "DC++ (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"
SectionEnd

Section "Debug Information (recommended, helps finding bugs)"
  SetOutPath $INSTDIR
  File "/oname=DCPlusPlus.pdb" "App\DCPlusPlus.pdb"
  File "dbghelp.dll"
SectionEnd

; optional section
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\DC++"
  CreateShortCut "$SMPROGRAMS\DC++\DC++.lnk" "$INSTDIR\DCPlusPlus.exe" "" "$INSTDIR\DCPlusPlus.exe" 0 "" "" "DC++ File Sharing Application"
  CreateShortCut "$SMPROGRAMS\DC++\Readme.lnk" "$INSTDIR\ReadMe.txt"
  CreateShortCut "$SMPROGRAMS\DC++\License.lnk" "$INSTDIR\License.txt"
  CreateShortCut "$SMPROGRAMS\DC++\Change Log.lnk" "$INSTDIR\ChangeLog.txt"
  CreateShortCut "$SMPROGRAMS\DC++\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

; uninstall stuff

UninstallText "This will uninstall DC++. Hit next to continue."

; special uninstall section.
Section "Uninstall"
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++"
  DeleteRegKey HKLM SOFTWARE\DC++
  ; remove files
  Delete "$INSTDIR\DCPlusPlus.exe"
  Delete "$INSTDIR\dbghelp.dll"
  Delete "$INSTDIR\DCPlusPlus.pdb"
  Delete "$INSTDIR\License.txt"
  Delete "$INSTDIR\ChangeLog.txt"
  Delete "$INSTDIR\ReadMe.txt"
  Delete "$INSTDIR\Example.xml"

  ; MUST REMOVE UNINSTALLER, too
  Delete $INSTDIR\uninstall.exe
  ; remove shortcuts, if any.
  Delete "$SMPROGRAMS\DC++\*.*"
  ; remove directories used.
  RMDir "$SMPROGRAMS\DC++"
  RMDir "$INSTDIR"
SectionEnd

; eof
