/*
 spotyxbmc2 - A project to integrate Spotify into XBMC
 Copyright (C) 2011  David Erenger

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For contact with the author:
 david.erenger@gmail.com
 */

#include "Addon.music.spotify.h"
#include <stdint.h>
#include "Logger.h"
#include "session/Session.h"
#include "playlist/StarredList.h"
#include "playlist/TopLists.h"
#include "playlist/PlaylistStore.h"
#include "playlist/SxPlaylist.h"
#include "artist/SxArtist.h"
#include "artist/ArtistStore.h"
#include "track/TrackStore.h"
#include "track/SxTrack.h"
#include "album/SxAlbum.h"
#include "album/AlbumStore.h"
#include "../tags/MusicInfoTag.h"
#include "../Album.h"
#include "../Artist.h"
#include "../../MediaSource.h"
#include "player/PlayerHandler.h"
#include "search/SearchHandler.h"
#include "radio/RadioHandler.h"
#include "Settings.h"

using namespace addon_music_spotify;
using namespace std;

Addon_music_spotify* g_spotify;

Addon_music_spotify::Addon_music_spotify() {
  if (Settings::enabled()) Session::getInstance()->enable();
}

Addon_music_spotify::~Addon_music_spotify() {
  Session::getInstance()->deInit();
  Logger::printOut("removing spotify addon");

}

bool Addon_music_spotify::enable(bool enable) {
  if (enable)
    return Session::getInstance()->enable();
  else
    return Session::getInstance()->disable();
}

bool Addon_music_spotify::isReady() {
  return Session::getInstance()->isReady();
}

bool Addon_music_spotify::processAddonThread() {
  //return Session::getInstance()->processEvents();
  return true;
}

bool Addon_music_spotify::GetPlaylists(CFileItemList& items) {
  if (isReady()) {
    PlaylistStore* ps = Session::getInstance()->getPlaylistStore();
    CMediaSource playlistShare;
    for (int i = 0; i < ps->getPlaylistCount(); i++) {
      if (!ps->getPlaylist(i)->isFolder() && ps->getPlaylist(i)->isLoaded()) {
        playlistShare.strPath.Format("musicdb://3/spotify:playlist:%i", i);
        const char* owner = ps->getPlaylist(i)->getOwnerName();
        if (owner != NULL
        )
          playlistShare.strName.Format("%s %s %s", ps->getPlaylist(i)->getName(), Settings::getByString(), owner);
        else
          playlistShare.strName.Format("%s", ps->getPlaylist(i)->getName());
        CFileItemPtr pItem(new CFileItem(playlistShare));
        SxThumb* thumb = ps->getPlaylist(i)->getThumb();
        if (thumb != NULL ) pItem->SetThumbnailImage(thumb->getPath());
        pItem->SetProperty("fanart_image", Settings::getFanart());
        items.Add(pItem);
      }
    }
  }
  return true;
}

bool Addon_music_spotify::GetAlbums(CFileItemList& items, CStdString& path, CStdString artistName) {
  CURL url(path);
  CStdString uri = url.GetFileNameWithoutPath();
  if (uri.Left(14).Equals("spotify:artist")) {
    return getArtistAlbums(items, uri);
  } else if (uri.Left(15).Equals("spotify:toplist")) {
    return getTopListAlbums(items);
  } else {
    return getAllAlbums(items, artistName);
  }
  return true;
}

bool Addon_music_spotify::getArtistAlbums(CFileItemList& items, CStdString& artistStr) {
  if (isReady()) {
    sp_link *spLink = sp_link_create_from_string(artistStr);
    if (!spLink) return true;
    sp_artist *spArtist = sp_link_as_artist(spLink);
    if (!spArtist) return true;
    SxArtist* artist = ArtistStore::getInstance()->getArtist(spArtist, true);
    if (!artist) return true;

    //if its the first call the artist might not be loaded yet
    artist->doLoadTracksAndAlbums();
    Logger::printOut("get artist albums, done browsing, waiting for all albums to load");
    while (!artist->isAlbumsLoaded()) {
      // Session::getInstance()->processEvents();
    }

    //add the similar artists item
    if (artist->getArtists().size() > 0) {
      CFileItemPtr pItem(new CFileItem(Settings::getSimilarArtistsString()));
      CStdString path;
      path.Format("musicdb://1/%s/", artistStr);
      pItem->SetPath(path);
      pItem->m_bIsFolder = true;
      items.Add(pItem);
      pItem->SetIconImage("DefaultMusicArtists.png");
      pItem->SetProperty("fanart_image", Settings::getFanart());
    }

    //add the albums
    vector<SxAlbum*> albums = artist->getAlbums();
    for (int i = 0; i < albums.size(); i++) {
      items.Add(SxAlbumToItem(albums[i]));

    }
  }
  return true;
}

bool Addon_music_spotify::getAllAlbums(CFileItemList& items, CStdString& artistStr) {

  Logger::printOut("get album");
  if (isReady()) {
    if (artistStr.IsEmpty()) {
      //load all starred albums
      PlaylistStore* ps = Session::getInstance()->getPlaylistStore();
      StarredList* sl = ps->getStarredList();
      if (sl == NULL
      ) return true;
      if (!sl->isLoaded()) return true;
      for (int i = 0; i < sl->getNumberOfAlbums(); i++) {
        SxAlbum* album = sl->getAlbum(i);
        //if its a multidisc we need to add them all
        int discNumber = album->getNumberOfDiscs();
        if (discNumber == 1)
          items.Add(SxAlbumToItem(sl->getAlbum(i)));
        else {
          while (discNumber > 0) {
            items.Add(SxAlbumToItem(sl->getAlbum(i), "", discNumber));
            discNumber--;
          }
        }
      }
    } else {
      //TODO do a small search for the artist and fetch the albums from the result

    }
  }
  return true;

}

bool Addon_music_spotify::GetTracks(CFileItemList& items, CStdString& path, CStdString artistName, int albumId) {
  Logger::printOut("get tracks");
  CURL url(path);
  CStdString uri = url.GetFileNameWithoutPath();
  //the path will look something like this "musicdb://2/spotify:artist:0LcJLqbBmaGUft1e9Mm8HV/-1/"
  //if we are trying to show all tracks for a spotify artist, we cant use the params becouse they are only integers.
  CURL url2(path.Left(path.GetLength() - 3));
  CStdString artist = url2.GetFileNameWithoutPath();

  if (uri.Left(13).Equals("spotify:album")) {
    return getAlbumTracks(items, uri);
  } else if (artist.Left(14).Equals("spotify:artist")) {
    return getArtistTracks(items, artist);
  } else if (uri.Left(16).Equals("spotify:playlist")) {
    uri.Delete(0, 17);
    return getPlaylistTracks(items, atoi(uri));
  } else if (artist.Left(15).Equals("spotify:toplist")) {
    return g_spotify->getTopListTracks(items);
  } else if (uri.Left(13).Equals("spotify:radio")) {
    return getRadioTracks(items, atoi(uri.Right(1)));
  } else if (albumId == -1) {
    return getAllTracks(items, artistName);
  }
  return true;
}

bool Addon_music_spotify::getAlbumTracks(CFileItemList& items, CStdString& path) {
  if (isReady()) {
    //lets split the string to get the album uri and the disc number
    CStdString uri = path.Left(path.Find('#'));
    CStdString discStr = path.Right(path.GetLength() - path.Find('#') - 1);
    Logger::printOut(discStr);
    int disc = atoi(discStr);

    //0 means its not a multidisc, so we need to change it to 1
    if (disc == 0) disc = 1;

    sp_link *spLink = sp_link_create_from_string(uri);
    sp_album *spAlbum = sp_link_as_album(spLink);

    SxAlbum* salbum = AlbumStore::getInstance()->getAlbum(spAlbum, true);

    vector<SxTrack*> tracks = salbum->getTracks();
    for (int i = 0; i < tracks.size(); i++) {
      if (disc == tracks[i]->getDisc()) items.Add(SxTrackToItem(tracks[i]));
    }
  }
  return true;
}

bool Addon_music_spotify::getArtistTracks(CFileItemList& items, CStdString& path) {
  Logger::printOut("get artist tracks");
  if (isReady()) {
    sp_link *spLink = sp_link_create_from_string(path);
    if (!spLink) return true;
    sp_artist *spArtist = sp_link_as_artist(spLink);
    if (!spArtist) return true;
    SxArtist* artist = ArtistStore::getInstance()->getArtist(spArtist, true);
    if (!artist) return true;

    //if its the first call the artist might not be loaded yet, the artist will update the view when its ready
    artist->doLoadTracksAndAlbums();
    while (!artist->isTracksLoaded())
      ;
    vector<SxTrack*> tracks = artist->getTracks();
    for (int i = 0; i < tracks.size(); i++) {
      items.Add(SxTrackToItem(tracks[i]));
    }
  }
  return true;
}

bool Addon_music_spotify::getAllTracks(CFileItemList& items, CStdString& path) {
  Logger::printOut("get tracks");
  Logger::printOut(path);
  if (isReady()) {
    if (path.IsEmpty()) {
      //load the starred tracks
      PlaylistStore* ps = Session::getInstance()->getPlaylistStore();
      StarredList* sl = ps->getStarredList();

      if (sl == NULL
      ) return true;
      if (!sl->isLoaded()) return true;

      for (int i = 0; i < sl->getNumberOfTracks(); i++) {
        items.Add(SxTrackToItem(sl->getTrack(i)));
      }
    }
  }
  return true;
}

bool Addon_music_spotify::getRadioTracks(CFileItemList& items, int radio) {
  Logger::printOut("get radio tracks");
  if (isReady()) {
    int lowestTrackNumber = RadioHandler::getInstance()->getLowestTrackNumber(radio);
    if (radio == 1 || radio == 2) {
      vector<SxTrack*> tracks = RadioHandler::getInstance()->getTracks(radio);
      for (int i = 0; i < tracks.size(); i++) {
        const CFileItemPtr pItem = SxTrackToItem(tracks[i], "", i + lowestTrackNumber + 1);
        CStdString path;
        path.Format("%s%s%i%s%i", pItem->GetPath(), "radio#", radio, "#", i + lowestTrackNumber);
        pItem->SetPath(path);
        items.Add(pItem);
      }
    }
    return true;
  }
  return false;
}

bool Addon_music_spotify::GetArtists(CFileItemList& items, CStdString& path) {
  CURL url(path);
  CStdString uri = url.GetFileNameWithoutPath();
  if (uri.Left(15).Equals("spotify:toplist")) {
    getTopListArtists(items);
  } else if (uri.Left(14).Equals("spotify:artist")) {
    getArtistSimilarArtists(items, uri);
  } else {
    getAllArtists(items);
  }
  return true;
}

bool Addon_music_spotify::getAllArtists(CFileItemList& items) {
  Logger::printOut("get starred artists");
  if (isReady()) {
    PlaylistStore* ps = Session::getInstance()->getPlaylistStore();
    StarredList* sl = ps->getStarredList();

    if (sl == NULL
    ) return true;
    if (!sl->isLoaded()) return true;

    for (int i = 0; i < sl->getNumberOfArtists(); i++) {
      items.Add(SxArtistToItem(sl->getArtist(i)));
    }
  }
  return true;
}

bool Addon_music_spotify::getArtistSimilarArtists(CFileItemList& items, CStdString uri) {
  Logger::printOut("get similar artists");
  if (isReady()) {
    sp_link *spLink = sp_link_create_from_string(uri);
    if (!spLink) return true;
    sp_artist *spArtist = sp_link_as_artist(spLink);
    if (!spArtist) return true;
    SxArtist* artist = ArtistStore::getInstance()->getArtist(spArtist, true);
    if (!artist) return true;

    //if its the first call the artist might not be loaded yet, the artist will update the view when its ready
    artist->doLoadTracksAndAlbums();
    while (!artist->isTracksLoaded())
      ;
    vector<SxArtist*> artists = artist->getArtists();
    for (int i = 0; i < artists.size(); i++) {
      items.Add(SxArtistToItem(artists[i]));
    }
  }
  return true;
}

bool Addon_music_spotify::getPlaylistTracks(CFileItemList& items, int index) {
  Logger::printOut("get playlist tracks");
  if (isReady()) {
    PlaylistStore* ps = Session::getInstance()->getPlaylistStore();
    SxPlaylist* pl = ps->getPlaylist(index);

    for (int i = 0; i < pl->getNumberOfTracks(); i++) {
      items.Add(SxTrackToItem(pl->getTrack(i), "", i + 1));
    }
  }
  return true;
}

bool Addon_music_spotify::GetTopLists(CFileItemList& items) {
  if (isReady()) {
    Logger::printOut("get the toplist entry list");
    TopLists* topLists = Session::getInstance()->getTopLists();

    if (topLists == NULL || !topLists->isLoaded()) return true;

    //add the tracks entry
    CFileItemPtr pItem(new CFileItem(Settings::getTopListTrackString()));
    CStdString path;
    path.Format("musicdb://2/spotify:toplist/-1/");
    pItem->SetPath(path);
    pItem->m_bIsFolder = true;
    items.Add(pItem);
    pItem->SetProperty("fanart_image", Settings::getFanart());

    //add the album entry
    CFileItemPtr pItem2(new CFileItem(Settings::getTopListAlbumString()));
    path.Format("musicdb://2/spotify:toplist");
    pItem2->SetPath(path);
    pItem2->m_bIsFolder = true;
    items.Add(pItem2);
    pItem2->SetProperty("fanart_image", Settings::getFanart());

    //add the artist entry
    CFileItemPtr pItem3(new CFileItem(Settings::getTopListArtistString()));
    path.Format("musicdb://1/spotify:toplist");
    pItem3->SetPath(path);
    pItem3->m_bIsFolder = true;
    items.Add(pItem3);
    pItem3->SetProperty("fanart_image", Settings::getFanart());
  }
  return true;
}

bool Addon_music_spotify::GetCustomEntries(CFileItemList& items) {
  if (isReady()) {
    //add radio 1
    CStdString name;
    name.Format("%s%s", Settings::getRadioPrefixString(), Settings::getRadio1Name());
    CFileItemPtr pItem(new CFileItem(name));
    CStdString path;
    path.Format("musicdb://3/spotify:radio:1/");
    pItem->SetPath(path);
    pItem->m_bIsFolder = true;
    items.Add(pItem);
    pItem->SetProperty("fanart_image", Settings::getFanart());

    //add radio 2
    name.Format("%s%s", Settings::getRadioPrefixString(), Settings::getRadio2Name());
    CFileItemPtr pItem2(new CFileItem(name));
    path.Format("musicdb://3/spotify:radio:2/");
    pItem2->SetPath(path);
    pItem2->m_bIsFolder = true;
    items.Add(pItem2);
    pItem2->SetProperty("fanart_image", Settings::getFanart());

  }
  return true;
}

bool Addon_music_spotify::getTopListArtists(CFileItemList& items) {
  Logger::printOut("get toplist artists");
  if (isReady()) {
    PlaylistStore* ps = Session::getInstance()->getPlaylistStore();
    TopLists* tl = ps->getTopLists();

    if (!tl->isArtistsLoaded()) tl->reLoadArtists();

    while (!tl->isArtistsLoaded()) {
      //Session::getInstance()->processEvents();
    }

    vector<SxArtist*> artists = tl->getArtists();
    for (int i = 0; i < artists.size(); i++) {
      items.Add(SxArtistToItem(artists[i]));
    }

  }
  return true;
}

bool Addon_music_spotify::getTopListAlbums(CFileItemList& items) {
  Logger::printOut("get toplist albums");
  if (isReady()) {
    PlaylistStore* ps = Session::getInstance()->getPlaylistStore();
    TopLists* tl = ps->getTopLists();

    if (!tl->isAlbumsLoaded()) tl->reLoadAlbums();

    while (!tl->isAlbumsLoaded()) {
      // Session::getInstance()->processEvents();
    }

    vector<SxAlbum*> albums = tl->getAlbums();
    for (int i = 0; i < albums.size(); i++) {
      items.Add(SxAlbumToItem(albums[i]));
    }
  }
  return true;
}

bool Addon_music_spotify::getTopListTracks(CFileItemList& items) {
  Logger::printOut("get toplist tracks");
  if (isReady()) {
    PlaylistStore* ps = Session::getInstance()->getPlaylistStore();
    TopLists* tl = ps->getTopLists();

    if (!tl->isTracksLoaded()) tl->reLoadTracks();

    while (!tl->isTracksLoaded()) {
      //Session::getInstance()->processEvents();
    }

    vector<SxTrack*> tracks = tl->getTracks();
    for (int i = 0; i < tracks.size(); i++) {
      items.Add(SxTrackToItem(tracks[i], "", i + 1));
    }
  }
  return true;
}

bool Addon_music_spotify::Search(CStdString query, CFileItemList& items) {
//do the search, if we are already searching and are to fetch results this want do anything
  Logger::printOut("new search");
  if (isReady()) {
    if (!SearchHandler::getInstance()->search(query)) {
      CStdString albumPrefix;
      albumPrefix.Format("[%s] ", g_localizeStrings.Get(558).c_str());
      vector<SxAlbum*> albums = SearchHandler::getInstance()->getAlbumResults();
      for (int i = 0; i < albums.size(); i++) {
        //if its a multidisc we need to add them all
        int discNumber = albums[i]->getNumberOfDiscs();
        if (discNumber == 1) {
          items.Add(SxAlbumToItem(albums[i], albumPrefix));
        } else {
          while (discNumber > 0) {
            items.Add(SxAlbumToItem(albums[i], albumPrefix, discNumber));
            discNumber--;
          }
        }
      }
      Logger::printOut("search fetch tracks");
      vector<SxTrack*> tracks = SearchHandler::getInstance()->getTrackResults();
      for (int i = 0; i < tracks.size(); i++)
        items.Add(SxTrackToItem(tracks[i]));

      CStdString artistPrefix;
      artistPrefix.Format("[%s] ", g_localizeStrings.Get(557).c_str());

      Logger::printOut("search fetch artists");
      vector<SxArtist*> artists = SearchHandler::getInstance()->getArtistResults();
      for (int i = 0; i < artists.size(); i++)
        items.Add(SxArtistToItem(artists[i], artistPrefix));
    }
  }
  return true;
}

ICodec* Addon_music_spotify::GetCodec() {
  return (ICodec*) PlayerHandler::getInstance()->getCodec();
}

const CFileItemPtr Addon_music_spotify::SxAlbumToItem(SxAlbum *album, string prefix, int discNumber) {
//wait for it to finish loading
  while (!album->isLoaded()) {
  }

  CAlbum outAlbum;
  outAlbum.strArtist = album->getAlbumArtistName();
  CStdString title;
  if (discNumber != 0)
    title.Format("%s%s %s %i", prefix, album->getAlbumName(), "disc", discNumber);
  else
    title.Format("%s%s", prefix, album->getAlbumName());
  outAlbum.strAlbum = title;
  outAlbum.iYear = album->getAlbumYear();
  outAlbum.strReview = album->getReview();
  outAlbum.iRating = album->getAlbumRating();
  CStdString path;
  path.Format("musicdb://3/%s#%i", album->getUri(), discNumber);
  const CFileItemPtr pItem(new CFileItem(path, outAlbum));
  if (album->hasThumb()) pItem->SetThumbnailImage(album->getThumb()->getPath());
  pItem->SetProperty("fanart_image", Settings::getFanart());
  return pItem;
}

const CFileItemPtr Addon_music_spotify::SxTrackToItem(SxTrack* track, string prefix, int trackNumber) {
//wait for it to finish loading
  while (!track->isLoaded()) {
  }

  CSong outSong;
  CStdString path;
  path.Format("%s.spotify", track->getUri());
  outSong.strFileName = path;
  CStdString title;
  title.Format("%s%s", prefix, track->getName());
  outSong.strTitle = title;
  outSong.iTrack = trackNumber == -1 ? track->getTrackNumber() : trackNumber;
  outSong.iDuration = track->getDuration();
  outSong.rating = track->getRating();
  char* ratingChar = new char();
  CStdString ratingStr = itoa(1 + (track->getRating() / 2), ratingChar, 10);
  delete ratingChar;
  outSong.rating = ratingStr[0];
  outSong.strArtist = track->getArtistName();
  outSong.iYear = track->getYear();
  outSong.strAlbum = track->getAlbumName();
  outSong.strAlbumArtist = track->getAlbumArtistName();

  const CFileItemPtr pItem(new CFileItem(outSong));
  if (track->hasThumb()) pItem->SetThumbnailImage(track->getThumb()->getPath());
  pItem->SetProperty("fanart_image", Settings::getFanart());
  return pItem;
}

const CFileItemPtr Addon_music_spotify::SxArtistToItem(SxArtist* artist, string prefix) {
  //wait for it to finish loading
  while (!artist->isLoaded()) {
  }

  CStdString path;
  path.Format("musicdb://2/%s/", artist->getUri());

  CFileItemPtr pItem(new CFileItem(path, true));
  CStdString label;
  label.Format("%s%s", prefix, artist->getArtistName());
  pItem->SetLabel(label);
  label.Format("A %s", artist->getArtistName());

  pItem->GetMusicInfoTag()->SetTitle(label);
  if (artist->hasThumb()) pItem->SetThumbnailImage(artist->getThumb()->getPath());
  pItem->SetIconImage("DefaultArtist.png");
  pItem->SetProperty("fanart_image", Settings::getFanart());
  pItem->SetProperty("artist_description", artist->getBio());

  return pItem;
}

