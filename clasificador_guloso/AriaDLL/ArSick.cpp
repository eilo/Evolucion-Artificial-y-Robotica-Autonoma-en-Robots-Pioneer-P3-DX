#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArSick.h"
#include "ArRobot.h"
#include "ArSerialConnection.h"
#include "ariaInternal.h"
#include <time.h>


AREXPORT ArSick::ArSick(
	size_t currentBufferSize, size_t cumulativeBufferSize,
	const char *name, bool addAriaExitCB, bool isSecondLaser) : 
  ArLMS2xx(!isSecondLaser ? 1 : 2, name, !isSecondLaser ? false : true)
{
  setCurrentBufferSize(currentBufferSize);
  setCumulativeBufferSize(cumulativeBufferSize);
}

AREXPORT ArSick::~ArSick()
{

}


/**
 * Manually set laser configuration options for connection. This must be called
 * only before connecting to the laser (not while the laser is connected).
 * This configuration is automatically performed if you are using
 * ArSimpleConnector to connect to the laser based on command line parameters,
 * so calling this function is only neccesary if you are not using
 * ArSimpleConnector, or you wish to always override ArSimpleConnector's
 * configuration. 
 *
 * (Don't forget, you must lock ArLMS2xx with lockDevice() if multiple threads
 * are accessing the ArLMS2xx, e.g. if you used runAsync().)
**/
AREXPORT void ArSick::configure(bool useSim, bool powerControl,
				bool laserFlipped, BaudRate baud,
				Degrees deg, Increment incr)
{
  configureShort(useSim, baud, deg, incr);

  setPowerControlled(powerControl);
  setFlipped(laserFlipped);
}

/**
 * @copydoc configure()
**/
AREXPORT void ArSick::configureShort(bool useSim, BaudRate baud,
				     Degrees deg, Increment incr)
{
  myUseSim = useSim;

  setPowerControlled(true);
  setFlipped(false);

  if (baud == BAUD9600)
    chooseAutoBaud("9600");
  else if (baud == BAUD19200)
    chooseAutoBaud("19200");
  else if (baud == BAUD38400)
    chooseAutoBaud("38400");
  else
    ArLog::log(ArLog::Normal, "%s: Bad baud choice", getName());

  if (deg == DEGREES180)
    chooseDegrees("180");
  if (deg == DEGREES100)
    chooseDegrees("100");
  else
    ArLog::log(ArLog::Normal, "%s: Bad degrees choice", getName());


  if (incr == INCREMENT_ONE)
    chooseIncrement("one");
  if (incr == INCREMENT_HALF)
    chooseIncrement("half");
  else
    ArLog::log(ArLog::Normal, "%s: Bad increment choice", getName());

}

/**
   Sets the range/bit information.  The old immutable combination is
   (in effect) the same as the new default.  If you look at the enums
   for these units you can see the effect this has on range.  
**/
AREXPORT void ArSick::setRangeInformation(Bits bits, Units units)
{
  if (bits == BITS_1REFLECTOR)
    chooseReflectorBits("1ref");
  else if (bits == BITS_2REFLECTOR)
    chooseReflectorBits("2ref");
  else if (bits == BITS_3REFLECTOR)
    chooseReflectorBits("3ref");
  else
    ArLog::log(ArLog::Normal, "%s: Bad reflectorBits choice", getName());

  if (units == UNITS_1MM)
    chooseUnits("1mm");
  else if (units == UNITS_1MM)
    chooseUnits("1cm");
  else if (units == UNITS_10CM)
    chooseUnits("10cm");
  else
    ArLog::log(ArLog::Normal, "%s: Bad units choice", getName());
}



AREXPORT bool ArSick::isControllingPower(void)
{
  return getPowerControlled(); 
}

AREXPORT bool ArSick::isLaserFlipped(void)
{
  return getFlipped();
}

AREXPORT ArSick::Degrees ArSick::getDegrees(void)
{
  if (strcasecmp(getDegreesChoice(), "180") == 0)
    return DEGREES180;
  else if (strcasecmp(getDegreesChoice(), "100") == 0)
    return DEGREES100;
  else
  {
    ArLog::log(ArLog::Normal, "ArSick::getDegrees: Invalid degrees %s",
	       getDegreesChoice());
    return DEGREES_INVALID;
  }
}

AREXPORT ArSick::Increment ArSick::getIncrement(void)
{
  if (strcasecmp(getIncrementChoice(), "one") == 0)
    return INCREMENT_ONE;
  else if (strcasecmp(getIncrementChoice(), "half") == 0)
    return INCREMENT_HALF;
  else
  {
    ArLog::log(ArLog::Normal, "ArSick::getIncrement: Invalid increment %s",
	       getIncrementChoice());
    return INCREMENT_INVALID;
  }
}

AREXPORT ArSick::Bits ArSick::getBits(void)
{
  if (strcasecmp(getReflectorBitsChoice(), "1ref") == 0)
    return BITS_1REFLECTOR;
  else if (strcasecmp(getReflectorBitsChoice(), "2ref") == 0)
    return BITS_2REFLECTOR;
  else if (strcasecmp(getReflectorBitsChoice(), "3ref") == 0)
    return BITS_3REFLECTOR;
  else
  {
    ArLog::log(ArLog::Normal, "ArSick::getReflectorBits: Invalid ReflectorBits %s",
	       getReflectorBitsChoice());
    return BITS_INVALID;
  }
}

AREXPORT ArSick::Units ArSick::getUnits(void)
{
  if (strcasecmp(getUnitsChoice(), "1mm") == 0)
    return UNITS_1MM;
  else if (strcasecmp(getUnitsChoice(), "1cm") == 0)
    return UNITS_1CM;
  else if (strcasecmp(getUnitsChoice(), "10cm") == 0)
    return UNITS_10CM;
  else
  {
    ArLog::log(ArLog::Normal, "ArSick::getUnits: Invalid units %s",
	       getUnitsChoice());
    return UNITS_INVALID;
  }
}

AREXPORT void ArSick::setIsControllingPower(bool controlPower)
{
  setPowerControlled(controlPower);
}

AREXPORT void ArSick::setIsLaserFlipped(bool laserFlipped)
{
  setFlipped(laserFlipped);
}


AREXPORT bool ArSick::isUsingSim(void)
{
  return sickGetIsUsingSim();
}

AREXPORT void ArSick::setIsUsingSim(bool usingSim)
{
  return sickSetIsUsingSim(usingSim);
}