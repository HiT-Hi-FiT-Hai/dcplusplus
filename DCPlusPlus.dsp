# Microsoft Developer Studio Project File - Name="DCPlusPlus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DCPlusPlus - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DCPlusPlus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DCPlusPlus.mak" CFG="DCPlusPlus - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DCPlusPlus - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DCPlusPlus - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DCPlusPlus - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "App"
# PROP Intermediate_Dir "vc6\Release\windows"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /Gr /MT /W4 /Gm /GX /Zi /Og /Oi /Os /Oy /Ob2 /Gf /Gy /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_REENTRANT" /D "BZ_NO_STDIO" /FAs /Yu"stdafx.h" /Gs256 /FD /GF /Zm150 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib comctl32.lib Ws2_32.lib shlwapi.lib delayimp.lib vc6\Release\client\client.lib vc6\Release\bzip\bzip2.lib vc6\release\zlib\zlib.lib /nologo /version:0.201 /subsystem:windows /profile /map /debug /machine:I386 /delayload:imagehlp.dll /delayload:comdlg32.dll

!ELSEIF  "$(CFG)" == "DCPlusPlus - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "App"
# PROP Intermediate_Dir "vc6\Debug\windows"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W4 /Gm /Gi /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_REENTRANT" /D "BZ_NO_STDIO" /FR /Yu"stdafx.h" /FD /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib vc6\Debug\client\client.lib vc6\Debug\bzip\bzip2.lib vc6\Debug\zlib\zlib.lib /nologo /version:0.171 /subsystem:windows /pdb:"App/DCPlusPlus6.pdb" /debug /machine:I386
# SUBTRACT LINK32 /profile /pdb:none /map

!ENDIF 

# Begin Target

# Name "DCPlusPlus - Win32 Release"
# Name "DCPlusPlus - Win32 Debug"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\arrows.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\client\res\DCPlusPlus.ico
# End Source File
# Begin Source File

SOURCE=.\res\DCPlusPlus.ico
# End Source File
# Begin Source File

SOURCE=.\res\DCPlusPlus.Manifest
# End Source File
# Begin Source File

SOURCE=.\DCPlusPlus.rc
# End Source File
# Begin Source File

SOURCE=.\client\res\DCPlusPlusdoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\DCPlusPlusdoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Directory.ico
# End Source File
# Begin Source File

SOURCE=.\res\Favorites.ico
# End Source File
# Begin Source File

SOURCE=.\res\folders.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hub.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_mdid.ico
# End Source File
# Begin Source File

SOURCE=.\res\mdichild.bmp
# End Source File
# Begin Source File

SOURCE=.\res\notepad.ico
# End Source File
# Begin Source File

SOURCE=.\res\PublicHubs.ico
# End Source File
# Begin Source File

SOURCE=.\res\Queue.ico
# End Source File
# Begin Source File

SOURCE=.\res\Search.ico
# End Source File
# Begin Source File

SOURCE=.\res\Speeds.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar20.bmp
# End Source File
# Begin Source File

SOURCE=.\res\User.ico
# End Source File
# Begin Source File

SOURCE=.\res\users.bmp
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\windows\ADLSearchFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\ADLSProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\Advanced2Page.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\AdvancedPage.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\AppearancePage.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\DirectoryListingFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\DownloadPage.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\ExListViewCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\ExtendedTrace.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\FavHubProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\FavoritesFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\FinishedFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\FinishedULFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\GeneralPage.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\HubFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\main.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\NotepadFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\PrivateFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\PropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\PropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\PublicHubsFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\QueueFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\SearchFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\SpyFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\windows\TextFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\TransferView.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\TreePropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\UploadPage.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\UsersFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\WinUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\windows\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\windows\ADLSearchFrame.h
# End Source File
# Begin Source File

SOURCE=.\windows\ADLSProperties.h
# End Source File
# Begin Source File

SOURCE=.\windows\Advanced2Page.h
# End Source File
# Begin Source File

SOURCE=.\windows\AdvancedPage.h
# End Source File
# Begin Source File

SOURCE=.\windows\AppearancePage.h
# End Source File
# Begin Source File

SOURCE=.\windows\CommandDlg.h
# End Source File
# Begin Source File

SOURCE=.\windows\DirectoryListingFrm.h
# End Source File
# Begin Source File

SOURCE=.\windows\DownloadPage.h
# End Source File
# Begin Source File

SOURCE=.\windows\ExListViewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\windows\ExtendedTrace.h
# End Source File
# Begin Source File

SOURCE=.\windows\FavHubProperties.h
# End Source File
# Begin Source File

SOURCE=.\windows\FavoritesFrm.h
# End Source File
# Begin Source File

SOURCE=.\windows\FinishedFrame.h
# End Source File
# Begin Source File

SOURCE=.\windows\FinishedULFrame.h
# End Source File
# Begin Source File

SOURCE=.\windows\FlatTabCtrl.h
# End Source File
# Begin Source File

SOURCE=.\windows\GeneralPage.h
# End Source File
# Begin Source File

SOURCE=.\windows\HubFrame.h
# End Source File
# Begin Source File

SOURCE=.\windows\LineDlg.h
# End Source File
# Begin Source File

SOURCE=.\windows\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\windows\NotepadFrame.h
# End Source File
# Begin Source File

SOURCE=.\windows\PrivateFrame.h
# End Source File
# Begin Source File

SOURCE=.\windows\PropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\windows\PropPage.h
# End Source File
# Begin Source File

SOURCE=.\windows\PublicHubsFrm.h
# End Source File
# Begin Source File

SOURCE=.\windows\QueueFrame.h
# End Source File
# Begin Source File

SOURCE=.\windows\resource.h
# End Source File
# Begin Source File

SOURCE=.\windows\SearchFrm.h
# End Source File
# Begin Source File

SOURCE=.\windows\SingleInstance.h
# End Source File
# Begin Source File

SOURCE=.\windows\SpyFrame.h
# End Source File
# Begin Source File

SOURCE=.\windows\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\windows\TextFrame.h
# End Source File
# Begin Source File

SOURCE=.\windows\TransferView.h
# End Source File
# Begin Source File

SOURCE=.\windows\TreePropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\windows\UploadPage.h
# End Source File
# Begin Source File

SOURCE=.\windows\UsersFrame.h
# End Source File
# Begin Source File

SOURCE=.\windows\WinUtil.h
# End Source File
# End Group
# End Target
# End Project
