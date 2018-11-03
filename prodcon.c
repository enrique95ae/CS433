#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sched.h>

#define BUFFER_SIZE 64000

//To compile: gcc prodcon.c -o prodconExec -lpthread -lm -lrt

//NOT IMPLEMENTED:
/*

  -checksum
  -FIX: producer and random ints. (every two ints, one is = 0)

 */

//function prototypes:
void *Producer(void*);
void *Consumer(void*);
void setlock();
void setunlock();

//global variables:
int buffer[BUFFER_SIZE];
int ntimes = 0;
int memsize = 0;
int checksum = 0;
sem_t prod, cons;
pthread_mutex_t mutex;


int main(int argc, char* argv[])
{
  if(argc != 3)
    {
      printf("Enter the command as follows: ./prodcon <memsize> <ntimes> \n");
      exit(1);
    }
  if(atoi(argv[2]) > 2000)
    {
      perror("ERROR: memsize too large. Try less than 2000. \n");
      exit(1);
    }
  if(atoi(argv[2]) < 1)
    {
      perror("ERROR: ntimes must be positive");
      exit(1);
    }

  memsize = atoi(argv[1]);
  ntimes = atoi(argv[2]);

  if(sem_init(&prod, 0, 1)<0)
    {
      perror("sem_init");
      exit(1);
    }
  
  if(sem_init(&cons, 0, 1) <0)
    {
      perror("sem_init");
      exit(1);
    }


  pthread_t tid[2];

  if(pthread_create(&tid[0], NULL, &Producer, NULL)!=0){
    perror("pthread_create");
    exit(1);
  }
  if(pthread_create(&tid[1], NULL, &Consumer, NULL)!=0){
    perror("pthread_create");
    exit(1);
  }

  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);

  sem_destroy(&Producer);
  sem_destroy(&Consumer);
  
  pthread_mutex_destroy(&mutex);

  return 0;
}

void setlock()
{
  pthread_mutex_lock(&mutex);
}

void setunlock()
{
  pthread_mutex_unlock(&mutex);
}

//PRODUCER:
/*
  waits for the consumer to be done
  setslock
  generates a random int 0-255 
  stores it in the buffer
  unlocks
*/
void *Producer(void* argv)
{
  int i;
  int random = 0;

  srand(time(NULL));
  for(i=0; i<ntimes; i++)
    {
      sem_wait(&cons);
      setlock();
      sched_yield();
      //srand(time(NULL));
      random = rand()%256;
      buffer[i] = random;
      setunlock();
      sem_post(&prod);

    }
}
//CONSUMER
/*
  waits for the producer to be done
  setslock
  prints data in buffer
  unlocks
*/
void *Consumer(void* argv)
{
  int i;

  for(i=0; i<ntimes; i++)
    {
      sem_wait(&prod);
      setlock();
      sched_yield();	
      printf("buffer[%d] = %d \n", i, buffer[i % BUFFER_SIZE]);
      setunlock();
      sem_post(&cons);
    }
}
  


