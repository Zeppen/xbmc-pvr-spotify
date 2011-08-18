/*
 * TrackContainer.cpp
 *
 *  Created on: Aug 17, 2011
 *      Author: david
 */

#include "ArtistStore.h"
#include "ArtistContainer.h"
#include "SxArtist.h"

namespace addon_music_spotify {

  ArtistContainer::ArtistContainer() {
    // TODO Auto-generated constructor stub

  }

  ArtistContainer::~ArtistContainer() {
    // TODO Auto-generated destructor stub
  }

  void ArtistContainer::removeAllArtists() {
    while (!m_artists.empty()) {
      ArtistStore::getInstance()->removeArtist(m_artists.back());
      m_artists.pop_back();
    }
  }

  bool ArtistContainer::artistsLoaded() {
    for (int i = 0; i < m_artists.size(); i++) {
      if (!m_artists[i]->isLoaded()) return false;
    }
    return true;
  }

} /* namespace addon_music_spotify */
