#ifndef ARSYNCLOOP_H
#define ARSYNCLOOP_H


#include "ariaTypedefs.h"
#include "ArASyncTask.h"
#include "ArSyncTask.h"


class ArRobot;


class ArSyncLoop : public ArASyncTask
{
public:

  AREXPORT ArSyncLoop();
  AREXPORT virtual ~ArSyncLoop();

  AREXPORT void setRobot(ArRobot *robot);

  AREXPORT void stopRunIfNotConnected(bool stopRun);
  AREXPORT virtual void * runThread(void *arg);

  AREXPORT virtual const char *getThreadActivity(void);


protected:
  bool myStopRunIfNotConnected;
  ArRobot *myRobot;
  bool myInRun;

};


#endif // ARSYNCLOOP_H
