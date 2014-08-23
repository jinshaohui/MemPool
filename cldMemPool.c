/*
  *
  */


#include "cldMemPool.h"


/*_cldMemBlock_create
  *       Create a memory block
  *param:
  *       nUnitSize : allocation unit of Memory blocks size
  *       num         : allocation unit of Memory blocks num
  *return:
  *       NULL:  failed
  *       other: sucessful
  */
static cldMemBlock * _cldMemBlock_create(int nUnitSize,int num)
{
    cldMemBlock *pMemBlock = NULL;
    int i = 0;
    char *offset = NULL;

    pMemBlock = (cldMemBlock *)malloc(CLD_MEMBLOCK_HEAD + num * nUnitSize);
    if(!pMemBlock){
        return NULL;
    }

    pMemBlock->next = NULL;
    pMemBlock->nFirst = 1;
    pMemBlock->nFree = num - 1;
    pMemBlock->nSize = num*nUnitSize;

    /*Each allocation unit next allocation unit serial number written on the 
        *current unit in the first four bytes
        */
    offset = pMemBlock->aData;
    for(i = 1;i < num; i++){
        *((int *)offset) = i;
        offset += nUnitSize;
    }
   // printf("###%s,create OK!\n",__func__);
    return pMemBlock;
    
}

/*_cldMemPool_init
  *       Create a memory Pool and init it
  *param:
  *       nUnitSize : allocation unit of Memory blocks size
  *       nInitNum : init allocation unit of Memory blocks num
  *       nGrowNum:
  *return:
  *       NULL:  failed,  param error or malloc failed;
  *       other: sucessful,return cldMemPool pointer;
  */
cldMemPool * _cldMemPool_init(int nUnitSize,int nInitNum,int nGrowNum)
{
    cldMemPool *pMemPool = NULL;

    if(!nInitNum || !nGrowNum){
        printf("###%s, init failed\n",__func__);
        return NULL;
    }

    pMemPool = (cldMemPool *) malloc(sizeof(cldMemPool));
    if(!pMemPool){
        return NULL;
    }
    
    memset(pMemPool,0,sizeof(cldMemPool));
    /*Guarantee deposit 4 bytes (memory allocation unit number)*/
    if(nUnitSize < sizeof(int)){
        pMemPool->nUnitSize = sizeof(int);
    }
    /*Deposit guarantee 8 bytes alignment*/
    pMemPool->nUnitSize = (nUnitSize + (CLD_MEMPOOL_ALIGNMENT - 1))&~(CLD_MEMPOOL_ALIGNMENT -1);
    pMemPool->nInitNum  = nInitNum;
    pMemPool->nGrowNum  = nGrowNum;
    pMemPool->nMemBlockNum = 0;
    pMemPool->head = NULL;
    printf("###%s,init OK!\n",__func__);
    return pMemPool;
}

/*_cldMemPool_exit
  *       free memory block add memory pool
  *param:
  *       pMemPool : memory Pool pointer
  */
void _cldMemPool_exit(cldMemPool *pMemPool)
{
    cldMemBlock *pMemBlock = NULL;
    cldMemBlock *ptemp = NULL;

    if(!pMemPool){
        return;
    }

    pMemBlock = pMemPool->head;
    while(pMemBlock){
        ptemp = pMemBlock;
        pMemBlock = pMemBlock->next;
        free(ptemp);
    }

    free(pMemPool);
    pMemPool = NULL;

    printf("###%s,exit OK!\n",__func__);
}

/*_cldMem_malloc
  *       Traverse the memory pool memory blocks, find unused memory unit.If the memory 
  *       unit is not found, malloc a new  memory block, and added to the 
  *       memory pool memory block list of the first node
  *param:
  *       pMemPool : memory Pool pointer
  *return:
  *       NULL: failed.
  *       other:sucessful, return a pointer to the malloc memory 
  */

void * _cldMem_malloc(cldMemPool *pMemPool)
{
    cldMemBlock *pMemBlock = NULL;
    
    if(!pMemPool){
        printf("###%s() line[%d] parameter is NULL\n",__func__,__LINE__);
        return NULL;
    }

    pMemBlock = pMemPool->head;

    if(!pMemBlock){/*the head is NULL,first create Memory block*/
        pMemBlock = _cldMemBlock_create(pMemPool->nUnitSize,pMemPool->nInitNum);
        if(!pMemBlock){
            printf("###%s() line[%d] _cldMemBlock_create failed!\n",__func__,__LINE__);
            return NULL;
        }
        pMemPool->head = pMemBlock;
        pMemPool->nMemBlockNum ++;
        return pMemBlock->aData;
    }

    /*find the  free allocation unit */
    while(pMemBlock && pMemBlock->nFree == 0){
        pMemBlock = pMemBlock->next;
    }

    if(pMemBlock){/*have allocation unit*/
        /*find the free allocation unit address*/
        char *pMemtep = pMemBlock->aData + (pMemBlock->nFirst)*(pMemPool->nUnitSize);
        pMemBlock->nFirst = *((int *)pMemtep);
        pMemBlock->nFree--;
        return pMemtep;
    } else {
        /*Not find free memory Block,alloc the new add add to the list head*/

        pMemBlock = _cldMemBlock_create(pMemPool->nUnitSize,pMemPool->nGrowNum);
        if(!pMemBlock){
            printf("###%s() line[%d] _cldMemBlock_create failed!\n",__func__,__LINE__);
            return NULL;
        }

        /*add to list head*/
        pMemBlock->next = pMemPool->head;
        pMemPool->head = pMemBlock;
        pMemPool->nMemBlockNum ++;
        return pMemBlock->aData;
    }

    
}

/*cldMem_free
  *       Recycling memory allocation unit.Recycling, if the current memory 
  *       block is free state (memory units are not allocated) and number of
  *       memory blocks is greater than 1, free memory brock.If memory pool
  *       first node has full distribution of the current memory blocks to the first node
  *param:
  *       pMemPool : memory Pool pointer
  *return:
  *       -1: failed.
  *       0:sucessful, return a pointer to the malloc memory 
  */

int cldMem_free(cldMemPool * pMemPool,void *pFree)
{
    cldMemBlock *pMemBlock = NULL;
    cldMemBlock *ptemp = NULL;

    if(!pMemPool || !pFree){
        printf("###%s() line[%d] pMemPool or pFree is free\n",__func__,__LINE__);
        return -1;
    }

    pMemBlock = pMemPool->head;

    while(pMemBlock && (pMemBlock->aData > pFree || pFree >= (pMemBlock->aData +  pMemBlock->nSize))){
        ptemp = pMemBlock;
        pMemBlock = pMemBlock->next;
    }

    if(!pMemBlock){/*something error*/
        printf("###%s() line[%d] pfree[%p] is not in the cldMemPool!\n",__func__,__LINE__,pFree);
        return -1;
    }

    pMemBlock->nFree++;
    *((int *)pFree) = pMemBlock->nFirst;
    pMemBlock->nFirst = ((char *)pFree - pMemBlock->aData)/(pMemPool->nUnitSize);

    /*Whether to release the memory block*/
    if(pMemBlock->nSize == pMemBlock->nFree * pMemPool->nUnitSize){
        if(pMemPool->nMemBlockNum > 1 && pMemPool->head->nFree){/*Ensure that at least a memory block*/
            if(pMemBlock == pMemPool->head){
                pMemPool->head = pMemBlock->next;
            } else {
                ptemp->next = pMemBlock->next;
            }
            free(pMemBlock);
            pMemPool->nMemBlockNum--;
        }
    } else if(pMemPool->head->nFree == 0){
        /*Memory block move to the first node list*/
        ptemp->next = pMemBlock->next;
        pMemBlock->next = pMemPool->head;
        pMemPool->head = pMemBlock;
    }

    /*Recycling is successful, the pointer to NULL*/
    pFree = NULL;
    return 0;

}

void print_MemoryBlockInfoByPointer(cldMemPool *pMemPool, void *ptr)
{
    cldMemBlock *pMemBlock = NULL;
    
    if(!pMemPool){
        printf("###%s() line[%d] pMemPool is NULL!\n",__func__,__LINE__);
        return;
    }

    if(!ptr){
        printf("###%s() line[%d] ptr is NULL!\n",__func__,__LINE__);
        return;
    }

    pMemBlock = pMemPool->head;

    while(pMemBlock && (pMemBlock->aData > ptr || ptr >= (pMemBlock->aData +  pMemBlock->nSize))){
        pMemBlock = pMemBlock->next;
    }

    if(!pMemBlock){/*something error*/
        printf("###%s() line[%d] pfree[%p] is not in the cldMemPool!\n",__func__,__LINE__,ptr);
        return;
    } else {
         printf("###%s() line[%d] pfree[%p] is in pMemBlock[%p]!\n",__func__,__LINE__,ptr,pMemBlock);
         printf("**********pMemBlock info**************\n");
         printf("*\tnSize = %d\n*\tnFree = %d\n*\tnFirst = %d\n*\taData = %p\n",pMemBlock->nSize,pMemBlock->nFree,pMemBlock->nFirst,pMemBlock->aData);
         printf("**************************************\n");
    }
}

void Print_MemoryPool_INFO(cldMemPool *pMemPool)
{
    cldMemBlock *pMemBlock = NULL;
    int i = 1;

    if(!pMemPool){
        return;
    }

    printf("************PMemPool Info*****************\n");
    printf("*\tnUnitSize = %d\n*\tnMemBlockNum = %d\n*\tnInitNum = %d\n*\tnGrowNum = %d\n",pMemPool->nUnitSize,pMemPool->nMemBlockNum,pMemPool->nInitNum,pMemPool->nGrowNum);
    printf("******************************************\n");

    pMemBlock = pMemPool->head;
    if(pMemBlock){
        for(; pMemBlock != NULL;pMemBlock = pMemBlock->next){
            printf("**********pMemBlock info[%d]**************\n",i++);
            printf("*\tnSize = %d\n*\tnFree = %d\n*\tnFirst = %d\n*\taData = %p\n",pMemBlock->nSize,pMemBlock->nFree,pMemBlock->nFirst,pMemBlock->aData);
            printf("*****************************************\n");
        }
    }
}