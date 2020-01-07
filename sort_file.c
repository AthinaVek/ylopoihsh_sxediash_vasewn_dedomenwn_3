#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort_file.h"
#include "bf.h"

SR_ErrorCode SR_Init() {
  // Your code goes here

  return SR_OK;
}

SR_ErrorCode SR_CreateFile(const char *fileName) {
  // Your code goes here
  int fd;
  BF_Block *block;
  char* data;
  BF_Block_Init(&block);
  if( BF_CreateFile(fileName) != BF_OK )			return SR_ERROR;									// File 'filename' is created, opened and it's first Block is allocated
	if( BF_OpenFile(fileName,&fd) != BF_OK )	return SR_ERROR;
	if( BF_AllocateBlock(fd,block) != BF_OK )	return SR_ERROR;
  data = BF_Block_GetData(block);
  memset(data, 'x', 5);
  BF_Block_SetDirty(block);
  BF_UnpinBlock(block);
  BF_Block_Destroy(&block);
  BF_CloseFile(fd);
  return SR_OK;
}

SR_ErrorCode SR_OpenFile(const char *fileName, int *fileDesc) {
  // Your code goes here
  if ( BF_OpenFile(fileName, fileDesc)!= BF_OK)				return SR_ERROR;
  char* data;
  BF_Block *block;
  BF_Block_Init(&block);
  BF_GetBlock(*fileDesc, 0, block);
  data = BF_Block_GetData(block);
  if(!memcmp(data, "xxxxx", 5)){
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
    return SR_OK;
  }
  return SR_ERROR;
}

SR_ErrorCode SR_CloseFile(int fileDesc) {
  // Your code goes here
  BF_CloseFile(fileDesc);
  return SR_OK;
}

int counter = 0;
int recCount = 0;
SR_ErrorCode SR_InsertEntry(int fileDesc,	Record record) {
  // Your code goes here
  BF_Block *block;
  // number of records in block
  //static int counter = 0;
  int block_counter;
  char* data;
  char id[5];
  char num[5];
  //static int recCount = 1;
  BF_Block_Init(&block);
  BF_GetBlockCounter(fileDesc, &block_counter);
  // if == 1, then this is the header block
  // create one block
  if(block_counter == 1) {
    if( BF_AllocateBlock(fileDesc,block) != BF_OK )	return SR_ERROR;
    data = BF_Block_GetData(block);
    recCount++;
    // convert record.id into a string
    sprintf(id, "%d", record.id);
    sprintf(num, "%d", recCount);
    //write number of records in block
    memcpy(data, num, 5*sizeof(char));
    memcpy(data + 5*sizeof(char), id, 5*sizeof(char));
    memcpy(data + 5*sizeof(char) + 5*sizeof(char), record.name, 15*sizeof(char));
    memcpy(data + 5*sizeof(char) + 15*sizeof(char) + 5*sizeof(char), record.surname, 20*sizeof(char));
    memcpy(data + 5*sizeof(char) + 15*sizeof(char) + 20*sizeof(char) + 5*sizeof(char), record.city, 20*sizeof(char));
    counter += 60;

    BF_Block_SetDirty(block);
    BF_UnpinBlock(block);
    return SR_OK;
  }
  else {
    // chech if there is space for a new entry
    if(counter <= BF_BLOCK_SIZE - 60){
      BF_GetBlock(fileDesc, block_counter - 1, block);
      data = BF_Block_GetData(block);
      recCount++;
      sprintf(id, "%d", record.id);
      sprintf(num, "%d", recCount);
      //write number of records in block
      memcpy(data, num, 5*sizeof(char));
      memcpy(data + counter +5*sizeof(char), id, 5*sizeof(char));
      memcpy(data + counter + 5*sizeof(char) + 5*sizeof(char), record.name, 15*sizeof(char));
      memcpy(data + counter + 5*sizeof(char) + 15*sizeof(char) + 5*sizeof(char), record.surname, 20*sizeof(char));
      memcpy(data + counter + 5*sizeof(char) + 15*sizeof(char) + 20*sizeof(char) + 5*sizeof(char), record.city, 20*sizeof(char));
      counter += 60;
      BF_Block_SetDirty(block);
      BF_UnpinBlock(block);
      return SR_OK;
    }
    else{
      // else create a new block and insert the new entry
      counter = 0;
      recCount = 1;
      if( BF_AllocateBlock(fileDesc,block) != BF_OK )	return SR_ERROR;
      data = BF_Block_GetData(block);
      // convert record.id into a string
      sprintf(id, "%d", record.id);
      sprintf(num, "%d", recCount);
      //write number of records in block
      memcpy(data, num, 5*sizeof(char));
      memcpy(data + 5*sizeof(char), id, 5*sizeof(char));
      memcpy(data + 5*sizeof(char) + 5*sizeof(char), record.name, 15*sizeof(char));
      memcpy(data + 5*sizeof(char) + 15*sizeof(char) + 5*sizeof(char), record.surname, 20*sizeof(char));
      memcpy(data + 5*sizeof(char) + 15*sizeof(char) + 20*sizeof(char) + 5*sizeof(char), record.city, 20*sizeof(char));
      counter += 60;
      BF_Block_SetDirty(block);
      BF_UnpinBlock(block);
      return SR_OK;
    }
  }
  BF_Block_Destroy(&block);
}

SR_ErrorCode SR_SortedFile(
  const char* input_filename,
  const char* output_filename,
  int fieldNo,
  int bufferSize
) {
  // // Your code goes here
   if(bufferSize < 3 || bufferSize > BF_BUFFER_SIZE){
     return SR_ERROR;
   }

   int tempFd;
   int fd,blocks=1,firstRecord = 0;
   int lastRecord=0,blockCounter;
   int typeStep,size;

   SR_OpenFile(input_filename, &fd);

  BF_GetBlockCounter(fd, &blockCounter);
  BF_Block *block;
  char *data, *recCount;
  int rCounter;
  recCount = (char*)malloc(5*sizeof(char));
  //find the type of the value
  switch(fieldNo) {
    case 0:
      typeStep = 0*sizeof(char);
      size = 5*sizeof(char);
      break;
    case 1:
      typeStep = 5*sizeof(char);
      size = 15*sizeof(char);
      break;
    case 2:
      typeStep = 20*sizeof(char);
      size = 20*sizeof(char);
      break;
    case 3:
      typeStep = 40*sizeof(char);
      size = 20*sizeof(char);
      break;
    default :
      printf("Wrong Input!");
      exit(0);
  }
  BF_Block_Init(&block);
  lastRecord += bufferSize*17-1;
  while((blocks-1) < blockCounter){
    //if we are at the last block
    if((blocks+bufferSize-1) >= blockCounter - 1){
      BF_GetBlock(fd, blockCounter - 1, block);
      data = BF_Block_GetData(block);
      memcpy(recCount, data, 5*sizeof(char));
      rCounter = atoi(recCount);
      lastRecord = (blockCounter-2)*17 + (rCounter-1);
      quickSort(fd, firstRecord, lastRecord, typeStep, size);
      break;
    }
    else{
      quickSort(fd, firstRecord, lastRecord, typeStep, size);
      firstRecord = lastRecord+1;
      lastRecord += bufferSize*17;
    }
    blocks += bufferSize;
  }
  BF_UnpinBlock(block);

  int mergingBlocks = bufferSize;
  int firstBlock = 1;
  int lastBlock = firstBlock + bufferSize;

  SR_CreateFile("output_filename.db");
  SR_OpenFile("output_filename.db", &tempFd);
  BF_Block *SortedBlock;
  BF_Block_Init(&SortedBlock);
  char *sdata;

  for(int i=0; i<blockCounter; i++){
    BF_AllocateBlock(tempFd,SortedBlock);
  }
  while(mergingBlocks < blockCounter-1){
    if(lastBlock+(bufferSize - 1)+2*bufferSize <= blockCounter - 1){
      merging(mergingBlocks,size,typeStep,firstBlock, lastBlock, tempFd, fd);
      firstBlock += 2*bufferSize;
      lastBlock += 2*bufferSize;
    }
    //if we have left only one set of blocks
    else if((blockCounter-1) - (lastBlock + bufferSize - 1) <= bufferSize){
        int step = 0;
        int currentBlock = lastBlock + bufferSize;
        int counter = ((blockCounter-1) - (lastBlock + bufferSize - 1) - 1)*17 + rCounter;

        BF_GetBlock(fd, currentBlock, block);
        data = BF_Block_GetData(block);
        BF_GetBlock(tempFd, currentBlock, SortedBlock);
        sdata = BF_Block_GetData(SortedBlock);
        for(int i=0; i<counter; i++){
          if(i == 17){
            step = 0;
            currentBlock++;
            BF_UnpinBlock(SortedBlock);
            BF_UnpinBlock(block);
            BF_GetBlock(tempFd,currentBlock,SortedBlock);
            BF_GetBlock(fd,currentBlock,block);
            data = BF_Block_GetData(block);
            sdata = BF_Block_GetData(SortedBlock);
          }
          //we copy the records as they are
          memcpy(sdata + step*60*sizeof(char), data + step*60*sizeof(char), 60*sizeof(char));
          BF_Block_SetDirty(SortedBlock);
          step++;
        }
      }
      mergingBlocks *= 2;
    }
  BF_UnpinBlock(SortedBlock);
  BF_UnpinBlock(block);
  BF_Block_Destroy(&SortedBlock);
  BF_Block_Destroy(&block);
  SR_CloseFile(tempFd);
  SR_CloseFile(fd);
  return SR_OK;
}


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


SR_ErrorCode SR_PrintAllEntries(int fileDesc) {
  // Your code goes here
  int blocks;
  int records;
  BF_Block *block;
  char* data;
  int num;
  int block_counter;
  BF_Block_Init(&block);
  BF_GetBlockCounter(fileDesc, &block_counter);
  // the first is for the number of blocks
  // loop through the blocks
  for(blocks = 1; blocks < block_counter; blocks++){
    BF_GetBlock(fileDesc, blocks, block);
    data = BF_Block_GetData(block);
    // 17 is the number of records (1024/60)
    // 60 is the size of its record
    // loop through the records
    for(records = 0; records < 17; records++){
      // if(records == 0){
      //   printf("TOCounter:");
      //   printf("%s\n",data);
      // }
      printf("%s ",data + records*sizeof(Record)+5*sizeof(char));
      printf("%s ",data + 5*sizeof(char) + records*sizeof(Record)+5*sizeof(char));
      printf("%s ",data + 5*sizeof(char) + 15*sizeof(char) + records*sizeof(Record)+5*sizeof(char));
      printf("%s ",data + 5*sizeof(char) + 15*sizeof(char) + 20*sizeof(char) + records*sizeof(Record)+5*sizeof(char));
      printf("\n");
    }
    BF_UnpinBlock(block);
  }
  BF_Block_Destroy(&block);

  return SR_OK;
}
