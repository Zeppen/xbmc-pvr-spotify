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

#include <stdio.h>
#include <math.h>
#include "SxArtist.h"
#include "../session/Session.h"
#include "../Logger.h"

#include "../album/SxAlbum.h"
#include "../album/AlbumStore.h"
#include "../track/SxTrack.h"
#include "../track/TrackStore.h"
#include "../artist/SxArtist.h"
#include "../artist/ArtistStore.h"
#include "../thumb/SxThumb.h"
#include "../thumb/ThumbStore.h"

namespace addon_music_spotify {

SxArtist::SxArtist(sp_artist *artist, bool loadTracksAndAlbums) {
  m_spArtist = artist;
  Logger::printOut("creating artist");
  while (!sp_artist_is_loaded(m_spArtist))
    ;
  m_references = 1;
  m_browse = NULL;
  m_isLoadingDetails = false;
  m_hasDetails = false;
  m_hasTracksAndAlbums = false;
  m_thumb = NULL;
  m_hasThumb = false;
  m_bio = "";
  sp_link *link = sp_link_create_from_artist(artist);
  m_uri = new char[256];
  sp_link_as_string(link, m_uri, 256);
  sp_link_release(link);
  m_loadTrackAndAlbums = loadTracksAndAlbums;

  //disable for now, libspotify is not thread safe yet so this might crash it

// if (Settings::getPreloadArtistDetails())
  //   doLoadDetails();

  Logger::printOut("creating artist done");
}

SxArtist::~SxArtist() {
  while (m_isLoadingDetails) {
    Session::getInstance()->processEvents();
    Logger::printOut("waiting for artist to die");
  }

  while (!m_tracks.empty()) {
    TrackStore::getInstance()->removeTrack(m_tracks.back());
    m_tracks.pop_back();
  }

  while (!m_albums.empty()) {
    AlbumStore::getInstance()->removeAlbum(m_albums.back());
    m_albums.pop_back();
  }

  while (!m_artists.empty()) {
    ArtistStore::getInstance()->removeArtist(m_artists.back());
    m_artists.pop_back();
  }

  if (m_thumb)
    ThumbStore::getInstance()->removeThumb(m_thumb);
  delete m_uri;
  if (hasDetails() && m_browse != NULL)
    sp_artistbrowse_release(m_browse);
  sp_artist_release(m_spArtist);
}

bool SxArtist::isAlbumsLoaded() {
  if (!m_hasTracksAndAlbums)
    return false;

  for (int i = 0; i < m_albums.size(); i++) {
    if (!m_albums[i]->isLoaded())
      return false;
  }
  return true;
}

bool SxArtist::isTracksLoaded() {
  if (!m_hasTracksAndAlbums)
    return false;

  for (int i = 0; i < m_tracks.size(); i++) {
    if (!m_tracks[i]->isLoaded())
      return false;
  }

  return true;
}

bool SxArtist::isArtistsLoaded() {
  if (!m_hasTracksAndAlbums)
    return false;

  for (int i = 0; i < m_artists.size(); i++) {
    if (!m_artists[i]->isLoaded())
      return false;
  }

  return true;
}

void SxArtist::doLoadTracksAndAlbums() {
  Logger::printOut("loading artist tracks and albums");
  m_loadTrackAndAlbums = true;
  //if we allready have it all..
  if (m_hasTracksAndAlbums)
    return;

  //if the details not is preloaded, do a artist browse now
  if (!m_hasDetails) {
    if (!m_isLoadingDetails)
      doLoadDetails();
    return;
  }

  if (m_browse == NULL)
    return;

  //add the albums
  int maxAlbums = Settings::getArtistNumberAlbums() == -1 ? sp_artistbrowse_num_albums(m_browse) : Settings::getArtistNumberAlbums();

  int addedAlbums = 0;
  for (int index = 0; index < sp_artistbrowse_num_albums(m_browse) && addedAlbums < maxAlbums; index++) {
    if (sp_album_is_available(sp_artistbrowse_album(m_browse, index))) {
      m_albums.push_back(AlbumStore::getInstance()->getAlbum(sp_artistbrowse_album(m_browse, index), true));
      addedAlbums++;
    }
  }

  //add the tracks
  int maxTracks = Settings::getArtistNumberTracks() == -1 ? sp_artistbrowse_num_tracks(m_browse) : Settings::getArtistNumberTracks();

  int addedTracks = 0;
  for (int index = 0; index < sp_artistbrowse_num_tracks(m_browse) && addedTracks < maxTracks; index++) {
    if (sp_track_is_available(Session::getInstance()->getSpSession(), sp_artistbrowse_track(m_browse, index))) {
      m_tracks.push_back(TrackStore::getInstance()->getTrack(sp_artistbrowse_track(m_browse, index)));
      addedTracks++;
    }
  }

  //add the artists
  int maxArtists = Settings::getArtistNumberArtists() == -1 ? sp_artistbrowse_num_similar_artists(m_browse) : Settings::getArtistNumberArtists();

  int addedArtists = 0;
  for (int index = 0; index < sp_artistbrowse_num_similar_artists(m_browse) && addedArtists < maxArtists; index++) {
    m_artists.push_back(ArtistStore::getInstance()->getArtist(sp_artistbrowse_similar_artist(m_browse, index), false));
    addedArtists++;
  }

  Logger::printOut("loading artist tracks and albums done");
  m_hasTracksAndAlbums = true;
}

void SxArtist::doLoadDetails() {
  if (m_hasDetails || m_isLoadingDetails)
    return;

  m_isLoadingDetails = true;
  m_browse = sp_artistbrowse_create(Session::getInstance()->getSpSession(), m_spArtist, &cb_artistBrowseComplete, this);
}

void SxArtist::detailsLoaded(sp_artistbrowse *result) {
  if (sp_artistbrowse_error(result) == SP_ERROR_OK) {
    //check if there is a thumb
    if (sp_artistbrowse_num_portraits > 0) {
      const byte* image = sp_artistbrowse_portrait(result, 0);
      if (image) {
        m_thumb = ThumbStore::getInstance()->getThumb(image);
      }
    }
    if (m_thumb != NULL)
      m_hasThumb = true;

    m_bio = sp_artistbrowse_biography(result);
    //remove the links from the bio text (it contains spotify uris so maybe we can do something fun with it later)
    bool done = false;
    while (!done) {
      // Look for start of tag:
      size_t leftPos = m_bio.find('<');
      if (leftPos != string::npos) {
        // See if tag close is in this line:
        size_t rightPos = m_bio.find('>');
        if (rightPos == string::npos) {
          done = true;
          m_bio.erase(leftPos);
        } else
          m_bio.erase(leftPos, rightPos - leftPos + 1);
      } else
        done = true;
    }

    if (m_loadTrackAndAlbums) {
      //add the albums
      int maxAlbums = Settings::getArtistNumberAlbums() == -1 ? sp_artistbrowse_num_albums(m_browse) : Settings::getArtistNumberAlbums();

      int addedAlbums = 0;
      for (int index = 0; index < sp_artistbrowse_num_albums(m_browse) && addedAlbums < maxAlbums; index++) {
        if (sp_album_is_available(sp_artistbrowse_album(m_browse, index))) {
          m_albums.push_back(AlbumStore::getInstance()->getAlbum(sp_artistbrowse_album(m_browse, index), true));
          addedAlbums++;
        }
      }

      //add the tracks
      int maxTracks = Settings::getArtistNumberTracks() == -1 ? sp_artistbrowse_num_tracks(m_browse) : Settings::getArtistNumberTracks();

      int addedTracks = 0;
      for (int index = 0; index < sp_artistbrowse_num_tracks(m_browse) && addedTracks < maxTracks; index++) {
        if (sp_track_is_available(Session::getInstance()->getSpSession(), sp_artistbrowse_track(m_browse, index))) {
          m_tracks.push_back(TrackStore::getInstance()->getTrack(sp_artistbrowse_track(m_browse, index)));
          addedTracks++;
        }
      }

      //add the artists
      int maxArtists = Settings::getArtistNumberArtists() == -1 ? sp_artistbrowse_num_similar_artists(m_browse) : Settings::getArtistNumberArtists();

      int addedArtists = 0;
      for (int index = 0; index < sp_artistbrowse_num_similar_artists(m_browse) && addedArtists < maxArtists; index++) {
        m_artists.push_back(ArtistStore::getInstance()->getArtist(sp_artistbrowse_similar_artist(m_browse, index), false));
        addedArtists++;
      }

      m_hasTracksAndAlbums = true;
    }
  }
  m_isLoadingDetails = false;
  m_hasDetails = true;
  Logger::printOut("artist browse complete done");
}

void SxArtist::cb_artistBrowseComplete(sp_artistbrowse *result, void *userdata) {
  Logger::printOut("artist browse complete");
  SxArtist *artist = (SxArtist*) (userdata);
  Logger::printOut(artist->getArtistName());
  artist->detailsLoaded(result);
}
} /* namespace addon_music_spotify */

