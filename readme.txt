DC++ 0.181 Readme

--------------------------------------------------
DC++
Copyright (C) 2001-2002 Jacek Sieka, jacek@creatio.se

License

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
--------------------------------------------------

This Readme will give you a quick startup guide and explain some features
that are NOT obvious :)
So this is especially for NMDC upgraders.
Newbies should (also) check out the web site, http://dcplusplus.sourceforge.net

1 Installation
--------------
  Plain simple:
  Just extract everything to a folder of your choice and run DCPluPlus.exe :)

2 Upgrade
---------
  If u used NMDC: You can import your queue file now !
  If u used older DC++ version:
   As simple as Installing:
   Backup your old DCPlusPlus folder, extract the new version to it
   (overwriting old files) and run it.
   Your old settings will still be there as is your queue.
  BEWARE: Old (ver <= 0.163) Notepad notes will get lost, format change in 0.17

3 Uninstall
----------
  Even more simple:
  Just delete the DCPlusPlus folder.
  There are no registry entries, installed dll's or likewise :)

4 Available commands (type in main chat or pm)
----------------------------------------
  /grant				grants a slot to the user of the pm window you type in
  /close				close current window
  /help				short help message
  /refresh               Refreshes list of shared files :)
  /away <message>        Specifies a message to auto-respond in PM's while
                          you're AFK (there's a default message so you don't
                          need to specify one)
  /back                  Turn away message off
  /slots <#>             Changes number of slots to <#>
  /clear                 Clears the main chat windows
  /ts                    Switches timestamps in chat windows on and off
  /showjoins			Toggles joins/parts messages for the current hub
  /search <string>       Searches for <string>
  /join <hub>            Joins <hub>
  /dc++                  Gives a comment about DC++ and shows the URL where
                          you can get it

5 What do those Icons mean ??
-----------------------------
 green       = normal Icon
 blue        = DC++ user (identified on 1st direct connection to this user)
 with bricks = User is in passive mode
 with key    = User is an Operator

6 Not-so-obvious options
------------------------
  Settings:
   General:
    Active:              The usual connection mode, you can specify your IP
                          if detection fails and a port of your choice if
                          you need to.
    Passive:             Compatibility connection mode for users behind
                          Firewalls they can't change to let DC++
                          connections trough.
                         Only use this if Active is not working.
                         Note: Passive <-> Passive connections aren't possible.
   Downloads:
    Unfinished files temporary directory:	You can specify a Directory for your
    					partial files here, suggestion si to have it on another
    					partition than the "Default download directory" to have
    					the files defragmented through moving there :)
    Download slots:		Maximum number of Downloads runing at once, suggestion
    					is to set it to the same number as your upload slots
   Appearance:
    Use system icons:	Displays the icons set in Windows for filetypes
    Language File:       A XML-file containing most of the text used in DC++.
                         You can specify a file to have DC++ in your
                          favourite language.
                         There's an example.xml if you want to make your own.
                         Language files available at
                          http://DCPlusPlus.sourceforge.net
   Logs and Sound:
    Log Downloads:       You can use this feature to see what downloads
                          completed while you were AFK :)
   Advanced:
    Rollback:            Size of bytes to rollback when resuming a file to
                          ensure it contains no errors.
                         If there is an error DC++ deletes <rollback>
                          bytes and checks again.
    Write buffer size:   Anti-fragmentation feature, DC++ saves every
                          <write buffer size> bytes to keep fragmentation low.
    Client version:      Since most hubs specify a min client version that
                          is much higher than DC++'s real version number you
                          can set something here, i suggest adding 1 to version
                          number e.g. 1.181
                         to be able to enter the hub.
    Automatically search for alternate download locations:  Allows DC++ to try
                          to find other locations to download your files.
                          Useful if you're AFK a lot.
    Install URL handler on startup:	Set if you want URLS of type dchub://
    					to open in DC++
    Use small send buffer:	If uploads slow down your downloads A LOT you may
    					try this option, but beware it increases HD usage and
    					slows down upload, so it's not suggested.

7 Other things you might want to know about
--------------------------------------------
  * you can doubleclick on a user name in chat to select him in the user list
  * you can doubleclick on stuff starting with www. http:// of ftp:// to open :)
  * DC++ supports uploading filelists and files <16 kB to other DC++ users
    WITHOUT REQUIRING A SLOT. There's a max of 3 connections in addition to
    normal slots.
  * Files <16 kB and filelists are downloaded first.
  * There is information added to the description field:
    <++ V:x,M:x,H:x,S:x> where
     V = client version,
     M = mode (a=active, p=passive),
     H = number of hubs connected to where you're not a registered user,
     S = number of slots you have open.
      This is updated every 10-15 minutes.
  * There is a limit so that only 15 users and 1 op can be kicked at a time
    from the hub user list
  * Passive user detection, those that are behind a set of bricks are passive.
    (detected when the user searches or tries to connect to you)
  * DC++ user detection, those appear blue.
  * Dupe file removal, files with same name and size are automatically removed
    from your share
  * Search flood detection (If more than 5 searches are received from the same
    user within 7 seconds, DC++ will send out a warning)

8 Further information
------------------------
  * Homepage                  @ http://dcplusplus.sourceforge.net
  * Report bugs               @ http://sourceforge.net/tracker/?atid=427632&group_id=40287&func=browse
  * Request features          @ http://sourceforge.net/tracker/?atid=427635&group_id=40287&func=browse
  * Download language files   @ http://dcplusplus.sourceforge.net/index.php?page=download
  * Get newest version        @ http://dcplusplus.sourceforge.net/index.php?page=download
  * Typos and errors in readme to Fireball@enLightning.de

9 Have fun
----------
  Go out, share some files, be a happy DC++ user and have fun.