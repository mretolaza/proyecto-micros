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

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include <sys/types.h>
#include<unistd.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream> //file processing
#include <iomanip> //read file

using namespace std;

int in, out, cont, buffer[88]; 
pthread_cond_t lleno, vacio; 
pthread_mutex_t semaf;

typedef struct caracter{
	int tarea;
	int bit;
	int key;
	int mod;
	int result;
	int fin;
	int b_bits[88];
} caracter;


void escribir (char block) 
{
	ofstream escribir("Textodes.txt", ios::app);
	// Protección en caso el archivo falle en su ejecución
	if (!escribir)
	{
		cerr << "Error. No se ha podido crear el archivo,  Textodes.txt" << endl;
		exit(EXIT_FAILURE);
	}
	escribir<<block;	
}

int inverso (int a, int mod){
	int b, d;
	for (b=0; b<mod; b++){
		d=(a*b)%mod;
		if (d==1){
			return b;
		}
	}
}

int Escribe(int DATO){ 
  pthread_mutex_lock (& semaf); 
  while (cont == 88) 
    pthread_cond_wait(& vacio, &semaf); 
  cont++; buffer[in]= DATO;
  in = (in+1) % 88;
  pthread_cond_broadcast(& lleno); 
  pthread_mutex_unlock(& semaf); 
} 


void operarbit (void *unbit){
	caracter *ps = (caracter *)unbit;
	int result;
	int inv= inverso(ps->key,ps->mod);
	result=(ps->bit)*(inv);
	if (result>(ps->mod)){
		result=result%(ps->mod);
	}
	ps->result=result;
	pthread_exit(NULL);	
}

int Lee(){ 
	int dato; 
	char midato;
	pthread_mutex_lock(& semaf);
	while (cont == 0) 
	    pthread_cond_wait(& lleno, &semaf); 
	cont--; dato = buffer[out]; 
	out = (out+1) % 88;
	pthread_cond_broadcast(& vacio); 
	pthread_mutex_unlock(& semaf); 
	midato = static_cast<char>(dato);
	return midato; 
} 


void productor(void * arg){
	caracter *ps = (caracter *)arg;
  	int i, final; 
  	final=ps->fin;
	for (i= 0; i< 88; i++) 
	    Escribe(ps->b_bits[i]); 
	pthread_exit(0); 
} 

void *taskpool (void *argumento){

	caracter *ps = (caracter *) argumento;

	switch(ps->tarea){
		case 1:
			operarbit(argumento);
		break;
		case 2:
			productor(argumento);
		break;
	}

	return NULL;
}

int main(int argc, char *argv[])
{ 
  	in = out = cont = 0;

	//Declaracion de la estructura

	caracter parametros;

	int llave[10]={
		2,3,5,7,11,
		31,37,41,43,47
	};

 	
 	//Inicializacion de variables de condicion

	pthread_mutex_init(&semaf, NULL); 
  	pthread_cond_init(&lleno, NULL); 
  	pthread_cond_init(&vacio, NULL);

  	//Inicializacion de variables del pthread

  	int rc; //valor de retorno del pthread
	pthread_t tid;
	  
	pthread_attr_t attr;
	
	pthread_attr_init(&attr);
	  
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int f=0;
	char x;

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
		int y= static_cast<unsigned char>(x);
		
		y=y-33;

		parametros.bit=y;

		int w=9;

		// 10 rondas de encripcion

		while (w>=0){

			parametros.tarea=1;
			parametros.key=llave[w];

			switch (w){
				case 9:
					parametros.mod=223;
				break;
			}
			
			rc = pthread_create(&tid, &attr, taskpool, (void *)&parametros);
						
			if (rc) {              
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}

			rc = pthread_join(tid, NULL);
			parametros.bit=parametros.result;
			if (rc) {
				printf("ERROR; return code from pthread_join() is %d\n", rc);
				exit(-1);
			}
			w--;
		}

		f++;

		//Si no es ningun caracter fuera de rango

		if (parametros.result>0){
			if (parametros.result!=215){
				if (x=='|'){
					parametros.result=32;
				}
				parametros.b_bits[f]=parametros.result;
			}
		}
		
		
		//Utiliza las variables de condicion para escribir el texto

		if (f%88==0)
		{ 	
			parametros.fin=88;
			parametros.tarea = 2;
			pthread_create(&tid,NULL,taskpool,(void *)&parametros);
			int e=0;
			for (e=0; e<88; e++){
				escribir(Lee());
			}
			f=0;
		}
	}

	// Si un bloque esta incompleto

	if (f<88)
	{ 	
		parametros.fin=f;
		parametros.tarea = 2;
		pthread_create(&tid,NULL,taskpool,(void *)&parametros);
		int e=0;
		for (e=0; e<f; e++){
			escribir(Lee());
		}
		f=0;
		cout<<endl<<endl;
	}

	cout << endl;

	printf("\n#####Desencripcion Finalizada#####\n");

	pthread_attr_destroy(&attr);
	pthread_exit(NULL);

  	exit(0); 
}


