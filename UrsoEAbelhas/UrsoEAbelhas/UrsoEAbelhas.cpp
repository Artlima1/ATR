/*********************************************************************************
*
*  AUTOMA��O EM TEMPO REAL - ELT012
*  Prof. Luiz T. S. Mendes - 2019/1
*
*  Atividade em classe - 27/03/2019
*
*  Este programa deve ser completado com as linhas de programa necess�rias
*  para solucionar o "problema do urso e das abelhas" ("The Bear and the Honeybees",
*  G. Andrew, "Multithread, Parallel and Distributed Computing",
*  Addison-Wesley, 2000).
*
* O programa � composto de uma thread prim�ria e 21 threads secund�rias. A thread
* prim�ria cria os objetos de sincroniza��o e as threads secund�rias. As threads
* secund�rias correspondem a um urso e 20 abelhas.
*
* A sinaliza��o de t�rmino de programa � feita atrav�s da tecla ESC. Leituras da
* �ltima tecla digitada devem ser feitas em pontos apropriados para que as threads
* detectem esta tecla.
*
**********************************************************************************/

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>							//_getch
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <semaphore.h>

#define	ESC				0x1B			// Tecla para encerrar o programa
#define N_ABELHAS		20				// N�mero de abelhas
#define MAX_PORCOES     10              // Capacidade do pote de mel

#define WHITE    FOREGROUND_RED   | FOREGROUND_GREEN     | FOREGROUND_BLUE
#define HLGREEN  FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define HLRED    FOREGROUND_RED   | FOREGROUND_INTENSITY
#define HLYELLOW FOREGROUND_RED   | FOREGROUND_GREEN     | FOREGROUND_INTENSITY

/* Declaracao dos prototipos de funcoes correspondetes aas threads secundarias*/
/* Atencao para o formato conforme o exemplo abaixo! */
void *Thread_Abelha(void *arg);
void *Thread_Urso(void *arg);

/* Declara��o dos objetos de sincroniza��o */
pthread_mutexattr_t MutexAttr;
pthread_mutex_t AcessaPote;		// Mutex para proteger acesso ao pote de mel
sem_t AcordaUrso;				// Sem�foro para acordar o urso

int nTecla;						// Vari�vel que armazena a tecla digitada para sair
int nPorcoes = 0;				// N�mero de porcoes depositadas no pote de mel

HANDLE hOut;					 //Handle para a sa�da da console

struct fifo_node{
	int value;
	struct fifo_node* next;
};

struct fifo_node * first_node_thread;
struct fifo_node * last_node_thread;
int fifo_size=0;

/*=====================================================================================*/
/* Corpo das funcoes locais Wait(), Signal(), LockMutex e UnLockMutex. Estas funcoes   */
/* assumem que o semaforo [Wait() e Signal()] ou o mutex [LockMutex() e UnLockMutex()] */
/* recebido como parametro ja� tenha sido previamente criado.                          */
/*=====================================================================================*/

void Wait(sem_t *Semaforo) {
	int status;
	status = sem_wait(Semaforo);
	if (status != 0) {
		printf("Erro na obtencao do semaforo! Codigo = %x\n", errno);
		exit(0);
	}
}

void Signal(sem_t *Semaforo) {
	int status;
	status = sem_post(Semaforo);
	if (status != 0) {
		printf("Erro na liberacao do semaforo! Codigo = %x\n", errno);
		exit(0);
	}
}

void LockMutex(pthread_mutex_t *Mutex, int thread_num) {
	int status;

	// status = pthread_mutex_lock(Mutex);
	// if (status != 0) {
	// 	printf("Erro na conquista do mutex! Codigo = %d\n", status);
	// 	exit(0);
	// }
	// printf("Add %d to fifo (%d)\n", thread_num, ++fifo_size);
	
	struct fifo_node * new_node = (struct fifo_node *) malloc(sizeof(struct fifo_node));
	new_node->value = thread_num;
	new_node->next = NULL;
	if(first_node_thread == NULL){
		first_node_thread = new_node;
		last_node_thread = new_node;
	}
	last_node_thread->next = new_node;
	last_node_thread = new_node;
	fifo_size++;

	while(1){
		status = pthread_mutex_lock(Mutex);
		if (status != 0) {
			printf("Erro na conquista do mutex! Codigo = %d\n", status);
			exit(0);
		}
		if (first_node_thread->value == thread_num){
			// printf("Thread %d got the mutex\n", thread_num);
			break;
		}
		else {
			// printf("Thread %d cannot get the mutex, it's %d turn\n", thread_num, first_node_thread->value);
			status = pthread_mutex_unlock(Mutex);
			if (status != 0) {
				printf("Erro na liberacao do mutex! Codigo = %d\n", status);
				exit(0);
			}
		}
	}
}

void UnLockMutex(pthread_mutex_t *Mutex) {
	// printf("Remove %d from fifo (%d)\n", first_node_thread->value, --fifo_size);

	struct fifo_node *curr = first_node_thread;
	if(last_node_thread == first_node_thread) {
		first_node_thread = NULL;
		last_node_thread = NULL;
	}
	else {
		first_node_thread = first_node_thread->next;
	}
	free(curr);
	fifo_size--;

	int status;
	status = pthread_mutex_unlock(Mutex);
	if (status != 0) {
		printf("Erro na liberacao do mutex! Codigo = %d\n", status);
		exit(0);
	}
}

/*=====================================================================================*/
/* Thread Primaria                                                                     */
/*=====================================================================================*/

int main(){
	pthread_t hThreads[N_ABELHAS+1];
	void *tRetStatus;
	int i, status;

	// --------------------------------------------------------------------------
    // Obt�m um handle para a sa�da da console
    // --------------------------------------------------------------------------

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		printf("Erro ao obter handle para a sa�da da console\n");

	// --------------------------------------------------------------------------
	// Cria��o dos mutexes
	// --------------------------------------------------------------------------

	pthread_mutexattr_init(&MutexAttr); //sempre retorna 0
	status = pthread_mutexattr_settype(&MutexAttr, PTHREAD_MUTEX_ERRORCHECK);
	if (status != 0) {
		printf("Erro nos atributos do Mutex ! Codigo = %d\n", status);
		exit(0);
	}
	status = pthread_mutex_init(&AcessaPote, &MutexAttr);
	if (status != 0) {
		printf("Erro na cria��o do mutex AcessaPote! Codigo = %d\n", status);
		exit(0);
	}
	
	// --------------------------------------------------------------------------
	// Cria��o do sem�foro bin�rio
	// --------------------------------------------------------------------------

	status = sem_init(&AcordaUrso, 0, 0); //sempre retorna 0
	if (status != 0) {
		printf("Erro na inicializacao do semaforo! Codigo = %d\n", errno);
		exit(0);
	}

	// --------------------------------------------------------------------------
	// Cria��o das threads secund�rias
	// --------------------------------------------------------------------------
	first_node_thread = NULL;
	for (i = 0; i < N_ABELHAS; i++) {
		status = pthread_create(&hThreads[i], NULL, Thread_Abelha, (void *)i);
		SetConsoleTextAttribute(hOut, WHITE);
		if (status == 0) printf("Thread Abelha %d criada com Id= %0d \n", i, (int)&hThreads[i]);
		else {
			printf("Erro na criacao da thread Abelha %d! Codigo = %d\n", i, status);
			exit(0);
		}
	}// end for

	status = pthread_create(&hThreads[N_ABELHAS], NULL, Thread_Urso, (void *) -1);
	if (status == 0) printf("Thread Urso criada com Id= %0d \n", (int)&hThreads[N_ABELHAS]);
	else {
		printf("Erro na criacao da thread urso! Codigo = %d\n", status);
		exit(0);
	}

	// --------------------------------------------------------------------------
	// Leitura do teclado
	// --------------------------------------------------------------------------

	do {
		printf("Tecle <Esc> para terminar\n");
		nTecla = _getch();
	} while (nTecla != ESC);
	
	// --------------------------------------------------------------------------
	// Aguarda termino das threads secundarias
	// --------------------------------------------------------------------------

	for (i = 0; i < N_ABELHAS + 1; i++) {
		SetConsoleTextAttribute(hOut, WHITE);
		printf("Aguardando termino da thread %d [%d]...\n", i, (int)&hThreads[i]);
		status = pthread_join(hThreads[i], (void **) &tRetStatus);
		if (status != 0) printf("Erro em pthread_join()! Codigo = %d\n", status);
		else
			if ((int)tRetStatus != -1)
			  printf("Thread Abelha %d: status de retorno = %d\n", i, (int)tRetStatus);
			else printf("Thread Urso: status de retorno = %d\n", (int)tRetStatus);
	}
	
	// --------------------------------------------------------------------------
	// Elimina os objetos de sincroniza��o criados
	// --------------------------------------------------------------------------

	SetConsoleTextAttribute(hOut, WHITE);
	status = pthread_mutex_destroy(&AcessaPote);
	if (status != 0) printf("Erro na remocao do mutex! valor = %d\n", status);

	status = sem_destroy(&AcordaUrso);
	if (status != 0) printf("Erro na remocao do semaforo! Valor = %d\n", errno);

	return EXIT_SUCCESS;
	
}//end main

/*=====================================================================================*/
/* Threads secundarias                                                                 */
/*=====================================================================================*/

void *Thread_Abelha(void *arg) {  /* Threads representando as abelhas */

	int i = (int)arg;
	do {

		// ACRESCENTE OS COMANDOS DE SINCRONIZACAO VIA SEMAFOROS ONDE NECESSARIO
		LockMutex(&AcessaPote, i);

		// Deposita uma porcao de mel no pote e incrementa a contagem de porcoes
		if (nPorcoes < MAX_PORCOES) {
			nPorcoes = nPorcoes + 1;
			SetConsoleTextAttribute(hOut, HLYELLOW);
			printf("Abelha %02d depositou uma porcao. Numero atual de porcoes = %d\n", i, nPorcoes);
			if (nPorcoes == MAX_PORCOES) {
				// Pote cheio: acorda o urso
				Signal(&AcordaUrso);
				printf("Abelha %02d encheu o pote: acorda o urso e espera o pote esvaziar-se...\n", i);
			}
		}
		
		UnLockMutex(&AcessaPote);

		// Dorme um tempo apenas para facilitar visualiza��o das mensagens
		//Sleep(100);

	} while (nTecla != ESC);

	// Encerramento da thread. Aqui passamos (a t�tulo de exemplo) o valor da vari�vel "i"
	// como status de encerramento da thread.
	printf("Thread abelha %d encerrando execucao...\n", i);
	pthread_exit((void *) i);
	// O comando "return" abaixo � desnecess�rio, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return(0);
}//Thread abelha

void *Thread_Urso(void *arg) {  /* Thread representando o urso */

	int i = (int)arg;
	do {

        // ACRESCENTE OS COMANDOS DE SINCRONIZACAO VIA SEMAFOROS ONDE NECESSARIO

		// Aguarda ser acordado pelas abelhas
		SetConsoleTextAttribute(hOut, HLRED);
		printf("Urso dormindo...\n");
		Wait(&AcordaUrso);

		//Esvazia o pote de mel
		LockMutex(&AcessaPote, i);
		printf("Urso consumiu todo o mel do pote\n");
		nPorcoes = 0;
		UnLockMutex(&AcessaPote);

	} while (nTecla != ESC);

	// Encerramento da thread. Aqui passamos (a t�tulo de exemplo) o valor da vari�vel "i"
	// como status de encerramento da thread.
	printf("Thread urso encerrando execucao...\n");
	pthread_exit((void *) i);
	// O comando "return" abaixo � desnecess�rio, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return(0);
}//Thread urso
