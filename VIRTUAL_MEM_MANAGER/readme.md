
/*. 
  Purpose of the program:  
  This program read a .txt file containing 1000 logical addresses.   
  From each of the addresses we get it's Page Number and Offset.  
  The TLB is used in order to translate from logical to physical.  
  For doing so:  
  
  1st: The TLB is checked.   
       If the page number is in the TLB we have a TLB HIT. Then: Frame is obtained from the TLB. 
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
 */. 
