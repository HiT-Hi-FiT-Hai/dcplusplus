/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#include "resource.h"

#include "HashProgressDlg.h"

#include <dcpp/HashManager.h>

HashProgressDlg::HashProgressDlg(SmartWin::Widget* parent, bool aAutoClose) :
	SmartWin::WidgetFactory<SmartWin::WidgetModalDialog>(parent),
	progress(0),
	autoClose(aAutoClose)
{
	onInitDialog(std::tr1::bind(&HashProgressDlg::handleInitDialog, this));
}

HashProgressDlg::~HashProgressDlg() {
	HashManager::getInstance()->setPriority(Thread::IDLE);
}

bool HashProgressDlg::handleInitDialog() {
	setHelpId(IDH_HASH_PROGRESS);

	setText(T_("Creating file index..."));
	::SetDlgItemText(handle(), IDC_HASH_INDEXING, CT_("Please wait while DC++ indexes your files (they won't be shared until they've been indexed)..."));
	::SetDlgItemText(handle(), IDC_STATISTICS, CT_("Statistics"));

	progress = attachProgressBar(IDC_HASH_PROGRESS);
	progress->setRange(0, 10000);

	WidgetButtonPtr ok = attachButton(IDOK);
	ok->setText(T_("Run in background"));
	ok->onClicked(std::tr1::bind(&HashProgressDlg::endDialog, this, IDOK));

	string tmp;
	startTime = GET_TICK();
	HashManager::getInstance()->getStats(tmp, startBytes, startFiles);

	updateStats();

	HashManager::getInstance()->setPriority(Thread::NORMAL);

	createTimer(std::tr1::bind(&HashProgressDlg::updateStats, this), 1000);

	return false;
}

bool HashProgressDlg::updateStats() {
	string file;
	int64_t bytes = 0;
	size_t files = 0;
	uint32_t tick = GET_TICK();

	HashManager::getInstance()->getStats(file, bytes, files);
	if(bytes > startBytes)
		startBytes = bytes;

	if(files > startFiles)
		startFiles = files;

	if(autoClose && files == 0) {
		close(true);
		return true;
	}
	double diff = tick - startTime;
	if(diff < 1000 || files == 0 || bytes == 0) {
		::SetDlgItemText(handle(), IDC_FILES_PER_HOUR, str(TF_("-.-- files/h, %1% files left") % (uint32_t)files).c_str());
		::SetDlgItemText(handle(), IDC_HASH_SPEED, str(TF_("-.-- B/s, %1% left") % Text::toT(Util::formatBytes(bytes))).c_str());
		::SetDlgItemText(handle(), IDC_TIME_LEFT, CT_("-:--:-- left"));
		progress->setPosition(0);
	} else {
		double filestat = (((double)(startFiles - files)) * 60 * 60 * 1000) / diff;
		double speedStat = (((double)(startBytes - bytes)) * 1000) / diff;

		::SetDlgItemText(handle(), IDC_FILES_PER_HOUR, str(TF_("%1% files/h, %2% files left") % filestat % (uint32_t)files).c_str());
		::SetDlgItemText(handle(), IDC_HASH_SPEED, str(TF_("%1%/s, %2% left") % Text::toT(Util::formatBytes((int64_t)speedStat)) % Text::toT(Util::formatBytes(bytes))).c_str());

		if(filestat == 0 || speedStat == 0) {
			::SetDlgItemText(handle(), IDC_TIME_LEFT, CT_("-:--:-- left"));
		} else {
			double fs = files * 60 * 60 / filestat;
			double ss = bytes / speedStat;

			::SetDlgItemText(handle(), IDC_TIME_LEFT, str(TF_("%1% left") % Text::toT(Util::formatSeconds((int64_t)(fs + ss) / 2))).c_str());
		}
	}

	if(files == 0) {
		::SetDlgItemText(handle(), IDC_CURRENT_FILE, CT_("Done"));
	} else {
		::SetDlgItemText(handle(), IDC_CURRENT_FILE, Text::toT(file).c_str());
	}

	if(startFiles == 0 || startBytes == 0) {
		progress->setPosition(0);
	} else {
		progress->setPosition((int)(10000 * ((0.5 * (startFiles - files)/startFiles) + 0.5 * (startBytes - bytes) / startBytes)));
	}

	return true;
}
