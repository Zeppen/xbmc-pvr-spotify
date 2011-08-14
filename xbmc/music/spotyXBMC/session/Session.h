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

#ifndef SESSION_H_
#define SESSION_H_

namespace spotify {
#include <libspotify/api.h>
}

using namespace spotify;

#include <stdlib.h>
#include <stdio.h>
#include "../../../threads/SystemClock.h"
#include "SessionCallbacks.h"
#include "../playlist/PlaylistStore.h"
#include "../Settings.h"
#include "../Logger.h"

namespace addon_music_spotify {

  class PlayLists;
  class TopLists;
  class TrackStore;
  class AlbumStore;

  class Session {
  public:

    static Session *getInstance();
    static void deInit();
    bool processEvents();

    void notifyMainThread() {
      m_nextEvent.SetExpired();
    }

    void loggedIn();
    void loggedOut();
    bool isLoggedOut() {
      return m_isLoggedOut;
    }
    bool enable();
    bool disable();
    bool isEnabled() {
      return m_isEnabled;
    }
    bool isReady();

    sp_session *getSpSession() {
      return m_session;
    }

    PlaylistStore* getPlaylistStore() {
      return m_playlists;
    }
    TopLists* getTopLists() {
      if (m_playlists == NULL || !m_playlists->isLoaded()) return NULL;
      return m_playlists->getTopLists();
    }

  private:
    Session();
    virtual ~Session();

    static Session *m_instance;
    sp_session *m_session;
    SessionCallbacks *m_sessionCallbacks;
    XbmcThreads::EndTime m_nextEvent;

    bool m_isEnabled;
    bool m_isLoggedOut;

    PlaylistStore *m_playlists;

    bool connect();
    bool disConnect();
  };

} /* namespace addon_music_spotify */
#endif /* SESSION_H_ */
