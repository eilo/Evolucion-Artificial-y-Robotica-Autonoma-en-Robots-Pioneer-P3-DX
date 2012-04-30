#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArLaserConnector.h"
#include "ArRobot.h"
#include "ArLaser.h"
#include "ariaInternal.h"
#include "ArSick.h"
#include "ArUrg.h"
#include "ArSimulatedLaser.h"
#include "ArCommands.h"
#include "ArRobotConfigPacketReader.h"

/** @warning do not delete @a parser during the lifetime of this
 ArLaserConnector, which may need to access its contents later.

 @param parser the parser with the arguments to parse 

 @param robot the robot these lasers are attached to (or NULL for none)

 @param robotConnector the connector used for connecting to the robot
 (so we can see if it was a sim or not)

 @param autoParseArgs if this class should autoparse the args if they
 aren't parsed explicitly

 @param infoLogLevel The log level for information about creating
 lasers and such, this is also passed to all the lasers created as
 their infoLogLevel too
 */
AREXPORT ArLaserConnector::ArLaserConnector(
	ArArgumentParser *parser, ArRobot *robot,
	ArRobotConnector *robotConnector, bool autoParseArgs,
	ArLog::LogLevel infoLogLevel) :
  myParseArgsCB(this, &ArLaserConnector::parseArgs),
  myLogOptionsCB(this, &ArLaserConnector::logOptions)
{
  myParser = parser;
  myOwnParser = false;
  myRobot = robot;
  myRobotConnector = robotConnector;
  myAutoParseArgs = autoParseArgs;
  myParsedArgs = false;
  myInfoLogLevel = infoLogLevel;

  myParseArgsCB.setName("ArLaserConnector");
  Aria::addParseArgsCB(&myParseArgsCB, 60);
  myLogOptionsCB.setName("ArLaserConnector");
  Aria::addLogOptionsCB(&myLogOptionsCB, 80);
}

AREXPORT ArLaserConnector::~ArLaserConnector(void)
{

}


/**
 * Parse command line arguments using the ArArgumentParser given in the ArLaserConnector constructor.
 *
 * See parseArgs(ArArgumentParser*) for details about argument parsing.
 * 
  @return true if the arguments were parsed successfully false if not
 **/

AREXPORT bool ArLaserConnector::parseArgs(void)
{
  return parseArgs(myParser);
}

/**
 * Parse command line arguments held by the given ArArgumentParser.
 *
  @return true if the arguments were parsed successfully false if not

   The following arguments are used for the robot connection:

   <dl>
    <dt><code>-robotPort</code> <i>port</i></dt>
    <dt><code>-rp</code> <i>port</i></dt>
    <dd>Use the given serial port device name for a serial port connection (e.g. <code>COM1</code>, or <code>/dev/ttyS0</code> if on Linux.)
    The default is the first serial port, or COM1, which is the typical Pioneer setup.
    </dd>

  The following arguments are accepted for laser connections.  A program may request support for more than one laser
  using setMaxNumLasers(); if multi-laser support is enabled in this way, then these arguments must have the laser index
  number appended. For example, "-laserPort" for laser 1 would instead by "-laserPort1", and for laser 2 it would be
  "-laserPort2".

  <dl>
    <dt>-laserPort <i>port</i></dt>
    <dt>-lp <i>port</i></dt>
    <dd>Use the given port device name when connecting to a laser. For example, <code>COM2</code> or on Linux, <code>/dev/ttyS1</code>.
    The default laser port is COM2, which is the typical Pioneer laser port setup.
    </dd>

    <dt>-laserFlipped <i>true|false</i></dt>
    <dt>-lf <i>true|false</i></dt>
    <dd>If <code>true</code>, then the laser is mounted upside-down on the robot and the ordering of readings
    should be reversed.</dd>

    <dt>-connectLaser</dt>
    <dt>-cl</dt>
    <dd>Explicitly request that the client program connect to a laser, if it does not always do so</dd>

    <dt>-laserPowerControlled <i>true|false</i></dt>
    <dt>-lpc <i>true|false</i></dt>
    <dd>If <code>true</code>, then the laser is powered on when the serial port is initially opened, so enable
    certain features when connecting such as a waiting period as the laser initializes.</dd>

    <dt>-laserDegrees <i>degrees</i></dt>
    <dt>-ld <i>degrees</i></dt>
    <dd>Indicate the size of the laser field of view, either <code>180</code> (default) or <code>100</code>.</dd>

    <dt>-laserIncrement <i>increment</i></dt>
    <dt>-li <i>increment</i></dt>
    <dd>Configures the laser's angular resolution. If <code>one</code>, then configure the laser to take a reading every degree.
     If <code>half</code>, then configure it for a reading every 1/2 degrees.</dd>

    <dt>-laserUnits <i>units</i></dt>
    <dt>-lu <i>units</i></dt>
    <dd>Configures the laser's range resolution.  May be 1mm for one milimiter, 1cm for ten milimeters, or 10cm for one hundred milimeters.</dd>

    <dt>-laserReflectorBits <i>bits</i></dt>
    <dt>-lrb <i>bits</i></dt>
    <dd>Enables special reflectance detection, and configures the granularity of reflector detection information. Using more bits allows the laser to provide values for several different
    reflectance levels, but also may force a reduction in range.  (Note, the SICK LMS-200 only detects high reflectance on special reflector material
    manufactured by SICK.)
    </dd>
  </dl>

 **/

AREXPORT bool ArLaserConnector::parseArgs(ArArgumentParser *parser)
{
  if (myParsedArgs)
    return true;

  myParsedArgs = true;

  bool typeReallySet;
  const char *type;
  char buf[1024];
  int i;
  std::map<int, LaserData *>::iterator it;
  LaserData *laserData;

  bool wasReallySetOnlyTrue = parser->getWasReallySetOnlyTrue();
  parser->setWasReallySetOnlyTrue(true);

  for (i = 1; i <= Aria::getMaxNumLasers(); i++)
  {
    if (i == 1)
      buf[0] = '\0';
    else
      sprintf(buf, "%d", i);
    
    typeReallySet = false;
    
    // see if the laser is being added from the command line
    if (!parser->checkParameterArgumentStringVar(&typeReallySet, &type,
						 "-laserType%s", buf) ||
	!parser->checkParameterArgumentStringVar(&typeReallySet, &type,
						 "-lt%s", buf))
    {
      ArLog::log(ArLog::Normal, 
		 "ArLaserConnector: Bad laser type given for laser number %d", 
		 i);
      parser->setWasReallySetOnlyTrue(wasReallySetOnlyTrue);
      return false;
    }

    // if we didn't have an argument then just return
    if (!typeReallySet)
      continue;

    if ((it = myLasers.find(i)) != myLasers.end())
    {
      ArLog::log(ArLog::Normal, "ArLaserConnector: A laser already exists for laser number %d, replacing it with a new one of type %s", 
		 i, type);
      laserData = (*it).second;   
      delete laserData;
      myLasers.erase(i);
    }

    if (typeReallySet && type != NULL)
    {
      ArLaser *laser = NULL;
      if ((laser = Aria::laserCreate(type, i, "ArLaserConnector: ")) != NULL)
      {
	ArLog::log(myInfoLogLevel, 
		   "ArLaserConnector: Created %s as laser %d from arguments",
		   laser->getName(), i);
	myLasers[i] = new LaserData(i, laser);
	laser->setInfoLogLevel(myInfoLogLevel);
      }
      else
      {
	ArLog::log(ArLog::Normal, 
		   "Unknown laser type %s for laser %d, choices are %s", 
		   type, i, Aria::laserGetTypes());
	parser->setWasReallySetOnlyTrue(wasReallySetOnlyTrue);
	return false;
      }
    }
  }

  // go through the robot param list and add the lasers defined
  // in the parameter file.
  const ArRobotParams *params = NULL;
  if (myRobot != NULL)
  {
    params = myRobot->getRobotParams();
    if (params != NULL)
    {

      for (i = 1; i <= Aria::getMaxNumLasers(); i++)
      {
	// if we already have a laser for this then don't add one from
	// the param file, since it was added either explicitly by a
	// program or from the command line
	if (myLasers.find(i) != myLasers.end())
	  continue;
	
	type = params->getLaserType(i);

	// if we don't have a laser type for that number continue
	if (type == NULL || type[0] == '\0')
	  continue;

	ArLaser *laser = NULL;
	if ((laser = 
	     Aria::laserCreate(type, i, "ArLaserConnector: ")) != NULL)
	{
	  ArLog::log(myInfoLogLevel, 
	     "ArLaserConnector: Created %s as laser %d from parameter file",
		     laser->getName(), i);
	  myLasers[i] = new LaserData(i, laser);
	  laser->setInfoLogLevel(myInfoLogLevel);
	}
	else
	{
	  ArLog::log(ArLog::Normal, 
		     "Unknown laser type %s for laser %d from the .p file, choices are %s", 
		     type, i, Aria::laserGetTypes());
	  parser->setWasReallySetOnlyTrue(wasReallySetOnlyTrue);
	  return false;
	}
      }
    }
    else
    {
      ArLog::log(ArLog::Normal, "ArLaserConnector: Have robot, but robot has NULL params, so cannot configure its laser");
    }
  }

  // now go through and parse the args for any laser that we have
  for (it = myLasers.begin(); it != myLasers.end(); it++)
  {
    laserData = (*it).second;
    if (!parseLaserArgs(parser, laserData))
    {
      parser->setWasReallySetOnlyTrue(wasReallySetOnlyTrue);
      return false;
    }
  }

  parser->setWasReallySetOnlyTrue(wasReallySetOnlyTrue);
  return true;
}

AREXPORT bool ArLaserConnector::parseLaserArgs(ArArgumentParser *parser, 
						LaserData *laserData)
{
  char buf[512];


  if (laserData == NULL)
  {
    ArLog::log(ArLog::Terse, "Was given NULL laser");
    return false;
  }

  if (laserData->myLaser == NULL)
  {
    ArLog::log(ArLog::Normal, 
	       "ArLaserConnector: There is no laser for laser number %d but there should be", 
	       laserData->myNumber);
    return false;
  }

  ArLaser *laser = laserData->myLaser;

  if (laserData->myNumber == 1)
    buf[0] = '\0';
  else
    sprintf(buf, "%d", laserData->myNumber);

  // see if we want to connect to the laser automatically
  if (parser->checkArgumentVar("-connectLaser%s", buf) || 
      parser->checkArgumentVar("-cl%s", buf))
  {
    laserData->myConnect = true;
    laserData->myConnectReallySet = true;
  }

  // see if we do not want to connect to the laser automatically
  if (parser->checkArgumentVar("-doNotConnectLaser%s", buf) || 
      parser->checkArgumentVar("-dncl%s", buf))
  {
    laserData->myConnect = false;
    laserData->myConnectReallySet = true;
  }

  if (!parser->checkParameterArgumentStringVar(NULL, &laserData->myPort,
					       "-lp%s", buf) ||
      !parser->checkParameterArgumentStringVar(NULL, &laserData->myPort, 
					       "-laserPort%s", buf) ||
      

      !parser->checkParameterArgumentStringVar(NULL, &laserData->myPortType,
					    "-lpt%s", buf) ||
      !parser->checkParameterArgumentStringVar(NULL, &laserData->myPortType, 
					       "-laserPortType%s", buf) ||


      !parser->checkParameterArgumentIntegerVar(
	      &laserData->myRemoteTcpPortReallySet,
	      &laserData->myRemoteTcpPort,
	      "-rltp%s", buf) ||
      !parser->checkParameterArgumentIntegerVar(
	      &laserData->myRemoteTcpPortReallySet,
	      &laserData->myRemoteTcpPort, 
	      "-remoteLaserTcpPort%s", buf) ||

      !parser->checkParameterArgumentBoolVar(&laserData->myFlippedReallySet,
					     &laserData->myFlipped,
					     "-lf%s", buf) ||
      !parser->checkParameterArgumentBoolVar(&laserData->myFlippedReallySet,
					     &laserData->myFlipped,
					     "-laserFlipped%s", buf) ||


      (laser->canSetDegrees() && 
       !parser->checkParameterArgumentDoubleVar(
	       &laserData->myDegreesStartReallySet, &laserData->myDegreesStart,
	       "-lds%s", buf)) ||
      (laser->canSetDegrees() && 
       !parser->checkParameterArgumentDoubleVar(
	       &laserData->myDegreesStartReallySet, &laserData->myDegreesStart,
	       "-laserDegreesStart%s", buf)) ||
      (laser->canSetDegrees() && 
       !parser->checkParameterArgumentDoubleVar(
	       &laserData->myDegreesEndReallySet, &laserData->myDegreesEnd,
	       "-lde%s", buf)) ||
      (laser->canSetDegrees() && 
       !parser->checkParameterArgumentDoubleVar(
	       &laserData->myDegreesEndReallySet, &laserData->myDegreesEnd,
						"-laserDegreesEnd%s", buf)) ||
      
      
      (laser->canChooseDegrees() && 
      !parser->checkParameterArgumentStringVar(NULL, &laserData->myDegrees,
					       "-ld%s", buf)) ||
      (laser->canChooseDegrees() && 
       !parser->checkParameterArgumentStringVar(NULL, &laserData->myDegrees,
						"-laserDegrees%s", buf)) ||

      (laser->canSetIncrement() && 
       !parser->checkParameterArgumentDoubleVar(
	       &laserData->myIncrementByDegreesReallySet,
	       &laserData->myIncrementByDegrees,
	       "-libd%s", buf)) ||
      (laser->canSetIncrement() && 
       !parser->checkParameterArgumentDoubleVar(
	       &laserData->myIncrementByDegreesReallySet,
	       &laserData->myIncrementByDegrees,
	       "-laserIncrementByDegrees%s", buf)) ||


      (laser->canChooseIncrement() &&
       !parser->checkParameterArgumentStringVar(NULL, &laserData->myIncrement,
						"-li%s", buf)) ||
      (laser->canChooseIncrement() && 
       !parser->checkParameterArgumentStringVar(NULL, &laserData->myIncrement,
						"-laserIncrement%s", buf)) ||

      (laser->canChooseUnits() && 
       !parser->checkParameterArgumentStringVar(NULL, &laserData->myUnits,
						"-lu%s", buf)) ||
      (laser->canChooseUnits() && 
       !parser->checkParameterArgumentStringVar(NULL, &laserData->myUnits,
						"-laserUnits%s", buf)) ||
      
      (laser->canChooseReflectorBits() && 
       !parser->checkParameterArgumentStringVar(
	       NULL, &laserData->myReflectorBits,
	       "-lrb%s", buf)) ||
      (laser->canChooseReflectorBits() && 
       !parser->checkParameterArgumentStringVar(
	       NULL, &laserData->myReflectorBits, 
	       "-laserReflectorBits%s", buf)) ||

      (laser->canSetPowerControlled() && 
       !parser->checkParameterArgumentBoolVar(
	       &laserData->myPowerControlledReallySet,
	       &laserData->myPowerControlled,
	       "-lpc%s", buf))  ||
      (laser->canSetPowerControlled() && 
       !parser->checkParameterArgumentBoolVar(
	       &laserData->myPowerControlledReallySet,
	       &laserData->myPowerControlled,
	       "-laserPowerControlled%s", buf)) ||

      (laser->canChooseStartingBaud() && 
       !parser->checkParameterArgumentStringVar(
	       NULL, &laserData->myStartingBaud,
	       "-lsb%s", buf)) ||
      (laser->canChooseStartingBaud() && 
       !parser->checkParameterArgumentStringVar(
	       NULL, &laserData->myStartingBaud, 
	       "-laserStartingBaud%s", buf)) ||

      (laser->canChooseAutoBaud() && 
       !parser->checkParameterArgumentStringVar(
	       NULL, &laserData->myAutoBaud,
	       "-lab%s", buf)) ||
      (laser->canChooseAutoBaud() && 
       !parser->checkParameterArgumentStringVar(
	       NULL, &laserData->myAutoBaud, 
	       "-laserAutoBaud%s", buf)) ||

      !parser->checkParameterArgumentStringVar(
	       NULL, &laserData->myAdditionalIgnoreReadings,
	       "-lair%s", buf) ||
      !parser->checkParameterArgumentStringVar(
	      NULL, &laserData->myAdditionalIgnoreReadings, 
	      "-laserAdditionalIgnoreReadings%s", buf)

      )
  {
    return false;
  }

  return internalConfigureLaser(laserData);
}

bool ArLaserConnector::internalConfigureLaser(
	LaserData *laserData)
{
  ArLaser *laser = laserData->myLaser;
  
  if (laser == NULL)
  {
    ArLog::log(ArLog::Terse, "ArLaserConnector: No laser for number %d",
	       laserData->myNumber);
    return false;
  }

  if (laserData->myMaxRangeReallySet)
    laser->setMaxRange(laserData->myMaxRange);

  if (laserData->myFlippedReallySet && 
      !laser->setFlipped(laserData->myFlipped))
    return false;

  if (laser->canSetDegrees() && 
      laserData->myDegreesStartReallySet && 
      !laser->setStartDegrees(laserData->myDegreesStart))
    return false;

  if (laser->canSetDegrees() && 
      laserData->myDegreesEndReallySet && 
      !laser->setEndDegrees(laserData->myDegreesEnd))
    return false;

  if (laser->canChooseDegrees() && laserData->myDegrees != NULL &&
      !laser->chooseDegrees(laserData->myDegrees))
    return false;

  if (laser->canSetIncrement() && 
      laserData->myIncrementByDegreesReallySet && 
      !laser->setIncrement(laserData->myIncrementByDegrees))
    return false;

  if (laser->canChooseIncrement() && laserData->myIncrement != NULL &&
      !laser->chooseIncrement(laserData->myIncrement))
    return false;

  if (laser->canChooseUnits() && laserData->myUnits != NULL &&
      !laser->chooseUnits(laserData->myUnits))
    return false;

  if (laser->canChooseReflectorBits() && laserData->myReflectorBits != NULL &&
      !laser->chooseReflectorBits(laserData->myReflectorBits))
    return false;

  if (laser->canSetPowerControlled() && 
      laserData->myPowerControlledReallySet && 
      !laser->setPowerControlled(laserData->myPowerControlled))
    return false;

  if (laser->canChooseStartingBaud() && laserData->myStartingBaud != NULL &&
      !laser->chooseStartingBaud(laserData->myStartingBaud))
    return false;

  if (laser->canChooseAutoBaud() && laserData->myAutoBaud != NULL &&
      !laser->chooseAutoBaud(laserData->myAutoBaud))
    return false;

  if (laserData->myAdditionalIgnoreReadings != NULL && 
      !laser->addIgnoreReadings(laserData->myAdditionalIgnoreReadings))
    return false;

  // if this is a placeholder, don't do the device connection stuff since we need to set it on the real laser
  if (laserData->myLaserIsPlaceholder)
  {
    return true;
  }
  
  // the rest handles all the connection stuff
  const ArRobotParams *params;

  char portBuf[1024];
  if (laserData->myLaser == NULL)
  {
    ArLog::log(ArLog::Terse, "ArLaserConnector: There is no laser, cannot connect");
    return false;
  }
  sprintf(portBuf, "%d", laserData->myLaser->getDefaultTcpPort());

  if (myRobotConnector == NULL)
  {
    ArLog::log(ArLog::Terse, "ArLaserConnector: No ArRobotConnector is passed in so simulators and remote hosts will not work correctly");
  }

  /*
    if a laser isn't a placeholder and we should be using the sim, then use it
  */
  if (!laserData->myLaserIsPlaceholder && 
      myRobotConnector != NULL && myRobotConnector->getRemoteIsSim())
  {
    if (laserData->myNumber != 1)
    {
      ArLog::log(ArLog::Normal, "Cannot use the simulator with multiple lasers yet, will continue but will be unable to connect laser %s (num %d)", laserData->myLaser->getName(), laserData->myNumber);
      return true;
    }
    ArSick *sick = NULL;
    if ((sick = dynamic_cast<ArSick *>(laser)) != NULL)
    {
      ArLog::log(ArLog::Normal, "Using old style sim laser for %s", 
		 laser->getName());
      sick->setIsUsingSim(true);
    }
    else
    {
      ArLog::log(ArLog::Normal, "Using new style simulated laser for %s", 
		 laser->getName());
      laserData->myLaser = new ArSimulatedLaser(laser);
      laser = laserData->myLaser;
    }
    // return here, since the rest is just dealing with how to connect
    // to the laser, but if its a simulated laser then we don't even
    // do that
    return true;
  } 
    

  if ((laserData->myPort == NULL || strlen(laserData->myPort) == 0) && 
      (laserData->myPortType != NULL && strlen(laserData->myPortType) > 0))
  {
    ArLog::log(ArLog::Normal, "There is a laser port type given ('%s') for laser %d, but no laser port given, cannot configure laser", 
	       laserData->myPortType, laserData->myNumber);
    return false;
  }
  
  if ((laserData->myPort != NULL && strlen(laserData->myPort) > 0) &&
      (laserData->myPortType != NULL && strlen(laserData->myPortType) > 0))
  {
    ArLog::log(ArLog::Normal, "ArLaserConnector: Connection type and port given for laser, so overriding everything and using that information");
    if ((laserData->myConn = Aria::deviceConnectionCreate(
		 laserData->myPortType, laserData->myPort, portBuf, 
		 "ArLaserConnector:")) == NULL)
    {
      return false;
    }
    laser->setDeviceConnection(laserData->myConn);
    return true;
  }
      
      
  if (myRobotConnector != NULL && !myRobotConnector->getRemoteIsSim() && 
      myRobotConnector->getRemoteHost() != NULL && 
      strlen(myRobotConnector->getRemoteHost()) > 0)
  { 
    ArLog::log(ArLog::Normal, "ArLaserConnector: Remote host is used for robot, so remote host is being used for the laser too");
    // if a port was given for the laser, then use that one... 
    ArTcpConnection *tcpConn = new ArTcpConnection;
    laserData->myConn = tcpConn;
    if (laserData->myRemoteTcpPortReallySet)
      tcpConn->setPort(myRobotConnector->getRemoteHost(),
		       laserData->myRemoteTcpPort);
    // otherwise use the default for that laser type
    else
      tcpConn->setPort(myRobotConnector->getRemoteHost(),
		       laser->getDefaultTcpPort());
    /*
      This code is commented out because it created problems with demo
      (or any other program that used ArLaserConnector::connectLasers
      with addAllLasersToRobot as true)

    // now try and open the port, since if it doesn't open nothing will work
    if (!tcpConn->openSimple())
    { 
      if (laserData->myRemoteTcpPortReallySet)
	ArLog::log(ArLog::Terse, 
		   "Could not connect laser to remote host %s with given remote port %d.", 
		   myRobotConnector->getRemoteHost(), 
		   laserData->myRemoteTcpPort);
      else
	/// TODO is this next line wrong?
	ArLog::log(ArLog::Terse, 
		   "Could not connect laser to remote host %s with default remote port %d.", 
		   myRobotConnector->getRemoteHost(), 
		   laserData->myRemoteTcpPort);
      delete tcpConn;
      return false; 
    } 
    */
    // set the laser to use that
    laserData->myConn = tcpConn;
    laser->setDeviceConnection(laserData->myConn);
    return true;
  }

  if ((laserData->myPort != NULL && strlen(laserData->myPort) > 0) && 
      (laserData->myPortType == NULL || strlen(laserData->myPortType) == 0))
  {
    if (myRobot != NULL && (params = myRobot->getRobotParams()) != NULL)
    {
      if (params->getLaserPortType(laserData->myNumber) != NULL &&
	  params->getLaserPortType(laserData->myNumber)[0] != '\0')
      {	  
	ArLog::log(ArLog::Normal, "ArLaserConnector: There is a port given, but no port type given so using the robot parameters port type");
	if ((laserData->myConn = Aria::deviceConnectionCreate(
		     params->getLaserPortType(laserData->myNumber), 
		     laserData->myPort, portBuf, 
		     "ArLaserConnector: ")) == NULL)
	{
	  return false;
	}
      }
      else if (laser->getDefaultPortType() != NULL && 
	       laser->getDefaultPortType()[0] != '\0')
      {
	ArLog::log(ArLog::Normal, "ArLaserConnector: There is a port given, but no port type given and no robot parameters port type so using the laser's default port type");
	if ((laserData->myConn = Aria::deviceConnectionCreate(
		     laser->getDefaultPortType(),
		     laserData->myPort, portBuf, 
		     "ArLaserConnector: ")) == NULL)
	{
	  return false;
	}
      }
      else
      {
	ArLog::log(ArLog::Normal, "ArLaserConnector: There is a port given, but no port type given, no robot parameters port type, and no laser default port type, so using serial");
	if ((laserData->myConn = Aria::deviceConnectionCreate(
		     "serial",
		     laserData->myPort, portBuf, 
		     "ArLaserConnector: ")) == NULL)
	{
	  return false;
	}
      }
      laser->setDeviceConnection(laserData->myConn);
      return true;
    }
    else
    {
      ArLog::log(ArLog::Normal, "There is a laser port given ('%s') for laser %d, but no laser port type given and there are no robot params to find the information in, so assuming serial", 
		 laserData->myPort, laserData->myNumber);
      if ((laserData->myConn = Aria::deviceConnectionCreate(
		   "serial", laserData->myPort, portBuf, 
		   "ArLaserConnector: ")) == NULL)
      {
	return false;
      }
      laser->setDeviceConnection(laserData->myConn);
      return true;
    }
  }

  // if we get down here there was no information provided by the command line or in a laser connector, so see if we have params... if not then fail, if so then use those

  if (myRobot == NULL || (params = myRobot->getRobotParams()) == NULL)
  {
    ArLog::log(ArLog::Normal, "ArLaserConnector: No robot params are available, and no command line information given on how to connect to the laser, so cannot connect");
    return false;
  }

  ArLog::log(ArLog::Normal, "ArLaserConnector: Using robot params for connecting to laser");

  if ((laserData->myConn = Aria::deviceConnectionCreate(
		   params->getLaserPortType(laserData->myNumber), 
		   params->getLaserPort(laserData->myNumber), portBuf,
		   "ArLaserConnector: ")) == NULL)
  {
    return false;
  }

  laser->setDeviceConnection(laserData->myConn);
  return true;
}

AREXPORT void ArLaserConnector::logOptions(void) const
{
  ArLog::log(ArLog::Terse, "Options for ArLaserConnector:");
  ArLog::log(ArLog::Terse, "\nOptions shown are for currently set up lasers.  Activate lasers with -laserType<N> option");
  ArLog::log(ArLog::Terse, "to see options for that laser (e.g. \"-help -laserType1 lms2xx\").");
  ArLog::log(ArLog::Terse, "Valid laser types are: %s", Aria::laserGetTypes()); 
  ArLog::log(ArLog::Terse, "\nSee docs for details.");

  std::map<int, LaserData *>::const_iterator it;
  LaserData *laserData;

  for (it = myLasers.begin(); it != myLasers.end(); it++)
  {
    laserData = (*it).second;
    logLaserOptions(laserData);
  }

}

AREXPORT void ArLaserConnector::logLaserOptions(
	LaserData *laserData, bool header, bool metaOpts) const
{
  char buf[512];

  if (laserData == NULL)
  {
    ArLog::log(ArLog::Normal, 
	       "Tried to log laser options with NULL laser data");
    return;
  }

  if (laserData->myLaser == NULL)
  {
    ArLog::log(ArLog::Normal, 
	       "ArLaserConnector: There is no laser for laser number %d but there should be", 
	       laserData->myNumber);
    return;
  }

  ArLaser *laser = laserData->myLaser;

  if (laserData->myNumber == 1)
    buf[0] = '\0';
  else
    sprintf(buf, "%d", laserData->myNumber);
  
  if(header)
  {
    ArLog::log(ArLog::Terse, "");
    ArLog::log(ArLog::Terse, "Laser%s: (\"%s\")", buf, laser->getName());
  }

  if (metaOpts)
  {
    ArLog::log(ArLog::Terse, "-laserType%s <%s>", buf, Aria::laserGetTypes());
    ArLog::log(ArLog::Terse, "-lt%s <%s>", buf, Aria::laserGetTypes());
    
    ArLog::log(ArLog::Terse, "-connectLaser%s", buf);
    ArLog::log(ArLog::Terse, "-cl%s", buf);
  }

  ArLog::log(ArLog::Terse, "-laserPort%s <laserPort>", buf);
  ArLog::log(ArLog::Terse, "-lp%s <laserPort>", buf);

  ArLog::log(ArLog::Terse, "-laserPortType%s <%s>", buf, Aria::deviceConnectionGetTypes());
  ArLog::log(ArLog::Terse, "-lpt%s <%s>", buf, Aria::deviceConnectionGetTypes());

  ArLog::log(ArLog::Terse, "-remoteLaserTcpPort%s <remoteLaserTcpPort>", buf);
  ArLog::log(ArLog::Terse, "-rltp%s <remoteLaserTcpPort>", buf);  

  ArLog::log(ArLog::Terse, "-laserFlipped%s <true|false>", buf);
  ArLog::log(ArLog::Terse, "-lf%s <true|false>", buf);

  ArLog::log(ArLog::Terse, "-laserMaxRange%s <maxRange>", buf);
  ArLog::log(ArLog::Terse, "-lmr%s <maxRange>", buf);
  ArLog::log(ArLog::Terse, "\t<maxRange> is an unsigned int less than %d", 
	     laser->getAbsoluteMaxRange());
  
  
  if (laser->canSetDegrees())
  {
    ArLog::log(ArLog::Terse, "-laserDegreesStart%s <startAngle>", buf);
    ArLog::log(ArLog::Terse, "-lds%s <startAngle>", buf);
    ArLog::log(ArLog::Terse, "\t<startAngle> is a double between %g and %g",
	       laser->getStartDegreesMin(), laser->getStartDegreesMax());
    ArLog::log(ArLog::Terse, "-laserDegreesEnd%s <endAngle>", buf);
    ArLog::log(ArLog::Terse, "-lde%s <endAngle>", buf);
    ArLog::log(ArLog::Terse, "\t<endAngle> is a double between %g and %g",
	       laser->getEndDegreesMin(), laser->getEndDegreesMax());
	       
  }

  if (laser->canChooseDegrees())
  {
    ArLog::log(ArLog::Terse, "-laserDegrees%s <%s>", buf, 
	       laser->getDegreesChoicesString());
    ArLog::log(ArLog::Terse, "-ld%s <%s>", buf,
	       laser->getDegreesChoicesString());
  }

  if (laser->canSetIncrement())
  {
    ArLog::log(ArLog::Terse, "-laserIncrementByDegrees%s <incrementByDegrees>", buf);
    ArLog::log(ArLog::Terse, "-libd%s <incrementByDegrees>", buf);
    ArLog::log(ArLog::Terse, 
	       "\t<incrementByDegrees> is a double between %g and %g",
	       laser->getIncrementMin(), laser->getIncrementMax());

  }

  if (laser->canChooseIncrement())
  {
    ArLog::log(ArLog::Terse, "-laserIncrement%s <%s>", buf,
	       laser->getIncrementChoicesString());
    ArLog::log(ArLog::Terse, "-li%s <%s>", buf,
	       laser->getIncrementChoicesString());
  }
  
  if (laser->canChooseUnits())
  {
    ArLog::log(ArLog::Terse, "-laserUnits%s <%s>", buf,
	       laser->getUnitsChoicesString());
    ArLog::log(ArLog::Terse, "-lu%s <%s>", buf,
	       laser->getUnitsChoicesString());
  }

  if (laser->canChooseReflectorBits())
  {
    ArLog::log(ArLog::Terse, "-laserReflectorBits%s <%s>", buf,
	       laser->getReflectorBitsChoicesString());
    ArLog::log(ArLog::Terse, "-lrb%s <%s>", buf,
	       laser->getReflectorBitsChoicesString());
  }

  if (laser->canSetPowerControlled())
  {
    ArLog::log(ArLog::Terse, "-laserPowerControlled%s <true|false>", buf);
    ArLog::log(ArLog::Terse, "-lpc%s <true|false>", buf);
  }

  if (laser->canChooseStartingBaud())
  {
    ArLog::log(ArLog::Terse, "-laserStartingBaud%s <%s>", buf,
	       laser->getStartingBaudChoicesString());
    ArLog::log(ArLog::Terse, "-lsb%s <%s>", buf,
	       laser->getStartingBaudChoicesString());
  }

  if (laser->canChooseAutoBaud())
  {
    ArLog::log(ArLog::Terse, "-laserAutoBaud%s <%s>", buf,
	       laser->getAutoBaudChoicesString());
    ArLog::log(ArLog::Terse, "-lab%s <%s>", buf,
	       laser->getAutoBaudChoicesString());
  }

  ArLog::log(ArLog::Terse, "-laserAdditionalIgnoreReadings%s <readings>", buf);
  ArLog::log(ArLog::Terse, "-lair%s <readings>", buf);
  ArLog::log(ArLog::Terse, "\t<readings> is a string that contains readings to ignore separated by commas, where ranges are acceptable with a -, example '75,76,90-100,-75,-76,-90--100'");
  

}

/**
   Normally adding lasers is done from the .p file, you can use this
   if you want to add them explicitly in a program (which will
   override the .p file, and may cause some problems).

   This is mainly for backwards compatibility (ie used for
   ArSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just ArLaserConnector::connectLasers.
**/
AREXPORT bool ArLaserConnector::addLaser(
	ArLaser *laser, int laserNumber)
{
  std::map<int, LaserData *>::iterator it;
  LaserData *laserData = NULL;

  if ((it = myLasers.find(laserNumber)) != myLasers.end())
    laserData = (*it).second;

  if (laserData != NULL)
  {
    if (laserData->myLaserIsPlaceholder)
    {
      ArLog::log(myInfoLogLevel, 
		 "ArLaserConnector::addLaser: Replacing placeholder laser #%d of type %s but a replacement laser of type %s was passed in", 
		 laserNumber, laserData->myLaser->getName(), laser->getName());
      if (laserData->myOwnPlaceholder)
	delete laserData->myLaser;
      laserData->myLaser = laser;
    }
    else
    {
      if (laserData->myLaser != NULL)
	ArLog::log(ArLog::Terse, 
		   "ArLaserConnector::addLaser: Already have laser for number #%d of type %s but a replacement laser of type %s was passed in", 
		   laserNumber, laserData->myLaser->getName(), laser->getName());
      else
	ArLog::log(ArLog::Terse, 
		   "ArLaserConnector::addLaser: Already have laser for number #%d but a replacement laser of type %s was passed in", 
		   laserNumber, laser->getName());
      delete laserData;
      myLasers.erase(laserNumber);
    }
  }

  myLasers[laserNumber] = new LaserData(laserNumber, laser);
  return true;
}

/**
   Normally adding lasers is done from the .p file, you can use this
   if you want to add them explicitly in a program (which will
   override the .p file, and may cause some problems).

   This is only for backwards compatibility (ie used for
   ArSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just ArLaserConnector::connectLasers.
**/

AREXPORT bool ArLaserConnector::addPlaceholderLaser(
	ArLaser *placeholderLaser,
	int laserNumber, bool takeOwnershipOfPlaceholder)
{
  std::map<int, LaserData *>::iterator it;
  LaserData *laserData = NULL;

  if ((it = myLasers.find(laserNumber)) != myLasers.end())
    laserData = (*it).second;
  
  if (laserData != NULL)
  {
    if (laserData->myLaserIsPlaceholder)
    {
      ArLog::log(myInfoLogLevel, 
		 "ArLaserConnector::addPlaceholderLaser: Replacing placeholder laser #%d of type %s but a replacement laser of type %s was passed in", 
		 laserNumber, laserData->myLaser->getName(), 
		 placeholderLaser->getName());
      if (laserData->myOwnPlaceholder)
	delete laserData->myLaser;
      laserData->myLaser = placeholderLaser;
    }
    else
    {
      if (laserData->myLaser != NULL)
	ArLog::log(ArLog::Terse, 
		   "ArLaserConnector::addPlaceholderLaser: Already have laser for number #%d of type %s but a replacement laser of type %s was passed in", 
		   laserNumber, laserData->myLaser->getName(), placeholderLaser->getName());
      else
	ArLog::log(ArLog::Terse, 
		   "ArLaserConnector::addPlaceholderLaser: Already have laser for number #%d but a replacement laser of type %s was passed in", 
		   laserNumber, placeholderLaser->getName());
      delete laserData;
      myLasers.erase(laserNumber);
    }
  }

  myLasers[laserNumber] = new LaserData(laserNumber, placeholderLaser, true, 
					takeOwnershipOfPlaceholder);
  return true;
}


/**
   This is mainly for backwards compatibility (ie used for
   ArSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just ArLaserConnector::connectLasers.
**/
AREXPORT bool ArLaserConnector::setupLaser(ArLaser *laser,
					   int laserNumber)
{

  if (myRobot == NULL && myRobotConnector != NULL)
    myRobot = myRobotConnector->getRobot();

  std::map<int, LaserData *>::iterator it;
  LaserData *laserData = NULL;
  const ArRobotParams *params;

  if ((it = myLasers.find(laserNumber)) != myLasers.end())
    laserData = (*it).second;

  if (laserData == NULL && laser == NULL)
  {
    ArLog::log(ArLog::Terse, "ArLaserConnector::setupLaser: Do not have laser #%d", laserNumber) ;
    return false;
  }
  if (laserData != NULL && laser != NULL && !laserData->myLaserIsPlaceholder &&
      laserData->myLaser != laser)
  {
    if (laserData->myLaser != NULL)
      ArLog::log(ArLog::Terse, "ArLaserConnector::setupLaser: Already have laser for number #%d (%s) but a replacement laser (%s) was passed in, this will replace all of the command line arguments for that laser", 
		 laserNumber, laserData->myLaser->getName(), laser->getName());
    else
      ArLog::log(ArLog::Terse, "ArLaserConnector::setupLaser: Already have laser for number #%d but a replacement laser (%s) was passed in, this will replace all of the command line arguments for that laser", 
		 laserNumber, laser->getName());
      
    delete laserData;
    myLasers.erase(laserNumber);
    myLasers[laserNumber] = new LaserData(laserNumber, laser);
  }

  if (laserData == NULL && laser != NULL)
  {
    laserData = new LaserData(laserNumber, laser);
    myLasers[laserNumber] = laserData;
    if (myAutoParseArgs && !parseLaserArgs(myParser, laserData))
    {
      ArLog::log(ArLog::Normal, "ArLaserConnector: Auto parsing args for laser %s (num %d)", laserData->myLaser->getName(), laserNumber);
      return false;
    }
  }


  // see if there is no laser (ie if it was a sick done in the old
  // style), or if the laser passed in doesn't match the one this
  // class created (I don't know how it'd happen, but...)... and then
  // configure it
  if ((laserData->myLaser == NULL || laserData->myLaser != laser ||
	  laserData->myLaserIsPlaceholder))
  {
    if (laserData->myLaserIsPlaceholder)
    {
      if (laserData->myOwnPlaceholder && laserData->myLaser != NULL)
	delete laserData->myLaser;
      laserData->myLaser = laser;
      laserData->myLaserIsPlaceholder = false;
    }

    if (!internalConfigureLaser(laserData))
      return false;
  }
  

  // setupLaser automatically adds this to the robot, since the
  // connectlaser stuff is the newer more supported way and is more
  // configurable.. it only adds it as a laser since the legacy code
  // won't add it that way, but will add it as a range device
  if (myRobot != NULL)
  {
    myRobot->addLaser(laser, laserNumber);
    //myRobot->addRangeDevice(laser);
  }
  else
  {
    ArLog::log(ArLog::Normal, "ArLaserConnector::setupLaser: No robot, so laser cannot be added to robot");
  }
  return true;
}

/**
   This is mainly for backwards compatibility (ie used for
   ArSimpleConnector).  If you're using this class you should probably
   use the new functionality which is just ArLaserConnector::connectLasers.
**/
AREXPORT bool ArLaserConnector::connectLaser(ArLaser *laser,
					     int laserNumber, 
					     bool forceConnection)
{
  std::map<int, LaserData *>::iterator it;
  LaserData *laserData = NULL;
  
  laser->lockDevice();
  // set up the laser regardless
  if (!setupLaser(laser, laserNumber))
  {
    laser->unlockDevice();
    return false;
  }
  laser->unlockDevice();

  if ((it = myLasers.find(laserNumber)) != myLasers.end())
    laserData = (*it).second;

  if (laserData == NULL)
  {
    ArLog::log(ArLog::Normal, "ArLaserConnector::connectLaser: Some horrendous error in connectLaser with laser number %d", laserNumber);
    return false;
  }
  // see if we want to connect
  if (!forceConnection && !laserData->myConnect)
    return true;
  else
    return laser->blockingConnect();
}

AREXPORT bool ArLaserConnector::connectLasers(
	bool continueOnFailedConnect, bool addConnectedLasersToRobot, 
	bool addAllLasersToRobot, bool turnOnLasers,
	bool powerCycleLaserOnFailedConnect)
{
  std::map<int, LaserData *>::iterator it;
  LaserData *laserData = NULL;
  
  ArLog::log(myInfoLogLevel, 
	     "ArLaserConnector: Connecting lasers");


  if (myAutoParseArgs && !myParsedArgs)
  {
    ArLog::log(ArLog::Normal, 
	       "ArLaserConnector: Auto parsing args for lasers");
    if (!parseArgs())
    {
      return false;
    }
  }

  if (addAllLasersToRobot)
  {
    if (myRobot != NULL)
    {
      for (it = myLasers.begin(); it != myLasers.end(); it++)
      {
	laserData = (*it).second;
	myRobot->addLaser(laserData->myLaser, laserData->myNumber);
	//myRobot->addRangeDevice(laserData->myLaser);
	ArLog::log(ArLog::Verbose, 
	    "ArLaserConnector::connectLasers: Added %s to robot as laser %d", 
		   laserData->myLaser->getName(), laserData->myNumber);
      }
    }
    else
    {
      ArLog::log(ArLog::Normal, "ArLaserConnect::connectLasers: Supposed to add all lasers to robot, but there is no robot");
      return false;
    }
  }

  for (it = myLasers.begin(); it != myLasers.end(); it++)
  {
    laserData = (*it).second;
    if (laserData->myLaserIsPlaceholder)
    {
      ArLog::log(ArLog::Normal, "ArLaserConnector::connectLasers: This function was called to connect laser %s (num %d) but there is a placeholder laser, so things are not configured correctly, you must use setupLaser or connectLaser with a placeholder laser, see the documenation for more details",
		 laserData->myLaser->getName(), laserData->myNumber);
      continue;
    }
    if (laserData->myConnectReallySet && laserData->myConnect)
    {
      // if we want to turn on the lasers if we can, then see if the
      // firwmare supports the power command for the lasers by
      // checking the config (and only LRF and LRF5B2 are specified in
      // firmware right now too)
      if (turnOnLasers)
      {
	if (laserData->myNumber == 1)
	{
	  // see if the firmware supports the LRF command
	  if (myRobot->getOrigRobotConfig() != NULL && 
	      myRobot->getOrigRobotConfig()->hasPacketArrived() && 
	      myRobot->getOrigRobotConfig()->getPowerBits() & ArUtil::BIT1)
	  {
	    ArLog::log(myInfoLogLevel, 
		       "ArLaserConnector::connectLasers: Turning on LRF power for %s",
		       laserData->myLaser->getName());
	    myRobot->comInt(ArCommands::POWER_LRF, 1);
	    ArUtil::sleep(250);
	  }
	  else
	  {
	    ArLog::log(myInfoLogLevel, 
		       "ArLaserConnector::connectLasers: Using legacy method to turn on LRF power for %s since firmware or robot doesn't support new way",
		       laserData->myLaser->getName());
	    myRobot->com2Bytes(31, 11, 1);
	    ArUtil::sleep(250);
	  }
	}
	else if (laserData->myNumber == 2)
	{
	  // see if the firmware supports the LRF2 command
	  if (myRobot->getOrigRobotConfig() != NULL && 
	      myRobot->getOrigRobotConfig()->hasPacketArrived() && 
	      myRobot->getOrigRobotConfig()->getPowerBits() & ArUtil::BIT9)
	  {
	    ArLog::log(myInfoLogLevel, 
		       "ArLaserConnector::connectLasers: Turning on LRF2 power for %s",
		       laserData->myLaser->getName());
	    myRobot->comInt(ArCommands::POWER_LRF2, 1);
	    ArUtil::sleep(250);
	  }
	  else
	  {
	    ArLog::log(myInfoLogLevel, 
		       "ArLaserConnector::connectLasers: Cannot turn on LRF2 power for %s since firmware or robot doesn't support it",
		       laserData->myLaser->getName());
	    ArUtil::sleep(250);
	  }
	}
	else
	{
	  ArLog::log(myInfoLogLevel, 
	      "ArLaserConnector::connectLasers: Cannot turn power on for %s, since it is number %d (higher than 2)",
		     laserData->myLaser->getName(), 
		     laserData->myLaser->getLaserNumber());
	}
      }
      ArLog::log(myInfoLogLevel, 
		 "ArLaserConnector::connectLasers: Connecting %s",
		 laserData->myLaser->getName());

      laserData->myLaser->setRobot(myRobot);
      
      bool connected = false;

      connected = laserData->myLaser->blockingConnect();
      
      // if we didn't connect and we can power cycle the lasers then
      // do that and see if we can connect again
      /// TODO see if this firmware can actually do the power cycling
      if (!connected && powerCycleLaserOnFailedConnect)
      {
	if (laserData->myLaser->canSetPowerControlled())
	  laserData->myLaser->setPowerControlled(true);

	if (laserData->myNumber == 1)
	{
	  // see if the firmware supports the LRF command
	  if (myRobot->getOrigRobotConfig() != NULL && 
	      myRobot->getOrigRobotConfig()->hasPacketArrived() && 
	      myRobot->getOrigRobotConfig()->getPowerBits() & ArUtil::BIT1)
	  {
	    ArLog::log(ArLog::Normal, 
		       "ArLaserConnector::connectLasers: Cycling LRF power for %s and trying to connect again",
		       laserData->myLaser->getName());
	    myRobot->comInt(ArCommands::POWER_LRF, 0);
	    ArUtil::sleep(250);
	    myRobot->comInt(ArCommands::POWER_LRF, 1);
	    ArUtil::sleep(250);
	    connected = laserData->myLaser->blockingConnect();
	  }
	  else
	  {

	    ArLog::log(myInfoLogLevel, 
		       "ArLaserConnector::connectLasers: Using legacy method to cycle LRF power for %s since firmware or robot doesn't support new way",
		       laserData->myLaser->getName());
	    myRobot->com2Bytes(31, 11, 0);
	    ArUtil::sleep(250);
	    myRobot->com2Bytes(31, 11, 1);
	    ArUtil::sleep(250);
	    connected = laserData->myLaser->blockingConnect();
	  }
	}
	else if (laserData->myNumber == 2)
	{
	  // see if the firmware supports the LRF2 command
	  if (myRobot->getOrigRobotConfig() != NULL && 
	      myRobot->getOrigRobotConfig()->hasPacketArrived() && 
	      myRobot->getOrigRobotConfig()->getPowerBits() & ArUtil::BIT9)
	  {
	    ArLog::log(ArLog::Normal, 
		       "ArLaserConnector::connectLasers: Cycling LRF2 power for %s and trying to connect again",
		       laserData->myLaser->getName());
	    
	    myRobot->comInt(ArCommands::POWER_LRF2, 0);
	    ArUtil::sleep(250);
	    myRobot->comInt(ArCommands::POWER_LRF2, 1);
	    ArUtil::sleep(250);
	    connected = laserData->myLaser->blockingConnect();
	  }
	  else
	  {
	    ArLog::log(myInfoLogLevel, 
		       "ArLaserConnector::connectLasers: Cannot cycle LRF2 power for %s since firmware or robot doesn't support it",
		       laserData->myLaser->getName());
	    ArUtil::sleep(250);
	    ArUtil::sleep(250);
	  }
	}
	else
	{
	  ArLog::log(myInfoLogLevel, 
	      "ArLaserConnector::connectLasers: Cannot cycle power for %s, since it is number %d (higher than 2)",
		     laserData->myLaser->getName(), 
		     laserData->myLaser->getLaserNumber());
	}
      }

      if (connected)
      {
	if (!addAllLasersToRobot && addConnectedLasersToRobot)
	{
	  if (myRobot != NULL)
	  {
	    myRobot->addLaser(laserData->myLaser, laserData->myNumber);
	    //myRobot->addRangeDevice(laserData->myLaser);
	    ArLog::log(ArLog::Verbose, 
		       "ArLaserConnector::connectLasers: Added %s to robot",
		       laserData->myLaser->getName());
	  }
	  else
	  {
	    ArLog::log(ArLog::Normal, 
		       "ArLaserConnector::connectLasers: Could not add %s to robot, since there is no robot",
		       laserData->myLaser->getName());
	  }

	}
	else if (addAllLasersToRobot && myRobot != NULL)
	{
	  ArLog::log(ArLog::Verbose, 
		     "ArLaserConnector::connectLasers: %s already added to robot)", 
		     laserData->myLaser->getName());
	}
	else if (myRobot != NULL)
	{
	  ArLog::log(ArLog::Verbose, 
	     "ArLaserConnector::connectLasers: Did not add %s to robot", 
		     laserData->myLaser->getName());
	}
      }
      else
      {
	if (!continueOnFailedConnect)
	{
	  ArLog::log(ArLog::Normal, 
		     "ArLaserConnector::connectLasers: Could not connect %s, stopping", 
		     laserData->myLaser->getName());
	  return false;
	}
	else
	  ArLog::log(ArLog::Normal, 
		     "ArLaserConnector::connectLasers: Could not connect %s, continuing with remainder of lasers", 
		     laserData->myLaser->getName());

	  
      }
    }
  }

  ArLog::log(myInfoLogLevel, 
	     "ArLaserConnector: Done connecting lasers");
  return true;
}
