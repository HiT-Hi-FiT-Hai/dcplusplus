// @Prolog: #include "stdinc.h"
// @Prolog: #include "DCPlusPlus.h"
// @Prolog: #include "ResourceManager.h"
// @Strings: string ResourceManager::strings[]
// @Names: string ResourceManager::names[]

enum Strings { // @DontAdd
	ACTIVE, // "Active"
	ACTIVE_SEARCH_STRING, // "Enabled / Search String"
	ADD, // "&Add"
	ADD_TO_FAVORITES, // "Add To Favorites"
	ADDED, // "Added"
	ADDRESS_ALREADY_IN_USE, // "Address already in use"
	ADDRESS_NOT_AVAILABLE, // "Address not available"
	ADL_SEARCH, // "Automatic Directory Listing Search"
	ADLS_DESTINATION, // "Destination Directory"
	ADLS_DISCARD, // "Discard"
	ADLS_DOWNLOAD, // "Download Matches"
	ADLS_ENABLED, // "Enabled"
	ADLS_FULL_PATH, // "Full Path"
	ADLS_PROPERTIES, // "ADLSearch Properties"
	ADLS_SEARCH_STRING, // "Search String"
	ADLS_SIZE_MAX, // "Max FileSize"
	ADLS_SIZE_MIN, // "Min FileSize"
	ADLS_TYPE, // "Search Type"
	ADLS_UNITS, // "Size Type"
	ALL_DOWNLOAD_SLOTS_TAKEN, // "All download slots taken"
	ALL_USERS_OFFLINE, // "All %d users offline"
	ALL_3_USERS_OFFLINE, // "All 3 users offline"
	ALL_4_USERS_OFFLINE, // "All 4 users offline"
	ANY, // "Any"
	AT_LEAST, // "At least"
	AT_MOST, // "At most"
	AUDIO, // "Audio"
	AUTO_CONNECT, // "Auto connect / Name"
	AUTO_GRANT, // "Auto grant slot / Nick"
	AVERAGE, // "Average/s: "
	AWAY, // "AWAY"
	AWAY_MODE_OFF, // "Away mode off"
	AWAY_MODE_ON, // "Away mode on: "
	B, // "B"
	BAN_USER, // "Ban user(s)"
	BOTH_USERS_OFFLINE, // "Both users offline"
	BROWSE, // "Browse..."
	BROWSE_ACCEL, // "&Browse..."
	CHOOSE_FOLDER, // "Choose folder"
	CLOSE, // "Close"
	CLOSE_CONNECTION, // "Close connection"
	CLOSING_CONNECTION, // "Closing connection..."
	COMPRESSED, // "Compressed"
	COMPRESSION_ERROR, // "Error during compression"
	CONNECT, // "&Connect"
	CONNECTED, // "Connected"
	CONNECTING, // "Connecting..."
	CONNECTING_FORCED, // "Connecting (forced)..."
	CONNECTING_TO, // "Connecting to "
	CONNECTION, // "Connection"
	CONNECTION_CLOSED, // "Connection closed"
	CONNECTION_REFUSED, // "Connection refused by target machine"
	CONNECTION_RESET, // "Connection reset by server"
	CONNECTION_TIMEOUT, // "Connection timeout"
	COPY_HUB, // "Copy address to clipboard"
	COPY_MAGNET, // "Copy magnet link to clipboard"
	COPY_NICK, // "Copy nick to clipboard"
	COULD_NOT_OPEN_TARGET_FILE, // "Could not open target file: "
	COUNT, // "Count"
	COUNTRY, // "Country"
	CRC_CHECKED, // "CRC Checked"
	DECOMPRESSION_ERROR, // "Error during decompression"
	DESCRIPTION, // "Description"
	DESTINATION, // "Destination"
	DIRECTORY, // "Directory"
	DIRECTORY_ALREADY_SHARED, // "Directory already shared"
	DISC_FULL, // "Disk full(?)"
	DISCONNECT_USER, // "Disconnect user(s)"
	DISCONNECTED, // "Disconnected"
	DISCONNECTED_USER, // "Disconnected user leaving the hub: "
	DOCUMENT, // "Document"
	DONE, // "Done"
	DONT_SHARE_TEMP_DIRECTORY, // "The temporary download directory cannot be shared"
	DOWNLOAD, // "Download"
	DOWNLOAD_FAILED, // "Download failed: "
	DOWNLOAD_FINISHED_IDLE, // "Download finished, idle..."
	DOWNLOAD_QUEUE, // "Download Queue"
	DOWNLOAD_STARTING, // "Download starting..."
	DOWNLOAD_TO, // "Download to..."
	DOWNLOAD_WHOLE_DIR, // "Download whole directory"
	DOWNLOAD_WHOLE_DIR_TO, // "Download whole directory to..."
	DOWNLOADED, // "Downloaded"
	DOWNLOADED_BYTES, // "Downloaded %s (%.01f%%) in %s"
	DOWNLOADED_FROM, // " downloaded from "
	DOWNLOADING, // "Downloading..."
	DOWNLOADING_HUB_LIST, // "Downloading public hub list..."
	DOWNLOADS, // "Downloads"
	DUPLICATE_FILE_NOT_SHARED, // "Duplicate file will not be shared: "
	DUPLICATE_MATCH, // "Dupe matched against: "
	DUPLICATE_SOURCE, // "Duplicate source"
	EDIT, // "Edit"
	EMAIL, // "E-Mail"
	ENTER_NICK, // "Please enter a nickname in the settings dialog!"
	ENTER_PASSWORD, // "Please enter a password"
	ENTER_REASON, // "Please enter a reason"
	ENTER_SEARCH_STRING, // "Enter search string"
	ENTER_SERVER, // "Please enter a destination server"
	ERRORS, // "Errors"
	EXACT_SIZE, // "Exact size"
	EXECUTABLE, // "Executable"
	FAV_JOIN_SHOWING_OFF, // "Join/part of favorite users showing off"
	FAV_JOIN_SHOWING_ON, // "Join/part of favorite users showing on"
	FAVORITE_HUB_ADDED, // "Favorite hub added"
	FAVORITE_HUB_IDENTITY, // "Identification (leave blank for defaults)"
	FAVORITE_HUB_PROPERTIES, // "Favorite Hub Properties"
	FAVORITE_HUBS, // "Favorite Hubs"
	FAVORITE_USER_ADDED, // "Favorite user added"
	FAVORITE_USERS, // "Favorite Users"
	FILE, // "File"
	FILES, // "Files"
	FILE_LIST_REFRESH_FINISHED, // "File list refresh finished"
	FILE_LIST_REFRESH_INITIATED, // "File list refresh initiated"
	FILE_NOT_AVAILABLE, // "File not available"
	FILE_TYPE, // "File type"
	FILE_WITH_DIFFERENT_SIZE, // "A file with a different size already exists in the queue"
	FILE_WITH_DIFFERENT_TTH, // "A file with diffent tth root already exists in the queue"
	FILENAME, // "Filename"
	FILES_LEFT, // "files left"
	FILES_PER_HOUR, // "files/h"
	FILTER, // "F&ilter"
	FILTERED, // "Filtered"
	FIND, // "Find"
	FINISHED_DOWNLOADS, // "Finished Downloads"
	FINISHED_UPLOADS, // "Finished Uploads"
	FORBIDDEN_DOLLAR_FILE, // "File with '$' cannot be downloaded and will not be shared: "
	FORCE_ATTEMPT, // "Force attempt"
	GB, // "GiB"
	GET_FILE_LIST, // "Get file list"
	GO_TO_DIRECTORY, // "Go to directory"
	GRANT_EXTRA_SLOT, // "Grant extra slot"
	HASH_DATABASE, // "Hash database"
	HASH_PROGRESS, // "Creating file index..."
	HASH_PROGRESS_BACKGROUND, // "Run in background"
	HASH_PROGRESS_STATS, // "Statistics"
	HASH_PROGRESS_TEXT, // "Please wait while DC++ indexes your files (they won't be shared until they've been indexed)..."
	HASH_REBUILT, // "Hash database rebuilt"
	HASHING_FINISHED, // "Finished hashing "
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
	INVALID_NUMBER_OF_SLOTS, // "Invalid number of slots"
	INVALID_TARGET_FILE, // "Invalid target file (missing directory, check default download directory setting)"
	INVALID_TREE, // "Downloaded tree does not match TTH root"
	IP, // "IP: "
	IP_BARE, // "IP"
	ITEMS, // "Items"
	JOIN_SHOWING_OFF, // "Join/part showing off"
	JOIN_SHOWING_ON, // "Join/part showing on"
	JOINS, // "Joins: "
	KB, // "KiB"
	KBPS, // "KiB/s"
	KICK_USER, // "Kick user(s)"
	LARGER_TARGET_FILE_EXISTS, // "A file of equal or larger size already exists at the target location"
	LAST_CHANGE, // "Last change: "
	LAST_HUB, // "Hub (last seen on if offline)"
	LAST_SEEN, // "Time last seen"
	LEFT, // "left"
	LOADING, // "Loading DC++, please wait..."
	LOOKUP_AT_BITZI, // "Lookup TTH at Bitzi.com"
	LOW, // "Low"
	LOWEST, // "Lowest"
	MAGNET_DLG_FILE, // "Filename:"
	MAGNET_DLG_HASH, // "File Hash:"
	MAGNET_DLG_NOTHING, // "Do nothing"
	MAGNET_DLG_QUEUE, // "Add this file to your download queue"
	MAGNET_DLG_REMEMBER, // "Do the same action next time without asking"
	MAGNET_DLG_SEARCH, // "Start a search for this file"
	MAGNET_DLG_TEXT_BAD, // "A MAGNET link was given to DC++, but it didn't contain a valid file hash for use on the Direct Connect network.  No action will be taken."
	MAGNET_DLG_TEXT_GOOD, // "DC++ has detected a MAGNET link with a file hash that can be searched for on the Direct Connect network.  What would you like to do?"
	MAGNET_DLG_TITLE, // "MAGNET Link detected"
	MAGNET_HANDLER_DESC, // "Download files from the Direct Connect network"
	MAGNET_HANDLER_ROOT, // "DC++"
	MAGNET_SHELL_DESC, // "URL:MAGNET URI"
	MANUAL_ADDRESS, // "Manual connect address"
	MATCH_QUEUE, // "Match queue"
	MATCHED_FILES, // "Matched %d file(s)"
	MAX_HUBS, // "Max Hubs"
	MAX_USERS, // "Max Users"
	MB, // "MiB"
	MBPS, // "MiB/s"
	MENU_ABOUT, // "About DC++..."
	MENU_ADL_SEARCH, // "ADL Search"
	MENU_ARRANGE, // "Arrange icons"
	MENU_CASCADE, // "Cascade"
	MENU_CHANGELOG, // "Change Log"
	MENU_CLOSE_DISCONNECTED, // "Close disconnected"
	MENU_CONTENTS, // "Help &Contents\tF1"
	MENU_DISCUSS, // "DC++ discussion forum"
	MENU_DONATE, // "Donate ���/$$$ (paypal)"
	MENU_DOWNLOAD_QUEUE, // "&Download Queue\tCtrl+D"
	MENU_EXIT, // "&Exit"
	MENU_FAQ, // "Frequently asked questions"
	MENU_FAVORITE_HUBS, // "&Favorite Hubs\tCtrl+F"
	MENU_FAVORITE_USERS, // "Favorite &Users\tCtrl+U"
	MENU_FILE, // "&File"
	MENU_FOLLOW_REDIRECT, // "Follow last redirec&t\tCtrl+T"
	MENU_HASH_PROGRESS, // "Indexing progress"
	MENU_HELP, // "&Help"
	MENU_HELP_DOWNLOADS, // "Downloads"
	MENU_HELP_FORUM, // "Help forum"
	MENU_HELP_TRANSLATIONS, // "Translations"
	MENU_HOMEPAGE, // "DC++ Homepage"
	MENU_HORIZONTAL_TILE, // "Horizontal Tile"
	MENU_MINIMIZE_ALL, // "Minimize &All"
	MENU_NETWORK_STATISTICS, // "Network Statistics"
	MENU_NOTEPAD, // "&Notepad\tCtrl+N"
	MENU_OPEN_DOWNLOADS_DIR, // "Open downloads directory"
	MENU_OPEN_FILE_LIST, // "Open file list...\tCtrl+L"
	MENU_PUBLIC_HUBS, // "&Public Hubs\tCtrl+P"
	MENU_QUICK_CONNECT, // "&Quick Connect ...\tCtrl+Q"
	MENU_README, // "Readme / Newbie help"
	MENU_RECONNECT, // "&Reconnect\tCtrl+R"
	MENU_REFRESH_FILE_LIST, // "Refresh file list\tCtrl+E"
	MENU_REPORT_BUG, // "Report a bug"
	MENU_REQUEST_FEATURE, // "Request a feature"
	MENU_SEARCH, // "&Search\tCtrl+S"
	MENU_SEARCH_SPY, // "Search Spy"
	MENU_SETTINGS, // "Settings..."
	MENU_SHOW, // "Show"
	MENU_STATUS_BAR, // "&Status bar\tCtrl+2"
	MENU_TOOLBAR, // "&Toolbar\tCtrl+1"
	MENU_TRANSFER_VIEW, // "T&ransfers\tCtrl+3"
	MENU_VERTICAL_TILE, // "Vertical Tile"
	MENU_VIEW, // "&View"
	MENU_WINDOW, // "&Window"
	MIN_SHARE, // "Min Share"
	MIN_SLOTS, // "Min Slots"
	MOVE, // "Move/Rename"
	MOVE_DOWN, // "Move &Down"
	MOVE_UP, // "Move &Up"
	NETWORK_STATISTICS, // "Network Statistics"
	NETWORK_UNREACHABLE, // "Network unreachable (are you connected to the internet?)"
	NEW, // "&New..."
	NEXT, // "Next"
	NICK, // "Nick"
	NICK_TAKEN, // "Your nick was already taken, please change to something else!"
	NICK_UNKNOWN, // " (Nick unknown)"
	NO, // "No"
	NO_DIRECTORY_SPECIFIED, // "No directory specified"
	NO_DOWNLOADS_FROM_SELF, // "You're trying to download from yourself!"
	NO_ERRORS, // "No errors"
	NO_MATCHES, // "No matches"
	NO_SLOTS_AVAILABLE, // "No slots available"
	NO_USERS, // "No users"
	NO_USERS_TO_DOWNLOAD_FROM, // "No users to download from"
	NON_BLOCKING_OPERATION, // "Non-blocking operation still in progress"
	NORMAL, // "Normal"
	NOT_CONNECTED, // "Not connected"
	NOT_SOCKET, // "Not a socket"
	NOTEPAD, // "Notepad"
	OFFLINE, // "Offline"
	ONLINE, // "Online"
	ONLY_FREE_SLOTS, // "Only users with free slots"
	ONLY_WHERE_OP, // "Only where I'm op"
	OPEN, // "Open"
	OPEN_DOWNLOAD_PAGE, // "Open download page?"
	OPEN_FOLDER, // "Open folder"
	OPERATING_SYSTEM_NOT_COMPATIBLE, // "Operating system does not match minimum requirements for feature"
	OPERATION_WOULD_BLOCK_EXECUTION, // "Operation would block execution"
	OUT_OF_BUFFER_SPACE, // "Out of buffer space"
	PARTS, // "Parts: "
	PASSIVE_USER, // "Passive user"
	PASSWORD, // "Password"
	PATH, // "Path"
	PAUSED, // "Paused"
	PERMISSION_DENIED, // "Permission denied"
	PIB, // "PiB"
	PICTURE, // "Picture"
	PORT, // "Port: "
	PORT_IS_BUSY, // "Port %d is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++"
	PREPARING_FILE_LIST, // "Preparing file list..."
	PRESS_FOLLOW, // "Press the follow redirect button to connect to "
	PRIORITY, // "Priority"
	PRIVATE_MESSAGE_FROM, // "Private message from "
	PROPERTIES, // "&Properties"
	PUBLIC_HUBS, // "Public Hubs"
	QUICK_CONNECT, // "Quick Connect"
	RATING, // "Rating"
	RATIO, // "Ratio"
	READD_SOURCE, // "Re-add source"
	REALLY_EXIT, // "Really exit?"
	REDIRECT, // "Redirect"
	REDIRECT_ALREADY_CONNECTED, // "Redirect request received to a hub that's already connected"
	REDIRECT_USER, // "Redirect user(s)"
	REFRESH, // "&Refresh"
	REFRESH_USER_LIST, // "Refresh user list"
	RELIABILITY, // "Reliability"
	REMOVE, // "&Remove"
	REMOVE_ALL, // "Remove all"
	REMOVE_ALL_SUBDIRECTORIES, // "Remove all subdirectories before adding this one"
	REMOVE_FROM_ALL, // "Remove user from queue"
	REMOVE_SOURCE, // "Remove source"
	ROLLBACK_INCONSISTENCY, // "Rollback inconsistency, existing file does not match the one being downloaded"
	RUNNING, // "Running..."
	SEARCH, // "Search"
	SEARCH_BY_TTH, // "Search by TTH"
	SEARCH_FOR, // "Search for"
	SEARCH_FOR_ALTERNATES, // "Search for alternates"
	SEARCH_FOR_FILE, // "Search for file"
	SEARCH_OPTIONS, // "Search options"
	SEARCH_SPAM_FROM, // "Search spam detected from "
	SEARCH_SPY, // "Search Spy"
	SEARCH_STRING, // "Search String"
	SEARCHING_FOR, // "Searching for "
	SEEK_BEYOND_END, // "Request to seek beyond the end of data"
	SEND_PRIVATE_MESSAGE, // "Send private message"
	SEPARATOR, // "Separator"
	SERVER, // "Server"
	SET_PRIORITY, // "Set priority"
	SETTINGS, // "Settings"
	SETTINGS_ADD_FINISHED_INSTANTLY, // "Add finished files to share instantly (if shared)"
	SETTINGS_ADD_FOLDER, // "&Add folder"
	SETTINGS_ADLS_BREAK_ON_FIRST,  // "Break on first ADLSearch match"
	SETTINGS_ADVANCED, // "Advanced"
	SETTINGS_ADVANCED_SETTINGS, // "Advanced settings"
	SETTINGS_ANTI_FRAG, // "Use antifragmentation method for downloads"
	SETTINGS_APPEARANCE, // "Appearance"
	SETTINGS_AUTO_AWAY, // "Auto-away on minimize (and back on restore)"
	SETTINGS_AUTO_FOLLOW, // "Automatically follow redirects"
	SETTINGS_AUTO_KICK, // "Automatically disconnect users who leave the hub (does not disconnect when hub goes down / you leave it)"
	SETTINGS_AUTO_SEARCH, // "Automatically search for alternative download locations"
	SETTINGS_AUTO_SEARCH_AUTO_MATCH, // "Automatically match queue for auto search hits"
	SETTINGS_AUTO_UPDATE_LIST, // "Automatically refresh share list every hour"
	SETTINGS_CHANGE, // "&Change"
	SETTINGS_CLEAR_SEARCH, // "Clear search box after each search"
	SETTINGS_COLORS, // "Colors"
	SETTINGS_COMMAND, // "Command"
	SETTINGS_COMPRESS_TRANSFERS, // "Enable safe and compressed transfers"
	SETTINGS_CONFIRM_EXIT, // "Confirm application exit"
	SETTINGS_CONNECTION_SETTINGS, // "Connection Settings (see the readme / newbie help if unsure)"
	SETTINGS_CONNECTION_TYPE, // "Connection Type"
	SETTINGS_DEFAULT_AWAY_MSG, // "Default away message"
	SETTINGS_DIRECTORIES, // "Directories"
	SETTINGS_DONT_DL_ALREADY_SHARED, // "Don't download files already in share"
	SETTINGS_DOWNLOAD_DIRECTORY, // "Default download directory"
	SETTINGS_DOWNLOAD_LIMITS, // "Limits"
	SETTINGS_DOWNLOADS, // "Downloads"
	SETTINGS_DOWNLOADS_MAX, // "Maximum simultaneous downloads (0 = infinite)"
	SETTINGS_DOWNLOADS_SPEED_PAUSE, // "No new downloads if speed exceeds (KiB/s, 0 = disable)"
	SETTINGS_EXAMPLE_TEXT, // "Donate ���:s! (ok, dirty dollars are fine as well =) (see help menu)"
	SETTINGS_FAV_SHOW_JOINS, // "Only show joins / parts for favorite users"
	SETTINGS_FILTER_MESSAGES, // "Filter kick and NMDC debug messages"
	SETTINGS_FINISHED_DIRTY, // "Set Finished Manager(s) tab bold when an entry is added"
	SETTINGS_FORMAT, // "Format"
	SETTINGS_GENERAL, // "General"
	SETTINGS_GET_USER_COUNTRY, // "Get User Country"
	SETTINGS_HUB_USER_COMMANDS, // "Accept custom user commands from hub"
	SETTINGS_IGNORE_OFFLINE, // "Ignore private messages from offline users"
	SETTINGS_IP, // "IP"
	SETTINGS_KEEP_LISTS, // "Don't delete file lists when exiting"
	SETTINGS_LANGUAGE_FILE, // "Language file"
	SETTINGS_LIST_DUPES, // "Keep duplicate files in your file list (duplicates never count towards your share size)"
	SETTINGS_LOG_DOWNLOADS, // "Log downloads"
	SETTINGS_LOG_FILELIST_TRANSFERS, // "Log filelist transfers"
	SETTINGS_LOG_MAIN_CHAT, // "Log main chat"
	SETTINGS_LOG_PRIVATE_CHAT, // "Log private chat"
	SETTINGS_LOG_STATUS_MESSAGES, // "Log status messages"
	SETTINGS_LOG_SYSTEM_MESSAGES, // "Log system messages"
	SETTINGS_LOG_UPLOADS, // "Log uploads"
	SETTINGS_LOGGING, // "Logging"
	SETTINGS_LOGS, // "Logs and Sound"
	SETTINGS_MAGNET_ASK, // "Ask what to do when a magnet link is detected."
	SETTINGS_MAX_HASH_SPEED, // "Max hash speed"
	SETTINGS_MAX_TAB_ROWS, // "Max tab rows"
	SETTINGS_MINIMIZE_TRAY, // "Minimize to tray"
	SETTINGS_NAME, // "Name"
	SETTINGS_NO_AWAYMSG_TO_BOTS, // "Don't send the away message to bots"
	SETTINGS_ONLY_HASHED, // "Note; New files are added to the share only once they've been hashed!"
	SETTINGS_OPEN_FAVORITE_HUBS, // "Open the favorite hubs window at startup"
	SETTINGS_OPEN_FINISHED_DOWNLOADS, // "Open the finished downloads window at startup"
	SETTINGS_OPEN_PUBLIC, // "Open the public hubs window at startup"
	SETTINGS_OPEN_QUEUE, // "Open the download queue window at startup"
	SETTINGS_OPEN_USER_CMD_HELP, // "Always open help file with this dialog"
	SETTINGS_OPTIONS, // "Options"
	SETTINGS_PASSIVE, // "Passive"
	SETTINGS_PERSONAL_INFORMATION, // "Personal Information"
	SETTINGS_PM_BEEP, // "Make an annoying sound every time a private message is received"
	SETTINGS_PM_BEEP_OPEN, // "Make an annoying sound when a private message window is opened"
	SETTINGS_POPUNDER_FILELIST, // "Open new file list windows in the background"
	SETTINGS_POPUNDER_PM, // "Open new private message windows in the background"
	SETTINGS_POPUP_OFFLINE, // "Open private messages from offline users in their own window"
	SETTINGS_POPUP_PMS, // "Open private messages in their own window"
	SETTINGS_PORT, // "Port (empty=random)"
	SETTINGS_PUBLIC_HUB_LIST, // "Public Hubs list"
	SETTINGS_PUBLIC_HUB_LIST_HTTP_PROXY, // "HTTP Proxy (for hublist only)"
	SETTINGS_PUBLIC_HUB_LIST_URL, // "Public Hubs list URL"
	SETTINGS_QUEUE_DIRTY, // "Set Download Queue tab bold when contents change"
	SETTINGS_REQUIRES_RESTART, // "Note; most of these options require that you restart DC++"
	SETTINGS_ROLLBACK, // "Rollback"
	SETTINGS_SELECT_TEXT_FACE, // "Select &text style"
	SETTINGS_SELECT_WINDOW_COLOR, // "Select &window color"
	SETTINGS_SEND_UNKNOWN_COMMANDS, // "Send unknown /commands to the hub"
	SETTINGS_SFV_CHECK, // "Enable automatic SFV checking"
	SETTINGS_SHARE_HIDDEN, // "Share hidden files"
	SETTINGS_SHARE_SIZE, // "Total size:"
	SETTINGS_SHARED_DIRECTORIES, // "Shared directories"
	SETTINGS_SHOW_JOINS, // "Show joins / parts in chat by default"
	SETTINGS_SHOW_PROGRESS_BARS, // "Show progress bars for transfers (uses some CPU)"
	SETTINGS_SKIP_ZERO_BYTE, // "Skip zero-byte files"
	SETTINGS_SMALL_SEND_BUFFER, // "Use small send buffer (enable if uploads slow downloads a lot)"
	SETTINGS_SOCKS5, // "SOCKS5"
	SETTINGS_SOCKS5_IP, // "Socks IP"
	SETTINGS_SOCKS5_PORT, // "Port"
	SETTINGS_SOCKS5_RESOLVE, // "Use SOCKS5 server to resolve hostnames"
	SETTINGS_SOCKS5_USERNAME, // "Username"
	SETTINGS_SOUNDS, // "Sounds"
	SETTINGS_SPEEDS_NOT_ACCURATE, // "Note; because of changing download speeds, this is not 100% accurate..."
	SETTINGS_STATUS_IN_CHAT, // "View status messages in main chat"
	SETTINGS_TAB_COMPLETION, // "Tab completion of nicks in chat"
	SETTINGS_TAB_DIRTY, // "Set hub/PM tab bold when contents change"
	SETTINGS_TIME_STAMPS, // "Show timestamps in chat by default"
	SETTINGS_TIME_STAMPS_FORMAT, // "Set timestamps"
	SETTINGS_UNFINISHED_DOWNLOAD_DIRECTORY, // "Unfinished downloads directory"
	SETTINGS_UPLOADS, // "Sharing"
	SETTINGS_UPLOADS_MIN_SPEED, // "Automatically open an extra slot if speed is below (0 = disable)"
	SETTINGS_UPLOADS_SLOTS, // "Upload slots"
	SETTINGS_URL_HANDLER, // "Register with Windows to handle dchub:// URL links"
	SETTINGS_URL_MAGNET, // "Register with Windows to handle magnet: URI links"
	SETTINGS_USE_OEM_MONOFONT, // "Use OEM monospaced font for viewing text files"
	SETTINGS_USE_SYSTEM_ICONS, // "Use system icons when browsing files (slows browsing down a bit)"
	SETTINGS_USE_UPNP, // "Use UPnP Control"
	SETTINGS_USER_COMMANDS, // "Advanced\\User Commands"
	SETTINGS_WRITE_BUFFER, // "Write buffer size"
	SFV_INCONSISTENCY, // "CRC32 inconsistency (SFV-Check)"
	SHARED, // "Shared"
	SHARED_FILES, // "Shared Files"
	SIZE, // "Size"
	SIZE_MAX, // "Max Size"
	SIZE_MIN, // "Min Size"
	SLOT_GRANTED, // "Slot granted"
	SLOTS, // "Slots"
	SLOTS_SET, // "Slots set"
	SOCKET_SHUT_DOWN, // "Socket has been shut down"
	SOCKS_AUTH_FAILED, // "Socks server authentication failed (bad username / password?)"
	SOCKS_AUTH_UNSUPPORTED, // "The socks server doesn't support user / password authentication"
	SOCKS_FAILED, // "The socks server failed establish a connection"
	SOCKS_NEEDS_AUTH, // "The socks server requires authentication"
	SOCKS_SETUP_ERROR, // "Failed to set up the socks server for UDP relay (check socks address and port)"
	SOURCE_TYPE, // "Source Type"
	SPECIFY_SEARCH_STRING, // "Specify a search string"
	SPECIFY_SERVER, // "Specify a server to connect to"
	SPEED, // "Speed"
	STATUS, // "Status"
	STORED_PASSWORD_SENT, // "Stored password sent..."
	TAG, // "Tag"
	TARGET_FILENAME_TOO_LONG, // "Target filename too long"
	TB, // "TiB"
	TIME, // "Time"
	TIME_LEFT, // "Time left"
	TIMESTAMPS_DISABLED, // "Timestamps disabled"
	TIMESTAMPS_ENABLED, // "Timestamps enabled"
	TOO_MUCH_DATA, // "More data was sent than was expected"
	TOTAL, // "Total: "
	TTH_ALREADY_SHARED, // "A file with the same hash already exists in your share"
	TTH_INCONSISTENCY, // "TTH inconsistency"
	TTH_ROOT, // "TTH Root"
	TYPE, // "Type"
	UNABLE_TO_CREATE_THREAD, // "Unable to create thread"
	UNKNOWN, // "Unknown"
	UNKNOWN_ADDRESS, // "Unknown address"
	UNKNOWN_COMMAND, // "Unknown command: "
	UNKNOWN_ERROR, // "Unknown error: 0x%x"
	UNSUPPORTED_FILELIST_FORMAT, // "Unsupported filelist format"
	UPLOAD_FINISHED_IDLE, // "Upload finished, idle..."
	UPLOAD_STARTING, // "Upload starting..."
	UPLOADED_BYTES, // "Uploaded %s (%.01f%%) in %s"
	UPLOADED_TO, // " uploaded to "
	UPLOADS, // "Uploads"
	UPNP_FAILED_TO_CREATE_MAPPINGS, // "Failed to create port mappings"
	UPNP_FAILED_TO_REMOVE_MAPPINGS, // "Failed to remove port mappings"
	USER, // "User"
	USER_CMD_CHAT, // "Chat"
	USER_CMD_COMMAND, // "Command"
	USER_CMD_CONTEXT, // "Context"
	USER_CMD_HUB, // "Hub IP / DNS (empty = all, 'op' = where operator)"
	USER_CMD_HUB_MENU, // "Hub Menu"
	USER_CMD_ONCE, // "Send once per nick"
	USER_CMD_PARAMETERS, // "Parameters"
	USER_CMD_PM, // "PM"
	USER_CMD_PREVIEW, // "Text sent to hub"
	USER_CMD_RAW, // "Raw"
	USER_CMD_SEARCH_MENU, // "Search Menu"
	USER_CMD_TO, // "To"
	USER_CMD_TYPE, // "Command Type"
	USER_CMD_USER_MENU, // "User Menu"
	USER_CMD_WINDOW, // "Create / Modify Command"
	USER_DESCRIPTION, // "User Description"
	USER_OFFLINE, // "User offline"
	USER_WENT_OFFLINE, // "User went offline"
	USERS, // "Users"
	VIDEO, // "Video"
	VIEW_AS_TEXT, // "View as text"
	VIRTUAL_NAME, // "Virtual name"
	VIRTUAL_NAME_EXISTS, // "Virtual directory name already exists"
	VIRTUAL_NAME_LONG, // "Name under which the others see the directory"
	WAITING, // "Waiting..."
	WAITING_TO_RETRY, // "Waiting to retry..."
	WAITING_USER_ONLINE, // "Waiting (User online)"
	WAITING_USERS_ONLINE, // "Waiting (%d of %d users online)"
	WHATS_THIS, // "What's &this?"
	YES, // "Yes"
	YOU_ARE_BEING_REDIRECTED, // "You are being redirected to "
	LAST // @DontAdd
};
