#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArFunctorASyncTask.h"

AREXPORT ArFunctorASyncTask::ArFunctorASyncTask(ArRetFunctor1<void *, void *> *functor)
{
  setThreadName(functor->getName());
  myFunc = functor;
}

AREXPORT ArFunctorASyncTask::~ArFunctorASyncTask()
{

}

AREXPORT void *ArFunctorASyncTask::runThread(void *arg)
{
  threadStarted();
  void *ret = myFunc->invokeR(arg);
  threadFinished();
  return ret;
}
