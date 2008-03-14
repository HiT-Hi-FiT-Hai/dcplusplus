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
  File "mingwm10.dll"
  File "magnet.exe"
  SetOutPath "$INSTDIR\locale\ar\LC_MESSAGES\"
File "locale\ar\LC_MESSAGES\dcpp-win32.mo"
File "locale\ar\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\bg\LC_MESSAGES\"
File "locale\bg\LC_MESSAGES\dcpp-win32.mo"
File "locale\bg\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\bs\LC_MESSAGES\"
File "locale\bs\LC_MESSAGES\dcpp-win32.mo"
File "locale\bs\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\ca\LC_MESSAGES\"
File "locale\ca\LC_MESSAGES\dcpp-win32.mo"
File "locale\ca\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\cs\LC_MESSAGES\"
File "locale\cs\LC_MESSAGES\dcpp-win32.mo"
File "locale\cs\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\da\LC_MESSAGES\"
File "locale\da\LC_MESSAGES\dcpp-win32.mo"
File "locale\da\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\de\LC_MESSAGES\"
File "locale\de\LC_MESSAGES\dcpp-win32.mo"
File "locale\de\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\el\LC_MESSAGES\"
File "locale\el\LC_MESSAGES\dcpp-win32.mo"
File "locale\el\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\en_GB\LC_MESSAGES\"
File "locale\en_GB\LC_MESSAGES\dcpp-win32.mo"
File "locale\en_GB\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\es\LC_MESSAGES\"
File "locale\es\LC_MESSAGES\dcpp-win32.mo"
File "locale\es\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\fi\LC_MESSAGES\"
File "locale\fi\LC_MESSAGES\dcpp-win32.mo"
File "locale\fi\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\fr\LC_MESSAGES\"
File "locale\fr\LC_MESSAGES\dcpp-win32.mo"
File "locale\fr\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\gl\LC_MESSAGES\"
File "locale\gl\LC_MESSAGES\dcpp-win32.mo"
File "locale\gl\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\he\LC_MESSAGES\"
File "locale\he\LC_MESSAGES\dcpp-win32.mo"
File "locale\he\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\hr\LC_MESSAGES\"
File "locale\hr\LC_MESSAGES\dcpp-win32.mo"
File "locale\hr\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\hu\LC_MESSAGES\"
File "locale\hu\LC_MESSAGES\dcpp-win32.mo"
File "locale\hu\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\id\LC_MESSAGES\"
File "locale\id\LC_MESSAGES\dcpp-win32.mo"
File "locale\id\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\is\LC_MESSAGES\"
File "locale\is\LC_MESSAGES\dcpp-win32.mo"
File "locale\is\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\it\LC_MESSAGES\"
File "locale\it\LC_MESSAGES\dcpp-win32.mo"
File "locale\it\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\ja\LC_MESSAGES\"
File "locale\ja\LC_MESSAGES\dcpp-win32.mo"
File "locale\ja\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\km\LC_MESSAGES\"
File "locale\km\LC_MESSAGES\dcpp-win32.mo"
File "locale\km\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\ko\LC_MESSAGES\"
File "locale\ko\LC_MESSAGES\dcpp-win32.mo"
File "locale\ko\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\lt\LC_MESSAGES\"
File "locale\lt\LC_MESSAGES\dcpp-win32.mo"
File "locale\lt\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\lv\LC_MESSAGES\"
File "locale\lv\LC_MESSAGES\dcpp-win32.mo"
File "locale\lv\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\mk\LC_MESSAGES\"
File "locale\mk\LC_MESSAGES\dcpp-win32.mo"
File "locale\mk\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\ms\LC_MESSAGES\"

File "locale\ms\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\nb\LC_MESSAGES\"
File "locale\nb\LC_MESSAGES\dcpp-win32.mo"
File "locale\nb\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\nl\LC_MESSAGES\"
File "locale\nl\LC_MESSAGES\dcpp-win32.mo"
File "locale\nl\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\pl\LC_MESSAGES\"
File "locale\pl\LC_MESSAGES\dcpp-win32.mo"
File "locale\pl\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\pt\LC_MESSAGES\"
File "locale\pt\LC_MESSAGES\dcpp-win32.mo"
File "locale\pt\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\pt_BR\LC_MESSAGES\"
File "locale\pt_BR\LC_MESSAGES\dcpp-win32.mo"
File "locale\pt_BR\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\ro\LC_MESSAGES\"
File "locale\ro\LC_MESSAGES\dcpp-win32.mo"
File "locale\ro\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\ru\LC_MESSAGES\"
File "locale\ru\LC_MESSAGES\dcpp-win32.mo"
File "locale\ru\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\sl\LC_MESSAGES\"
File "locale\sl\LC_MESSAGES\dcpp-win32.mo"
File "locale\sl\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\sq\LC_MESSAGES\"
File "locale\sq\LC_MESSAGES\dcpp-win32.mo"
File "locale\sq\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\sv\LC_MESSAGES\"
File "locale\sv\LC_MESSAGES\dcpp-win32.mo"
File "locale\sv\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\tr\LC_MESSAGES\"
File "locale\tr\LC_MESSAGES\dcpp-win32.mo"
File "locale\tr\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\uk\LC_MESSAGES\"
File "locale\uk\LC_MESSAGES\dcpp-win32.mo"
File "locale\uk\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\zh_CN\LC_MESSAGES\"
File "locale\zh_CN\LC_MESSAGES\dcpp-win32.mo"
File "locale\zh_CN\LC_MESSAGES\libdcpp.mo"
SetOutPath "$INSTDIR\locale\zh_TW\LC_MESSAGES\"
File "locale\zh_TW\LC_MESSAGES\dcpp-win32.mo"
File "locale\zh_TW\LC_MESSAGES\libdcpp.mo"

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
  Delete "$INSTDIR\mingwm10.dll"

 Delete "$INSTDIR\locale\ar\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\ar\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\bg\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\bg\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\bs\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\bs\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\ca\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\ca\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\cs\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\cs\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\da\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\da\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\de\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\de\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\el\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\el\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\en_GB\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\en_GB\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\es\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\es\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\fi\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\fi\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\fr\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\fr\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\gl\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\gl\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\he\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\he\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\hr\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\hr\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\hu\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\hu\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\id\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\id\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\is\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\is\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\it\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\it\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\ja\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\ja\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\km\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\km\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\ko\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\ko\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\lt\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\lt\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\lv\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\lv\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\mk\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\mk\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\ms\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\ms\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\nb\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\nb\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\nl\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\nl\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\pl\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\pl\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\pt\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\pt\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\pt_BR\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\pt_BR\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\ro\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\ro\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\ru\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\ru\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\sl\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\sl\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\sq\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\sq\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\sv\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\sv\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\tr\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\tr\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\uk\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\uk\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\zh_CN\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\zh_CN\LC_MESSAGES\libdcpp.mo"
Delete "$INSTDIR\locale\zh_TW\LC_MESSAGES\dcpp-win32.mo"
Delete "$INSTDIR\locale\zh_TW\LC_MESSAGES\libdcpp.mo"

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
