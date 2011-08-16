/*
 * SearchResultBackgroundLoading.h
 *
 *  Created on: Aug 16, 2011
 *      Author: david
 */

#ifndef SEARCHRESULTBACKGROUNDLOADER_H_
#define SEARCHRESULTBACKGROUNDLOADER_H_

#include "threads/Thread.h"

#include "Search.h"

namespace addon_music_spotify {

  class SearchResultBackgroundLoader: public CThread {
  public:
    SearchResultBackgroundLoader(Search* search);
    virtual ~SearchResultBackgroundLoader();

    void SleepMs(int ms) {
      Sleep(ms);
    }
  private:
    void OnStartup();
    void OnExit();
    void OnException(); // signal termination handler
    void Process();
    Search* m_search;
  };

} /* namespace addon_music_spotify */
#endif /* SEARCHRESULTBACKGROUNDLOADER_H_ */
