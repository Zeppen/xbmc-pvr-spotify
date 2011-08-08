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

#ifndef XBMCUPDATER_H_
#define XBMCUPDATER_H_

#include <string>

using namespace std;

namespace addon_music_spotify {

class XBMCUpdater {
public:
  static void updateMenu();
  static void updatePlaylists();
  static void updatePlaylist(int index);

  static void updateAllAlbums();
  static void updateAllTracks();

  static void updateRadio(int radio);

  static void updateSearchResults(string query);

private:
  XBMCUpdater();
  virtual ~XBMCUpdater();
};

} /* namespace addon_music_spotify */
#endif /* XBMCUPDATER_H_ */
