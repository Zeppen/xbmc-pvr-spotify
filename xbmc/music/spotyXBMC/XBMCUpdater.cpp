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

#include "XBMCUpdater.h"
#include "Logger.h"
#include "guilib/GUIWindowManager.h"

namespace addon_music_spotify {

  void XBMCUpdater::updatePlaylists() {
    CStdString path;
    path.Format("special://musicplaylists/");
    updatePath(path);
  }

  void XBMCUpdater::updateMenu() {
    Logger::printOut("updating main music menu");
    CStdString path;
    path.Format("");
    updatePath(path);
  }

  void XBMCUpdater::updatePlaylist(int index) {
    //TODO FIX!
    Logger::printOut("updating playlist view");
    CStdString path;
    path.Format("musicdb://3/spotify:playlist:%i/", index);
    updatePath(path);
  }

  void XBMCUpdater::updateAllArtists() {
    Logger::printOut("updating all artists");
    CStdString path;
    path.Format("musicdb://2/");
    updatePath(path);
  }

  void XBMCUpdater::updateAllAlbums() {
    Logger::printOut("updating all albums");
    CStdString path;
    path.Format("musicdb://3/");
    updatePath(path);
  }

  void XBMCUpdater::updateAllTracks() {
    Logger::printOut("updating all tracks");
    CStdString path;
    path.Format("musicdb://4/");
    updatePath(path);
  }

  void XBMCUpdater::updateRadio(int radio) {
    Logger::printOut("updating radio results");
    CStdString path;
    path.Format("musicdb://3/spotify:radio:%i/", radio);
    updatePath(path);
  }

  void XBMCUpdater::updateToplistMenu() {
    Logger::printOut("updating toplistmenu");
    CStdString path;
    path.Format("musicdb://5/");
    updatePath(path);
  }

  void XBMCUpdater::updateSearchResults(string query) {
    //we need to replace the whitespaces with %20

    int pos = 0;
    while (pos != string::npos) {
      pos = query.find(' ');
      if (pos != string::npos) {
        query.replace(pos, 1, "%20");
      }
    }

    Logger::printOut("updating search results");
    CStdString path;
    path.Format("musicsearch://%s/", query);
    updatePath(path);
  }

  void XBMCUpdater::updatePath(CStdString& path) {
    CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
    message.SetStringParam(path);
    g_windowManager.SendThreadMessage(message);
  }

} /* namespace addon_music_spotify */
