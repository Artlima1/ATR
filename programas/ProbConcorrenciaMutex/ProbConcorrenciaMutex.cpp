//  Automação em tempo Real - ELT012 - UFMG
//  
//  EXEMPLO 4 - Threads concorrentes acessando um recurso compartilhado via MUTEX
//  ------------------------------------------------------------------------------
//
//  Versão 1.0 - 26/02/2010 - Prof. Luiz T. S. Mendes
//
//	Neste programa, o problema anterior do acesso concorrente a uma variavel
//  compartilhada entre diferentes threads de um mesmo processo (Exemplo 5) e
//  solucionado com o emprego de um mutex. O mutex criado e´ do tipo
//  PTHREAD_MUTEX_ERRORCHECK para maior garantia de robustez do programa.
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
#include <conio.h>
#include <errno.h>

#define NTHREADS	20
#define	ESC			0x1B

// DECLARACAO DO PROTOTIPO DE FUNCAO CORRESPONDENTES ÀS THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void* TestFunc(void* arg);

// VARIAVEIS GLOBAIS
long unsigned contador = 0;   // Variavel incrementada continuamente pelas threads
int  tecla;          // Caracter digitado no teclado

// Declaração do MUTEX
pthread_mutex_t MeuMutex;
pthread_mutexattr_t MeuMutexAttr;

// THREAD PRIMARIA
int main()
{
	pthread_t hThreads[NTHREADS];
	int i;
	int status;
    void *tRetStatus;
	
	// Particularidade do Windows - Define o título da janela
	SetConsoleTitle("Exemplo 1 - Criando threads via Pthreads");

	// CRIACAO DO MUTEX. Antes de criá-lo definimos seu tipo como PTHREAD_MUTEX_ERRORCHECK
	// de forma que:
	// (1) chamadas a pthread_mutex_lock pela mesma thread, se esta já é detentora do mutex,
	//     retornem com o erro EDEADLK. (O valor default é PTHREAD_MUTEX_NORMAL, que permite
	//     a uma thread colocar-se em auto-deadlock ao tentar obter novamente a posse do
	//     mutex que já detém).
	// (2) apenas a thread que detém um mutex possa liberá-lo: na biblioteca Pthreads-W32,
	//     para maior desempenho, mutexes do tipo PTHREAD_MUTEX_NORMAL não são verificados
	//     quanto ao registro de propriedade ("ownership") das threads que os utilizam,
	//     permitindo assim que uma thread os conquistem e outra os libere.
	// A definição do tipo do mutex é feita em dois passos: inicialmente cria-se
	// um objeto do tipo "atributos de mutex", cujas propriedades são os atributos "default",
	// e depois altera-se a propriedade desejada.
	//
	pthread_mutexattr_init(&MeuMutexAttr); //sempre retorna 0
    status = pthread_mutexattr_settype(&MeuMutexAttr, PTHREAD_MUTEX_NORMAL);
	if (status != 0){
		printf ("Erro nos atributos do Mutex ! Codigo = %d\n", status);
		exit(0);
	}
	status = pthread_mutex_init(&MeuMutex, &MeuMutexAttr);
	if (status !=0){
		printf ("Erro na criação do Mutex! Codigo = %d\n", status);
		return 0;
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
	   pthread_join(hThreads[i], (void **) &tRetStatus);
	   printf ("Thread %d: status de retorno = %u\n", i, (int) tRetStatus);
	}

	// Destruição do Mutex
	pthread_mutex_destroy(&MeuMutex);
	if (status != 0){
		printf ("Erro na destruição do Mutex! Código = %d\n", status);
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
		//Conquista MUTEX
		status = pthread_mutex_lock(&MeuMutex);
		if (status !=0){
		  if (status == EDEADLK) printf ("Erro EDEADLK na conquista do Mutex!\n");
		  else printf ("Erro inesperado na conquista do Mutex! Codigo = %x\n", status);
		  exit(0);
		}

		// Incrementa variável global de 100000 em 100000 vezes
		for (i=0; i<100000; i++)
		  contador = contador + 1;
		//if (contador == 32704) contador = 0;
		printf ("Thread %02d: contador = %lu\n", index, contador);
		
		// Libera MUTEX
		status = pthread_mutex_unlock(&MeuMutex);
		if (status !=0){
		  if (status == EPERM) printf ("Erro: tentativa de liberar mutex nao-conquistado!\n");
		  else printf ("Erro inesperado na liberacao do Mutex! Codigo = %x\n", status);
		exit(0);
		}
	} while (tecla != ESC);

	pthread_exit((void *)index);
	// O comando "return" abaixo é desnecessário, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return (void *) index;
}
