#include "stdafx.h"
#include "DCPlusPlus.h"
#include "ResourceManager.h"
const string ResourceManager::strings[ResourceManager::LAST] = {
	"Close connection", "Connecting...", "Connecting (forced)...", "Download finished, idle...", "Download starting...", "Downloaded %s (%.01f%%), %s/s, %s left", "File", "Force attempt", "Get file list", "Port %d is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++", "Preparing file list...", "Really exit?", "Send private message", "Size", "Status", "Upload finished, idle...", "Upload starting...", "Uploaded %s (%.01f%%), %s/s, %s left", "User", "Waiting to retry..."
};
const string ResourceManager::names[ResourceManager::LAST] = {
	"CloseConnection", "Connecting", "ConnectingForced", "DownloadFinishedIdle", "DownloadStarting", "DownloadedLeft", "File", "ForceAttempt", "GetFileList", "PortIsBusy", "PreparingFileList", "ReallyExit", "SendPrivateMessage", "Size", "Status", "UploadFinishedIdle", "UploadStarting", "UploadedLeft", "User", "WaitingToRetry"
};
