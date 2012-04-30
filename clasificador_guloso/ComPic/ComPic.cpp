// Proyecto : Guloso-clasificador - Pioneer P3-DX
// Nombre : ventana-robot, dentro de un mapa por puntos, alcanza los puntos designados 
//          describiendo una ruta en el suelo de mapeo simple
// Creado : 03.01.2011
// Autor : Marco Flores
// Version : 1.0
//
//	Copyright (C) 1999-2003 Ramon de Klein (Ramon.de.Klein@ict.nl)
// * AVISO IMPORTANTE * Este programa es propiedad (c)2010 Marco Flores (edicion)
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

#define STRICT
#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "Serial.h"

enum { EOF_Char = 78 };

int ShowError (LONG lError, LPCTSTR lptszMessage)
{
	// Generate a message text
	TCHAR tszMessage[256];
	wsprintf(tszMessage,_T("%s\n(error code %d)"), lptszMessage, lError);
	// Display message-box and retu rn with an error-code
	::MessageBox(0,tszMessage,_T("Listener"), MB_ICONSTOP|MB_OK);
	return 1;
}

int __cdecl _tmain (int /*argc*/, char** /*argv*/)
{
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
/*
	// Setup handshaking (default is no handshaking)
	lLastError = serial.SetupHandshaking(CSerial::EHandshakeOff);
	if (lLastError != ERROR_SUCCESS)
		return ::ShowError(serial.GetLastError(), _T("Unable to set COM-port handshaking"));
*/
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

    // Keep reading data, until an EOF (CTRL-Z) has been received
	bool fContinue = true;

	// --------------P R U E B A--------------- //

	lLastError = serial.Write("1");
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
				// Read data from the COM-port
				lLastError = serial.Read(szBuffer,sizeof(szBuffer)-1,&dwBytesRead);
				if (lLastError != ERROR_SUCCESS)
					return ::ShowError(serial.GetLastError(), _T("Unable to read from COM-port."));

				if (dwBytesRead > 0)
				{
					// Finalize the data, so it is a valid string
					szBuffer[dwBytesRead] = '\0';

					// Display the data
					printf("%s", szBuffer);

					// Check if EOF (CTRL+'[') has been specified
					if (strchr(szBuffer,EOF_Char))
					{
						//salir del bucle
						fContinue = false;
						printf("\n--------------->NOOOOOOOOOOOOOOOOOOOOO!!!!!<------------------");
						// The serial port is now ready and we can send/receive data. If
						// the following call blocks, then the other side doesn't support
						// hardware handshaking.
					}
				}
			}
		    while (dwBytesRead == sizeof(szBuffer)-1);
		}
	}
	while (fContinue);

    // Close the port again
    serial.Close();
    return 0;
}
