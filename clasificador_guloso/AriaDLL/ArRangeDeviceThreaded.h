#ifndef ARRANGEDEVICETHREADED_H
#define ARRANGEDEVICETHREADED_H

#include "ariaTypedefs.h"
#include "ArRangeDevice.h"
#include "ArFunctorASyncTask.h"

/// A range device which can run in its own thread
/** 
    This is a range device thats threaded, it doesn't do
    multipleInheritance from both ArASyncTask and ArRangeDevice any
    more since JAVA doesn't support this and the wrapper software
    can't deal with it.  Its still functionally the same however.
 **/
class ArRangeDeviceThreaded : public ArRangeDevice
{
public:
  /// Constructor
  AREXPORT ArRangeDeviceThreaded(size_t currentBufferSize, 
				 size_t cumulativeBufferSize,
				 const char *name, unsigned int maxRange,
				 int maxSecondsTokeepCurrent = 0,
				 int maxSecondsToKeepCumulative = 0,
				 double maxDistToKeepCumulative = 0,
				 bool locationDependent = false);
  /// Destructor
  AREXPORT virtual ~ArRangeDeviceThreaded();
  /// The functor you need to implement that will be the one executed by the thread
  AREXPORT virtual void * runThread(void *arg) = 0;
  /// Run in this thread
  AREXPORT virtual void run(void) { myTask.run(); }
  /// Run in its own thread
  AREXPORT virtual void runAsync(void) { myTask.runAsync(); }
  /// Stop the thread
  AREXPORT virtual void stopRunning(void) { myTask.stopRunning(); }
  /// Get the running status of the thread
  AREXPORT virtual bool getRunning(void) { return myTask.getRunning();}
  /// Get the running status of the thread, locking around the variable
  AREXPORT virtual bool getRunningWithLock(void) 
    { return myTask.getRunningWithLock(); }

  AREXPORT virtual int lockDevice(void) { return myTask.lock(); }
  AREXPORT virtual int tryLockDevice(void) { return myTask.tryLock(); }
  AREXPORT virtual int unlockDevice(void) { return myTask.unlock(); }
protected:
  ArRetFunctor1C<void *, ArRangeDeviceThreaded, void *> myRunThreadCB;
  ArFunctorASyncTask myTask;

};

#endif // ARRANGEDEVICETHREADED_H
