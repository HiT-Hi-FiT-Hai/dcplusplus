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
#include <dcpp/QueueManagerListener.h>
#include <dcpp/TaskQueue.h>
#include <dcpp/forward.h>
#include <dcpp/MerkleTree.h>
#include <dcpp/Util.h>

#include "AspectSpeaker.h"
#include "TypedTable.h"
#include "WidgetFactory.h"
#include "AspectUserCommand.h"

#include "UserInfoBase.h"

class TransferView : 
	public WidgetFactory<dwt::Container>, 
	private DownloadManagerListener, 
	private UploadManagerListener, 
	private ConnectionManagerListener,
	private QueueManagerListener,
	public AspectSpeaker<TransferView>,
	public AspectUserInfo<TransferView>,
	public AspectUserCommand<TransferView>
{
public:
	TransferView(dwt::Widget* parent, dwt::TabView* mdi);

	void prepareClose();

	virtual ~TransferView();
	
private:
	friend class AspectUserInfo<TransferView>;
	
	enum {
		DOWNLOAD_COLUMN_FIRST,
		DOWNLOAD_COLUMN_FILE = DOWNLOAD_COLUMN_FIRST,
		DOWNLOAD_COLUMN_PATH,
		DOWNLOAD_COLUMN_STATUS,
		DOWNLOAD_COLUMN_TIMELEFT,
		DOWNLOAD_COLUMN_SPEED,
		DOWNLOAD_COLUMN_DONE,
		DOWNLOAD_COLUMN_SIZE,
		DOWNLOAD_COLUMN_LAST
	};
	
	enum {
		DOWNLOADS_ADD_USER,
		DOWNLOADS_TICK,
		DOWNLOADS_REMOVE_USER,
		DOWNLOADS_REMOVED,
		CONNECTIONS_ADD,
		CONNECTIONS_REMOVE,
		CONNECTIONS_UPDATE
	};

	enum {
		CONNECTION_COLUMN_FIRST,
		CONNECTION_COLUMN_USER = CONNECTION_COLUMN_FIRST,
		CONNECTION_COLUMN_HUB,
		CONNECTION_COLUMN_STATUS,
		CONNECTION_COLUMN_SPEED,
		CONNECTION_COLUMN_CHUNK,
		CONNECTION_COLUMN_TRANSFERED,
		CONNECTION_COLUMN_QUEUED,
		CONNECTION_COLUMN_CIPHER,
		CONNECTION_COLUMN_IP,
		CONNECTION_COLUMN_LAST
	};

	enum {
		IMAGE_DOWNLOAD = 0,
		IMAGE_UPLOAD
	};

	struct UpdateInfo;
	
	class ConnectionInfo : public UserInfoBase {
	public:
		enum Status {
			STATUS_RUNNING,		///< Transfering
			STATUS_WAITING		///< Idle
		};

		ConnectionInfo(const UserPtr& u, bool aDownload);

		bool download;
		bool transferFailed;
		
		Status status;
		
		int64_t actual;
		int64_t lastActual;
		int64_t transfered;
		int64_t lastTransfered;
		int64_t queued;
		int64_t speed;
		int64_t chunk;
		int64_t chunkPos;
		
		tstring columns[CONNECTION_COLUMN_LAST];
		void update(const UpdateInfo& ui);

		void disconnect();

		double getRatio() { return (transfered > 0) ? (double)actual / (double)transfered : 1.0; }

		const tstring& getText(int col) const {
			return columns[col];
		}
		int getImage() const {
			return download ? IMAGE_DOWNLOAD : IMAGE_UPLOAD;
		}

		static int compareItems(ConnectionInfo* a, ConnectionInfo* b, int col);
	};

	struct UpdateInfo : public Task {
		enum {
			MASK_STATUS = 1 << 0,
			MASK_STATUS_STRING = 1 << 1,
			MASK_SPEED = 1 << 2,
			MASK_TRANSFERED = 1 << 3,
			MASK_IP = 1 << 4,
			MASK_CIPHER = 1 << 5,
			MASK_CHUNK = 1 << 6
		};

		bool operator==(const ConnectionInfo& ii) { return download == ii.download && user == ii.user; }

		UpdateInfo(const UserPtr& aUser, bool isDownload, bool isTransferFailed = false) : updateMask(0), user(aUser), download(isDownload), transferFailed(isTransferFailed) { }

		uint32_t updateMask;

		UserPtr user;
		bool download;
		bool transferFailed;
		
		void setStatus(ConnectionInfo::Status aStatus) { status = aStatus; updateMask |= MASK_STATUS; }
		ConnectionInfo::Status status;
		void setTransfered(int64_t aTransfered, int64_t aActual) {
			transfered = aTransfered; actual = aActual; updateMask |= MASK_TRANSFERED; 
		}
		int64_t actual;
		int64_t transfered;
		void setSpeed(int64_t aSpeed) { speed = aSpeed; updateMask |= MASK_SPEED; }
		int64_t speed;
		void setStatusString(const tstring& aStatusString) { statusString = aStatusString; updateMask |= MASK_STATUS_STRING; }
		tstring statusString;
		void setChunk(int64_t aChunkPos, int64_t aChunk) { chunkPos = aChunkPos; chunk = aChunk; updateMask |= MASK_CHUNK; }
		int64_t chunkPos;
		int64_t chunk;
		
		void setIP(const tstring& aIp) { ip = aIp; updateMask |= MASK_IP; }
		tstring ip;
		void setCipher(const tstring& aCipher) { cipher = aCipher; updateMask |= MASK_CIPHER; }
		tstring cipher;
	};

	struct TickInfo : public Task {
		TickInfo(const string& path_) : path(path_), done(0), bps(0) { }
		
		string path;
		int64_t done;
		double bps;
	};

	
	static int connectionIndexes[CONNECTION_COLUMN_LAST];
	static int connectionSizes[CONNECTION_COLUMN_LAST];
	
	class DownloadInfo {
	public:
		DownloadInfo(const string& filename, int64_t size, const TTHValue& tth);
		
		const tstring& getText(int col) const {
			return columns[col];
		}

		int getImage() const;

		static int compareItems(DownloadInfo* a, DownloadInfo* b, int col) {
			switch(col) {
			case DOWNLOAD_COLUMN_STATUS: return compare(a->users, b->users);
			case DOWNLOAD_COLUMN_TIMELEFT: return compare(a->timeleft(), b->timeleft());
			case DOWNLOAD_COLUMN_SPEED: return compare(a->bps, b->bps);
			case DOWNLOAD_COLUMN_SIZE: return compare(a->size, b->size);
			case DOWNLOAD_COLUMN_DONE: return compare(a->done, b->done);
			default: return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
			}
		}
		
		void update();
		void update(const TickInfo& ti);
		
		int64_t timeleft() { return bps == 0 ? 0 : (size - done) / bps; }
		string path;
		int64_t done;
		int64_t size;
		double bps;
		int users;
		TTHValue tth;
		
		tstring columns[DOWNLOAD_COLUMN_LAST];
	};
	

	static int downloadIndexes[DOWNLOAD_COLUMN_LAST];
	static int downloadSizes[DOWNLOAD_COLUMN_LAST];

	typedef TypedTable<ConnectionInfo> WidgetConnections;
	typedef WidgetConnections* WidgetConnectionsPtr;
	WidgetConnectionsPtr connections;
	ContainerPtr connectionsWindow;
	
	typedef TypedTable<DownloadInfo> WidgetDownloads;
	typedef WidgetDownloads* WidgetDownloadsPtr;
	WidgetDownloadsPtr downloads;
	ContainerPtr downloadsWindow;

	TabSheetPtr tabs;
	
	dwt::TabView* mdi;
	dwt::ImageListPtr arrows;

	bool startup;

	TaskQueue tasks;
	StringMap ucLineParams;

	void handleSized(const dwt::SizedEvent& sz);
	bool handleConnectionsMenu(dwt::ScreenCoordinate pt);
	bool handleDownloadsMenu(dwt::ScreenCoordinate pt);
	HRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);
	HRESULT handleDestroy(WPARAM wParam, LPARAM lParam);
	void handleForce();
	void handleCopyNick();
	void handleDisconnect();
	void runUserCommand(const UserCommand& uc);
	bool handleKeyDown(int c);
	void handleDblClicked();
	void handleTabSelected();
	
	WidgetMenuPtr makeContextMenu(ConnectionInfo* ii);

	WidgetConnectionsPtr getUserList() { return connections; }
	
	int find(const string& path);
	
	void layout();

	using AspectSpeaker<TransferView>::speak;
	
	void speak(int type, Task* ui) { tasks.add(type, ui); speak(); }

	virtual void on(ConnectionManagerListener::Added, ConnectionQueueItem* aCqi) throw();
	virtual void on(ConnectionManagerListener::Failed, ConnectionQueueItem* aCqi, const string& aReason) throw();
	virtual void on(ConnectionManagerListener::Removed, ConnectionQueueItem* aCqi) throw();
	virtual void on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* aCqi) throw();

	virtual void on(DownloadManagerListener::Requesting, Download* aDownload) throw();
	virtual void on(DownloadManagerListener::Complete, Download* aDownload) throw();
	virtual void on(DownloadManagerListener::Failed, Download* aDownload, const string& aReason) throw();
	virtual void on(DownloadManagerListener::Starting, Download* aDownload) throw();
	virtual void on(DownloadManagerListener::Tick, const DownloadList& aDownload) throw();

	virtual void on(UploadManagerListener::Starting, Upload* aUpload) throw();
	virtual void on(UploadManagerListener::Tick, const UploadList& aUpload) throw();
	virtual void on(UploadManagerListener::Complete, Upload* aUpload) throw();

	virtual void on(QueueManagerListener::Removed, QueueItem*) throw();

	void onTransferTick(Transfer* aTransfer, bool isDownload);
	void onTransferComplete(Transfer* aTransfer, bool isDownload);
	void starting(UpdateInfo* ui, Transfer* t);
	
#ifdef PORT_ME
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
#endif
};

#endif // !defined(TRANSFER_VIEW_H)
