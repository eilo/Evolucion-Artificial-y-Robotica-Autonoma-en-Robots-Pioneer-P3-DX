#ifndef ARLMS2XXPACKETRECEIVER_H
#define ARLMS2XXPACKETRECEIVER_H

#include "ariaTypedefs.h"
#include "ArDeviceConnection.h"
#include "ArLMS2xxPacket.h"

/// Given a device connection it receives packets from the sick through it
class ArLMS2xxPacketReceiver
{
public:
  /// Constructor without an already assigned device connection
  AREXPORT ArLMS2xxPacketReceiver(unsigned char receivingAddress = 0, 
				bool allocatePackets = false,
				bool useBase0Address = false);
  /// Constructor with assignment of a device connection
  AREXPORT ArLMS2xxPacketReceiver(ArDeviceConnection *deviceConnection, 
				unsigned char receivingAddress = 0,
				bool allocatePackets = false,
				bool useBase0Address = false);
  /// Destructor
  AREXPORT virtual ~ArLMS2xxPacketReceiver();
  
  /// Receives a packet from the robot if there is one available
  AREXPORT ArLMS2xxPacket *receivePacket(unsigned int msWait = 0);

  /// Sets the device this instance receives packets from
  AREXPORT void setDeviceConnection(ArDeviceConnection *deviceConnection);
  /// Gets the device this instance receives packets from
  AREXPORT ArDeviceConnection *getDeviceConnection(void);
  
  /// Gets whether or not the receiver is allocating packets
  AREXPORT bool isAllocatingPackets(void) { return myAllocatePackets; }

protected:
  ArDeviceConnection *myDeviceConn;
  bool myAllocatePackets;
  ArLMS2xxPacket myPacket;
  unsigned char myReceivingAddress;
  bool myUseBase0Address;
  enum { STATE_START, STATE_ADDR, STATE_START_COUNT, STATE_ACQUIRE_DATA };
};

typedef ArLMS2xxPacketReceiver ArSickPacketReceiver;

#endif // ARSICKPACKETRECEIVER_H
