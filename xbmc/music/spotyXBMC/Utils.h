/*
 * Utils.h
 *
 *  Created on: Aug 17, 2011
 *      Author: david
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include "artist/SxArtist.h"
#include "track/SxTrack.h"
#include "album/SxAlbum.h"

using namespace std;

namespace addon_music_spotify {

  class Utils {
  public:
    static void cleanTags(string& text);

    static const CFileItemPtr SxAlbumToItem(SxAlbum* album, string prefix = "", int discNumber = 0);
    static const CFileItemPtr SxTrackToItem(SxTrack* track, string prefix = "", int trackNumber = -1);
    static const CFileItemPtr SxArtistToItem(SxArtist* artist, string prefix = "");

    static void createDir(string path);
    static void removeDir(string path);
    static void removeFile(string file);

    static void updateMenu();
    static void updatePlaylists();
    static void updatePlaylist(int index);
    static void updateAllArtists();
    static void updateAllAlbums();
    static void updateAllTracks();
    static void updateRadio(int radio);
    static void updateToplistMenu();
    static void updateSearchResults(string query);

  private:
    Utils();
    virtual ~Utils();
    static void updatePath(CStdString& path);
  };

} /* namespace addon_music_spotify */
#endif /* UTILS_H_ */
