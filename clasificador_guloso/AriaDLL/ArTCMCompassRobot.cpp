#include "ariaOSDef.h"
#include "ArExport.h"
#include "ArCommands.h"
#include "ArRobot.h"
#include "ArTCMCompassRobot.h"

AREXPORT ArTCMCompassRobot::ArTCMCompassRobot(ArRobot *robot) :
  myPacketHandlerCB(this, &ArTCMCompassRobot::packetHandler)
{
  myRobot = robot;
  myPacketHandlerCB.setName("ArTCMCompassRobot");
  if (myRobot != NULL)
    myRobot->addPacketHandler(&myPacketHandlerCB);
}

AREXPORT ArTCMCompassRobot::~ArTCMCompassRobot()
{
  if (myRobot != NULL)
    myRobot->remPacketHandler(&myPacketHandlerCB);
}

bool ArTCMCompassRobot::packetHandler(ArRobotPacket *packet)
{
  if (packet->getID() != 0xC0)
    return false;
  
  myHeading = ArMath::fixAngle(packet->bufToByte2() / 10.0);
  myPitch = ArMath::fixAngle(packet->bufToByte2() / 10.0);
  myRoll = ArMath::fixAngle(packet->bufToByte2() / 10.0);
  myXMag = packet->bufToByte2() / 100.0;  
  myYMag = packet->bufToByte2() / 100.0;
  myZMag = packet->bufToByte2() / 100.0;
  myTemperature = packet->bufToByte2() / 10.0;
  myError = packet->bufToByte2();
  myCalibrationH = packet->bufToByte();
  myCalibrationV = packet->bufToByte();
  myCalibrationM = packet->bufToByte2() / 100.0;

  myHaveHeading = 
    myHavePitch = 
    myHaveRoll = 
    myHaveXMag = 
    myHaveYMag = 
    myHaveZMag = 
    myHaveTemperature = 
    myHaveCalibrationH =
    myHaveCalibrationV =
    myHaveCalibrationM = true;

  incrementPacketCount();

  invokeHeadingDataCallbacks(myHeading);
  return true;
}

