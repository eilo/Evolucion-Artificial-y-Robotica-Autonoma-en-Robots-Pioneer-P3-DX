#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArActionAvoidSide.h"
#include "ArRobot.h"

/**
   @param name name of the action
   @param obstacleDistance distance at which to start avoiding (mm)
   @param turnAmount degrees at which to turn (deg)
*/

AREXPORT ArActionAvoidSide::ArActionAvoidSide(const char *name,
					      double obstacleDistance,
					      double turnAmount) :
  ArAction(name, "Avoids side obstacles, ie walls")
{
  setNextArgument(ArArg("obstacle distance", &myObsDist, 
			"Distance at which to start avoiding (mm)"));
  myObsDist = obstacleDistance;
  setNextArgument(ArArg("turn amount", &myTurnAmount,
			"Degrees at which to turn (deg)"));
  myTurnAmount = turnAmount;

  myTurning = false;

}

AREXPORT ArActionAvoidSide::~ArActionAvoidSide()
{

}

AREXPORT ArActionDesired *ArActionAvoidSide::fire(
	ArActionDesired currentDesired)
{
  double leftDist, rightDist;

  leftDist = (myRobot->checkRangeDevicesCurrentPolar(60, 120) - 
	      myRobot->getRobotRadius());
  rightDist = (myRobot->checkRangeDevicesCurrentPolar(-120, -60) - 
	      myRobot->getRobotRadius());
  
  myDesired.reset();
  if (leftDist < myObsDist)
  {
    myDesired.setDeltaHeading(-myTurnAmount);
    myDesired.setVel(0);
    myTurning = true;
  }
  else if (rightDist < myObsDist)
  {
    myDesired.setDeltaHeading(myTurnAmount);
    myDesired.setVel(0);
    myTurning = true;
  }
  else if (myTurning)
    myDesired.setDeltaHeading(0);

  return &myDesired;
}
