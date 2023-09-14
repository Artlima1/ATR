//  Automação em tempo Real - ELT012 - UFMG
//  
//  EXEMPLO 5 - Threads concorrentes acessando um recurso compartilhado via Semaforo
//  --------------------------------------------------------------------------------
//
//  Versão 1.0 - 26/02/2010 - Prof. Luiz T. S. Mendes
//
//	Este programa e´ completamente equivalente ao anterior (Exemplo 6). A unica
//  diferenca e´ a utilizacao de um semaforo binario ao inves dde mutex.
//
//  Para a compilação correta deste programa, assumindo que a biblioteca Pthreads
//  Win32 ja´ esteja instalada em seu computador, faça os seguintes ajustes no
//  Visual Studio 2008 Express Edition:
//
//  1. Selecione Project -> Properties -> Configuration Properties -> C/C++ -> General
//     e defina em "Additional Include Directories" o diretório onde encontram-se os
//     "header files" da distribuição pthreads, em geral
//
//          [...]\pthreads\Pre-built.2\include
// 
//  2. Selecione Project -> Properties -> Configuration Properties -> Linker -> General
//     e defina em "Additional Library Directories" o diretório onde se encontra
//     a biblioteca Pthreads (extensão .LIB), em geral
//
//          [...]\pthreads\Pre-built.2\lib
//  
//  3. Selecione Project -> Properties -> Configuration Properties -> Linker -> Input
//     e declare a biblioteca "pthreadVC2.lib" em "Additional Dependencies".
//

#include <windows.h>
#include <stdio.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <semaphore.h>
#include <conio.h>
#include <errno.h>

#define NTHREADS	20
#define	ESC			0x1B

// DECLARACAO DO PROTOTIPO DE FUNCAO CORRESPONDENTES ÀS THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void* TestFunc(void* arg);

// VARIAVEIS GLOBAIS
long contador = 0;   // Variavel incrementada continuamente pelas threads
int  tecla;          // Caracter digitado no teclado

// Declaração do MUTEX
sem_t MeuSemaforo;

// THREAD PRIMARIA
int main()
{
	pthread_t hThreads[NTHREADS];
	int i;
	int status;
    void *tRetStatus;
	
	// Particularidade do Windows - Define o título da janela
	SetConsoleTitle("Exemplo 1 - Criando threads via Pthreads");

	// INICIALIZACAO DO SEMAFORO.
	// Diferentemente de mutexes, semaforos sao mais simples de inicializar.
	status = sem_init(&MeuSemaforo, 0, 1); //sempre retorna 0
	if (status != 0){
		printf ("Erro na inicializacao do semaforo ! Codigo = %d\n", errno);
		exit(0);
	}
	
	// Loop de criacao das threads
    for (i=0; i<NTHREADS; ++i) {	// cria 3 threads	
		status = pthread_create(&hThreads[i], NULL, TestFunc, (void *) i);
		if (!status) printf("Thread %d criada com Id= %0d \n", i, (int) &hThreads[i]);
		else printf ("Erro na criacao da thread %d! Codigo = %d\n", i, status);
	}	// for

	// Aguarda usuario digitar a tecla ESC para encerrar
	do {
		printf("Tecle <Esc> para terminar\n");
		tecla = _getch();
	} while (tecla != ESC);

    // Aguarda termino das threads
	for (i=0; i<NTHREADS; i++){
	   printf("Aguardando termino da thread %d...\n", (int) &hThreads[i]);
	   pthread_join(hThreads[i], &tRetStatus);
	   printf ("Thread %d: status de retorno = %d\n", i, (int) tRetStatus);
	}

	// Destruição do Semaforo
	status = sem_destroy(&MeuSemaforo);
	if (status !=0){
		printf ("Erro na destruição do Semaforo! Código = %d\n", errno);
		exit(0);
	}

	return EXIT_SUCCESS;
}	// main

// THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void *TestFunc(void *arg){
	int i, index, status;
	
	index = (int) arg;
	do {
		// Conquista Semaforo
		status = sem_wait(&MeuSemaforo);
		if (status !=0){
		  printf ("Erro na obtencao do Semaforo! Codigo = %d\n", errno);
		  exit(0);
		}

		// Incrementa variável global de 100000 em 100000 vezes
		for (i=0; i<100000; i++)
		  contador = contador + 1;
		printf ("Thread %02d: contador = %d\n", index, contador);
		
		// Libera Semaforo
		status = sem_post(&MeuSemaforo);
		if (status !=0){
		  printf ("Erro na liberacao do Semaforo! Codigo = %d\n", errno);
		  exit(0);
		}
	} while (tecla != ESC);

	pthread_exit((void *)index);
	// O comando "return" abaixo é desnecessário, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return (void *) index;
}
