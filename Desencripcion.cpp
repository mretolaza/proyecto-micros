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

struct benc{
	char b_bits[94];
};

struct dato{
	char dat;
}

typedef struct opb{
	char bit;
	int key;
	int mod;
	char result;
} opb; 


void *escribir (void *block) 
{
	struct dato *ps;
	ps=(struct dato *)block;

	ofstream escribir("Textodes.txt", ios::app);
	// Protección en caso el archivo falle en su ejecución
	if (!escribir)
	{
		cerr << "Error. No se ha podido crear el archivo,  Textodes.txt" << endl;
		exit(EXIT_FAILURE);
	}
	escribir<<ps->dat<<endl;	
}

int Escribe(int DATO){ 
  pthread_mutex_lock (& semaf); 
  while (cont == 88) 
    pthread_cond_wait(& vacio, &semaf); 
  cont++; buffer[in]= DATO;
  in = (in+1) % 88;
  //int i;
  //for(i=0; i<(0xFFFFFF);i++);
  pthread_cond_broadcast(& lleno); 
  pthread_mutex_unlock(& semaf); 
} 


void *operarbit (void *unbit){
	opb *ps = (opb *)unbit;
	if (((ps->bit)-33)<33){
		ps->bit=(ps->bit)-33;
	}
	int result;
	result=(ps->bit)+(ps->key);
	if (result>(ps->mod)){
		result=result%(ps->mod);
		if (result<33){
			result=result+'!';
		}
	}
	Escribe(ps->result);
	//ps->result=char(result);
	pthread_exit(NULL);	
}

int Lee(){ 
  int dato; 
  pthread_mutex_lock(& semaf);
  while (cont == 0) 
    pthread_cond_wait(& lleno, &semaf); 
  cont--; dato = buffer[out]; 
  out = (out+1) % 88;
  pthread_cond_broadcast(& vacio); 
  pthread_mutex_unlock(& semaf); 
  return dato; 
} 

main(){ 
  int i; 
  pthread_t hijo;
  in = out = cont = 0;

  struct benc ben;

	//struct opb opbe;
	opb opbe;

	int perini[88]={
		24,26,9,39,75,
		36,33,52,47,23,
		64,59,78,84,48,
		79,77,87,88,32,
		66,58,82,49,16,
		70,51,43,15,27,
		10,65,74,72,67,
		68,6,29,76,44,
		40,21,18,2,7,
		57,73,41,28,53,
		63,61,81,83,56,
		50,34,55,31,12,
		4,22,1,38,62,
		69,54,11,86,37,
		46,25,20,14,85,
		19,71,13,8,35,
		17,42,5,30,45,
		80,60,3};


	int llave[94]={
		2,3,5,7,11,
		31,37,41,43,47,
		73,79,83,89,97,
		127,131,137,139,149,
		179,181,191,193,197,
		233,239,241,251,257,
		283,293,307,311,313,
		353,359,367,373,379,
		419,421,431,433,439,
		13,17,19,23,29,
		53,39,61,67,71,
		101,103,109,107,113,
		151,157,163,167,173,
		199,211,223,227,229,
		263,269,271,277,281,
		317,331,337,347,349,
		383,389,397,401,409,
		443,449,457/*,461,463,
		487,491,467,479*/
	};

 
  pthread_mutex_init(&semaf, NULL); 
  pthread_cond_init(&lleno, NULL); 
  pthread_cond_init(&vacio, NULL);
  pthread_create(&hijo,NULL,productor,NULL);
  /*printf("Padre\n");
  for (i= 0; i< 100; i++) 
    printf("%d\n ", Lee());*/
  exit(0); 
}

