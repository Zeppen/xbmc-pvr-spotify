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
#include "Scraper.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "filesystem/FileCurl.h"
#include "AddonManager.h"
#include "utils/ScraperParser.h"
#include "utils/ScraperUrl.h"
#include "utils/CharsetConverter.h"
#include "utils/log.h"
#include "music/infoscanner/MusicAlbumInfo.h"
#include "music/infoscanner/MusicArtistInfo.h"
#include "utils/fstrcmp.h"
#include "settings/AdvancedSettings.h"
#include "FileItem.h"
#include "utils/URIUtils.h"
#include "utils/XMLUtils.h"
#include "music/MusicDatabase.h"
#include "video/VideoDatabase.h"
#include "music/Album.h"
#include "music/Artist.h"
#include <sstream>

using namespace std;
using namespace XFILE;
using namespace MUSIC_GRABBER;

namespace ADDON
{

typedef struct
{
  const char*  name;
  CONTENT_TYPE type;
  int          pretty;
} ContentMapping;

static const ContentMapping content[] =
  {{"unknown",       CONTENT_NONE,          231 },
   {"albums",        CONTENT_ALBUMS,        132 },
   {"music",         CONTENT_ALBUMS,        132 },
   {"artists",       CONTENT_ARTISTS,       133 },
   {"movies",        CONTENT_MOVIES,      20342 },
   {"tvshows",       CONTENT_TVSHOWS,     20343 },
   {"musicvideos",   CONTENT_MUSICVIDEOS, 20389 }};

CStdString TranslateContent(const CONTENT_TYPE &type, bool pretty/*=false*/)
{
  for (unsigned int index=0; index < sizeof(content)/sizeof(content[0]); ++index)
  {
    const ContentMapping &map = content[index];
    if (type == map.type)
    {
      if (pretty && map.pretty)
        return g_localizeStrings.Get(map.pretty);
      else
        return map.name;
    }
  }
  return "";
}

CONTENT_TYPE TranslateContent(const CStdString &string)
{
  for (unsigned int index=0; index < sizeof(content)/sizeof(content[0]); ++index)
  {
    const ContentMapping &map = content[index];
    if (string.Equals(map.name))
      return map.type;
  }
  return CONTENT_NONE;
}

TYPE ScraperTypeFromContent(const CONTENT_TYPE &content)
{
  switch (content)
  {
  case CONTENT_ALBUMS:
    return ADDON_SCRAPER_ALBUMS;
  case CONTENT_ARTISTS:
    return ADDON_SCRAPER_ARTISTS;
  case CONTENT_MOVIES:
    return ADDON_SCRAPER_MOVIES;
  case CONTENT_MUSICVIDEOS:
    return ADDON_SCRAPER_MUSICVIDEOS;
  case CONTENT_TVSHOWS:
    return ADDON_SCRAPER_TVSHOWS;
  default:
    return ADDON_UNKNOWN;
  }
}

// if the XML root is <error>, throw CScraperError with enclosed <title>/<message> values
static void CheckScraperError(const TiXmlElement *pxeRoot)
{
  if (!pxeRoot || stricmp(pxeRoot->Value(), "error"))
    return;
  CStdString sTitle;
  CStdString sMessage;
  XMLUtils::GetString(pxeRoot, "title", sTitle);
  XMLUtils::GetString(pxeRoot, "message", sMessage);
  throw CScraperError(sTitle, sMessage);
}

CScraper::CScraper(const cp_extension_t *ext) : CAddon(ext), m_fLoaded(false)
{
  if (ext)
  {
    m_language = CAddonMgr::Get().GetExtValue(ext->configuration, "@language");
    m_requiressettings = CAddonMgr::Get().GetExtValue(ext->configuration,"@requiressettings").Equals("true");
    CStdString persistence = CAddonMgr::Get().GetExtValue(ext->configuration, "@cachepersistence");
    if (!persistence.IsEmpty())
      m_persistence.SetFromTimeString(persistence);
  }
  switch (Type())
  {
    case ADDON_SCRAPER_ALBUMS:
      m_pathContent = CONTENT_ALBUMS;
      break;
    case ADDON_SCRAPER_ARTISTS:
      m_pathContent = CONTENT_ARTISTS;
      break;
    case ADDON_SCRAPER_MOVIES:
      m_pathContent = CONTENT_MOVIES;
      break;
    case ADDON_SCRAPER_MUSICVIDEOS:
      m_pathContent = CONTENT_MUSICVIDEOS;
      break;
    case ADDON_SCRAPER_TVSHOWS:
      m_pathContent = CONTENT_TVSHOWS;
      break;
    default:
      m_pathContent = CONTENT_NONE;
      break;
  }
}

AddonPtr CScraper::Clone(const AddonPtr &self) const
{
  return AddonPtr(new CScraper(*this, self));
}

CScraper::CScraper(const CScraper &rhs, const AddonPtr &self)
  : CAddon(rhs, self), m_fLoaded(false)
{
  m_pathContent = rhs.m_pathContent;
  m_persistence = rhs.m_persistence;
  m_requiressettings = rhs.m_requiressettings;
  m_language = rhs.m_language;
}

bool CScraper::Supports(const CONTENT_TYPE &content) const
{
  return Type() == ScraperTypeFromContent(content);
}

bool CScraper::SetPathSettings(CONTENT_TYPE content, const CStdString& xml)
{
  m_pathContent = content;
  if (!LoadSettings())
    return false;

  if (xml.IsEmpty())
    return true;

  TiXmlDocument doc;
  doc.Parse(xml.c_str());
  m_userSettingsLoaded = SettingsFromXML(doc);

  return m_userSettingsLoaded;
}

CStdString CScraper::GetPathSettings()
{
  if (!LoadSettings())
    return "";

  stringstream stream;
  TiXmlDocument doc;
  SettingsToXML(doc);
  if (doc.RootElement())
    stream << *doc.RootElement();

  return stream.str();
}

void CScraper::ClearCache()
{
  CStdString strCachePath = URIUtils::AddFileToFolder(g_advancedSettings.m_cachePath, "scrapers");

  // create scraper cache dir if needed
  if (!CDirectory::Exists(strCachePath))
    CDirectory::Create(strCachePath);

  strCachePath = URIUtils::AddFileToFolder(strCachePath, ID());
  URIUtils::AddSlashAtEnd(strCachePath);

  if (CDirectory::Exists(strCachePath))
  {
    CFileItemList items;
    CDirectory::GetDirectory(strCachePath,items);
    for (int i=0;i<items.Size();++i)
    {
      // wipe cache
      if (items[i]->m_dateTime + m_persistence <= CDateTime::GetCurrentDateTime())
        CFile::Delete(items[i]->GetPath());
    }
  }
  else
    CDirectory::Create(strCachePath);
}

// returns a vector of strings: the first is the XML output by the function; the rest
// is XML output by chained functions, possibly recursively
// the CFileCurl object is passed in so that URL fetches can be canceled from other threads
// throws CScraperError abort on internal failures (e.g., parse errors)
vector<CStdString> CScraper::Run(const CStdString& function,
                                 const CScraperUrl& scrURL,
                                 CFileCurl& http,
                                 const vector<CStdString>* extras)
{
  if (!Load())
    throw CScraperError();

  CStdString strXML = InternalRun(function,scrURL,http,extras);
  if (strXML.IsEmpty())
  {
    if (function != "NfoUrl")
      CLog::Log(LOGERROR, "%s: Unable to parse web site",__FUNCTION__);
    throw CScraperError();
  }

  CLog::Log(LOGDEBUG,"scraper: %s returned %s",function.c_str(),strXML.c_str());

  if (!XMLUtils::HasUTF8Declaration(strXML))
    g_charsetConverter.unknownToUTF8(strXML);

  TiXmlDocument doc;
  doc.Parse(strXML.c_str(),0,TIXML_ENCODING_UTF8);
  if (!doc.RootElement())
  {
    CLog::Log(LOGERROR, "%s: Unable to parse XML",__FUNCTION__);
    throw CScraperError();
  }

  vector<CStdString> result;
  result.push_back(strXML);
  TiXmlElement* xchain = doc.RootElement()->FirstChildElement();
  // skip children of the root element until <url> or <chain>
  while (xchain && strcmp(xchain->Value(),"url") && strcmp(xchain->Value(),"chain"))
      xchain = xchain->NextSiblingElement();
  while (xchain)
  {
    // <chain|url function="...">param</>
    const char* szFunction = xchain->Attribute("function");
    if (szFunction)
    {
      CScraperUrl scrURL2;
      vector<CStdString> extras;
      // for <chain>, pass the contained text as a parameter; for <url>, as URL content
      if (strcmp(xchain->Value(),"chain")==0)
      {
        if (xchain->FirstChild())
          extras.push_back(xchain->FirstChild()->Value());
      }
      else
        scrURL2.ParseElement(xchain);
      vector<CStdString> result2 = RunNoThrow(szFunction,scrURL2,http,&extras);
      result.insert(result.end(),result2.begin(),result2.end());
    }
    xchain = xchain->NextSiblingElement();
    // continue to skip past non-<url> or <chain> elements
    while (xchain && strcmp(xchain->Value(),"url") && strcmp(xchain->Value(),"chain"))
      xchain = xchain->NextSiblingElement();
  }
  
  return result;
}

// just like Run, but returns an empty list instead of throwing in case of error
// don't use in new code; errors should be handled appropriately
std::vector<CStdString> CScraper::RunNoThrow(const CStdString& function,
  const CScraperUrl& url,
  XFILE::CFileCurl& http,
  const std::vector<CStdString>* extras)
{
  std::vector<CStdString> vcs;
  try
  {
    vcs = Run(function, url, http, extras);
  }
  catch (const CScraperError &sce)
  {
    ASSERT(sce.FAborted());  // the only kind we should get
  }
  return vcs;
}

CStdString CScraper::InternalRun(const CStdString& function,
                                 const CScraperUrl& scrURL,
                                 CFileCurl& http,
                                 const vector<CStdString>* extras)
{
  // walk the list of input URLs and fetch each into parser parameters
  unsigned int i;
  for (i=0;i<scrURL.m_url.size();++i)
  {
    CStdString strCurrHTML;
    if (!CScraperUrl::Get(scrURL.m_url[i],m_parser.m_param[i],http,ID()) || m_parser.m_param[i].size() == 0)
      return "";
  }
  // put the 'extra' parameterts into the parser parameter list too
  if (extras)
  {
    for (unsigned int j=0;j<extras->size();++j)
      m_parser.m_param[j+i] = (*extras)[j];
  }

  return m_parser.Parse(function,this);
}

bool CScraper::Load()
{
  if (m_fLoaded)
    return true;

  bool result=m_parser.Load(LibPath());
  if (result)
  {
    // TODO: this routine assumes that deps are a single level, and assumes the dep is installed.
    //       1. Does it make sense to have recursive dependencies?
    //       2. Should we be checking the dep versions or do we assume it is ok?
    ADDONDEPS deps = GetDeps();
    ADDONDEPS::iterator itr = deps.begin();
    while (itr != deps.end())
    {
      if (itr->first.Equals("xbmc.metadata"))
      {
        ++itr;
        continue;
      }  
      AddonPtr dep;

      bool bOptional = itr->second.second;

      if (CAddonMgr::Get().GetAddon((*itr).first, dep))
      {
        TiXmlDocument doc;
        if (dep->Type() == ADDON_SCRAPER_LIBRARY && doc.LoadFile(dep->LibPath()))
          m_parser.AddDocument(&doc);
      }
      else
      {
        if (!bOptional)
        {
          result = false;
          break;
        }
      }
      itr++;
    }
  }

  if (!result)
    CLog::Log(LOGWARNING, "failed to load scraper XML");
  return m_fLoaded = result;
}

bool CScraper::IsInUse() const
{
  if (Supports(CONTENT_ALBUMS) || Supports(CONTENT_ARTISTS))
  { // music scraper
    CMusicDatabase db;
    if (db.Open() && db.ScraperInUse(ID()))
      return true;
  }
  else
  { // video scraper
    CVideoDatabase db;
    if (db.Open() && db.ScraperInUse(ID()))
      return true;
  }
  return false;
}

// pass in contents of .nfo file; returns URL (possibly empty if none found)
// and may populate strId, or throws CScraperError on error
CScraperUrl CScraper::NfoUrl(const CStdString &sNfoContent)
{
  CScraperUrl scurlRet;

  // scraper function takes contents of .nfo file, returns XML (see below)
  vector<CStdString> vcsIn;
  vcsIn.push_back(sNfoContent);
  CScraperUrl scurl;
  CFileCurl fcurl;
  vector<CStdString> vcsOut = Run("NfoUrl", scurl, fcurl, &vcsIn);
  if (vcsOut.empty() || vcsOut[0].empty())
    return scurlRet;
  if (vcsOut.size() > 1)
    CLog::Log(LOGWARNING, "%s: scraper returned multiple results; using first", __FUNCTION__);

  // parse returned XML: either <error> element on error, blank on failure,
  // or <url>...</url> or <url>...</url><id>...</id> on success
  for (unsigned int i=0; i < vcsOut.size(); ++i)
  {
    TiXmlDocument doc;
    doc.Parse(vcsOut[i], 0, TIXML_ENCODING_UTF8);
    CheckScraperError(doc.RootElement());

    if (doc.RootElement())
    {
      /*
       NOTE: Scrapers might return invalid xml with some loose
       elements (eg. '<url>http://some.url</url><id>123</id>').
       Since XMLUtils::GetString() is assuming well formed xml
       with start and end-tags we're not able to use it.
       Check for the desired Elements instead.
      */
      TiXmlElement* pxeUrl=NULL;
      TiXmlElement* pId=NULL;
      if (!strcmp(doc.RootElement()->Value(),"details"))
      {
        pxeUrl = doc.RootElement()->FirstChildElement("url");
        pId = doc.RootElement()->FirstChildElement("id");
      }
      else
      {
        pId = doc.FirstChildElement("id");
        pxeUrl = doc.FirstChildElement("url");
      }
      if (pId && pId->FirstChild())
        scurlRet.strId = pId->FirstChild()->Value();

      if (pxeUrl && pxeUrl->Attribute("function"))
        continue;

      if (pxeUrl)
        scurlRet.ParseElement(pxeUrl);
      else if (!strcmp(doc.RootElement()->Value(), "url"))
        scurlRet.ParseElement(doc.RootElement());
      else
        continue;
      break;
    }
  }
  return scurlRet;
}

static bool RelevanceSortFunction(const CScraperUrl &left, const CScraperUrl &right)
{
  return left.relevance > right.relevance;
}

// fetch list of matching movies sorted by relevance (may be empty);
// throws CScraperError on error; first called with fFirst set, then unset if first try fails
std::vector<CScraperUrl> CScraper::FindMovie(XFILE::CFileCurl &fcurl, const CStdString &sMovie,
  bool fFirst)
{
  // prepare parameters for URL creation
  CStdString sTitle, sTitleYear, sYear;
  CUtil::CleanString(sMovie, sTitle, sTitleYear, sYear, true/*fRemoveExt*/, fFirst);

  if (!fFirst || Content() == CONTENT_MUSICVIDEOS)
    sTitle.Replace("-"," ");

  CLog::Log(LOGDEBUG, "%s: Searching for '%s' using %s scraper "
    "(path: '%s', content: '%s', version: '%s')", __FUNCTION__, sTitle.c_str(),
    Name().c_str(), Path().c_str(),
    ADDON::TranslateContent(Content()).c_str(), Version().c_str());

  sTitle.ToLower();

  vector<CStdString> vcsIn(1);
  g_charsetConverter.utf8To(SearchStringEncoding(), sTitle, vcsIn[0]);
  CURL::Encode(vcsIn[0]);
  if (!sYear.IsEmpty())
    vcsIn.push_back(sYear);

  // request a search URL from the title/filename/etc.
  CScraperUrl scurl;
  vector<CStdString> vcsOut = Run("CreateSearchUrl", scurl, fcurl, &vcsIn);
  std::vector<CScraperUrl> vcscurl;
  if (vcsOut.empty())
  {
    CLog::Log(LOGDEBUG, "%s: CreateSearchUrl failed", __FUNCTION__);
    throw CScraperError();
  }
  scurl.ParseString(vcsOut[0]);

  // do the search, and parse the result into a list
  vcsIn.clear();
  vcsIn.push_back(scurl.m_url[0].m_url);
  vcsOut = Run("GetSearchResults", scurl, fcurl, &vcsIn);

  bool fSort(true);
  std::set<CStdString> stsDupeCheck;
  bool fResults(false);
  for (CStdStringArray::const_iterator i = vcsOut.begin(); i != vcsOut.end(); ++i)
  {
    TiXmlDocument doc;
    doc.Parse(*i, 0, TIXML_ENCODING_UTF8);
    if (!doc.RootElement())
    {
      CLog::Log(LOGERROR, "%s: Unable to parse XML", __FUNCTION__);
      continue;  // might have more valid results later
    }

    CheckScraperError(doc.RootElement());

    TiXmlHandle xhDoc(&doc);
    TiXmlHandle xhResults = xhDoc.FirstChild("results");
    if (!xhResults.Element())
      continue;
    fResults = true;  // even if empty

    // we need to sort if returned results don't specify 'sorted="yes"'
    if (fSort)
      fSort = CStdString(xhResults.Element()->Attribute("sorted")).CompareNoCase("yes") != 0;

    for (TiXmlElement *pxeMovie = xhResults.FirstChild("entity").Element();
      pxeMovie; pxeMovie = pxeMovie->NextSiblingElement())
    {
      CScraperUrl scurlMovie;
      TiXmlNode *pxnTitle = pxeMovie->FirstChild("title");
      TiXmlElement *pxeLink = pxeMovie->FirstChildElement("url");
      if (pxnTitle && pxnTitle->FirstChild() && pxeLink && pxeLink->FirstChild())
      {
        scurlMovie.strTitle = pxnTitle->FirstChild()->Value();
        XMLUtils::GetString(pxeMovie, "id", scurlMovie.strId);

        for ( ; pxeLink && pxeLink->FirstChild(); pxeLink = pxeLink->NextSiblingElement("url"))
          scurlMovie.ParseElement(pxeLink);

        // calculate the relavance of this hit
        CStdString sCompareTitle = scurlMovie.strTitle;
        sCompareTitle.ToLower();
        CStdString sMatchTitle = sTitle;
        sMatchTitle.ToLower();

        /*
         * Identify the best match by performing a fuzzy string compare on the search term and
         * the result. Additionally, use the year (if available) to further refine the best match.
         * An exact match scores 1, a match off by a year scores 0.5 (release dates can vary between
         * countries), otherwise it scores 0.
         */
        CStdString sCompareYear;
        XMLUtils::GetString(pxeMovie, "year", sCompareYear);

        double yearScore = 0;
        if (!sYear.empty() && !sCompareYear.empty())
          yearScore = std::max(0.0, 1-0.5*abs(atoi(sYear)-atoi(sCompareYear)));

        scurlMovie.relevance = fstrcmp(sMatchTitle.c_str(), sCompareTitle.c_str(), 0.0) + yearScore;

        // reconstruct a title for the user
        if (!sCompareYear.empty())
          scurlMovie.strTitle.AppendFormat(" (%s)", sCompareYear.c_str());

        CStdString sLanguage;
        if (XMLUtils::GetString(pxeMovie, "language", sLanguage))
          scurlMovie.strTitle.AppendFormat(" (%s)", sLanguage.c_str());

        // filter for dupes from naughty scrapers
        if (stsDupeCheck.insert(scurlMovie.m_url[0].m_url + " " + scurlMovie.strTitle).second)
          vcscurl.push_back(scurlMovie);
      }
    }
  }

  if (!fResults)
    throw CScraperError();  // scraper aborted

  if (fSort)
    std::stable_sort(vcscurl.begin(), vcscurl.end(), RelevanceSortFunction);

  return vcscurl;
}

// find album by artist, using fcurl for web fetches
// returns a list of albums (empty if no match or failure)
std::vector<CMusicAlbumInfo> CScraper::FindAlbum(CFileCurl &fcurl, const CStdString &sAlbum,
  const CStdString &sArtist)
{
  CLog::Log(LOGDEBUG, "%s: Searching for '%s - %s' using %s scraper "
    "(path: '%s', content: '%s', version: '%s')", __FUNCTION__, sArtist.c_str(),
    sAlbum.c_str(), Name().c_str(), Path().c_str(),
    ADDON::TranslateContent(Content()).c_str(), Version().c_str());

  // scraper function is given the album and artist as parameters and
  // returns an XML <url> element parseable by CScraperUrl
  std::vector<CStdString> extras(2);
  g_charsetConverter.utf8To(SearchStringEncoding(), sAlbum, extras[0]);
  g_charsetConverter.utf8To(SearchStringEncoding(), sArtist, extras[1]);
  CURL::Encode(extras[0]);
  CURL::Encode(extras[1]);
  CScraperUrl scurl;
  vector<CStdString> vcsOut = RunNoThrow("CreateAlbumSearchUrl", scurl, fcurl, &extras);
  if (vcsOut.size() > 1)
    CLog::Log(LOGWARNING, "%s: scraper returned multiple results; using first", __FUNCTION__);

  std::vector<CMusicAlbumInfo> vcali;
  if (vcsOut.empty() || vcsOut[0].empty())
    return vcali;
  scurl.ParseString(vcsOut[0]);

  // the next function is passed the contents of the returned URL, and returns
  // an empty string on failure; on success, returns XML matches in the form:
  // <results>
  //  <entity>
  //   <title>...</title>
  //   <url>...</url> (with the usual CScraperUrl decorations like post or spoof)
  //   <artist>...</artist>
  //   <year>...</year>
  //   <relevance [scale="..."]>...</relevance> (scale defaults to 1; score is divided by it)
  //  </entity>
  //  ...
  // </results>
  vcsOut = RunNoThrow("GetAlbumSearchResults", scurl, fcurl);

  // parse the returned XML into a vector of album objects
  for (CStdStringArray::const_iterator i = vcsOut.begin(); i != vcsOut.end(); ++i)
  {
    TiXmlDocument doc;
    doc.Parse(*i, 0, TIXML_ENCODING_UTF8);
    TiXmlHandle xhDoc(&doc);

    for (TiXmlElement* pxeAlbum = xhDoc.FirstChild("results").FirstChild("entity").Element();
      pxeAlbum; pxeAlbum = pxeAlbum->NextSiblingElement())
    {
      CStdString sTitle;
      if (XMLUtils::GetString(pxeAlbum, "title", sTitle))
      {
        CStdString sArtist;
        CStdString sAlbumName;
        if (XMLUtils::GetString(pxeAlbum, "artist", sArtist))
          sAlbumName.Format("%s - %s", sArtist.c_str(), sTitle.c_str());
        else
          sAlbumName = sTitle;

        CStdString sYear;
        if (XMLUtils::GetString(pxeAlbum, "year", sYear))
          sAlbumName.Format("%s (%s)", sAlbumName.c_str(), sYear.c_str());

        // if no URL is provided, use the URL we got back from CreateAlbumSearchUrl
        // (e.g., in case we only got one result back and were sent to the detail page)
        TiXmlElement* pxeLink = pxeAlbum->FirstChildElement("url");
        CScraperUrl scurlAlbum;
        if (!pxeLink)
          scurlAlbum.ParseString(scurl.m_xml);
        for ( ; pxeLink && pxeLink->FirstChild(); pxeLink = pxeLink->NextSiblingElement("url"))
          scurlAlbum.ParseElement(pxeLink);

        if (!scurlAlbum.m_url.size())
          continue;

        CMusicAlbumInfo ali(sTitle, sArtist, sAlbumName, scurlAlbum);

        TiXmlElement* pxeRel = pxeAlbum->FirstChildElement("relevance");
        if (pxeRel && pxeRel->FirstChild())
        {
          const char* szScale = pxeRel->Attribute("scale");
          float flScale = szScale ? float(atof(szScale)) : 1;
          ali.SetRelevance(float(atof(pxeRel->FirstChild()->Value())) / flScale);
        }

        vcali.push_back(ali);
      }
    }
  }
  return vcali;
}

// find artist, using fcurl for web fetches
// returns a list of artists (empty if no match or failure)
std::vector<CMusicArtistInfo> CScraper::FindArtist(CFileCurl &fcurl,
  const CStdString &sArtist)
{
  CLog::Log(LOGDEBUG, "%s: Searching for '%s' using %s scraper "
    "(file: '%s', content: '%s', version: '%s')", __FUNCTION__, sArtist.c_str(),
    Name().c_str(), Path().c_str(),
    ADDON::TranslateContent(Content()).c_str(), Version().c_str());

  // scraper function is given the artist as parameter and
  // returns an XML <url> element parseable by CScraperUrl
  std::vector<CStdString> extras(1);
  g_charsetConverter.utf8To(SearchStringEncoding(), sArtist, extras[0]);
  CURL::Encode(extras[0]);
  CScraperUrl scurl;
  vector<CStdString> vcsOut = RunNoThrow("CreateArtistSearchUrl", scurl, fcurl, &extras);

  std::vector<CMusicArtistInfo> vcari;
  if (vcsOut.empty() || vcsOut[0].empty())
    return vcari;
  scurl.ParseString(vcsOut[0]);

  // the next function is passed the contents of the returned URL, and returns
  // an empty string on failure; on success, returns XML matches in the form:
  // <results>
  //  <entity>
  //   <title>...</title>
  //   <year>...</year>
  //   <genre>...</genre>
  //   <url>...</url> (with the usual CScraperUrl decorations like post or spoof)
  //  </entity>
  //  ...
  // </results>
  vcsOut = RunNoThrow("GetArtistSearchResults", scurl, fcurl);

  // parse the returned XML into a vector of artist objects
  for (CStdStringArray::const_iterator i = vcsOut.begin(); i != vcsOut.end(); ++i)
  {
    TiXmlDocument doc;
    doc.Parse(*i, 0, TIXML_ENCODING_UTF8);
    if (!doc.RootElement())
    {
      CLog::Log(LOGERROR, "%s: Unable to parse XML", __FUNCTION__);
      return vcari;
    }
    TiXmlHandle xhDoc(&doc);
    for (TiXmlElement* pxeArtist = xhDoc.FirstChild("results").FirstChild("entity").Element();
      pxeArtist; pxeArtist = pxeArtist->NextSiblingElement())
    {
      TiXmlNode* pxnTitle = pxeArtist->FirstChild("title");
      if (pxnTitle && pxnTitle->FirstChild())
      {
        CScraperUrl scurlArtist;

        TiXmlElement* pxeLink = pxeArtist->FirstChildElement("url");
        if (!pxeLink)
          scurlArtist.ParseString(scurl.m_xml);
        for ( ; pxeLink && pxeLink->FirstChild(); pxeLink = pxeLink->NextSiblingElement("url"))
          scurlArtist.ParseElement(pxeLink);

        if (!scurlArtist.m_url.size())
          continue;

        CMusicArtistInfo ari(pxnTitle->FirstChild()->Value(), scurlArtist);
        XMLUtils::GetString(pxeArtist, "genre", ari.GetArtist().strGenre);
        XMLUtils::GetString(pxeArtist, "year", ari.GetArtist().strBorn);

        vcari.push_back(ari);
      }
    }
  }
  return vcari;
}

// fetch list of episodes from URL (from video database)
EPISODELIST CScraper::GetEpisodeList(XFILE::CFileCurl &fcurl, const CScraperUrl &scurl)
{
  CLog::Log(LOGDEBUG, "%s: Searching '%s' using %s scraper "
    "(file: '%s', content: '%s', version: '%s')", __FUNCTION__,
    scurl.m_url[0].m_url.c_str(), Name().c_str(), Path().c_str(),
    ADDON::TranslateContent(Content()).c_str(), Version().c_str());

  EPISODELIST vcep;
  if (scurl.m_url.empty())
    return vcep;

  vector<CStdString> vcsIn;
  vcsIn.push_back(scurl.m_url[0].m_url);
  vector<CStdString> vcsOut = RunNoThrow("GetEpisodeList", scurl, fcurl, &vcsIn);

  // parse the XML response
  for (CStdStringArray::const_iterator i = vcsOut.begin(); i != vcsOut.end(); ++i)
  {
    TiXmlDocument doc;
    doc.Parse(*i);
    if (!doc.RootElement())
    {
      CLog::Log(LOGERROR, "%s: Unable to parse XML",__FUNCTION__);
      continue;
    }

    TiXmlHandle xhDoc(&doc);
    for (TiXmlElement *pxeMovie = xhDoc.FirstChild("episodeguide").FirstChild("episode").
      Element(); pxeMovie; pxeMovie = pxeMovie->NextSiblingElement())
    {
      EPISODE ep;
      TiXmlElement *pxeLink = pxeMovie->FirstChildElement("url");
      if (pxeLink && XMLUtils::GetInt(pxeMovie, "season", ep.key.first) &&
        XMLUtils::GetInt(pxeMovie, "epnum", ep.key.second))
      {
        CScraperUrl &scurlEp(ep.cScraperUrl);
        if (!XMLUtils::GetString(pxeMovie, "title", scurlEp.strTitle))
            scurlEp.strTitle = g_localizeStrings.Get(416);
        XMLUtils::GetString(pxeMovie, "id", scurlEp.strId);

        for ( ; pxeLink && pxeLink->FirstChild(); pxeLink = pxeLink->NextSiblingElement("url"))
          scurlEp.ParseElement(pxeLink);

        // date must be the format of yyyy-mm-dd
        ep.cDate.SetValid(FALSE);
        CStdString sDate;
        if (XMLUtils::GetString(pxeMovie, "aired", sDate) && sDate.length() == 10)
        {
          tm tm;
          if (strptime(sDate, "%Y-%m-%d", &tm))
            ep.cDate.SetDate(1900+tm.tm_year, tm.tm_mon + 1, tm.tm_mday);
        }
        vcep.push_back(ep);
      }
    }
  }

  // find minimum in each season
  map<int, int> mpMin;
  for (EPISODELIST::const_iterator i = vcep.begin(); i != vcep.end(); ++i)
  {
    map<int, int>::iterator iMin = mpMin.find(i->key.first);
    if (iMin == mpMin.end())
      mpMin.insert(i->key);
    else if (i->key.second < iMin->second)
      iMin->second = i->key.second;
  }

  // correct episode numbers
  for (EPISODELIST::iterator i = vcep.begin(); i != vcep.end(); ++i)
  {
    i->key.second -= mpMin[i->key.first];
    if (mpMin[i->key.first] > 0)
      ++i->key.second;
  }

  return vcep;
}

// takes URL; returns true and populates video details on success, false otherwise
bool CScraper::GetVideoDetails(XFILE::CFileCurl &fcurl, const CScraperUrl &scurl,
  bool fMovie/*else episode*/, CVideoInfoTag &video)
{
  CLog::Log(LOGDEBUG, "%s: Reading %s '%s' using %s scraper "
    "(file: '%s', content: '%s', version: '%s')", __FUNCTION__,
    fMovie ? "movie" : "episode", scurl.m_url[0].m_url.c_str(), Name().c_str(), Path().c_str(),
    ADDON::TranslateContent(Content()).c_str(), Version().c_str());

  video.Reset();
  CStdString sFunc = fMovie ? "GetDetails" : "GetEpisodeDetails";
  vector<CStdString> vcsIn;
  vcsIn.push_back(scurl.strId);
  vcsIn.push_back(scurl.m_url[0].m_url);
  vector<CStdString> vcsOut = RunNoThrow(sFunc, scurl, fcurl, &vcsIn);

  // parse XML output
  bool fRet(false);
  for (CStdStringArray::const_iterator i = vcsOut.begin(); i != vcsOut.end(); ++i)
  {
    TiXmlDocument doc;
    doc.Parse(*i, 0, TIXML_ENCODING_UTF8);
    if (!doc.RootElement())
    {
      CLog::Log(LOGERROR, "%s: Unable to parse XML", __FUNCTION__);
      continue;
    }

    TiXmlHandle xhDoc(&doc);
    TiXmlElement *pxeDetails = xhDoc.FirstChild("details").Element();
    if (!pxeDetails)
    {
      CLog::Log(LOGERROR, "%s: Invalid XML file (want <details>)", __FUNCTION__);
      continue;
    }
    video.Load(pxeDetails, true/*fChain*/);
    fRet = true;  // but don't exit in case of chaining
  }
  return fRet;
}

// takes a URL; returns true and populates album on success, false otherwise
bool CScraper::GetAlbumDetails(CFileCurl &fcurl, const CScraperUrl &scurl, CAlbum &album)
{
  CLog::Log(LOGDEBUG, "%s: Reading '%s' using %s scraper "
    "(file: '%s', content: '%s', version: '%s')", __FUNCTION__,
    scurl.m_url[0].m_url.c_str(), Name().c_str(), Path().c_str(),
    ADDON::TranslateContent(Content()).c_str(), Version().c_str());

  vector<CStdString> vcsOut = RunNoThrow("GetAlbumDetails", scurl, fcurl);

  // parse the returned XML into an album object (see CAlbum::Load for details)
  bool fRet(false);
  for (CStdStringArray::const_iterator i = vcsOut.begin(); i != vcsOut.end(); ++i)
  {
    TiXmlDocument doc;
    doc.Parse(*i, 0, TIXML_ENCODING_UTF8);
    if (!doc.RootElement())
    {
      CLog::Log(LOGERROR, "%s: Unable to parse XML", __FUNCTION__);
      return false;
    }
    fRet = album.Load(doc.RootElement(), i != vcsOut.begin());
  }
  return fRet;
}

// takes a URL (one returned from FindArtist), the original search string, and
// returns true and populates artist on success, false on failure
bool CScraper::GetArtistDetails(CFileCurl &fcurl, const CScraperUrl &scurl,
  const CStdString &sSearch, CArtist &artist)
{
  if (!scurl.m_url.size())
    return false;

  CLog::Log(LOGDEBUG, "%s: Reading '%s' ('%s') using %s scraper "
    "(file: '%s', content: '%s', version: '%s')", __FUNCTION__,
    scurl.m_url[0].m_url.c_str(), sSearch.c_str(), Name().c_str(), Path().c_str(),
    ADDON::TranslateContent(Content()).c_str(), Version().c_str());

  // pass in the original search string for chaining to search other sites
  vector<CStdString> vcIn;
  vcIn.push_back(sSearch);
  CURL::Encode(vcIn[0]);

  vector<CStdString> vcsOut = RunNoThrow("GetArtistDetails", scurl, fcurl, &vcIn);

  // ok, now parse the xml file
  bool fRet(false);
  for (vector<CStdString>::const_iterator i = vcsOut.begin(); i != vcsOut.end(); ++i)
  {
    TiXmlDocument doc;
    doc.Parse(*i, 0, TIXML_ENCODING_UTF8);
    if (!doc.RootElement())
    {
      CLog::Log(LOGERROR, "%s: Unable to parse XML", __FUNCTION__);
      return false;
    }

    fRet = artist.Load(doc.RootElement(), i != vcsOut.begin());
  }
  return fRet;
}

}

