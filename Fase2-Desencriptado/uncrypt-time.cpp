/* 
Universidad del Valle de Guatemala
Proyecto 2 de Microprocesadores
Encripcion de un archivo txt
Fecha: 26 de septiembre de 2019
Programa final
Integrantes:
-Diana Ximena de Leon Figueroa
-Maria Mercedes Retolaza Reyna
-Andrea Abril Palencia Gutierrez
-Cristina Maria Bautista Silva 
*/
//Declaración de librerías
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
#include <time.h>  //variable de tiempo

using namespace std;

//declaración de variables a utilizar
int in, out, cont, buffer[88];
//Inicialización de variables mutex
pthread_cond_t fullValue, emptyValue;
pthread_mutex_t semaf;

//Creacion de estructuras
//que se utiizarán para el desencriptado
//del texto o palabra cargada en el archivo .txt
typedef struct charOfValue
{
	int task;
	int bit;
	int key;
	int mod;
	int result;
	int fin;
	int b_bits[88];
	FILE *desfile; //se declara dentro de la estructura el archivo .txt a desencriptar
} charOfValue;

//Se crea la función que se encarga de escribir en el texto
//la palabra obtenida. Se hace uso de una variable interna
//función que tiene c++
void writeToFileText(char block, FILE *desfile)
{
	fwrite(&block, 1, 1, desfile);
}

//Recibe un valor (a) y recibe un mod
//este se encarga de obtener el inverso de cada uno de los
//valores en el mod que nosotros estámos solicitando en dicho
//momento. Estos valores se usan en la desencripción
//del texto encriptado recibido.
int reverseInfo(int a, int mod)
{ //declación de variables internas temporales
	int b, d;
	//ciclo donde se obtiene el inverso según el mod indicado.
	for (b = 0; b < mod; b++)
	{
		d = (a * b) % mod;
		//validación que el valor devuelto sea exacto
		if (d == 1)
		{
			return b; //valor a utilizar
		}
	}
}

//En esta sección se pasa el parámetro que contiene los valores
//a desencriptar. Entonces dicha función se escribe en el buffer
//la cadena de caractéres final que se obtiene después de la desencripción
int writeToFile(int valueOfUncrypt)
{
	pthread_mutex_lock(&semaf);
	while (cont == 88) //separador de 88 bits establecido en flujo inicial
		pthread_cond_wait(&emptyValue, &semaf);
	cont++;
	buffer[in] = valueOfUncrypt;		//el buffer recibe la cadena encriptada
	in = (in + 1) % 88;					//incremento de bloque
	pthread_cond_broadcast(&fullValue); //uso de condiciónes mutex
	pthread_mutex_unlock(&semaf);
}

//Obtiene el valor al que pertenece el caracter encriptado
//retorna al módulo obtenido y obtiene el valor deseado
//lo procesa y lo retorna al resultado.
void getBitOfUncrypt(void *firstBit)
{
	charOfValue *ps = (charOfValue *)firstBit; //se declara la estructura
	int result;								   //variable temporal result
	int inv = reverseInfo(ps->key, ps->mod);   //se obtiene el modulo al que pertenece
	result = (ps->bit) * (inv);
	if (result > (ps->mod))
	{
		result = result % (ps->mod);
	}
	ps->result = result; //retorna el resultado
	pthread_exit(NULL);  //destruye el hilo utilizado
}

//se encarga de leer el archivo e instanciar los
//hilos que se usarán en el proceso de desencripción
int reedFile()
{
	int valueOfUncrypt;			//variable temporal que declara la cadena a desencriptar
	char myValue;				//variable temporal que nos indica la palabra en la cual estamos
	pthread_mutex_lock(&semaf); //instancia mutex
	while (cont == 0)
		pthread_cond_wait(&fullValue, &semaf);
	cont--;
	valueOfUncrypt = buffer[out]; //se guarda el valor en el buffer
	out = (out + 1) % 88;		  //se aplica distancia de bloque
	pthread_cond_broadcast(&emptyValue);
	pthread_mutex_unlock(&semaf);
	myValue = static_cast<char>(valueOfUncrypt);
	return myValue; //retorna el valor final
}

//Obtiene el texto encriptado y lo converiete al texto
//real de la palabra.
void getText(void *arg)
{
	charOfValue *ps = (charOfValue *)arg;
	int i, final;
	final = ps->fin;
	for (i = 0; i < 88; i++)
		writeToFile(ps->b_bits[i]);
	pthread_exit(0);
}
//Implementación del taskpoll de tareas
//Resumen: se encarga de recibir las tareas que debe de realizar y
//ejecuta de acorde al valor que retorna el argumento.
void *taskpool(void *argumento)
{

	charOfValue *ps = (charOfValue *)argumento; //establecer bloqueo antes utilizar recurso
	//Declaración de valor a descriptar (obtiene el valor almanecado)
	//Case de tareas, deacorde al valor (1 o 2) será
	//el indicador de la tarea que deberá de ejecutar.
	switch (ps->task)
	{
	case 1:
		getBitOfUncrypt(argumento); //llamado a la función
		break;
	case 2:
		getText(argumento); //llamado a la función
		break;
	}

	return NULL;
}

//Inicio del programa principal
int main(int argc, char *argv[])
{
	clock_t t_ini, t_fin; //variable de inicio de tiempo
	double secs;		  // variable de fin
	//inicialización de variables contadoras globales
	//en 0
	in = out = cont = 0;

	//Declaracion de la estructura
	charOfValue params;

	//declaración de las llaves
	int llave[10] = {
		2, 3, 5, 7, 11,
		31, 37, 41, 43, 47};

	t_ini = clock(); // inicio de conteo de tiempo
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

	//creación de variables temporales
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
	t_fin = clock(); //Fin de conteo de tiempo
	//retorno de total de tiempo
	secs = (double)(t_fin - t_ini) / CLOCKS_PER_SEC;
	printf("%.16g milisegundos\n", secs * 1000.0);

	pthread_attr_destroy(&attr);
	pthread_exit(NULL);

	exit(0);
}
