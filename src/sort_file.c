#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort_file.h"
#include "bf.h"
#include "merging.h"
#include "quicksort.h"

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
