#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <errno.h>


/* Function prototypes */
 
static void sig_handler( int );


sem_t semid;

int main(int argc ,char *argv[])
{

  int i, j;

  signal(SIGUSR1, sig_handler);

  if (sem_init(&semid, 0, 0) == -1) {
    fprintf(stderr, "unable to initialize semaphore, errno = %d (%s) \n",
	    errno, strerror(errno));
    return EXIT_FAILURE;
  }


  sem_wait(&semid); 

  printf("Semaphore 0x%x available \n", semid); 

  sem_destroy(&semid);

  return EXIT_SUCCESS;

}

static void sig_handler(int signo)
{

  printf("received signal #%d \n", signo);
  sem_post(&semid);
}
