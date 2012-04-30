#include "ArExport.h"
#include "ariaOSDef.h"
#include <list>
#include "ArThread.h"
#include "ArLog.h"
#include "ArSignalHandler.h"


static DWORD WINAPI run(void *arg)
{
  ArThread *t=(ArThread*)arg;
  void *ret=NULL;

  if (t->getBlockAllSignals())
    ArSignalHandler::blockCommonThisThread();

  if (dynamic_cast<ArRetFunctor<void*>*>(t->getFunc()))
    ret=((ArRetFunctor<void*>*)t->getFunc())->invokeR();
  else
    t->getFunc()->invoke();

  return((DWORD)ret);
}

void ArThread::init()
{
  ArThread *main;
  ThreadType pt;
  MapType::iterator iter;

  pt=GetCurrentThreadId();

  ourThreadsMutex.lock();
  if (ourThreads.size())
  {
    ourThreadsMutex.unlock();
    return;
  }
  main=new ArThread;
  main->myJoinable=true;
  main->myRunning=true;
  main->myThread=pt;
  ourThreads.insert(MapType::value_type(pt, main));
  ourThreadsMutex.unlock();
}

AREXPORT ArThread::~ArThread()
{
  CloseHandle(myThreadHandle);

  // Just make sure the thread is no longer in the map.
  ourThreadsMutex.lock();
  ourThreads.erase(myThread);
  ourThreadsMutex.unlock();
}


AREXPORT ArThread * ArThread::self()
{
  ThreadType pt;
  MapType::iterator iter;

  ourThreadsMutex.lock();
  pt=GetCurrentThreadId();
  iter=ourThreads.find(pt);
  ourThreadsMutex.unlock();

  if (iter != ourThreads.end())
    return((*iter).second);
  else
    return(NULL);
}

AREXPORT ArThread::ThreadType ArThread::osSelf()
{ 
  return GetCurrentThreadId();
}

AREXPORT void ArThread::cancelAll()
{
  DWORD ret=0;
  std::map<HANDLE, ArThread *>::iterator iter;

  ourThreadsMutex.lock();
  for (iter=ourThreadHandles.begin(); iter != ourThreadHandles.end(); ++iter)
    TerminateThread((*iter).first, ret);
  ourThreadHandles.clear();
  ourThreads.clear();
  ourThreadsMutex.unlock();
}


AREXPORT int ArThread::create(ArFunctor *func, bool joinable,
			      bool lowerPriority)
{
  DWORD err;

  myJoinable=joinable;
  myFunc=func;
  myRunning=true;

  myThreadHandle = CreateThread(0, 0, &run, this, 0, &myThread);
  err=GetLastError();
  if (myThreadHandle == 0)
  {
    ArLog::log(ArLog::Terse, "ArThread::create: Failed to create thread.");
    return(STATUS_FAILED);
  }
  else
  {
    if (myName.size() == 0)
      ArLog::log(ourLogLevel, "Created anonymous thread with ID %d", 
		 myThread);
    else
      ArLog::log(ourLogLevel, "Created %s thread with ID %d", myName.c_str(),
		 myThread);
    ourThreadsMutex.lock();
    ourThreads.insert(MapType::value_type(myThread, this));
	ourThreadHandles.insert(std::map<HANDLE, ArThread *>::value_type(myThreadHandle, this));
    ourThreadsMutex.unlock();
    if (lowerPriority)
      SetThreadPriority(myThreadHandle, THREAD_PRIORITY_IDLE);
    return(0);
  }
}

AREXPORT int ArThread::doJoin(void **iret)
{
  DWORD ret;

  ret=WaitForSingleObject(myThreadHandle, INFINITE);
  if (ret == WAIT_FAILED)
  {
    ArLog::log(ArLog::Terse, "ArThread::doJoin: Failed to join on thread.");
    return(STATUS_FAILED);
  }

  return(0);
}

AREXPORT int ArThread::detach()
{
  return(0);
}

AREXPORT void ArThread::cancel()
{
  DWORD ret=0;

  ourThreadsMutex.lock();
  ourThreads.erase(myThread);
  ourThreadHandles.erase(myThreadHandle);
  ourThreadsMutex.unlock();
  TerminateThread(myThreadHandle, ret);
}

AREXPORT void ArThread::yieldProcessor()
{
  Sleep(0);
}


AREXPORT void ArThread::threadStarted(void)
{
  myStarted = true;
  if (myName.size() == 0)
    ArLog::log(ourLogLevel, "Anonymous thread (%d) is running",
	             myThread);
  else
    ArLog::log(ourLogLevel, "Thread %s (%d) is running", 
	             myName.c_str(), myThread);
}

AREXPORT void ArThread::threadFinished(void)
{
  myFinished = true;
  if (myName.size() == 0)
    ArLog::log(ourLogLevel, "Anonymous thread (%d) is finished",
	             myThread);
  else
    ArLog::log(ourLogLevel, "Thread %s (%d) is finished", 
	             myName.c_str(), myThread);
}

AREXPORT void ArThread::logThreadInfo(void)
{
  if (myName.size() == 0)
    ArLog::log(ourLogLevel, "Anonymous thread (%d) is running",
	             myThread);
  else
    ArLog::log(ourLogLevel, "Thread %s (%d) is running", 
	             myName.c_str(), myThread);
}
