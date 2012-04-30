#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArDeviceConnection.h"

bool ArDeviceConnection::ourStrMapInited = false;
ArStrMap ArDeviceConnection::ourStrMap;

AREXPORT ArDeviceConnection::ArDeviceConnection()
{
  if (!ourStrMapInited)
  {
    ourStrMapInited = true;
    buildStrMap();
  }
}

AREXPORT ArDeviceConnection::~ArDeviceConnection()
{
  close();
}


void ArDeviceConnection::buildStrMap(void)
{
  ourStrMap[STATUS_NEVER_OPENED] = "never opened";
  ourStrMap[STATUS_OPEN] = "open";
  ourStrMap[STATUS_OPEN_FAILED] = "open failed";
  ourStrMap[STATUS_CLOSED_NORMALLY] = "closed";
  ourStrMap[STATUS_CLOSED_ERROR] = "closed on error";
}

AREXPORT const char * ArDeviceConnection::getStatusMessage(int messageNumber) const
{
  ArStrMap::const_iterator it;
  if ((it = ourStrMap.find(messageNumber)) != ourStrMap.end())
    return (*it).second.c_str();
  else
    return NULL;
}
