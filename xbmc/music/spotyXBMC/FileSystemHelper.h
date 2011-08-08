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

#ifndef FILESYSTEMHELPER_H_
#define FILESYSTEMHELPER_H_

#include <string>

using namespace std;

class FileSystemHelper {
public:
  static void createDir(string path);
  static void removeDir(string path);
  static void removeFile(string file);

private:
  FileSystemHelper();
  virtual ~FileSystemHelper();
  static int path_is_directory (string path);
};

#endif /* FILESYSTEMHELPER_H_ */
