/*********************************************************************************
*
*  AUTOMAÇÃO EM TEMPO REAL - ELT012
*  Prof. Luiz T. S. Mendes - 2021/2
*
*  Atividade em classe: "O problema barbeiro dorminhoco"
*
*  Este programa deve ser completado com as linhas de programa necessárias
*  para solucionar o "problema do banheiro dorminhoco" ("The Sleeping Barber").
*
*  O programa é composto de uma thread primária e 11 threads secundárias. A thread
*  primária cria os objetos de sincronização e as threads secundárias. As threads
*  secundárias correspondem ao barbeiro e a 10 clientes.
*
* A sinalização de término de programa é feita através da tecla ESC. Leituras da
* última tecla digitada devem ser feitas em pontos apropriados para que as threads
* detectem esta tecla. Este tipo de sincronização, contudo, irá falhar sempre
* que o ESC for digitado e as threads secundárias estiverem bloqueadas na função
* WaitForSingleObject().
*
**********************************************************************************/

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>						// _beginthreadex() e _endthreadex()
#include <conio.h>							// _getch

#include "CheckForError.h"

typedef unsigned (WINAPI *CAST_FUNCTION)(LPVOID);	// Casting para terceiro e sexto parâmetros da função
                                                    // _beginthreadex
typedef unsigned *CAST_LPDWORD;

#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN      | FOREGROUND_BLUE
#define HLGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define HLRED   FOREGROUND_RED   | FOREGROUND_INTENSITY

#define	ESC				0x1B			// Tecla para encerrar o programa
#define N_CLIENTES		10			// Número de clientes
#define N_LUGARES       5           // Número de cadeiras (4 de espera e 1 de barbear)

DWORD WINAPI ThreadBarbeiro();		// Thread´representando o babrbeiro
DWORD WINAPI ThreadCliente(int);		// Thread representando o cliente

void FazABarbaDoCliente(int);		// Função que simula o ato de fazer a barba
void TemABarbaFeita(int);			// Função que simula o ato de ter a barba feita

int n_clientes = 0;					// Contador de clientes
HANDLE hBarbeiroLivre;				// Semáforo para indicar ao cliente que o barbeiro está livre
HANDLE hAguardaCliente;				// Semáforo para indicar ao barbeiro que chegou um cliente
HANDLE hMutex;						// Permite acesso exclusicvo à variável n_clientes

int nTecla;							// Variável que armazena a tecla digitada para sair
int id_cliente;                     // Identificador do cliente

HANDLE hOut;						// Handle para a saída da console

// THREAD PRIMÁRIA
int main(){

	HANDLE hThreads[N_CLIENTES+1];       // N clientes mais o barbeiro
	DWORD dwIdBarbeiro, dwIdCliente;
	DWORD dwExitCode = 0;
	DWORD dwRet;
	int i;

	// Obtém um handle para a saída da console
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		printf("Erro ao obter handle para a saída da console\n");

	// Cria objetos de sincronização
    // [INSIRA AQUI OS COMANDOS DE CRIAÇÃO DO MUTEX / SEMÁFOROS]
	hMutex = CreateMutex(NULL, FALSE, "N_CLIENTES");
	CheckForError(hMutex);

	hBarbeiroLivre = CreateSemaphore(NULL, 1, 1, "BARBEIRO_LIVRE");
	CheckForError(hBarbeiroLivre);

	hAguardaCliente = CreateSemaphore(NULL, 1, 1, "AGUARDA_CLIENTE");
	CheckForError(hAguardaCliente);

	// Criação de threads
	// Note que _beginthreadex() retorna -1L em caso de erro
	for (i=0; i<N_CLIENTES; ++i) {
		hThreads[i] = (HANDLE) _beginthreadex(
						       NULL,
							   0,
							   (CAST_FUNCTION) ThreadCliente,	//Casting necessário
							   (LPVOID)(INT_PTR)i,
							   0,								
							   (CAST_LPDWORD)&dwIdCliente);		//Casting necessário
		SetConsoleTextAttribute(hOut, WHITE);
		if (hThreads[i] != (HANDLE) -1L)
			printf("Thread Cliente %d criada com Id=%0x\n", i, dwIdCliente);
		else {
			printf("Erro na criacao da thread Cliente! N = %d Codigo = %d\n", i, errno);
			exit(0);
		}
	}//for

	hThreads[N_CLIENTES] = (HANDLE) _beginthreadex(
					       NULL,
						   0,
						   (CAST_FUNCTION) ThreadBarbeiro,	//Casting necessário
						   (LPVOID)(INT_PTR)i,
						   0,								
						   (CAST_LPDWORD)&dwIdBarbeiro);		//Casting necessário
	SetConsoleTextAttribute(hOut, WHITE);
	if (hThreads[N_CLIENTES] != (HANDLE)-1L)
		printf("Thread Barbeiro  %d criada com Id=%0x\n", i, dwIdBarbeiro);
	else {
	   printf("Erro na criacao da thread Barbeiro! N = %d Erro = %d\n", i, errno);
	   exit(0);
	}
	
	// Leitura do teclado
	do {
		nTecla = _getch();
	} while (nTecla != ESC);
	
	// Aguarda término das threads homens e mulheres
	dwRet = WaitForMultipleObjects(N_CLIENTES+1,hThreads,TRUE,INFINITE);
	CheckForError(dwRet==WAIT_OBJECT_0);
	
	// Fecha todos os handles de objetos do kernel
	for (int i=0; i<N_CLIENTES+1; ++i)
		CloseHandle(hThreads[i]);
	//for

	// Fecha os handles dos objetos de sincronização
	// [INSIRA AQUI AS CHAMADAS DE FECHAMENTO DE HANDLES]
	CloseHandle(hMutex);
	CloseHandle(hBarbeiroLivre);
	CloseHandle(hAguardaCliente);

	return EXIT_SUCCESS;
	
}//main

DWORD WINAPI ThreadCliente(int i) {

	DWORD dwStatus;
	BOOL bStatus;

	LONG lOldValue;
	
	do {
		WaitForSingleObject(hMutex, INFINITE);
		// Verifica se há lugar na barbearia
		if (n_clientes == N_LUGARES){
			SetConsoleTextAttribute(hOut, HLRED);
		    printf("Cliente %d: barbearia cheia... tento de novo daqui a pouco\n", i);
			ReleaseMutex(hMutex);
			Sleep(2000); //Simula uma voltinha nas redondezas
			continue;
		}
		// Cliente entra na barbearia
		n_clientes++;
		ReleaseMutex(hMutex);
		SetConsoleTextAttribute(hOut, WHITE);
		printf("Cliente %d entrou na barbearia...\n", i);
		
		// Cliente aguarda sua vez
		// [INSIRA AQUI O COMANDO DE SINCRONIZAÇÃO ADEQUADO]
		WaitForSingleObject(hBarbeiroLivre, INFINITE);

		// Cliente acorda o barbeiro
		id_cliente = i;
		// [INSIRA AQUI O COMANDO DE SINCRONIZAÇÃO ADEQUADO]
		ReleaseSemaphore(hAguardaCliente, 1, &lOldValue);


		// Cliente tem sua barba feita pelo barbeiro
		TemABarbaFeita(i);
		
		// Cliente sai da barbearia
		WaitForSingleObject(hMutex, INFINITE);
		n_clientes--;
		ReleaseMutex(hMutex);
		SetConsoleTextAttribute(hOut, WHITE);
		printf("Cliente %d saindo da barbearia...\n", i);

		Sleep(2000);

	} while (nTecla != ESC);

	SetConsoleTextAttribute(hOut, WHITE);
	printf("Thread cliente %d encerrando execucao...\n", i);
	_endthreadex(0);
	return(0);
}//ThreadCliente

DWORD WINAPI ThreadBarbeiro() {

	DWORD dwStatus;
	BOOL bStatus;

	LONG lOldValue;

	do {

		// Tira um cochilo até um cliente chegar
		// [INSIRA AQUI O COMANDO DE SINCRONIZAÇÃO ADEQUADO]
		WaitForSingleObject(hAguardaCliente, INFINITE);

		// Faz a barba do cliente
		FazABarbaDoCliente(id_cliente);

		// Sinaliza que está livre para atender o próximo cliente
		// [INSIRA AQUI O COMANDO DE SINCRONIZAÇÃO ADEQUADO]
		ReleaseSemaphore(hBarbeiroLivre, 1, &lOldValue);

	} while (nTecla != ESC);

	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Thread barbeiro encerrando execucao...\n");
	_endthreadex(0);
	return(0);
}//ThreadHomem

void FazABarbaDoCliente(int id) {

	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Barbeiro fazendo a barba do cliente %d...\n", id);
	Sleep(1000);
	return;
}

void TemABarbaFeita(int id) {

	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Cliente %d tem sua barba feita...\n", id);
	Sleep(1000);
	return;
}

