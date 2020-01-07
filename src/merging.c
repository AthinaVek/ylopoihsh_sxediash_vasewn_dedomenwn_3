#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort_file.h"
#include "bf.h"
#include "bf.h"
#include "merging.h"


void merging(int mergingBlocks,int size,int typeStep, int firstBlock,int lastBlock, int tempFd,int fd){
  int blockRecords = BF_BLOCK_SIZE / 60;
  int rCounter,fBlock1,fBlock2,currentSortedBlock = 1;

  BF_Block *Block1,*Block2,*SortedBlock;
  fBlock1 = firstBlock;
  fBlock2 = lastBlock;

  int first1 = fBlock1;
  int last2 = fBlock2;
  char *fdata1,*fdata2,*sorted,*val1,*val2,*recCount;
  int currec1=0,currec2=0,recs=0;
  val1 = (char*)malloc(size);
  val2 = (char*)malloc(size);
  recCount = (char*)malloc(5*sizeof(char));
  int blockCounter;
  BF_GetBlockCounter(fd, &blockCounter);

  BF_Block_Init(&Block1);
  BF_Block_Init(&Block2);
  BF_Block_Init(&SortedBlock);

  BF_GetBlock(fd,fBlock1,Block1);
  BF_GetBlock(fd,fBlock2,Block2);
  BF_GetBlock(tempFd,currentSortedBlock,SortedBlock);

  fdata1 = BF_Block_GetData(Block1);
  fdata2 = BF_Block_GetData(Block2);
  sorted = BF_Block_GetData(SortedBlock);

  memcpy(val1, fdata1 + typeStep+5*sizeof(char), size);
  memcpy(val2, fdata2 + typeStep+5*sizeof(char), size);

  while(recs <= blockRecords){
    if(recs == blockRecords){
      BF_UnpinBlock(SortedBlock);
      currentSortedBlock++;
      BF_GetBlock(tempFd,currentSortedBlock,SortedBlock);
      sorted = BF_Block_GetData(SortedBlock);
      recs=0;
    }

    if(strcmp(val1,val2) <= 0){
      //then store val1, and change val1 to the next value
      memcpy(sorted+recs*60*sizeof(char), fdata1+currec1*60*sizeof(char)+5*sizeof(char), 60*sizeof(char));
      currec1++;
      BF_Block_SetDirty(SortedBlock);
      memcpy(recCount, fdata1, 5*sizeof(char));
      rCounter = atoi(recCount);
      //if we are at the last record
      if((currec1 == blockRecords ) || ((fBlock1 == blockCounter - 1) && currec1 == rCounter)){
        if(fBlock1 == first1 + mergingBlocks -1 || fBlock1 == blockCounter - 1){
          for(int i=currec2; i<17; i++){
            if(recs == blockRecords){
              BF_UnpinBlock(SortedBlock);
              currentSortedBlock++;
              BF_GetBlock(tempFd,currentSortedBlock,SortedBlock);
              sorted = BF_Block_GetData(SortedBlock);
              recs=0;
            }
            memcpy(sorted+recs*60*sizeof(char), fdata2+i*60*sizeof(char)+5*sizeof(char), 60*sizeof(char));
            BF_Block_SetDirty(SortedBlock);
            recs++;
          }
          break;
        }
        fBlock1++;
        BF_UnpinBlock(Block1);
        BF_GetBlock(fd, fBlock1, Block1);
        fdata1 = BF_Block_GetData(Block1);
        currec1 = 0;
      }
      memcpy(val1,fdata1 + currec1*60*sizeof(char) +typeStep+5*sizeof(char),size);
    }
    else{
      //store val2 and change val2 to the next value of 2nd block
      memcpy(sorted+recs*60*sizeof(char), fdata2+currec2*60*sizeof(char)+5*sizeof(char), 60*sizeof(char));
      currec2++;
      BF_Block_SetDirty(SortedBlock);
      memcpy(recCount, fdata2, 5*sizeof(char));
      rCounter = atoi(recCount);
      if((currec2 == blockRecords ) || ((fBlock2 == blockCounter - 1) && currec2 == rCounter)){
        if(fBlock2 == last2 + mergingBlocks -1 || fBlock2 == blockCounter - 1){
          for(int i=currec1; i<17; i++){
            if(recs == blockRecords){
              BF_UnpinBlock(SortedBlock);
              currentSortedBlock++;
              BF_GetBlock(tempFd,currentSortedBlock,SortedBlock);
              sorted = BF_Block_GetData(SortedBlock);
              recs=0;
            }
            memcpy(sorted+recs*60*sizeof(char), fdata1+i*60*sizeof(char)+5*sizeof(char), 60*sizeof(char));
            BF_Block_SetDirty(SortedBlock);
            recs++;
          }
          break;
        }
        fBlock2++;
        BF_UnpinBlock(Block2);
        BF_GetBlock(fd, fBlock2, Block2);
        fdata2 = BF_Block_GetData(Block2);
        currec2 = 0;
      }
      memcpy(val2,fdata2 + currec2*60*sizeof(char) +typeStep+5*sizeof(char),size);
    }
    recs++;
  }
BF_Block *block;
char *data;
BF_Block_Init(&block);
// for(int blocks=1; blocks < 6; blocks++){
//   BF_GetBlock(tempFd, blocks, block);
//   data = BF_Block_GetData(block);
//   // 17 is the number of records (1024/60)
//   // 60 is the size of its record
//   // loop through the records
//   for(int records = 0; records < 17; records++){
//     // if(records == 0){
//     //   printf("TOCounter:");
//     //   printf("%s\n",data);
//     // }
//     printf("%s ",data + records*sizeof(Record));
//     printf("%s ",data + 5*sizeof(char) + records*sizeof(Record));
//     printf("%s ",data + 5*sizeof(char) + 15*sizeof(char) + records*sizeof(Record));
//     printf("%s ",data + 5*sizeof(char) + 15*sizeof(char) + 20*sizeof(char) + records*sizeof(Record));
//     printf("\n");
//   }
//   BF_UnpinBlock(block);
// }
  BF_Block_Destroy(&block);


  BF_UnpinBlock(Block1);
  BF_UnpinBlock(Block2);
  BF_UnpinBlock(SortedBlock);
  BF_Block_Destroy(&Block1);
  BF_Block_Destroy(&Block2);
  BF_Block_Destroy(&SortedBlock);
  free(val1);
  free(val2);
  free(recCount);
}

