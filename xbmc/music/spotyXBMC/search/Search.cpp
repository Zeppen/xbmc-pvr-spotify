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

#include "Search.h"
#include "../session/Session.h"
#include "../Logger.h"
#include "../XBMCUpdater.h"
#include "../Settings.h"

namespace addon_music_spotify {

Search::Search(string query) {
  //hardcoded, move out to settings later
  m_maxArtistResults = Settings::getSearchNumberArtists();
  m_maxAlbumResults = Settings::getSearchNumberAlbums();
  m_maxTrackResults = Settings::getSearchNumberTracks();

  m_query = query;

  m_artistsDone = false;
  m_albumsDone = false;
  m_tracksDone = false;

  //do the initial search
  m_cancelSearch = false;
  Logger::printOut("creating search");
  Logger::printOut(query);
  m_currentSearch = sp_search_create(Session::getInstance()->getSpSession(), m_query.c_str(), 0, m_maxTrackResults, 0, m_maxAlbumResults, 0, m_maxArtistResults, &cb_searchComplete, this);

}

Search::~Search() {
  //we need to wait for the results
  //m_cancelSearch = true;
  //while (m_currentSearch != NULL)
  //  ;
  Logger::printOut("cleaning after search");
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

  Logger::printOut("cleaning after search done");

}

void Search::newResults(sp_search *search) {
  m_currentSearch = NULL;
  if (m_cancelSearch) {
    Logger::printOut("search results arived, aborting due to request");
    sp_search_release(search);
    return;
  }
  Logger::printOut("search results arived");

  //add the albums
  for (int index = 0; index < sp_search_num_albums(search); index++) {
    if (sp_album_is_available(sp_search_album(search, index))) {
      m_albums.push_back(AlbumStore::getInstance()->getAlbum(sp_search_album(search, index), true));
    }
  }

//add the tracks
  for (int index = 0; index < sp_search_num_tracks(search); index++) {
    if (sp_track_is_available(Session::getInstance()->getSpSession(),sp_search_track(search, index))) {
      m_tracks.push_back(TrackStore::getInstance()->getTrack(sp_search_track(search, index)));
    }
  }

  //add the artists
  for (int index = 0; index < sp_search_num_artists(search); index++) {
    //dont load the albums and tracks for all artists here, it takes forever
    m_artists.push_back(ArtistStore::getInstance()->getArtist(sp_search_artist(search, index), false));
  }

  XBMCUpdater::updateSearchResults(m_query);
  sp_search_release(search);
  Logger::printOut("search results done");
}

void Search::SP_CALLCONV cb_searchComplete(sp_search *search, void *userdata) {
  Search* searchObj = (Search*) userdata;
  searchObj->newResults(search);
}

} /* namespace addon_music_spotify */
