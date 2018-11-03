#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int value = 5;

int main()
{
	pid_t pid;

	pid = fork();
	
	if (pid == 0) { /* child process */
		value += 15;
		printf("CHILD: Value = %d \n", value);
		fflush(stdout);
		return 0;
	}
	else if (pid > 0) { /* parent process */
		wait(NULL);
		printf ("PARENT: value = %d\n",value); /* LINE A */
		return 0;
	}
}
