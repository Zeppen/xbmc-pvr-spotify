SPOTYXBMC2
========== 
This is a rewrite of the s potyxbmc project, the code is cleaner and better encapsulated from XBMC. It is now being prepared to be lifted out to a binary addon. 
The code is not heavely tested and has known issues, dont install if you dont know what you are doing.

The main discussion for spotyxbmc is [here](http://forum.xbmc.org/showthread.php?t=67012)
A discusion concerning a unified music addon frontend can be read [here](http://forum.xbmc.org/showthread.php?t=105147).

Features and usage
------------------
A video showing most of the features can be seen [here](http://www.youtube.com/watch?v=xFSdxKyWXpU).

* Starred tracks, albums and artists will show up in the songs, albums and artists sections alongside the local music.
* Spotify playlists shows up in the playlist section.
* The normal music search will return local music and spotify search result.
* Top 100 spotify lists with artists, albums and tracks is available in the top 100 section. 
* Modifying playlists and star/unstar items in the spotify client will result in that the lists is updated in XBMC.
* Multidisc albums are split up into seperate albums with suffix "disc #".
* Browsing artist albums will provide a "similar artists" folder with spotify similar artists.
* The settings is changed from within a builtin addon.
* Two spotify "radios" is provided int the music root, visit the settings to set name, years and genres.
* Toplists will update once every 24 hour.

Missing features
----------------
* Interaction with spotify items like creating and modify playlists, adding spotify tracks to playlists and so on is not supported yet.
* The year and genre nodes will not lead to any spotify items yet. 
* A lot more I guess.

Platforms
---------
Linux - supported
Windows - Not yet, will be supported soon
OSX - Not supported, if you want to help please contact me

Known issues
------------
Exiting XBMC will result in a crash, will be fixed soon I hope.

Enable preloading of artists together with preloading of top 100 lists and/or a massiv collection of starred tracks will result in a short freeze of XBMC during start (about 5-10 seconds depending on your internet speed, computer...).

Memmory leaks do exist, beware.

A lot of other bugs, the implementation is not heavely tested.
 
Want to help killing a bug?
---------------------------
Right now the there is a lot of trace prints, they are printed out straight to the console so be sure that you start XBMC from a console in order to fetch the traces and create a bug report.

Please submit a report to the github issues and provide all relevant data like logs, OS info, what track, playlist or album you have problems with. Or even better, fix it yourself and send me a pull request or an e-mail. 


Installation instructions Ubuntu Linux 32/64
--------------------------------------------
. Download libSpotify

For 32 bit:
`$ wget http://developer.spotify.com/download/libspotify/libspotify-0.0.8-linux6-i686.tar.gz`

64 bit OS:
`$ wget http://developer.spotify.com/download/libspotify/libspotify-0.0.8-linux6-x86_64.tar.gz`

2. Untar:
`$ tar xzf libspotify-*.tar.gz`

3. Install libspotify
`$ cd libspotify-0.0.8...`
`$ sudo make install`

4. Obtain spotyXBMC source 
   Make sure you have git installed, if not and in ubuntu install with `sudo apt-get install git-core`
`$ cd ..`
`$ git clone git://github.com/akezeke/xbmc.git`
`$ cd xbmc`

5. Spotify API key
   Get your own spotify API key from http://developer.spotify.com/en/libspotify/application-key/
   Click on c-code and copy the content to a new file called appkey.h placed in the xbmc source root folder. (where this readme is located).

6. Install all XBMC dependencies listed in the corresponding readme file.
   For ubuntu 11.04 run:

`$ sudo apt-get install git-core make g++ gcc gawk pmount libtool nasm yasm automake cmake gperf zip unzip bison libsdl-dev libsdl-image1.2-dev libsdl-gfx1.2-dev libsdl-mixer1.2-dev libfribidi-dev liblzo2-dev libfreetype6-dev libsqlite3-dev libogg-dev libasound-dev python-sqlite libglew-dev libcurl3 libcurl4-gnutls-dev libxrandr-dev libxrender-dev libmad0-dev libogg-dev libvorbisenc2 libsmbclient-dev libmysqlclient-dev libpcre3-dev libdbus-1-dev libhal-dev libhal-storage-dev libjasper-dev libfontconfig-dev libbz2-dev libboost-dev libenca-dev libxt-dev libxmu-dev libpng-dev libjpeg-dev libpulse-dev mesa-utils libcdio-dev libsamplerate-dev libmpeg3-dev libflac-dev libiso9660-dev libass-dev libssl-dev fp-compiler gdc libmpeg2-4-dev libmicrohttpd-dev libmodplug-dev libssh-dev gettext cvs python-dev libyajl-dev libboost-thread-dev autopoint`


7. Configure, make and install xbmc
`$ ./bootstrap`
`$ ./configure`
`$ make`
`$ sudo make install`

8. Start xbmc
`$ xbmc`

9. Start spotyXBMC
start the preinstalled music addon spotyXBMC and set the settings

10. Restart XBMC

11. Enable the music library and enjoy spotify music inside xbmc

Done!

Source
------
The spotify related code lives all in xbmc/music/spotyXBMC/ and can easely be extracted and used in other applications.

Added files:
* xbmc/music/spotyXBMC/Addon.music.spotify.cpp
* xbmc/music/spotyXBMC/Addon.music.spotify.h
* xbmc/music/spotyXBMC/FileSystemHelper.cpp
* xbmc/music/spotyXBMC/FileSystemHelper.h
* xbmc/music/spotyXBMC/Logger.cpp
* xbmc/music/spotyXBMC/Logger.h
* xbmc/music/spotyXBMC/Settings.cpp 
* xbmc/music/spotyXBMC/Settings.h
* xbmc/music/spotyXBMC/XBMCUpdater.cpp
* xbmc/music/spotyXBMC/XBMCUpdater.h
* xbmc/music/spotyXBMC/radio/SxRadio.cpp
* xbmc/music/spotyXBMC/radio/SxRadio.h
* xbmc/music/spotyXBMC/radio/RadioHandler.cpp
* xbmc/music/spotyXBMC/radio/RadioHandler.h
* xbmc/music/spotyXBMC/album/SxAlbum.cpp
* xbmc/music/spotyXBMC/album/SxAlbum.h
* xbmc/music/spotyXBMC/album/AlbumStore.cpp
* xbmc/music/spotyXBMC/album/AlbumStore.h
* xbmc/music/spotyXBMC/artist/SxArtist.cpp
* xbmc/music/spotyXBMC/artist/SxArtist.h
* xbmc/music/spotyXBMC/artist/ArtistStore.cpp
* xbmc/music/spotyXBMC/artist/ArtistStore.h
* xbmc/music/spotyXBMC/search/Search.cpp
* xbmc/music/spotyXBMC/search/Search.h
* xbmc/music/spotyXBMC/search/SearchHandler.cpp
* xbmc/music/spotyXBMC/search/SearchHandler.h
* xbmc/music/spotyXBMC/player/Codec.cpp
* xbmc/music/spotyXBMC/player/Codec.h
* xbmc/music/spotyXBMC/player/PlayerHandler.cpp
* xbmc/music/spotyXBMC/player/PlayerHandler.h
* xbmc/music/spotyXBMC/playlist/TopLists.cpp
* xbmc/music/spotyXBMC/playlist/TopLists.h
* xbmc/music/spotyXBMC/playlist/SxPlaylist.cpp
* xbmc/music/spotyXBMC/playlist/SxPlaylist.h
* xbmc/music/spotyXBMC/playlist/StarredList.cpp
* xbmc/music/spotyXBMC/playlist/StarredList.h
* xbmc/music/spotyXBMC/playlist/PlaylistStore.cpp
* xbmc/music/spotyXBMC/playlist/PlaylistStore.h
* xbmc/music/spotyXBMC/session/Session.cpp
* xbmc/music/spotyXBMC/session/Session.h
* xbmc/music/spotyXBMC/session/SessionCallbacks.cpp
* xbmc/music/spotyXBMC/session/SessionCallbacks.h
* xbmc/music/spotyXBMC/thumb/SxThumb.cpp
* xbmc/music/spotyXBMC/thumb/SxThumb.h
* xbmc/music/spotyXBMC/thumb/ThumbStore.cpp
* xbmc/music/spotyXBMC/thumb/ThumbStore.h
* xbmc/music/spotyXBMC/track/SxTrack.cpp
* xbmc/music/spotyXBMC/track/SxTrack.h
* xbmc/music/spotyXBMC/track/TrackStore.cpp 
* xbmc/music/spotyXBMC/track/TrackStore.h

* addons/plugin.music.spotyXBMC/icon.png
* addons/plugin.music.spotyXBMC/fanart.jpg
* addons/plugin.music.spotyXBMC/default.py
* addons/plugin.music.spotyXBMC/changelog.txt
* addons/plugin.music.spotyXBMC/addon.xml
* addons/plugin.music.spotyXBMC/resources/settings.xml
* addons/plugin.music.spotyXBMC/resources/language/English/strings.xml

Modified files:
* xbmc/cores/paplayer/CodecFactory.cpp
* xbmc/filesystem/MusicSearchDirectory.cpp
* xbmc/MusicDatabaseDirectory/DirectoryNodeAlbum.cpp
* xbmc/MusicDatabaseDirectory/DirectoryNodeArtist.cpp
* xbmc/MusicDatabaseDirectory/DirectoryNodeOverview.cpp
* xbmc/MusicDatabaseDirectory/DirectoryNodeSong.cpp
* xbmc/MusicDatabaseDirectory/DirectoryNodeTop100.cpp
* xbmc/music/windows/GUIWindowMusicBase.cpp
* xbmc/music/Windows/GuiWindowMusicNav.cpp
* xbmc/music/Makefile
* xbmc/settings/Settings.cpp
* xbmc/Application.cpp
* xbmc/GUIInfoManager.cpp
* xbmc/XBApplicationEx.cpp

Contact
-------

http://github.com/akezeke/spotyxbmc 
david.erenger@gmail.com

/David
