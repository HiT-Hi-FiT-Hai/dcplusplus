 /*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdafx.h"
#include <client/DCPlusPlus.h>

#include "SingleInstance.h"
#include "WinUtil.h"
#include "MainWindow.h"
#include "SplashWindow.h"

#include <client/MerkleTree.h>
#include <client/File.h>
#include <client/Text.h>

#ifdef PORT_ME

#include "Resource.h"
#include "ExtendedTrace.h"

#include "MainFrm.h"

#include <delayimp.h>
CAppModule _Module;

CriticalSection cs;
enum { DEBUG_BUFSIZE = 8192 };
static char guard[DEBUG_BUFSIZE];
static int recursion = 0;
static bool firstException = true;

static char buf[DEBUG_BUFSIZE];

#ifndef _DEBUG

FARPROC WINAPI FailHook(unsigned /* dliNotify */, PDelayLoadInfo /* pdli */) {
	MessageBox(WinUtil::mainWnd, _T("DC++ just encountered an unhandled exception and will terminate. Please do not report this as a bug, as DC++ was unable to collect the information needed for a useful bug report (Your Operating System doesn't support the functionality needed, probably because it's too old)."), _T("DC++ Has Crashed"), MB_OK | MB_ICONERROR);
	exit(-1);
}

#endif

#include "../client/SSLSocket.h"

LONG __stdcall DCUnhandledExceptionFilter( LPEXCEPTION_POINTERS e )
{
	Lock l(cs);

	if(recursion++ > 30)
		exit(-1);

#ifndef _DEBUG
#if _MSC_VER == 1200
	__pfnDliFailureHook = FailHook;
#elif _MSC_VER == 1300 || _MSC_VER == 1310 || _MSC_VER == 1400
	__pfnDliFailureHook2 = FailHook;
#else
#error Unknown Compiler version
#endif

	// The release version loads the dll and pdb:s here...
	EXTENDEDTRACEINITIALIZE( Util::getDataPath().c_str() );

#endif

	if(firstException) {
		File::deleteFile(Util::getConfigPath() + "exceptioninfo.txt");
		firstException = false;
	}

	if(File::getSize(Util::getDataPath() + "DCPlusPlus.pdb") == -1) {
		// No debug symbols, we're not interested...
		::MessageBox(WinUtil::mainWnd, _T("DC++ has crashed and you don't have debug symbols installed. Hence, I can't find out why it crashed, so don't report this as a bug unless you find a solution..."), _T("DC++ has crashed"), MB_OK);
#ifndef _DEBUG
		exit(1);
#else
		return EXCEPTION_CONTINUE_SEARCH;
#endif
	}

	File f(Util::getConfigPath() + "exceptioninfo.txt", File::WRITE, File::OPEN | File::CREATE);
	f.setEndPos(0);

	DWORD exceptionCode = e->ExceptionRecord->ExceptionCode ;

	sprintf(buf, "Code: %x\r\nVersion: %s\r\n",
		exceptionCode, VERSIONSTRING);

	f.write(buf, strlen(buf));

	OSVERSIONINFOEX ver;
	WinUtil::getVersionInfo(ver);

	sprintf(buf, "Major: %d\r\nMinor: %d\r\nBuild: %d\r\nSP: %d\r\nType: %d\r\n",
		(DWORD)ver.dwMajorVersion, (DWORD)ver.dwMinorVersion, (DWORD)ver.dwBuildNumber,
		(DWORD)ver.wServicePackMajor, (DWORD)ver.wProductType);

	f.write(buf, strlen(buf));
	time_t now;
	time(&now);
	strftime(buf, DEBUG_BUFSIZE, "Time: %Y-%m-%d %H:%M:%S\r\n", localtime(&now));

	f.write(buf, strlen(buf));

	f.write(LIT("TTH: "));
	f.write(tth, strlen(tth));
	f.write(LIT("\r\n"));

	f.write(LIT("\r\n"));

	STACKTRACE2(f, e->ContextRecord->Eip, e->ContextRecord->Esp, e->ContextRecord->Ebp);

	f.write(LIT("\r\n"));

	f.close();

	if(MessageBox(WinUtil::mainWnd, _T("DC++ just encountered a fatal bug and should have written an exceptioninfo.txt the same directory as the executable. You can upload this file at http://dcplusplus.sf.net/crash/ to help us find out what happened (please do not report this bug in the bug tracker unless you know the exact steps to reproduce it...). Go there now?"), _T("DC++ Has Crashed"), MB_YESNO | MB_ICONERROR) == IDYES) {
		WinUtil::openLink(_T("http://dcplusplus.sf.net/crash/"));
	}

#ifndef _DEBUG
	EXTENDEDTRACEUNINITIALIZE();

	exit(-1);
#else
	return EXCEPTION_CONTINUE_SEARCH;
#endif
}

static void sendCmdLine(HWND hOther, LPTSTR lpstrCmdLine)
{
	tstring cmdLine = lpstrCmdLine;
	LRESULT result;

	COPYDATASTRUCT cpd;
	cpd.dwData = 0;
	cpd.cbData = sizeof(TCHAR)*(cmdLine.length() + 1);
	cpd.lpData = (void *)cmdLine.c_str();
	result = SendMessage(hOther, WM_COPYDATA, NULL,	(LPARAM)&cpd);
}

BOOL CALLBACK searchOtherInstance(HWND hWnd, LPARAM lParam) {
	DWORD result;
	LRESULT ok = ::SendMessageTimeout(hWnd, WMU_WHERE_ARE_YOU, 0, 0,
		SMTO_BLOCK | SMTO_ABORTIFHUNG, 5000, &result);
	if(ok == 0)
		return TRUE;
	if(result == WMU_WHERE_ARE_YOU) {
		// found it
		HWND *target = (HWND *)lParam;
		*target = hWnd;
		return FALSE;
	}
	return TRUE;
}

static int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	splash.DestroyWindow();
	dummy.DestroyWindow();

	if(ResourceManager::getInstance()->isRTL()) {
		SetProcessDefaultLayout(LAYOUT_RTL);
	}

	SettingsManager::getInstance()->setDefault(SettingsManager::BACKGROUND_COLOR, (int)(GetSysColor(COLOR_WINDOW)));
	SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_COLOR, (int)(GetSysColor(COLOR_WINDOWTEXT)));

	MainFrame wndMain;

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();

	shutdown();

	return nRet;
}
#endif

static void checkCommonControls() {
#define PACKVERSION(major,minor) MAKELONG(minor,major)

	HINSTANCE hinstDll;
	DWORD dwVersion = 0;

	hinstDll = LoadLibrary(_T("comctl32.dll"));

	if(hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;

		pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");

		if(pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;

			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			hr = (*pDllGetVersion)(&dvi);

			if(SUCCEEDED(hr))
			{
				dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			}
		}

		FreeLibrary(hinstDll);
	}

	if(dwVersion < PACKVERSION(5,80)) {
		MessageBox(NULL, _T("Your version of windows common controls is too old for DC++ to run correctly, and you will most probably experience problems with the user interface. You should download version 5.80 or higher from the DC++ homepage or from Microsoft directly."), _T("User Interface Warning"), MB_OK);
	}
}

bool checkOtherInstances() {
#ifndef _DEBUG
	SingleInstance dcapp(_T("{DCPLUSPLUS-AEE8350A-B49A-4753-AB4B-E55479A48351}"));
#else
	SingleInstance dcapp(_T("{DCPLUSPLUS-AEE8350A-B49A-4753-AB4B-E55479A48350}"));
#endif

#ifdef PORT_ME
	if(dcapp.isRunning()) {
		HWND hOther = NULL;
		EnumWindows(searchOtherInstance, (LPARAM)&hOther);

#ifndef _DEBUG
		if( hOther != NULL ) {
#else
		if( hOther != NULL && _tcslen(lpstrCmdLine) > 0 ) {
#endif
			// pop up
			::SetForegroundWindow(hOther);

			if( IsIconic(hOther)) {
				// restore
				::ShowWindow(hOther, SW_RESTORE);
			}
			sendCmdLine(hOther, lpstrCmdLine);
			return false;
		}
	}
#endif
	return true;
}

void callBack(void* ptr, const string& a) {
	SplashWindow& splash = *((SplashWindow*)ptr);
	splash(a);
}

int SmartWinMain(SmartWin::Application& app) {

#ifdef PORT_ME
#ifdef _DEBUG
	EXTENDEDTRACEINITIALIZE( Util::getDataPath().c_str() );
	//File::deleteFile(Util::getDataPath() + "exceptioninfo.txt");
#endif
	SetUnhandledExceptionFilter(&DCUnhandledExceptionFilter);
#endif

	if(!checkOtherInstances()) {
		return 1;
	}

	checkCommonControls();

	// For SHBrowseForFolder, UPnP
	/// @todo check return
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	try {
		std::string module = File(app.getModuleFileName(), File::READ, File::OPEN).read();
		TigerTree tth(TigerTree::calcBlockSize(module.size(), 1));
		tth.update(module.data(), module.size());
		tth.finalize();
		WinUtil::tth = Text::toT(tth.getRoot().toBase32());
	} catch(const FileException&) {
		dcdebug("Failed reading exe\n");
	}
	
	{
		SplashWindow* splash(new SplashWindow);
		startup(&callBack, splash);
		splash->close();
	}

	MainWindow* wnd(new MainWindow);
	int ret = app.run();

	shutdown();

	::CoUninitialize();
	
#ifdef PORT_ME
#ifdef _DEBUG
	EXTENDEDTRACEUNINITIALIZE();
#endif
#endif

	return ret;
}

