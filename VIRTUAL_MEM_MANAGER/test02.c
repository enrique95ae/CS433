#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

const int PAGE_SIZE = 256;
const int VM_SIZE = 256;
const int MM_SIZE = 128;
const int TLB_SIZE = 16;


int main(int argc, char *argv[]){

  int physical_memory[MM_SIZE];
  int virtual_memory[VM_SIZE][2];
  int tlb[TLB_SIZE][2];

  // INITIAL PAGE TABLE FILLING
  // [0] Physical Address [1] Age
  int i;
  for(i = 0; i < VM_SIZE; i++){
    virtual_memory[i][0] = (i > MM_SIZE - 1)? -1 : i;
    virtual_memory[i][1] = (i > MM_SIZE - 1)? -1 : MM_SIZE - i;
  }
  int j;
  for(j = 0; j < MM_SIZE; j++){
    physical_memory[j] = j;
  }

  int k;
  for(k = 0; j < TLB_SIZE; j++){
    tlb[j][0] = (j > TLB_SIZE - 1)? -1 : j;
    tlb[j][1] = (j > TLB_SIZE - 1)? -1 : TLB_SIZE - j;
  }

  // Check to see if correct arguments exist
  if(argc != 2){
    printf("Incorrect Number of Arguments.\n");
    return 1;
  }

  FILE *pFile;
  pFile = fopen(argv[1], "r");

  //checks to see if the .txt file supplied is empty
  if(pFile == NULL){
    printf("Error opening a file %s: %s\n", argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  int page_number = 0;
  int physicalAddress = 0;
  int pageFaultCount = 0;

  double pageFaultRate = 0.0;

  printf("==================================\n" );
  printf("||    Virtual Memory Manager    ||\n" );
  printf("==================================\n" );

  printf("Translating 1000 Logical Addresses: \n\n");


  while((read = getline(&line, &len, pFile)) != -1){
    int offset = atoi(line) & 255;
    int page = atoi(line) & 65280;
    int page_table_number = page >> 8;
    int tlb_hit = 0;

    int x;
    for(x = 0; x < TLB_SIZE; x++){
      if(tlb[x][0] == page_table_number){
	tlb_hit = 1;
	printf("TLB HIT\n");
	break;
      }
    }

    if(virtual_memory[page_table_number][0] < 0 && !tlb_hit){
      pageFaultCount++;
      srand(time(NULL));
      int r = rand();

      // EVICT SOMEONE
      int largest = 0;
      int evict = 0;
      
      int y;
      for(y = 0; y < VM_SIZE; y++){
	if(virtual_memory[y][1] > largest){
	  largest = virtual_memory[y][1];
	  evict = y;
	}
      }

      int tlb_replacement = r % 15;
      tlb[tlb_replacement][0] = page_table_number;
      tlb[tlb_replacement][1] = virtual_memory[evict][0];
      virtual_memory[page_table_number][0] = virtual_memory[evict][0];
      virtual_memory[page_table_number][1] = 0;
      virtual_memory[evict][0] = -1;
      virtual_memory[evict][1] = 0;
    }

    //printing formatting for Virtual Address
    if (page < 100) {
      printf("Virtual Address = %d     \t", page);
    }
    else if (page < 1000) {
      printf("Virtual Address = %d   \t", page);
    }
    else {
      printf("Virtual Address = %d  \t", page);
    }


    //calculates the physical address
    physicalAddress = (physical_memory[virtual_memory[page_table_number][0]] * PAGE_SIZE) + offset;


    printf("Physical Address: %d\n", physicalAddress);


    page_number++;

    int z;
    for(z = 0; z < VM_SIZE; z++){
      virtual_memory[z][1]++;
    }
  }

  //calculates the % of page faults
  pageFaultRate = (double) pageFaultCount / 1000 * 100;
  printf("\nPage Fault Rate: %.2f%% \n", pageFaultRate);

  free(line);
  fclose(pFile);

  exit(EXIT_SUCCESS);

}
