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

#ifndef BUTTON_TRANSLATOR_H
#define BUTTON_TRANSLATOR_H

#pragma once

#include <map>
#include "system.h" // for HAS_EVENT_SERVER, HAS_SDL_JOYSTICK, HAS_LIRC

#ifdef HAS_EVENT_SERVER
#include "network/EventClient.h"
#endif

#if defined(HAS_SDL_JOYSTICK) || defined(HAS_EVENT_SERVER)
#include "SDLJoystick.h"
#endif

class CKey;
class CAction;
class TiXmlNode;

struct CButtonAction
{
  int id;
  CStdString strID; // needed for "XBMC.ActivateWindow()" type actions
};
///
/// singleton class to map from buttons to actions
/// Warning: _not_ threadsafe!
class CButtonTranslator
{
#ifdef HAS_EVENT_SERVER
  friend class EVENTCLIENT::CEventButtonState;
#endif

private:
  //private construction, and no assignements; use the provided singleton methods
  CButtonTranslator();
  CButtonTranslator(const CButtonTranslator&);
  CButtonTranslator const& operator=(CButtonTranslator const&);
  virtual ~CButtonTranslator();
  bool HasDeviceType(TiXmlNode *pWindow, CStdString type);
public:
  ///access to singleton
  static CButtonTranslator& GetInstance();

  // Add/remove a HID device with custom mappings
  void AddDevice(CStdString& strDevice);
  void RemoveDevice(CStdString& strDevice);

  /// loads Lircmap.xml/IRSSmap.xml (if enabled) and Keymap.xml
  bool Load(bool AlwaysLoad = false);
  /// clears the maps
  void Clear();

  CAction GetAction(int window, const CKey &key, bool fallback = true);

  /*! \brief Translate between a window name and it's id
   \param window name of the window
   \return id of the window, or WINDOW_INVALID if not found
   */
  static int TranslateWindow(const CStdString &window);

  /*! \brief Translate between a window id and it's name
   \param window id of the window
   \return name of the window, or an empty string if not found
   */
  static CStdString TranslateWindow(int window);

  static bool TranslateActionString(const char *szAction, int &action);

#if defined(HAS_LIRC) || defined(HAS_IRSERVERSUITE)
  int TranslateLircRemoteString(const char* szDevice, const char *szButton);
#endif
#if defined(HAS_SDL_JOYSTICK) || defined(HAS_EVENT_SERVER)
  bool TranslateJoystickString(int window, const char* szDevice, int id,
                               short inputType, int& action, CStdString& strAction,
                               bool &fullrange);
#endif

private:
  typedef std::multimap<uint32_t, CButtonAction> buttonMap; // our button map to fill in

  // m_translatorMap contains all mappings i.e. m_BaseMap + HID device mappings
  std::map<int, buttonMap> m_translatorMap;
  // m_deviceList contains the list of connected HID devices
  std::list<CStdString> m_deviceList;

  int GetActionCode(int window, const CKey &key, CStdString &strAction) const;

  static uint32_t TranslateGamepadString(const char *szButton);
  static uint32_t TranslateRemoteString(const char *szButton);
  static uint32_t TranslateUniversalRemoteString(const char *szButton);

  static uint32_t TranslateKeyboardString(const char *szButton);
  static uint32_t TranslateKeyboardButton(TiXmlElement *pButton);

  static uint32_t TranslateMouseCommand(const char *szButton);

  static uint32_t TranslateAppCommand(const char *szButton);

  void MapWindowActions(TiXmlNode *pWindow, int wWindowID);
  void MapAction(uint32_t buttonCode, const char *szAction, buttonMap &map);

  bool LoadKeymap(const CStdString &keymapPath);
#if defined(HAS_LIRC) || defined(HAS_IRSERVERSUITE)
  bool LoadLircMap(const CStdString &lircmapPath);
  void MapRemote(TiXmlNode *pRemote, const char* szDevice);
  typedef std::map<CStdString, CStdString> lircButtonMap;
  std::map<CStdString, lircButtonMap*> lircRemotesMap;
#endif

#if defined(HAS_SDL_JOYSTICK) || defined(HAS_EVENT_SERVER)
  void MapJoystickActions(int windowID, TiXmlNode *pJoystick);

  typedef std::map<int, std::map<int, std::string> > JoystickMap; // <window, <button/axis, action> >
  std::map<std::string, JoystickMap> m_joystickButtonMap;      // <joy name, button map>
  std::map<std::string, JoystickMap> m_joystickAxisMap;        // <joy name, axis map>
  std::map<std::string, JoystickMap> m_joystickHatMap;        // <joy name, hat map>
#endif

  bool m_Loaded;
};

#endif

