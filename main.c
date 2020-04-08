#include <stdio.h>
#include <omp.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>


void printMatriz(int matriz[16][16]);
void sudoku();
void calcularPosibles(int ejercicio[16][16], int ***numPosibles, int** cantNumPOsibles, int fila, int columna);
void validarCombinacion(int cominacion[16][16]);
int existeEnFila(int tablero[16][16], int fila, int num);
int existeEnColumna(int tablero[16][16], int columna, int num);
int existeEnRecuadro(int tablero[16][16], int subFila, int subColumna, int num);


int main()
{
	printf("n thread\tspeedUP\n");
	double tUnNucleo = 0;
	for (unsigned char i = 1; i <= 16; i++)
	{
		
		omp_set_num_threads(i);
		clock_t start = clock();
		sudoku();
		double tiempoExe = (((double)clock() - start) / CLOCKS_PER_SEC);
		if (i == 1) 
		{
			tUnNucleo = tiempoExe;
		}
		printf("%d\t%f\n",i,tUnNucleo / tiempoExe);
		
		

	}
	

	return 0;
}

void sudoku()
{
	int ejercicio[16][16] = {
									{0,12,9,0,7,8,0,14,0,0,0,5,4,0,0,0},
									{7,0,0,0,3,0,0,0,15,0,0,14,6,0,0,0},
									{0,0,0,2,0,9,0,0,0,0,10,0,0,0,0,0},
									{0,6,0,0,0,0,5,0,0,0,0,12,0,3,11,0},
									{0,0,0,0,0,2,4,0,0,13,0,0,0,0,0,11},
									{3,0,11,0,0,0,8,0,0,0,1,0,0,0,16,9},
									{0,0,0,0,0,11,6,0,0,15,0,0,10,0,3,0},
									{0,2,6,0,9,12,0,0,7,0,0,0,5,0,0,8},
									{0,0,13,9,8,3,0,0,2,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,4,7,0,13,10,15,0},
									{0,0,0,14,0,16,0,0,6,0,0,0,9,0,12,7},
									{0,8,3,6,12,0,9,13,0,5,0,10,0,0,0,2},
									{12,0,0,0,0,14,0,6,0,3,0,0,11,15,0,0},
									{0,0,0,0,2,15,0,0,11,0,6,4,3,0,0,0},
									{0,0,0,0,0,10,0,0,0,0,5,16,0,14,4,12},
									{0,0,0,0,0,4,11,12,14,0,0,8,7,0,5,0}
								   };
	// almacena los numeros validos para probar, los que no se encuentran en el ejercicio inicial
	int*** numPosibles;
	numPosibles = (int***)calloc(16, sizeof(int**));
	// almacena la cantidad de numeros validas para probar por casilla del sudoku
	int** cantNumPOsibles;
	cantNumPOsibles = (int**)calloc(16, sizeof(int*));
	// reserva memoria e inicializa en 0
	#pragma omp parallel for
	for (int i = 0; i < 16; i++)
	{
		numPosibles[i] = (int**)calloc(16, sizeof(int*));
	}

	#pragma omp parallel for collapse(2)
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			numPosibles[i][j] = (int*)calloc(16, sizeof(int));
		}
	}

	#pragma omp parallel for collapse(3)
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			for (int k = 0; k < 16; k++)
			{
				numPosibles[i][j][k] = 0;
			}
		}
	}

	#pragma omp parallel for
	for (int i = 0; i < 16; i++) 
	{
	cantNumPOsibles[i] = (int*)calloc(16, sizeof(int));
	}

	#pragma omp parallel for collapse(2)
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			cantNumPOsibles[i][j] = 0;
		}
	}
	

	//calcula cuales son los numeros validos para probar
	#pragma omp parallel for collapse(2)
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			calcularPosibles(ejercicio, numPosibles, cantNumPOsibles, i, j);
		}
	}
	
	
	long cantCombinaciones = 1;

	//calcula la cantidad de combinaciones que se pueden formar	
	#pragma omp parallel for collapse(2) reduction(*:cantCombinaciones)
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			cantCombinaciones *= cantNumPOsibles[i][j];
		}
	}
	
	int combinacion[16][16];
	//genera una cada combinacion de numeros
		#pragma omp parallel for private(combinacion)
		for (long i = 0; i < cantCombinaciones; i++)
		{
			
			for (int j = 0; j < 16; j++)
			{
				for (int k = 0; k < 16; k++)
				{
					combinacion[j][k] = numPosibles[j][k][i % cantNumPOsibles[j][k]];
				}
			}
			// determina si la combinacion cumple con las reglas del sudoku
			validarCombinacion(combinacion);
		}
	
	
	return;
}
/**
 * para una posicion fila columna del ejercicio inicial carga en el arreglo cubico numPosibles 
 * los numeros entre 1 y 16 que no se encuentren en la fila, columna, y cuadrante
 */
void calcularPosibles(int ejercicio[16][16], int*** numPosibles, int** cantNumPOsibles, int fila, int columna)
{
	//si ya hay un valor este es la unica opcion posible
	if (ejercicio[fila][columna] != 0)
	{
		numPosibles[fila][columna][0] = ejercicio[fila][columna];
		cantNumPOsibles[fila][columna] = 1;
	}
	else
	{
		
		int count = 0;
		int flag1 = 0;
		int flag2 = 0;
		int flag3 = 0;
		//carga cada valor posible
		#pragma omp parallel for private(flag1, flag2, flag3) shared(count)
		for (int i = 1; i <= 16; i++)
		{
			flag1 = 0;
			flag2 = 0;
			flag3 = 0;
			flag1 += existeEnFila(ejercicio,fila,i);
			flag2 += existeEnColumna(ejercicio,columna,i);
			flag3 += existeEnRecuadro(ejercicio, fila - fila % 4, columna - columna % 4, i);

			#pragma omp critical
			{
				if (flag1 == 0 && flag2 && 0 && flag3 == 0) 
				{
					numPosibles[fila][columna][count] = i;
					cantNumPOsibles[fila][columna]++;
					count++;
				}
			}
		}
	}
}
/**
 * determina si una combinacion [16][16] cumple con las reglas del sudoku
 */
void validarCombinacion(int cominacion[16][16])
{
	int flag = 0;
	
	#pragma omp parallel for collapse(2) reduction(+:flag)
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			flag += existeEnFila(cominacion, i, cominacion[i][j]);
			flag += existeEnColumna(cominacion,j,cominacion[i][j]);
			flag += existeEnRecuadro(cominacion, i - i % 4, j - j % 4, cominacion[i][j]);
		}
	}

	if (flag == 0)
	{
		//printMatriz(cominacion);
	}

}

/**
 * retorna 0 si un valor existe una unica vez en la fila
 */
int existeEnFila(int tablero[16][16], int fila, int num)
{
	int flag = 0;
    #pragma omp parallel for reduction(+:flag)
	for (int columna = 0; columna < 16; columna++)
	{
		flag += (int)(tablero[fila][columna] == num);
	}
	return (int)(flag !=1);
}

/**
 * retorna 0 si un valor existe una unica vez en la columna
 */
int existeEnColumna(int tablero[16][16], int columna, int num)
{
	int flag = 0;
	#pragma omp parallel for reduction(+:flag)
	for (int fila = 0; fila < 16; fila++)
	{
		flag += (int)(tablero[fila][columna] == num);
	}
	return (int)(flag != 1);
}

/**
 * retorna 0 si un valor existe una unica vez en el sub cuadro
 */
int existeEnRecuadro(int tablero[16][16], int subFila, int subColumna, int num)
{
	int flag = 0;
	#pragma omp parallel for collapse(2) reduction(+:flag)
	for (int fila = 0; fila < 4; fila++)
	{
		for (int columna = 0; columna < 4; columna++)
		{
			flag += (int)(tablero[fila + subFila][columna + subColumna] == num);
		}
	}
	
	return (int)(flag != 1);
}

void printMatriz(int matriz[16][16])
{
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			printf("%d\t", matriz[i][j]);
		}
		printf("\n");
	}
}
