#include "stdafx.h"
#include "PropPage.h"
#include "SettingsManager.h"

PropPage::PropPage(SettingsManager *s)
{
	settings = s;
}

PropPage::~PropPage()
{
}

namespace
{
	template <long bufSize, class Source> class ItemText
	{
		public:
			ItemText(Source const& s) : m_src(s) { }
			char const* Get(WORD id)
			{
				*m_buf = '\0';
				m_src.GetDlgItemText(id, m_buf, SETTINGS_BUF_LEN);
				return m_buf;
			}
			
		protected:
			Source const& m_src;
			char m_buf[bufSize];
	};
}

#define SETTING_STR_MAXLEN 512

void PropPage::read(HWND page, Item const* items)
{
	dcassert(page != NULL);

	bool const useDef = true;
	for(Item const* i = items; i->type != T_END; i++)
	{
		switch(i->type)
		{
		case T_STR:
			::SetDlgItemText(page, i->itemID,
				settings->get((SettingsManager::StrSetting)i->setting, useDef).c_str());
			break;
		case T_INT:
			::SetDlgItemInt(page, i->itemID,
				settings->get((SettingsManager::IntSetting)i->setting, useDef), FALSE);
			break;
		case T_BOOL:
			if(settings->getBool((SettingsManager::IntSetting)i->setting, useDef))
				::CheckDlgButton(page, i->itemID, BST_CHECKED);
			else
				::CheckDlgButton(page, i->itemID, BST_UNCHECKED);
		}
	}
}

void PropPage::write(HWND page, Item const* items)
{
	dcassert(page != NULL);

	char buf[SETTING_STR_MAXLEN];
	for(Item const* i = items; i->type != T_END; i++)
	{
		switch(i->type)
		{
		case T_STR:
			{
				::GetDlgItemText(page, i->itemID, buf, SETTING_STR_MAXLEN);
				settings->set((SettingsManager::StrSetting)i->setting, buf);
				break;
			}
		case T_INT:
			{
				BOOL transl;
				int val = ::GetDlgItemInt(page, i->itemID, &transl, FALSE);
				//if(transl == FALSE) throw ...
				settings->set((SettingsManager::IntSetting)i->setting, val);
				break;
			}
		case T_BOOL:
			{
				if(::IsDlgButtonChecked(page, i->itemID) == BST_CHECKED)
					settings->set((SettingsManager::IntSetting)i->setting, true);
				else
					settings->set((SettingsManager::IntSetting)i->setting, false);
			}
		}
	}
}
