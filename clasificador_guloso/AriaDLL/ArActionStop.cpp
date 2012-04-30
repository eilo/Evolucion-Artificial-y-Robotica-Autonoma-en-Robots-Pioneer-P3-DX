#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArActionStop.h"
#include "ArRobot.h"

/**
   @param name name of the action
*/
AREXPORT ArActionStop::ArActionStop(const char *name) :
    ArAction(name, "Stops the robot")
{
}

AREXPORT ArActionStop::~ArActionStop()
{
}

AREXPORT ArActionDesired *ArActionStop::fire(
	ArActionDesired currentDesired)
{
  myDesired.reset();

  myDesired.setVel(0);
  myDesired.setDeltaHeading(0);
  if (myRobot->hasLatVel())
    myDesired.setLeftLatVel(0);

  return &myDesired;
}
