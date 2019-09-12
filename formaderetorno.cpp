#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
using namespace std;

typedef struct thread_data
{
   int a;
   int b;
   int result;

} thread_data;

void *rutineThread(void *arg)
{
   thread_data *tdata = (thread_data *)arg;

   int a = tdata->a;
   int b = tdata->b;
   int result = a + b;

   tdata->result = result;
   pthread_exit(NULL);
}

int main()
{
   pthread_t tid;
   thread_data tdata;
   int a;
   int b;

   cout << "Ingresa el valor para sumar de la variable a:";
   cin >> a;

   cout << "Ingresa el valor para sumar de la variable b:";
   cin >> b;

   tdata.a = a;
   tdata.b = b;

   pthread_create(&tid, NULL, rutineThread, (void *)&tdata);
   pthread_join(tid, NULL);

   printf("%d + %d = %d\n", tdata.a, tdata.b, tdata.result);

   return 0;
}