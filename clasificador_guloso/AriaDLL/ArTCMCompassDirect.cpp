
#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArTCMCompassDirect.h"
#include "ArDeviceConnection.h"
#include "ArSerialConnection.h"
#include "ariaUtil.h"

//#define DEBUG_ARTCMCOMPASSDIRECT 1
//#define DEBUG_ARTCMCOMPASSDIRECT_STATS 1

#if defined(DEBUG_ARTCMCOMPASSDIRECT) || defined(DEBUG_ARTCMCOMPASSDIRECT_STATS)
void ArTCMCompassDirect_printTransUnprintable( const char *data, int size){  for(int i = 0; i < size; ++i)  {    if(data[i] < ' ' || data[i] > '~')    {      printf("[0x%X]", data[i] & 0xff);    }    else    {      putchar(data[i]);    }  }}
#endif

AREXPORT ArTCMCompassDirect::ArTCMCompassDirect(ArDeviceConnection *devCon) :
  myDeviceConnection(devCon), myCreatedOwnDeviceConnection(false),
  myNMEAParser("ArTCMCompassDirect"),
  myHCHDMHandler(this, &ArTCMCompassDirect::handleHCHDM)
{
  myNMEAParser.addHandler("HCHDM", &myHCHDMHandler);
}

AREXPORT ArTCMCompassDirect::ArTCMCompassDirect(const char *serialPortName) :
  myCreatedOwnDeviceConnection(true),
  myNMEAParser("ArTCMCompassDirect"),
  myHCHDMHandler(this, &ArTCMCompassDirect::handleHCHDM)
{
  ArSerialConnection *newSerialCon = new ArSerialConnection();
  newSerialCon->setPort(serialPortName);
  newSerialCon->setBaud(9600);
  myDeviceConnection = newSerialCon;
  myNMEAParser.addHandler("HCHDM", &myHCHDMHandler);
}
  

AREXPORT ArTCMCompassDirect::~ArTCMCompassDirect() {
  if(myCreatedOwnDeviceConnection && myDeviceConnection)
    delete myDeviceConnection;
}

bool ArTCMCompassDirect::sendTCMCommand(const char *fmt, ...)
{
  if(!myDeviceConnection) return false;
  if(myDeviceConnection->getStatus() != ArDeviceConnection::STATUS_OPEN) return false;
  va_list args;
  va_start(args, fmt);
  char buf[32];
  vsnprintf(buf, sizeof(buf)-1, fmt, args);
  buf[sizeof(buf)-1] = 0;
  return myDeviceConnection->write(buf, strlen(buf));
}

AREXPORT bool ArTCMCompassDirect::blockingConnect(unsigned long connectTimeout)
{
  ArTime start;
  start.setToNow();
  if(!connect()) return false;
  ArLog::log(ArLog::Normal, "ArTCMCompassDirect: Opened connection, waiting for initial data...");
  while((unsigned long)start.mSecSince() <= connectTimeout)
  {
    if(read(0) > 0)
      return true;
    ArUtil::sleep(100);
  }
  ArLog::log(ArLog::Terse, "ArTCMCompassDirect: Error: No response from compass after %dms.", connectTimeout);
  return false;
}


AREXPORT bool ArTCMCompassDirect::connect()
{
  if(!myDeviceConnection) return false;
  if(myDeviceConnection->getStatus() != ArDeviceConnection::STATUS_OPEN)
  {
    if(!myDeviceConnection->openSimple()) return false;
  }

  if(!sendTCMCommand("h\r"))
    return false;


  // sp= is sampling/send rate, maximum of 8hz. sdo is output format (t for cm
  // protocol or n for nmea).
  if(!sendTCMCommand("h\rsp=8\rsn=m\rsdo=n\rgo\r"))  
    return false;

#ifdef DEBUG_ARTCMCOMPASSDIRECT
  char buf[640];
  memset(buf, 0, 640);
  myDeviceConnection->read(buf, 640, 2000);
  printf("Compass responded to init commands with: %s\n", buf);
#endif

  return true;
}



AREXPORT void ArTCMCompassDirect::commandUserCalibration()
{
  sendTCMCommand("cc\rmpcal=e\rgo\r");
}

AREXPORT void ArTCMCompassDirect::commandStopCalibration()
{
  sendTCMCommand("h\rmpcal=d\rautocal=d\r");
}

AREXPORT void ArTCMCompassDirect::commandContinuousPackets()
{
  sendTCMCommand("go\r");
}

AREXPORT void ArTCMCompassDirect::commandOff()
{
  sendTCMCommand("h\r");
}

AREXPORT void ArTCMCompassDirect::commandOnePacket()
{
  sendTCMCommand("c?\r");
}

AREXPORT void ArTCMCompassDirect::commandAutoCalibration()
{
  sendTCMCommand("h\rcc\rautocal=e\r");
}

AREXPORT int ArTCMCompassDirect::read(unsigned int msWait)
{
  return myNMEAParser.parse(myDeviceConnection);
}

void ArTCMCompassDirect::handleHCHDM(ArNMEAParser::Message m)
{
  myHeading = ArMath::fixAngle(atof((*m.message)[1].c_str()));
#ifdef DEBUG_ARTCMCOMPASSDIRECT 
  printf("XXX ArTCMCompassDirect: recieved HCHDM message with compass heading %f.\n", myHeading);
#endif
  myHaveHeading = true;
  incrementPacketCount();
  invokeHeadingDataCallbacks(myHeading);
}

