// Proyecto : Guloso-clasificador - Pioneer P3-DX
// Nombre : Programa principal clasificador guloso
// Creado : 14.01.2011
// Autor : Marco Flores, MariaJose Pinto, Andrés Villacís
// Version : 3.0
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

#define STRICT
#include "Aria.h"
#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "Serial.h"

enum { EOF_Char = 76 };

int ShowError (LONG lError, LPCTSTR lptszMessage)
{
	// Generate a message text
	TCHAR tszMessage[256];
	wsprintf(tszMessage,_T("%s\n(error code %d)"), lptszMessage, lError);
	// Display message-box and retu rn with an error-code
	::MessageBox(0,tszMessage,_T("Listener"), MB_ICONSTOP|MB_OK);
	return 1;
}

// ======================= F U N C I O N E S   A D I C I O N A L E S ========================//

// == ACCION  DE GIRO (TURNS) ===============================================================//

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
  int myTurning; // -1 == izq, 1 == der, 0 == ninguna
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
    ArLog::log(ArLog::Terse, "No sonares encontrados, desactivando.");
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

  // return a pointer to the actionDesired, so resolver knows what to do
  return &myDesired;
}

// == ACCION  DE IDA(GOS) ===============================================================//
/* 
 * Action that drives the robot forward, but stops if obstacles are
 * detected by sonar. 
 */
class ActionGos : public ArAction
{
public:
  // constructor, sets myMaxSpeed and myStopDistance
  ActionGos(double maxSpeed, double stopDistance);
  // destructor. does not need to do anything
  virtual ~ActionGos(void) {};
  // called by the action resolver to obtain this action's requested behavior
  virtual ArActionDesired *fire(ArActionDesired currentDesired);
  // store the robot pointer, and it's ArSonarDevice object, or deactivate this action if there is no sonar.
  virtual void setRobot(ArRobot *robot);
protected:
  // the sonar device object obtained from the robot by setRobot()
  ArRangeDevice *mySonar;

  /* Our current desired action: fire() modifies this object and returns
      to the action resolver a pointer to this object.
      This object is kept as a class member so that it persists after fire()
      returns (otherwise fire() would have to create a new object each invocation,
      but would never be able to delete that object).
  */
  ArActionDesired myDesired;

  double myMaxSpeed;
  double myStopDistance;
  public: int myFound;
};

/*
  Note the use of constructor chaining with 
  ArAction(actionName). Also note how it uses setNextArgument, which makes it so that 
  other parts of the program could find out what parameters this action has, and possibly modify them.
*/
ActionGos::ActionGos(double maxSpeed, double stopDistance) :
  ArAction("Go")
{
  mySonar = NULL;
  myMaxSpeed = maxSpeed;
  myStopDistance = stopDistance;
  setNextArgument(ArArg("maximum speed", &myMaxSpeed, "Maximum speed to go."));
  setNextArgument(ArArg("stop distance", &myStopDistance, "Distance at which to stop."));
  myFound = 0;
}

/*
  Override ArAction::setRobot() to get the sonar device from the robot, or deactivate this action if it is missing.
  You must also call ArAction::setRobot() to properly store
  the ArRobot pointer in the ArAction base class.
*/
void ActionGos::setRobot(ArRobot *robot)
{
  ArAction::setRobot(robot);
  mySonar = robot->findRangeDevice("sonar");
  if (robot == NULL)
    {
      ArLog::log(ArLog::Terse, "actionExample: ActionGo: Warning: I found no sonar, deactivating.");
      deactivate();
    }
}

/*
  This fire is the whole point of the action.
  currentDesired is the combined desired action from other actions
  previously processed by the action resolver.  In this case, we're
  not interested in that, we will set our desired 
  forward velocity in the myDesired member, and return it.

  Note that myDesired must be a class member, since this method
  will return a pointer to myDesired to the caller. If we had
  declared the desired action as a local variable in this method,
  the pointer we returned would be invalid after this method
  returned.
*/
ArActionDesired *ActionGos::fire(ArActionDesired currentDesired)
{
  double range;
  double speed;

  // reset the actionDesired (must be done), to clear
  // its previous values.
  myDesired.reset();

  // if the sonar is null we can't do anything, so deactivate
  if (mySonar == NULL)
  {
    deactivate();
    return NULL;
  }
  // get the range of the sonar
  range = mySonar->currentReadingPolar(-70, 70) - myRobot->getRobotRadius();
  // if the range is greater than the stop distance, find some speed to go
  if (range > myStopDistance)
  {
    // just an arbitrary speed based on the range
    speed = range * .3;
    // if that speed is greater than our max, cap it
    if (speed > myMaxSpeed)
      speed = myMaxSpeed;
    // now set the velocity
    myDesired.setVel(speed);
  }
  // the range was less than the stop distance, so request stop
  else
  {
    myDesired.setVel(0);
	myFound = 1;
  }
  // return a pointer to the actionDesired to the resolver to make our request
  return &myDesired;
}

// ================ F I N   F U N C I O N E S    A D I C I O N A L E S ================ //

int __cdecl _tmain (int argc, char** argv)
{

	//------------ I N I C I O   M A I N    D E L   P R O G R A M A   D E L    R O B O T-----------//

	  //inicializaion de variables
	  Aria::init();
	  ArArgumentParser parser(&argc, argv);
	  parser.loadDefaultArguments();
	  ArSimpleConnector simpleConnector(&parser);
	  ArRobot robot;
	  ArSonarDevice sonar;
	  ArAnalogGyro gyro(&robot);
	  robot.addRangeDevice(&sonar);
	  ActionGos go(500, 350);	  
	  robot.addAction(&go, 48);
	  ActionTurns turn(400, 110);
	  robot.addAction(&turn, 49);
	  ActionTurns turn2(400, 110);
	  robot.addAction(&turn2, 49);

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

	  // ============================ INICIO CONFIG COM =================================//
	    CSerial serial;
		LONG    lLastError = ERROR_SUCCESS;

		// Trata de abrir el com seleccionado
		lLastError = serial.Open(_T("COM3"),0,0,false);
		if (lLastError != ERROR_SUCCESS)
			return ::ShowError(serial.GetLastError(), _T("Imposible abrir el COM"));

		// Inicia el puerto serial (9600,8N1)
		lLastError = serial.Setup(CSerial::EBaud9600,CSerial::EData8,CSerial::EParNone,CSerial::EStop1);
		if (lLastError != ERROR_SUCCESS)
			return ::ShowError(serial.GetLastError(), _T("Imposible setear la config del COM"));

		// Register only for the receive event
		lLastError = serial.SetMask(CSerial::EEventBreak |
									CSerial::EEventCTS   |
									CSerial::EEventDSR   |
									CSerial::EEventError |
									CSerial::EEventRing  |
									CSerial::EEventRLSD  |
									CSerial::EEventRecv);
		if (lLastError != ERROR_SUCCESS)
			return ::ShowError(serial.GetLastError(), _T("Unable to set COM-port event mask"));

		// Use 'non-blocking' reads, because we don't know how many bytes
		// will be received. This is normally the most convenient mode
		// (and also the default mode for reading data).
		lLastError = serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);
		if (lLastError != ERROR_SUCCESS)
			return ::ShowError(serial.GetLastError(), _T("Unable to set COM-port read timeout."));
		// ============================ FIN CONFIG COM =================================//

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
			// Create the sound queue.
			ArSoundsQueue soundQueue;
			// Run the sound queue in a new thread
			soundQueue.runAsync();
			std::vector<const char*> filenames;
			filenames.push_back("sound-r2a.wav");
			soundQueue.play(filenames[0]);
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
			  robot.unlock();
			  //Valor para el while
			  bool fContinue = true;
				// <<<<<<------------- 1 Parte Secuencia: BAJA BRAZO ------------->>>>>> //
				lLastError = serial.Write("b");
				if (lLastError != ERROR_SUCCESS)
					return ::ShowError(serial.GetLastError(), _T("Unable to send data"));

				//-------------------------E S C U C H A   C O M ----------------------------//
				do
				{
					// Wait for an event
					lLastError = serial.WaitEvent();
					if (lLastError != ERROR_SUCCESS)
						return ::ShowError(serial.GetLastError(), _T("Unable to wait for a COM-port event."));

					// Save event
					const CSerial::EEvent eEvent = serial.GetEventType();

					// Handle break event
					if (eEvent & CSerial::EEventBreak)
					{
						printf("\n### BREAK received ###\n");
					}

					// Handle CTS event
					if (eEvent & CSerial::EEventCTS)
					{
						printf("\n### Clear to send %s ###\n", serial.GetCTS()?"on":"off");
					}

					// Handle DSR event
					if (eEvent & CSerial::EEventDSR)
					{
						printf("\n### Data set ready %s ###\n", serial.GetDSR()?"on":"off");
					}

					// Handle error event
					if (eEvent & CSerial::EEventError)
					{
						printf("\n### ERROR: ");
						switch (serial.GetError())
						{
						case CSerial::EErrorBreak:		printf("Break condition");			break;
						case CSerial::EErrorFrame:		printf("Framing error");			break;
						case CSerial::EErrorIOE:		printf("IO device error");			break;
						case CSerial::EErrorMode:		printf("Unsupported mode");			break;
						case CSerial::EErrorOverrun:	printf("Buffer overrun");			break;
						case CSerial::EErrorRxOver:		printf("Input buffer overflow");	break;
						case CSerial::EErrorParity:		printf("Input parity error");		break;
						case CSerial::EErrorTxFull:		printf("Output buffer full");		break;
						default:						printf("Unknown");					break;
						}
						printf(" ###\n");
					}

					// Handle ring event
					if (eEvent & CSerial::EEventRing)
					{
						printf("\n### RING ###\n");
					}

					// Handle RLSD/CD event
					if (eEvent & CSerial::EEventRLSD)
					{
						printf("\n### RLSD/CD %s ###\n", serial.GetRLSD()?"on":"off");
					}

					// Handle data receive event
					if (eEvent & CSerial::EEventRecv)
					{
						// Read data, until there is nothing left
						DWORD dwBytesRead = 0;
						char szBuffer[101];
						do
						{
							// Lee datos del Puerto COM
							lLastError = serial.Read(szBuffer,sizeof(szBuffer)-1,&dwBytesRead);
							if (lLastError != ERROR_SUCCESS)
								return ::ShowError(serial.GetLastError(), _T("Unable to read from COM-port."));

							if (dwBytesRead > 0)
							{
								//Preseteo color
								int color = 0;
								// Finaliza el dato, asi que sea una string valida
								szBuffer[dwBytesRead] = '\0';
								// Display the data
								printf("%s", szBuffer);

								// <<<<<<----------- 2 Parte Secuencia: CIERRA GRIPPER ----------->>>>>> //
								if (strchr(szBuffer,76))
								{
									lLastError = serial.Write("c");
									if (lLastError != ERROR_SUCCESS)
										return ::ShowError(serial.GetLastError(), _T("Unable to send data"));
								}
								
								// <<<<<<------------- 3 Parte Secuencia: SUBE BRAZO ------------->>>>>> //
								if (strchr(szBuffer,117))
								{
									lLastError = serial.Write("s");
									if (lLastError != ERROR_SUCCESS)
										return ::ShowError(serial.GetLastError(), _T("Unable to send data"));
								}

								// <<<<<<------------- 4 Parte Secuencia: COLOR ------------->>>>>> //
								if (strchr(szBuffer,72))
								{
									lLastError = serial.Write("C");
									if (lLastError != ERROR_SUCCESS)
										return ::ShowError(serial.GetLastError(), _T("Unable to send data"));
								}

								// <<<<<<---------- 5.1 Parte Secuencia: COLOR ROJO---------->>>>>> //
								if (strchr(szBuffer,82))
								{
									color = 1;
									//salir del bucle
									fContinue = false;
								}

								// <<<<<<---------- 5.2 Parte Secuencia: COLOR AZUL ---------->>>>>> //
								if (strchr(szBuffer,66))
								{
									color = 2;
									//salir del bucle
									fContinue = false;
								}

								// <<<<<<---------- 5.3 Parte Secuencia: COLOR VERDE ---------->>>>>> //
								if (strchr(szBuffer,71))
								{
									color = 3;
									//salir del bucle
									fContinue = false;
								}
							}
						}
						while (dwBytesRead == sizeof(szBuffer)-1);
					}
				}
				while (fContinue);
				// Close the port again
				serial.Close();
				robot.lock();
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

	//------------ F I N   M A I N    D E L   P R O G R A M A   D E L    R O B O T-----------//
    
    return 0;
}
