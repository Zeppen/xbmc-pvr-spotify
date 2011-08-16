/*
 * StarredBackgroundLoader.h
 *
 *  Created on: Aug 15, 2011
 *      Author: david
 */

#ifndef STARREDBACKGROUNDLOADER_H_
#define STARREDBACKGROUNDLOADER_H_

#include "threads/Thread.h"

namespace addon_music_spotify {

  class StarredBackgroundLoader: public CThread {
  public:
    StarredBackgroundLoader();
    virtual ~StarredBackgroundLoader();

    void SleepMs(int ms){ Sleep(ms); }

  private:
    void OnStartup();
    void OnExit();
    void OnException(); // signal termination handler
    void Process();
  };

} /* namespace addon_music_spotify */
#endif /* STARREDBACKGROUNDLOADER_H_ */
