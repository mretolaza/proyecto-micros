/* 
Universidad del Valle de Guatemala
Proyecto 2 de Microprocesadores
Encripcion de un archivo txt
Fecha: 09 de septiembre de 2019

Integrantes:
-Diana Ximena de Leon Figueroa
-Maria Mercedes Retolaza Reyna
-Andrea Abril Palencia Gutierrez
-Cristina Maria Bautista Silva 
*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream> //file processing
#include <iomanip> //read file

using namespace std;

int in, out, cont, buffer[88];
pthread_cond_t fullValue, emptyValue;
pthread_mutex_t semaf;

typedef struct charOfValue
{
	int task;
	int bit;
	int key;
	int mod;
	int result;
	int fin;
	int b_bits[88];
	FILE *desfile;
} charOfValue;

void writeToFileText(char block, FILE *desfile)
{
	fwrite(&block, 1, 1, desfile);
}

int inverso(int a, int mod)
{
	int b, d;
	for (b = 0; b < mod; b++)
	{
		d = (a * b) % mod;
		if (d == 1)
		{
			return b;
		}
	}
}

int writeToFile(int valueOfUncrypt)
{
	pthread_mutex_lock(&semaf);
	while (cont == 88)
		pthread_cond_wait(&emptyValue, &semaf);
	cont++;
	buffer[in] = valueOfUncrypt;
	in = (in + 1) % 88;
	pthread_cond_broadcast(&fullValue);
	pthread_mutex_unlock(&semaf);
}

void getBitOfUncrypt(void *unbit)
{
	charOfValue *ps = (charOfValue *)unbit;
	int result;
	int inv = inverso(ps->key, ps->mod);
	result = (ps->bit) * (inv);
	if (result > (ps->mod))
	{
		result = result % (ps->mod);
	}
	ps->result = result;
	pthread_exit(NULL);
}

int reedFile()
{
	int valueOfUncrypt;
	char myValue;
	pthread_mutex_lock(&semaf);
	while (cont == 0)
		pthread_cond_wait(&fullValue, &semaf);
	cont--;
	valueOfUncrypt = buffer[out];
	out = (out + 1) % 88;
	pthread_cond_broadcast(&emptyValue);
	pthread_mutex_unlock(&semaf);
	myValue = static_cast<char>(valueOfUncrypt);
	return myValue;
}

void getText(void *arg)
{
	charOfValue *ps = (charOfValue *)arg;
	int i, final;
	final = ps->fin;
	for (i = 0; i < 88; i++)
		writeToFile(ps->b_bits[i]);
	pthread_exit(0);
}

void *taskpool(void *argumento)
{

	charOfValue *ps = (charOfValue *)argumento;

	switch (ps->task)
	{
	case 1:
		getBitOfUncrypt(argumento);
		break;
	case 2:
		getText(argumento);
		break;
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	in = out = cont = 0;

	//Declaracion de la estructura

	charOfValue params;

	int llave[10] = {
		2, 3, 5, 7, 11,
		31, 37, 41, 43, 47};

	//Inicializacion de variables de condicion

	pthread_mutex_init(&semaf, NULL);
	pthread_cond_init(&fullValue, NULL);
	pthread_cond_init(&emptyValue, NULL);

	//Inicializacion de variables del pthread

	int rc; //valor de retorno del pthread
	pthread_t tid;

	pthread_attr_t attr;

	pthread_attr_init(&attr);

	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int f = 0;
	char x;

	//Abrir archivo para escritura del texto

	FILE *writingNewText;
	writingNewText = fopen("Textodes.txt", "w");

	//Si hay un error para abrir el archivo

	if (!writingNewText)
	{
		cerr << "Error. No se ha podido crear el archivo,  Textodes.txt" << endl;
		exit(EXIT_FAILURE);
	}

	//Lectura del texto

	ifstream texto("Textoencriptado.txt", ios::in);

	// Si hay un error al escribir el texto

	if (!texto)
	{
		cerr << "Error, No se puede abrir Textoencriptado.txt";
		exit(EXIT_FAILURE); // terminate with error
	}

	printf("\n*****Comenzando desencripcion del texto*****\n");

	while (texto >> x)
	{
		int y = static_cast<unsigned char>(x);

		y = y - 33;

		params.bit = y;

		int w = 9;

		// 10 rondas de encripcion

		while (w >= 0)
		{

			params.task = 1;
			params.key = llave[w];

			switch (w)
			{
			case 9:
				params.mod = 223;
				break;
			}

			rc = pthread_create(&tid, &attr, taskpool, (void *)&params);

			if (rc)
			{
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}

			rc = pthread_join(tid, NULL);
			params.bit = params.result;
			if (rc)
			{
				printf("ERROR; return code from pthread_join() is %d\n", rc);
				exit(-1);
			}
			w--;
		}

		//Si no es ningun caracter fuera de rango

		if (params.result > 0)
		{
			if (params.result != 215)
			{
				if (x == '|')
				{
					params.result = 32;
				}
				params.b_bits[f] = params.result;
			}
		}

		f++;

		//Utiliza las variables de condicion para escribir el texto

		if (f % 88 == 0)
		{
			params.fin = f;
			params.task = 2;
			pthread_create(&tid, NULL, taskpool, (void *)&params);
			int e = 0;
			for (e = 0; e < 88; e++)
			{
				writeToFileText(reedFile(), writingNewText);
			}
			f = 0;
		}
	}

	// Si un bloque esta incompleto

	if (f < 88)
	{
		params.fin = f;
		params.task = 2;
		pthread_create(&tid, NULL, taskpool, (void *)&params);
		int e = 0;
		for (e = 0; e < f; e++)
		{
			writeToFileText(reedFile(), writingNewText);
		}
		f = 0;
		cout << endl
			 << endl;
	}

	cout << endl;

	printf("\n#####Desencripcion Finalizada#####\n");

	pthread_attr_destroy(&attr);
	pthread_exit(NULL);

	exit(0);
}
