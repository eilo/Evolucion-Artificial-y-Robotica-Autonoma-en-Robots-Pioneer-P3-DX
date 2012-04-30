

#include "ArExport.h"
#include "ArSpeech.h"
#include "ArConfig.h"
#include "ariaInternal.h"


AREXPORT ArSpeechSynth::ArSpeechSynth() : 
  mySpeakCB(this, &ArSpeechSynth::speak), 
  myInitCB(this, &ArSpeechSynth::init), 
  myInterruptCB(this, &ArSpeechSynth::interrupt),
  myAudioPlaybackCB(0),
  myProcessConfigCB(this, &ArSpeechSynth::processConfig)
{
  myProcessConfigCB.setName("ArSpeechSynth");
}


AREXPORT bool ArSpeechSynth::init()
{
  return true;
}

AREXPORT void ArSpeechSynth::addToConfig(ArConfig *config)
{
  addVoiceConfigParam(config);
  config->addProcessFileCB(&myProcessConfigCB, 100);
}

AREXPORT ArSpeechSynth::~ArSpeechSynth()
{
}

AREXPORT ArRetFunctorC<bool, ArSpeechSynth>* ArSpeechSynth::getInitCallback(void) 
{
  return &myInitCB;
}

AREXPORT ArRetFunctor2C<bool, ArSpeechSynth, const char*, const char*>* ArSpeechSynth::getSpeakCallback(void) 
{
  return &mySpeakCB;
}


AREXPORT ArFunctorC<ArSpeechSynth>*  ArSpeechSynth::getInterruptCallback() 
{
  return &myInterruptCB;
}

AREXPORT void ArSpeechSynth::setAudioCallback(ArRetFunctor2<bool, ArTypes::Byte2*, int>* cb)
{
  myAudioPlaybackCB = cb;
}


AREXPORT bool ArSpeechSynth::speak(const char* text, const char* voiceParams) {
  return speak(text, voiceParams, NULL, 0);
}
  


bool ArSpeechSynth::processConfig()
{
  setVoice(myConfigVoice);
  return true;
}

void ArSpeechSynth::addVoiceConfigParam(ArConfig *config)
{
  const char *current = getCurrentVoiceName();
  if(current)
  {
    strncpy(myConfigVoice, current, sizeof(myConfigVoice));
  }
  else
  {
    myConfigVoice[0] = 0;
  }
  std::string displayHint;
  std::list<std::string> voices = getVoiceNames();
  for(std::list<std::string>::const_iterator i = voices.begin(); i != voices.end(); i++)
  {
    if(i == voices.begin())
      displayHint = "Choices:";
    else
      displayHint += ";;";
    displayHint += *i;
  }
  config->addParam(
    ArConfigArg("Voice", myConfigVoice, "Name of voice to use for speech synthesis", sizeof(myConfigVoice)), 
    "Speech Synthesis",
    ArPriority::NORMAL,
    displayHint.c_str()
  );
}


