// Proyecto : Guloso-clasificador - Pioneer P3-DX
// Nombre : Programa principal clasificador guloso
// Creado : 05.01.2011
// Autor : Marco Flores, MariaJose Pinto, Andrés Villacís
// Version : 1.0
//
// * AVISO IMPORTANTE * Este programa es propiedad (c)2010 Marco Flores
// * AVISO IMPORTANTE * Se autoriza su uso, solo con fines formativos.
// 
// EL USO DE ESTA APLICACION IMPLICA LA ACEPTACION DE LAS CONDICIONES DE USO QUE SE APLICAN
// AL PROGRAMARIO "Open Source", ESTE PROGRAMA SE PROPORCIONA "COMO ESTA", NO OBLIGANDO AL 
// AUTOR A CONTRAER COMPROMISO ALGUNO PARA CON QUIENES LO UTILICEN, ASI COMO DECLINANDO 
// CUALQUIER REPONSABILIDAD DIRECTA O INDIRECTA, CONTRAIDA POR LOS MISMOS EN SU UTILIZACION 
// FUERA DE LOS PROPOSITOS PARA EL QUE FUE ESCRITO Y DISEÑADO.
//
// CUALQUIER MODIFICACIÓN Y DISTRIBUCION DEL MISMO DEBERA CONTENER Y CITAR SU FUENTE Y ORIGEN.
//
// ASI MISMO EL AUTOR AGRADECERA CUALQUIER COMENTARIO o CORRECCION QUE LOS LECTORES ESTIMEN
// OPORTUNA CONTRIBUYENDO ESTOS ULTIMOS A MEJORAR LA APLICACION SOLO CON FINES FORMATIVOS.
// CONSIDEREN ENVIAR SUS COMENTARIOS A : mflorespazos@gmail.com
// --------------------------------------------------------------------------------------------
//
//

#include "stdafx.h"
#include "Aria.h"
#include <windows.h>
#include <string.h>
#include "Serial.h"

// ======================= F U N C I O N E S   A D I C I O N A L E S ========================//
// ==========================================================================================//

/* Esta estructura crea una funcion que gira al robot de acuerdo al sonar */
class ActionTurns : public ArAction
{
public:
  // Contructor, se lleva un umbral y una cantidad de grados
  ActionTurns(double turnThreshold, double turnAmount);
  // destructor
  virtual ~ActionTurns(void) {};
  // esta inicia la accion
  virtual ArActionDesired *fire(ArActionDesired currentDesired);
  // setea el robot y los sonares
  virtual void setRobot(ArRobot *robot);
protected:
  // el objeto del sonar
  ArRangeDevice *mySonar;
  // que es lo que se necesita hacer, se hace luego de iniciar el fire
  ArActionDesired myDesired;
  // distancia a la que empieza a moverse
  double myTurnThreshold;
  // cantidad de giro que necesita dar
  double myTurnAmount;
  // recuerda la direccion de giro
  int myTurning; // -1 == left, 1 == right, 0 == none
  //Creo un flag para que se active o que pare
  public: double myActivate;
  public: int myDirection;
};


/*
  This is the ActionTurn constructor, note the use of constructor chaining 
  with the ArAction. also note how it uses setNextArgument, which makes 
  it so that other things can see what parameters this action has, and 
  set them.  It also initializes the classes variables.
*/
ActionTurns::ActionTurns(double turnThreshold, double turnAmount) :
  ArAction("Turn")
{
  myTurnThreshold = turnThreshold;
  myTurnAmount = turnAmount;
  setNextArgument(ArArg("turn threshold (mm)", &myTurnThreshold, "The number of mm away from obstacle to begin turnning."));
  setNextArgument(ArArg("turn amount (deg)", &myTurnAmount, "The number of degress to turn if turning."));
  myTurning = 0;
}

/*
  Sets the myRobot pointer (all setRobot overloaded functions must do this),
  finds the sonar device from the robot, and if the sonar isn't there, 
  then it deactivates itself.
*/
void ActionTurns::setRobot(ArRobot *robot)
{
  ArAction::setRobot(robot);
  mySonar = robot->findRangeDevice("sonar");
  if (mySonar == NULL)
  {
    ArLog::log(ArLog::Terse, "No sonares, desactivando.");
    deactivate(); 
  }
}

/*
  This is the guts of the Turn action.
*/
ArActionDesired *ActionTurns::fire(ArActionDesired currentDesired)
{
  double leftRange, rightRange;
  // reset the actionDesired (must be done)
  myDesired.reset();
  // if the sonar is null we can't do anything, so deactivate
  if (mySonar == NULL)
  {
    deactivate();
    return NULL;
  }
  // Get the left readings and right readings off of the sonar
  leftRange = (mySonar->currentReadingPolar(0, 100) - 
        myRobot->getRobotRadius());
  rightRange = (mySonar->currentReadingPolar(-100, 0) - 
        myRobot->getRobotRadius());
  //-----------------------------
  /*
  // if neither left nor right range is within the turn threshold,
  // reset the turning variable and don't turn
  if (leftRange > myTurnThreshold && rightRange > myTurnThreshold)
  {
    myTurning = 0;
    myDesired.setDeltaHeading(0);
  }
  */
  //-----------------------------

  // si es que el activador esta en cero que resetee el turn y que no se mueva
  if (myActivate == 0)
  {
	  myTurning = 0;
	  myDesired.setDeltaHeading(0);
  }
  // if we're already turning some direction, keep turning that direction
  else if (myTurning)
  {
    myDesired.setDeltaHeading(myTurnAmount * myTurning);
  }
  // Gira a la izquierda
  else if (myDirection == 2)
  {
    myTurning = -1;
    myDesired.setDeltaHeading(myTurnAmount * myTurning);
  }
  // Gira a la derecha
  else if (myDirection == 1)
  {
    myTurning = 1;
    myDesired.setDeltaHeading(myTurnAmount * myTurning);
  }

  //-----------------------------
  /*
  // if we're not turning already, but need to, and left is closer, turn right
  // and set the turning variable so we turn the same direction for as long as
  // we need to
  else if (leftRange < rightRange)
  {
    myTurning = -1;
    myDesired.setDeltaHeading(myTurnAmount * myTurning);
  }
  // if we're not turning already, but need to, and right is closer, turn left
  // and set the turning variable so we turn the same direction for as long as
  // we need to
  else 
  {
    myTurning = 1;
    myDesired.setDeltaHeading(myTurnAmount * myTurning);
  }
  */
  //-----------------------------

  // return a pointer to the actionDesired, so resolver knows what to do
  return &myDesired;
}

// ================ F I N   F U N C I O N E S    A D I C I O N A L E S ================ //

int _tmain(int argc, char** argv)
{
  //-------------- M A I N    D E L   P R O G R A M A   D E L    R O B O T------------//
  //----------------------------------------------------------------------------------//
	
  //inicializaion de variables
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArSimpleConnector simpleConnector(&parser);
  ArRobot robot;
  ArSonarDevice sonar;
  ArAnalogGyro gyro(&robot);
  robot.addRangeDevice(&sonar);
  ActionTurns turn(400, 55);
  robot.addAction(&turn, 49);
  ActionTurns turn2(400, 55);
  robot.addAction(&turn2, 49);
  turn.deactivate();
  turn2.deactivate();

  // presionar tecla escape para salir del programa
  ArKeyHandler keyHandler;
  Aria::setKeyHandler(&keyHandler);
  robot.attachKeyHandler(&keyHandler);
  printf("Presionar ESC para salir\n");

  // uso de sonares para evitar colisiones con las paredes u 
  // obstaculos grandes, mayores a 8cm de alto
  ArActionLimiterForwards limiterAction("limitador velocidad cerca", 300, 600, 250);
  ArActionLimiterForwards limiterFarAction("limitador velocidad lejos", 300, 1100, 400);
  ArActionLimiterTableSensor tableLimiterAction;
  robot.addAction(&tableLimiterAction, 100);
  robot.addAction(&limiterAction, 95);
  robot.addAction(&limiterFarAction, 90);


  // Inicializon la funcion de goto
  ArActionGoto gotoPoseAction("goto");
  robot.addAction(&gotoPoseAction, 50);
  
  // Finaliza el goto si es que no hace nada
  ArActionStop stopAction("stop");
  robot.addAction(&stopAction, 40);

  // Parser del CLI
  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {    
    Aria::logOptions();
    exit(1);
  }
  
  // Conexion del robot
  if (!simpleConnector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::exit(1);
  }
  robot.runAsync(true);

  // enciende motores, apaga sonidos
  robot.enableMotors();
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // Imprimo algunos datos del robot como posicion velocidad y bateria
	robot.lock();
	ArLog::log(ArLog::Normal, "Posicion=(%.2f,%.2f,%.2f), Trans. Vel=%.2f, Bateria=%.2fV",
		robot.getX(), robot.getY(), robot.getTh(), robot.getVel(), robot.getBatteryVoltage());
	robot.unlock();

  const int duration = 100000; //msec
  ArLog::log(ArLog::Normal, "Completados los puntos en %d segundos", duration/1000);

  bool first = true;
  int goalNum = 0;
  int color = 3;
  ArTime start;
  start.setToNow();
  while (Aria::getRunning()) 
  {
    robot.lock();

    // inicia el primer punto 
    if (first || gotoPoseAction.haveAchievedGoal())
    {
      first = false;
	  
      goalNum++; //cambia de 0 a 1 el contador
	  printf("El contador esta en: --> %d <---\n",goalNum);
      if (goalNum > 20)
        goalNum = 1;

	  //comienza la secuencia de puntos
      if (goalNum == 1)
	  {
        gotoPoseAction.setGoal(ArPose(1150, 0));
		ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
		gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		// Imprimo algunos datos del robot como posicion velocidad y bateria
		robot.lock();
		ArLog::log(ArLog::Normal, "Posicion=(%.2f,%.2f,%.2f), Trans. Vel=%.2f, Bateria=%.2fV",
			robot.getX(), robot.getY(), robot.getTh(), robot.getVel(), robot.getBatteryVoltage());
		robot.unlock();
	  }
      else if (goalNum == 2)
	  {
		  printf("Gira 90 grados izquierda\n");
		  robot.unlock();
		  turn.myActivate = 1;
		  turn.myDirection = 1;
		  turn.activate();
		  ArUtil::sleep(1000);
		  turn.deactivate();
		  turn.myActivate = 0;
		  turn.myDirection = 0;
		  robot.lock();
	  }
	  else if (goalNum == 3)
	  {
        gotoPoseAction.setGoal(ArPose(1150, 2670));
		ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
		gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		// Imprimo algunos datos del robot como posicion velocidad y bateria
		robot.lock();
		ArLog::log(ArLog::Normal, "Posicion=(%.2f,%.2f,%.2f), Trans. Vel=%.2f, Bateria=%.2fV",
			robot.getX(), robot.getY(), robot.getTh(), robot.getVel(), robot.getBatteryVoltage());
		robot.unlock();
	  }
	  else if (goalNum == 4)
	  {
		  printf("Gira 90 grados izquierda\n");
		  robot.unlock();
		  turn2.myActivate = 1;
		  turn2.myDirection = 1;
		  turn2.activate();
		  ArUtil::sleep(1000);
		  turn2.deactivate();
		  turn2.myActivate = 0;
		  turn2.myDirection = 0;
		  robot.lock();
	  }
	  else if (goalNum == 5)
	  {
		gotoPoseAction.setGoal(ArPose(650, 2670));
		ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
		gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
	  }
	  else if (goalNum == 6)
	  {
		  printf("Gira 90 grados izquierda\n");
		  robot.unlock();
		  turn2.myActivate = 1;
		  turn2.myDirection = 1;
		  turn2.activate();
		  ArUtil::sleep(1000);
		  turn2.deactivate();
		  turn2.myActivate = 0;
		  turn2.myDirection = 0;
		  robot.lock();
	  }
	  else if (goalNum == 7)
	  {
		gotoPoseAction.setGoal(ArPose(650, 0));
		ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
		gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
	  }
	  else if (goalNum == 8)
	  {
		gotoPoseAction.setGoal(ArPose(1800,1199));
		ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
		gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
	  }
	  else if (goalNum == 9)
	  {
		gotoPoseAction.setGoal(ArPose(2600, 1199));
		ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
		gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
	  }
	  else if (goalNum == 10)
	  {
		  if (color == 1)
		  {
			gotoPoseAction.setGoal(ArPose(2800, 850));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
		  if (color == 2)
		  {
			gotoPoseAction.setGoal(ArPose(3500, 1199));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY()); 
		  }
		  if (color == 3)
		  {
			gotoPoseAction.setGoal(ArPose(2800, 1550));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
	  }
	  else if (goalNum == 11)
	  {
		  if (color == 1)
		  {
			gotoPoseAction.setGoal(ArPose(2800, 613));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
		  if (color == 2)
		  {
			  printf("Gira 180 grados derecha\n");
			  robot.unlock();
			  turn2.myActivate = 1;
			  turn2.myDirection = 2;
			  turn2.activate();
			  ArUtil::sleep(2000);
			  turn2.deactivate();
			  turn2.myActivate = 0;
			  turn2.myDirection = 0;
			  robot.lock();
			  goalNum = 19;
		  }
		  if (color == 3)
		  {
			gotoPoseAction.setGoal(ArPose(2800, 1785));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
	  }
	  else if (goalNum == 12)
	  {
		  if (color == 1)
		  {
			gotoPoseAction.setGoal(ArPose(3300, 413));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
		  if (color == 3)
		  {
			gotoPoseAction.setGoal(ArPose(3300, 1985));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
	  }
	  else if (goalNum == 13)
	  {
		  if (color == 1)
		  {
			gotoPoseAction.setGoal(ArPose(3500, 413));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
		  if (color == 3)
		  {
			gotoPoseAction.setGoal(ArPose(3500, 1985));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
	  }
	  else if (goalNum == 14)
	  {
		  //secuencia de drop de la figura
	  }
	  else if (goalNum == 15)
	  {
		  printf("Gira 180 grados derecha\n");
		  robot.unlock();
		  turn2.myActivate = 1;
		  turn2.myDirection = 2;
		  turn2.activate();
		  ArUtil::sleep(2000);
		  turn2.deactivate();
		  turn2.myActivate = 0;
		  turn2.myDirection = 0;
		  robot.lock();
	  }
	  else if (goalNum == 16)
	  {
		  if (color == 1)
		  {
			gotoPoseAction.setGoal(ArPose(3300, 413));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
		  if (color == 3)
		  {
			gotoPoseAction.setGoal(ArPose(3300, 1985));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
	  }
	  else if (goalNum == 17)
	  {
		  if (color == 1)
		  {
			gotoPoseAction.setGoal(ArPose(2800, 603));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
		  if (color == 3)
		  {
			gotoPoseAction.setGoal(ArPose(2800, 1795));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
	  }
	  else if (goalNum == 18)
	  {
		  if (color == 1)
		  {
			gotoPoseAction.setGoal(ArPose(2800, 860));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
		  if (color == 3)
		  {
			gotoPoseAction.setGoal(ArPose(2800, 1540));
			ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
			gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
		  }
	  }
	  else if (goalNum == 19)
	  {
		gotoPoseAction.setGoal(ArPose(2600, 1199));
		ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
		gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
	  }
	  else if (goalNum == 20)
	  {
		gotoPoseAction.setGoal(ArPose(1800, 1199));
		ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
		gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
	  }
    }

    if(start.mSecSince() >= duration) {
      ArLog::log(ArLog::Normal, "No puede llegar al punto, y la aplicacion saldra en %d", duration/1000);
      gotoPoseAction.cancelGoal();
      robot.unlock();
      ArUtil::sleep(3000);
      break;
    }
    
    robot.unlock();
    ArUtil::sleep(10);
  }

  // Robot desconectado al terminal el sleep
  Aria::shutdown();

//----------------------------------------------------------------------------------//

	return 0;
}

