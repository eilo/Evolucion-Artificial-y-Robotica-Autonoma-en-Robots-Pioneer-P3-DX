#ifndef ARLASERLOGGER_H
#define ARLASERLOGGER_H

#include <stdio.h>

#include "ariaUtil.h"
#include "ArFunctor.h"

class ArLaser;
class ArRobot;
class ArJoyHandler;
class ArRobotJoyHandler;
class ArRobotPacket;

/// This class can be used to create log files for the laser mapper
/**
   This class has a pointer to a robot and a laser... every time the
   robot has EITHER moved the distDiff, or turned the degDiff, it will
   take the current readings from the laser and log them into the log
   file given as the fileName to the constructor.  Readings can also
   be taken by calling takeReading which explicitly tells the logger
   to take a reading.  

   The class can also add goals, see the constructor arg addGoals for
   information about that... you can also explicitly have it add a
   goal by calling addGoal.

   @see @ref LaserLogFileFormat for details on the laser scan log output file format.
**/
class ArLaserLogger
{
public:
  /// Constructor
  AREXPORT ArLaserLogger(
	  ArRobot *robot, ArLaser *laser, double distDiff, 
	  double degDiff, const char *fileName, 
	  bool addGoals = false, 
	  ArJoyHandler *joyHandler = NULL,
	  const char *baseDirectory = NULL,
	  bool useReflectorValues = false,
	  ArRobotJoyHandler *robotJoyHandler = NULL,
	  const std::map<std::string, ArRetFunctor2<int, ArTime, ArPose *> *, 
	  ArStrCaseCmpOp> *extraLocationData = NULL);
  /// Destructor
  AREXPORT virtual ~ArLaserLogger();

#ifndef SWIG
  /** @brief Adds a string to the log file with a tag at the given moment
   *  @swigomit
   */
  AREXPORT void addTagToLog(const char *str, ...);
#endif

  /// Same ass addToLog, but no varargs, wrapper for java
  AREXPORT void addTagToLogPlain(const char *str);

#ifndef SWIG
  /** @brief Adds a string to the log file without a tag for where or when we are
   *  @swigomit
   */
  AREXPORT void addInfoToLog(const char *str, ...);
#endif

  /// Same as addToInfo, but does it without marking robot position
  AREXPORT void addInfoToLogPlain(const char *str);
  /// Sets the distance at which the robot will take a new reading
  void setDistDiff(double distDiff) { myDistDiff = ArMath::fabs(distDiff); }
  /// Gets the distance at which the robot will take a new reading
  double getDistDiff(void) { return myDistDiff; }
  /// Sets the degrees to turn at which the robot will take a new reading
  void setDegDiff(double degDiff) { myDistDiff = ArMath::fabs(degDiff); }
  /// Gets the degrees to turn at which the robot will take a new reading
  double getDegDiff(void) { return myDegDiff; }
  /// Explicitly tells the robot to take a reading
  void takeReading(void) { myTakeReadingExplicit = true; }
  /// Adds a goal where the robot is at the moment
  void addGoal(void) { myAddGoalExplicit = true; }
  /// Sees if the file was opened successfully
  bool wasFileOpenedSuccessfully(void) 
    { if (myFile != NULL) return true; else return false; }
  /// Gets if we're taking old (sick1:) readings
  bool takingOldReadings(void) { return myOldReadings; }
  /// Sets if we're taking old (sick1:) readings
  void takeOldReadings(bool takeOld) { myOldReadings = takeOld; }
  /// Gets if we're taking new (scan1:) readings
  bool takingNewReadings(void) { return myNewReadings; }
  /// Sets if we're taking old (scan1:) readings
  void takeNewReadings(bool takeNew) { myNewReadings = takeNew; }
protected:
  /// The task which gets attached to the robot
  AREXPORT void robotTask(void);
  // internal function that adds goals if needed (and specified)
  void internalAddGoal(void);
  // internal function that writes tags
  void internalWriteTags(void);
  // internal function that takes a reading
  void internalTakeReading(void);
  // internal function that prints the position
  void internalPrintPos(ArPose encoderPoseTaken, ArPose goalPoseTaken, 
			ArTime timeTaken);
  // internal packet for handling the loop packets
  AREXPORT bool loopPacketHandler(ArRobotPacket *packet);


  // what type of readings we are taking
  bool myOldReadings;
  // what type of readings we are taking
  bool myNewReadings;
  // if we're taking reflector values too
  bool myUseReflectorValues;
  std::list<std::string> myTags;
  std::list<std::string> myInfos;
  bool myWrote;
  ArRobot *myRobot;
  ArLaser *myLaser;
  bool myAddGoals;
  ArJoyHandler *myJoyHandler;
  ArRobotJoyHandler *myRobotJoyHandler;
  std::string myFileName;
  std::string myBaseDirectory;
  FILE *myFile;
  bool myFirstTaken;

  ArPose myLast;
  double myLastVel;
  double myDistDiff;
  double myDegDiff;
  ArSectors mySectors;
  ArFunctorC<ArLaserLogger> myTaskCB;
  int myScanNumber;
  ArTime myStartTime;
  bool myTakeReadingExplicit;
  bool myAddGoalExplicit;
  bool myAddGoalKeyboard;
  bool myLastAddGoalKeyboard;
  bool myLastJoyButton;
  bool myLastRobotJoyButton;
  bool myFirstGoalTaken;
  int myNumGoal;
  ArPose myLastGoalTakenPose;
  ArTime myLastGoalTakenTime;

  void goalKeyCallback(void);
  unsigned char myLastLoops;
  
  bool myFlipped;
  
  bool myIncludeRawEncoderPose;
  std::map<std::string, ArRetFunctor2<int, ArTime, ArPose *> *, 
	   ArStrCaseCmpOp> myExtraLocationData;

  ArFunctorC<ArLaserLogger> myGoalKeyCB;
  ArRetFunctor1C<bool, ArLaserLogger, ArRobotPacket *> myLoopPacketHandlerCB;
};

typedef ArLaserLogger ArSickLogger;

#endif // ARLASERLOGGER_H
