
#ifndef ARGPSCONNECTOR_H
#define ARGPSCONNECTOR_H

#include <string>
#include <vector>

#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "ArFunctor.h"
#include "ArGPS.h"

class ArDeviceConnection;
class ArRobot;

/** 
 *  @brief Factory for creating ArGPS objects based on command-line
 *  parameters.
 *
 *  Use createGPS() to create a GPS object.
 *
 *  @note The device connection object created by 
 *  ArGPSConnector is destroyed  when ArGPSConnector is 
 *  destroyed. Therefore, you must not destroy an ArGPSConnector
 *  while its associated ArGPS is in use.
 *
 * The following command-line arguments are checked:
 * @verbinclude ArGPSConnector_options
 *
*/

class ArGPSConnector {
public:
  AREXPORT ArGPSConnector(ArArgumentParser* argParser);
  AREXPORT ~ArGPSConnector();

  /** Gets command line arguments */
  AREXPORT bool parseArgs();


  /** Create a new GPS object (may be an ArGPS subclass based on device type)
   * and a device connection for that GPS. Use ArGPS::blockingConnect() to open the connection.
   *
   * @param robot If not NULL, obtain default values for GPS type, port and baud
   * from this robot's parameters (given in parameter file), for any of these
   * not set from command-line arguments in parseArgs().
   *
   * @return NULL if there was an error creating a GPS object or an error
   * creating and opening its device connection. Otherwise, return the new GPS
   * object.  
   */
  AREXPORT ArGPS* createGPS(ArRobot *robot = NULL);
  /** @copydoc createGPS() */
  AREXPORT ArGPS* create(ArRobot *robot = NULL) { return createGPS(robot); }

#if 0  

//doesn't really do anything :
  /** Try to establish a device connection between @a gps (created by calling
   * createGPS() and the GPS receiver.
   */
  AREXPORT bool connectGPS(ArGPS *gps);
  /** @copydoc connectGPS() */
  AREXPORT bool connect(ArGPS *gps) { return connectGPS(gps) ; }
#endif

  /** @brief Device type identifiers */
  typedef enum {
      /// For a standard NMEA GPS device (no extra initialization or interpretation needed) accessible using ArGPS 
      Standard, 
      /// For a Novatel device accessible using ArNovatelGPS 
      Novatel,
      /// For a Trimble device accessible using ArTrimbleGPS
      Trimble,
      /// Not set or invalid
      Invalid,
      /// Novatel SPAN
      /// @since Aria 2.7.2
      NovatelSPAN
  } GPSType;

  AREXPORT GPSType getGPSType() const { return myDeviceType; }

protected:
  ArDeviceConnection *myDeviceCon;
  ArArgumentParser *myArgParser;
  ArRetFunctorC<bool, ArGPSConnector> myParseArgsCallback;
  ArFunctorC<ArGPSConnector> myLogArgsCallback;
  int myBaud;
  const char *myPort;
  const char *myTCPHost;
  int myTCPPort;
  GPSType myDeviceType;


  /** Log argument option information */
  void logOptions();
  GPSType deviceTypeFromString(const char *str);
};


#endif  // ifdef ARGPSCONNECTOR_H


