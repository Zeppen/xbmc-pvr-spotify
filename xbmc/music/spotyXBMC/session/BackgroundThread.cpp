/*
 * BackgroundThread.cpp
 *
 *  Created on: Aug 15, 2011
 *      Author: david
 */

#include "Session.h"
#include "BackgroundThread.h"
#include "../Logger.h"

namespace addon_music_spotify {

  BackgroundThread::BackgroundThread() {
    // TODO Auto-generated constructor stub

  }

  BackgroundThread::~BackgroundThread() {
    // TODO Auto-generated destructor stub
  }

  void BackgroundThread::OnStartup() {
    Logger::printOut("bgthread OnStartup");
    Session::getInstance()->connect();
    Session::getInstance()->unlock();
    Logger::printOut("bgthread OnStartup done");
  }

  void BackgroundThread::OnExit() {
    Logger::printOut("bgthread OnExit");
  }

  void BackgroundThread::OnException() {
    Logger::printOut("bgthread OnException");
  }

  void BackgroundThread::Process() {

    while (Session::getInstance()->isEnabled()) {
      //Logger::printOut("bgthread Process");
      //if the session is locked, sleep for awhile and try later again
     if (Session::getInstance()->m_nextEvent <= 0 && !Session::getInstance()->isLocked()){
        Session::getInstance()->lock();
        Session::getInstance()->processEvents();
        Session::getInstance()->unlock();
      }
      Session::getInstance()->m_nextEvent-= 10;
      Sleep(10);
    }
    Logger::printOut("exiting process thread");
  }

} /* namespace addon_music_spotify */
