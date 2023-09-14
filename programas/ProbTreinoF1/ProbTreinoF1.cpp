//  Automação em tempo Real - ELT012 - UFMG
//  
//  EXEMPLO 6 - Solução do problema do treino de F-1 via semaforos contadores
//  -------------------------------------------------------------------------
//
//  Versão 1.0 - 26/02/2010 - Prof. Luiz T. S. Mendes
//
//	Este programa e´ equivalente ao programa 4.1 apresentado pelo Prof.
//  Constantino Seixas em seu livro "Programação concorrente para ambiente Windows:
//  Uma Visão de Automação". Foram feitas apenas as adaptações necessárias para uso
//  de semáforos conforme a biblioteca Pthreads-Win32.
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
#include <conio.h>      // _getch
#include <errno.h>

#define	EQUIPES				7
#define MAX_CARROS_PISTA	5
#define NUM_CARROS			2*EQUIPES	// número de carros na simulação

// Declaração dos objetos de sincronização
pthread_mutex_t Mutex[EQUIPES];
pthread_mutexattr_t MutexAttr[EQUIPES];
sem_t Semaphore;

// DECLARACAO DO PROTOTIPO DE FUNCAO CORRESPONDENTES ÀS THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void* BoxFunc(void* arg);

int main()
{
	pthread_t hThreads[NUM_CARROS];
	void *tRetStatus;
	int i, status;
	int	nEquipe, nCar;
	
	// --------------------------------------------------------------------------
	// Criação dos mutexes, um para cada equipe. Antes da criação dos mutexes,
	// definimos seu atributo como PTHREAD_MUTEX_ERRORCHECK para que os mutexes
	// sejam do tipo "seguro".
	// --------------------------------------------------------------------------

	for (nEquipe=0; nEquipe<EQUIPES; ++nEquipe){
		pthread_mutexattr_init(&MutexAttr[nEquipe]); //sempre retorna 0
        status = pthread_mutexattr_settype(&MutexAttr[nEquipe], PTHREAD_MUTEX_ERRORCHECK);
	    if (status != 0){
		    printf ("Erro nos atributos do Mutex ! Codigo = %d indice = %d\n", status, nEquipe);
		    exit(0);
	    }
	    status = pthread_mutex_init(&Mutex[nEquipe], &MutexAttr[nEquipe]);
	    if (status !=0){
		    printf ("Erro na criação do Mutex! Codigo = %d\n", status);
		    exit(0);
		}
	}

	// --------------------------------------------------------------------------
	// Criação do semáforo contador
	// --------------------------------------------------------------------------

    status = sem_init(&Semaphore, 0, MAX_CARROS_PISTA); //sempre retorna 0
	if (status != 0){
		printf ("Erro na inicializacao do semaforo ! Codigo = %d\n", errno);
		exit(0);
	}

	// --------------------------------------------------------------------------
    // Loop de criacao das threads. Para cada equipe são criadas duas threads,
	// representando o primeiro e o segundo piloto e seus respectivos carros.
	// Cada carro recebe uma numeração aleatória de 0 a 13. Cada thread criada
	// recebe como parâmetro um inteiro de 16 bits onde os 8 bits mais
	// significativos contém o número do carro (0..13), e os 8 bits menos
	// significativos correspondem ao número da equipe (0..6).
	// --------------------------------------------------------------------------

    for (nCar = 0; nCar < NUM_CARROS; ++nCar) {
		nEquipe = nCar / 2;     	            // Gera numero da equipe
		status = pthread_create(&hThreads[nCar],   // Cria threads
			                    NULL,
								BoxFunc,
								(int *)((nCar<<8) + nEquipe)
								);
		if (!status) printf("Thread %d criada com Id= %0d \n", nCar, (int) &hThreads[nCar]);
		else printf ("Erro na criacao da thread %d! Codigo = %d\n", nCar, status);
	}	// for


	// --------------------------------------------------------------------------
    // Aguarda termino das threads
	// --------------------------------------------------------------------------

	for (i = 0; i < NUM_CARROS; i++){
	   printf("Aguardando termino da thread %d...\n", (int) &hThreads[i]);
	   pthread_join(hThreads[i], &tRetStatus);
	   printf ("Thread %d: status de retorno = %d\n", i, (int) tRetStatus);
	}
	
	// --------------------------------------------------------------------------
	// Elimina os objetos de sincronização criados
	// --------------------------------------------------------------------------

	for (i = 0; i < EQUIPES; i++) {
		status = pthread_mutex_destroy(&Mutex[i]);
	    if (status != 0)
			printf("Erro na remocao do mutex! i = %d valor = %d\n", i, status);
	}

    status = sem_destroy(&Semaphore);
	if (status != 0)
		printf("Erro na remocao do semaforo! Valor = %d\n", errno);

	printf("\nAcione uma tecla para terminar\n");
	_getch(); // // Pare aqui, caso não esteja executando no ambiente MDS

	return EXIT_SUCCESS;
}  // main

// THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void *BoxFunc(void *arg)
{	
	int id, status;
	int nCar, iTeam;
	
	id = (int) arg;

	iTeam= (DWORD)id % 256;    // Obtém o número da equipe
	nCar = (DWORD)id / 256;    // Obtém o número do carro
	Sleep(100);	// Dá um tempo para criar todo mundo e aumentar a concorrência

	for (int i = 0; i < 3; ++i) {  // dá 3 voltas na pista
		printf("Carro %d da Equipe %d quer treinar...\n", nCar, iTeam);

		// Conquista direito de sair do box
		status = pthread_mutex_lock(&Mutex[iTeam]);
		if (status !=0){
		  if (status == EDEADLK) printf ("Erro EDEADLK na conquista do Mutex!\n");
		  else printf ("Erro na conquista do Mutex! Valor = %x\n", status);
		  exit(0);
		}

		// Conquista direito de fazer tomada de tempo na pista
		status = sem_wait(&Semaphore);
		if (status !=0){
		  printf ("Erro na obtencao do Semaforo! Codigo = %x\n", errno);
		  exit(0);
		}

		// Dá uma volta na pista. O tempo gasto na volta é simulado com um numero aleatório.
		printf("Carro %d da Equipe %d treinando... volta %d \n", nCar, iTeam, i+1);
		Sleep(1000*(rand() % 10));

		// Sai da pista e a libera para outro piloto.
		status = sem_post(&Semaphore);
		if (status !=0){
		  printf ("Erro na liberacao do Semaforo! Codigo = %x\n", errno);
		  exit(0);
		}

		// Volta ao Box e libera companheiro de equipe para correr
		status = pthread_mutex_unlock(&Mutex[iTeam]);
		if (status !=0){
		  if (status == EPERM) printf ("Erro: tentativa de liberar mutex nao-conquistado!\n");
		  else printf ("Erro inesperado na liberacao d!@o Mutex! Codigo = %x\n", status);
		  exit(0);
		}
		
		printf("Carro %d da Equipe %d acabou de treinar...\n", nCar, iTeam);
	} // for 

	pthread_exit((void *)id);
	// O comando "return" abaixo é desnecessário, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return(0);
}



