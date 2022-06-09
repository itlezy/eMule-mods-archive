eMule Copyright (C)2002 Merkur (merkur-@users.sourceforge.net)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

-----------------------------------------------------------------------------------


Welcome to eMule Plus, a filesharing client based on the eDonkey2000(C) network.

Visit us at http://emuleplus.info

Visit our forum for bug reports, feature requests or support.

If you have questions or serious problems, please read the FAQ first at
http://emuleplus.info/faq/

If you didn't find an answer, SEARCH the forum for a topic related to your problem,
DO NOT open a new topic at once, most likely someone else had the same problem before.

The official Forum is also at http://emuleplus.info/forum/
Please use only English there and PLEASE do not report bugs that are already posted
by someone else.


INSTALLATION:
-------------

- Unzip eMule Plus to a directory of your choice, or start the installer if you 
  downloaded the .exe installer version.

- You can move your "Temp" and "Incoming" folders from eDonkey (or a previous version
  of eMule) to the new directory in order to continue your partial downloads.
  If you don't want to move them, you can set the "Temp" and "Incoming" paths
  in preferences and restart it to get the same result.

- Updating from an earlier version of eMule Plus:
  The best way to do this is simply to download .zip file (eMulePlus-xx.Binary.zip),
  unzip it and copy all unpacked files to the directory eMule Plus was installed before.
  Note 1: do not copy config\ipfilter.dat if you use your own ipfilter.dat.
  Note 2: do not copy config\staticservers.dat if you use your own static servers list.


START:
------

- You must download a server list to get started. Just go to the "Servers" tab and
  press "Update". Note that this sites are not related to the eMule Plus team, and
  we are not responsible for their content.

- Don't forget to connect to a server. You can also activate the "Autoconnect on Startup"
  option (Preferences -> Connection -> Server) to do it automatically after start.


CONFIGURATION:
--------------

- Click the "Preferences" button

- Enter a nice nickname

- Enter the "Download Capacity" and "Upload Capacity" (Preferences -> Connection
  -> Server) according to your Internet connection or use the profiles provided
  by eMule Plus. All values in eMule Plus are in kiloBytes (KB), but your
  Internet Service Provider's (ISP) numbers are most likely kiloBits (kB).
  8 Bits make up 1 Byte, so when your Internet Connection is 768kB Downstream and
  128kB Upstream, your correct values are:

    Downstream: 768kB / 8 = 96KB, you enter 96 as "Download Capacity"
    Upstream: 128kB / 8 = 16 KB, you enter 16 as "Upload Capacity"

  The "capacity" values are used for the statistics display only.
  Nevertheless, you need to know them to determine the following down/upload limits...

- Enter "Download Limit" and "Upload Limit" (IMPORTANT!)
 
  Download Limit: check "Limitless download" to remove download limit (should
  eMule Plus becomes too fast and you are unable to surf the Internet or whatever,
  uncheck it and reduce "Download Limit" to 80-90% of "Download Capacity").
 
  Upload Limit: set this to ~80% of your "Upload Capacity" (so when your Upload Capacity
  is 16, set Upload Limit to 12 or 13).

  Setting Upload Limit to a value < 10 will automatically reduce your Download Limit, so
  upload as fast as you can.

- Maximum Connections: depends on your operating system. As a general rule...

  - Windows 98/ME (and 56k Modem/ISDN) users enter 50 or less here
  - Windows 2000/XP users should set this according to their Internet Connection.
    250 is a good value for 128k upstream connection, for example.
    DO NOT set this too high, it will kill your upload and with that, your download.

- In 5 sec: depends on your operating system. As a general rule...

  - Windows 98/ME (and 56k Modem/ISDN) users enter 10 or less here
  - Windows 2000/XP users should set this to 20 or more depending on your connection
  - Windows XP SP2 users might want to use 10 or less because of the new protections
    bundled in this update (half open connections limit)

- Choose the directories you want to share with other users.
  DO NOT SHARE YOUR COMPLETE HARDDISK!
  Put the stuff you want to share in a separate folder.
  If you share more than 1000 files, you should reconsider that...

- The other options are pretty self-explaining. If you don't understand what it does,
  read the FAQ or ask at our forum.


FAQ: (for more, see http://emuleplus.info/faq/)
----

- Will I lose my credits when switching to a new version of eMule Plus?

  Not when you move your old preferences.dat (your user ID), cryptkey.dat (your secure ID)
  and clients.met (other people's credits) files to the directory you installed the new version in.
  The best way to update is just to replace your old files with the new eMule.exe, .tmpl files
  and webserver directory from the .zip download, as well as the new configuration files.

- Why is eMule Plus so slow? My brother/friend/whatever is downloading at 100K constantly!

  When you did setup eMule Plus properly, it's all about the availability of the files you are
  downloading, a bit of luck and a lot of patience. ;)

- What is the addresses.dat file for?

  You can enter a server list URL in that file. eMule Plus will then get the server list from that
  URL at startup (when the option "Auto-update serverlist at startup" is activated).

- What is the staticservers.dat file good for?

  You can enter your favorite servers here to have them permanently in your server list.
  You can enter the static IP of the server, or an address like 'goodserver.dyndns.net'. 
  You can also add static servers to this file via the server tab in eMule Plus
  (right-click -> Add to static serverlist).

- Why do I always have a low ID (means: firewalled)? What can I do against that?

  An ID is calculated by your IP address and is assigned by the server when you connect to it.
  The actual value of the ID (can be found in the log window) is not related to your credit score,
  but having a high value is important. If you can get a highID your client can freely access
  the servers and vice versa and if not (a lowID in that case) something is preventing your
  client from doing so. LowID users can still download, but not from those who also have lowIDs.

- How do I know whether my ID is high or low?

  Look at the world in the bottom right corner, next to the server name you are connected to.
  When it's green, your ID is high. When it's yellow, your ID is low.
 
NOTE: you can also get a low ID when the server you connected to is too busy to answer
      properly, or simply badly configured. When you are sure your settings are ok and
      you SHOULD have a high ID, connect to another server.

- What is the difference between up/down CAPACITY and LIMIT?

  The CAPACITY is used only by the statistics tab to determine the vertical limits of the diagram.
  The LIMITS set the actual network traffic limits (see configuration notes).


COMPILING THE SOURCECODE:
-------------------------

The sourcecode of eMule Plus is available as separate download.
You need Microsoft(C) Visual C++ .NET (7.0 or 7.1 SP1) to compile eMule Plus.

- Unzip the sources (with subdirs) into a new folder
- Download the additional libs file or follow the instructions to
  get the other libs needed by eMule Plus (i.e. Crypto++)
- Open the eMule.vcproj Visual Studio Project
- Select "Release" or "Debug" build (2002 to compile on VS 7.0)
  in the solution configuration manager
- Build the solution. That's it!
- If the compile was successful, the eMule.exe is either in the
  \Debug or \Release directory ("Debug 2002 or Release 2002" for VS 7.0)
 
- IMPORTANT:
  If you make modifications to eMule and you distribute the compiled version of your
  build, be sure to obey the GPL license. You have to include the sourcecode, or make
  it available for download together with the binary version.


LEGAL:
------

eMule Copyright (C)2002 Merkur (merkur-@users.sourceforge.net)

eDonkey2000 (C)Jed McCaleb, MetaMachine (www.eDonkey2000.com)

Windows(TM), Windows 95(TM), Windows 98(TM), Windows ME(TM),
Windows NT(TM), Windows 2000(TM) and Windows XP(TM)
are Copyright (C)Microsoft Corporation. All rights reserved.
