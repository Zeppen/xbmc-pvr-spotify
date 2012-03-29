#pragma once

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

#include "utils/StdString.h"
#include "tinyXML/tinyxml.h" // no use forwarding these, as this class is the main workhorse anyway,
                             // thus it simplifies the include patterns

class XMLUtils
{
public:
  static bool HasUTF8Declaration(const CStdString &strXML);
  static bool HasChild(const TiXmlNode* pRootNode, const char* strTag);

  static bool GetHex(const TiXmlNode* pRootNode, const char* strTag, uint32_t& dwHexValue);
  static bool GetUInt(const TiXmlNode* pRootNode, const char* strTag, uint32_t& dwUIntValue);
  static bool GetLong(const TiXmlNode* pRootNode, const char* strTag, long& lLongValue);
  static bool GetFloat(const TiXmlNode* pRootNode, const char* strTag, float& value);
  static bool GetDouble(const TiXmlNode* pRootNode, const char* strTag, double &value);
  static bool GetInt(const TiXmlNode* pRootNode, const char* strTag, int& iIntValue);
  static bool GetBoolean(const TiXmlNode* pRootNode, const char* strTag, bool& bBoolValue);
  static bool GetString(const TiXmlNode* pRootNode, const char* strTag, CStdString& strStringValue);
  /*! \brief Get multiple tags, concatenating the values together.
   Transforms
     <tag>value1</tag>
     <tag clear="true">value2</tag>
     ...
     <tag>valuen</tag>
   into value2<sep>...<sep>valuen, appending it to the value string. Note that <value1> is overwritten by the clear="true" tag.

   \param rootNode    the parent containing the <tag>'s.
   \param tag         the <tag> in question.
   \param separator   the separator to use when concatenating values.
   \param value [out] the resulting string. Remains untouched if no <tag> is available, else is appended (or cleared based on the clear parameter).
   \param clear       if true, clears the string prior to adding tags, if tags are available. Defaults to false.
   */
  static bool GetAdditiveString(const TiXmlNode* rootNode, const char* tag, const CStdString& separator, CStdString& value, bool clear = false);
  static bool GetEncoding(const TiXmlDocument* pDoc, CStdString& strEncoding);
  static bool GetPath(const TiXmlNode* pRootNode, const char* strTag, CStdString& strStringValue);
  static bool GetFloat(const TiXmlNode* pRootNode, const char* strTag, float& value, const float min, const float max);
  static bool GetInt(const TiXmlNode* pRootNode, const char* strTag, int& iIntValue, const int min, const int max);

  static void SetString(TiXmlNode* pRootNode, const char *strTag, const CStdString& strValue);
  static void SetAdditiveString(TiXmlNode* pRootNode, const char *strTag, const CStdString& strSeparator, const CStdString& strValue);
  static void SetInt(TiXmlNode* pRootNode, const char *strTag, int value);
  static void SetFloat(TiXmlNode* pRootNode, const char *strTag, float value);
  static void SetBoolean(TiXmlNode* pRootNode, const char *strTag, bool value);
  static void SetHex(TiXmlNode* pRootNode, const char *strTag, uint32_t value);
  static void SetPath(TiXmlNode* pRootNode, const char *strTag, const CStdString& strValue);
  static void SetLong(TiXmlNode* pRootNode, const char *strTag, long iValue);

  static const int path_version = 1;
};

