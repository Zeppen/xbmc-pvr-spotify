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
#include "GUIUserMessages.h"
#include "guilib/GUIWindowManager.h"

namespace addon_music_spotify {

XBMCUpdater::XBMCUpdater() {
  // TODO Auto-generated constructor stub

}

XBMCUpdater::~XBMCUpdater() {
  // TODO Auto-generated destructor stub
}

void XBMCUpdater::updatePlaylists() {
  CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
  message.SetStringParam("special://musicplaylists/");
  g_windowManager.SendThreadMessage(message);
}

void XBMCUpdater::updateMenu(){
  Logger::printOut("updating main music menu");
  CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
  CStdString path;
  path.Format("");
  Logger::printOut(path);
  message.SetStringParam(path);
  g_windowManager.SendThreadMessage(message);
}

void XBMCUpdater::updatePlaylist(int index) {
  //TODO FIX!
  /* Logger::printOut("updating playlist view");
   CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
   CStdString path;
   Logger::printOut(path);
   path.Format("musicdb://3/spotify:playlist:%i", index);
   message.SetStringParam(path);
   g_windowManager.SendThreadMessage(message);*/
}

void XBMCUpdater::updateAllArtists() {
  Logger::printOut("updating all artists");
  CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
  CStdString path;
  path.Format("musicdb://2/");
  Logger::printOut(path);
  message.SetStringParam(path);
  g_windowManager.SendThreadMessage(message);
}

void XBMCUpdater::updateAllAlbums() {
  Logger::printOut("updating all albums");
  CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
  CStdString path;
  path.Format("musicdb://3/");
  Logger::printOut(path);
  message.SetStringParam(path);
  g_windowManager.SendThreadMessage(message);
}

void XBMCUpdater::updateAllTracks() {
  Logger::printOut("updating all tracks");
  CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
  CStdString path;
  path.Format("musicdb://4/");
  Logger::printOut(path);
  message.SetStringParam(path);
  g_windowManager.SendThreadMessage(message);
}

void XBMCUpdater::updateRadio(int radio){
  Logger::printOut("updating radio results");
  CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
  CStdString path;
  path.Format("musicdb://3/spotify:radio:%i/", radio);
  Logger::printOut(path);
  message.SetStringParam(path);
  g_windowManager.SendThreadMessage(message);
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
  CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
  CStdString path;
  path.Format("musicsearch://%s/", query);
  Logger::printOut(path);
  message.SetStringParam(path);
  g_windowManager.SendThreadMessage(message);
}

} /* namespace addon_music_spotify */
