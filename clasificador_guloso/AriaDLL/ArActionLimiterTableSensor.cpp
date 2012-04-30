#include "ArExport.h"

#include "ariaOSDef.h"
#include "ArActionLimiterTableSensor.h"
#include "ArRobot.h"

AREXPORT ArActionLimiterTableSensor::ArActionLimiterTableSensor(
	const char *name) :
  ArAction(name, "Limits speed to 0 if a table is seen")
{

}

AREXPORT ArActionLimiterTableSensor::~ArActionLimiterTableSensor()
{
}

AREXPORT ArActionDesired *ArActionLimiterTableSensor::fire(
	ArActionDesired currentDesired)
{
  myDesired.reset();

  if (myRobot->hasTableSensingIR() && 
      ((myRobot->isLeftTableSensingIRTriggered()) ||
      (myRobot->isRightTableSensingIRTriggered())))
  {
    myDesired.setMaxVel(0);
    return &myDesired;
  }
  return NULL;  
}
