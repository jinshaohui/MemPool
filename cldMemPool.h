/*
  *
  */

#ifndef _CLD_MEMPOOL_H
#define _CLD_MEMPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define CLD_MEMPOOL_ALIGNMENT   8    /*Byte alignment length*/
#define CLD_MEMBLOCK_HEAD       (sizeof(struct _cldMemBlock) - sizeof(char))/*Memory block head*/
#define CLD_MEMPOOL_INITNUM    100  /*Init memory allocation unit number*/
#define CLD_MEMPOOL_GROWNUM    150  /*Grow memory allocation unit number*/

/*Memory blocks, each memory block management a large block of memory, 
  *including many allocation unit
  */
typedef struct _cldMemBlock{
    int nSize;                  /*The size of the memory block(), in bytes*/
    int nFree;                  /*Free allocation unit of Memory blocks  */
    int nFirst;                 /*Currently available unit serial number, starting  0*/
    struct _cldMemBlock *next;  /*Points to the next memory block*/
    char aData[1];              /*Mark allocation unit start position, */
}cldMemBlock;

/*Memory pool*/
typedef struct _cldMemPool{
    int nInitNum;               /*Init memory block num*/
    int nUnitSize;              /*allocation unit of Memory blocks size*/
    int nGrowNum;               /*Grow memory block num*/
    int nMemBlockNum;           /*Current Memory block num*/
    cldMemBlock *head;
}cldMemPool;


/*function insterface*/

#define CLD_MEMPOOL_INIT(type)  (_cldMemPool_init(sizeof(struct type),CLD_MEMPOOL_INITNUM,CLD_MEMPOOL_GROWNUM))
cldMemPool * _cldMemPool_init(int nUnitSize,int nInitNum,int nGrowNum);
void _cldMemPool_exit(cldMemPool *pMemPool);
void * _cldMem_malloc(cldMemPool * pMemPool);
int cldMem_free(cldMemPool * pMemPool,void * pFree);
void print_MemoryBlockInfoByPointer(cldMemPool *pMemPool, void *ptr);
void Print_MemoryPool_INFO(cldMemPool *pMemPool);



#endif
