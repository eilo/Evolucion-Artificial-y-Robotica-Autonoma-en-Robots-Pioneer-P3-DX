#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArRangeDeviceThreaded.h"

AREXPORT ArRangeDeviceThreaded::ArRangeDeviceThreaded(
	size_t currentBufferSize, size_t cumulativeBufferSize,
	const char *name, unsigned int maxRange,
	int maxSecondsToKeepCurrent, int maxSecondsToKeepCumulative,
	double maxDistToKeepCumulative, bool locationDependent) :
  ArRangeDevice(currentBufferSize, cumulativeBufferSize, name, maxRange,
		maxSecondsToKeepCurrent, maxSecondsToKeepCumulative, 
		maxDistToKeepCumulative, locationDependent),
  myRunThreadCB(this, &ArRangeDeviceThreaded::runThread),
  myTask(&myRunThreadCB)
{
  myTask.setThreadName(name);
}

AREXPORT ArRangeDeviceThreaded::~ArRangeDeviceThreaded()
{
}
