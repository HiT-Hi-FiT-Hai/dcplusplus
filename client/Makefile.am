lib_LTLIBRARIES = libdcppclient.la

LIBDCPP_CURRENT=0
LIBDCPP_REVISION=0
LIBDCPP_AGE=0

libdcppclient_la_LDFLAGS = -export-dynamic -version-info $(LIBDCPP_CURRENT):$(LIBDCPP_REVISION):$(LIBDCPP_AGE)

libdcppclient_la_CXXFLAGS = -Wall -pedantic -ansi -Wno-long-long

libdcppclient_la_SOURCES= \
AdcCommand.h \
AdcCommand.cpp \
AdcHub.cpp \
AdcHub.h \
ADLSearch.cpp \
ADLSearch.h \
BitInputStream.h \
BitOutputStream.h \
BloomFilter.h \
BufferedSocket.cpp \
BufferedSocket.h \
BZUtils.cpp \
BZUtils.h \
CID.h \
Client.cpp \
Client.h \
ClientManager.cpp \
ClientManager.h \
ClientManagerListener.h \
config.h \
ConnectionManager.cpp \
ConnectionManager.h \
ConnectionManagerListener.h \
CriticalSection.h \
CryptoManager.cpp \
CryptoManager.h \
DCPlusPlus.cpp \
DCPlusPlus.h \
DirectoryListing.cpp \
DirectoryListing.h \
DownloadManager.cpp \
DownloadManager.h \
Encoder.cpp \
Encoder.h \
Exception.cpp \
Exception.h \
FastAlloc.h \
FavoriteUser.h \
File.h \
FilteredFile.h \
FinishedManager.cpp \
FinishedManager.h \
HashManager.cpp \
HashManager.h \
HashValue.h \
HttpConnection.cpp \
HttpConnection.h \
HubManager.cpp \
HubManager.h \
LogManager.cpp \
LogManager.h \
MerkleTree.h \
NmdcHub.cpp \
NmdcHub.h \
Pointer.h \
QueueItem.h \
QueueManager.cpp \
QueueManager.h \
QueueManagerListener.h \
ResourceManager.cpp \
ResourceManager.h \
SearchManager.cpp \
SearchManager.h \
SearchManagerListener.h \
Semaphore.h \
ServerSocket.cpp \
ServerSocket.h \
SettingsManager.cpp \
SettingsManager.h \
SFVReader.cpp \
SFVReader.h \
ShareManager.cpp \
ShareManager.h \
SimpleXML.cpp \
SimpleXML.h \
Singleton.h \
Socket.cpp \
Socket.h \
Speaker.h \
stdinc.cpp \
stdinc.h \
StringDefs.cpp \
StringDefs.h \
StringSearch.h \
StringTokenizer.cpp \
StringTokenizer.h \
Text.cpp \
Text.h \
Thread.cpp \
Thread.h \
TigerHash.cpp \
TigerHash.h \
TimerManager.cpp \
TimerManager.h \
UploadManager.cpp \
UploadManager.h \
UserCommand.h \
UserConnection.cpp \
UserConnection.h \
User.cpp \
User.h \
Util.cpp \
Util.h \
version.h \
ZUtils.cpp \
ZUtils.h 

pch: stdinc.h.gch

stdinc.h.gch: stdinc.h
	$(CXXCOMPILE) -fPIC -DPIC -c $< 

.PHONY: pch
	
