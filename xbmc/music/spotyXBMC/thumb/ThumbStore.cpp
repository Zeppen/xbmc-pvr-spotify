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

#include "ThumbStore.h"
#include "SxThumb.h"
#include "../SxSettings.h"
#include "../Logger.h"
#include "../session/Session.h"
#include "../Utils.h"
#include <string.h>

namespace addon_music_spotify {

  using namespace std;

  ThumbStore::ThumbStore() {
    Utils::removeDir(Settings::getThumbPath());
    Utils::createDir(Settings::getThumbPath());
  }

  void ThumbStore::deInit() {
    delete m_instance;
    Utils::removeDir(Settings::getThumbPath());
  }

  ThumbStore::~ThumbStore() {
    for (thumbMap::iterator it = m_thumbs.begin(); it != m_thumbs.end(); ++it) {
      delete it->second;
    }
  }

  ThumbStore* ThumbStore::m_instance = 0;
  ThumbStore *ThumbStore::getInstance() {
    return m_instance ? m_instance : (m_instance = new ThumbStore);
  }

  SxThumb *ThumbStore::getThumb(const unsigned char* image) {
    //check if we got the thumb
    thumbMap::iterator it = m_thumbs.find(image);
    SxThumb *thumb;
    if (it == m_thumbs.end()) {
      //we need to create it
      //Logger::printOut("create thumb");
      sp_image* spImage = sp_image_create(Session::getInstance()->getSpSession(), (unsigned char*) image);

      if (!spImage) {
        Logger::printOut("no image");
        return NULL;
      }

      string path = Settings::getThumbPath();
      thumb = new SxThumb(spImage, path);
      m_thumbs.insert(thumbMap::value_type(image, thumb));
    } else {
      //Logger::printOut("loading thumb from store");
      thumb = it->second;
      thumb->addRef();
    }

    return thumb;
  }

  void ThumbStore::removeThumb(const unsigned char* image) {
    thumbMap::iterator it = m_thumbs.find(image);
    SxThumb *thumb;
    if (it != m_thumbs.end()) {
      thumb = it->second;
      if (thumb->getReferencesCount() <= 1) {
        m_thumbs.erase(image);
        delete thumb;
      } else
        thumb->rmRef();
    }
  }

  void ThumbStore::removeThumb(SxThumb* thumb) {
    removeThumb((const unsigned char*) thumb->m_image);
  }

} /* namespace addon_music_spotify */
