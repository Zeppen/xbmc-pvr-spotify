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

#include "Logger.h"
#include "FileSystemHelper.h"
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace addon_music_spotify;

FileSystemHelper::FileSystemHelper() {
  // TODO Auto-generated constructor stub

}

FileSystemHelper::~FileSystemHelper() {
  // TODO Auto-generated destructor stub
}

void FileSystemHelper::createDir(string path) {

  mkdir(path.c_str(), 0777);

}

void FileSystemHelper::removeDir(string path) {
  /*DIR* dp;
   struct dirent*  ep;
   char            p_buf[512] = {0};

   dp = opendir(path);

   while ((ep = readdir(dp)) != NULL) {
   sprintf(p_buf, "%s/%s", path, ep->d_name);
   if (path_is_directory(p_buf))
   removeDir(p_buf);
   else
   unlink(p_buf);
   }

   closedir(dp);*/
  rmdir(path.c_str());
}

int FileSystemHelper::path_is_directory(string path) {
  /*struct stat s_buf;

   if (stat(path, &s_buf))
   return 0;

   return S_ISDIR(s_buf.st_mode);*/
}

void FileSystemHelper::removeFile(string file) {
  remove(file.c_str());
}

