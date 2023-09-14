//  Automação em tempo Real - ELT012 - UFMG
//  
//  EXEMPLO 1 - Programa demonstrativo para criação de processos na plataforma Windows
//  ----------------------------------------------------------------------------------
//
//  Versão 1.0 - 26/02/2010 - Prof. Luiz T. S. Mendes


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>		// _getch

int main()
{

	BOOL status;
	STARTUPINFO si;				    // StartUpInformation para novo processo
	PROCESS_INFORMATION NewProcess;	// Informações sobre novo processo criado
	
	SetConsoleTitle("Programa 2.1 - Criando Threads");
	printf ("Digite uma tecla qualquer para criar uma instancia do Notepad:\n");
	_getch();

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);	// Tamanho da estrutura em bytes

	status = CreateProcess(
		         "C:\\Windows\\Notepad.exe", // Caminho do arquivo executável
	             NULL,                       // Apontador p/ parâmetros de linha de comando
                 NULL,                       // Apontador p/ descritor de segurança
				 NULL,                       // Idem, threads do processo
				 FALSE,	                     // Herança de handles
		         NORMAL_PRIORITY_CLASS,	     // Flags de criação
		         NULL,	                     // Herança do amniente de execução
				 "C:\\Windows",              // Diretório do arquivo executável
		         &si,			             // lpStartUpInfo
		         &NewProcess);	             // lpProcessInformation
    if (!status) printf ("Erro na criacao do Notepad! Codigo = %d\n", GetLastError());

	printf ("Digite uma tecla qualquer para criar uma instancia do Firefox:\n");
	_getch();

	status = CreateProcess(
		         //"C:\\Arquivos de programas\\Mozilla Firefox\\firefox.exe", // Caminho do arquivo executável
		         "C:\\Program Files\\Mozilla Firefox\\firefox.exe", // Caminho do arquivo executável
	             NULL,                       // Apontador p/ parâmetros de linha de comando
                 NULL,                       // Apontador p/ descritor de segurança
				 NULL,                       // Idem, threads do processo
				 FALSE,	                     // Herança de handles
		         NORMAL_PRIORITY_CLASS,	     // Flags de criação
		         NULL,	                     // Herança do amniente de execução
				 //"C:\\Arquivos de programas\\Mozilla Firefox", // Diretório do arquivo executável
				 "C:\\Program Files\\Mozilla Firefox", // Diretório do arquivo executável
		         &si,			             // lpStartUpInfo
		         &NewProcess);	             // lpProcessInformation
	if (!status) printf ("Erro na criacao do Firefox! Codigo = %d\n", GetLastError());

	return EXIT_SUCCESS;
}	// main



