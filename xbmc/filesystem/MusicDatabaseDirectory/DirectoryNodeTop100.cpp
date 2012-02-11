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
#include "DirectoryNodeTop100.h"
#include "FileItem.h"
#include "guilib/LocalizeStrings.h"

using namespace std;
using namespace XFILE::MUSICDATABASEDIRECTORY;

Node Top100Children[] = {
                          { NODE_TYPE_SONG_TOP100,  1, 10504 },
                          { NODE_TYPE_ALBUM_TOP100, 2, 10505 },
                        };

CDirectoryNodeTop100::CDirectoryNodeTop100(const CStdString& strName, CDirectoryNode* pParent)
  : CDirectoryNode(NODE_TYPE_TOP100, strName, pParent)
{

}

NODE_TYPE CDirectoryNodeTop100::GetChildType() const
{
  for (unsigned int i = 0; i < sizeof(Top100Children) / sizeof(Node); ++i)
    if (GetID() == Top100Children[i].id)
      return Top100Children[i].node;

  return NODE_TYPE_NONE;
}

CStdString CDirectoryNodeTop100::GetLocalizedName() const
{
  for (unsigned int i = 0; i < sizeof(Top100Children) / sizeof(Node); ++i)
    if (GetID() == Top100Children[i].id)
      return g_localizeStrings.Get(Top100Children[i].label);
  return "";
}

bool CDirectoryNodeTop100::GetContent(CFileItemList& items) const
{
  for (unsigned int i = 0; i < sizeof(Top100Children) / sizeof(Node); ++i)
  {
    CFileItemPtr pItem(new CFileItem(g_localizeStrings.Get(Top100Children[i].label)));
    CStdString strDir;
    strDir.Format("%ld/", Top100Children[i].id);
    pItem->SetPath(BuildPath() + strDir);
    pItem->m_bIsFolder = true;
    items.Add(pItem);
  }

  //spotify
  //get all toplists from addons, now just fetch the spotify lists
  g_spotify->GetTopLists(items);

  return true;
}
