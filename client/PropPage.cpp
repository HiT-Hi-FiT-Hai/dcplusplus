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
#include "DCPlusPlus.h"

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

#define SETTING_STR_MAXLEN 1024

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

	char *buf = new char[SETTING_STR_MAXLEN];
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
	delete buf;
}

/**
 * @file PropPage.cpp
 * $Id: PropPage.cpp,v 1.5 2002/02/18 23:48:32 arnetheduck Exp $
 * @if LOG
 * $Log: PropPage.cpp,v $
 * Revision 1.5  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.4  2002/02/06 12:29:06  arnetheduck
 * New Buffered socket handling with asynchronous sending (asynchronous everything really...)
 *
 * Revision 1.3  2002/01/26 12:52:51  arnetheduck
 * More minor fixes
 *
 * @endif
 */

