#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort_file.h"
#include "bf.h"
#include "quicksort.h"


void quickSort(int fd, int l, int r, int typeStep, int size)
{
   if( l < r )
   {
      int j;
      j = partition(fd, l, r, typeStep, size);
      quickSort(fd, l, j-1, typeStep, size);
      quickSort(fd, j+1, r, typeStep, size);
   }
}


int partition(int fd, int l, int r, int typeStep, int size){
  //17 is the number of records in every block
  int lBlock = r/17 +1;
  int lRecord=r%17;
  int curRec,curNewRec,curBlock,curNewBlock;
  BF_Block *firstBlock,*lastBlock, *newBlock;
  int j,i = l-1;
  char *fdata,*ldata,*newData,pivot[20],value[20],t[60];

  BF_Block_Init(&newBlock);
  BF_Block_Init(&firstBlock);
  BF_Block_Init(&lastBlock);

  BF_GetBlock(fd,lBlock,lastBlock);
  ldata = BF_Block_GetData(lastBlock);
  //we take the last record as pivot
  memcpy(pivot, ldata + lRecord*60*sizeof(char)+5*sizeof(char)+typeStep, size);

  for(j = l; j <= r - 1;j++){
    curBlock = j/17+1;
    curRec = j%17;
    BF_GetBlock(fd, curBlock, firstBlock);
    fdata = BF_Block_GetData(firstBlock);
    memcpy(value, fdata+curRec*60+typeStep+5*sizeof(char), size);
    //compare the value and the pivot and swap them
    if(strcmp(value, pivot) <= 0){
      i++;
      curNewBlock = i/17+1;
      curNewRec = i%17;
      BF_GetBlock(fd, curNewBlock, newBlock);
      newData = BF_Block_GetData(newBlock);
      memcpy(t, fdata+curRec*60*sizeof(char)+5*sizeof(char), 60*sizeof(char));
      memcpy(fdata+curRec*60*sizeof(char)+5*sizeof(char), newData+curNewRec*60*sizeof(char)+5*sizeof(char), 60*sizeof(char));
      memcpy(newData+curNewRec*60*sizeof(char)+5*sizeof(char), t, 60*sizeof(char));
      BF_Block_SetDirty(newBlock);
      BF_Block_SetDirty(firstBlock);
      BF_UnpinBlock(firstBlock);
      BF_UnpinBlock(newBlock);
    }
  }
  //
  curNewBlock = (i+1)/17+1;
  curNewRec = (i+1)%17;
  BF_GetBlock(fd, curNewBlock, firstBlock);
  fdata = BF_Block_GetData(firstBlock);

  memcpy(t, fdata+curNewRec*60*sizeof(char)+5*sizeof(char), 60*sizeof(char));
  memcpy(fdata+curNewRec*60*sizeof(char)+5*sizeof(char), ldata+lRecord*60*sizeof(char)+5*sizeof(char), 60*sizeof(char));
  memcpy(ldata+lRecord*60*sizeof(char)+5*sizeof(char), t, 60*sizeof(char));

  BF_Block_SetDirty(firstBlock);
  BF_Block_SetDirty(lastBlock);

  BF_UnpinBlock(firstBlock);
  BF_UnpinBlock(lastBlock);
  BF_Block_Destroy(&firstBlock);
  BF_Block_Destroy(&lastBlock);
  BF_Block_Destroy(&newBlock);
  return (i + 1);
}