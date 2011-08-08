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

#include "SxRadio.h"
#include "../Logger.h"
#include "../track/SxTrack.h"
#include "../track/TrackStore.h"
#include "../session/Session.h"
#include "RadioHandler.h"

namespace addon_music_spotify {

SxRadio::SxRadio(int radioNumber, int fromYear, int toYear, sp_radio_genre genres) {
  m_radioNumber = radioNumber;
  m_fromYear = fromYear;
  m_toYear = toYear;
  m_genres = genres;
  m_currentPlayingPos = 0;
  m_currentResultPos = 0;
  m_currentSearch = NULL;
  //hard code the number of tracks to 15 for now
  m_numberOfTracksToDisplay = Settings::getRadioNumberTracks();
  m_isWaitingForResults = false;
  fetchNewTracks();
}

SxRadio::~SxRadio() {
  while (!m_tracks.empty()) {
    TrackStore::getInstance()->removeTrack(m_tracks.back());
    m_tracks.pop_back();
  }
}

bool SxRadio::isLoaded() {
  if (m_isWaitingForResults)
    return false;
  for (int i = 0; i < m_tracks.size(); i++) {
    if (!m_tracks[i]->isLoaded())
      return false;
  }
  return true;
}

void SxRadio::pushToTrack(int trackNumber) {
  Logger::printOut("SxRadio::pushToTrack");
  while (m_currentPlayingPos < trackNumber) {
    //kind of stupid to use a vector here, a list or dequeue is maybe better, change sometime...
    TrackStore::getInstance()->removeTrack(m_tracks.front());
    m_tracks.erase(m_tracks.begin());
    m_currentPlayingPos++;
  }
  m_currentPlayingPos = trackNumber;
  fetchNewTracks();
}

void SxRadio::fetchNewTracks() {
  if (m_isWaitingForResults)
    return;

  //add the tracks
  if (m_currentSearch != NULL) {
    //if there are no tracks, return and break
    if (sp_search_num_tracks(m_currentSearch) == 0)
      return;

    for (int index = m_currentResultPos; index < sp_search_num_tracks(m_currentSearch) && m_tracks.size() < m_numberOfTracksToDisplay; index++) {
      if (sp_track_is_available(Session::getInstance()->getSpSession(), sp_search_track(m_currentSearch, index))) {
        m_tracks.push_back(TrackStore::getInstance()->getTrack(sp_search_track(m_currentSearch, index)));
      }
      m_currentResultPos++;
    }
  }

  //are we still missing tracks? Do a new search
  if (m_tracks.size() < m_numberOfTracksToDisplay) {
    m_isWaitingForResults = true;
    m_currentSearch = sp_radio_search_create(Session::getInstance()->getSpSession(), m_fromYear, m_toYear, m_genres, &cb_searchComplete, this);
  }
  //the list is full or we are waiting for a new search, update the view
  if (m_tracks.size() > 0)
    RadioHandler::getInstance()->allTracksLoaded(m_radioNumber);
  Logger::printOut("radio fetch tracks done");
}

void SxRadio::newResults(sp_search* search) {
  Logger::printOut("new radio result");
  m_currentSearch = search;
  m_currentResultPos = 0;
  m_isWaitingForResults = false;
  fetchNewTracks();
}

void SxRadio::SP_CALLCONV cb_searchComplete(sp_search *search, void *userdata) {
  SxRadio* searchObj = (SxRadio*) userdata;
  searchObj->newResults(search);
}

} /* namespace addon_music_spotify */
