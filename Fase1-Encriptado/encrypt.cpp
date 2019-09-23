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

//Declaracion de variable mutex
pthread_mutex_t lock;

//Creacion de estructuras

typedef struct charOfValue
{
	int task;
	int bit;
	int key;
	int mod;
	int result;
	char b_bits[88];
} charOfValue;

void writeTextOfFile(void *block)
{
	charOfValue *ps = (charOfValue *)block;

	ofstream writeTextOfFile("Textoencriptado.txt", ios::app);
	// Protección en caso el archivo falle en su ejecución
	if (!writeTextOfFile)
	{
		cerr << "Error. No se ha podido crear el archivo,  Textoencriptado.txt" << endl;
		exit(EXIT_FAILURE);
	}
	writeTextOfFile << ps->b_bits << endl;
}

void getBitOfEncrypt(void *getOneChar)
{
	charOfValue *ps = (charOfValue *)getOneChar;
	int result;
	result = (ps->bit) * (ps->key);
	if (result > (ps->mod))
	{
		result = result % (ps->mod);
	}
	ps->result = result;
}

void *taskpool(void *argument)
{

	pthread_mutex_lock(&lock); //establecer bloqueo antes utilizar recurso

	charOfValue *ps = (charOfValue *)argument;

	switch (ps->task)
	{
	case 1:
		getBitOfEncrypt(argument);
		break;
	case 2:
		writeTextOfFile(argument);
		break;
	}

	pthread_mutex_unlock(&lock);
	return NULL;
}

int main(int argc, char *argv[])
{
	// Si existe un fallo en la inicializacion de mutex

	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n Fallo de inicializacion de mutex \n");
		return 1;
	}

	//Declaracion de la estructura

	charOfValue params;

	int llave[10] = {
		2, 3, 5, 7, 11,
		31, 37, 41, 43, 47};

	//Declaracion de atributos de los hilos

	int rc; //valor de retorno del pthread
	long i = 0;

	pthread_t tid;

	pthread_attr_t attr;

	pthread_attr_init(&attr);

	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int f = 0;
	char x;
	params.result = 'a';

	//Abrir el archivo para lectura

	FILE *textFile;
	textFile = fopen("Prueba.txt", "r");

	//Deteccion de errores en lectura del archivo

	if (!textFile)
	{
		cerr << "Error, No se puede abrir Prueba.txt";
		exit(EXIT_FAILURE); // terminate with error
	}

	printf("\n*****Comenzando encripcion del textFile*****\n");

	//Mientras no haya llegado al final del archivo

	while (!feof(textFile))
	{
		//Leer el charOfValue y convertirlo a su correspondiente en decimal

		size_t result;
		result = fread(&x, 1, 1, textFile);
		int y = static_cast<unsigned char>(x);

		switch (y)
		{
		case 225:
			params.bit = 97;
			break;
		case 233:
			params.bit = 101;
			break;
		case 237:
			params.bit = 105;
			break;
		case 243:
			params.bit = 111;
			break;
		case 250:
			params.bit = 117;
			break;
		case 241:
			params.bit = 110;
			break;
		default:
			params.bit = y;
			break;
		}

		params.task = 1;

		int w = 0;

		//Realizacion de las rondas de encripcion
		while (w < 10)
		{

			params.key = llave[w];

			switch (w)
			{
			case 0:
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
			w++;
		}

		//Se le suma 33 al valor obtenido para que sea un charOfValue imprimible
		params.result = params.result + 33;

		//Casteo a char y escritura en el array
		params.b_bits[f] = static_cast<char>(params.result);

		//Si el charOfValue es un espacio se escribe un charOfValue representativo

		if (y == 32)
		{
			params.b_bits[f] = '|';
		}

		f++;

		//Escritura del bloque en el archivo de textFile

		if (f % 88 == 0)
		{

			params.task = 2;

			rc = pthread_create(&tid, &attr, taskpool, (void *)&params);

			if (rc)
			{
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}

			rc = pthread_join(tid, NULL);
			if (rc)
			{
				printf("ERROR; return code from pthread_join() is %d\n", rc);
				exit(-1);
			}

			f = 0;
		}
	}

	//Escritura de caracteres si existe un bloque incompleto

	if (f < 88)
	{
		int j = 0;
		for (j = f; j < 89; j++)
		{
			params.b_bits[j] = 00;
		}
		params.task = 2;
		rc = pthread_create(&tid, &attr, taskpool, (void *)&params);
	}

	printf("\n#####Encripcion Finalizada#####\n");

	pthread_mutex_destroy(&lock); //destruccion de mutex dinamica ya usada
	pthread_attr_destroy(&attr);
	pthread_exit(NULL);
}