#include "ArExport.h"
#include "ArMD5Calculator.h"

#include "ariaOSDef.h"
#include "ariaInternal.h"

#include "ArLog.h"

AREXPORT ArMD5Calculator::ArMD5Calculator(ArFunctor1<const char*> *secondFunctor) :
  myFunctor(this, &ArMD5Calculator::append),
  mySecondFunctor(secondFunctor),
  myState(),
  myDigest(),
  myIsFinished(false)
{
  myDigest[0] = '\0';
  md5_init(&myState);
}

AREXPORT ArMD5Calculator::~ArMD5Calculator()
{
}
  
AREXPORT void ArMD5Calculator::reset()
{
  myDigest[0] = '\0';
  md5_init(&myState);

  myIsFinished = false;
}

AREXPORT ArFunctor1<const char *> *ArMD5Calculator::getFunctor()
{
  return &myFunctor;
}

AREXPORT ArFunctor1<const char *> *ArMD5Calculator::getSecondFunctor()
{
  return mySecondFunctor;
}

AREXPORT void ArMD5Calculator::setSecondFunctor(ArFunctor1<const char *> *secondFunctor)
{
  mySecondFunctor = secondFunctor;
}

AREXPORT unsigned char *ArMD5Calculator::getDigest()
{
  if (!myIsFinished) {
    md5_finish(&myState, myDigest);
    myIsFinished = true;
  }
  return myDigest;

} // end method getDigest


AREXPORT void ArMD5Calculator::toDisplay(const unsigned char *digestBuf,
                                         size_t digestLength,
                                         char *displayBuf,
                                         size_t displayLength)
{
  if ((displayBuf == NULL) || (digestBuf == NULL)) {
    return;
  }
  int j = 0;

  memset(displayBuf, 0, displayLength);

  for (int i = 0; 
        ((i < digestLength) && (j < displayLength - 1)); 
        i++) {

    j = i * 2;
    if (j > displayLength - 3) {
      break;
    }
    // Seems that we need the 3 to account for the end-of-string (because otherwise
    // (j + 1) holds the null char)
    snprintf(&displayBuf[j], 3, "%02x", digestBuf[i]);

    /***
    ArLog::log(ArLog::Normal,
               "ArMD5Calculator::toDisplay()  i = %i  j = %i,  \"%02x\" = \"%x,%x\"",
               i, j, digestBuf[i], displayBuf[j], displayBuf[j + 1]);
    ***/
  } // end for each char

  displayBuf[displayLength - 1] = '\0'; 

  /***
  ArLog::log(ArLog::Normal,
             "ArMD5Calculator::toDisplay() returns %s displayLength = %i",
             displayBuf,
             displayLength);
  ***/

} // end method toDisplay

AREXPORT bool ArMD5Calculator::calculateChecksum(const char *fileName,
                                                 unsigned char *md5DigestBuffer,
                                                 size_t md5DigestBufferLen)
{
  ArTime calcTime;

  if (ArUtil::isStrEmpty(fileName)) {
    // TODO ArLog
    return false;
  }
  FILE *file = fopen(fileName, "r");
  if (file == NULL) {
    // TODO ArLog
    return false;
  }

  ArMD5Calculator calculator;

  // TODO: Make this static and protect w/ mutex?
  char line[10000];
  bool ret = true;

  while (fgets(line, sizeof(line), file) != NULL)
  {
    calculator.append(line);
  }
  
  fclose(file);

  if (md5DigestBuffer != NULL) {
    if (md5DigestBufferLen != ArMD5Calculator::DIGEST_LENGTH) {
      // log warning
    }
    memset(md5DigestBuffer, 0, md5DigestBufferLen);
    memcpy(md5DigestBuffer, calculator.getDigest(), 
          ArUtil::findMin(md5DigestBufferLen, ArMD5Calculator::DIGEST_LENGTH));
  }

  int elapsed = calcTime.mSecSince();
  
  ArLog::log(ArLog::Normal,
             "ArMD5Calculator::calculateChecksum(%s) took %i msecs",
             fileName, elapsed);

  return true;


} // end method calculateChecksum



AREXPORT void ArMD5Calculator::append(const char *str)
{
  if (str == NULL) {
    ArLog::log(ArLog::Terse,
               "ArMD5Calculator::append cannot append null string");
  }
  // YUCK to the cast!
  md5_append(&myState, (unsigned char *) str, strlen(str));

  if (mySecondFunctor != NULL) {
    mySecondFunctor->invoke(str);
  }

} // end method append

