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

struct bloques{
	char b_bits[44];
};

struct benc{
	char b_bits[94];
};

struct bloque{
	char b_bits[47];
};

typedef struct opb{
	char bit;
	int key;
	int mod;
	char result;
} opb;

void *escribir (void *block) 
{
	struct benc *ps;
	ps=(struct benc *)block;

	ofstream escribir("Textoencriptado.txt", ios::app);
	// Protección en caso el archivo falle en su ejecución
	if (!escribir)
	{
		cerr << "Error. No se ha podido crear el archivo,  Textoencriptado.txt" << endl;
		exit(EXIT_FAILURE);
	}
	escribir<<ps->b_bits<<endl;	
}

void *operarbit (void *unbit){
	opb *ps = (opb *)unbit;
	int result;
	result=(ps->bit)*(ps->key);
	if (result>(ps->mod)){
		result=result%(ps->mod);
		if (result<33){
			result=result+'!';
		}
	}
	ps->result=char(result);
	pthread_exit(NULL);	
}

void *output(void *block)
{
	struct bloques *ps;
	ps=(struct bloques *)block;
	cout <<ps->b_bits<<endl<<endl;
}


int main(int argc, char *argv[])
{
	//Declaracion de la estructura
	struct bloques bls;

	struct benc ben;

	//struct opb opbe;
	opb opbe;

	int perini[88]={
		63,44,88,61,83,
		37,45,79,3,31,
		68,60,78,74,29,
		25,81,43,76,73,
		42,62,10,1,72,
		2,30,49,38,84,
		59,20,7,57,80,
		6,70,64,4,41,
		48,82,28,40,85,
		71,9,15,24,56,
		27,8,50,67,58,
		55,46,22,12,87,
		52,65,51,11,32,
		21,35,36,66,26,
		77,34,47,33,5,
		39,17,13,16,86,
		53,23,54,14,75,
		69,18,19};

	int llave[94]={
		/*2,3,5,7,*/11,
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
		443,449,457,461,463,
		487,491/*,467,479*/
	};

	int rc, rc1; //valor de retorno del pthread
	long i=0;
	  
	pthread_t tid;
	  
	pthread_attr_t attr;
	
	pthread_attr_init(&attr);
	  
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	char cadini[88];
	int f=0;
	char x;
	opbe.result='a';

	ifstream texto("Prueba.txt", ios::in);

	if (!texto)
	{
		cerr << "Error, No se puede abrir primos.txt";
		exit(EXIT_FAILURE); // terminate with error
	}

	while (texto >> x)
	{
		//int y= static_cast<unsigned char>(x);
		char y=x;
		if (y==' '){
			y=157;
		}

		y=y-' ';

		opbe.mod=220;
		opbe.bit=y;
		opbe.key=llave[f];
		
		rc = pthread_create(&tid, &attr/*NULL*/, operarbit, (void *)&opbe);
						
		if (rc) {              
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}

		rc = pthread_join(tid, NULL);
		char res;
		res=opbe.result;
		ben.b_bits[perini[f]-1]=res;
		opbe.bit=res;
		if (rc) {
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
		int w=0;
		while (w<9){
			opbe.mod=opbe.mod-10;
			if (w==8){
				opbe.mod=100;
			}
			rc = pthread_create(&tid, &attr/*NULL*/, operarbit, (void *)&opbe);
						
			if (rc) {              
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}

			rc = pthread_join(tid, NULL);
			char res;
			res=opbe.result;
			ben.b_bits[f]=res;
			opbe.bit=res;
			if (rc) {
				printf("ERROR; return code from pthread_join() is %d\n", rc);
				exit(-1);
			}
			w++;
		}

		f++;
		
		if (f%88==0)
		{ 	
			
			rc = pthread_create(&tid, &attr, escribir, (void *)&ben);
						
			if (rc) {              
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}

			rc = pthread_join(tid, NULL);
			if (rc) {
				printf("ERROR; return code from pthread_join() is %d\n", rc);
				exit(-1);
			}
				
			f=0;
		}
	}

	cout << endl;

	pthread_attr_destroy(&attr);
	pthread_exit(NULL);
}



