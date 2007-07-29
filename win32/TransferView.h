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

#ifndef DCPLUSPLUS_WIN32_TRANSFER_VIEW_H
#define DCPLUSPLUS_WIN32_TRANSFER_VIEW_H

#include <dcpp/DownloadManagerListener.h>
#include <dcpp/UploadManagerListener.h>
#include <dcpp/ConnectionManagerListener.h>
#include <dcpp/TaskQueue.h>
#include <dcpp/forward.h>
#include <dcpp/Util.h>
#include <dcpp/Download.h>
#include <dcpp/Upload.h>

#include "AspectSpeaker.h"
#include "TypedListViewCtrl.h"
#include "WidgetFactory.h"
#include "AspectUserCommand.h"

#include "UserInfoBase.h"

class TransferView : 
	public WidgetFactory<SmartWin::WidgetChildWindow>, 
	private DownloadManagerListener, 
	private UploadManagerListener, 
	private ConnectionManagerListener,
	public AspectSpeaker<TransferView>,
	public AspectUserInfo<TransferView>,
	public AspectUserCommand<TransferView>
{
public:
	TransferView(SmartWin::Widget*, SmartWin::WidgetMDIParent* parent);

	void prepareClose();

	virtual ~TransferView();
	
private:
	friend class AspectUserInfo<TransferView>;
	
	enum {
		ADD_ITEM,
		REMOVE_ITEM,
		UPDATE_ITEM,
	};

	enum {
		COLUMN_FIRST,
		COLUMN_USER = COLUMN_FIRST,
		COLUMN_HUB,
		COLUMN_STATUS,
		COLUMN_TIMELEFT,
		COLUMN_SPEED,
		COLUMN_FILE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_IP,
		COLUMN_RATIO,
		COLUMN_CID,
		COLUMN_LAST
	};

	enum {
		IMAGE_DOWNLOAD = 0,
		IMAGE_UPLOAD
	};

	struct UpdateInfo;
	class ItemInfo : public UserInfoBase {
	public:
		enum Status {
			STATUS_RUNNING,
			STATUS_WAITING
		};

		ItemInfo(const UserPtr& u, bool aDownload);

		bool download;
		bool transferFailed;
		Status status;
		int64_t pos;
		int64_t size;
		int64_t start;
		int64_t actual;
		int64_t speed;
		int64_t timeLeft;

		tstring columns[COLUMN_LAST];
		void update(const UpdateInfo& ui);

		void disconnect();
		void deleteSelf() { delete this; }

		double getRatio() { return (pos > 0) ? (double)actual / (double)pos : 1.0; }

		const tstring& getText(int col) const {
			return columns[col];
		}
		int getImage() const {
			return download ? IMAGE_DOWNLOAD : IMAGE_UPLOAD;
		}

		static int compareItems(ItemInfo* a, ItemInfo* b, int col);
	};

	struct UpdateInfo : public Task {
		enum {
			MASK_POS = 1 << 0,
			MASK_SIZE = 1 << 1,
			MASK_START = 1 << 2,
			MASK_ACTUAL = 1 << 3,
			MASK_SPEED = 1 << 4,
			MASK_FILE = 1 << 5,
			MASK_STATUS = 1 << 6,
			MASK_TIMELEFT = 1 << 7,
			MASK_IP = 1 << 8,
			MASK_STATUS_STRING = 1 << 9,
			MASK_COUNTRY = 1 << 10,
		};

		bool operator==(const ItemInfo& ii) { return download == ii.download && user == ii.user; }

		UpdateInfo(const UserPtr& aUser, bool isDownload, bool isTransferFailed = false) : updateMask(0), user(aUser), download(isDownload), transferFailed(isTransferFailed) { }

		uint32_t updateMask;

		UserPtr user;
		bool download;
		bool transferFailed;
		void setStatus(ItemInfo::Status aStatus) { status = aStatus; updateMask |= MASK_STATUS; }
		ItemInfo::Status status;
		void setPos(int64_t aPos) { pos = aPos; updateMask |= MASK_POS; }
		int64_t pos;
		void setSize(int64_t aSize) { size = aSize; updateMask |= MASK_SIZE; }
		int64_t size;
		void setStart(int64_t aStart) { start = aStart; updateMask |= MASK_START; }
		int64_t start;
		void setActual(int64_t aActual) { actual = aActual; updateMask |= MASK_ACTUAL; }
		int64_t actual;
		void setSpeed(int64_t aSpeed) { speed = aSpeed; updateMask |= MASK_SPEED; }
		int64_t speed;
		void setTimeLeft(int64_t aTimeLeft) { timeLeft = aTimeLeft; updateMask |= MASK_TIMELEFT; }
		int64_t timeLeft;
		void setStatusString(const tstring& aStatusString) { statusString = aStatusString; updateMask |= MASK_STATUS_STRING; }
		tstring statusString;
		void setFile(const tstring& aFile) { file = Util::getFileName(aFile); path = Util::getFilePath(aFile); updateMask|= MASK_FILE; }
		tstring file;
		tstring path;
		void setIP(const tstring& aIP) { IP = aIP; updateMask |= MASK_IP; }
		tstring IP;
	};

	static int columnIndexes[];
	static int columnSizes[];

	typedef TypedListViewCtrl<TransferView, ItemInfo> WidgetTransfers;
	typedef WidgetTransfers* WidgetTransfersPtr;
	WidgetTransfersPtr transfers;
	SmartWin::WidgetMDIParent* mdi;
	SmartWin::ImageListPtr arrows;

	TaskQueue tasks;
	StringMap ucLineParams;

	bool handleSized(const SmartWin::WidgetSizedEventResult& sz);
	HRESULT handleContextMenu(WPARAM wParam, LPARAM lParam);
	HRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);
	HRESULT handleDestroy(WPARAM wParam, LPARAM lParam);
	void handleForce();
	void handleSearchAlternates();
	void handleCopyNick();
	void handleRemove();
	void runUserCommand(const UserCommand& uc);
	bool handleKeyDown(int c);
	void handleDblClicked();

	WidgetMenuPtr makeContextMenu(ItemInfo* ii);

	WidgetTransfersPtr getUserList() { return transfers; }
	
	using AspectSpeaker<TransferView>::speak;
	
	void speak(int type, UpdateInfo* ui) { tasks.add(type, ui); speak(); }

	virtual void on(ConnectionManagerListener::Added, ConnectionQueueItem* aCqi) throw();
	virtual void on(ConnectionManagerListener::Failed, ConnectionQueueItem* aCqi, const string& aReason) throw();
	virtual void on(ConnectionManagerListener::Removed, ConnectionQueueItem* aCqi) throw();
	virtual void on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* aCqi) throw();

	virtual void on(DownloadManagerListener::Complete, Download* aDownload) throw() { onTransferComplete(aDownload, false);}
	virtual void on(DownloadManagerListener::Failed, Download* aDownload, const string& aReason) throw();
	virtual void on(DownloadManagerListener::Starting, Download* aDownload) throw();
	virtual void on(DownloadManagerListener::Tick, const DownloadList& aDownload) throw();

	virtual void on(UploadManagerListener::Starting, Upload* aUpload) throw();
	virtual void on(UploadManagerListener::Tick, const UploadList& aUpload) throw();
	virtual void on(UploadManagerListener::Complete, Upload* aUpload) throw() { onTransferComplete(aUpload, true); }

	void onTransferComplete(Transfer* aTransfer, bool isUpload);

#ifdef PORT_ME
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
#endif
};

#endif // !defined(TRANSFER_VIEW_H)
