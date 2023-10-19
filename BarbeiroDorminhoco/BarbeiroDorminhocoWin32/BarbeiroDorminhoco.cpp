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
#define N_CLIENTES		15			// N�mero de clientes
#define N_LUGARES       5           // N�mero de cadeiras (4 de espera e 1 de barbear)

#define N_BARBEIROS     3           // N�mero de Barbeiros

DWORD WINAPI ThreadBarbeiro(int i);		// Thread�representando o barbeiro
DWORD WINAPI ThreadCliente(int);		// Thread representando o cliente

void FazABarbaDoCliente(int id_cliente, int id_barbeiro, int cadeira);		// Fun��o que simula o ato de fazer a barba
void TemABarbaFeita(int id_cliente, int cadeira);				// Fun��o que simula o ato de ter a barba feita

int n_clientes = 0;					// Contador de clientes
HANDLE hBarbeiroLivre;				// Sem�foro para indicar aos clientes que pelo menos um barbeiro est� livre
HANDLE hAguardaCliente;				// Sem�foro para indicar aos barbeiros que chegou pelo menos um cliente

HANDLE hEscEvent;					// Evento de finalizacao do programa
HANDLE hMutex_n_clientes;			// Permite acesso exclusicvo � vari�vel n_clientes
HANDLE hMutex_cadeiras;				// Permite acesso exclusicvo � vari�vel n_clientes

int nTecla;									// Vari�vel que armazena a tecla digitada para sair
int id_cliente_cadeira[N_BARBEIROS];    	// Lista circular de cadeiras para os clientes, armazenando o id
int cadeira_livre=0; int cadeira_ocupada=0;	// Contadores para lista circular
HANDLE hFimDoCorte[N_BARBEIROS];			// Semáforos para notificar cliente do fim do corte

HANDLE hOut;					// Handle para a sa�da da console
HANDLE hPrint;					// Mutex para controlar prints

// THREAD PRIM�RIA
int main(){

	HANDLE hThreads[N_CLIENTES+N_BARBEIROS];       // N clientes mais os barbeiros
	DWORD dwIdBarbeiro, dwIdCliente;
	DWORD dwExitCode = 0;
	DWORD dwRet;
	int i;

	// Obt�m um handle para a sa�da da console
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		printf("Erro ao obter handle para a sa�da da console\n");

	// Cria objetos de sincroniza��o

	hPrint = CreateMutex(NULL, FALSE, "N_CLIENTES");
	CheckForError(hPrint);

	hEscEvent	= CreateEvent(NULL, TRUE, FALSE, "EscEvento");
	CheckForError(hEscEvent);

	hMutex_n_clientes = CreateMutex(NULL, FALSE, "N_CLIENTES_MUTEX");
	CheckForError(hMutex_n_clientes);

	hMutex_cadeiras = CreateMutex(NULL, FALSE, "CADEIRAS_MUTEX");
	CheckForError(hMutex_cadeiras);

	hBarbeiroLivre = CreateSemaphore(NULL, N_BARBEIROS, N_BARBEIROS, "BARBEIRO_LIVRE_SEM");
	CheckForError(hBarbeiroLivre);

	hAguardaCliente = CreateSemaphore(NULL, 0, N_BARBEIROS, "AGUARDA_CLIENTE_SEM");
	CheckForError(hAguardaCliente);

	char CorteName[7];
	for(i=0; i<N_BARBEIROS; i++){
		sprintf(CorteName, "Corte%d", i);
		hFimDoCorte[i] = CreateSemaphore(NULL, 0, 1, CorteName);
	}

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
		WaitForSingleObject(hPrint, INFINITE);
		SetConsoleTextAttribute(hOut, WHITE);
		if (hThreads[i] != (HANDLE) -1L){
			printf("Thread Cliente %d criada com Id=%0x\n", i, dwIdCliente);
			ReleaseMutex(hPrint);
		}
		else {
			printf("Erro na criacao da thread Cliente! N = %d Codigo = %d\n", i, errno);
			ReleaseMutex(hPrint);
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
		
		if (hThreads[i] != (HANDLE)-1L) {
			WaitForSingleObject(hPrint, INFINITE);
			SetConsoleTextAttribute(hOut, WHITE);
			printf("Thread Barbeiro %d criada com Id=%0x\n", i, dwIdBarbeiro);
			ReleaseMutex(hPrint);
		}
		else {
			printf("Erro na criacao da thread Cliente! N = %d Codigo = %d\n", i, errno);
			ReleaseMutex(hPrint);
			exit(0);
		}
	}//for
	
	// Leitura do teclado
	do {
		nTecla = _getch();
	} while (nTecla != ESC);
	SetEvent(hEscEvent);
	
	// Aguarda t�rmino das threads homens e mulheres

	dwRet = WaitForMultipleObjects(N_CLIENTES+N_BARBEIROS,hThreads,TRUE,INFINITE);
	CheckForError(dwRet==WAIT_OBJECT_0);
	
	// Fecha todos os handles de objetos do kernel
	for (int i=0; i<N_CLIENTES+N_BARBEIROS; ++i)
		CloseHandle(hThreads[i]);
	//for

	// Fecha os handles dos objetos de sincroniza��o

	CloseHandle(hMutex_n_clientes);
	CloseHandle(hMutex_cadeiras);
	CloseHandle(hBarbeiroLivre);
	CloseHandle(hAguardaCliente);
	CloseHandle(hPrint);

	return EXIT_SUCCESS;
	
}//main

DWORD WINAPI ThreadCliente(int i) {

	DWORD dwStatus;
	BOOL bStatus;

	LONG lOldValue;

	HANDLE mult_hMutexClientes[2] = {hMutex_n_clientes, hEscEvent};
	HANDLE mult_hBarbeiroLivre[2] = {hBarbeiroLivre, hEscEvent};
	HANDLE mult_hMutexCadeiras[2] = {hMutex_cadeiras, hEscEvent};
	DWORD ret;
	
	do {
		ret = WaitForMultipleObjects(2, mult_hMutexClientes, FALSE, INFINITE);
		if((ret - WAIT_OBJECT_0) == 1) break;
		// Verifica se h� lugar na barbearia
		if (n_clientes == N_LUGARES){

			WaitForSingleObject(hPrint, INFINITE);
			SetConsoleTextAttribute(hOut, HLRED);
		    printf("Cliente %d: barbearia cheia... tento de novo daqui a pouco\n", i);
			ReleaseMutex(hPrint);

			ReleaseMutex(hMutex_n_clientes);
			Sleep(2000); //Simula uma voltinha nas redondezas
			continue;
		}
		// Cliente entra na barbearia
		n_clientes++;

		ReleaseMutex(hMutex_n_clientes);
		
		WaitForSingleObject(hPrint, INFINITE);
		SetConsoleTextAttribute(hOut, WHITE);
		printf("Cliente %d entrou na barbearia...\n", i);
		ReleaseMutex(hPrint);

		// Cliente aguarda sua vez
		ret = WaitForMultipleObjects(2, mult_hBarbeiroLivre, FALSE, INFINITE);
		if((ret - WAIT_OBJECT_0) == 1) break;

		// Ocupa primeira cadeira livre
		WaitForMultipleObjects(2, mult_hMutexCadeiras, FALSE, INFINITE);
		if((ret - WAIT_OBJECT_0) == 1) break;

		id_cliente_cadeira[cadeira_livre] = i;
		int cadeira = cadeira_livre;
		cadeira_livre = (cadeira_livre+1) % N_BARBEIROS;

		ReleaseMutex(hMutex_cadeiras);

		// Acorda um barbeiro livre
		ReleaseSemaphore(hAguardaCliente, 1, &lOldValue);

		// Cliente tem sua barba feita pelo barbeiro
		TemABarbaFeita(i, cadeira);
		
		// Cliente sai da barbearia
		ret = WaitForMultipleObjects(2, mult_hMutexClientes, FALSE, INFINITE);
		if((ret - WAIT_OBJECT_0) == 1) break;
		n_clientes--;
		ReleaseMutex(hMutex_n_clientes);

		WaitForSingleObject(hPrint, INFINITE);
		SetConsoleTextAttribute(hOut, WHITE);
		printf("Cliente %d saindo da barbearia...\n", i);
		ReleaseMutex(hPrint);

		Sleep(2000);

	} while (nTecla != ESC);

	WaitForSingleObject(hPrint, INFINITE);
	SetConsoleTextAttribute(hOut, WHITE);
	printf("Thread cliente %d encerrando execucao...\n", i);
	ReleaseMutex(hPrint);

	_endthreadex(0);
	return(0);
}//ThreadCliente

DWORD WINAPI ThreadBarbeiro(int i) {

	DWORD dwStatus;
	BOOL bStatus;

	LONG lOldValue;

	HANDLE mult_hAguardaCliente[2] = {hAguardaCliente, hEscEvent};
	HANDLE mult_hMutexCadeira[2] = {hMutex_cadeiras, hEscEvent};
	DWORD ret;
	
	do {

		// Tira um cochilo at� um cliente chegar
		ret = WaitForMultipleObjects(2, mult_hAguardaCliente, FALSE, INFINITE);
		if((ret - WAIT_OBJECT_0) == 1) break;

		// Vai até a primeira cadeira ocupada
		ret = WaitForMultipleObjects(2, mult_hMutexCadeira, FALSE, INFINITE);
		if((ret - WAIT_OBJECT_0) == 1) break;

		int id_cliente = id_cliente_cadeira[cadeira_ocupada];
		int cadeira = cadeira_ocupada;
		cadeira_ocupada = (cadeira_ocupada + 1) % N_BARBEIROS;
		
		ReleaseMutex(hMutex_cadeiras);

		// Faz a barba do cliente
		FazABarbaDoCliente(id_cliente, i, cadeira);

		// Sinaliza que est� livre para atender o pr�ximo cliente
		ReleaseSemaphore(hBarbeiroLivre, 1, &lOldValue);

	} while (nTecla != ESC);

	WaitForSingleObject(hPrint, INFINITE);
	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Thread barbeiro encerrando execucao...\n");
	ReleaseMutex(hPrint);

	_endthreadex(0);
	return(0);
}//ThreadHomem

void FazABarbaDoCliente(int id_cliente, int id_barbeiro, int cadeira) {

	WaitForSingleObject(hPrint, INFINITE);
	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Barbeiro %d fazendo a barba do cliente %d na cadeira %d...\n", id_barbeiro, id_cliente, cadeira);
	ReleaseMutex(hPrint);

	// Faz o servico em tempo randomico e sinaliza para o cliente que acabou
	Sleep((rand() % 1000) + 1000);
	ReleaseSemaphore(hFimDoCorte[cadeira], 1, NULL);
	return;
}

void TemABarbaFeita(int id_cliente, int cadeira) {

	// Aguarda ate que barbeiro notifique que finalizou
	HANDLE mult_hFimDoCorte[2] = { hFimDoCorte[cadeira], hEscEvent};
	DWORD ret = WaitForMultipleObjects(2, mult_hFimDoCorte, FALSE, INFINITE);
	if ((ret - WAIT_OBJECT_0) == 1) return;

	WaitForSingleObject(hPrint, INFINITE);
	SetConsoleTextAttribute(hOut, HLGREEN);
	printf("Cliente %d teve sua barba feita na cadeira %d...\n", id_cliente, cadeira);
	ReleaseMutex(hPrint);
	
	return;
}

