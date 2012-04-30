#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArActionLimiterBackwards.h"
#include "ArRobot.h"

/**
   @param name name of the action
   @param stopDistance distance at which to stop (mm)
   @param slowDistance distance at which to slow down (mm)
   @param maxBackwardsSpeed maximum backwards speed, speed allowed scales
     from this to 0 at the stop distance (mm/sec)
   @param widthRatio The ratio of robot width to the width of the region this action checks for sensor readings. 
*/
AREXPORT ArActionLimiterBackwards::ArActionLimiterBackwards(
	const char *name, double stopDistance, double slowDistance, 
	double maxBackwardsSpeed, double widthRatio) :
  ArAction(name,
	   "Slows the robot down so as not to hit anything in front of it.")
{
  setNextArgument(ArArg("stop distance", &myStopDist, 
			"Distance at which to stop. (mm)"));
  myStopDist = stopDistance;

  setNextArgument(ArArg("slow distance", &mySlowDist, 
			"Distance at which to slow down. (mm)"));
  mySlowDist = slowDistance;

  setNextArgument(ArArg("maximum backwards speed", &myMaxBackwardsSpeed, 
			 "Maximum backwards speed, scales from this to 0 at stopDistance (-mm/sec)"));
  myMaxBackwardsSpeed = maxBackwardsSpeed;

  setNextArgument(ArArg("width ratio", &myWidthRatio, 
			 "The ratio of robot width to how wide an area to check (ratio)"));
  myWidthRatio = widthRatio;
  
}

AREXPORT ArActionLimiterBackwards::~ArActionLimiterBackwards()
{

}

AREXPORT ArActionDesired *
ArActionLimiterBackwards::fire(ArActionDesired currentDesired)
{
  double dist;
  double maxVel;
  
  double slowStopDist = ArUtil::findMax(myStopDist, mySlowDist);
  

  myDesired.reset();
  dist = myRobot->checkRangeDevicesCurrentBox(-myRobot->getRobotLength()/2,
					      -(myRobot->getRobotWidth()/2.0 * 
						myWidthRatio),
			       slowStopDist + (-myRobot->getRobotLength()),
					      (myRobot->getRobotWidth()/2.0 * 
					       myWidthRatio));
  dist -= myRobot->getRobotRadius();
  if (dist < -myStopDist)
  {
    //printf("backwards stop\n");
    myDesired.setMaxNegVel(0);
    return &myDesired;
  }
  if (dist > -mySlowDist)
  {
    //printf("backwards nothing\n");
    myDesired.setMaxNegVel(-ArMath::fabs(myMaxBackwardsSpeed));
    return &myDesired;
  }
      
			
  maxVel = -ArMath::fabs(myMaxBackwardsSpeed) * ((-dist - myStopDist) / (mySlowDist - myStopDist));
  //printf("Neg Max vel %f (stopdist %.1f slowdist %.1f slowspeed %.1f\n", maxVel,	 myStopDist, mySlowDist, myMaxBackwardsSpeed);
  myDesired.setMaxNegVel(maxVel);
  return &myDesired;
  
}
