#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArActionRobotJoydrive.h"
#include "ArRobot.h"
#include "ariaInternal.h"
#include "ArCommands.h"

/**
 * @param name Name for this action
   @param requireDeadmanPushed if true the button must be pushed to drive, 
    if false we'll follow the joystick input no matter what
**/

AREXPORT ArActionRobotJoydrive::ArActionRobotJoydrive(
	const char *name, bool requireDeadmanPushed) :
  ArAction(name, "This action reads the joystick on the robot and sets the translational and rotational velocities based on this."),
  myHandleJoystickPacketCB(this, &ArActionRobotJoydrive::handleJoystickPacket),
  myConnectCB(this, &ArActionRobotJoydrive::connectCallback)
{
  myRequireDeadmanPushed = requireDeadmanPushed;
  setNextArgument(ArArg("whether to require the deadman to be pushed or not", &myRequireDeadmanPushed, "If this is true then deadman will need to be pushed to drive, if false we'll drive based on the joystick all the time"));
  myDeadZoneLast = false;
  myHandleJoystickPacketCB.setName("ArActionRobotJoydrive");
}

AREXPORT ArActionRobotJoydrive::~ArActionRobotJoydrive()
{

}

AREXPORT void ArActionRobotJoydrive::setRobot(ArRobot *robot)
{
  ArAction::setRobot(robot);
  if (myRobot != NULL)
  {
    myRobot->addConnectCB(&myConnectCB);
    myRobot->addPacketHandler(&myHandleJoystickPacketCB);
    if (robot->isConnected())
      connectCallback();
  }
}

AREXPORT void ArActionRobotJoydrive::connectCallback(void)
{
  myRobot->comInt(ArCommands::JOYINFO, 2);
}

AREXPORT bool ArActionRobotJoydrive::handleJoystickPacket(
	ArRobotPacket *packet)
{
  if (packet->getID() != 0xF8)
    return false;
  
  myPacketReceivedTime.setToNow();

  myButton1 = packet->bufToUByte();
  myButton2 = packet->bufToUByte();
  myJoyX = packet->bufToUByte2();
  myJoyY = packet->bufToUByte2();
  myThrottle = packet->bufToUByte2();

  //printf("%d %d %d %d %d\n", myButton1, myButton2, myJoyX, myJoyY, myThrottle);
  return true;
}

AREXPORT ArActionDesired *ArActionRobotJoydrive::fire(ArActionDesired currentDesired)
{
  bool printing = false;
  myDesired.reset();
  // if we need the deadman to activate and it isn't pushed just bail
  if (myRequireDeadmanPushed && !myButton1)
  {
    if (printing)
      printf("ArActionRobotJoydrive: Nothing\n");
    myDeadZoneLast = false;
    return NULL;
  }

  // these should vary between 1 and -1
  double ratioRot = -(myJoyX - 512) / 512.0;
  double ratioTrans = (myJoyY - 512) / 512.0;
  double ratioThrottle = myThrottle / 1024.0;
  
  bool doTrans = ArMath::fabs(ratioTrans) > .33;
  bool doRot = ArMath::fabs(ratioRot) > .33;

  if (0)
    printf("%.0f %.0f (x %.3f y %.3f throttle %.3f)\n", ratioTrans * ratioThrottle * 1000,
	   ratioRot * ratioThrottle * 50, ratioTrans, ratioRot, ratioThrottle);
  if (!doTrans && !doRot)
  {
    // if the joystick is in the center, we don't need the deadman,
    // and we were stopped lasttime, then just let other stuff go
    if (myDeadZoneLast && !myRequireDeadmanPushed) 
    {
      if (printing)
	printf("ArActionRobotJoydrive: deadzone Nothing\n");
      return NULL;
    }
    // if the deadman doesn't need to be pushed let something else happen here
    if (printing)
      printf("ArActionRobotJoydrive: deadzone\n");
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
    myDeadZoneLast = true;
    return &myDesired;
  }

  myDeadZoneLast = false;
  // if they have the stick the opposite direction of the velocity
  // then let people crank up the deceleration
  if (doTrans && ((myRobot->getVel() > 0 && ratioTrans < -0.5) || 
		  (myRobot->getVel() < 0 && ratioTrans > 0.5)))
  {
    if (printing)
      printf("ArActionRobotJoydrive: Decelerating trans more\n");
    myDesired.setTransDecel(myRobot->getTransDecel() * 3);
  }

  // if they have the stick the opposite direction of the velocity
  // then let people crank up the deceleration
  if (doRot && ((myRobot->getRotVel() > 0 && ratioRot < -0.5) || 
		  (myRobot->getRotVel() < 0 && ratioRot > 0.5)))
  {
    if (printing)
      printf("ArActionRobotJoydrive: Decelerating rot more\n");
    myDesired.setRotDecel(myRobot->getRotDecel() * 3);
  }

  if (doTrans)
    myDesired.setVel(ratioTrans * ratioThrottle * myRobot->getTransVelMax());
  else
    myDesired.setVel(0);

  printf("%.0f %.0f\n", ratioTrans * ratioThrottle * myRobot->getTransVelMax(),
	 ratioRot * ratioThrottle * myRobot->getRotVelMax());

  
  if (doRot)
    myDesired.setRotVel(ratioRot * ratioThrottle * myRobot->getRotVelMax());
  else
    myDesired.setRotVel(0);

  if(printing)
    printf("ArActionRobotJoydrive: (%ld ms ago) we got %d %d %.2f %.2f %.2f (speed %.0f %.0f)\n", 
	 myPacketReceivedTime.mSecSince(),
	 myButton1, myButton2, ratioTrans, ratioRot, ratioThrottle,
	 myRobot->getVel(), myRobot->getRotVel());
  return &myDesired;
}
