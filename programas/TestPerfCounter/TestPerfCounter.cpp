// Article ID: 815668 - Last Review: November 14, 2007 - Revision: 3.2
// How to use the QueryPerformanceCounter function to time code in Visual C++
//
// http://support.microsoft.com/kb/815668
//
// Programa original da Microsoft convertido para C++ nativo (unmanaged)
// por Luiz T. S. Mendes em 22/04/2010
//

#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>		// _getch

int main()
{
__int64 ctr1 = 0, ctr2 = 0, freq = 0;
int acc = 0, i = 0;

// Start timing the code.
if (QueryPerformanceCounter((LARGE_INTEGER *)&ctr1)!= 0)
{
	// Code segment is being timed.
	for (i=0; i<1000; i++) acc++;

	// Finish timing the code.
	QueryPerformanceCounter((LARGE_INTEGER *)&ctr2);

	printf("Valor inicial do contador: %lu\n", (unsigned long) ctr1);
	printf("valor final do contador:   %lu\n", (unsigned long) ctr2);
	
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
	
	//***************************************************************
	// FALHA NO PRINTF COM USO DE VARIAVEL DE TIPO _INT64 ?????
	// O printf abaixo vai provocar falha na conversao da segunda
	// variavel, a menos que seja feito um CAST da primeira variavel
	// para UNSIGNED LONG !!!!
	// Comando que provoca erro:
	//   printf ("%d %e\n", freq, 1.0/freq);
    // Comando que funciona:
	//   printf ("%d %e\n", (unsigned long)freq, 1.0/freq);
	//***************************************************************
	printf("Resolucao minima de QueryPerformanceCounter: 1/%ld = %e segundos\n", (unsigned long)freq, 1.0/freq);
    printf("Tempo de 1000 incrementos: %e segundos.\n",(ctr2 - ctr1) * 1.0 / freq);
}
else
{
	DWORD dwError = GetLastError();
	printf("Codigo de erro = %d", dwError);
}

// Make the console window wait.
//printf("Press ENTER to finish.\n");
//_getch();

return 0;
}
