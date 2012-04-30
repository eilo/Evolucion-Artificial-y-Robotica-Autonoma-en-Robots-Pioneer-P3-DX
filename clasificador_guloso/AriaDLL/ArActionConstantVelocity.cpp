#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArActionConstantVelocity.h"

/**
   @param name name of the action
   @param velocity velocity to travel at (mm/sec)
*/
AREXPORT ArActionConstantVelocity::ArActionConstantVelocity(const char *name,
						   double velocity) :
  ArAction(name, "Sets the robot to travel straight at a constant velocity.")
{
  setNextArgument(ArArg("velocity", &myVelocity, 
			"The velocity to make the robot travel at. (mm/sec)"));
  myVelocity = velocity;  
}

AREXPORT ArActionConstantVelocity::~ArActionConstantVelocity()
{

}

AREXPORT ArActionDesired *ArActionConstantVelocity::fire(
	ArActionDesired currentDesired)
{
  myDesired.reset();

  myDesired.setVel(myVelocity);
  myDesired.setDeltaHeading(0);

  return &myDesired;
}
