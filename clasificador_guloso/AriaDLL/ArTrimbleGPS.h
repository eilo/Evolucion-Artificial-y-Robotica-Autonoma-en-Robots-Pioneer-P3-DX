
#ifndef ARTRIMBLEGPS_H
#define ARTRIMBLEGPS_H


#include "ariaTypedefs.h"
#include "ArFunctor.h"
#include "ariaUtil.h"
#include "ArGPS.h"
#include "ArDeviceConnection.h"
#include "ArMutex.h"

#include <deque>

class ArTrimbleAuxDeviceConnection;

/** @brief GPS subclass to support the Trimble AgGPS and other Trimble GPS devices.
 *  
 *  This subclass extends ArGPS to send initialization commands specific
 *  to Trimble GPS devices, and to handle the PTNLAG001 message which
 *  is specific to the Trimble GPS (this message contains data received 
 *  from an auxilliary device connected to the GPS; ArTrimbleGPS
 *  simply parses its contents as a new NMEA message; i.e. data received
 *  by the Trimble is assumed to be NMEA messages that it forwards
 *  via the PTNLAG001 message.)
 *
 *  @note You must also configure the ports using
 *  the Trimble AgRemote program
 *  (http://www.trimble.com/support_trl.asp?pt=AgRemote&Nav=Collection-1545).  
 *  Enable the following messages on whichever
 *  GPS port the computer is connected to: GPRMC, GPGGA, GPGSA, GPGSV, GPGST,
 *  GPMSS, and set input (I) protocol to TSIP 38k baud, and output
 *  protocol (O) to NMEA 38k baud. 
 *  This configuration is done by MobileRobots when shipping a Trimble AgGPS
 *  but you may need to do this if the GPS loses its configuration or after
 *  changing any other settings (Note that AgRemote resets the port settings each time
 *  it connects, so you must reset them each time before exiting AgRemote!)
 */
class ArTrimbleGPS : public virtual ArGPS {
private:
  ArFunctor1C<ArTrimbleGPS, ArNMEAParser::Message> myAuxDataHandler;
  void handlePTNLAG001(ArNMEAParser::Message message);
public:
  AREXPORT ArTrimbleGPS();
  AREXPORT virtual ~ArTrimbleGPS();

  /** Send a TSIP command to the Trimble GPS.
   *  See the TSIP Reference guide for details.
   *  Note, the data must be 66 characters or less.
   */
  AREXPORT bool sendTSIPCommand(char command, const char *data, size_t dataLen);

protected:
  AREXPORT virtual bool initDevice();

};


#endif 


