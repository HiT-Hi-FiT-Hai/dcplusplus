/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "MainFrm.h"
#include "ExtendedTrace.h"
#include "WinUtil.h"
#include "SingleInstance.h"

#include "../client/MerkleTree.h"

#include <delayimp.h>
CAppModule _Module;

CriticalSection cs;
enum { DEBUG_BUFSIZE = 8192 };
static char guard[DEBUG_BUFSIZE];
static int recursion = 0;
static char tth[192*8/(5*8)+2];
static bool firstException = true;

static char buf[DEBUG_BUFSIZE];

#ifndef _DEBUG

FARPROC WINAPI FailHook(unsigned /* dliNotify */, PDelayLoadInfo  /* pdli */) {
	MessageBox(WinUtil::mainWnd, _T("DC++ just encountered an unhandled exception and will terminate. Please do not report this as a bug, as DC++ was unable to collect the information needed for a useful bug report (Your Operating System doesn't support the functionality needed, probably because it's too old)."), _T("DC++ Has Crashed"), MB_OK | MB_ICONERROR);
	exit(-1);
}

#endif

LONG __stdcall DCUnhandledExceptionFilter( LPEXCEPTION_POINTERS e )
{
	Lock l(cs);

	if(recursion++ > 30)
		exit(-1);

#ifndef _DEBUG
#if _MSC_VER == 1200
	__pfnDliFailureHook = FailHook;
#elif _MSC_VER == 1300 || _MSC_VER == 1310
	__pfnDliFailureHook2 = FailHook;
#else
#error Unknown Compiler version
#endif

	// The release version loads the dll and pdb:s here...
	EXTENDEDTRACEINITIALIZE( Util::getAppPath().c_str() );

#endif

	if(firstException) {
		File::deleteFile(Util::getAppPath() + "exceptioninfo.txt");
		firstException = false;
	}

	if(File::getSize(Util::getAppPath() + "DCPlusPlus.pdb") == -1) {
		// No debug symbols, we're not interested...
		::MessageBox(WinUtil::mainWnd, _T("DC++ has crashed and you don't have debug symbols installed. Hence, I can't find out why it crashed, so don't report this as a bug unless you find a solution..."), _T("DC++ has crashed"), MB_OK);
#ifndef _DEBUG
		exit(1);
#else
		return EXCEPTION_CONTINUE_SEARCH;
#endif
	}

	File f(Util::getAppPath() + "exceptioninfo.txt", File::WRITE, File::OPEN | File::CREATE);
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

void callBack(void* x, const string& a) {
	::SetWindowText((HWND)x, Text::toT(STRING(LOADING) + "(" + a + ")").c_str());
	::RedrawWindow((HWND)x, NULL, NULL, RDW_UPDATENOW);
}

static int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	checkCommonControls();

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);
	
	MainFrame wndMain;

	CEdit dummy;
	CEdit splash;

	CRect rc;
	rc.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	rc.top = (rc.bottom / 2) - 20;

	rc.right = GetSystemMetrics(SM_CXFULLSCREEN);
	rc.left = rc.right / 2 - 150;
	rc.right = rc.left + 300;

	dummy.Create(NULL, rc, _T(APPNAME) _T(" ") _T(VERSIONSTRING), WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_CENTER | ES_READONLY, WS_EX_STATICEDGE);
	splash.Create(NULL, rc, _T(APPNAME) _T(" ") _T(VERSIONSTRING), WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_CENTER | ES_READONLY, WS_EX_STATICEDGE);
	splash.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	
	rc.bottom = rc.top + WinUtil::getTextHeight(splash.m_hWnd, splash.GetFont()) + 4;
	splash.HideCaret();
	splash.SetWindowPos(HWND_TOPMOST, &rc, SWP_SHOWWINDOW);
	splash.SetFocus();
	splash.RedrawWindow();

	startup(callBack, (void*)splash.m_hWnd);

	splash.DestroyWindow();
	dummy.DestroyWindow();

	SettingsManager::getInstance()->setDefault(SettingsManager::BACKGROUND_COLOR, (int)(GetSysColor(COLOR_WINDOW)));
	SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_COLOR, (int)(GetSysColor(COLOR_WINDOWTEXT)));
	
	rc = wndMain.rcDefault;

	if( (SETTING(MAIN_WINDOW_POS_X) != CW_USEDEFAULT) &&
		(SETTING(MAIN_WINDOW_POS_Y) != CW_USEDEFAULT) &&
		(SETTING(MAIN_WINDOW_SIZE_X) != CW_USEDEFAULT) &&
		(SETTING(MAIN_WINDOW_SIZE_Y) != CW_USEDEFAULT) ) {

		rc.left = SETTING(MAIN_WINDOW_POS_X);
		rc.top = SETTING(MAIN_WINDOW_POS_Y);
		rc.right = rc.left + SETTING(MAIN_WINDOW_SIZE_X);
		rc.bottom = rc.top + SETTING(MAIN_WINDOW_SIZE_Y);
		// Now, let's ensure we have sane values here...
		if( (rc.left < 0 ) || (rc.top < 0) || (rc.right - rc.left < 10) || ((rc.bottom - rc.top) < 10) ) {
			rc = wndMain.rcDefault;
		}
	}

	if(wndMain.CreateEx(NULL, rc) == NULL) {
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}
	
	wndMain.ShowWindow(((nCmdShow == SW_SHOWDEFAULT) || (nCmdShow == SW_SHOWNORMAL)) ? SETTING(MAIN_WINDOW_STATE) : nCmdShow);
	
	int nRet = theLoop.Run();
	
	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
#ifndef _DEBUG
	SingleInstance dcapp(_T("{DCPLUSPLUS-AEE8350A-B49A-4753-AB4B-E55479A48351}"));
#else
	SingleInstance dcapp(_T("{DCPLUSPLUS-AEE8350A-B49A-4753-AB4B-E55479A48350}"));
#endif

	if(dcapp.IsAnotherInstanceRunning()) {
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
			return FALSE;
		}
	}
	
	// For SHBrowseForFolder, UPnP
	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); 
#ifdef _DEBUG
	EXTENDEDTRACEINITIALIZE( Util::getAppPath().c_str() );
	//File::deleteFile(Util::getAppPath() + "exceptioninfo.txt");
#endif
	SetUnhandledExceptionFilter(&DCUnhandledExceptionFilter);
	
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	
	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES |
		ICC_TAB_CLASSES | ICC_UPDOWN_CLASS);	// add flags to support other controls
	
	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	try {
		File f(Util::getAppName(), File::READ, File::OPEN);
		TigerTree tth(TigerTree::calcBlockSize(f.getSize(), 1));
		size_t n = 0;
		size_t n2 = DEBUG_BUFSIZE;
		while( (n = f.read(buf, n2)) > 0) {
			tth.update(buf, n);
			n2 = DEBUG_BUFSIZE;
		}
		tth.finalize();
		strcpy(::tth, tth.getRoot().toBase32().c_str());
		WinUtil::tth = Text::toT(::tth);
	} catch(const FileException&) {
		dcdebug("Failed reading exe\n");
	}
	
	int nRet = Run(lpstrCmdLine, nCmdShow);
	
	_Module.Term();
	::CoUninitialize();
	::WSACleanup();
#ifdef _DEBUG
	EXTENDEDTRACEUNINITIALIZE();
#endif
	return nRet;
}

/**
 * @file
 * $Id: main.cpp,v 1.36 2005/04/10 21:23:27 arnetheduck Exp $
 */
