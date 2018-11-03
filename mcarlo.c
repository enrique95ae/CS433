/*
  PROGRAM: Monte Carlo PI approximation using threads.
  AUTHOR: Enrique Alonso Esposito
  DATE: Feb 21st 2018
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
//#include <time.h>

//Global Variables
double hit_count = 0;

//Generates a random num
double randomNum()
{
  // srand(time(NULL));
  return random() / ((double)RAND_MAX + 1);
}

//PURPOSE: creates a thread. generates a number within a square. If the numbers happen to be also in the inscribed
//circle increments hit_count.
void *monteCarlo(void* argv)
{
  //conversion from char input argument to int.
  char* stillChar = (char*)argv;
  int num = atoi(stillChar);

  //num = input = number of points the loop is going to plot and check.

  int i; 

  double x, y; //set of coordinates.

  for(i = 0; i < num; i++)
    {
      x = randomNum() * 2.0 - 1.0;
      y = randomNum() * 2.0 - 1.0;

      if(sqrt(x*x + y*y) < 1.0)
	{
	  hit_count++; //if the point happens to be IN the inscribed circle --> increment hit_count
	}
    }
  pthread_exit(0); 
}
      

int main(int argc, char *argv[])
{
  int thread_count = 5; //number of threads
  pthread_t tid[thread_count];
  int counter = 0;

  //validations
  if(argc == 1)
    {
      printf("More arguments needed.");
      return 0;
    }
  else if(argc > 2)
    {
      printf("Too many arguments.");
      return 0;
    }

  double input = atoi(argv[1]);
  int i;

  for(i = 0; i < thread_count; i++)
    {
      pthread_create(&tid[i], NULL, &monteCarlo, (void *)argv[1]);  
    }

  for(i = 0; i<thread_count; i++)
    {
      pthread_join(tid[i], NULL);
    }

  //PI approximation and printting out
  double pi = ((4* hit_count)/input)/thread_count;
  printf("\n PI approximation: %f\n", pi);
  return 0;
}


