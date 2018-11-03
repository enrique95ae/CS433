#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


int main(int argc ,char *argv[])
{
  union  sigval  value;
  pid_t pid;

  if (argc != 2) {
    printf("Usage:: %s <pid> \n", argv[0]);
    exit(1);
  }

  pid = atoi(argv[1]);

  printf("Sending signal %d to pid = %d \n", SIGUSR1, pid);

  if (sigqueue(pid, SIGUSR1, value) == -1)  {
    printf("Error:: sending signal %d to pid = %d \n", SIGUSR1, pid);
    exit(-1);
  }

  exit(0);
}
