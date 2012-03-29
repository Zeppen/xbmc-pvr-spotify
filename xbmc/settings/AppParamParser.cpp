/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://xbmc.org
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

#include "AppParamParser.h"
#include "AdvancedSettings.h"
#include "GUIInfoManager.h"
#include "PlayListPlayer.h"
#include "FileItem.h"
#include "Application.h"
#include "utils/log.h"
#ifdef TARGET_WINDOWS
#include "WIN32Util.h"
#endif
#ifdef HAS_LIRC
#include "input/linux/LIRC.h"
#endif
#ifndef TARGET_WINDOWS
#include "linux/XTimeUtils.h"
#endif

CAppParamParser::CAppParamParser()
{
  m_testmode = false;
}

void CAppParamParser::Parse(const char* argv[], int nArgs)
{
  if (nArgs > 1)
  {
    for (int i = 1; i < nArgs; i++)
    {
      ParseArg(argv[i]);
#ifdef HAS_LIRC
      if (strnicmp(argv[i], "-l", 2) == 0 || strnicmp(argv[i], "--lircdev", 9) == 0)
      {
        // check the next arg with the proper value.
        int next=i+1;
        if (next < nArgs)
        {
          if ((argv[next][0] != '-' ) && (argv[next][0] == '/' ))
          {
            g_RemoteControl.setDeviceName(argv[next]);
            i++;
          }
        }
      }
      else if (strnicmp(argv[i], "-n", 2) == 0 || strnicmp(argv[i], "--nolirc", 8) == 0)
         g_RemoteControl.setUsed(false);
#endif
      if (stricmp(argv[i], "-d") == 0)
      {
        if (i + 1 < nArgs)
        {
          int sleeptime = atoi(argv[i + 1]);
          if (sleeptime > 0 && sleeptime < 360)
            Sleep(sleeptime*1000);
        }
        i++;
      }
    }
  }
  PlayPlaylist();
}

void CAppParamParser::DisplayVersion()
{
  printf("XBMC Media Center %s\n", g_infoManager.GetVersion().c_str());
  printf("Copyright (C) 2005-2011 Team XBMC - http://www.xbmc.org\n");
  exit(0);
}

void CAppParamParser::DisplayHelp()
{
  printf("Usage: xbmc [OPTION]... [FILE]...\n\n");
  printf("Arguments:\n");
  printf("  -d <n>\t\tdelay <n> seconds before starting\n");
  printf("  -fs\t\t\tRuns XBMC in full screen\n");
  printf("  --standalone\t\tXBMC runs in a stand alone environment without a window \n");
  printf("  -s or --server\t\tXBMC runs in a server mode i.e. without a gui \n");
  printf("\t\t\tmanager and supporting applications. For example, that\n");
  printf("\t\t\tenables network settings.\n");
  printf("  -p or --portable\tXBMC will look for configurations in install folder instead of ~/.xbmc\n");
  printf("  --legacy-res\t\tEnables screen resolutions such as PAL, NTSC, etc.\n");
#ifdef HAS_LIRC
  printf("  -l or --lircdev\tLircDevice to use default is "LIRC_DEVICE" .\n");
  printf("  -n or --nolirc\tdo not use Lirc, i.e. no remote input.\n");
#endif
  printf("  --debug\t\tEnable debug logging\n");
  printf("  --version\t\tPrint version information\n");
  printf("  --test\t\tEnable test mode. [FILE] required.\n");
  printf("  --settings=<filename>\t\tLoads specified file after advancedsettings.xml replacing any settings specified\n");
  printf("  \t\t\t\tspecified file must exist in special://xbmc/system/\n");
  exit(0);
}

void CAppParamParser::EnableDebugMode()
{
  g_advancedSettings.m_logLevel     = LOG_LEVEL_DEBUG;
  g_advancedSettings.m_logLevelHint = LOG_LEVEL_DEBUG;
  CLog::SetLogLevel(g_advancedSettings.m_logLevel);
}

void CAppParamParser::ParseArg(const CStdString &arg)
{
  if (arg == "-fs" || arg == "--fullscreen")
    g_advancedSettings.m_startFullScreen = true;
  else if (arg == "-h" || arg == "--help")
    DisplayHelp();
  else if (arg == "-v" || arg == "--version")
    DisplayVersion();
  else if (arg == "-s" || arg == "--server")
    g_application.SetServerMode(true);
  else if (arg == "--standalone")
    g_application.SetStandAlone(true);
  else if (arg == "-p" || arg  == "--portable")
    g_application.EnablePlatformDirectories(false);
  else if (arg == "--debug")
    EnableDebugMode();
  else if (arg == "--legacy-res")
    g_application.SetEnableLegacyRes(true);
  else if (arg == "--test")
    m_testmode = true;
  else if (arg.substr(0, 11) == "--settings=")
    g_advancedSettings.AddSettingsFile(arg.substr(11));
  else if (arg.length() != 0 && arg[0] != '-')
  {
    if (m_testmode)
      g_application.SetEnableTestMode(true);
    CFileItemPtr pItem(new CFileItem(arg));
    pItem->SetPath(arg);
    m_playlist.Add(pItem);
  }
}

void CAppParamParser::PlayPlaylist()
{
  if (m_playlist.Size() > 0)
  {
    g_playlistPlayer.Add(0, m_playlist);
    g_playlistPlayer.SetCurrentPlaylist(0);
  }

  ThreadMessage tMsg = {TMSG_PLAYLISTPLAYER_PLAY, (DWORD) -1};
  g_application.getApplicationMessenger().SendMessage(tMsg, false);
}
