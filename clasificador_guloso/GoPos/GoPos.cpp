// Proyecto : Guloso-clasificador - Pioneer P3-DX
// Nombre : ventana-robot, dentro de un mapa por puntos, alcanza los puntos designados 
//          describiendo una ruta en el suelo de mapeo simple
// Creado : 03.01.2011
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
//

#include "stdafx.h"
#include "VentanaRobot.h"
#include "Aria.h"
#include <windows.h> // esta libreria es la que nos permite usar Sleep()
#include <stdio.h>

#define MAX_LOADSTRING 100

//Variables gloabales:
int argc;
char** argv;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

  //-------------- M A I N    D E L   P R O G R A M A   D E L    R O B O T------------//
  //----------------------------------------------------------------------------------------------------------
	
  //inicializaion de variables
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArSimpleConnector simpleConnector(&parser);
  ArRobot robot;
  ArSonarDevice sonar;
  ArAnalogGyro gyro(&robot);
  robot.addRangeDevice(&sonar);

  // presionar tecla escape para salir del programa
  ArKeyHandler keyHandler;
  Aria::setKeyHandler(&keyHandler);
  robot.attachKeyHandler(&keyHandler);
  printf("You may press escape to exit\n");

  // uso de sonares para evitar colisiones con las paredes u 
  // obstaculos grandes, mayores a 8cm de alto
  ArActionLimiterForwards limiterAction("speed limiter near", 300, 600, 250);
  ArActionLimiterForwards limiterFarAction("speed limiter far", 300, 1100, 400);
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

  const int duration = 100000; //msec
  ArLog::log(ArLog::Normal, "Completados los puntos en %d segundos", duration/1000);

  bool first = true;
  int horiz = 1800;
  int vert = 380;
  int goalNum = 0;
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
      if (goalNum > 7)
        goalNum = 1;

	  //comienza la secuencia de puntos
      if (goalNum == 1)
        gotoPoseAction.setGoal(ArPose(horiz, vert*0));
      else if (goalNum == 2)
        gotoPoseAction.setGoal(ArPose(0, vert*1));
      else if (goalNum == 3)
        gotoPoseAction.setGoa l(ArPose(horiz, vert*2)+5);
      else if (goalNum == 4)
        gotoPoseAction.setGoal(ArPose(0, vert*3));
	  else if (goalNum == 5)
        gotoPoseAction.setGoal(ArPose(horiz, vert*4+5));
	  else if (goalNum == 6)
        gotoPoseAction.setGoal(ArPose(0, vert*5));
	  else if (goalNum == 7)
        gotoPoseAction.setGoal(ArPose(0, vert*0));

      ArLog::log(ArLog::Normal, "Siguiente punto en %.0f %.0f", 
		    gotoPoseAction.getGoal().getX(), gotoPoseAction.getGoal().getY());
    }

    if(start.mSecSince() >= duration) {
      ArLog::log(ArLog::Normal, "%d seconds have elapsed. Cancelling current goal, waiting 3 seconds, and exiting.", duration/1000);
      gotoPoseAction.cancelGoal();
      robot.unlock();
      ArUtil::sleep(3000);
      break;
    }
    
    robot.unlock();
    ArUtil::sleep(100);
  }

  // Robot desconectado al terminal el sleep
  Aria::shutdown();
  return 0;

//----------------------------------------------------------------------------------------------------------

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_VENTANAROBOT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VENTANAROBOT));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VENTANAROBOT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_VENTANAROBOT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
