// @Prolog: #include "stdafx.h"
// @Prolog: #include "DCPlusPlus.h"
// @Prolog: #include "ResourceManager.h"
// @Strings: const string ResourceManager::strings[ResourceManager::LAST]
// @Names: const string ResourceManager::names[ResourceManager::LAST]

enum Strings { // @DontAdd
	CLOSE_CONNECTION, // "Close connection"
	CONNECTING, // "Connecting..."
	CONNECTING_FORCED, // "Connecting (forced)..."
	DOWNLOAD_FINISHED_IDLE, // "Download finished, idle..."
	DOWNLOAD_STARTING, // "Download starting..."
	DOWNLOADED_LEFT, // "Downloaded %s (%.01f%%), %s/s, %s left"
	FILE, // "File"
	FORCE_ATTEMPT, // "Force attempt"
	GET_FILE_LIST, // "Get file list"
	PORT_IS_BUSY, // "Port %d is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++"
	PREPARING_FILE_LIST, // "Preparing file list..."
	REALLY_EXIT, // "Really exit?"
	SEND_PRIVATE_MESSAGE, // "Send private message"
	SIZE, // "Size"
	STATUS, // "Status"
	UPLOAD_FINISHED_IDLE, // "Upload finished, idle..."
	UPLOAD_STARTING, // "Upload starting..."
	UPLOADED_LEFT, // "Uploaded %s (%.01f%%), %s/s, %s left"
	USER, // "User"
	WAITING_TO_RETRY, // "Waiting to retry..."
	LAST // @DontAdd
};
