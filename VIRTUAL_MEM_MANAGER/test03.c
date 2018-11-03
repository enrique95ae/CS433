//
//  main.c
//  VirtualMemoryManagerV2
//
//  Created by Enrique Alonso Esposito.
//  For CS433 - Operative Systems
//  Professor Springer.
//  04/13/2018
//
/*
  Purpose of the program:
  This program read a .txt file containing 1000 logical addresses. 
  From each of the addresses we get it's Page Number and Offset.

  The TLB is used in order to translate from logical to physical.

  For doing so:
  
  1st: The TLB is checked. 
       If the page number is in the TLB we have a TLB HIT. Then: Frame is obtained from the TLB
       If the page number is not in the TLB we have a TLB MISS. Then the Page Table is checked.
  2nd: The Page Table is checked.
       If the page number is in the page table we extrat the frame from the page table.
       If the page number is not in the page table we have a Page Fault. Then: we read the BACKING STORE file.
  3rd: If Page fault, we load a page into the physical memory. Page Table and TLB are then updated.
  4th: Since the TLB has a max of 16 entries we use a FIFO policy in order to update it. 

  Outputs:
    -x1000 Logical Address to be translated.
    -x1000 corresponding Physical Addresses.
    -x1000 Value stored in each address.
    -Page Fault rate.
    -TLB hit rate.
 */


/* KNOWN ERRORS:
--------------------------------------
Segmentation Fault inside the readBackingStore function for loop.   ----->   SOLVED   04/16/2018
Masks do not isolate the desired 16 and 8 rightmost bits            ----->   SOLVED   04/16/2018

logical addresses are not read properly. 3 digits from 1st address are read
then the remaining 2 digits are considered a new address and so on... ------>  SOLVED 18/18/2018    FINALLY!!!!!

error when reading from backing store is needed. Addresses after the first time the file is attempted to be read become 0 until the end of execution. --->   SOLVED    18/18/2018

Pagefaults are not correctly incremented.    ---->  SOLVED   18/18/2018
TLB_Hits are not correctly incremented.      ---->  SOLVED   18/18/2018
Therefor both Page Fault Rate and TLB fault rate give incorrect results.     ---->  SOLVED 18/18/2018

printf output format needs adjusting.    ----->  ADJUSTED BUT FURTHER ADJUSTING WOULD BE BETTER.
--------------------------------------
*/


typedef enum { false, true } bool;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER  10                //CHARACTERS READ FROM EACH LINE (ADDRESSES)
#define BYTES  256               //NUM OF BYTES READING (backingStore)
#define NUM_FRAMES  256          //NUMBER OF FRAMES
#define FRAME_SIZE  256          //SIZE OF EACH FRAME
#define PAGE_SIZE  256           //SIZE OF EACH PAGE
#define PM_SIZE  65536           //PHYSICAL MEMORY SIZE
#define TLB_SIZE  16             //ENTRIES IN THE TLB
#define LOGICAL_ADDRESSES_TO_TRANSLATE  1000    //NUMBER OF ADDRESSES TO BE TRANSLATED.

unsigned pageNumberMask = 65280;
unsigned offsetMask = 255;

//ARRAYS:
int logicalAddressesArray[LOGICAL_ADDRESSES_TO_TRANSLATE];
int pageTableNumbersInTLB[NUM_FRAMES];
int frameNumbersInTLB[NUM_FRAMES];
int numbersInPageTable[PAGE_SIZE];
int frameNumbersInPageTable[NUM_FRAMES];
int PMem[256][256];



char logicalAddressBuff[BUFFER]; //holds a string of characters reaad from file. later will be converted to int.


//FUNTIONS:
int getPageNum(int);            //MASKING OUT PAGE NUMBER FROM THE FRAME
int getOffset(int);             //MASKING OUT THE OFFSET FROM THE FRAME
int checkingTLB(int);           //Checking if pagenumber is alredy in the TLB
int checkingPageTable(int);     //Checking if pagenumber is alredy in the page Table
void readBackingStore(int);     //If TLB miss and Page Table miss ---> read from BACKING_STORE.bin
void FIFOintoTLB(int, int);     //Inserting data into the TLB following a FIFO algorithm.


//Input Files:
FILE *addressesFile;            //1000 logical addresses to be translated.
FILE *backingStoreFile;         //Simulated as hard drive.

//VARIABLES:
int TLB_hits = 0;
int emptyPageTableSpot = 0;
int emptyFrameSpot = 0;
int pageFaults = 0;
int translated = 0;
int TLBcount = 0; //entries in TLB

signed char tempBuff[BYTES];

int main(int argc, const char * argv[]) {
    
  //Number of arguments check.
  if(argc != 2){
    printf("ERROR: number of arguments doesn't match. \n");
    return 1;
  }

  //OPENING INPUT FILES
  backingStoreFile = fopen("BACKING_STORE.bin", "rb"); //opens the file read binary mode
  addressesFile = fopen(argv[1], "r"); //Opens the file read mode
    
  int temp;
  int pageNumber = 0;
  int frameNumber = 0;
  int offset = 0;
  int value = 0;
  
  double PageFaultRate = 0.0;
  double TLBhitRate = 0.0;

  //Loop for reading all the addresses from file into an array.
  int i;
  for(i=0; i<LOGICAL_ADDRESSES_TO_TRANSLATE; i++)
    {
      fgets(logicalAddressBuff, sizeof(logicalAddressBuff), addressesFile);
      temp = atoi(logicalAddressBuff);
      logicalAddressesArray[i] = temp;
      //TESTING OUTPUT:
      //printf("Address number %d : %d \n", i , logicalAddressesArray[i]);
    }
    
  //Main loop to translate addresses.
  int j;
  for(j=0; j<1000; j++)
    {
      pageNumber = getPageNum(logicalAddressesArray[j]);
      frameNumber = checkingTLB(pageNumber);
      offset = getOffset(logicalAddressesArray[j]);
        
      FIFOintoTLB(pageNumber, frameNumber);
      value = PMem[frameNumber][offset];
        
      printf("# %4d :", j+1);
      printf("   Logical Address: %5d   ,   Physical Address:  %5d   ,   Value:  %5d  \n" , logicalAddressesArray[j], (frameNumber << 8) | offset, value);
      translated++;
    }

  //Calculations
  PageFaultRate = (double) pageFaults / 1000 * 100;
  TLBhitRate = (double) TLB_hits / 1000 * 100;

  //Outputs
  printf("RESULTS: \n -----------------  \n");
  printf("Page fault rate: %.2f%\n", PageFaultRate);
  printf("TLB hit rate: %.2f%\n", TLBhitRate);
}

//Parameters: page number
//Purpose: Checking the contents of the TLB. If the page number we are looking for is in the TLB = TLB HIT!
//          Then we get the frame number from the TLB. Otherwise we call page table checking function.
//Returns: frameNumber.
int checkingTLB(int pageNumber)
{
    bool found = false;
    
    int frameNumber = 0;
    
    
    //Checking the TLB
    int i;
    for(i=0; i<TLB_SIZE; i++)
    {
        if(pageTableNumbersInTLB[i] == pageNumber) //TLB HIT
        {
	  frameNumber = frameNumbersInTLB[i]; //Getting the frame from the array.
	  found = true;  //Switching found to true.
	  TLB_hits++;    //Incrementing counter of TLBhits
        }
	found = false;
    }
    
    //If not found 
    if(found == false) //If TLB MISS
    {
      frameNumber = checkingPageTable(pageNumber);  //Calling the checking page table function which will return the frame
    }
    return frameNumber; //Returning the frame.
}

//Parameters: pageNumber
//Purpose: after a TLB miss this function is called and we look in the page table. If in the page table we set a temporary variable to the frame's value and then return it. If not we call the read from Backing Store funtcion.
//Returns: frameNumber.
int checkingPageTable(int pageNumber)
{
    bool found = false;
    int frameNumber = 0;
    int i;
    for(i=0; i < emptyPageTableSpot; i++)
    {
        if(numbersInPageTable[i] == pageNumber)
        {
            frameNumber = frameNumbersInPageTable[i];
            found = true;
        }
    }
    
    if(found == false)
    {
        readBackingStore(pageNumber);
        pageFaults++;
        frameNumber = emptyFrameSpot -1;
    }
    
    return frameNumber;
}


//Paremeters: pageNumber
//Purpose: after a TLB miss and Page Table miss. this function is called. We bring a 256 byte page from the bin file and store it in the first available slot in the Physical Memory array. First empty slots are then updated to the next one.
//Returns: none
void readBackingStore(int pageNumber)
{   
    if(fread(tempBuff, sizeof(signed char), BYTES, backingStoreFile) == 0)
    {
      fprintf(stderr,"ERROR reading BS.bin\n");
    }  

    int i;
    for(i=0; i<BYTES; i++)
    {
        PMem[emptyFrameSpot][i] = logicalAddressBuff[i];
    }
    
    numbersInPageTable[emptyPageTableSpot] = pageNumber;
    frameNumbersInPageTable[emptyPageTableSpot] = emptyFrameSpot;
    
    emptyPageTableSpot++;
    emptyFrameSpot++;
}


//Parameters: pageNumber and frameNumber
//Purpose: To use a First In First Out algorithm in order to insert data into the TLB. 
void FIFOintoTLB(int pageNumber, int frameNumber)
{
   
    int i;
    for(i=0; i<TLBcount; i++)
    {
      if(pageTableNumbersInTLB[i] == pageNumber) //Page is already in the TLB. Then we break.
        {
            break;
        }
    }
    
    
    if(i == TLBcount)
    {
      if(TLBcount < TLB_SIZE)//If there is still space in the TLB
        {
	  pageTableNumbersInTLB[TLBcount] = pageNumber; //Insert page
	  frameNumbersInTLB[TLBcount] = frameNumber;  //Insert frame
        }
      else //If no space
        {
	  for(i=0; i<TLB_SIZE-1; i++)//Move everything. First element gets overwritten. Last element becomes empty and page and frame are inserted there.
            {
                pageTableNumbersInTLB[i] = pageTableNumbersInTLB[i+1];
                frameNumbersInTLB[i] = frameNumbersInTLB[i+1];
            }
	  //Insert in the newly created slot.
            pageTableNumbersInTLB[TLBcount-1] = pageNumber;
            frameNumbersInTLB[TLBcount-1] = frameNumber;
        }
    }
    else 
    {
      for(i=i; i<TLBcount-1; i++) //from begginig to previous to last element
        {
	  pageTableNumbersInTLB[i] = pageTableNumbersInTLB[i+1]; //Moving everything over.
	  frameNumbersInTLB[i] = frameNumbersInTLB[i+1];//moving everything over
        }
      if(TLBcount < TLB_SIZE)//if there is still space in the TLB
        {
	  pageTableNumbersInTLB[TLBcount] = pageNumber; //Insert in the desired slot
	  frameNumbersInTLB[TLBcount] = frameNumber;//Insert in the desired slot
        }
      else //if  not
	  {
            pageTableNumbersInTLB[TLBcount-1] = pageNumber; 
            frameNumbersInTLB[TLBcount-1] = frameNumber;
	  }
    }
    if(TLBcount < TLB_SIZE)
      {
        TLBcount++;
      }
}

//Parameters: logical address
//Purpose: To apply the mask and get the page number
int getPageNum(int logicalAddress)
{
  int pageNumber = logicalAddress & pageNumberMask;
  pageNumber = pageNumber >> 8;
  return pageNumber;
}

//Parameters: logical address                                                                                                                        
//Purpose: To apply the mask and get the offset
int getOffset(int logicalAddress)
{
  int offset = logicalAddress & offsetMask;
  return offset;
}
