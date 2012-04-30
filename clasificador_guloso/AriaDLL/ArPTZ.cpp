#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArPTZ.h"
#include "ArRobot.h"
#include "ArRobotPacket.h"
#include "ArCommands.h"
#include "ArDeviceConnection.h"

/**
   @param robot The robot this camera is attached to, can be NULL
**/
AREXPORT ArPTZ::ArPTZ(ArRobot *robot) :
  myConnectCB(this, &ArPTZ::connectHandler),
  mySensorInterpCB(this, &ArPTZ::sensorInterpHandler),
  myRobotPacketHandlerCB(this, &ArPTZ::robotPacketHandler)
{
  myRobot = robot;
  myConn = NULL;
  myAuxPort = 1;
  myAuxTxCmd = ArCommands::TTY2;
  myAuxRxCmd = ArCommands::GETAUX;
  myRobotPacketHandlerCB.setName("ArPTZ");
  if (myRobot != NULL)
  {
    myRobot->addConnectCB(&myConnectCB, ArListPos::LAST);
    myRobot->addPacketHandler(&myRobotPacketHandlerCB, ArListPos::FIRST);
  }
}

AREXPORT ArPTZ::~ArPTZ()
{
  if (myRobot != NULL)
  {
    myRobot->remConnectCB(&myConnectCB);
    myRobot->remPacketHandler(&myRobotPacketHandlerCB);
    myRobot->remSensorInterpTask(&mySensorInterpCB);
  }
  
}

/**
   @param packet the packet to send
   @return true if the packet could be sent, false otherwise
**/
   
AREXPORT bool ArPTZ::sendPacket(ArBasePacket *packet)
{
  packet->finalizePacket();
  if (myConn != NULL)
    return myConn->write(packet->getBuf(), packet->getLength());
  else if (myRobot != NULL)
    return myRobot->comStrN(myAuxTxCmd, packet->getBuf(), 
			    packet->getLength());
  else
    return false;
}

AREXPORT bool ArPTZ::robotPacketHandler(ArRobotPacket *packet)
{
  //printf("%x\n", packet->getID());
  if ((packet->getID() == 0xb0 && myAuxPort == 1) ||
      (packet->getID() == 0xb8 && myAuxPort == 2))
    return packetHandler(packet);
  else
    return false;
}

AREXPORT void ArPTZ::connectHandler(void)
{
  init();
}

AREXPORT void ArPTZ::sensorInterpHandler(void)
{
  ArBasePacket *packet;
  while ((packet = readPacket()) != NULL)
    packetHandler(packet);
}

/**
   @param connection the device connection the camera is connected to,
   normally a serial port
   
   @param driveFromRobotLoop if this is true then a sensor interp
   callback wil be set and that callback will read packets and call
   the packet handler on them

   @return true if the serial port is opened or can be opened, false
   otherwise
**/
AREXPORT bool ArPTZ::setDeviceConnection(ArDeviceConnection *connection,
					 bool driveFromRobotLoop)
{
  if (myRobot != NULL)
  {
    myRobot->remPacketHandler(&myRobotPacketHandlerCB);
    myRobot->remSensorInterpTask(&mySensorInterpCB);
  }
  if (myRobot == NULL)
    return false;
  myConn = connection;
  if (driveFromRobotLoop && myRobot != NULL && myConn != NULL)
    myRobot->addSensorInterpTask("ptz", 50, &mySensorInterpCB);
  if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN)
    return myConn->openSimple();
  else
    return true;
}

AREXPORT ArDeviceConnection *ArPTZ::getDeviceConnection(void)
{
  return myConn;
}


/**
 @param auxPort The AUX port on the robot's microcontroller that the device
 is connected to.  The C166 controller only has one port.  The H8 has two.

 @return true if the port was valid (1 or 2).  False otherwise.

**/
AREXPORT bool ArPTZ::setAuxPort(int auxPort)
{
  if (auxPort == 1)
  {
    myAuxTxCmd = ArCommands::TTY2;
    myAuxRxCmd = ArCommands::GETAUX;
    myAuxPort = 1;
    return true;
  }
  else if (auxPort == 2)
  {
    myAuxTxCmd = ArCommands::TTY3;
    myAuxRxCmd = ArCommands::GETAUX2;
    myAuxPort = 2;
    return true;
  }
  else
    return false;
}

