#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArSyncLoop.h"
#include "ArLog.h"
#include "ariaUtil.h"
#include "ArRobot.h"


AREXPORT ArSyncLoop::ArSyncLoop() :
  ArASyncTask(),
  myStopRunIfNotConnected(false),
  myRobot(0)
{
  setThreadName("ArSyncLoop");
  myInRun = false;
}

AREXPORT ArSyncLoop::~ArSyncLoop()
{
}

AREXPORT void ArSyncLoop::setRobot(ArRobot *robot)
{
  myRobot=robot;
}

AREXPORT void ArSyncLoop::stopRunIfNotConnected(bool stopRun)
{
  myStopRunIfNotConnected = stopRun;
}

AREXPORT void * ArSyncLoop::runThread(void *arg)
{
  threadStarted();

  long timeToSleep;
  ArTime loopEndTime;
  std::list<ArFunctor *> *runList;
  std::list<ArFunctor *>::iterator iter;
  ArTime lastLoop;
  bool firstLoop = true;
  bool warned = false;

  if (!myRobot)
  {
    ArLog::log(ArLog::Terse, "ArSyncLoop::runThread: Trying to run the synchronous loop without a robot.");
    return(0);
  }

  if (!myRobot->getSyncTaskRoot())
  {
    ArLog::log(ArLog::Terse, "ArSyncLoop::runThread: Can not run the synchronous loop without a task tree");
    return(0);
  }

  while (myRunning)
  {

    myRobot->lock();
    if (!firstLoop && !warned && !myRobot->getNoTimeWarningThisCycle() && 
	myRobot->getCycleWarningTime() != 0 && 
	myRobot->getCycleWarningTime() > 0 && 
	lastLoop.mSecSince() > (signed int) myRobot->getCycleWarningTime())
    {
      ArLog::log(ArLog::Normal, 
 "Warning: ArRobot cycle took too long because the loop was waiting for lock.");
      ArLog::log(ArLog::Normal,
		 "\tThe cycle took %u ms, (%u ms normal %u ms warning)", 
		 lastLoop.mSecSince(), myRobot->getCycleTime(), 
		 myRobot->getCycleWarningTime());
    }
    myRobot->setNoTimeWarningThisCycle(false);
    firstLoop = false;
    warned = false;
    lastLoop.setToNow();

    loopEndTime.setToNow();
    loopEndTime.addMSec(myRobot->getCycleTime());
    myRobot->incCounter();
    myRobot->unlock();

    // note that all the stuff beyond here should maybe have a lock
    // but it should be okay because its just getting data
    myInRun = true;
    myRobot->getSyncTaskRoot()->run();
    myInRun = false;
    if (myStopRunIfNotConnected && !myRobot->isConnected())
    {
      if (myRunning)
	ArLog::log(ArLog::Normal, "Exiting robot run because of lost connection.");
      break;
    }
    timeToSleep = loopEndTime.mSecTo();
    // if the cycles chained and we're connected the packet handler will be 
    // doing the timing for us
    if (myRobot->isCycleChained() && myRobot->isConnected())
      timeToSleep = 0;

    if (!myRobot->getNoTimeWarningThisCycle() && 
	myRobot->getCycleWarningTime() != 0 && 
	myRobot->getCycleWarningTime() > 0 && 
	lastLoop.mSecSince() > (signed int) myRobot->getCycleWarningTime())
    {
      ArLog::log(ArLog::Normal, 
	"Warning: ArRobot sync tasks too long at %u ms, (%u ms normal %u ms warning)", 
		 lastLoop.mSecSince(), myRobot->getCycleTime(), 
		 myRobot->getCycleWarningTime());
      warned = true;
    }
    

    if (timeToSleep > 0)
      ArUtil::sleep(timeToSleep);
  }   
  myRobot->lock();
  myRobot->wakeAllRunExitWaitingThreads();
  myRobot->unlock();

  myRobot->lock();
  runList=myRobot->getRunExitListCopy();
  myRobot->unlock();
  for (iter=runList->begin();
       iter != runList->end(); ++iter)
    (*iter)->invoke();
  delete runList;

  threadFinished();
  return(0);
}

AREXPORT const char *ArSyncLoop::getThreadActivity(void)
{
  if (myRunning)
  {
    ArSyncTask *syncTask;
    syncTask = myRobot->getSyncTaskRoot()->getRunning();
    if (syncTask != NULL)
      return syncTask->getName().c_str();
    else
      return "Unknown running";
  }
  else
    return "Unknown"; 
}
