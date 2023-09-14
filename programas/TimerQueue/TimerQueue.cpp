//  Automação em tempo Real - ELT012 - UFMG
//  
//  EXEMPLO 8 - Temporização via "Timer Queues"
//  ---------------------------------------------------------------
//
//  Versão 1.0 - 20/04/2010 - Prof. Luiz T. S. Mendes
//
//	Este programa exemplifica o uso de  "Timer Queues" para fins de temporização,
//  e baseia-se integralmente no Programa 4.5 do livro "Programação Concorrente
//  em ambiente Windows", de C. S. Filho e M. Szuster. As únicas diferenças
//  com relação a este último são:
//  - O uso de "Timer Queues" ao invés "Timers Multimídia";
//  - A thread secundária enfileira uma segunda rotina callback (TimerFunc2)
//    na fila de temporizadores, para fins de demonstração da fila de mensagens.
//
//  A API de "Queue Timers" só existe nas versões do Microsoft Windows 2000 em
//  diante, portanto este programa não compilará em versões anteriores dos S.O.
//  Microsoft.
//

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>		// _getch
#include <ctype.h>		// _isdigit
#include <process.h>	// _beginthreadex() e _endthreadex() 

#include "bGetFloat.h"	// le valor floating point

#define _CHECKERROR	1	// Ativa função CheckForError
#include "CheckForError.h"

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI *CAST_FUNCTION)(LPVOID);
typedef unsigned *CAST_LPDWORD;

#define	ESC			0x1B
#define CR			0x0D

HANDLE hEvent;			// Handle para Evento
HANDLE hEscEvent;		// Handle para Evento Aborta

DWORD WINAPI PidControlFunc(LPVOID);	  // declaração da função
void CALLBACK Pid(PVOID, BOOLEAN);	      // Função PID
void CALLBACK TimerFunc2(PVOID, BOOLEAN); // Função de temporização secundária
void CALLBACK TimerFunc3(PVOID, BOOLEAN); // Função de temporização secundária

DOUBLE SetPoint= 0.0;
DOUBLE dInput;

SYSTEMTIME tempo;

int main()
{
	HANDLE hThread;
	
	DWORD dwThreadId;
	DWORD dwExitCode = 0;
	BOOL  bStatus;

	// apenas uma thread é acordada a cada pulso
	hEvent		= CreateEvent(NULL, FALSE, FALSE,"Evento");
	CheckForError(hEvent);
	// todas as threads são acordadas a cada pulso
	hEscEvent	= CreateEvent(NULL, TRUE, FALSE,"EscEvento");
	CheckForError(hEscEvent);
	// Cria controlador PID
	hThread = (HANDLE) _beginthreadex(
			NULL,
			0,
			(CAST_FUNCTION)PidControlFunc,	// casting necessário
			(LPVOID)0,
			0,
			(CAST_LPDWORD)&dwThreadId		// casting necessário
		);
	CheckForError(hThread);
	
	do {
		printf("\nEscreva novo valor de SetPoint:\n");
		// le string ou ESC
		bStatus = bGetFloat(&dInput, 9);
		if (bStatus) PulseEvent(hEvent); // SetPoint mudou
		else PulseEvent(hEscEvent);		// Termina threads
	} while (bStatus);

	// Espera thread terminar
	if (WaitForSingleObject(hThread, INFINITE) != WAIT_OBJECT_0) 
		printf ("Erro em WaitForSingleObject! Codigo = %d\n", (int) GetLastError);
	GetExitCodeThread(hThread, &dwExitCode);

	CloseHandle(hThread);	// apaga referência ao objeto
	CloseHandle(hEvent);
	CloseHandle(hEscEvent);

	return EXIT_SUCCESS;
}  // main


DWORD WINAPI PidControlFunc(LPVOID id)
{	
	HANDLE Events[2]= {hEvent, hEscEvent};
	DWORD ret;
	int TipoEvento= 0;

	HANDLE hTimerQueue;
	HANDLE hTimerID1, hTimerID2, hTimerID3;
	BOOL status;
	
	// Cria fila de temporizadores
	hTimerQueue = CreateTimerQueue();
	if (hTimerQueue == NULL){
        printf("Falha em CreateTimerQueue! Codigo =%d)\n", GetLastError());
        return 0;
    }
	// Enfileira primeiro temporizador com sua função callback
	status = CreateTimerQueueTimer(&hTimerID1, hTimerQueue, (WAITORTIMERCALLBACK)Pid,
								   NULL, 7000, 5000, WT_EXECUTEDEFAULT);
								   //NULL, 1000, 1000, WT_EXECUTEDEFAULT);
	if (!status){
        printf("Erro em CreateTimerQueueTimer [1]! Codigo = %d)\n", GetLastError());
        return 0;
    }
    // Enfileira segundo temporizador com sua função callback
	status = CreateTimerQueueTimer(&hTimerID2, hTimerQueue, (WAITORTIMERCALLBACK)TimerFunc2,
								   NULL, 1000, 1000, WT_EXECUTEDEFAULT);
	if (!status){
        printf("Erro em CreateTimerQueueTimer [2]! Codigo = %d)\n", GetLastError());
        return 0;
	}

    // Enfileira segundo temporizador com sua função callback
	status = CreateTimerQueueTimer(&hTimerID3, hTimerQueue, (WAITORTIMERCALLBACK)TimerFunc3,
								   NULL, 3000, 3000, WT_EXECUTEDEFAULT);
	if (!status){
        printf("Erro em CreateTimerQueueTimer [3]! Codigo = %d)\n", GetLastError());
        return 0;
    }

	do {// observe: quando ocorre evento a temporização é perdida !  
		ret=WaitForMultipleObjects(2, Events, FALSE, INFINITE);
		TipoEvento = ret - WAIT_OBJECT_0;
		if (TipoEvento == 0){// Novo valor de Pid
			SetPoint= dInput;
			printf("Novo valor de set point: %6.2f\n", SetPoint);
		}
	} while (TipoEvento != 1);	// Esc foi escolhido

	// Cancela a fila de temporizadores
	if (!DeleteTimerQueueEx(hTimerQueue, NULL))
        printf("Falha em DeleteTimerQueue! Codigo = %d\n", GetLastError());
	
	printf("Thread Pid terminando...\n");	

	printf("\nAcione uma tecla para terminar\n");
	_getch(); // // Pare aqui, caso não esteja executando no ambiente MDS

	_endthreadex((DWORD) 0);

	return 0;
} // WaitEventFunc

void CALLBACK Pid(PVOID nTimerID, BOOLEAN TimerOrWaitFired)
{	// Algoritmo Pid;
	//printf("Pid rodando... SP= %6.2f\n", SetPoint);
	GetLocalTime(&tempo);
	printf("Pid rodando... SP= %6.2f  %02d:%02d:%02d\n", SetPoint, tempo.wHour,
		   tempo.wMinute, tempo.wSecond);
};	// Pid

void CALLBACK TimerFunc2(PVOID nTimerID, BOOLEAN TimerOrWaitFired){
	// Rotina callback para o segundo temporizador
	GetLocalTime(&tempo);
	printf("Rotina Secundaria executando... %02d:%02d:%02d\n", tempo.wHour,
		   tempo.wMinute, tempo.wSecond);
};	// Pid
void CALLBACK TimerFunc3(PVOID nTimerID, BOOLEAN TimerOrWaitFired){
	// Rotina callback para o segundo temporizador
	GetLocalTime(&tempo);
	printf("Rotina Terciaria executando... %02d:%02d:%02d\n", tempo.wHour,
		   tempo.wMinute, tempo.wSecond);
};	// Pid
