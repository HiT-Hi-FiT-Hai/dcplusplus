Function GetDCPlusPlusVersion
        Exch $0
	GetDllVersion "$INSTDIR\$0" $R0 $R1
	IntOp $R2 $R0 / 0x00010000
	IntOp $R3 $R0 & 0x0000FFFF
	IntOp $R4 $R1 / 0x00010000
	IntOp $R5 $R1 & 0x0000FFFF
        StrCpy $1 "$R2.$R3$R4$R5"
        Exch $1
FunctionEnd

SetCompressor "lzma"

; The name of the installer
Name "DC++"

ShowInstDetails show
ShowUninstDetails show

Page license
Page components
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

; The file to write
OutFile "DCPlusPlus-xxx.exe"

; The default installation directory
InstallDir $PROGRAMFILES\DC++
; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\DC++ "Install_Dir"

LicenseText "DC++ is licensed under the GPL, here's the full text!"
LicenseData "License.txt"
LicenseForceSelection checkbox

; The text to prompt the user to enter a directory
ComponentText "Welcome to the DC++ installer."
; The text to prompt the user to enter a directory
DirText "Choose a directory to install in to:"

; The stuff to install
Section "DC++ (required)"
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  IfFileExists "$INSTDIR\*.xml" 0 no_backup
  MessageBox MB_YESNO|MB_ICONQUESTION "A previous installation of DC++ has been found, backup settings and queue? (You can find it in $INSTDIR\BACKUP later)" IDNO no_backup
  CreateDirectory "$INSTDIR\BACKUP\"
  CopyFiles "$INSTDIR\*.xml" "$INSTDIR\BACKUP\"

no_backup:
  ; Put file there
  File "changelog.txt"
  File "dcppboot.xml"
  File "DCPlusPlus.chm"
  File "DCPlusPlus.exe"
  File "Example.xml"
  File "License.txt"
  File "LICENSE-GeoIP.txt"
  File "LICENSE-OpenSSL.txt"
  File "magnet.exe"
  ; Remove opencow just in case we're upgrading
  Delete "$INSTDIR\opencow.dll"
  
  ; Get DCPlusplus version we just installed and store in $1
  Push "DCPlusPlus.exe"
  Call "GetDCPlusPlusVersion"
  Pop $1

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\DC++ "Install_Dir" "$INSTDIR"
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "DisplayIcon" '"$INSTDIR\DCPlusPlus.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "DisplayName" "DC++ $1"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "DisplayVersion" "$1"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "Publisher" "Jacek Sieka"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "URLInfoAbout" "http://dcplusplus.sourceforge.net/"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "URLUpdateInfo" "http://dcplusplus.sourceforge.net/download/"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "HelpLink" "http://dcpp.net/forum/"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++" "NoRepair" "1"
  
  WriteUninstaller "uninstall.exe"
SectionEnd

Section "IP -> Country mappings"
  SetOutPath $INSTDIR
  File "GeoIPCountryWhois.csv"
SectionEnd

; optional section
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\DC++"
  CreateShortCut "$SMPROGRAMS\DC++\DC++.lnk" "$INSTDIR\DCPlusPlus.exe" "" "$INSTDIR\DCPlusPlus.exe" 0 "" "" "DC++ File Sharing Application"
  CreateShortCut "$SMPROGRAMS\DC++\License.lnk" "$INSTDIR\License.txt"
  CreateShortCut "$SMPROGRAMS\DC++\Help.lnk" "$INSTDIR\DCPlusPlus.chm"
  CreateShortCut "$SMPROGRAMS\DC++\Change Log.lnk" "$INSTDIR\ChangeLog.txt"
  CreateShortCut "$SMPROGRAMS\DC++\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

; uninstall stuff

UninstallText "This will uninstall DC++. Hit next to continue."

; special uninstall section.
Section "un.Uninstall"
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DC++"
  DeleteRegKey HKLM SOFTWARE\DC++
  ; remove files
  Delete "$INSTDIR\DCPlusPlus.exe"
  Delete "$INSTDIR\DCPlusPlus.chm"
  Delete "$INSTDIR\dcppboot.xml"
  Delete "$INSTDIR\License-GeoIP.txt"
  Delete "$INSTDIR\License.txt"
  Delete "$INSTDIR\ChangeLog.txt"
  Delete "$INSTDIR\LICENSE-OpenSSL.tx"
  Delete "$INSTDIR\Example.xml"
  Delete "$INSTDIR\Magnet.exe"
  Delete "$INSTDIR\GeoIPCountryWhois.csv"

  ; Remove registry entries
  ;  dchub is likely only to be registered to us
  ;  magnet is likely to be registere to other p2p apps
  DeleteRegKey HKCR "dchub"
  DeleteRegKey HKCR "adc"
  DeleteRegKey HKLM "SOFTWARE\Magnet\Handlers\DC++"
  ; MUST REMOVE UNINSTALLER, too
  Delete $INSTDIR\uninstall.exe
  ; remove shortcuts, if any.
  Delete "$SMPROGRAMS\DC++\*.*"
  ; remove directories used.
  RMDir "$SMPROGRAMS\DC++"

  MessageBox MB_YESNO|MB_ICONQUESTION "Also remove queue and settings?" IDYES kill_dir

  RMDir "$INSTDIR"
  goto end_uninstall
kill_dir:
  RMDir /r "$INSTDIR"
end_uninstall:

SectionEnd

; eof
