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

#ifndef SXSETTINGS_H_
#define SXSETTINGS_H_

#include <libspotify/api.h>
#include "addons/Addon.h"
#include "addons/AddonManager.h"
#include "filesystem/Directory.h"
#include "filesystem/SpecialProtocol.h"

namespace addon_music_spotify {

  using namespace std;

  class Settings {
  public:
    Settings();
    virtual ~Settings();

    static bool enabled() {
      return getAddonSetting("enable") == "true";
    }
    static CStdString getUserName() {
      return getAddonSetting("username");
    }
    static CStdString getPassword() {
      return getAddonSetting("password");
    }
    static CStdString getCachePath();
    static CStdString getThumbPath();
    static bool useHighBitrate() {
      return getAddonSetting("highBitrate") == "true";
    }
    static bool useNormalization(){
    	return getAddonSetting("normalization") == "true";
    }

    static CStdString getFanart();

    static int getSearchNumberArtists() {
      return 10 * atoi(getAddonSetting("searchNoArtists")) + 1;
    }
    static int getSearchNumberAlbums() {
      return 10 * atoi(getAddonSetting("searchNoAlbums")) + 1;
    }
    static int getSearchNumberTracks() {
      return 10 * atoi(getAddonSetting("searchNoTracks")) + 1;
    }

    static bool getPreloadArtistDetails() {
      return getAddonSetting("preloadArtistDetails") == "true";
    }
    static int getArtistNumberArtists();
    static int getArtistNumberAlbums();
    static int getArtistNumberTracks();

    static CStdString getRadio1Name() {
      return getAddonSetting("radio1name");
    }
    static int getRadio1From() {
      return 1900 + (10 * (4 + atoi(getAddonSetting("radio1from"))));
    }
    static int getRadio1To() {
      return 1900 + (10 * (4 + atoi(getAddonSetting("radio1to"))));
    }
    static sp_radio_genre getRadio1Genres();

    static CStdString getRadio2Name() {
      return getAddonSetting("radio2name");
    }
    static int getRadio2From() {
      return 1900 + (10 * (4 + atoi(getAddonSetting("radio12rom"))));
    }
    static int getRadio2To() {
      return 1900 + (10 * (4 + atoi(getAddonSetting("radio2to"))));
    }
    static sp_radio_genre getRadio2Genres();

    static int getRadioNumberTracks() {
      return atoi(getAddonSetting("radioNoTracks")) + 3;
    }

    static bool toplistRegionEverywhere() {
      return getAddonSetting("topListRegion") == "1";
    }

    static bool getPreloadTopLists() {
      return getAddonSetting("preloadToplists") == "true";
    }
    static CStdString getByString() {
      return getAddonString(30002);
    }
    static CStdString getTopListArtistString() {
      return getAddonString(30500);
    }
    static CStdString getTopListAlbumString() {
      return getAddonString(30501);
    }
    static CStdString getTopListTrackString() {
      return getAddonString(30502);
    }
    static CStdString getRadioPrefixString() {
      return getAddonString(30503);
    }
    static CStdString getSimilarArtistsString() {
      return getAddonString(30504);
    }

  private:
    static CStdString getAddonSetting(CStdString key);
    static CStdString getAddonString(int id);
  };

} /* namespace addon_music_spotify */
#endif /* SXSETTINGS_H_ */
