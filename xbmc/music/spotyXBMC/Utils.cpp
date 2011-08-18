/*
 * Utils.cpp
 *
 *  Created on: Aug 17, 2011
 *      Author: david
 */

#include "Utils.h"
#include "Settings.h"
#include "../tags/MusicInfoTag.h"
#include "../Album.h"
#include "../Artist.h"
#include "../../MediaSource.h"

namespace addon_music_spotify {

  Utils::Utils() {
  }

  Utils::~Utils() {
  }

  void Utils::cleanTags(string & text) {
    bool done = false;
    while (!done) {
      // Look for start of tag:
      size_t leftPos = text.find('<');
      if (leftPos != string::npos) {
        // See if tag close is in this line:
        size_t rightPos = text.find('>');
        if (rightPos == string::npos) {
          done = true;
          text.erase(leftPos);
        } else
          text.erase(leftPos, rightPos - leftPos + 1);
      } else
        done = true;
    }
  }

  const CFileItemPtr Utils::SxAlbumToItem(SxAlbum *album, string prefix, int discNumber) {
    //wait for it to finish loading
    while (!album->isLoaded()) {
    }

    CAlbum outAlbum;
    outAlbum.strArtist = album->getAlbumArtistName();
    CStdString title;
    if (discNumber != 0)
      title.Format("%s%s %s %i", prefix, album->getAlbumName(), "disc", discNumber);
    else
      title.Format("%s%s", prefix, album->getAlbumName());
    outAlbum.strAlbum = title;
    outAlbum.iYear = album->getAlbumYear();
    outAlbum.strReview = album->getReview();
    outAlbum.iRating = album->getAlbumRating();
    CStdString path;
    path.Format("musicdb://3/%s#%i", album->getUri(), discNumber);
    const CFileItemPtr pItem(new CFileItem(path, outAlbum));
    if (album->hasThumb()) pItem->SetThumbnailImage(album->getThumb()->getPath());
    pItem->SetProperty("fanart_image", Settings::getFanart());
    return pItem;
  }

  const CFileItemPtr Utils::SxTrackToItem(SxTrack* track, string prefix, int trackNumber) {
    //wait for it to finish loading
    while (!track->isLoaded()) {
    }

    CSong outSong;
    CStdString path;
    path.Format("%s.spotify", track->getUri());
    outSong.strFileName = path;
    CStdString title;
    title.Format("%s%s", prefix, track->getName());
    outSong.strTitle = title;
    outSong.iTrack = trackNumber == -1 ? track->getTrackNumber() : trackNumber;
    outSong.iDuration = track->getDuration();
    outSong.rating = track->getRating();
    char* ratingChar = new char();
    CStdString ratingStr = itoa(1 + (track->getRating() / 2), ratingChar, 10);
    delete ratingChar;
    outSong.rating = ratingStr[0];
    outSong.strArtist = track->getArtistName();
    outSong.iYear = track->getYear();
    outSong.strAlbum = track->getAlbumName();
    outSong.strAlbumArtist = track->getAlbumArtistName();

    const CFileItemPtr pItem(new CFileItem(outSong));
    if (track->hasThumb()) pItem->SetThumbnailImage(track->getThumb()->getPath());
    pItem->SetProperty("fanart_image", Settings::getFanart());
    return pItem;
  }

  const CFileItemPtr Utils::SxArtistToItem(SxArtist* artist, string prefix) {
    //wait for it to finish loading
    while (!artist->isLoaded()) {
    }

    CStdString path;
    path.Format("musicdb://2/%s/", artist->getUri());

    CFileItemPtr pItem(new CFileItem(path, true));
    CStdString label;
    label.Format("%s%s", prefix, artist->getArtistName());
    pItem->SetLabel(label);
    label.Format("A %s", artist->getArtistName());

    pItem->GetMusicInfoTag()->SetTitle(label);
    if (artist->hasThumb()) pItem->SetThumbnailImage(artist->getThumb()->getPath());
    pItem->SetIconImage("DefaultArtist.png");
    pItem->SetProperty("fanart_image", Settings::getFanart());
    pItem->SetProperty("artist_description", artist->getBio());

    return pItem;
  }
} /* namespace addon_music_spotify */
