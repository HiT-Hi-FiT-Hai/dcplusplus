// @Prolog: #include "stdinc.h"
// @Prolog: #include "DCPlusPlus.h"
// @Strings: string ResourceManager::strings[]
// @Names: string ResourceManager::names[]

enum Strings { // @DontAdd
	ADD_TO_FAVORITES, // "Add To Favorites"
	ADDRESS_ALREADY_IN_USE, // "Address already in use"
	ADDRESS_NOT_AVAILABLE, // "Address not available"
	ALL_DOWNLOAD_SLOTS_TAKEN, // "All download slots taken"
	ALL_USERS_OFFLINE, // "All %d users offline"
	ALREADY_GETTING_THAT_LIST, // "Already getting that file list"
	ANY, // "Any"
	AT_LEAST, // "At least"
	AT_MOST, // "At most"
	AUDIO, // "Audio"
	AUTO_CONNECT, // "Auto connect / Name"
	AVERAGE, // "Average/s: "
	AWAY_MODE_OFF, // "Away mode off"
	AWAY_MODE_ON, // "Away mode on: "
	B, // "B"
	BOTH_USERS_OFFLINE, // "Both users offline"
	BROWSE, // "Browse..."
	CANT_CONNECT_IN_PASSIVE_MODE, // "Can't connect to passive user while in passive mode"
	CHOOSE_FOLDER, // "Choose folder"
	CLOSE_CONNECTION, // "Close connection"
	CLOSING_CONNECTION, // "Closing connection..."
	CLOSE, // "Close"
	COMPRESSED, // "Compressed"
	CONNECT, // "Connect"
	CONNECTED, // "Connected"
	CONNECTING, // "Connecting..."
	CONNECTING_FORCED, // "Connecting (forced)..."
	CONNECTING_TO, // "Connecting to "
	CONNECTION, // "Connection"
	CONNECTION_CLOSED, // "Connection closed"
	CONNECTION_REFUSED, // "Connection refused by target machine"
	CONNECTION_RESET, // "Connection reset by server"
	CONNECTION_TIMEOUT, // "Connection timeout"
	COULD_NOT_OPEN_TARGET_FILE, // "Could not open target file: "
	COULD_NOT_OPEN_FILE, // "Could not open file"
	COUNT, // "Count"
	DESCRIPTION, // "Description"
	DIRECTORY_ALREADY_SHARED, // "Directory already shared"
	DISCONNECTED, // "Disconnected"
	DISC_FULL, // "Disk full(?)"
	DOCUMENT, // "Document"
	DONE, // "Done"
	DOWNLOAD, // "Download"
	DOWNLOAD_FAILED, // "Download failed: "
	DOWNLOAD_FINISHED_IDLE, // "Download finished, idle..."
	DOWNLOAD_STARTING, // "Download starting..."
	DOWNLOAD_TO, // "Download to..."
	DOWNLOAD_QUEUE, // "Download queue"
	DOWNLOADED_LEFT, // "Downloaded %s (%.01f%%), %s/s, %s left"
	DOWNLOADED_FROM, // " downloaded from "
	DOWNLOADING, // "Downloading..."
	DOWNLOADING_HUB_LIST, // "Downloading public hub list..."
	EMAIL, // "E-Mail"
	ENTER_NICK, // "Please enter a nickname in the settings dialog!"
	ENTER_PASSWORD, // "Please enter a password"
	ENTER_REASON, // "Please enter a reason"
	ENTER_SEARCH_STRING, // "Enter search string"
	ENTER_SERVER, // "Please enter a destination server"
	ERROR_OPENING_FILE, // "Error opening file"
	EXECUTABLE, // "Executable"
	FAVORITE_HUBS, // "Favorite Hubs"
	FAVORITE_USERS, // "Favorite Users"
	FILE, // "File"
	FILES, // "Files"
	FILE_LIST_REFRESHED, // "File list refreshed"
	FILE_NOT_AVAILABLE, // "File not available"
	FILE_TYPE, // "File type"
	FILE_WITH_DIFFERENT_SIZE, // "A file with a different size already exists in the queue"
	FILENAME, // "Filename"
	FILTER, // "Filter"
	FIND, // "Find"
	FORCE_ATTEMPT, // "Force attempt"
	GB, // "GB"
	GET_FILE_LIST, // "Get file list"
	GRANT_EXTRA_SLOT, // "Grant extra slot"
	HIGH, // "High"
	HIGHEST, // "Highest"
	HIT_RATIO, // "Hit Ratio: "
	HITS, // "Hits: "
	HOST_UNREACHABLE, // "Host unreachable"
	HUB, // "Hub"
	HUBS, // "Hubs"
	HUB_ADDRESS, // "Address"
	HUB_LIST_DOWNLOADED, // "Hub list downloaded..."
	HUB_NAME, // "Name"
	HUB_PASSWORD, // "Hub password"
	HUB_USERS, // "Users"
	IGNORED_MESSAGE, // "Ignored message: "
	IMPORT_QUEUE, // "Import queue from NMDC"
	INVALID_NUMBER_OF_SLOTS, // "Invalid number of slots"
	JOINS, // "Joins: "
	JOIN_SHOWING_OFF, // "Join/part showing off"
	JOIN_SHOWING_ON, // "Join/part showing on"
	KB, // "kB"
	KICK_USER, // "Kick user(s)"
	LARGER_TARGET_FILE_EXISTS, // "A file of equal or larger size already exists at the target location"
	LAST_HUB, // "Hub (last seen on if offline)"
	LOADING, // "Loading DC++, please wait..."
	LOW, // "Low"
	LOWEST, // "Lowest"
	MANUAL_ADDRESS, // "Manual connect address"
	MB, // "MB"
	MENU_FILE, // "&File"
	MENU_FILE_DOWNLOAD_QUEUE, // "&Download Queue\tCtrl+D"
	MENU_FILE_EXIT, // "&Exit"
	MENU_FILE_FAVORITE_HUBS, // "&Favorite Hubs\tCtrl+F"
	MENU_FILE_FAVORITE_USERS, // "Favorite &Users\tCtrl+U"
	MENU_FILE_FOLLOW_REDIRECT, // "Follow last redirec&t\tCtrl+T"
	MENU_FILE_NOTEPAD, // "&Notepad\tCtrl+N"
	MENU_FILE_PUBLIC_HUBS, // "&Public Hubs\tCtrl+P"
	MENU_FILE_RECONNECT, // "&Reconnect\tCtrl+R"
	MENU_FILE_SEARCH, // "&Search\tCtrl+S"
	MENU_FILE_SEARCH_SPY, // "Search spy"
	MENU_FILE_SETTINGS, // "Settings..."
	MENU_HELP, // "&Help"
	MENU_HELP_ABOUT, // "About DC++..."
	MENU_HELP_DISCUSS, // "DC++ discussion forum"
	MENU_HELP_DOWNLOADS, // "Downloads and translations"
	MENU_HELP_FAQ, // "Frequently asked questions"
	MENU_HELP_HELP_FORUM, // "Help forum"
	MENU_HELP_HOMEPAGE, // "DC++ Homepage"
	MENU_HELP_README, // "Readme / Newbie help"
	MENU_HELP_REQUEST_FEATURE, // "Request a feature"
	MENU_HELP_REPORT_BUG, // "Report a bug"
	MENU_VIEW, // "&View"
	MENU_VIEW_STATUS_BAR, // "&Status bar"
	MENU_VIEW_TOOLBAR, // "&Toolbar"
	MENU_WINDOW, // "&Window"
	MENU_WINDOW_MINIMIZE_ALL, // "Minimize &All"
	MENU_WINDOW_ARRANGE, // "Arrange icons"
	MENU_WINDOW_CASCADE, // "Cascade"
	MENU_WINDOW_TILE, // "Tile"
	NEXT, // "Next"
	NEW, // "New..."
	NICK, // "Nick"
	NICK_TAKEN, // "Your nick was already taken, please change to something else!"
	NICK_UNKNOWN, // " (Nick unknown)"
	NON_BLOCKING_OPERATION, // "Non-blocking operation still in progress"
	NOT_CONNECTED, // "Not connected"
	NOT_SOCKET, // "Not a socket"
	NO_DIRECTORY_SPECIFIED, // "No directory specified"
	NO_DOWNLOADS_FROM_SELF, // "You're trying to download from yourself!"
	NO_MATCHES, // "No matches"
	NO_SLOTS_AVAILABLE, // "No slots available"
	NO_USERS, // "No users"
	NO_USERS_TO_DOWNLOAD_FROM, // "No users to download from"
	NORMAL, // "Normal"
	NOTEPAD, // "Notepad"
	OFFLINE, // "Offline"
	ONLINE, // "Online"
	ONLY_FREE_SLOTS, // "Only users with free slots"
	OPEN_FILE_LIST, // "Open file list"
	OPERATION_WOULD_BLOCK_EXECUTION, // "Operation would block execution"
	OUT_OF_BUFFER_SPACE, // "Out of buffer space"
	PASSWORD, // "Password"
	PARTS, // "Parts: "
	PATH, // "Path"
	PAUSED, // "Paused"
	PERMISSION_DENIED, // "Permission denied"
	PICTURE, // "Picture"
	PORT_IS_BUSY, // "Port %d is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++"
	PREPARING_FILE_LIST, // "Preparing file list..."
	PRESS_FOLLOW, // "Press the follow redirect button to connect to "
	PRIORITY, // "Priority"
	PRIVATE_MESSAGE_FROM, // "Private message from "
	PROPERTIES, // "Properties"
	PUBLIC_HUBS, // "Public Hubs"
	REALLY_EXIT, // "Really exit?"
	REDIRECT, // "Redirect"
	REDIRECT_ALREADY_CONNECTED, // "Redirect request received to a hub that's already connected"
	REDIRECT_USER, // "Redirect user(s)"
	REFRESH, // "Refresh"
	REFRESH_USER_LIST, // "Refresh user list"
	REMOVE, // "Remove"
	REMOVE_ALL_SUBDIRECTORIES, // "Remove all subdirectories before adding this one"
	REMOVE_SOURCE, // "Remove source"
	ROLLBACK_INCONSISTENCY, // "Rollback inconsistency, existing file does not match the one being downloaded"
	RUNNING, // "Running..."
	SEARCH, // "Search"
	SEARCH_FOR, // "Search for"
	SEARCH_FOR_ALTERNATES, // "Search for alternates"
	SEARCH_FOR_FILE, // "Search for file"
	SEARCH_OPTIONS, // "Search options"
	SEARCH_SPAM_FROM, // "Search spam detected from "
	SEARCH_SPY, // "Search Spy"
	SEARCH_STRING, // "Search string"
	SEARCHING_FOR, // "Searching for "
	SEND_PRIVATE_MESSAGE, // "Send private message"
	SERVER, // "Server"
	SET_PRIORITY, // "Set priority"
	SHARED, // "Shared"
	SHARED_FILES, // "Shared Files"
	SIZE, // "Size"
	SLOT_GRANTED, // "Slot granted"
	SLOTS, // "Slots"
	SLOTS_SET, // "Slots set"
	SOCKET_SHUT_DOWN, // "Socket has been shut down"
	SPECIFY_SERVER, // "Specify a server to connect to"
	SPECIFY_SEARCH_STRING, // "Specify a search string"
	STATUS, // "Status"
	TB, // "TB"
	TIMESTAMPS_DISABLED, // "Timestamps disabled"
	TIMESTAMPS_ENABLED, // "Timestamps enabled"
	TOTAL, // "Total: "
	TYPE, // "Type"
	UNABLE_TO_CREATE_THREAD, // "Unable to create thread"
	UNKNOWN, // "Unknown"
	UNKNOWN_ADDRESS, // "Unknown address"
	UNKNOWN_ERROR, // "Unknown error: 0x%x"
	UPLOAD_FINISHED_IDLE, // "Upload finished, idle..."
	UPLOAD_STARTING, // "Upload starting..."
	UPLOADED_LEFT, // "Uploaded %s (%.01f%%), %s/s, %s left"
	UPLOADED_TO, // " uploaded to "
	USER, // "User"
	USER_OFFLINE, // "User offline"
	USER_WENT_OFFLINE, // "User went offline"
	USERS, // "Users"
	VIDEO, // "Video"
	WAITING, // "Waiting..."
	WAITING_USER_ONLINE, // "Waiting (User online)"
	WAITING_USERS_ONLINE, // "Waiting (%d of %d users online)"
	WAITING_TO_RETRY, // "Waiting to retry..."
	YOU_ARE_BEING_REDIRECTED, // "You are being redirected to "
	LAST // @DontAdd
};
