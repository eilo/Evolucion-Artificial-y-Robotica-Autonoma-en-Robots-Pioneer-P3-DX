#ifndef ARFUNCTORASYNCTASK_H
#define ARFUNCTORASYNCTASK_H

#include "ariaTypedefs.h"
#include "ArASyncTask.h"
#include "ArFunctor.h"

/// This is like ArASyncTask, but instead of runThread it uses a functor to run
class ArFunctorASyncTask : public ArASyncTask
{
public:
  /// Constructor
  AREXPORT ArFunctorASyncTask(ArRetFunctor1<void *, void *> *functor);
  /// Destructor
  AREXPORT virtual ~ArFunctorASyncTask();
  /// Our reimplementation of runThread
  AREXPORT virtual void *runThread(void *arg);
protected:
  ArRetFunctor1<void *, void *> *myFunc;
};

#endif
