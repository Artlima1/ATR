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

int main()
{
	pthread_t hThreads[NUM_THREADS];
	int assento, i;
	int status;
	int tRetStatus[NUM_THREADS];
	
	srand( (unsigned)time(NULL));

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
	     printf ("Falha\n");
	   else printf ("Assento %d\n", tRetStatus[i]);
	}

	return EXIT_SUCCESS;
}	// main


// THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void *AlocFunc(void *arg){
	int assento, ValorRetornado;
	assento = (int) arg;
	
	if (MapaOcupacao[(int)assento] == LIVRE) {
		// O uso da função SwitchToThread(), aqui, é para simular um re-escalonamento
		// preemptivo de tarefas pelo S.O.
		SwitchToThread();
		MapaOcupacao[(int)assento] = OCUPADO;
		ValorRetornado = assento;
	}
	else ValorRetornado = FALHA;

	pthread_exit((void *) ValorRetornado);
	
    // O comando "return" abaixo é desnecessário, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return (void *) NULL;
} // AlocFunc




