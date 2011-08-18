/*
 * TrackContainer.h
 *
 *  Created on: Aug 17, 2011
 *      Author: david
 */

#ifndef TRACKCONTAINER_H_
#define TRACKCONTAINER_H_

#include "SxTrack.h"
#include <vector>
#include "FileItem.h"


using namespace std;

namespace addon_music_spotify {

  class TrackContainer {
  public:
    TrackContainer();
    virtual ~TrackContainer();

    virtual bool getTrackItems(CFileItemList& items) = 0;

  protected:
    vector<SxTrack*> m_tracks;

    void removeAllTracks();
    bool tracksLoaded();
  };

} /* namespace addon_music_spotify */
#endif /* TRACKCONTAINER_H_ */
