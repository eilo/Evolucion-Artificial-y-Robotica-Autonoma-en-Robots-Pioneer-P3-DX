#ifndef ARSIMULATEDLASER_H
#define ARSIMULATEDLASER_H

#include "ariaTypedefs.h"
#include "ArLaser.h"

class ArRobot;
class ArRobotPacket;

/**
   This class is a subclass of ArRangeDeviceThreaded meant for any
   planar scanning lasers, like the SICK lasers, Hokoyo URG series
   lasers, etc.  Unlike most base classes this contains the superset
   of everything that may need to be configured on any of the sensors,
   this is so that the configuration and parameter files don't have to
   deal with anything sensor specific.

   To see the different things you can set on a laser, call the
   functions canSetDegrees, canChooseRange, canSetIncrement,
   canChooseIncrement, canChooseUnits, canChooseReflectorBits,
   canSetPowerControlled, canChooseStartBaud, and canChooseAutoBaud to
   see what is available (the help for each of those tells you what
   functions they are associated with, and for each function
   associated with one of those it tells you to see the associated
   function).
**/

class ArSimulatedLaser : public ArLaser
{
public:
  /// Constructor
  AREXPORT ArSimulatedLaser(ArLaser *laser);
  /// Destructor
  AREXPORT virtual ~ArSimulatedLaser();

  AREXPORT virtual bool blockingConnect(void);
  AREXPORT virtual bool asyncConnect(void);
  AREXPORT virtual bool disconnect(void);
  AREXPORT virtual bool isConnected(void)
    { return myIsConnected; }
  AREXPORT virtual bool isTryingToConnect(void)
    { 
      if (myStartConnect)
	return true;
      else if (myTryingToConnect)
	return true; 
      else
	return false;
    }  

protected:
  AREXPORT virtual void * runThread(void *arg);
  AREXPORT virtual bool laserCheckParams(void);
  AREXPORT bool finishParams(void);
  AREXPORT bool simPacketHandler(ArRobotPacket *packet);
  ArLaser *myLaser;

  double mySimBegin;
  double mySimEnd;
  double mySimIncrement;

  // stuff for the sim packet
  ArPose mySimPacketStart;
  ArTransform mySimPacketTrans;
  ArTransform mySimPacketEncoderTrans;
  unsigned int mySimPacketCounter;
  unsigned int myWhichReading;
  unsigned int myTotalNumReadings;

  bool myStartConnect;
  bool myIsConnected;
  bool myTryingToConnect;

  std::list<ArSensorReading *>::iterator myIter;
  // range buffers to hold current range set and assembling range set
  std::list<ArSensorReading *> *myAssembleReadings;
  std::list<ArSensorReading *> *myCurrentReadings;

  ArRetFunctor1C<bool, ArSimulatedLaser, ArRobotPacket *> mySimPacketHandler;
};

#endif // ARSIMULATEDLASER_H
