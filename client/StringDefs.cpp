#include "stdinc.h"
#include "DCPlusPlus.h"
#include "ResourceManager.h"
string ResourceManager::strings[] = {
	"Add To Favorites", "All download slots taken", "All %d users offline", "Any", "At least", "At most", "Audio", "Auto connect / Name", "Away mode off", "Away mode on: ", 
"Both users offline", "Browse...", "Can't connect to passive user while in passive mode", "Close connection", "Close", "Compressed", "Connect", "Connected", "Connecting...", "Connecting (forced)...", 
"Connecting to ", "Connection", "Connection timeout", "Count", "Description", "Disconnected", "Document", "Done", "Download", "Download failed: ", 
"Download finished, idle...", "Download starting...", "Download to...", "Download queue", "Downloaded %s (%.01f%%), %s/s, %s left", " downloaded from ", "Downloading public hub list...", "E-Mail", "Please enter a nickname in the settings dialog!", "Please enter a password", 
"Please enter a reason", "Please enter a destination server", "Error opening file", "Executable", "Favorite Hubs", "Favorite Users", "File", "Files", "File list refreshed", "File type", 
"Filename", "Filter", "Force attempt", "Get file list", "Grant extra slot", "High", "Hub", "Hubs", "Address", "Hub list downloaded...", 
"Name", "Hub password", "Ignored message: ", "Import queue from NMDC", "Invalid number of slots", "Joins: ", "Join/part showing off", "Join/part showing on", "Kick user(s)", "Hub (last seen on if offline)", 
"Low", "Manual connect address", "&File", "&Download Queue\tCtrl+D", "&Exit", "&Favorite Hubs\tCtrl+F", "Favorite &Users\tCtrl+U", "Follow last redirec&t\tCtrl+T", "&Notepad\tCtrl+N", "&Public Hubs\tCtrl+P", 
"&Reconnect\tCtrl+R", "&Search\tCtrl+S", "Search spy", "Settings...", "&Help", "About DC++...", "DC++ discussion forum", "Downloads and translations", "Frequently asked questions", "Help forum", 
"DC++ Homepage", "Request a feature", "Report a bug", "&View", "&Status bar", "&Toolbar", "&Window", "Arrange icons", "Cascade", "Tile", 
"New...", "Nick", "Your nick was already taken, please change to something else!", "Not connected", "No slots available", "No users to download from", "Normal", "Offline", "Online", "Only users with free slots", 
"Password", "Parts: ", "Path", "Paused", "Picture", "Port %d is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++", "Preparing file list...", "Priority", "Private message from ", "Properties", 
"Public Hubs", "Really exit?", "Redirect", "Redirect user(s)", "Refresh", "Refresh user list", "Remove", "Remove source", "Rollback inconsistency, existing file does not match the one being downloaded", "Running...", 
"Search", "Search for", "Search for alternates", "Search options", "Search Spy", "Search string", "Searching for ", "Send private message", "Server", "Set priority", 
"Shared", "Size", "Slots", "Slots set", "Specify a server to connect to", "Specify a search string", "Status", "Timestamps disabled", "Timestamps enabled", "Type", 
"Unknown", "Upload finished, idle...", "Upload starting...", "Uploaded %s (%.01f%%), %s/s, %s left", " uploaded to ", "User", "User offline", "User went offline", "Users", "Video", 
"Waiting...", "Waiting (User online)", "Waiting (%d of %d users online)", "Waiting to retry...", "You are being redirected to "
};
string ResourceManager::names[] = {
	"AddToFavorites", "AllDownloadSlotsTaken", "AllUsersOffline", "Any", "AtLeast", "AtMost", "Audio", "AutoConnect", "AwayModeOff", "AwayModeOn", 
"BothUsersOffline", "Browse", "CantConnectInPassiveMode", "CloseConnection", "Close", "Compressed", "Connect", "Connected", "Connecting", "ConnectingForced", 
"ConnectingTo", "Connection", "ConnectionTimeout", "Count", "Description", "Disconnected", "Document", "Done", "Download", "DownloadFailed", 
"DownloadFinishedIdle", "DownloadStarting", "DownloadTo", "DownloadQueue", "DownloadedLeft", "DownloadedFrom", "DownloadingHubList", "Email", "EnterNick", "EnterPassword", 
"EnterReason", "EnterServer", "ErrorOpeningFile", "Executable", "FavoriteHubs", "FavoriteUsers", "File", "Files", "FileListRefreshed", "FileType", 
"Filename", "Filter", "ForceAttempt", "GetFileList", "GrantExtraSlot", "High", "Hub", "Hubs", "HubAddress", "HubListDownloaded", 
"HubName", "HubPassword", "IgnoredMessage", "ImportQueue", "InvalidNumberOfSlots", "Joins", "JoinShowingOff", "JoinShowingOn", "KickUser", "LastHub", 
"Low", "ManualAddress", "MenuFile", "MenuFileDownloadQueue", "MenuFileExit", "MenuFileFavoriteHubs", "MenuFileFavoriteUsers", "MenuFileFollowRedirect", "MenuFileNotepad", "MenuFilePublicHubs", 
"MenuFileReconnect", "MenuFileSearch", "MenuFileSearchSpy", "MenuFileSettings", "MenuHelp", "MenuHelpAbout", "MenuHelpDiscuss", "MenuHelpDownloads", "MenuHelpFaq", "MenuHelpHelpForum", 
"MenuHelpHomepage", "MenuHelpRequestFeature", "MenuHelpReportBug", "MenuView", "MenuViewStatusBar", "MenuViewToolbar", "MenuWindow", "MenuWindowArrange", "MenuWindowCascade", "MenuWindowTile", 
"New", "Nick", "NickTaken", "NotConnected", "NoSlotsAvailable", "NoUsersToDownload", "Normal", "Offline", "Online", "OnlyFreeSlots", 
"Password", "Parts", "Path", "Paused", "Picture", "PortIsBusy", "PreparingFileList", "Priority", "PrivateMessageFrom", "Properties", 
"PublicHubs", "ReallyExit", "Redirect", "RedirectUser", "Refresh", "RefreshUserList", "Remove", "RemoveSource", "RollbackInconsistency", "Running", 
"Search", "SearchFor", "SearchForAlternates", "SearchOptions", "SearchSpy", "SearchString", "SearchingFor", "SendPrivateMessage", "Server", "SetPriority", 
"Shared", "Size", "Slots", "SlotsSet", "SpecifyServer", "SpecifySearchString", "Status", "TimestampsDisabled", "TimestampsEnabled", "Type", 
"Unknown", "UploadFinishedIdle", "UploadStarting", "UploadedLeft", "UploadedTo", "User", "UserOffline", "UserWentOffline", "Users", "Video", 
"Waiting", "WaitingUserOnline", "WaitingUsersOnline", "WaitingToRetry", "YouAreBeingRedirected"
};
