SRCS=Album.cpp \
     Artist.cpp \
     GUIViewStateMusic.cpp \
     LastFmManager.cpp \
     MusicDatabase.cpp \
     MusicInfoLoader.cpp \
     Song.cpp \
     spotyXBMC/Addon.music.spotify.cpp \
     spotyXBMC/FileSystemHelper.cpp \
     spotyXBMC/Logger.cpp \
     spotyXBMC/Settings.cpp \
     spotyXBMC/XBMCUpdater.cpp \
     spotyXBMC/radio/SxRadio.cpp \
     spotyXBMC/radio/RadioHandler.cpp \
     spotyXBMC/album/SxAlbum.cpp \
     spotyXBMC/album/AlbumStore.cpp \
     spotyXBMC/artist/SxArtist.cpp \
     spotyXBMC/artist/ArtistStore.cpp \
     spotyXBMC/search/Search.cpp \
     spotyXBMC/search/SearchHandler.cpp \
     spotyXBMC/player/Codec.cpp \
     spotyXBMC/player/PlayerHandler.cpp \
     spotyXBMC/playlist/TopLists.cpp \
     spotyXBMC/playlist/SxPlaylist.cpp \
     spotyXBMC/playlist/StarredList.cpp \
     spotyXBMC/playlist/PlaylistStore.cpp \
     spotyXBMC/session/Session.cpp \
     spotyXBMC/session/SessionCallbacks.cpp \
     spotyXBMC/thumb/SxThumb.cpp \
     spotyXBMC/thumb/ThumbStore.cpp \
     spotyXBMC/track/SxTrack.cpp \
     spotyXBMC/track/TrackStore.cpp 
     
     
LIB=music.a

include ../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
