#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArActionGroup.h"
#include "ArAction.h"
#include "ArRobot.h"
#include "ArLog.h"

/**
   @param robot The robot that this action group is attached to. New actions added to this group (using addAction()) will be added to this robot object for evaluation in its action resolution task.
**/

AREXPORT ArActionGroup::ArActionGroup(ArRobot *robot)
{
  myRobot = robot;
}

AREXPORT ArActionGroup::~ArActionGroup()
{
  removeActions();
}


/**
 * The given action will be included in this group, and then added to this
 * group's robot (specified in the constructor) by using
 * ArRobot::addAction().
   @param action the action to add to the robot and to this group
   @param priority the priority to give the action; same meaning as in ArRobot::addAction
   @see ArRobot::addAction
*/
AREXPORT void ArActionGroup::addAction(ArAction *action, int priority)
{
  if (myRobot == NULL)
  {
    ArLog::log(ArLog::Terse, 
            "ArActionGroup::addAction: NULL robot pointer... failed.");
    return;
  }
  myActions.push_front(action);
  myRobot->addAction(action, priority);
}

/**
   @param action the action to remove from the robot
   @see ArRobot::remAction
*/
AREXPORT void ArActionGroup::remAction(ArAction *action)
{
  if (myRobot == NULL)
  {
    ArLog::log(ArLog::Terse, 
            "ArActionGroup::remAction: NULL robot pointer... failed.");
    return;
  }
  myActions.remove(action);
  myRobot->remAction(action);
}

AREXPORT void ArActionGroup::activate(void)
{
  std::list<ArAction *>::iterator it;
  if (myRobot == NULL)
  {
    ArLog::log(ArLog::Terse, 
            "ArActionGroup::activate: NULL robot pointer... failed.");
    return;
  }
  for (it = myActions.begin(); it != myActions.end(); it++)
    (*it)->activate();
}

AREXPORT void ArActionGroup::activateExclusive(void)
{
  std::list<ArAction *>::iterator it;
  
  if (myRobot == NULL)
  {
    ArLog::log(ArLog::Terse, 
            "ArActionGroup::activateExclusive: NULL robot pointer... failed.");
    return;
  }
  myRobot->deactivateActions();
  for (it = myActions.begin(); it != myActions.end(); it++)
    (*it)->activate();
}

AREXPORT void ArActionGroup::deactivate(void)
{
  std::list<ArAction *>::iterator it;
  if (myRobot == NULL)
  {
    ArLog::log(ArLog::Terse, 
            "ArActionGroup::deactivate: NULL robot pointer... failed.");
    return;
  }
  myRobot->deactivateActions();
  for (it = myActions.begin(); it != myActions.end(); it++)
    (*it)->deactivate();
}

AREXPORT std::list<ArAction *> *ArActionGroup::getActionList(void)
{
  return &myActions;
}

AREXPORT void ArActionGroup::removeActions(void)
{
  std::list<ArAction *>::iterator it;

  if (myRobot == NULL)
  {
    ArLog::log(ArLog::Terse, 
            "ArActionGroup::removeActions: NULL robot pointer... very bad.");
    return;
  }

  for (it = myActions.begin(); it != myActions.end(); ++it)
    myRobot->remAction((*it));
}

AREXPORT void ArActionGroup::deleteActions(void)
{
  /* MPL removed this since it doesn't differentiate between actions it added and actions added to it, double deletes are no fun
   */
  /*
  std::list<ArAction *>::iterator it;
  for (it = myActions.begin(); it != myActions.end(); ++it)
  {
    delete (*it);
  }
  */
}
