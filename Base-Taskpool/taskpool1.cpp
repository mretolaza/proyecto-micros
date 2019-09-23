/*Probando ejemplo de task pool del libro pag 278
Rauber, Thomas & Runder, Gundula. Parallel Programming for multicore and cluster systems
Fecha 07 de septiembre de 2019*/

/*No dice que librerias implementar, asi que usare las que se han utilizado en clase*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> /* "malloc" utiliza esta libreria */
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

//Representa una tarea a ser ejecutada
/*(typedef) Probemos con void*/
typedef struct work
{
	void (*routine)();
	void *arg;
	struct work *next;
} work_t;

//Contiene la lista de tareas con un puntero a la primera (head) y ultima (tail) tarea
/*(typedef) Probemos con void*/
typedef struct tpool
{
	int num_threads, max_size, current_size;
	pthread_t *threads;
	work_t *head, *tail;
	pthread_mutex_t lock;
	pthread_cond_t not_empty, not_full;
} tpool_t;

//Usado para extraer y ejecutar tareas
void *tpool_thread(void *arg)
{
	work_t *wl;
	tpool_t *tpl = (tpool_t *)arg;

	for (;;)
	{ /*El ejemplo no tiene parametros el for*/
		pthread_mutex_lock(&(tpl->lock));
		while (tpl->current_size == 0)
		{
			pthread_cond_wait(&(tpl->not_empty), &(tpl->lock));
		}
		wl = tpl->head;
		tpl->current_size--;
		if (tpl->current_size == 0)
		{
			tpl->head = tpl->tail = NULL;
		}
		else
		{
			tpl->head = wl->next;
		}
		if (tpl->current_size == tpl->max_size - 1)
		{
			pthread_cond_broadcast(&(tpl->not_full));
		}
		pthread_mutex_unlock(&(tpl->lock));
		/*(*(wl->routine))(wl->arg); Esto da error too many arguments to the function*/
		(*(wl->routine));
		(wl->arg);
		free(wl); /*Elimina un recurso o conjunto de recursos*/
				  /*NOTA: no use malloc () o calloc () con delete / delete [] , y no use new / new [] con free ()*/
	}
}

//Inicializa una estrucura task pool llamada tpl
tpool_t *tpool_init(int num_threads, int max_size)
{
	int i;
	tpool_t *tpl;

	tpl = (tpool_t *)malloc(sizeof(tpool_t)); /* "malloc" se utiliza para asignar un bloque de memoria*/
	tpl->num_threads = num_threads;
	tpl->max_size = max_size;
	tpl->current_size = 0;
	tpl->head = tpl->tail = NULL;

	/* Hubiera creido que estos van en el main (??)*/
	pthread_mutex_init(&(tpl->lock), NULL);
	pthread_cond_init(&(tpl->not_empty), NULL);
	pthread_cond_init(&(tpl->not_full), NULL);
	tpl->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
	for (i = 0; i < num_threads; i++)
	{
		pthread_create(&(tpl->threads[i]), NULL, tpool_thread, (void *)tpl);
	}
	return tpl;
}

//Inserta tareas en el task pool
void tpool_insert(tpool_t *tpl, void (*routine)(), void *arg)
{
	work_t *wl;

	pthread_mutex_lock(&(tpl->lock));
	while (tpl->current_size == tpl->max_size)
	{
		pthread_cond_wait(&(tpl->not_full), &(tpl->lock));
	}
	wl = (work_t *)malloc(sizeof(work_t));
	wl->routine = routine;
	wl->arg = arg;
	wl->next = NULL;
	if (tpl->current_size == 0)
	{
		tpl->tail = tpl->head = wl;
		pthread_cond_signal(&(tpl->not_empty));
	}
	else
	{
		tpl->tail->next = wl;
		tpl->tail = wl;
	}
	tpl->current_size++;
	pthread_mutex_unlock(&(tpl->lock));
}

main()
{

	exit(0);
}
