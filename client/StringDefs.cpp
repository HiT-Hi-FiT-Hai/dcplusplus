#include "stdinc.h"
#include "DCPlusPlus.h"
#include "ResourceManager.h"
string ResourceManager::strings[] = {
"Active", 
"Enabled / Search String", 
"&Add", 
"Add To Favorites", 
"Added", 
"Address already in use", 
"Address not available", 
"Automatic Directory Listing Search", 
"Destination Directory", 
"Discard", 
"Download Matches", 
"Enabled", 
"Full Path", 
"ADLSearch Properties", 
"Search String", 
"Max FileSize", 
"Min FileSize", 
"Search Type", 
"Size Type", 
"All download slots taken", 
"All %d users offline", 
"All 3 users offline", 
"All 4 users offline", 
"All", 
"Any", 
"At least", 
"At most", 
"Audio", 
"Auto connect / Name", 
"Auto grant slot / Nick", 
"Average/s: ", 
"AWAY", 
"Away mode off", 
"Away mode on: ", 
"B", 
"Ban user(s)", 
"Both users offline", 
"Browse...", 
"&Browse...", 
"Browse file list", 
"Choose folder", 
"Close", 
"Close connection", 
"Closing connection...", 
"Compressed", 
"Error during compression", 
"&Configure", 
"&Connect", 
"Connected", 
"Connecting...", 
"Connecting (forced)...", 
"Connecting to ", 
"Connection", 
"Connection closed", 
"Connection refused by target machine", 
"Connection reset by server", 
"Connection timeout", 
"Configured Public Hub Lists", 
"Copy address to clipboard", 
"Copy magnet link to clipboard", 
"Copy nick to clipboard", 
"Could not open target file: ", 
"Count", 
"Country", 
"CRC Checked", 
"Error during decompression", 
"Description", 
"Destination", 
"Directory", 
"Directory already shared", 
"Directory or directory name already exists", 
"Disk full(?)", 
"Disconnect user(s)", 
"Disconnected", 
"Disconnected user leaving the hub: ", 
"Document", 
"Done", 
"Don't remove /password before your password", 
"The temporary download directory cannot be shared", 
"Download", 
"Download failed: ", 
"Download finished, idle...", 
"Download Queue", 
"Download starting...", 
"Download to...", 
"Download whole directory", 
"Download whole directory to...", 
"Downloaded", 
"Downloaded %s (%.01f%%) in %s", 
" downloaded from ", 
"Downloading...", 
"Downloading public hub list...", 
"Downloading list...", 
"Downloads", 
"Duplicate file will not be shared: ", 
"Dupe matched against: ", 
"Duplicate source", 
"Edit", 
"&Edit", 
"E-Mail", 
"Please enter a nickname in the settings dialog!", 
"Please enter a password", 
"Please enter a reason", 
"Enter search string", 
"Please enter a destination server", 
"Errors", 
"Exact size", 
"Executable", 
"Join/part of favorite users showing off", 
"Join/part of favorite users showing on", 
"Favorite name", 
"Under what name you see the directory", 
"Favorite hub added", 
"Identification (leave blank for defaults)", 
"Favorite Hub Properties", 
"Favorite Hubs", 
"Favorite user added", 
"Favorite Users", 
"File", 
"Files", 
"File list refresh finished", 
"File list refresh initiated", 
"File not available", 
"File type", 
"A file with a different size already exists in the queue", 
"A file with diffent tth root already exists in the queue", 
"Filename", 
"files left", 
"files/h", 
"F&ilter", 
"Filtered", 
"Find", 
"Finished Downloads", 
"Finished Uploads", 
"File with '$' cannot be downloaded and will not be shared: ", 
"Force attempt", 
"GiB", 
"Get file list", 
"Go to directory", 
"Grant extra slot", 
"Hash database", 
"Creating file index...", 
"Run in background", 
"Statistics", 
"Please wait while DC++ indexes your files (they won't be shared until they've been indexed)...", 
"Hash database rebuilt", 
"Finished hashing: ", 
"High", 
"Highest", 
"Hit Ratio: ", 
"Hits: ", 
"Host unreachable", 
"Hub", 
"Hubs", 
"Address", 
"Hub list downloaded...", 
"Name", 
"Hub password", 
"Users", 
"Ignore TTH searches", 
"Ignored message: ", 
"Invalid number of slots", 
"Invalid target file (missing directory, check default download directory setting)", 
"Downloaded tree does not match TTH root", 
"IP: ", 
"IP", 
"Items", 
"Join/part showing off", 
"Join/part showing on", 
"Joins: ", 
"KiB", 
"KiB/s", 
"Kick user(s)", 
"A file of equal or larger size already exists at the target location", 
"Last change: ", 
"Hub (last seen on if offline)", 
"Time last seen", 
"left", 
"Loading DC++, please wait...", 
"Lookup TTH at Bitzi.com", 
"Low", 
"Lowest", 
"Filename:", 
"File Hash:", 
"Do nothing", 
"Add this file to your download queue", 
"Do the same action next time without asking", 
"Start a search for this file", 
"A MAGNET link was given to DC++, but it didn't contain a valid file hash for use on the Direct Connect network.  No action will be taken.", 
"DC++ has detected a MAGNET link with a file hash that can be searched for on the Direct Connect network.  What would you like to do?", 
"MAGNET Link detected", 
"Download files from the Direct Connect network", 
"DC++", 
"URL:MAGNET URI", 
"Match queue", 
"Matched %d file(s)", 
"Max Hubs", 
"Max Users", 
"MiB", 
"MiB/s", 
"About DC++...", 
"ADL Search", 
"Arrange icons", 
"Cascade", 
"Change Log", 
"Close disconnected", 
"Help &Contents\tF1", 
"DC++ discussion forum", 
"Donate ���/$$$ (paypal)", 
"&Download Queue\tCtrl+D", 
"&Exit", 
"Frequently asked questions", 
"&Favorite Hubs\tCtrl+F", 
"Favorite &Users\tCtrl+U", 
"&File", 
"Follow last redirec&t\tCtrl+T", 
"Indexing progress", 
"&Help", 
"Downloads", 
"Help forum", 
"Translations", 
"DC++ Homepage", 
"Horizontal Tile", 
"Minimize &All", 
"Restore All", 
"Network Statistics", 
"&Notepad\tCtrl+N", 
"Open downloads directory", 
"Open file list...\tCtrl+L", 
"Open own list", 
"&Public Hubs\tCtrl+P", 
"&Quick Connect ...\tCtrl+Q", 
"Readme / Newbie help", 
"&Reconnect\tCtrl+R", 
"Refresh file list\tCtrl+E", 
"Report a bug", 
"Request a feature", 
"&Search\tCtrl+S", 
"Search Spy", 
"Settings...", 
"Show", 
"&Status bar\tCtrl+2", 
"&Toolbar\tCtrl+1", 
"T&ransfers\tCtrl+3", 
"Vertical Tile", 
"&View", 
"&Window", 
"Min Share", 
"Min Slots", 
"Move/Rename", 
"Move &Down", 
"Move &Up", 
"Network Statistics", 
"Network unreachable (are you connected to the internet?)", 
"&New...", 
"Next", 
"Nick", 
"Your nick was already taken, please change to something else!", 
" (Nick unknown)", 
"No", 
"No directory specified", 
"You're trying to download from yourself!", 
"No errors", 
"No matches", 
"No slots available", 
"No users", 
"No users to download from", 
"Non-blocking operation still in progress", 
"Normal", 
"Not connected", 
"Not a socket", 
"Notepad", 
"Offline", 
"Online", 
"Only users with free slots", 
"Only results with TTH root", 
"Only where I'm op", 
"Open", 
"Open download page?", 
"Open folder", 
"Operating system does not match minimum requirements for feature", 
"Operation would block execution", 
"Out of buffer space", 
"Parts: ", 
"Passive user", 
"Password", 
"Path", 
"Paused", 
"Permission denied", 
"PiB", 
"Picture", 
"Port: ", 
"Port %d is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++", 
"Preparing file list...", 
"Press the follow redirect button to connect to ", 
"Priority", 
"Private message from ", 
"&Properties", 
"Public Hubs", 
"Purge", 
"Quick Connect", 
"Rating", 
"Ratio", 
"Re-add source", 
"Really exit?", 
"Really remove?", 
"Redirect", 
"Redirect request received to a hub that's already connected", 
"Redirect user(s)", 
"&Refresh", 
"Refresh user list", 
"Reliability", 
"&Remove", 
"Remove all", 
"Remove all subdirectories before adding this one", 
"Remove user from queue", 
"Remove source", 
"Rollback inconsistency, existing file does not match the one being downloaded", 
"Running...", 
"Search", 
"Search for", 
"Search for alternates", 
"Search for file", 
"Search options", 
"Search spam detected from ", 
"Search Spy", 
"Search String", 
"Searching for ", 
"Ready to search...", 
"Searching too soon, next search in %i seconds", 
"Request to seek beyond the end of data", 
"Send private message", 
"Separator", 
"Server", 
"Set priority", 
"Settings", 
"Add finished files to share instantly (if shared)", 
"&Add folder", 
"Break on first ADLSearch match", 
"Advanced", 
"Advanced\\Experts only", 
"Advanced resume using TTH", 
"Advanced settings", 
"Use antifragmentation method for downloads", 
"Appearance", 
"Colors and sounds", 
"Auto-away on minimize (and back on restore)", 
"Automatically follow redirects", 
"Automatically disconnect users who leave the hub (does not disconnect when hub goes down / you leave it)", 
"Automatically search for alternative download locations", 
"Automatically match queue for auto search hits", 
"Automatically refresh share list every hour", 
"Auto-open at startup", 
"Bind address", 
"&Change", 
"Clear search box after each search", 
"Colors", 
"Command", 
"Enable safe and compressed transfers", 
"Configure Public Hub Lists", 
"Confirm application exit", 
"Confirm favorite hub removal", 
"Confirm item removal in download queue", 
"Connection Settings (see the help file if unsure)", 
"Connection Type", 
"Default away message", 
"Directories", 
"Don't download files already in share", 
"Default download directory", 
"Limits", 
"Downloads", 
"Maximum simultaneous downloads (0 = infinite)", 
"No new downloads if speed exceeds (KiB/s, 0 = disable)", 
"Donate €€€:s! (ok, dirty dollars are fine as well =) (see help menu)", 
"Only show joins / parts for favorite users", 
"Downloads\\Favorites", 
"Favorite download directories", 
"Filename", 
"Filter kick and NMDC debug messages", 
"Set Finished Manager(s) tab bold when an entry is added", 
"Format", 
"Personal information ", 
"Get User Country", 
"Accept custom user commands from hub", 
"Ignore private messages from offline users", 
"IP", 
"Don't delete file lists when exiting", 
"Language file", 
"Keep duplicate files in your file list (duplicates never count towards your share size)", 
"Log downloads", 
"Log filelist transfers", 
"Log main chat", 
"Log private chat", 
"Log status messages", 
"Log system messages", 
"Log uploads", 
"Logging", 
"Logs", 
"Ask what to do when a magnet link is detected.", 
"Max hash speed", 
"Max tab rows", 
"Minimize to tray", 
"Name", 
"Connection settings", 
"Don't send the away message to bots", 
"Note; Files appear in the share only after they've been hashed!", 
"Search for files with TTH root only as standard", 
"Open new window when using /join", 
"Always open help file with this dialog", 
"Options", 
"Passive", 
"Personal Information", 
"Make an annoying sound every time a private message is received", 
"Make an annoying sound when a private message window is opened", 
"PM history", 
"Open new file list windows in the background", 
"Open new private message windows in the background", 
"Open private messages from offline users in their own window", 
"Open private messages in their own window", 
"Public Hubs list", 
"HTTP Proxy (for hublist only)", 
"Public Hubs list URL", 
"Set Download Queue tab bold when contents change", 
"Rename", 
"Note; most of these options require that you restart DC++", 
"Rollback", 
"Search history", 
"Select &text style", 
"Select &window color", 
"Send unknown /commands to the hub", 
"Enable automatic SFV checking", 
"Share hidden files", 
"Total size:", 
"Shared directories", 
"Show joins / parts in chat by default", 
"Show progress bars for transfers (uses some CPU)", 
"Skip zero-byte files", 
"Use small send buffer (enable if uploads slow downloads a lot)", 
"SOCKS5", 
"Socks IP", 
"Port", 
"Use SOCKS5 server to resolve hostnames", 
"Username", 
"Sounds", 
"Note; because of changing download speeds, this is not 100% accurate...", 
"View status messages in main chat", 
"Tab completion of nicks in chat", 
"Set hub/PM/Search tab bold when contents change", 
"TCP Port", 
"Mini slot size", 
"Show timestamps in chat by default", 
"Set timestamps", 
"Toggle window when selecting an active tab", 
"UDP Port", 
"Unfinished downloads directory", 
"Sharing", 
"Automatically open an extra slot if speed is below (0 = disable)", 
"Upload slots", 
"Register with Windows to handle dchub:// and adc:// URL links", 
"Register with Windows to handle magnet: URI links", 
"Use CTRL for line history", 
"Use OEM monospaced font for viewing text files", 
"Use system icons when browsing files (slows browsing down a bit)", 
"Use UPnP Control", 
"Advanced\\User Commands", 
"Windows / Startup", 
"Write buffer size", 
"CRC32 inconsistency (SFV-Check)", 
"Shared", 
"Shared Files", 
"Size", 
"Max Size", 
"Min Size", 
"New virtual name matches old name, skipping...", 
"Slot granted", 
"Slots", 
"Slots set", 
"Socket has been shut down", 
"Socks server authentication failed (bad username / password?)", 
"The socks server doesn't support user / password authentication", 
"The socks server failed establish a connection", 
"The socks server requires authentication", 
"Failed to set up the socks server for UDP relay (check socks address and port)", 
"Source Type", 
"Specify a search string", 
"Specify a server to connect to", 
"Speed", 
"Status", 
"Stored password sent...", 
"Tag", 
"Target filename too long", 
"TiB", 
"Time", 
"Time left", 
"Timestamps disabled", 
"Timestamps enabled", 
"More data was sent than was expected", 
"Total: ", 
"A file with the same hash already exists in your share", 
"TTH inconsistency", 
"TTH Root", 
"Type", 
"Unable to create thread", 
"Unknown", 
"Unknown address", 
"Unknown command: ", 
"Unknown error: 0x%x", 
"Unsupported filelist format", 
"Upload finished, idle...", 
"Upload starting...", 
"Uploaded %s (%.01f%%) in %s", 
" uploaded to ", 
"Uploads", 
"Failed to create port mappings. Please set up your NAT yourself.", 
"Failed to remove port mappings", 
"Failed to get external IP via  UPnP. Please set it yourself.", 
"User", 
"Chat", 
"Command", 
"Context", 
"Filelist Menu", 
"Hub IP / DNS (empty = all, 'op' = where operator)", 
"Hub Menu", 
"Send once per nick", 
"Parameters", 
"PM", 
"Text sent to hub", 
"Raw", 
"Search Menu", 
"To", 
"Command Type", 
"User Menu", 
"Create / Modify Command", 
"User Description", 
"User offline", 
"User went offline", 
"Users", 
"Video", 
"View as text", 
"Virtual name", 
"Virtual directory name already exists", 
"Name under which the others see the directory", 
"Waiting...", 
"Waiting to retry...", 
"Waiting (User online)", 
"Waiting (%d of %d users online)", 
"Yes", 
"You are being redirected to ", 
};
string ResourceManager::names[] = {
"Active", 
"ActiveSearchString", 
"Add", 
"AddToFavorites", 
"Added", 
"AddressAlreadyInUse", 
"AddressNotAvailable", 
"AdlSearch", 
"AdlsDestination", 
"AdlsDiscard", 
"AdlsDownload", 
"AdlsEnabled", 
"AdlsFullPath", 
"AdlsProperties", 
"AdlsSearchString", 
"AdlsSizeMax", 
"AdlsSizeMin", 
"AdlsType", 
"AdlsUnits", 
"AllDownloadSlotsTaken", 
"AllUsersOffline", 
"All3UsersOffline", 
"All4UsersOffline", 
"All", 
"Any", 
"AtLeast", 
"AtMost", 
"Audio", 
"AutoConnect", 
"AutoGrant", 
"Average", 
"Away", 
"AwayModeOff", 
"AwayModeOn", 
"B", 
"BanUser", 
"BothUsersOffline", 
"Browse", 
"BrowseAccel", 
"BrowseFileList", 
"ChooseFolder", 
"Close", 
"CloseConnection", 
"ClosingConnection", 
"Compressed", 
"CompressionError", 
"Configure", 
"Connect", 
"Connected", 
"Connecting", 
"ConnectingForced", 
"ConnectingTo", 
"Connection", 
"ConnectionClosed", 
"ConnectionRefused", 
"ConnectionReset", 
"ConnectionTimeout", 
"ConfiguredHubLists", 
"CopyHub", 
"CopyMagnet", 
"CopyNick", 
"CouldNotOpenTargetFile", 
"Count", 
"Country", 
"CrcChecked", 
"DecompressionError", 
"Description", 
"Destination", 
"Directory", 
"DirectoryAlreadyShared", 
"DirectoryAddError", 
"DiscFull", 
"DisconnectUser", 
"Disconnected", 
"DisconnectedUser", 
"Document", 
"Done", 
"DontRemoveSlashPassword", 
"DontShareTempDirectory", 
"Download", 
"DownloadFailed", 
"DownloadFinishedIdle", 
"DownloadQueue", 
"DownloadStarting", 
"DownloadTo", 
"DownloadWholeDir", 
"DownloadWholeDirTo", 
"Downloaded", 
"DownloadedBytes", 
"DownloadedFrom", 
"Downloading", 
"DownloadingHubList", 
"DownloadingList", 
"Downloads", 
"DuplicateFileNotShared", 
"DuplicateMatch", 
"DuplicateSource", 
"Edit", 
"EditAccel", 
"Email", 
"EnterNick", 
"EnterPassword", 
"EnterReason", 
"EnterSearchString", 
"EnterServer", 
"Errors", 
"ExactSize", 
"Executable", 
"FavJoinShowingOff", 
"FavJoinShowingOn", 
"FavoriteDirName", 
"FavoriteDirNameLong", 
"FavoriteHubAdded", 
"FavoriteHubIdentity", 
"FavoriteHubProperties", 
"FavoriteHubs", 
"FavoriteUserAdded", 
"FavoriteUsers", 
"File", 
"Files", 
"FileListRefreshFinished", 
"FileListRefreshInitiated", 
"FileNotAvailable", 
"FileType", 
"FileWithDifferentSize", 
"FileWithDifferentTth", 
"Filename", 
"FilesLeft", 
"FilesPerHour", 
"Filter", 
"Filtered", 
"Find", 
"FinishedDownloads", 
"FinishedUploads", 
"ForbiddenDollarFile", 
"ForceAttempt", 
"Gib", 
"GetFileList", 
"GoToDirectory", 
"GrantExtraSlot", 
"HashDatabase", 
"HashProgress", 
"HashProgressBackground", 
"HashProgressStats", 
"HashProgressText", 
"HashRebuilt", 
"HashingFinished", 
"High", 
"Highest", 
"HitRatio", 
"Hits", 
"HostUnreachable", 
"Hub", 
"Hubs", 
"HubAddress", 
"HubListDownloaded", 
"HubName", 
"HubPassword", 
"HubUsers", 
"IgnoreTthSearches", 
"IgnoredMessage", 
"InvalidNumberOfSlots", 
"InvalidTargetFile", 
"InvalidTree", 
"Ip", 
"IpBare", 
"Items", 
"JoinShowingOff", 
"JoinShowingOn", 
"Joins", 
"Kib", 
"Kibps", 
"KickUser", 
"LargerTargetFileExists", 
"LastChange", 
"LastHub", 
"LastSeen", 
"Left", 
"Loading", 
"LookupAtBitzi", 
"Low", 
"Lowest", 
"MagnetDlgFile", 
"MagnetDlgHash", 
"MagnetDlgNothing", 
"MagnetDlgQueue", 
"MagnetDlgRemember", 
"MagnetDlgSearch", 
"MagnetDlgTextBad", 
"MagnetDlgTextGood", 
"MagnetDlgTitle", 
"MagnetHandlerDesc", 
"MagnetHandlerRoot", 
"MagnetShellDesc", 
"MatchQueue", 
"MatchedFiles", 
"MaxHubs", 
"MaxUsers", 
"Mib", 
"Mibps", 
"MenuAbout", 
"MenuAdlSearch", 
"MenuArrange", 
"MenuCascade", 
"MenuChangelog", 
"MenuCloseDisconnected", 
"MenuContents", 
"MenuDiscuss", 
"MenuDonate", 
"MenuDownloadQueue", 
"MenuExit", 
"MenuFaq", 
"MenuFavoriteHubs", 
"MenuFavoriteUsers", 
"MenuFile", 
"MenuFollowRedirect", 
"MenuHashProgress", 
"MenuHelp", 
"MenuHelpDownloads", 
"MenuHelpForum", 
"MenuHelpTranslations", 
"MenuHomepage", 
"MenuHorizontalTile", 
"MenuMinimizeAll", 
"MenuRestoreAll", 
"MenuNetworkStatistics", 
"MenuNotepad", 
"MenuOpenDownloadsDir", 
"MenuOpenFileList", 
"MenuOpenOwnList", 
"MenuPublicHubs", 
"MenuQuickConnect", 
"MenuReadme", 
"MenuReconnect", 
"MenuRefreshFileList", 
"MenuReportBug", 
"MenuRequestFeature", 
"MenuSearch", 
"MenuSearchSpy", 
"MenuSettings", 
"MenuShow", 
"MenuStatusBar", 
"MenuToolbar", 
"MenuTransferView", 
"MenuVerticalTile", 
"MenuView", 
"MenuWindow", 
"MinShare", 
"MinSlots", 
"Move", 
"MoveDown", 
"MoveUp", 
"NetworkStatistics", 
"NetworkUnreachable", 
"New", 
"Next", 
"Nick", 
"NickTaken", 
"NickUnknown", 
"NoStr", 
"NoDirectorySpecified", 
"NoDownloadsFromSelf", 
"NoErrors", 
"NoMatches", 
"NoSlotsAvailable", 
"NoUsers", 
"NoUsersToDownloadFrom", 
"NonBlockingOperation", 
"Normal", 
"NotConnected", 
"NotSocket", 
"Notepad", 
"Offline", 
"Online", 
"OnlyFreeSlots", 
"OnlyTth", 
"OnlyWhereOp", 
"Open", 
"OpenDownloadPage", 
"OpenFolder", 
"OperatingSystemNotCompatible", 
"OperationWouldBlockExecution", 
"OutOfBufferSpace", 
"Parts", 
"PassiveUser", 
"Password", 
"Path", 
"Paused", 
"PermissionDenied", 
"Pib", 
"Picture", 
"Port", 
"PortIsBusy", 
"PreparingFileList", 
"PressFollow", 
"Priority", 
"PrivateMessageFrom", 
"Properties", 
"PublicHubs", 
"Purge", 
"QuickConnect", 
"Rating", 
"Ratio", 
"ReaddSource", 
"ReallyExit", 
"ReallyRemove", 
"Redirect", 
"RedirectAlreadyConnected", 
"RedirectUser", 
"Refresh", 
"RefreshUserList", 
"Reliability", 
"Remove", 
"RemoveAll", 
"RemoveAllSubdirectories", 
"RemoveFromAll", 
"RemoveSource", 
"RollbackInconsistency", 
"Running", 
"Search", 
"SearchFor", 
"SearchForAlternates", 
"SearchForFile", 
"SearchOptions", 
"SearchSpamFrom", 
"SearchSpy", 
"SearchString", 
"SearchingFor", 
"SearchingReady", 
"SearchingWait", 
"SeekBeyondEnd", 
"SendPrivateMessage", 
"Separator", 
"Server", 
"SetPriority", 
"Settings", 
"SettingsAddFinishedInstantly", 
"SettingsAddFolder", 
"SettingsAdlsBreakOnFirst", 
"SettingsAdvanced", 
"SettingsAdvanced3", 
"SettingsAdvancedResume", 
"SettingsAdvancedSettings", 
"SettingsAntiFrag", 
"SettingsAppearance", 
"SettingsAppearance2", 
"SettingsAutoAway", 
"SettingsAutoFollow", 
"SettingsAutoKick", 
"SettingsAutoSearch", 
"SettingsAutoSearchAutoMatch", 
"SettingsAutoUpdateList", 
"SettingsAutoOpen", 
"SettingsBindAddress", 
"SettingsChange", 
"SettingsClearSearch", 
"SettingsColors", 
"SettingsCommand", 
"SettingsCompressTransfers", 
"SettingsConfigureHubLists", 
"SettingsConfirmExit", 
"SettingsConfirmHubRemoval", 
"SettingsConfirmItemRemoval", 
"SettingsConnectionSettings", 
"SettingsConnectionType", 
"SettingsDefaultAwayMsg", 
"SettingsDirectories", 
"SettingsDontDlAlreadyShared", 
"SettingsDownloadDirectory", 
"SettingsDownloadLimits", 
"SettingsDownloads", 
"SettingsDownloadsMax", 
"SettingsDownloadsSpeedPause", 
"SettingsExampleText", 
"SettingsFavShowJoins", 
"SettingsFavoriteDirsPage", 
"SettingsFavoriteDirs", 
"SettingsFileName", 
"SettingsFilterMessages", 
"SettingsFinishedDirty", 
"SettingsFormat", 
"SettingsGeneral", 
"SettingsGetUserCountry", 
"SettingsHubUserCommands", 
"SettingsIgnoreOffline", 
"SettingsIp", 
"SettingsKeepLists", 
"SettingsLanguageFile", 
"SettingsListDupes", 
"SettingsLogDownloads", 
"SettingsLogFilelistTransfers", 
"SettingsLogMainChat", 
"SettingsLogPrivateChat", 
"SettingsLogStatusMessages", 
"SettingsLogSystemMessages", 
"SettingsLogUploads", 
"SettingsLogging", 
"SettingsLogs", 
"SettingsMagnetAsk", 
"SettingsMaxHashSpeed", 
"SettingsMaxTabRows", 
"SettingsMinimizeTray", 
"SettingsName", 
"SettingsNetwork", 
"SettingsNoAwaymsgToBots", 
"SettingsOnlyHashed", 
"SettingsOnlyTth", 
"SettingsOpenNewWindow", 
"SettingsOpenUserCmdHelp", 
"SettingsOptions", 
"SettingsPassive", 
"SettingsPersonalInformation", 
"SettingsPmBeep", 
"SettingsPmBeepOpen", 
"SettingsPmHistory", 
"SettingsPopunderFilelist", 
"SettingsPopunderPm", 
"SettingsPopupOffline", 
"SettingsPopupPms", 
"SettingsPublicHubList", 
"SettingsPublicHubListHttpProxy", 
"SettingsPublicHubListUrl", 
"SettingsQueueDirty", 
"SettingsRenameFolder", 
"SettingsRequiresRestart", 
"SettingsRollback", 
"SettingsSearchHistory", 
"SettingsSelectTextFace", 
"SettingsSelectWindowColor", 
"SettingsSendUnknownCommands", 
"SettingsSfvCheck", 
"SettingsShareHidden", 
"SettingsShareSize", 
"SettingsSharedDirectories", 
"SettingsShowJoins", 
"SettingsShowProgressBars", 
"SettingsSkipZeroByte", 
"SettingsSmallSendBuffer", 
"SettingsSocks5", 
"SettingsSocks5Ip", 
"SettingsSocks5Port", 
"SettingsSocks5Resolve", 
"SettingsSocks5Username", 
"SettingsSounds", 
"SettingsSpeedsNotAccurate", 
"SettingsStatusInChat", 
"SettingsTabCompletion", 
"SettingsTabDirty", 
"SettingsTcpPort", 
"SettingsTextMinislot", 
"SettingsTimeStamps", 
"SettingsTimeStampsFormat", 
"SettingsToggleActiveWindow", 
"SettingsUdpPort", 
"SettingsUnfinishedDownloadDirectory", 
"SettingsUploads", 
"SettingsUploadsMinSpeed", 
"SettingsUploadsSlots", 
"SettingsUrlHandler", 
"SettingsUrlMagnet", 
"SettingsUseCtrlForLineHistory", 
"SettingsUseOemMonofont", 
"SettingsUseSystemIcons", 
"SettingsUseUpnp", 
"SettingsUserCommands", 
"SettingsWindows", 
"SettingsWriteBuffer", 
"SfvInconsistency", 
"Shared", 
"SharedFiles", 
"Size", 
"SizeMax", 
"SizeMin", 
"SkipRename", 
"SlotGranted", 
"Slots", 
"SlotsSet", 
"SocketShutDown", 
"SocksAuthFailed", 
"SocksAuthUnsupported", 
"SocksFailed", 
"SocksNeedsAuth", 
"SocksSetupError", 
"SourceType", 
"SpecifySearchString", 
"SpecifyServer", 
"Speed", 
"Status", 
"StoredPasswordSent", 
"Tag", 
"TargetFilenameTooLong", 
"Tib", 
"Time", 
"TimeLeft", 
"TimestampsDisabled", 
"TimestampsEnabled", 
"TooMuchData", 
"Total", 
"TthAlreadyShared", 
"TthInconsistency", 
"TthRoot", 
"Type", 
"UnableToCreateThread", 
"Unknown", 
"UnknownAddress", 
"UnknownCommand", 
"UnknownError", 
"UnsupportedFilelistFormat", 
"UploadFinishedIdle", 
"UploadStarting", 
"UploadedBytes", 
"UploadedTo", 
"Uploads", 
"UpnpFailedToCreateMappings", 
"UpnpFailedToRemoveMappings", 
"UpnpFailedToGetExternalIp", 
"User", 
"UserCmdChat", 
"UserCmdCommand", 
"UserCmdContext", 
"UserCmdFilelistMenu", 
"UserCmdHub", 
"UserCmdHubMenu", 
"UserCmdOnce", 
"UserCmdParameters", 
"UserCmdPm", 
"UserCmdPreview", 
"UserCmdRaw", 
"UserCmdSearchMenu", 
"UserCmdTo", 
"UserCmdType", 
"UserCmdUserMenu", 
"UserCmdWindow", 
"UserDescription", 
"UserOffline", 
"UserWentOffline", 
"Users", 
"Video", 
"ViewAsText", 
"VirtualName", 
"VirtualNameExists", 
"VirtualNameLong", 
"Waiting", 
"WaitingToRetry", 
"WaitingUserOnline", 
"WaitingUsersOnline", 
"YesStr", 
"YouAreBeingRedirected", 
};
