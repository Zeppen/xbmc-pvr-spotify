/*
 * TrackContainer.cpp
 *
 *  Created on: Aug 17, 2011
 *      Author: david
 */

#include "TrackContainer.h"
#include "TrackStore.h"
#include "SxTrack.h"

namespace addon_music_spotify {

  TrackContainer::TrackContainer() {
    // TODO Auto-generated constructor stub

  }

  TrackContainer::~TrackContainer() {
    // TODO Auto-generated destructor stub
  }

  void TrackContainer::removeAllTracks() {
    while (!m_tracks.empty()) {
      TrackStore::getInstance()->removeTrack(m_tracks.back());
      m_tracks.pop_back();
    }
  }

  bool TrackContainer::tracksLoaded() {
    for (int i = 0; i < m_tracks.size(); i++) {
      if (!m_tracks[i]->isLoaded()) return false;
    }
    return true;
  }

} /* namespace addon_music_spotify */
