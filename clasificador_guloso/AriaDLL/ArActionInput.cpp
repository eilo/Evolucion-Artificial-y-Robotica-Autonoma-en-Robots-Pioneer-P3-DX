#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArActionInput.h"
#include "ArRobot.h"
/**
   @param name name of the action
*/
AREXPORT ArActionInput::ArActionInput(const char *name) :
    ArAction(name, "Inputs vel and heading")
{
  clear();
}

AREXPORT ArActionInput::~ArActionInput()
{
}

AREXPORT void ArActionInput::setVel(double vel)
{
  myUsingVel = true;
  myVelSet = vel;
}

AREXPORT void ArActionInput::setRotVel(double rotVel)
{
  myRotRegime = ROTVEL;
  myRotVal = rotVel;
}

AREXPORT void ArActionInput::deltaHeadingFromCurrent(double delta)
{
  myRotRegime = DELTAHEADING;
  myRotVal = delta;
}

AREXPORT void ArActionInput::setHeading(double heading)
{
  myRotRegime = SETHEADING;
  myRotVal = heading;
}

AREXPORT void ArActionInput::clear(void)
{
  myUsingVel = false;
  myRotRegime = NONE;
}

AREXPORT ArActionDesired *ArActionInput::fire(
	ArActionDesired currentDesired)
{
  myDesired.reset();

  if (myUsingVel)
    myDesired.setVel(myVelSet);
  
  if (myRotRegime == ROTVEL)
    myDesired.setRotVel(myRotVal);
  else if (myRotRegime == DELTAHEADING)
  {
    myDesired.setDeltaHeading(myRotVal);
    myRotVal = 0;
  }
  else if (myRotRegime == SETHEADING)
    myDesired.setHeading(myRotVal);
  else if (myRotRegime != NONE)
    ArLog::log(ArLog::Normal, "ArActionInput::fire: Bad rot regime %d", 
	       myRotRegime);

  return &myDesired;
}
