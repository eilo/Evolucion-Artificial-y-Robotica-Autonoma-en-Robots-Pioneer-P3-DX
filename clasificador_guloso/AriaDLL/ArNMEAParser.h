
#ifndef ARNMEAPARSER_H
#define ARNMEAPARSER_H

#include "ariaTypedefs.h"
#include "ArFunctor.h"
#include "ariaUtil.h"
#include "ArDeviceConnection.h"
#include <string>
#include <vector>


/** @brief NMEA Parser
 *
 *  Parses NMEA input data and calls callbacks for certain messages with message
 *  parts.   NMEA is a standard output data protocol used by GPS devices and
 *  others (e.g. compass, altimiter, etc.)   This class is used internally by ArNMEAParser and
 *  subclasses, and by ArTCMCompassDirect.
 */
class ArNMEAParser {

public:
    /** @param name Used in log messages */
    AREXPORT ArNMEAParser(const char *name = "NMEA Parser");

    /** @brief Flags to indicates what the parse() method did. 
     *  i.e. If nothing was done, then the
     *  result will be 0. To check a parse() return result @a result to see if data was updated, use
     *  (result & ParseUpdated). To check if there was an error, use (result &
     *  ParseError). 
     */
    enum {
      ParseFinished = 1,  ///< There was no data to parse
      ParseError = 2,     ///< There was an error
      ParseData = 4,      ///< Input was recieved and stored, but no complete messages were parsed
      ParseUpdated = 8    ///< At least one complete message was parsed
    } ParseFlags;

    /** @brief Set whether checksum is ignored (default behavior is not to ignore it, and
     * skip messages with incorrect checksums, and log a warning mesage) */
    AREXPORT void setIgnoreChecksum(bool ignore) { ignoreChecksum = ignore; }

    /** NMEA message, divided into parts.  */
    typedef std::vector<std::string> MessageVector;

    /** Message data passed to handlers */
    typedef struct {
      /** The parts of the message, including initial message ID (but excluding
       * checksum) */
      ArNMEAParser::MessageVector* message;
      /** Timestamp when the beginning of this message was recieved and parsing
       * began. */
      ArTime timeParseStarted;
    } Message;
      

    /** NMEA message handler type.  */
    typedef ArFunctor1<ArNMEAParser::Message> Handler;


    /** Set a handler for an NMEA message. Mostly for internal use or to be used
     * by related classes, but you could use for ususual or custom messages
     * emitted by a device that you wish to be handled outside of the ArNMEAParser
     * class. 
     */
    AREXPORT void addHandler(const char *message, ArNMEAParser::Handler *handler);
    AREXPORT void removeHandler(const char *message);

    /* Read a chunk of input text from the given device connection and 
     * parse with parse(char*, int).  The maximum amount of text read from the device
     * connection is determined by the internal buffer size in this class
     * (probably a few hundred bytes limit).
     * @return a result code from ParseFlags
     * @note You should only use one stream of data with ArNMEAParser, and in a
     * continuous fashion, since it will store partially recieved messages for
     * the next call to one of the parse() methods.
     */
    AREXPORT int parse(ArDeviceConnection *dev);

    /* Parse a chunk of input text. Call message handlers as complete NMEA
     * messages are parsed.  Parsing state is stored in this ArNMEAParser object.
     * @return a result code from ParseFlags
     */
    AREXPORT int parse(const char *buf, int n);

    

     
    
public:
    /* Map of message identifiers to handler functors */
    typedef std::map<std::string, ArNMEAParser::Handler*> HandlerMap;

private:
    /* NMEA message handlers used by ArNMEAParser */
    HandlerMap myHandlers;

public:
    const ArNMEAParser::HandlerMap& getHandlersRef() const { return myHandlers; }

private:

    const char *myName;

    /*  NMEA scanner state.
     *  There are possabilities for opmitization here, such 
     *  as just storing the read data in a buffer and handling
     *  each field as it is found in the buffer, or building
     *  a list of char* for each field pointing into the buffer
     *  instead of copying each field into a std::string in the
     *  currentMessage vector, etc. etc.
     */
    const unsigned short MaxNumFields;
    const unsigned short MaxFieldSize; // bytes
    bool ignoreChecksum;

    MessageVector currentMessage;
    ArTime currentMessageStarted;
    std::string currentField;
    char checksumBuf[3];
    short checksumBufOffset;
    bool inChecksum;
    bool inMessage;
    char currentChecksum;
    bool gotCR;

    // Tools to update state
    void beginMessage();
    void endMessage();
    void nextField();
    void beginChecksum();

    /* Data buffer used by handleInput(ArDeviceConnection*).
     * This should be enough to hold several NMEA messages. 
     * Most NMEA messages should be less than 50 bytes or so; 
     * 512 then allows a minumum of 10 messages parsed per 
     * call to handleInput.)
     */
    char myReadBuffer[256]; //[512];

};

#endif // ifdef ARNMEAPARSER_H

