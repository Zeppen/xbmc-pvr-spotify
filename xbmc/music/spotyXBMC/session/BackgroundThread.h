/*
 * BackgroundThread.h
 *
 *  Created on: Aug 15, 2011
 *      Author: david
 */

#ifndef BACKGROUNDTHREAD_H_
#define BACKGROUNDTHREAD_H_

#include "threads/Thread.h"

namespace addon_music_spotify {

  class BackgroundThread: public CThread {
  public:
    BackgroundThread();
    virtual ~BackgroundThread();

    void SleepMs(int ms){ Sleep(ms); }

  private:
    void OnStartup();
    void OnExit();
    void OnException(); // signal termination handler
    void Process();
  };

} /* namespace addon_music_spotify */
#endif /* BACKGROUNDTHREAD_H_ */
