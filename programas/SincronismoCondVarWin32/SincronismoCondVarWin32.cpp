//  Automação em tempo Real - ELT012 - UFMG
//  
//  EXEMPLO 8 - Sincronismo entre threads via Variáveis de Condição na API Win32
//  ----------------------------------------------------------------------------
//
//  Versão 1.0 - 22/04/2010 - Prof. Luiz T. S. Mendes
//
//  NOTA: Este programa é funcionalmente idêntico ao Exemplo 7, com a única
//  diferença de que as chamadas referentes à criação de threads e variáveis de
//  condição foram portadas para a API Win32.
//
//	Este programa exemplifica o uso de variáveis de condição para o sincronismo
//  entre threads. Para tal, a thread primária do programa cria 5 threads
//  secundárias, que ficam bloqueadas em 3 eventos distintos:
//
//    thread secundária   A       - desbloqueia-se quando as teclas "A" ou ESC
//                                  forem digitadas no teclado
//    thread secundária   B       - idem, referente às teclas "B" ou ESC
//    threads secundárias C, D, E - desbloqueiam-se ao mesmo tempo quando a
//                                  tecla de espaço ou ESC forem acionados.
//
//  Estes eventos são modelados como variáveis de condição. A tecla ESC,
//  quando digitada, sinaliza o término de execução para todas as threads.
//

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <process.h>	// _beginthreadex() e _endthreadex()
#include <stdio.h>
#include <conio.h>
#include <errno.h>

#define NTHREADS	5
#define	ESC			0x1B
#define TeclaA      0x61
#define TeclaB      0x62
#define TeclaEsp	0x20

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI *CAST_FUNCTION)(LPVOID);
typedef unsigned *CAST_LPDWORD;

// DECLARACAO DOS PROTOTIPOS DE FUNCAO CORRESPONDENTES ÀS THREADS SECUNDARIAS
DWORD WINAPI ThreadFunc(LPVOID);	// declaração da função


// VARIAVEIS GLOBAIS
long contador = 0;   // Variavel incrementada continuamente pelas threads
int  tecla = 0;      // Caracter digitado no teclado

// Declaração das variáveis de condição e CRITICAL SECTION
CONDITION_VARIABLE CondVarA, CondVarB, CondVarEsp;
CRITICAL_SECTION CritSec;

struct thread_data {int tecla;
		            int index;
                    CONDITION_VARIABLE *condvar;};

// THREAD PRIMARIA
int main()
{
	HANDLE hThreads[NTHREADS];
	int i;
	DWORD status, dwThreadID, dwExitCode;
	int teclas[5] = {TeclaA, TeclaB, TeclaEsp, TeclaEsp, TeclaEsp};
	CONDITION_VARIABLE *condvars[5] = {&CondVarA, &CondVarB, &CondVarEsp, &CondVarEsp, &CondVarEsp};
    thread_data parametros[5];

	//-----------------------------------------------------------------------------
	// Particularidade do Windows - Define o título da janela
	//-----------------------------------------------------------------------------

	SetConsoleTitle("Exemplo 1 - Criando threads via Pthreads");

	//-----------------------------------------------------------------------------
	// Criação da CRITICAL SECTION
	//-----------------------------------------------------------------------------

	InitializeCriticalSection(&CritSec);
	
	//-----------------------------------------------------------------------------
	// Criação das variáveis de condição. São utilizados os atributos "default"
	// para estas variáveis.
	//-----------------------------------------------------------------------------

	InitializeConditionVariable(&CondVarA);
	InitializeConditionVariable(&CondVarB);
    InitializeConditionVariable(&CondVarEsp);

	//-----------------------------------------------------------------------------
	// Loop de criacao das threads
	//-----------------------------------------------------------------------------

    for (i=0; i<NTHREADS; ++i) {	// cria 5 threads
		parametros[i].tecla = teclas[i];
		parametros[i].index = i;
		parametros[i].condvar = condvars[i];
		hThreads[i] = (HANDLE) _beginthreadex(
		                          NULL,
								  0,
								  (CAST_FUNCTION)ThreadFunc,	// casting necessário
								  (void *) &parametros[i],
								  0,
								  (CAST_LPDWORD)&dwThreadID	// cating necessário
		);
		
		if (hThreads[i]) printf("Thread %d criada com Id= %0d \n", i, dwThreadID);
		else printf ("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());
	}	// for

	//-----------------------------------------------------------------------------
	// Aguarda usuario digitar a tecla ESC para encerrar
	//-----------------------------------------------------------------------------

	do {
		printf("Digite \"a\", \"b\", <Espaco> ou <Esc> para terminar:\n");
		tecla = _getch();

		if (tecla == TeclaA || tecla == ESC)
			WakeConditionVariable(&CondVarA);
		
		if (tecla == TeclaB || tecla == ESC)
			WakeConditionVariable(&CondVarB);

		if (tecla == TeclaEsp || tecla == ESC)
			WakeAllConditionVariable(&CondVarEsp);

	} while (tecla != ESC);

 	//-----------------------------------------------------------------------------
    // Aguarda termino das threads e fecha seus handles
	//-----------------------------------------------------------------------------

	status = WaitForMultipleObjects(NTHREADS, hThreads, TRUE, INFINITE);
	if (status != WAIT_OBJECT_0){
		printf ("Erro em WaitForMultipleObjects! Codigo = %d\n", GetLastError());
		return 0;
	}

	for (i=0; i<NTHREADS; ++i) {
		GetExitCodeThread(hThreads[i], &dwExitCode);
		printf("thread %d terminou: codigo=%d\n", i, dwExitCode); 
		CloseHandle(hThreads[i]);	// apaga referência ao objeto
	}

	/* DESTRUIR VARIAVEIS DE CONDICAO */

	return 0;
}

// THREADS SECUNDARIAS
DWORD WINAPI ThreadFunc(LPVOID thread_arg){
	int index;
	BOOL status;
	struct thread_data *parametros;
	CONDITION_VARIABLE *condvar;

	parametros = (struct thread_data *) thread_arg;
	index = parametros ->index;
	condvar = parametros->condvar;
	
	do {
		// Devemos nos lembrar do processo de espera por uma variável de condição:
		// 1. Conquista-se a CRITICAL SECTION associada ao "predicado" a ser testado;
		// 2. while (PREDICADO NAO VERDADEIRO) {
		//      "Aguarda sinalizacao da variavel de condicao";
		//    }
		// 3. Libera CRITICAL SECTION associada à variável de condição.
		//
		EnterCriticalSection(&CritSec);
		while (tecla != parametros->tecla){
		    status = SleepConditionVariableCS(parametros->condvar, &CritSec, INFINITE);
		    if (!status){
				printf ("Thread %d: Erro %d na espera da condicao!\n", index, GetLastError());
		        exit(0);
		    }
			printf ("Thread %d: condicao sinalizada! tecla = \"%c\"\n", index, tecla);
			if (tecla == ESC) break;
		}

		if (tecla != ESC) tecla = 0;
		LeaveCriticalSection(&CritSec);
	} while(tecla != ESC);

	_endthreadex((DWORD) index);
	// O comando "return" abaixo é desnecessário, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return index;
}
