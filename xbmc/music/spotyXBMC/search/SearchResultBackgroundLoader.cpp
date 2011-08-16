/*
 * SearchResultBackgroundLoading.cpp
 *
 *  Created on: Aug 16, 2011
 *      Author: david
 */
#include "../album/AlbumStore.h"
#include "../track/TrackStore.h"
#include "../artist/ArtistStore.h"
#include "../session/Session.h"
#include "SearchResultBackgroundLoader.h"
#include "../XBMCUpdater.h"


namespace addon_music_spotify {

  SearchResultBackgroundLoader::SearchResultBackgroundLoader(Search* search) {
    m_search = search;
  }

  SearchResultBackgroundLoader::~SearchResultBackgroundLoader() {
    // TODO Auto-generated destructor stub
  }

  void SearchResultBackgroundLoader::OnStartup() {
  }

  void SearchResultBackgroundLoader::OnExit() {
  }

  void SearchResultBackgroundLoader::OnException() {
  }

  void SearchResultBackgroundLoader::Process() {
    while (!Session::getInstance()->lock()) {
      SleepMs(10);
    }

    if (m_search->m_cancelSearch) {
      Logger::printOut("search results arived, aborting due to request");
      sp_search_release(m_search->m_currentSearch);
      return;
    }
    Logger::printOut("search results arived");

    //add the albums
    for (int index = 0; index < sp_search_num_albums(m_search->m_currentSearch); index++) {
      if (sp_album_is_available(sp_search_album(m_search->m_currentSearch, index))) {
        m_search->m_albums.push_back(AlbumStore::getInstance()->getAlbum(sp_search_album(m_search->m_currentSearch, index), true));
      }
    }

    //add the tracks
    for (int index = 0; index < sp_search_num_tracks(m_search->m_currentSearch); index++) {
      if (sp_track_is_available(Session::getInstance()->getSpSession(), sp_search_track(m_search->m_currentSearch, index))) {
        m_search->m_tracks.push_back(TrackStore::getInstance()->getTrack(sp_search_track(m_search->m_currentSearch, index)));
      }
    }

    //add the artists
    for (int index = 0; index < sp_search_num_artists(m_search->m_currentSearch); index++) {
      //dont load the albums and tracks for all artists here, it takes forever
      m_search->m_artists.push_back(ArtistStore::getInstance()->getArtist(sp_search_artist(m_search->m_currentSearch, index), false));
    }

    sp_search_release(m_search->m_currentSearch);

    //wait for all albums, tracks and artists to load before calling out
    Session::getInstance()->unlock();
    while (!m_search->isLoaded()) {
      SleepMs(10);
    }

    XBMCUpdater::updateSearchResults(m_search->m_query);
    Logger::printOut("search results done");
  }

}

/* namespace addon_music_spotify */
