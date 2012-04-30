#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArRatioInputJoydrive.h"
#include "ArRobot.h"
#include "ariaInternal.h"

/**
   @param robot robot
   @param input Action to attach to and use to drive the robot. 
   @param priority Priority of this joystick input handler with respect to other input
   objects attached to the @a input action object.
   @param stopIfNoButtonPressed if this is true and there is a
   joystick and no button is pressed, we cause the action to stop the robot.
   .. otherwise it'll do nothing (letting lower priority actions
   fire)

   @param useOSCalForJoystick if this is true we'll use the OS
   calibration, if false we'll do our own.  See also ArJoyHandler::setUseOSCal()
**/

AREXPORT ArRatioInputJoydrive::ArRatioInputJoydrive(
	ArRobot *robot,
	ArActionRatioInput *input,
	int priority,
	bool stopIfNoButtonPressed,
	bool useOSCalForJoystick) :
  myFireCB(this, &ArRatioInputJoydrive::fireCallback)
{
  myRobot = robot;
  myInput = input;
  myInput->addFireCallback(priority, &myFireCB);
  myFireCB.setName("Joydrive");
  if ((myJoyHandler = Aria::getJoyHandler()) == NULL)
  {
    myJoyHandler = new ArJoyHandler;
    myJoyHandler->init();
    Aria::setJoyHandler(myJoyHandler);
  }

  myUseOSCal = useOSCalForJoystick;
  myPreviousUseOSCal = myUseOSCal;
  myStopIfNoButtonPressed = stopIfNoButtonPressed;
  myFiredLast = false;
  myPrinting = false;
}

AREXPORT ArRatioInputJoydrive::~ArRatioInputJoydrive()
{
  myInput->remFireCallback(&myFireCB);
}

AREXPORT void ArRatioInputJoydrive::setStopIfNoButtonPressed(
	bool stopIfNoButtonPressed)
{
  myStopIfNoButtonPressed = stopIfNoButtonPressed;
}

AREXPORT bool ArRatioInputJoydrive::getStopIfNoButtonPressed(void)
{
  return myStopIfNoButtonPressed;
}

AREXPORT bool ArRatioInputJoydrive::joystickInited(void)
{
  return myJoyHandler->haveJoystick();
}

/**
   @see ArJoyHandler::setUseOSCal
**/
AREXPORT void ArRatioInputJoydrive::setUseOSCal(bool useOSCal)
{
  myUseOSCal = useOSCal;
  myPreviousUseOSCal = useOSCal;
  myJoyHandler->setUseOSCal(useOSCal);
}

/**
   @see ArJoyHandler::getUseOSCal
**/
AREXPORT bool ArRatioInputJoydrive::getUseOSCal(void)
{
  return myUseOSCal;
}


void ArRatioInputJoydrive::fireCallback(void)
{
  double rot, trans, throttle;

  if (myPreviousUseOSCal != myUseOSCal)
  {
    myJoyHandler->setUseOSCal(myUseOSCal);
    myPreviousUseOSCal = myUseOSCal;
  }

  if (myJoyHandler->haveJoystick() && myJoyHandler->getButton(1))
  {
    // get the readings from the joystick
    myJoyHandler->getDoubles(&rot, &trans);
    
    if (!myJoyHandler->haveZAxis()) 
    {
      throttle = 1;
    }
    // if we are using the throttle, interpolate its position between
    // low and high throttle values
    else
    {
      throttle = myJoyHandler->getAxis(3);
      throttle += 1.0;
      throttle /= 2.0;
    }
    myInput->setRatios(trans * 100, -rot * 100, throttle * 100);
    myFiredLast = true;
    if (myPrinting)
      printf("joy %g %g %g\n", trans * 100, -rot * 100, throttle * 100);
  }
  else if (myJoyHandler->haveJoystick() && (myStopIfNoButtonPressed ||
					    myFiredLast))
  {
    if (myPrinting)
      printf("joy nothing\n");
    myFiredLast = false;
    myInput->setRatios(0, 0, myInput->getThrottleRatio());
  }
  
}
