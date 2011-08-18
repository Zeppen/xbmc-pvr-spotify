
#ifndef ARTISTCONTAINER_H_
#define ARTISTCONTAINER_H_

#include <vector>
#include "FileItem.h"

using namespace std;

namespace addon_music_spotify {

  class SxArtist;

  class ArtistContainer {
  public:
    ArtistContainer();
    virtual ~ArtistContainer();

    virtual bool getArtistItems(CFileItemList& items) = 0;

  protected:
    vector<SxArtist*> m_artists;

    void removeAllArtists();
    bool artistsLoaded();
  };

} /* namespace addon_music_spotify */
#endif /* ARTISTCONTAINER_H_ */
