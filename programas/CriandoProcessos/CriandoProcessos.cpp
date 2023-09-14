//  Automa��o em tempo Real - ELT012 - UFMG
//  
//  EXEMPLO 1 - Programa demonstrativo para cria��o de processos na plataforma Windows
//  ----------------------------------------------------------------------------------
//
//  Vers�o 1.0 - 26/02/2010 - Prof. Luiz T. S. Mendes


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>		// _getch

int main()
{

	BOOL status;
	STARTUPINFO si;				    // StartUpInformation para novo processo
	PROCESS_INFORMATION NewProcess;	// Informa��es sobre novo processo criado
	
	SetConsoleTitle("Programa 2.1 - Criando Threads");
	printf ("Digite uma tecla qualquer para criar uma instancia do Notepad:\n");
	_getch();

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);	// Tamanho da estrutura em bytes

	status = CreateProcess(
		         "C:\\Windows\\Notepad.exe", // Caminho do arquivo execut�vel
	             NULL,                       // Apontador p/ par�metros de linha de comando
                 NULL,                       // Apontador p/ descritor de seguran�a
				 NULL,                       // Idem, threads do processo
				 FALSE,	                     // Heran�a de handles
		         NORMAL_PRIORITY_CLASS,	     // Flags de cria��o
		         NULL,	                     // Heran�a do amniente de execu��o
				 "C:\\Windows",              // Diret�rio do arquivo execut�vel
		         &si,			             // lpStartUpInfo
		         &NewProcess);	             // lpProcessInformation
    if (!status) printf ("Erro na criacao do Notepad! Codigo = %d\n", GetLastError());

	printf ("Digite uma tecla qualquer para criar uma instancia do Firefox:\n");
	_getch();

	status = CreateProcess(
		         //"C:\\Arquivos de programas\\Mozilla Firefox\\firefox.exe", // Caminho do arquivo execut�vel
		         "C:\\Program Files\\Mozilla Firefox\\firefox.exe", // Caminho do arquivo execut�vel
	             NULL,                       // Apontador p/ par�metros de linha de comando
                 NULL,                       // Apontador p/ descritor de seguran�a
				 NULL,                       // Idem, threads do processo
				 FALSE,	                     // Heran�a de handles
		         NORMAL_PRIORITY_CLASS,	     // Flags de cria��o
		         NULL,	                     // Heran�a do amniente de execu��o
				 //"C:\\Arquivos de programas\\Mozilla Firefox", // Diret�rio do arquivo execut�vel
				 "C:\\Program Files\\Mozilla Firefox", // Diret�rio do arquivo execut�vel
		         &si,			             // lpStartUpInfo
		         &NewProcess);	             // lpProcessInformation
	if (!status) printf ("Erro na criacao do Firefox! Codigo = %d\n", GetLastError());

	return EXIT_SUCCESS;
}	// main



