#include "stdafx.h"
#include "DCPlusPlus.h"
#include "ResourceManager.h"
string ResourceManager::strings[] = {
	"Add To Favorites", "All %d users offline", "Any", "At least", "At most", "Audio", "Auto connect / Name", "Away mode off", "Away mode on: ", "Both users offline", 
"Browse...", "Close connection", "Compressed", "Connect", "Connected", "Connecting...", "Connecting (forced)...", "Connecting to ", "Connection", "Description", 
"Document", "Done", "Download", "Download failed: ", "Download finished, idle...", "Download starting...", "Download to...", "Download queue", "Downloaded %s (%.01f%%), %s/s, %s left", "Downloading public hub list...", 
"E-Mail", "Please enter a nickname in the settings dialog!", "Please enter a password", "Please enter a reason", "Please enter a destination server", "Executable", "Favorite Hubs", "Favorite Users", "File", "Files", 
"File list refreshed", "File type", "Filename", "Finished", "Filter", "Force attempt", "Get file list", "Grant extra slot", "High", "Hub", 
"Hubs", "Address", "Name", "Hub password", "Ignored message: ", "Invalid number of slots", "Kick user(s)", "Hub (last seen on if offline)", "Low", "Manual connect address", 
"New...", "Nick", "Your nick was already taken, please change to something else!", "No users to download from", "Normal", "Offline", "Online", "Only users with free slots", "Password", "Path", 
"Paused", "Picture", "Port %d is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++", "Preparing file list...", "Priority", "Private message from ", "Properties", "Public Hubs", "Really exit?", "Redirect", 
"Redirect user(s)", "Refresh", "Refresh user list", "Remove", "Remove source", "Running...", "Search", "Search for", "Search for alternates", "Search options", 
"Searching for ", "Send private message", "Server", "Set priority", "Shared", "Size", "Slots", "Slots set", "Specify a server to connect to", "Specify a search string", 
"Status", "Timestamps disabled", "Timestamps enabled", "Type", "Unknown", "Upload finished, idle...", "Upload starting...", "Uploaded %s (%.01f%%), %s/s, %s left", "User", "User offline", 
"User went offline", "Users", "Video", "Waiting...", "Waiting (User online)", "Waiting (%d of %d users online)", "Waiting to retry...", "You are being redirected to "
};
string ResourceManager::names[] = {
	"AddToFavorites", "AllUsersOffline", "Any", "AtLeast", "AtMost", "Audio", "AutoConnect", "AwayModeOff", "AwayModeOn", "BothUsersOffline", 
"Browse", "CloseConnection", "Compressed", "Connect", "Connected", "Connecting", "ConnectingForced", "ConnectingTo", "Connection", "Description", 
"Document", "Done", "Download", "DownloadFailed", "DownloadFinishedIdle", "DownloadStarting", "DownloadTo", "DownloadQueue", "DownloadedLeft", "DownloadingHubList", 
"Email", "EnterNick", "EnterPassword", "EnterReason", "EnterServer", "Executable", "FavoriteHubs", "FavoriteUsers", "File", "Files", 
"FileListRefreshed", "FileType", "Filename", "Finished", "Filter", "ForceAttempt", "GetFileList", "GrantExtraSlot", "High", "Hub", 
"Hubs", "HubAddress", "HubName", "HubPassword", "IgnoredMessage", "InvalidNumberOfSlots", "KickUser", "LastHub", "Low", "ManualAddress", 
"New", "Nick", "NickTaken", "NoUsersToDownload", "Normal", "Offline", "Online", "OnlyFreeSlots", "Password", "Path", 
"Paused", "Picture", "PortIsBusy", "PreparingFileList", "Priority", "PrivateMessageFrom", "Properties", "PublicHubs", "ReallyExit", "Redirect", 
"RedirectUser", "Refresh", "RefreshUserList", "Remove", "RemoveSource", "Running", "Search", "SearchFor", "SearchForAlternates", "SearchOptions", 
"SearchingFor", "SendPrivateMessage", "Server", "SetPriority", "Shared", "Size", "Slots", "SlotsSet", "SpecifyServer", "SpecifySearchString", 
"Status", "TimestampsDisabled", "TimestampsEnabled", "Type", "Unknown", "UploadFinishedIdle", "UploadStarting", "UploadedLeft", "User", "UserOffline", 
"UserWentOffline", "Users", "Video", "Waiting", "WaitingUserOnline", "WaitingUsersOnline", "WaitingToRetry", "YouAreBeingRedirected"
};
