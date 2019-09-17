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
};

typedef struct opb{
	int bit;
	int key;
	int mod;
	int result;
} opb; 


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
			cout<<"["<<b<<"]";
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


void *operarbit (void *unbit){
	opb *ps = (opb *)unbit;
	/*if (((ps->bit)-33)<33){
		ps->bit=(ps->bit)-33;
	}*/
	int result;
	int inv= inverso(ps->key,ps->mod);
	result=(ps->bit)*(inv);
	cout<<" * "<<result<<" * ";
	if (result>(ps->mod)){
		result=result%(ps->mod);
		cout<<" ¡ "<<result<<" ! ";
		/*if (result<33){
			result=result+33;
		}*/
	}
	//ps->result=char(result);
	ps->result=result;
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


void *productor(void * arg){
	struct benc *ps;
	ps=(struct benc *)arg; 
  	int i; 
	for (i= 0; i< 88; i++) 
	    Escribe(ps->b_bits[i]); 
	pthread_exit(0); 
} 

int main(int argc, char *argv[])
{ 
  	//int i; 
  	pthread_t hijo;
  	in = out = cont = 0;

  	struct benc ben;

	//struct opb opbe;
	opb opbe;

	int codllave[88]={
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

	/*Esta se uso para encriptar, tomar de base y calcular sus inversos multiplicativos 
	en 100,140,150,160,170,180,190,200,210,220*/
	int llave[88]={
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
		101,103,109,/*107,113,
		151,157,163,167,173,
		199,211,223,*/227,229,
		263,269,271,277,281,
		317,331,337,347,349,
		383,389,397,401,409,
		443,449,457,461,463,
		487,491,467,479,499,
		503,509,521
	};

 
	pthread_mutex_init(&semaf, NULL); 
  	pthread_cond_init(&lleno, NULL); 
  	pthread_cond_init(&vacio, NULL);

  	int rc, rc1; //valor de retorno del pthread
	pthread_t tid;
	  
	pthread_attr_t attr;
	
	pthread_attr_init(&attr);
	  
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	//char cadini[88];
	int f=0;
	char x;
  	ifstream texto("Textoencriptado.txt", ios::in);

	if (!texto)
	{
		cerr << "Error, No se puede abrir primos.txt";
		exit(EXIT_FAILURE); // terminate with error
	}


	while (texto >> x)
	{
		int y= static_cast<unsigned char>(x);
		//int y = atoi(x);
		cout<<y<<" - ";
		y=y-33;

		cout<<y<<" - ";

		opbe.bit=y;
		opbe.key=llave[f];

		int w=0;
		opbe.mod=223;

		rc = pthread_create(&tid, &attr, operarbit, (void *)&opbe);
						
		if (rc) {              
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}

		rc = pthread_join(tid, NULL);
		if (rc) {
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
		/*while (w<9){
			switch (w){
				case 0:
					opbe.mod=113;
				break;
				case 1:
					opbe.mod=151;
				break;
				case 2:
					opbe.mod=157;
				break;
				case 3:
					opbe.mod=163;
				break;
				case 4:
					opbe.mod=167;
				break;
				case 5:
					opbe.mod=173;
				break;
				case 6:
					opbe.mod=199;
				break;
				case 7:
					opbe.mod=211;
				break;
				case 8:
					opbe.mod=223;
				break;
			}
			
			rc = pthread_create(&tid, &attr, operarbit, (void *)&opbe);
						
			if (rc) {              
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}

			rc = pthread_join(tid, NULL);
			//char res;
			//res=opbe.result;
			opbe.bit=opbe.result;
			if (rc) {
				printf("ERROR; return code from pthread_join() is %d\n", rc);
				exit(-1);
			}
			w++;
			//escribir(res);
		}*/

		f++;

		/*opbe.mod=220;
		//opbe.key=llave[perini[f]-1];
		rc = pthread_create(&tid, &attr, operarbit, (void *)&opbe);
						
		if (rc) {              
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}

		rc = pthread_join(tid, NULL);
		//char res;
		//res=opbe.result;
		//escribir(res);
		/*res=res+' ';
		if (res==157){
			res=' ';
		}*/
		//ben.b_bits[perini[f]-1]=res;
		cout<<y<<endl;
		ben.b_bits[f]=static_cast<char>(opbe.result);
		escribir(static_cast<char>(opbe.result));
		//ben.b_bits[f]= itoa(opbe.result);
		/*if (rc) {
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}*/
		
		/*if (f%88==0)
		{ 	
			pthread_create(&hijo,NULL,productor,(void *)&ben);
			int e=0;
			for (e=0; e<88; e++){
				escribir(Lee());
			}	
			f=0;
		}*/
	}

	cout << endl;

	pthread_attr_destroy(&attr);
	pthread_exit(NULL);

  	exit(0); 
}


