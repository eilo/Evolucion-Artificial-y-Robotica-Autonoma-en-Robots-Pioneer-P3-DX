#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArSick.h"
#include "ArLaserReflectorDevice.h"
#include "ArRobot.h"

AREXPORT ArLaserReflectorDevice::ArLaserReflectorDevice(ArRangeDevice *laser,
							ArRobot *robot,
							const char *name) :
  /*
  ArRangeDevice(laser->getCurrentRangeBuffer()->getSize(), 
		laser->getCumulativeRangeBuffer()->getSize(), name,
		laser->getMaxRange()), */
  ArRangeDevice(361, 361, name, 32000),
  myProcessCB(this, &ArLaserReflectorDevice::processReadings)
{
  myLaser = laser;
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addSensorInterpTask(myName.c_str(), 10, &myProcessCB);
  setCurrentDrawingData(new ArDrawingData("polyDots", 
                                          ArColor(0xb0, 0xb0, 0xff), 
                                          60,  // mm length of arrow
                                          77,  // above the normal laser
					  200, // default refresh
					  "DefaultOff"), // defaults to off but can be turned on
			true);
  myReflectanceThreshold = 31;
}

AREXPORT ArLaserReflectorDevice::~ArLaserReflectorDevice()
{
  if (myRobot != NULL)
    myRobot->remSensorInterpTask(&myProcessCB);
}

AREXPORT void ArLaserReflectorDevice::setRobot(ArRobot *robot)
{
  // specifically do nothing since this is just here for debugging
}

AREXPORT void ArLaserReflectorDevice::addToConfig(ArConfig *config, 
						  const char *section)
{
  config->addParam(
	  ArConfigArg("ReflectanceThreshold", &myReflectanceThreshold,
		      "The threshold to start showing reflector readings at (normalized from 0 to 255, 31 is the default)", 
		      0, 255),
	  section, ArPriority::DETAILED);
		      
}

AREXPORT void ArLaserReflectorDevice::processReadings(void)
{
  //int i;
  ArSensorReading *reading;
  myLaser->lockDevice();
  lockDevice();
  
  const std::list<ArSensorReading *> *rawReadings;
  std::list<ArSensorReading *>::const_iterator rawIt;
  rawReadings = myLaser->getRawReadings();
  myCurrentBuffer.beginRedoBuffer();

  if (myReflectanceThreshold < 0 || myReflectanceThreshold > 255)
    myReflectanceThreshold = 0;

  if (rawReadings->begin() != rawReadings->end())
  {
    for (rawIt = rawReadings->begin(); rawIt != rawReadings->end(); rawIt++)
    {
      reading = (*rawIt);
      if (!reading->getIgnoreThisReading() && 
	  reading->getExtraInt() > myReflectanceThreshold)
	myCurrentBuffer.redoReading(reading->getPose().getX(), 
				    reading->getPose().getY());
    }
  }

  myCurrentBuffer.endRedoBuffer();

  unlockDevice();
  myLaser->unlockDevice();
}

