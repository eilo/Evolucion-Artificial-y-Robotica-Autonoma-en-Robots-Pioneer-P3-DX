#ifndef ARLASERREFLECTORDEVICE_H
#define ARLASERREFLECTORDEVICE_H

#include "ariaTypedefs.h"
#include "ArRangeDevice.h"
#include "ArFunctor.h"

class ArSick;
class ArRobot;

/// A class for keeping track of laser reflectors that we see right now
/** 
    This class is for showing the laser reflectors in MobileEyes.
    This requires that the range device you pass in uses the
    'extraInt' in the rawReadings ArSensorReading to note reflector
    value and that anything greater than 0 is a reflector.
*/
class ArLaserReflectorDevice : public ArRangeDevice
{
public:
  /// Constructor
  AREXPORT ArLaserReflectorDevice(ArRangeDevice *laser, ArRobot *robot,
				  const char * name = "reflector");
  /// Destructor
  AREXPORT virtual ~ArLaserReflectorDevice();
  /// Grabs the new readings from the robot and adds them to the buffers
  AREXPORT void processReadings(void);
  /// Specifically does nothing since it was done in the constructor
  AREXPORT virtual void setRobot(ArRobot *robot);
  /// Adds a reflector threshold to the task
  AREXPORT void addToConfig(ArConfig *config, const char *section);
protected:
  ArRangeDevice *myLaser;
  int myReflectanceThreshold;
  ArFunctorC<ArLaserReflectorDevice> myProcessCB;
};


#endif // ARLASERREFLECTORDEVICE_H
