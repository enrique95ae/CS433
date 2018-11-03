//#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
//#include <assert.h>
//#include <unistd.h>
#include <string.h>

// Data structure that represents tlb
struct tlbData {
  // virutal page from tlb
  unsigned int page;
  // physical frame from tlb
  unsigned int frame;
};

int main(int argc, char *argv[]) {
  // Addresses file
  FILE *addresses = fopen(argv[1], "r");
  FILE *backingStore = fopen("BACKING_STORE.bin", "rb");
  FILE *output = fopen("output.txt", "w");

  // total number of addresses read
  int addressCount = 0;
  // total page faults
  int pageFaultCount = 0;
  // count / total
  float pageFaultRate = 0;
  // total tlb hits
  int tlbHitCount = 0;
  // count / total
  float tlbHitRate = 0;

  // 1111111100000000
  unsigned pageNumberMask = 65280;
  // 11111111
  unsigned offsetMask = 255;

  // logical / virtual address
  int logicalAddress = 0;
  // page number found from bitmask
  int pageNumber = 0;
  // offset found from bitmask
  int offset = 0;

  // physical adddress in mem
  int physicalAddress = 0;
  // physical frame
  int frame = 0;
  // value stored at frame
  int value = 0;
  // what we found in the tlb
  int hit = 0;
  // current index of the tlb buf
  int tlbIndex = 0;
  // size of the tlb buf
  int tlbSize = 0;

  // page table
  int pageTable[256];
  memset(pageTable, -1, 256*sizeof(int));

  // buf of tlb data structures
  struct tlbData tlb[16];
  memset(pageTable, -1, 16*sizeof(char));
  
  // physical memory
  int physicalMemory[65536];

  // general purpose buffer, store from reads
  char buf[256];

  // index variable
  int index;

  while(fscanf(addresses, "%d", &logicalAddress) == 1) {
    addressCount++;
    // Get page number and offset from the integer
    pageNumber = logicalAddress & pageNumberMask;
    pageNumber = pageNumber >> 8;
    offset = logicalAddress & offsetMask;
    hit = -1;
    // Search through tlb for a match
    for(index = 0; index < tlbSize; index++) {
      if(tlb[index].page == pageNumber) {
        hit = tlb[index].frame;
        physicalAddress = hit*256 + offset;
      }
    }
    // if we found a match increment if not look at the table
    if(!(hit == -1)) {
      tlbHitCount++;
    } else if(pageTable[pageNumber] == -1) {
      // read from backing store
      fseek(backingStore, pageNumber*256, SEEK_SET);
      fread(buf, sizeof(char), 256, backingStore);
      pageTable[pageNumber] = frame;
      for(index = 0; index < 256; index++) {
        physicalMemory[frame*256 + index] = buf[index];
      }
      pageFaultCount++;
      frame++;

      // page fault, so add to tlb
      if(tlbSize == 16)
        tlbSize--;

      // rotate forward in the buffer, FIFO
      for(tlbIndex = tlbSize; tlbIndex > 0; tlbIndex--) {
        tlb[tlbIndex].page = tlb[tlbIndex-1].page;
        tlb[tlbIndex].frame = tlb[tlbIndex-1].frame;
      }

      if(tlbSize <=15)
        tlbSize++;

      tlb[0].page = pageNumber;
      tlb[0].frame = pageTable[pageNumber];
      physicalAddress = pageTable[pageNumber]*256 + offset;
    } else {
      physicalAddress = pageTable[pageNumber]*256 + offset;
    }
    value = physicalMemory[physicalAddress];
    fprintf(output, "Virtual address: %d Physical Address: %d Value: %d \n", logicalAddress, physicalAddress, value);
  }

  pageFaultRate = pageFaultCount*1.0f / addressCount;
  tlbHitRate = tlbHitCount*1.0f / addressCount;

  fclose(addresses);
  fclose(backingStore);
  fprintf(output, "Number of Translated Addresses = %d\n", addressCount);
  fprintf(output, "Page Faults = %d\n", pageFaultCount);
  fprintf(output, "Page Fault Rate = %f\n", pageFaultRate);
  fprintf(output, "TLB Hits = %d\n", tlbHitCount);
  fprintf(output, "TLB Hit Rate %f\n", tlbHitRate);
  fclose(output);
  return 0;
}
