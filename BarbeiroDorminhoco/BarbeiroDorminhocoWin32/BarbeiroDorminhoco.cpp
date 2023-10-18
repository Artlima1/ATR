/*********************************************************************************
*
*  AUTOMA��O EM TEMPO REAL - ELT012
*  Prof. Luiz T. S. Mendes - 2021/2
*
*  Atividade em classe: "O problema barbeiro dorminhoco"
*
*  Este programa deve ser completado com as linhas de programa necess�rias
*  para solucionar o "problema do banheiro dorminhoco" ("The Sleeping Barber").
*
*  O programa � composto de uma thread prim�ria e 11 threads secund�rias. A thread
*  prim�ria cria os objetos de sincroniza��o e as threads secund�rias. As threads
*  secund�rias correspondem ao barbeiro e a 10 clientes.
*
* A sinaliza��o de t�rmino de programa � feita atrav�s da tecla ESC. Leituras da
* �ltima tecla digitada devem ser feitas em pontos apropriados para que as threads
* detectem esta tecla. Este tipo de sincroniza��o, contudo, ir� falhar sempre
* que o ESC for digitado e as threads secund�rias estiverem bloqueadas na fun��o
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

typedef unsigned (WINAPI *CAST_FUNCTION)(LPVOID);	// Casting para terceiro e sexto par�metros da fun��o
                                                    // _beginthreadex
typedef unsigned *CAST_LPDWORD;

#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN      | FOREGROUND_BLUE
#define HLGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define HLRED   FOREGROUND_RED   | FOREGROUND_INTENSITY

#define	ESC				0x1B			// Tecla para encerrar o programa
#define N_CLIENTES		10			// N�mero de clientes
#define N_LUGARES       5           // N�mero de cadeiras (4 de espera e 1 de barbear)
#define N_BARBEIROS     2           // N�mero de Barbeiros

DWORD WINAPI ThreadBarbeiro(int i);		// Thread�representando o barbeiro
DWORD WINAPI ThreadCliente(int);		// Thread representando o cliente

void FazABarbaDoCliente(int id_cliente, int id_barbeiro);		// Fun��o que simula o ato de fazer a barba
void TemABarbaFeita(int);			// Fun��o que simula o ato de ter a barba feita

int n_clientes = 0;					// Contador de clientes
HANDLE hBarbeiroLivre;				// Sem�foro para indicar ao cliente que o barbeiro est� livre
HANDLE hAguardaCliente;				// Sem�foro para indicar ao barbeiro que chegou um cliente
HANDLE hMutex_n_clientes;			// Permite acesso exclusicvo � vari�vel n_clientes
HANDLE hMutex_cadeiras;				// Permite acesso exclusicvo � vari�vel n_clientes

int nTecla;								// Vari�vel que armazena a tecla digitada para sair
int id_cliente_cadeira[N_BARBEIROS];    // Identificador do cliente para cada barbeiro
int cadeira_livre=0; int cadeira_ocupada=0;

HANDLE hOut;						// Handle para a sa�da da console

// THREAD PRIM�RIA
int main(){

	HANDLE hThreads[N_CLIENTES+N_BARBEIROS];       // N clientes mais o barbeiro
	DWORD dwIdBarbeiro, dwIdCliente;
	DWORD dwExitCode = 0;
	DWORD dwRet;
	int i;

	// Obt�m um handle para a sa�da da console
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		printf("Erro ao obter handle para a sa�da da console\n");

	// Cria objetos de sincroniza��o
    // [INSIRA AQUI OS COMANDOS DE CRIA��O DO MUTEX / SEM�FOROS]
	hMutex_n_clientes = CreateMutex(NULL, FALSE, "N_CLIENTES_MUTEX");
	CheckForError(hMutex_n_clientes);

	hMutex_cadeiras = CreateMutex(NULL, FALSE, "CADEIRAS_MUTEX");
	CheckForError(hMutex_cadeiras);

	hBarbeiroLivre = CreateSemaphore(NULL, N_BARBEIROS, N_BARBEIROS, "BARBEIRO_LIVRE_SEM");
	CheckForError(hBarbeiroLivre);

	hAguardaCliente = CreateSemaphore(NULL, N_BARBEIROS, N_BARBEIROS, "AGUARDA_CLIENTE_SEM");
	CheckForError(hAguardaCliente);

	// Cria��o de threads
	// Note que _beginthreadex() retorna -1L em caso de erro
	for (i=0; i<N_CLIENTES; ++i) {
		hThreads[i] = (HANDLE) _beginthreadex(
						       NULL,
							   0,
							   (CAST_FUNCTION) ThreadCliente,	//Casting necess�rio
							   (LPVOID)(INT_PTR)i,
							   0,								
							   (CAST_LPDWORD)&dwIdCliente);		//Casting necess�rio
		SetConsoleTextAttribute(hOut, WHITE);
		if (hThreads[i] != (HANDLE) -1L)
			printf("Thread Cliente %d criada com Id=%0x\n", i, dwIdCliente);
		else {
			printf("Erro na criacao da thread Cliente! N = %d Codigo = %d\n", i, errno);
			exit(0);
		}
	}//for

	for (i=0; i<N_BARBEIROS; ++i) {
		hThreads[N_CLIENTES+i] = (HANDLE) _beginthreadex(
						       NULL,
							   0,
							   (CAST_FUNCTION) ThreadBarbeiro,	//Casting necess�rio
							   (LPVOID)(INT_PTR)i,
							   0,								
							   (CAST_LPDWORD)&dwIdBarbeiro);		//Casting necess�rio
		SetConsoleTextAttribute(hOut, WHITE);
		if (hThreads[i] != (HANDLE) -1L)
			printf("Thread Barbeiro %d criada com Id=%0x\n", i, dwIdBarbeiro);
		else {
			printf("Erro na criacao da thread Cliente! N = %d Codigo = %d\n", i, errno);
			exit(0);
		}
	}//for
	
	// Leitura do teclado
	do {
		nTecla = _getch();
	} while (nTecla != ESC);
	
	// Aguarda t�rmino das threads homens e mulheres
	dwRet = WaitForMultipleObjects(N_CLIENTES+N_BARBEIROS,hThreads,TRUE,INFINITE);
	CheckForError(dwRet==WAIT_OBJECT_0);
	
	// Fecha todos os handles de objetos do kernel
	for (int i=0; i<N_CLIENTES+1; ++i)
		CloseHandle(hThreads[i]);
	//for

	// Fecha os handles dos objetos de sincroniza��o
	// [INSIRA AQUI AS CHAMADAS DE FECHAMENTO DE HANDLES]
	CloseHandle(hMutex_n_clientes);
	CloseHandle(hBarbeiroLivre);
	CloseHandle(hAguardaCliente);

	return EXIT_SUCCESS;
	
}//main

DWORD WINAPI ThreadCliente(int i) {

	DWORD dwStatus;
	BOOL bStatus;

	LONG lOldValue;
	
	do {
		WaitForSingleObject(hMutex_n_clientes, INFINITE);
		// Verifica se h� lugar na barbearia
		if (n_clientes == N_LUGARES){
			SetConsoleTextAttribute(hOut, HLRED);
		    printf("Cliente %d: barbearia cheia... tento de novo daqui a pouco\n", i);
			ReleaseMutex(hMutex_n_clientes);
			Sleep(2000); //Simula uma voltinha nas redondezas
			continue;
		}
		// Cliente entra na barbearia
		n_clientes++;
		ReleaseMutex(hMutex_n_clientes);
		SetConsoleTextAttribute(hOut, WHITE);
		printf("Cliente %d entrou na barbearia...\n", i);
		
		// Cliente aguarda sua vez
		WaitForSingleObject(hBarbeiroLivre, INFINITE);

		// Cliente acorda o barbeiro
		WaitForSingleObject(hMutex_cadeiras, INFINITE);
		id_cliente_cadeira[cadeira_livre] = i;
		cadeira_livre = (cadeira_livre+1) % N_BARBEIROS;
		ReleaseMutex(hMutex_cadeiras);
		ReleaseSemaphore(hAguardaCliente, 1, &lOldValue);

		// Cliente tem sua barba feita pelo barbeiro
		TemABarbaFeita(i);
		
		// Cliente sai da barbearia
		WaitForSingleObject(hMutex_n_clientes, INFINITE);
		n_clientes--;
		ReleaseMutex(hMutex_n_clientes);
		SetConsoleTextAttribute(hOut, WHITE);
		printf("Cliente %d saindo da barbearia...\n", i);

		Sleep(2000);

	} while (nTecla != ESC);

	SetConsoleTextAttribute(hOut, WHITE);
	printf("Thread cliente %d encerrando execucao...\n", i);
	_endthreadex(0);
	return(0);
}//ThreadCliente

DWORD WINAPI ThreadBarbeiro(int i) {

	DWORD dwStatus;
	BOOL bStatus;

	LONG lOldValue;

	do {

		// Tira um cochilo at� um cliente chegar
		WaitForSingleObject(hAguardaCliente, INFINITE);
		WaitForSingleObject(hMutex_cadeiras, INFINITE);
		int id_cliente = id_cliente_cadeira[cadeira_ocupada];
		cadeira_ocupada = (cadeira_ocupada + 1) % N_BARBEIROS;
		ReleaseMutex(hMutex_cadeiras);
		// Faz a barba do cliente
		FazABarbaDoCliente(id_cliente, i);

		// Sinaliza que est� livre para atender o pr�ximo cliente
		ReleaseSemaphore(hBarbeiroLivre, 1, &lOldValue);

	} while (nTecla != ESC);

	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Thread barbeiro encerrando execucao...\n");
	_endthreadex(0);
	return(0);
}//ThreadHomem

void FazABarbaDoCliente(int id_cliente, int id_barbeiro) {

	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Barbeiro %d fazendo a barba do cliente %d...\n", id_barbeiro, id_cliente);
	Sleep(rand() % 1000);
	return;
}

void TemABarbaFeita(int id) {

	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Cliente %d tem sua barba feita...\n", id);
	Sleep(rand() % 1000);
	return;
}

