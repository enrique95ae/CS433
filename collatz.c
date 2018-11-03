/*
  NAME: Enrique Alonso Esposito
  DATE: Feb 12 2018
  ASSINGMENT: Homework #2
  PURPOSE: Use the system call fork() in order to create a program that uses the Collatz algorithm in order to reach 1.
*/

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

//Function Prototypes:
int collatz(int);


//Global variables
int n;


int main(int argc, char* argv[])
{
  //Input validation
  //NOTE: there is no max limit.
  if(argc == 1)
    {
      printf("Please enter a valid interger \n");
      return 1;
    }

  char* input = argv[1];
    
  //Converting from char into int.
      n = atoi(input);
      

      //FORK 
      pid_t pid;
      pid = fork();
      
      if(pid==0)
      {
	//Printing the proccess id
	printf("CHILD value: %d\n", pid);

	//If the sequence hasn´t finished yet
	while(n > 1)
	  {
	    //Printing the current int and doing the next step of the algorithm by calling the function
	    printf("%d ", n);
	    n = collatz(n);
	  }

	//Once it reaches 1 it prints it out and gets out of the process 
	printf("%d \n", n);
	return 0;
      
      }
      else if(pid > 0) 
      {
	//wait if the fork process hasn´t finished
	  wait(pid);
          return 0;
       }
}


//Function implementations:


//Purpose: get an int, determine if even/odd and return accordingly
int collatz(int currentInt)
{
 
  //If even
  if(currentInt%2 == 0)
    {
      return currentInt/2;
    }

  //If odd
  if(currentInt%2 == 1)
    {
      return (currentInt*3)+1;
    }
}
