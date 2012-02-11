/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

//spotify
#include "../../music/spotyXBMC/Addon.music.spotify.h"
#include "DirectoryNodeAlbum.h"
#include "QueryParams.h"
#include "music/MusicDatabase.h"

using namespace XFILE::MUSICDATABASEDIRECTORY;

CDirectoryNodeAlbum::CDirectoryNodeAlbum(const CStdString& strName, CDirectoryNode* pParent) :
    CDirectoryNode(NODE_TYPE_ALBUM, strName, pParent) {

}

NODE_TYPE CDirectoryNodeAlbum::GetChildType() const {
  return NODE_TYPE_SONG;
}

CStdString CDirectoryNodeAlbum::GetLocalizedName() const {
  if (GetID() == -1)
    return g_localizeStrings.Get(15102); // All Albums
  CMusicDatabase db;
  if (db.Open())
    return db.GetAlbumById(GetID());
  return "";
}

bool CDirectoryNodeAlbum::GetContent(CFileItemList& items) const {
  CMusicDatabase musicdatabase;
  if (!musicdatabase.Open())
    return false;

  CQueryParams params;
  CollectQueryParams(params);
  CStdString strBaseDir = BuildPath();
  CURL url(strBaseDir);

  bool bSuccess = musicdatabase.GetAlbumsNav(BuildPath(), items, params.GetGenreId(), params.GetArtistId(), -1, -1);

  //spotify
  // TODO ask all loaded music addons for albums
  bSuccess = g_spotify->GetAlbums(items, strBaseDir,musicdatabase.GetArtistById(params.GetArtistId()));

  musicdatabase.Close();

  return bSuccess;
}
