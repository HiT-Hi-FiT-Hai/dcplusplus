#include "stdafx.h"
#include "dcplusplus.h"
#include "AdvancedPage.h"
#include "SettingsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item AdvancedPage::items[] = {
	{ IDC_ROLLBACK, SettingsManager::ROLLBACK, PropPage::T_INT }, 
	{ IDC_CLVERSION, SettingsManager::CLIENTVERSION, PropPage::T_STR }, 
	{ IDC_FOLLOW, SettingsManager::AUTO_FOLLOW, PropPage::T_BOOL },
	{ 0, 0, PropPage::T_END }
};

AdvancedPage::AdvancedPage(SettingsManager *s) : PropPage(s)
{
}

AdvancedPage::~AdvancedPage()
{
}

LRESULT AdvancedPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items);

	// Do specialized reading here
	return TRUE;
}

void AdvancedPage::write()
{
	PropPage::write((HWND)*this, items);

	// Do specialized writing here
	// settings->set(XX, YY);
}
