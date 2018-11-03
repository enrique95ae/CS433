//Virtual Memory Manager
//Author: Enrique Alonso Esposito
//Class: CS 433  -   Operative Systems
//Professor: Tom Springer
//Date: April 14th, 2018
//Homework Assignment: 5


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


//GLOBAL VARIABLES, CONSTANTS AND DEFS:

typedef enum { false, true } bool;

#define Address_MASK      0xFFFF //Making the address out of the 32-bit
#define Offset_MASK       0XFF   //Masking the offset out of the 32-bit
#define BUFFER            5      //Characters from file
#define BYTES             256    //Num of bytes we are reading.

#define NumFrames  256
#define FrameSize  256
#define PageSize  256
#define PMsize  256
#define TLBsize  16


//Arrays:

int PM[NumFrames][FrameSize]; //Physical Memory
int TLBpageNums[TLBsize]; // hold the page numbers in the TLB    MAX16
int TLBframeNums[TLBsize]; //hold the frame numbers in the TLB   MAX16
int pageTableNums[PageSize]; //hold the page nums in the page table
int pageTableFrames[PageSize]; //hold the frame nums in the page table
int allAddresses[1000];  //Array that holds all the addresses in the .txt file





char address[BUFFER]; //String of Chars that holds the address read from the txt file 
int logicalAddress;  //Converted from the string of chars read from file
char fromBacking[BYTES]; //holds data read from BACKING_STORE.bin

/*Counters*/
int pageFaults = 0;
int TLB_Hits = 0;
int pageNum = 0;
int frameNum = 0;
int physicalAddress = 0;
int value = -1;
int offset = 0;
int FirstFrameNum = 0;
int FirstPageTableNum = 0;
int TLBcount = 0;
int countTranslated = 0;

double faultRate = 0.0;
double TLBhitRate = 0.0;

//Input Files:
FILE *addressesFile;
FILE *backingStoreFile;

//FUNCTIONS DECLARATIONS:
void printStats(int);
int getPageNum(int);
int getOffset(int);
void intoTLB(int, int);
void readBackingStore(int);
void FIFOintoTLB(int, int);
void logicalToPhysical(int, int, int);

int main(int argc, char *argv[])
{
  //Number of arguments check.
  if(argc != 2){
    printf("ERROR: number of arguments doesn't match. \n");
    return 1;
  }

  backingStoreFile = fopen("BACKING_STORE.bin", "rb");
  addressesFile = fopen(argv[1], "r"); //Opens the file (variable r)

  int i;
  for(i = 0; i < 1000 ; i++)
    {
      int temp;
 
      fgets(address, BUFFER, addressesFile);
      logicalAddress = atoi(address);
      
      pageNum = getPageNum(logicalAddress);
      offset = getOffset(logicalAddress);


      printf("Interaction number # %d :   \n", i);

      logicalToPhysical(pageNum, offset, logicalAddress);
      countTranslated++;
    }

  faultRate = pageFaults / (double)countTranslated;
  TLBhitRate = TLB_Hits / (double)countTranslated;

  printf("Page Faults: %d\n", pageFaults);
  printf("Fault rate: %d\n", faultRate);
  printf("TLB hits: %d\n", TLB_Hits);
  printf("TLB hit rate: %d", TLBhitRate);

  fclose(addressesFile);
  fclose(backingStoreFile);
  
  return 0;
} //main end



//FUNCTIONS IMPLEMENTATIONS:


//Parameters:  32 bit address from the file
//Purpose: Uses the masks previously defnied in order to mask everything but the pageNumber from every 32 bit number from the list.
int getPageNum(int logicAddress)
{
  int pageNumber = ((logicAddress & Address_MASK) >> 8);

  return pageNumber;
} 

//Parameters: 32 bit address from the file
//Purpose: Uses the masks previously defined in order to mask everything but the offset from every single 32 bit number from the list.
int getOffset(int logicAddress)
{
  int offset = (logicAddress & Offset_MASK);

  return offset;
}

void logicalToPhysical(int pageNumber, int offset, int logicalAddress)
{
  bool frameFound = false;

  //Checking the TLB
  int j;
  for(j = 0; j < TLBsize; j++)
    {
      if(TLBpageNums[j] == pageNumber)  //If TLBhit                                                         
	{
	  frameNum = TLBframeNums[j];   //get the frame number into the global variable to be used.      
	  printf("in log to phy\n");
	  TLB_Hits++;  //Update counter of TLB hits                                                      
	  frameFound = true;
	}
    }

  //Checking Page Table                                                                                  
  if(frameFound == false)
    {
      int z;
      for(z = 0; z < FirstPageTableNum; z++)
	{
	  if(pageTableNums[z] == pageNumber)
	    {
	      frameNum = pageTableNums[z];
	    }
	}

      if(frameFound == false)
	{
	  readBackingStore(pageNumber);
	  pageFaults++;
	  frameNum = FirstFrameNum - 1;
	}
    }

  FIFOintoTLB(pageNumber, frameNum);
  value = PM[frameNum][offset];
  printf("Frame Number: %d\n", frameNum);
  printf("Offset: %d\n", offset);
  printf("Logical Address: %d Physical Address: %d Value: %d\n\n", logicalAddress, (frameNum << 8) | offset, value);
}

void readBackingStore(int pageNumber)
{ 
  printf("in backstor\n");
  int k;
  for(k = 0; k < BYTES; k++)
    {
      PM[FirstFrameNum][k] = fromBacking[k];
      printf("in backstore for loop: %d\n", k);
    }

  pageTableNums[FirstPageTableNum] = pageNumber;
  pageTableFrames[FirstPageTableNum] = FirstFrameNum;

  FirstFrameNum++;
  FirstPageTableNum++;
}
  
void FIFOintoTLB(int pageNumber, int frameNumber)
{
  printf("in fifo\n");
  int x;
  for(x=0; x<TLBcount; x++)
    {
      if(TLBpageNums[TLBsize] == pageNumber){
	break;
      }
    }

  if(x == TLBcount)
    {
      if(TLBcount < TLBsize)
	{
	  TLBpageNums[TLBcount-1] = pageNumber;
	  TLBframeNums[TLBcount-1] = frameNumber;
	}
      else
	{
	  for(x = 0; x < TLBsize - 1 ; x++)
	    {
	      TLBpageNums[x] = TLBpageNums[x+1];
	      TLBframeNums[x] = TLBframeNums[x+1];
	    }
	  TLBpageNums[TLBcount-1] = pageNumber;
	  TLBframeNums[TLBcount-1] = frameNumber;
	}
    }
  else
    {
      int y;
      for(y = x; y < TLBcount - 1; y++)
	{
	  TLBpageNums[y] = TLBpageNums[y+1];
	  TLBframeNums[y] = TLBframeNums[y+1];
	}
      if(TLBcount < TLBsize)
	{
	  TLBpageNums[TLBcount] = pageNumber;
	  TLBframeNums[TLBcount] = frameNumber;
	}
      else
	{
	  TLBpageNums[TLBcount-1] = pageNumber;
	  TLBframeNums[TLBcount-1] = frameNumber;
	}
    }
  if(TLBcount < TLBsize)
    {
      TLBcount++;
    }
}
      
