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

#include "StarredList.h"
#include <set>

#include "../session/Session.h"
#include "../album/SxAlbum.h"
#include "../track/SxTrack.h"
#include "../artist/SxArtist.h"
#include "../album/AlbumStore.h"
#include "../track/TrackStore.h"
#include "../artist/ArtistStore.h"
#include "../XBMCUpdater.h"

namespace addon_music_spotify {

using namespace std;

StarredList::StarredList(sp_playlist* spPlaylist) :
    SxPlaylist(spPlaylist, 0, false) {
  populateAlbumsAndArtists();

}

StarredList::~StarredList() {
  while (!m_albums.empty()) {
    AlbumStore::getInstance()->removeAlbum(m_albums.back());
    m_albums.pop_back();
  }

  while (!m_artists.empty()) {
    ArtistStore::getInstance()->removeArtist(m_artists.back());
    m_artists.pop_back();
  }
}

void StarredList::populateAlbumsAndArtists() {
  Logger::printOut("Populate starred albums and artists");
  //ok so we got the tracks list populated within the playlist, now create the albums we want

  set<sp_album*> tempAlbums;
  set<sp_artist*> tempArtists;
  for (int i = 0; i < getNumberOfTracks(); i++) {

    sp_album* tempAlbum  = sp_track_album(m_tracks[i]->getSpTrack());
    if (tempAlbum != NULL){
      tempAlbums.insert(tempAlbum);
      tempArtists.insert(sp_track_artist(m_tracks[i]->getSpTrack(),0));
    }
  }

  //now we have a set with unique sp_albums, lets load them and see if they have all tracks starred or not

  vector<SxAlbum*> newAlbums;
  //find out witch albums that has complete set of starred tracks
  for (set<sp_album*>::const_iterator albumIt = tempAlbums.begin(); albumIt != tempAlbums.end(); ++albumIt) {
    SxAlbum* album = AlbumStore::getInstance()->getAlbum((sp_album*)*albumIt, true);
    Logger::printOut(album->getAlbumName());
    while (!album->isLoaded()) {
        clock_t goal = 50 + clock();
        while (goal > clock())
          ;
      }

    if (!album->hasTracksAndDetails() || album->getTracks().size() == 0 ||  !sp_album_is_available(album->getSpAlbum())) {
      AlbumStore::getInstance()->removeAlbum(album);
      continue;
    }
    bool albumIsStarred = true;
    vector<SxTrack*> tracks = album->getTracks();
    for (int i = 0; i < tracks.size(); i++) {
      if (!sp_track_is_starred(Session::getInstance()->getSpSession(), tracks[i]->getSpTrack())) {
        albumIsStarred = false;
        break;
      }
    }

    if (albumIsStarred)
      newAlbums.push_back(AlbumStore::getInstance()->getAlbum(album->getSpAlbum(), true));
  }
  while (!m_albums.empty()) {
    AlbumStore::getInstance()->removeAlbum(m_albums.back()->getSpAlbum());
    m_albums.pop_back();
  }

  //add all artists that we have collected

  vector<SxArtist*> newArtists;
  for (set<sp_artist*>::const_iterator artistIt = tempArtists.begin(); artistIt != tempArtists.end(); ++artistIt) {
    SxArtist* artist = ArtistStore::getInstance()->getArtist((sp_artist*)*artistIt, false);
    if (artist != NULL)
      newArtists.push_back(artist);
  }

  while (!m_artists.empty()) {
    ArtistStore::getInstance()->removeArtist(m_artists.back());
    m_artists.pop_back();
  }

  m_artists = newArtists;

  m_albums = newAlbums;
  Logger::printOut("Populate starred albums and artists done");
}

void StarredList::reLoad() {
  Logger::printOut("reload star");
  if (m_isValid && !isFolder()) {
    vector<SxTrack*> newTracks;
    for (int index = 0; index < sp_playlist_num_tracks(m_spPlaylist); index++) {
      if (!sp_track_is_available(Session::getInstance()->getSpSession(),sp_playlist_track(m_spPlaylist, index)))
        continue;
      SxTrack* track = TrackStore::getInstance()->getTrack(sp_playlist_track(m_spPlaylist, index));
      if (track) {
        newTracks.push_back(track);
      }
    }
    while (!m_tracks.empty()) {
      TrackStore::getInstance()->removeTrack(m_tracks.back()->getSpTrack());
      m_tracks.pop_back();
    }
    m_tracks = newTracks;
  }
  populateAlbumsAndArtists();
  Logger::printOut("reload star done");
  XBMCUpdater::updateAllAlbums();
  XBMCUpdater::updateAllTracks();
  //TODO update all artists

}

SxAlbum *StarredList::getAlbum(int index) {
  if (index < getNumberOfAlbums())
    return m_albums[index];
  return NULL;
}

SxArtist *StarredList::getArtist(int index) {
  if (index < getNumberOfArtists())
    return m_artists[index];
  return NULL;
}

bool StarredList::isLoaded() {
  for (int i = 0; i < m_albums.size(); i++) {
    if (!m_albums[i]->isLoaded())
      return false;
  }
  for (int i = 0; i < m_tracks.size(); i++) {
    if (!m_tracks[i]->isLoaded())
      return false;
  }
  for (int i = 0; i < m_artists.size(); i++) {
    if (!m_artists[i]->isLoaded())
      return false;
  }


  return true;
}

} /* namespace addon_music_spotify */

