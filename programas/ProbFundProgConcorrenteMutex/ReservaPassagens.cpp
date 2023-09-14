//
//  Programa para demonstrar o "Problema Fundamental da Prog. Concorrente"
//   
//	Baseado inteiramente no programa de exemplo 3, capítulo 3 do livro
//  "Programação Multithreaded em ambiente Windows NT© - uma visão de  Automação"
// 
//	Autores: Constantino Seixas Filho/ Marcelo Szuster
//

#include <windows.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <conio.h>		// _getch
#include <time.h>

#define NUM_THREADS	20	// Vamos disparar 20 threads concorrentes

#define LIVRE 0
#define OCUPADO 1
int MapaOcupacao[6] = {LIVRE, LIVRE, LIVRE, LIVRE, LIVRE, LIVRE};

#define FALHA -1
#define SUCESSO 1

// DECLARACAO DO PROTOTIPO DE FUNCAO CORRESPONDENTES ÀS THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void* AlocFunc(void* arg);

// Declaração do MUTEX
pthread_mutex_t MeuMutex;
pthread_mutexattr_t MeuMutexAttr;


int main()
{
	pthread_t hThreads[NUM_THREADS];
	int assento, i;
	int status;
	int tRetStatus[NUM_THREADS];
	
	srand( (unsigned)time(NULL));

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
    status = pthread_mutexattr_settype(&MeuMutexAttr, PTHREAD_MUTEX_ERRORCHECK);
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
    for (i=0; i<NUM_THREADS; ++i) {	// cria NUM_THREADS threads
		assento = rand() % 6; // cada thread tentará atualizar uma posição aleatória do array
		status = pthread_create(&hThreads[i], NULL, AlocFunc, (void *) assento);
		if (!status) printf("Thread %2d criada com assento %d Id = %0x\n", i, assento, (int) &hThreads[i]);
		else printf ("Erro na criacao da thread %d! Codigo = %d\n", i, status);
	}// for

    //printf ("Pressione uma tecla para continuar...\n");
	//_getch();

	// Espera todas as threads terminarem
	for (i=0; i<NUM_THREADS; i++){
	   printf("Aguardando termino da thread %0x... Codigo de saida: ", (int) &hThreads[i]);
	   pthread_join(hThreads[i], (void **) &tRetStatus[i]);
	   if (tRetStatus[i] == -1)
		   printf("Falha\n");
	   else printf("Assento %d\n", tRetStatus[i]);
	}

	return EXIT_SUCCESS;
}	// main


// THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void *AlocFunc(void *arg){
	int assento, ValorRetornado, status;
	assento = (int) arg;
	
    //Conquista MUTEX
	status = pthread_mutex_lock(&MeuMutex);
	if (status !=0){
	  if (status == EDEADLK) printf ("Erro EDEADLK na conquista do Mutex!\n");
	  else printf ("Erro inesperado na conquista do Mutex! Codigo = %x\n", status);
	  exit(0);
	}

	//Região Crítica
	if (MapaOcupacao[(int)assento] == LIVRE) {
		SwitchToThread();
		MapaOcupacao[(int)assento] = OCUPADO;
		ValorRetornado = assento;
	}
	else ValorRetornado = FALHA;

	// Libera MUTEX
	status = pthread_mutex_unlock(&MeuMutex);
	if (status !=0){
	  if (status == EPERM) printf ("Erro: tentativa de liberar mutex nao-conquistado!\n");
	  else printf ("Erro inesperado na liberacao do Mutex! Codigo = %x\n", status);
	  exit(0);
	}

	pthread_exit((void *) ValorRetornado);
	
    // O comando "return" abaixo é desnecessário, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return (void *) NULL;
} // AlocFunc




