// Proyecto : Guloso-clasificador - Pioneer P3-DX
// Nombre : consola - prueba de movilidad por secuencias con delays
// Creado : 05.01.2011
// Autor : Marco Flores
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

#include "stdafx.h"
#include "consola.h"
#include "Aria.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;


//------------- H A N D L E R   D E   L A   C O N E X I O N ---------------//
//<----------------------------------------------------------------------->//

class ConnHandler //Clase que determina la estructura del handler
{
public:
  // Constructor
  ConnHandler(ArRobot *robot);
  // Destructor
  ~ConnHandler(void) {}
  // llama cuando se conecta
  void connected(void);
  // llama cuando no conecta
  void connFail(void);
  // llama cuando se desconecta
  void disconnected(void);
protected:
  // puntero ** Tiene funciones especiales **
  ArRobot *myRobot;
  // functors de las conexiones del robot
  ArFunctorC<ConnHandler> myConnectedCB;
  ArFunctorC<ConnHandler> myConnFailCB;
  ArFunctorC<ConnHandler> myDisconnectedCB;
};

ConnHandler::ConnHandler(ArRobot *robot) :
  myConnectedCB(this, &ConnHandler::connected),  
  myConnFailCB(this, &ConnHandler::connFail),
  myDisconnectedCB(this, &ConnHandler::disconnected)

{
  myRobot = robot;
  myRobot->addConnectCB(&myConnectedCB, ArListPos::FIRST);
  myRobot->addFailedConnectCB(&myConnFailCB, ArListPos::FIRST);
  myRobot->addDisconnectNormallyCB(&myDisconnectedCB, ArListPos::FIRST);
  myRobot->addDisconnectOnErrorCB(&myDisconnectedCB, ArListPos::FIRST);
}

// salir si se pierde la conexion
void ConnHandler::connFail(void)
{
  printf("Fallo al conectar\n");
  myRobot->stopRunning();
  Aria::shutdown();
  return;
}

// encender motores (prepara), on el sonar, off sonidos del amigobot
void ConnHandler::connected(void)
{
  printf("Conectado\n");
  myRobot->comInt(ArCommands::SONAR, 1);
  myRobot->comInt(ArCommands::ENABLE, 1);
  myRobot->comInt(ArCommands::SOUNDTOG, 0);
}

// desconectar
void ConnHandler::disconnected(void)
{
  printf("Conexion perdida\n");
  exit(0);
}
//<----------------------------------------------------------------------->//


int main(int argc, char** argv)
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("MFC no iniciada\n"));
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.
		
	//------------------- M A I N    D E L    R O B O T -----------------------//
	//<----------------------------------------------------------------------->//

	// Inicia la conexion del Aria para el robot
	Aria::init();
	// Creo los objetos (el parser, el conector, el robot, y el handler que creamos arriba)
	ArArgumentParser argParser(&argc, argv);
	ArSimpleConnector con(&argParser);
	ArRobot robot;
	ConnHandler ch(&robot);
	
	// Parseo los datos
	if(!Aria::parseArgs())
	{
		Aria::logOptions();
		Aria::shutdown();
		return 1;
	}

	// Conecto con el robot (1)
	if(!con.connectRobot(&robot)) //no conectado
	{
		ArLog::log(ArLog::Normal, "No se pudo conectar con el robot, saliendo.");
		return 1;
	}
	ArLog::log(ArLog::Normal, "Conectado"); //conectado

	// ********** Inicio el run del robot de forma Asíncrona ********* //
	// Corre el ciclo del robot en un hilo propio, por eso hay que poner 
	// lock() y unlock() antes y despues de cada accion con ArRobot para
	// prevenir conflictos.
	robot.runAsync(false);

	// Imprimo algunos datos del robot como posicion velocidad y bateria
	robot.lock();
	ArLog::log(ArLog::Normal, "Posicion=(%.2f,%.2f,%.2f), Trans. Vel=%.2f, Bateria=%.2fV",
		robot.getX(), robot.getY(), robot.getTh(), robot.getVel(), robot.getBatteryVoltage());
	robot.unlock();

	  // Send the robot a series of motion commands directly, sleeping for a 
	  // few seconds afterwards to give the robot time to execute them.
	  printf("Velocidad rotacional a 10deg/s durante 3 segundos.\n");
	  robot.lock();
	  robot.setRotVel(100);
	  robot.unlock();
	  ArUtil::sleep(3*1000);
	  printf("Stopping\n");
	  robot.lock();
	  robot.setRotVel(0);
	  robot.unlock();
	  ArUtil::sleep(200);

	  printf("directMotionExample: Telling the robot to go 300 mm on left wheel and 100 mm on right wheel for 5 seconds\n");
	  robot.lock();
	  robot.setVel2(300, 100);
	  robot.unlock();
	  ArTime start;
	  start.setToNow();
	  while (1)
	  {
		robot.lock();
		if (start.mSecSince() > 5000)
		{
		  robot.unlock();
		  break;
		}   
		robot.unlock();
		ArUtil::sleep(50);
	  }
	  
	  printf("directMotionExample: Telling the robot to move forwards one meter, then sleeping 5 seconds\n");
	  robot.lock();
	  robot.move(1000);
	  robot.unlock();
	  start.setToNow();
	  while (1)
	  {
		robot.lock();
		if (robot.isMoveDone())
		{
		  printf("directMotionExample: Finished distance\n");
		  robot.unlock();
		  break;
		}
		if (start.mSecSince() > 5000)
		{
		  printf("directMotionExample: Distance timed out\n");
		  robot.unlock();
		  break;
		}   
		robot.unlock();
		ArUtil::sleep(50);
	  }
	  printf("directMotionExample: Telling the robot to move backwards one meter, then sleeping 5 seconds\n");
	  robot.lock();
	  robot.move(-1000);
	  robot.unlock();
	  start.setToNow();
	  while (1)
	  {
		robot.lock();
		if (robot.isMoveDone())
		{
		  printf("directMotionExample: Finished distance\n");
		  robot.unlock();
		  break;
		}
		if (start.mSecSince() > 10000)
		{
		  printf("directMotionExample: Distance timed out\n");
		  robot.unlock();
		  break;
		}
		robot.unlock();
		ArUtil::sleep(50);
	  }
	  printf("directMotionExample: Telling the robot to turn to 180, then sleeping 4 seconds\n");
	  robot.lock();
	  robot.setHeading(180);
	  robot.unlock();
	  start.setToNow();
	  while (1)
	  {
		robot.lock();
		if (robot.isHeadingDone(5))
		{
		  printf("directMotionExample: Finished turn\n");
		  robot.unlock();
		  break;
		}
		if (start.mSecSince() > 5000)
		{
		  printf("directMotionExample: Turn timed out\n");
		  robot.unlock();
		  break;
		}
		robot.unlock();
		ArUtil::sleep(100);
	  }
	  printf("directMotionExample: Telling the robot to turn to 90, then sleeping 2 seconds\n");
	  robot.lock();
	  robot.setHeading(90);
	  robot.unlock();
	  start.setToNow();
	  while (1)
	  {
		robot.lock();
		if (robot.isHeadingDone(5))
		{
		  printf("directMotionExample: Finished turn\n");
		  robot.unlock();
		  break;
		}
		if (start.mSecSince() > 5000)
		{
		  printf("directMotionExample: turn timed out\n");
		  robot.unlock();
		  break;
		}
		robot.unlock();
		ArUtil::sleep(100);
	  }
	  printf("directMotionExample: Setting vel2 to 200 mm/sec on both wheels, then sleeping 3 seconds\n");
	  robot.lock();
	  robot.setVel2(200, 200);
	  robot.unlock();
	  ArUtil::sleep(3000);
	  printf("directMotionExample: Stopping the robot, then sleeping for 2 seconds\n");
	  robot.lock();
	  robot.stop();
	  robot.unlock();
	  ArUtil::sleep(2000);
	  printf("directMotionExample: Setting velocity to 200 mm/sec then sleeping 3 seconds\n");
	  robot.lock();
	  robot.setVel(200);
	  robot.unlock();
	  ArUtil::sleep(3000);
	  printf("directMotionExample: Stopping the robot, then sleeping for 2 seconds\n");
	  robot.lock();
	  robot.stop();
	  robot.unlock();
	  ArUtil::sleep(2000);
	  printf("directMotionExample: Setting vel2 with 0 on left wheel, 200 mm/sec on right, then sleeping 5 seconds\n");
	  robot.lock();
	  robot.setVel2(0, 200);
	  robot.unlock();
	  ArUtil::sleep(5000);
	  printf("directMotionExample: Telling the robot to rotate at 50 deg/sec then sleeping 5 seconds\n");
	  robot.lock();
	  robot.setRotVel(50);
	  robot.unlock();
	  ArUtil::sleep(5000);
	  printf("directMotionExample: Telling the robot to rotate at -50 deg/sec then sleeping 5 seconds\n");
	  robot.lock();
	  robot.setRotVel(-50);
	  robot.unlock();
	  ArUtil::sleep(5000);
	  printf("directMotionExample: Setting vel2 with 0 on both wheels, then sleeping 3 seconds\n");
	  robot.lock();
	  robot.setVel2(0, 0);
	  robot.unlock();
	  ArUtil::sleep(3000);
	  printf("directMotionExample: Now having the robot change heading by -125 degrees, then sleeping for 6 seconds\n");
	  robot.lock();
	  robot.setDeltaHeading(-125);
	  robot.unlock();
	  ArUtil::sleep(6000);
	  printf("directMotionExample: Now having the robot change heading by 45 degrees, then sleeping for 6 seconds\n");
	  robot.lock();
	  robot.setDeltaHeading(45);
	  robot.unlock();
	  ArUtil::sleep(6000);
	  printf("directMotionExample: Setting vel2 with 200 on left wheel, 0 on right wheel, then sleeping 5 seconds\n");
	  robot.lock();
	  robot.setVel2(200, 0);
	  robot.unlock();
	  ArUtil::sleep(5000);

	  printf("Terminado, cerrando programa\n");
	  Aria::shutdown();
	  return 0;
	//<----------------------------------------------------------------------->//

	}

	return nRetCode;
}
