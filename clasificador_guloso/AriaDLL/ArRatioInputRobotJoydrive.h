#ifndef ARRATIOINPUTROBOTJOYDRIVE_H
#define ARRATIOINPUTROBOTJOYDRIVE_H

#include "ariaTypedefs.h"
#include "ArActionRatioInput.h"

class ArRobotPacket;
class ArRobot;
class ArRobotJoyHandler;

/// Use robot's joystick to control an ArActionRatioInput action and drive the robot.
/**
   This class connects the joystick data obtained from  the robot's built
   in joystick port (if it has one, not all robots have a joystick port)
   to an ArActionRatioInput which drives the robot. (See ArRatioInputJoydrive
   for a similar class that uses a joystick plugged in to the computer.)

   A callback is attached to the ArActionRatioInput object which reads joystick
   information using an ArRobotJoyHandler object, and sets requested drive rations on the ArActionRatioInput
   object.


    @sa ArRatioInputJoydrive
    @sa ArActionRatioInput

**/
class ArRatioInputRobotJoydrive 
{
public:
  /// Constructor
  AREXPORT ArRatioInputRobotJoydrive(ArRobot *robot, 
				     ArActionRatioInput *input,
				     int priority = 75,
				     bool requireDeadmanPushed = true);
  /// Destructor
  AREXPORT virtual ~ArRatioInputRobotJoydrive();
protected:
  AREXPORT void fireCallback(void);

  ArRobot *myRobot;
  ArActionRatioInput *myInput;
  bool myRequireDeadmanPushed;
  bool myDeadZoneLast;

  ArRobotJoyHandler *myRobotJoyHandler;
  ArFunctorC<ArRatioInputRobotJoydrive> myFireCB;
};

#endif //ARRATIOINPUTROBOTJOYDRIVE_H
