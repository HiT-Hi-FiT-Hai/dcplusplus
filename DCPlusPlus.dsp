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
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /Gr /MT /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /FAs /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib comctl32.lib Ws2_32.lib wininet.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "DCPlusPlus - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /Gr /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib comctl32.lib Ws2_32.lib wininet.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "DCPlusPlus - Win32 Release"
# Name "DCPlusPlus - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\client\BitInputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\client\BufferedSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\client\ClientListener.cpp
# End Source File
# Begin Source File

SOURCE=.\client\CriticalSection.cpp
# End Source File
# Begin Source File

SOURCE=.\client\CryptoManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\DCClient.cpp
# End Source File
# Begin Source File

SOURCE=.\client\DCPlusPlus.cpp
# End Source File
# Begin Source File

SOURCE=.\DCPlusPlus.rc
# End Source File
# Begin Source File

SOURCE=.\client\DirectoryListing.cpp
# End Source File
# Begin Source File

SOURCE=.\client\DirectoryListingFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\client\DownloadManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\client\ExListViewCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\client\HttpConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\client\HubFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\client\HubManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\IncomingManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\client\ProtocolHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\client\PublicHubsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\client\ServerSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Settings.cpp
# End Source File
# Begin Source File

SOURCE=.\client\SettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\client\SimpleXML.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Socket.cpp
# End Source File
# Begin Source File

SOURCE=.\client\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\client\StringTokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\client\UploadManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\UserConnection.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\client\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\client\AtlCmdBar2.h
# End Source File
# Begin Source File

SOURCE=.\client\BitInputStream.h
# End Source File
# Begin Source File

SOURCE=.\client\BufferedSocket.h
# End Source File
# Begin Source File

SOURCE=.\client\ClientListener.h
# End Source File
# Begin Source File

SOURCE=.\client\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\client\CryptoManager.h
# End Source File
# Begin Source File

SOURCE=.\client\DCClient.h
# End Source File
# Begin Source File

SOURCE=.\client\DCPlusPlus.h
# End Source File
# Begin Source File

SOURCE=.\client\DirectoryListing.h
# End Source File
# Begin Source File

SOURCE=.\client\DirectoryListingFrm.h
# End Source File
# Begin Source File

SOURCE=.\client\DownloadManager.h
# End Source File
# Begin Source File

SOURCE=.\client\Exception.h
# End Source File
# Begin Source File

SOURCE=.\client\ExListViewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\client\HttpConnection.h
# End Source File
# Begin Source File

SOURCE=.\client\HubFrame.h
# End Source File
# Begin Source File

SOURCE=.\client\HubManager.h
# End Source File
# Begin Source File

SOURCE=.\client\IncomingManager.h
# End Source File
# Begin Source File

SOURCE=.\client\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\client\ProtocolHandler.h
# End Source File
# Begin Source File

SOURCE=.\client\PublicHubsDlg.h
# End Source File
# Begin Source File

SOURCE=.\client\resource.h
# End Source File
# Begin Source File

SOURCE=.\client\ServerSocket.h
# End Source File
# Begin Source File

SOURCE=.\client\Settings.h
# End Source File
# Begin Source File

SOURCE=.\client\SettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\client\SimpleXML.h
# End Source File
# Begin Source File

SOURCE=.\client\Socket.h
# End Source File
# Begin Source File

SOURCE=.\client\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\client\StringTokenizer.h
# End Source File
# Begin Source File

SOURCE=.\client\UploadManager.h
# End Source File
# Begin Source File

SOURCE=.\client\UserConnection.h
# End Source File
# Begin Source File

SOURCE=.\client\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\client\res\DCPlusPlus.ico
# End Source File
# Begin Source File

SOURCE=.\res\DCPlusPlus.ico
# End Source File
# Begin Source File

SOURCE=.\client\res\DCPlusPlusdoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\DCPlusPlusdoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\folders.bmp
# End Source File
# Begin Source File

SOURCE=.\res\idr_mdid.ico
# End Source File
# Begin Source File

SOURCE=.\res\toolbar.bmp
# End Source File
# End Group
# End Target
# End Project
