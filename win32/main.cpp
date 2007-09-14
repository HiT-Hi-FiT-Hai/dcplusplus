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

#include "SingleInstance.h"
#include "WinUtil.h"
#include "MainWindow.h"
#include "SplashWindow.h"

#include <dcpp/MerkleTree.h>
#include <dcpp/File.h>
#include <dcpp/Text.h>

#ifdef PORT_ME

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

#ifdef _DEBUG
void (*old_handler)();

// Dummy function to have something to break at
void term_handler() {
	old_handler();
}
#endif

int SmartWinMain(SmartWin::Application& app) {
	dcdebug("StartWinMain\n");

	if(!checkOtherInstances()) {
		return 1;
	}

#ifdef _DEBUG
	old_handler = set_terminate(&term_handler);
#endif
	checkCommonControls();

	// For debugging
	::LoadLibrary(_T("exchndl.dll"));
	
	// For SHBrowseForFolder, UPnP
	/// @todo check return
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	try {
		std::string module = File(Text::fromT(app.getModuleFileName()), File::READ, File::OPEN).read();
		TigerTree tth(TigerTree::calcBlockSize(module.size(), 1));
		tth.update(module.data(), module.size());
		tth.finalize();
		WinUtil::tth = Text::toT(tth.getRoot().toBase32());
	} catch(const FileException&) {
		dcdebug("Failed reading exe\n");
	}
	
	int ret = 255;
	try {
		SplashWindow* splash(new SplashWindow);
		startup(&callBack, splash);

		if(ResourceManager::getInstance()->isRTL()) {
			SetProcessDefaultLayout(LAYOUT_RTL);
		}

		WinUtil::init();
		MainWindow* wnd = new MainWindow;
		WinUtil::mainWindow = wnd;
		WinUtil::mdiParent = wnd->getMDIParent();
		splash->close();
		ret = app.run();
	} catch(const SmartWin::xCeption& e) {
		printf("Exception: %s\n Additional info: %s\n", e.what(), e.whatWndMsg());
	} catch(const std::exception& e) {
		printf("Exception: %s\n", e.what());
	} catch(...) {
		printf("Unknown exception");
	}
	WinUtil::uninit();
	
	shutdown();

	::CoUninitialize();

	return ret;
}

