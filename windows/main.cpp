/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

CAppModule _Module;

#ifdef _DEBUG
CriticalSection cs;
enum { DEBUG_BUFSIZE = 2048 };
char buf[DEBUG_BUFSIZE];

LONG __stdcall DCUnhandledExceptionFilter( LPEXCEPTION_POINTERS e )
{
	Lock l(cs);
	
	File f(Util::getAppPath() + "exceptioninfo.txt", File::WRITE, File::OPEN | File::CREATE);
	f.setEndPos(0);
	
	DWORD exceptionCode = e->ExceptionRecord->ExceptionCode ;

	sprintf(buf, "\r\nUnhandled Exception\r\n  Code: %x\r\n", exceptionCode ) ;

	f.write(buf);
	STACKTRACE2(f, e->ContextRecord->Eip, e->ContextRecord->Esp, e->ContextRecord->Ebp);
	f.close();
	
	return EXCEPTION_CONTINUE_SEARCH;
}

#endif

void callBack(void* x, const string& a) {
	::SetWindowText((HWND)x, (STRING(LOADING) + "(" + a + ")").c_str());
	::RedrawWindow((HWND)x, NULL, NULL, RDW_UPDATENOW);
}

static void sendCmdLine(HWND hOther, LPTSTR lpstrCmdLine)
{
	string cmdLine = _T(lpstrCmdLine);
	LRESULT result;

	COPYDATASTRUCT cpd;
	cpd.dwData = 0;
	cpd.cbData = cmdLine.length() + 1;
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

static void installUrlHandler() {
	HKEY hk; 
	char Buf[512];
	string app = "\"" + Util::getAppName() + "\" %1";
	Buf[0] = 0;

	if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, "dchub\\Shell\\Open\\Command", 0, KEY_WRITE | KEY_READ, &hk) == ERROR_SUCCESS) {
		DWORD bufLen = sizeof(Buf);
		DWORD type;
		::RegQueryValueEx(hk, NULL, 0, &type, (LPBYTE)Buf, &bufLen);
		::RegCloseKey(hk);
	}

	if(Util::stricmp(app.c_str(), Buf) != 0) {
		::RegCreateKey(HKEY_CLASSES_ROOT, "dchub", &hk);
		char* tmp = "URL:Direct Connect Protocol";
		::RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)tmp, strlen(tmp) + 1);
		::RegSetValueEx(hk, "URL Protocol", 0, REG_SZ, (LPBYTE)"", 1);
		::RegCloseKey(hk);

		::RegCreateKey(HKEY_CLASSES_ROOT, "dchub\\Shell\\Open\\Command", &hk);
		::RegSetValueEx(hk, "", 0, REG_SZ, (LPBYTE)app.c_str(), app.length() + 1);
		::RegCloseKey(hk); 

		::RegCreateKey(HKEY_CLASSES_ROOT, "dchub\\DefaultIcon", &hk);
		app = Util::getAppName();
		::RegSetValueEx(hk, "", 0, REG_SZ, (LPBYTE)app.c_str(), app.length() + 1);
		::RegCloseKey(hk);
	}
} 

static int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);
	
	MainFrame wndMain;
	CEdit splash;
	RECT rc;
	rc.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	rc.right = GetSystemMetrics(SM_CXFULLSCREEN);
	rc.left = (rc.right / 2) - 150;
	rc.right = (rc.right / 2) + 150;
	rc.top = (rc.bottom / 2) - 12;
	rc.bottom = (rc.bottom / 2) + 12;

	splash.Create(NULL, splash.rcDefault, APPNAME " " VERSIONSTRING, WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_CENTER | ES_READONLY, WS_EX_STATICEDGE | WS_EX_TOPMOST);
	splash.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	
	HDC dc = splash.GetDC();
	rc.bottom = rc.top + WinUtil::getFontHeight(dc, splash.GetFont()) + 4;
	splash.ReleaseDC(dc);
	splash.HideCaret();
	splash.SetWindowPos(HWND_TOPMOST, &rc, SWP_SHOWWINDOW);
	splash.SetWindowText("Loading DC++, please wait...");
	splash.RedrawWindow();

	startup(callBack, (void*)splash.m_hWnd);

	splash.DestroyWindow();
	
	SettingsManager::getInstance()->setDefault(SettingsManager::BACKGROUND_COLOR, (int)(GetSysColor(COLOR_WINDOW)));
	SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_COLOR, (int)(GetSysColor(COLOR_WINDOWTEXT)));
	
	if(BOOLSETTING(URL_HANDLER)) {
		installUrlHandler();
	}

	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}
	
	wndMain.ShowWindow(nCmdShow);
	
	int nRet = theLoop.Run();
	
	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	SingleInstance dcapp("{DCPLUSPLUS-AEE8350A-B49A-4753-AB4B-E55479A48351}");

	if(dcapp.IsAnotherInstanceRunning()) {
		HWND hOther = NULL;
		EnumWindows(searchOtherInstance, (LPARAM)&hOther);

		if( hOther != NULL ) {
			// pop up
			::SetForegroundWindow(hOther);

			if( IsIconic(hOther)) {
				// restore
				::ShowWindow(hOther, SW_RESTORE);
			}
			sendCmdLine(hOther, lpstrCmdLine);
		}

		return FALSE;
	}

	HRESULT hRes = ::CoInitialize(NULL);
	// If you are running on NT 4.0 or higher you can use the following call instead to 
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	//	ATLASSERT(SUCCEEDED(hRes));
#ifdef _DEBUG
	EXTENDEDTRACEINITIALIZE( Util::getAppPath().c_str() );
	SetUnhandledExceptionFilter(&DCUnhandledExceptionFilter);
	File::deleteFile(Util::getAppPath() + "exceptioninfo.txt");
#endif

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	
	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);
	
	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES |
		ICC_UPDOWN_CLASS);	// add flags to support other controls
	
	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));
	
	
	int nRet = Run(lpstrCmdLine, nCmdShow);
	
	_Module.Term();
	::CoUninitialize();
#ifdef _DEBUG
	EXTENDEDTRACEUNINITIALIZE();
#endif
	return nRet;
}

/**
 * @file main.cpp
 * $Id: main.cpp,v 1.8 2002/06/18 19:06:35 arnetheduck Exp $
 */
