/*
 *      Copyright (C) 2005-2011 Team XBMC
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

#include "guilib/LocalizeStrings.h"
#include "Epg.h"
#include "EpgInfoTag.h"
#include "EpgContainer.h"
#include "EpgDatabase.h"
#include "pvr/channels/PVRChannelGroupsContainer.h"
#include "pvr/timers/PVRTimers.h"
#include "pvr/PVRManager.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"
#include "addons/include/xbmc_pvr_types.h"

using namespace std;
using namespace EPG;
using namespace PVR;

CEpgInfoTag::CEpgInfoTag(int iEpgId /* = -1 */, int iPVRChannelNumber /* = -1 */, int iPVRChannelID /* = -1 */, const CStdString &strTableName /* = StringUtils::EmptyString */) :
    m_bNotify(false),
    m_bChanged(false),
    m_iBroadcastId(-1),
    m_iGenreType(0),
    m_iGenreSubType(0),
    m_iParentalRating(0),
    m_iStarRating(0),
    m_iSeriesNumber(0),
    m_iEpisodeNumber(0),
    m_iEpisodePart(0),
    m_iUniqueBroadcastID(-1),
    m_strTitle(""),
    m_strPlotOutline(""),
    m_strPlot(""),
    m_strGenre(""),
    m_strEpisodeName(""),
    m_strIconPath(""),
    m_strFileNameAndPath(""),
    m_iTimerId(-1),
    m_iEpgId(iEpgId),
    m_iPVRChannelNumber(iPVRChannelNumber),
    m_iPVRChannelID(iPVRChannelID),
    m_strTableName(strTableName)
{
}

CEpgInfoTag::CEpgInfoTag(const EPG_TAG &data) :
    m_bNotify(false),
    m_bChanged(false),
    m_iBroadcastId(-1),
    m_iGenreType(0),
    m_iGenreSubType(0),
    m_iParentalRating(0),
    m_iStarRating(0),
    m_iSeriesNumber(0),
    m_iEpisodeNumber(0),
    m_iEpisodePart(0),
    m_iUniqueBroadcastID(-1),
    m_strTitle(""),
    m_strPlotOutline(""),
    m_strPlot(""),
    m_strGenre(""),
    m_strEpisodeName(""),
    m_strIconPath(""),
    m_strFileNameAndPath(""),
    m_iTimerId(-1),
    m_iEpgId(-1),
    m_iPVRChannelNumber(-1),
    m_iPVRChannelID(-1)
{
  Update(data);
}

CEpgInfoTag::CEpgInfoTag(const CEpgInfoTag &tag) :
    m_bNotify(tag.m_bNotify),
    m_bChanged(tag.m_bChanged),
    m_iBroadcastId(tag.m_iBroadcastId),
    m_iGenreType(tag.m_iGenreType),
    m_iGenreSubType(tag.m_iGenreSubType),
    m_iParentalRating(tag.m_iParentalRating),
    m_iStarRating(tag.m_iStarRating),
    m_iSeriesNumber(tag.m_iSeriesNumber),
    m_iEpisodeNumber(tag.m_iEpisodeNumber),
    m_iEpisodePart(tag.m_iEpisodePart),
    m_iUniqueBroadcastID(tag.m_iUniqueBroadcastID),
    m_strTitle(tag.m_strTitle),
    m_strPlotOutline(tag.m_strPlotOutline),
    m_strPlot(tag.m_strPlot),
    m_strGenre(tag.m_strGenre),
    m_strEpisodeName(tag.m_strEpisodeName),
    m_strIconPath(tag.m_strIconPath),
    m_strFileNameAndPath(tag.m_strFileNameAndPath),
    m_startTime(tag.m_startTime),
    m_endTime(tag.m_endTime),
    m_firstAired(tag.m_firstAired),
    m_iTimerId(tag.m_iTimerId),
    m_iEpgId(tag.m_iEpgId),
    m_iPVRChannelNumber(tag.m_iPVRChannelNumber),
    m_iPVRChannelID(tag.m_iPVRChannelID),
    m_strTableName(tag.m_strTableName)
{
}

CEpgInfoTag::~CEpgInfoTag()
{
  CPVRTimerInfoTag* tag = Timer();
  if (tag)
    tag->OnEpgTagDeleted();
}

bool CEpgInfoTag::operator ==(const CEpgInfoTag& right) const
{
  if (this == &right) return true;

  CSingleLock lock(m_critSection);
  return (m_bNotify            == right.m_bNotify &&
          m_bChanged           == right.m_bChanged &&
          m_iBroadcastId       == right.m_iBroadcastId &&
          m_iGenreType         == right.m_iGenreType &&
          m_iGenreSubType      == right.m_iGenreSubType &&
          m_iParentalRating    == right.m_iParentalRating &&
          m_firstAired         == right.m_firstAired &&
          m_iStarRating        == right.m_iStarRating &&
          m_iSeriesNumber      == right.m_iSeriesNumber &&
          m_iEpisodeNumber     == right.m_iEpisodeNumber &&
          m_iEpisodePart       == right.m_iEpisodePart &&
          m_iUniqueBroadcastID == right.m_iUniqueBroadcastID &&
          m_strTitle           == right.m_strTitle &&
          m_strPlotOutline     == right.m_strPlotOutline &&
          m_strPlot            == right.m_strPlot &&
          m_strGenre           == right.m_strGenre &&
          m_strEpisodeName     == right.m_strEpisodeName &&
          m_strIconPath        == right.m_strIconPath &&
          m_strFileNameAndPath == right.m_strFileNameAndPath &&
          m_startTime          == right.m_startTime &&
          m_endTime            == right.m_endTime &&
          m_iPVRChannelNumber  == right.m_iPVRChannelNumber &&
          m_iPVRChannelID      == right.m_iPVRChannelID &&
          m_strTableName       == right.m_strTableName);
}

bool CEpgInfoTag::operator !=(const CEpgInfoTag& right) const
{
  if (this == &right) return false;

  return !(*this == right);
}

CEpgInfoTag &CEpgInfoTag::operator =(const CEpgInfoTag &other)
{
  CSingleLock lock(other.m_critSection);

  m_bNotify            = other.m_bNotify;
  m_bChanged           = other.m_bChanged;
  m_iBroadcastId       = other.m_iBroadcastId;
  m_iGenreType         = other.m_iGenreType;
  m_iGenreSubType      = other.m_iGenreSubType;
  m_iParentalRating    = other.m_iParentalRating;
  m_iStarRating        = other.m_iStarRating;
  m_iSeriesNumber      = other.m_iSeriesNumber;
  m_iEpisodeNumber     = other.m_iEpisodeNumber;
  m_iEpisodePart       = other.m_iEpisodePart;
  m_iUniqueBroadcastID = other.m_iUniqueBroadcastID;
  m_strTitle           = other.m_strTitle;
  m_strPlotOutline     = other.m_strPlotOutline;
  m_strPlot            = other.m_strPlot;
  m_strGenre           = other.m_strGenre;
  m_strEpisodeName     = other.m_strEpisodeName;
  m_strIconPath        = other.m_strIconPath;
  m_strFileNameAndPath = other.m_strFileNameAndPath;
  m_startTime          = other.m_startTime;
  m_endTime            = other.m_endTime;
  m_firstAired         = other.m_firstAired;
  m_iTimerId           = other.m_iTimerId;
  m_timerStart         = other.m_timerStart;
  m_iEpgId             = other.m_iEpgId;
  m_iPVRChannelNumber  = other.m_iPVRChannelNumber;
  m_iPVRChannelID      = other.m_iPVRChannelID;
  m_strTableName       = other.m_strTableName;

  return *this;
}

bool CEpgInfoTag::Changed(void) const
{
  CSingleLock lock(m_critSection);
  return m_bChanged;
}

bool CEpgInfoTag::IsActive(void) const
{
  CDateTime now = CDateTime::GetUTCDateTime();
  CSingleLock lock(m_critSection);
  return (m_startTime <= now && m_endTime > now);
}

bool CEpgInfoTag::WasActive(void) const
{
  CDateTime now = CDateTime::GetUTCDateTime();
  CSingleLock lock(m_critSection);
  return (m_endTime < now);
}

bool CEpgInfoTag::InTheFuture(void) const
{
  CDateTime now = CDateTime::GetUTCDateTime();
  CSingleLock lock(m_critSection);
  return (m_startTime > now);
}

float CEpgInfoTag::ProgressPercentage(void) const
{
  float fReturn(0);
  int iDuration;
  time_t currentTime, startTime, endTime;
  CDateTime::GetCurrentDateTime().GetAsUTCDateTime().GetAsTime(currentTime);

  CSingleLock lock(m_critSection);
  m_startTime.GetAsTime(startTime);
  m_endTime.GetAsTime(endTime);
  iDuration = endTime - startTime > 0 ? endTime - startTime : 3600;

  if (currentTime >= startTime && currentTime <= endTime)
    fReturn = ((float) currentTime - startTime) / iDuration * 100;
  else if (currentTime > endTime)
    fReturn = 100;

  return fReturn;
}

const CEpgInfoTag *CEpgInfoTag::GetNextEvent(void) const
{
  CSingleLock lock(m_critSection);
  return GetTable()->GetNextEvent(*this);
}

const CEpgInfoTag *CEpgInfoTag::GetPreviousEvent(void) const
{
  CSingleLock lock(m_critSection);
  return GetTable()->GetPreviousEvent(*this);
}

void CEpgInfoTag::SetUniqueBroadcastID(int iUniqueBroadcastID)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_iUniqueBroadcastID != iUniqueBroadcastID)
    {
      m_iUniqueBroadcastID = iUniqueBroadcastID;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

int CEpgInfoTag::UniqueBroadcastID(void) const
{
  CSingleLock lock(m_critSection);
  return m_iUniqueBroadcastID;
}

void CEpgInfoTag::SetBroadcastId(int iId)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_iBroadcastId != iId)
    {
      m_iBroadcastId = iId;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

int CEpgInfoTag::BroadcastId(void) const
{
  CSingleLock lock(m_critSection);
  return m_iBroadcastId;
}

CDateTime CEpgInfoTag::StartAsUTC(void) const
{
  CSingleLock lock(m_critSection);
  return m_startTime;
}

CDateTime CEpgInfoTag::StartAsLocalTime(void) const
{
  CDateTime retVal;
  CSingleLock lock(m_critSection);
  retVal.SetFromUTCDateTime(m_startTime);
  return retVal;
}

void CEpgInfoTag::SetStartFromUTC(const CDateTime &start)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_startTime != start)
    {
      m_startTime = start;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

void CEpgInfoTag::SetStartFromLocalTime(const CDateTime &start)
{
  CDateTime tmp = start.GetAsUTCDateTime();
  SetStartFromUTC(tmp);
}

CDateTime CEpgInfoTag::EndAsUTC(void) const
{
  CDateTime retVal;
  CSingleLock lock(m_critSection);
  retVal = m_endTime;
  return retVal;
}

CDateTime CEpgInfoTag::EndAsLocalTime(void) const
{
  CDateTime retVal;
  CSingleLock lock(m_critSection);
  retVal.SetFromUTCDateTime(m_endTime);
  return retVal;
}

void CEpgInfoTag::SetEndFromUTC(const CDateTime &end)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_endTime != end)
    {
      m_endTime = end;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

void CEpgInfoTag::SetEndFromLocalTime(const CDateTime &end)
{
  CDateTime tmp = end.GetAsUTCDateTime();
  SetEndFromUTC(tmp);
}

int CEpgInfoTag::GetDuration(void) const
{
  time_t start, end;
  CSingleLock lock(m_critSection);
  m_startTime.GetAsTime(start);
  m_endTime.GetAsTime(end);
  return end - start > 0 ? end - start : 3600;
}

void CEpgInfoTag::SetTitle(const CStdString &strTitle)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_strTitle != strTitle)
    {
      m_strTitle = strTitle;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

CStdString CEpgInfoTag::Title(void) const
{
  CStdString retVal;
  CSingleLock lock(m_critSection);
  retVal = (m_strTitle.IsEmpty()) ?
      g_localizeStrings.Get(19055) :
      m_strTitle;
  return retVal;
}

void CEpgInfoTag::SetPlotOutline(const CStdString &strPlotOutline)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_strPlotOutline != strPlotOutline)
    {
      m_strPlotOutline = strPlotOutline;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

CStdString CEpgInfoTag::PlotOutline(void) const
{
  CStdString retVal;
  CSingleLock lock(m_critSection);
  retVal = m_strPlotOutline;
  return retVal;
}

void CEpgInfoTag::SetPlot(const CStdString &strPlot)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    CStdString strPlotClean = (m_strPlotOutline.length() > 0 && strPlot.Left(m_strPlotOutline.length()).Equals(m_strPlotOutline)) ?
      strPlot.Right(strPlot.length() - m_strPlotOutline.length()) :
      strPlot;

    if (m_strPlot != strPlotClean)
    {
      m_strPlot = strPlotClean;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

CStdString CEpgInfoTag::Plot(void) const
{
  CStdString retVal;
  CSingleLock lock(m_critSection);
  retVal = m_strPlot;
  return retVal;
}

void CEpgInfoTag::SetGenre(int iID, int iSubID, const char* strGenre)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_iGenreType != iID || m_iGenreSubType != iSubID)
    {
      m_iGenreType    = iID;
      m_iGenreSubType = iSubID;
      if ((iID == EPG_GENRE_USE_STRING) && (strGenre != NULL) && (strlen(strGenre) > 0))
      {
        /* Type and sub type are not given. No EPG color coding possible
         * Use the provided genre description as backup. */
        m_strGenre    = strGenre;
      }
      else
      {
        /* Determine the genre description from the type and subtype IDs */
        m_strGenre      = CEpg::ConvertGenreIdToString(iID, iSubID);
      }
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

int CEpgInfoTag::GenreType(void) const
{
  CSingleLock lock(m_critSection);
  return m_iGenreType;
}

int CEpgInfoTag::GenreSubType(void) const
{
  CSingleLock lock(m_critSection);
  return m_iGenreSubType;
}

CStdString CEpgInfoTag::Genre(void) const
{
  CStdString retVal;
  CSingleLock lock(m_critSection);
  retVal = m_strGenre;
  return retVal;
}

CDateTime CEpgInfoTag::FirstAiredAsUTC(void) const
{
  CDateTime retVal;
  CSingleLock lock(m_critSection);
  retVal = m_firstAired;
  return retVal;
}

CDateTime CEpgInfoTag::FirstAiredAsLocalTime(void) const
{
  CDateTime retVal;
  CSingleLock lock(m_critSection);
  retVal.SetFromUTCDateTime(m_firstAired);
  return retVal;
}

void CEpgInfoTag::SetFirstAiredFromUTC(const CDateTime &firstAired)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_firstAired != firstAired)
    {
      m_firstAired = firstAired;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

void CEpgInfoTag::SetFirstAiredFromLocalTime(const CDateTime &firstAired)
{
  CDateTime tmp = firstAired.GetAsUTCDateTime();
  SetStartFromUTC(tmp);
}

void CEpgInfoTag::SetParentalRating(int iParentalRating)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_iParentalRating != iParentalRating)
    {
      m_iParentalRating = iParentalRating;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

int CEpgInfoTag::ParentalRating(void) const
{
  CSingleLock lock(m_critSection);
  return m_iParentalRating;
}

void CEpgInfoTag::SetStarRating(int iStarRating)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_iStarRating != iStarRating)
    {
      m_iStarRating = iStarRating;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

int CEpgInfoTag::StarRating(void) const
{
  CSingleLock lock(m_critSection);
  return m_iStarRating;
}

void CEpgInfoTag::SetNotify(bool bNotify)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_bNotify != bNotify)
    {
      m_bNotify = bNotify;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

bool CEpgInfoTag::Notify(void) const
{
  CSingleLock lock(m_critSection);
  return m_bNotify;
}

void CEpgInfoTag::SetSeriesNum(int iSeriesNum)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_iSeriesNumber != iSeriesNum)
    {
      m_iSeriesNumber = iSeriesNum;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

int CEpgInfoTag::SeriesNum(void) const
{
  CSingleLock lock(m_critSection);
  return m_iSeriesNumber;
}

void CEpgInfoTag::SetEpisodeNum(int iEpisodeNum)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_iEpisodeNumber != iEpisodeNum)
    {
      m_iEpisodeNumber = iEpisodeNum;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

int CEpgInfoTag::EpisodeNum(void) const
{
  CSingleLock lock(m_critSection);
  return m_iEpisodeNumber;
}

void CEpgInfoTag::SetEpisodePart(int iEpisodePart)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_iEpisodePart != iEpisodePart)
    {
      m_iEpisodePart = iEpisodePart;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

int CEpgInfoTag::EpisodePart(void) const
{
  CSingleLock lock(m_critSection);
  return m_iEpisodePart;
}

void CEpgInfoTag::SetEpisodeName(const CStdString &strEpisodeName)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_strEpisodeName != strEpisodeName)
    {
      m_strEpisodeName = strEpisodeName;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

CStdString CEpgInfoTag::EpisodeName(void) const
{
  CStdString retVal;
  CSingleLock lock(m_critSection);
  retVal = m_strEpisodeName;
  return retVal;
}

void CEpgInfoTag::SetIcon(const CStdString &strIconPath)
{
  bool bUpdate(false);
  {
    CSingleLock lock(m_critSection);
    if (m_strIconPath != strIconPath)
    {
      m_strIconPath = strIconPath;
      m_bChanged = true;
      bUpdate = true;
    }
  }
  if (bUpdate)
    UpdatePath();
}

CStdString CEpgInfoTag::Icon(void) const
{
  CStdString retVal;
  CEpg *epg = g_EpgContainer.GetById(m_iEpgId);

  CSingleLock lock(m_critSection);
  retVal = m_strIconPath;
  if (retVal.IsEmpty() && epg && epg->HasPVRChannel())
     retVal = epg->Channel()->IconPath();
  return retVal;
}

void CEpgInfoTag::SetPath(const CStdString &strFileNameAndPath)
{
  CSingleLock lock(m_critSection);
  if (m_strFileNameAndPath != strFileNameAndPath)
  {
    m_strFileNameAndPath = strFileNameAndPath;
    m_bChanged = true;
  }
}

CStdString CEpgInfoTag::Path(void) const
{
  CStdString retVal;
  CSingleLock lock(m_critSection);
  retVal = m_strFileNameAndPath;
  return retVal;
}

void CEpgInfoTag::SetTimer(CPVRTimerInfoTag *newTimer)
{
  CPVRTimerInfoTag *oldTimer(NULL);
  {
    CSingleLock lock(m_critSection);
    if (g_PVRManager.IsStarted())
      oldTimer = Timer();
    m_timerStart = newTimer->StartAsUTC();
    m_iTimerId   = newTimer->m_iClientIndex;
  }
  if (oldTimer)
    oldTimer->SetEpgInfoTag(NULL);
}

void CEpgInfoTag::OnTimerDeleted(void)
{
  CSingleLock lock(m_critSection);
  m_iTimerId = -1;
}

bool CEpgInfoTag::HasTimer(void) const
{
  CSingleLock lock(m_critSection);
  return m_iTimerId != -1;
}

CPVRTimerInfoTag *CEpgInfoTag::Timer(void) const
{
  CPVRTimerInfoTag* tag(NULL);
  CSingleLock lock(m_critSection);
  if (m_iTimerId >= 0)
    tag = g_PVRTimers->GetTimer(m_timerStart, m_iTimerId);

  return tag;
}

bool CEpgInfoTag::HasPVRChannel(void) const
{
  CSingleLock lock(m_critSection);
  return m_iPVRChannelID != -1;
}

int CEpgInfoTag::PVRChannelNumber(void) const
{
  CSingleLock lock(m_critSection);
  return m_iPVRChannelNumber;
}

CStdString CEpgInfoTag::PVRChannelName(void) const
{
  CStdString strReturn;
  CSingleLock lock(m_critSection);
  strReturn = m_strTableName;
  return strReturn;
}

const PVR::CPVRChannel *CEpgInfoTag::ChannelTag(void) const
{
  int iChannelId(-1);
  {
    CSingleLock lock(m_critSection);
    iChannelId = m_iPVRChannelID;
  }

  return (iChannelId != -1) ?
      g_PVRChannelGroups->GetByChannelIDFromAll(iChannelId) :
      NULL;
}

void CEpgInfoTag::Update(const EPG_TAG &tag)
{
  CSingleLock lock(m_critSection);
  SetStartFromUTC(tag.startTime + g_advancedSettings.m_iPVRTimeCorrection);
  SetEndFromUTC(tag.endTime + g_advancedSettings.m_iPVRTimeCorrection);
  SetTitle(tag.strTitle);
  SetPlotOutline(tag.strPlotOutline);
  SetPlot(tag.strPlot);
  SetGenre(tag.iGenreType, tag.iGenreSubType, tag.strGenreDescription);
  SetParentalRating(tag.iParentalRating);
  SetUniqueBroadcastID(tag.iUniqueBroadcastId);
  SetNotify(tag.bNotify);
  SetFirstAiredFromUTC(tag.firstAired + g_advancedSettings.m_iPVRTimeCorrection);
  SetEpisodeNum(tag.iEpisodeNumber);
  SetEpisodePart(tag.iEpisodePartNumber);
  SetEpisodeName(tag.strEpisodeName);
  SetStarRating(tag.iStarRating);
  SetIcon(tag.strIconPath);
}

bool CEpgInfoTag::Update(const CEpgInfoTag &tag, bool bUpdateBroadcastId /* = true */)
{
  bool bChanged(false);
  {
    CSingleLock lock(m_critSection);
    bChanged = (
        m_strTitle           != tag.m_strTitle ||
        m_strPlotOutline     != tag.m_strPlotOutline ||
        m_strPlot            != tag.m_strPlot ||
        m_startTime          != tag.m_startTime ||
        m_endTime            != tag.m_endTime ||
        m_iGenreType         != tag.m_iGenreType ||
        m_iGenreSubType      != tag.m_iGenreSubType ||
        m_firstAired         != tag.m_firstAired ||
        m_iParentalRating    != tag.m_iParentalRating ||
        m_iStarRating        != tag.m_iStarRating ||
        m_bNotify            != tag.m_bNotify ||
        m_iEpisodeNumber     != tag.m_iEpisodeNumber ||
        m_iEpisodePart       != tag.m_iEpisodePart ||
        m_iSeriesNumber      != tag.m_iSeriesNumber ||
        m_strEpisodeName     != tag.m_strEpisodeName ||
        m_iUniqueBroadcastID != tag.m_iUniqueBroadcastID ||
        m_iEpgId             != tag.m_iEpgId ||
        m_iPVRChannelID      != tag.m_iPVRChannelID ||
        m_iPVRChannelNumber  != tag.m_iPVRChannelNumber ||
        m_strTableName       != tag.m_strTableName ||
        ( tag.m_strGenre.length() > 0 && m_strGenre != tag.m_strGenre )
    );
    if (bUpdateBroadcastId)
      bChanged = bChanged || m_iBroadcastId != tag.m_iBroadcastId;

    if (bChanged)
    {
      if (bUpdateBroadcastId)
        m_iBroadcastId       = tag.m_iBroadcastId;

      m_strTitle           = tag.m_strTitle;
      m_strPlotOutline     = tag.m_strPlotOutline;
      m_strPlot            = tag.m_strPlot;
      m_startTime          = tag.m_startTime;
      m_endTime            = tag.m_endTime;
      m_iGenreType         = tag.m_iGenreType;
      m_iGenreSubType      = tag.m_iGenreSubType;
      m_iEpgId             = tag.m_iEpgId;
      m_iPVRChannelID      = tag.m_iPVRChannelID;
      m_iPVRChannelNumber  = tag.m_iPVRChannelNumber;
      m_strTableName       = tag.m_strTableName;
      if (m_iGenreType == EPG_GENRE_USE_STRING && tag.m_strGenre.length() > 0)
      {
        /* No type/subtype. Use the provided description */
        m_strGenre         = tag.m_strGenre;
      }
      else
      {
        /* Determine genre description by type/subtype */
        m_strGenre         = CEpg::ConvertGenreIdToString(tag.m_iGenreType, tag.m_iGenreSubType);
      }
      m_firstAired         = tag.m_firstAired;
      m_iParentalRating    = tag.m_iParentalRating;
      m_iStarRating        = tag.m_iStarRating;
      m_bNotify            = tag.m_bNotify;
      m_iEpisodeNumber     = tag.m_iEpisodeNumber;
      m_iEpisodePart       = tag.m_iEpisodePart;
      m_iSeriesNumber      = tag.m_iSeriesNumber;
      m_strEpisodeName     = tag.m_strEpisodeName;
      m_iUniqueBroadcastID = tag.m_iUniqueBroadcastID;

      m_bChanged = true;
    }
  }
  if (bChanged)
    UpdatePath();

  return bChanged;
}

bool CEpgInfoTag::Persist(bool bSingleUpdate /* = true */)
{
  bool bReturn = false;
  CSingleLock lock(m_critSection);
  if (!m_bChanged)
    return true;
  CLog::Log(LOGDEBUG, "Epg - %s - Infotag '%s' %s, persisting...", __FUNCTION__, m_strTitle.c_str(), m_iBroadcastId > 0 ? "has changes" : "is new");
  CEpgDatabase *database = g_EpgContainer.GetDatabase();
  if (!database || (bSingleUpdate && !database->IsOpen()))
  {
    CLog::Log(LOGERROR, "%s - could not open the database", __FUNCTION__);
    return bReturn;
  }

  int iId = database->Persist(*this, bSingleUpdate);
  if (iId >= 0)
  {
    bReturn = true;

    if (iId > 0)
    {
      m_iBroadcastId = iId;
      m_bChanged = false;
    }
  }

  return bReturn;
}

void CEpgInfoTag::UpdatePath(void)
{
  CStdString path;
  {
    CSingleLock lock(m_critSection);
    path.Format("pvr://guide/%04i/%s.epg", m_iEpgId, m_startTime.GetAsDBDateTime().c_str());
  }

  SetPath(path);
}

const CEpg *CEpgInfoTag::GetTable() const
{
  return g_EpgContainer.GetById(m_iEpgId);
}
