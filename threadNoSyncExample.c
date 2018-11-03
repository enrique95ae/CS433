#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>

pthread_t tid[2];

int counter = 5;

void *producer(void *arg)
{
  int i, register1;

  /* Begin the critical section */
  register1 = counter;
  register1 = register1 + 1;

  /* use loop to simulate some processing */
  for (i = 0; i< (0xFFFFF); i++);

  /* enforce a thread switch to illustrate "interleaving" */
  sched_yield();

  counter = register1;
  /* end the critical section */

  printf("%s:Counter = %d \n", __FUNCTION__, counter);

  return NULL;
}

void *consumer(void *arg)
{

  int i,register2;

  /* Begin the critical section */
  register2 = counter;
  register2 = register2 - 1;

  /* use loop to simulate processing */
  for (i = 0; i< (0xFFFFF); i++);

  counter = register2;
  /* End the critical section */

  printf("%s:Counter = %d \n", __FUNCTION__, counter);

  return NULL;
}

int main(void)
{
  int i = 0;
  int err;

  err = pthread_create(&(tid[0]), NULL, &producer, NULL);
  if (err != 0)
    printf("\ncan't create thread 1 :[%s]", strerror(err));

  err = pthread_create(&(tid[1]), NULL, &consumer, NULL);
  if (err != 0)
    printf("\ncan't create thread 2 :[%s]", strerror(err));

  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);

  printf("Final Counter = %d \n", counter);

  return 0;

}
