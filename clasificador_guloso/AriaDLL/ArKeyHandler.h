#ifndef ARKEYHANDLER_H
#define ARKEYHANDLER_H

#include "ariaTypedefs.h"
#include "ArFunctor.h"
#include <map>
#include <stdio.h>

#ifndef WIN32
#include <termios.h>
#include <unistd.h>
#endif


/// Perform actions when keyboard keys are pressed
/**
   This class is used for handling input from the keyboard.
   Use addKeyHandler() to associate a functor with a keyboard key.
   ArKeyHandler will call that functor when the key is pressed.

   You should also register the keyhandler with Aria::setKeyHandler(),
   and before you create a key handler you should see if one is
   already there with Aria::getKeyHandler().  Only one key handler
   can be created in a program, and this lets independent parts
   of a program use the same key handler.
   
   You can attach a key handler to a robot with
   ArRobot::attachKeyHandler() which will put a task into the robots
   list of tasks to check for new keyboard input in each cycle, and which
   will also add a handler to end the robot's task cycle when Escape is pressed
   (you can override this by replacing it with your own handler).
   If you have mulitple ArRobot objects, only create one key handler and attach
   it to one robot.

   Alternatively, you can call checkKeys() periodically to check for new key input.
**/

class ArKeyHandler
{
public:
  /// This object will take over key capture when constructed, and release
  /// key capture when destroyed.
  AREXPORT ArKeyHandler(bool blocking = false, bool addAriaExitCB = true, 
			FILE *stream = NULL, 
			bool takeKeysInConstructor = true);

  /// Destructor. Reseases control of the keyboard and restores state before
  /// this key handler was created.
  AREXPORT ~ArKeyHandler();

  /// These are symbols for the non-ascii keys
  enum KEY {
    UP = 256, ///< Up arrow (keypad or 4 key dirs)
    DOWN, ///< Down arrow (keypad or 4 key dirs)
    LEFT, ///< Left arrow (keypad or 4 key dirs)
    RIGHT, ///< Right arrow (keypad or 4 key dirs)
    ESCAPE, ///< Escape key
    SPACE, ///< Space key
    TAB, ///< Tab key
    ENTER, ///< Enter key
    BACKSPACE, ///< Backspace key
    _StartFKeys,  ///< F key offset (internal; don't use).
    F1, ///< F1
    F2, ///< F2
    F3, ///< F3
    F4, ///< F4
    F5, ///< F5 (not supported on Windows yet)
    F6, ///< F6 (not supported on Windows yet)
    F7, ///< F7 (not supported on Windows yet)
    F8, ///< F8 (not supported on Windows yet)
    F9, ///< F9 (not supported on Windows yet)
    F10, ///< F10 (not supported on Windows yet)
    F11, ///< F11 (not supported on Windows yet)
    F12, ///< F12 (not supported on Windows yet)
    _EndFKeys, ///< F key range (internal; don't use)
    PAGEUP, ///< Page Up (not supported on Windows yet)
    PAGEDOWN, ///< Page Down (not supported on Windows yet)
    HOME,   ///< Home key (not supported on Windows yet)
    END,    ///< End key (not supported on Windows yet)
    INSERT, ///< Insert key (not supported on Windows yet)
    DEL  ///< Special delete key (often forward-delete) (not supported on Windows yet)
  };

  /// This adds a keyhandler, when the keyToHandle is hit, functor will fire
  AREXPORT bool addKeyHandler(int keyToHandle, ArFunctor *functor);

  /// This removes a key handler, by key
  AREXPORT bool remKeyHandler(int keyToHandler);
  /// This removes a key handler, by key
  AREXPORT bool remKeyHandler(ArFunctor *functor);

  /// Takes the key control over. For internal or special use, since it's
  /// called in the constructor.
  AREXPORT void takeKeys(bool blocking = false);

  /// Sets stdin back to its original settings, if its been restored
  /// it won't read anymore. For internal or special use, since it's 
  /// called in the destructor.
  AREXPORT void restore(void);

  /// intnernal, use addKeyHandler, Checks for keys and handles them
  AREXPORT void checkKeys(void);
  
  /// internal, use addKeyHandler instead... Gets a key from the stdin if ones
  /// available, -1 if there aren't any available
  AREXPORT int getKey(void);

protected:
#ifndef WIN32
  int getChar(void);
#endif 

  std::map<int, ArFunctor *> myMap;
  bool myBlocking;
  
  bool myRestored;
  ArFunctorC<ArKeyHandler> myAriaExitCB;
#ifndef WIN32
  struct termios myOriginalTermios;
#endif
  
  FILE *myStream;
  bool myTookKeys;
};


#endif // ARKEYHANDLER_H
