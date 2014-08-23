
#include "cldMemPool.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

typedef struct _test {
    int name;
    int size;
    int buf;
}test;

#define NUM  200

int main()
{
    test *test1[NUM] = {'\0'};
   // clock_t begin;
   // clock_t end;
    
    cldMemPool *pMemPool = _cldMemPool_init(sizeof(test),20,30);
#if 1
    int i;
    for(i= 0; i<51; i++){
        test1[i] = _cldMem_malloc( pMemPool);
    }
    Print_MemoryPool_INFO(pMemPool);
    print_MemoryBlockInfoByPointer(pMemPool,(void *)test1[0]);
    
    cldMem_free(pMemPool,(void *) test1[0]);
    Print_MemoryPool_INFO(pMemPool);

    for(i=51; i< NUM; i++){
        test1[i] = _cldMem_malloc( pMemPool);
    }
    print_MemoryBlockInfoByPointer(pMemPool,(void *)test1[51]);
    Print_MemoryPool_INFO(pMemPool);
    
#else
    void **ptr;
    int loops = 100;
    int nRandom = 0;
    int nw = 0;
    int nc = 0;
    int i,j;

    nRandom = rand()%100 + 100;
    printf("nRandom = %d\n",nRandom);
    ptr = malloc(nRandom * sizeof(void *));
    
    memset(ptr,0,nRandom * sizeof(void *));

    while(loops--){
        nc = rand()%nRandom;

        for(i =0;i < nc;i++){
            for(j=0;j<nRandom;j++){
                if(!ptr[j]){
                    ptr[j] = _cldMemBlock_malloc(pMemPool);
                    nw++;
                }
            }
        }

        if(j == nRandom){
            break;
        }
    }

	printf("nw = %d\n",nw);
    check_Memory_pool(pMemPool);
    nc =rand()%nw;
    void *p = NULL; 
    while(nc--){
        nRandom = rand()%nw;
        p = ptr[nRandom];
        if(p){
            check_MemoryBlock(pMemPool,p);
            cldMemBlock_free(pMemPool,p);
            p = NULL;
            nw--;
        }
    }
      check_Memory_pool(pMemPool);  

      _cldMemPool_exit(pMemPool);
#endif
#if 0
    begin = clock();
    for(i = 0; i < NUM; i++){
        test1[i] = _cldMemBlock_malloc(pMemPool);
        if(!test1[i]){
            printf("malloc failed\n");
        }
    }
    end = clock();
    printf("malloc use time %d s\n",(end-begin));
    begin = clock();
    for(i = 0 ; i < NUM; i++){
        cldMemBlock_free(pMemPool,(void *)test1[i]);
    }
    end = clock();
    printf("free use time %d s\n",(end-begin));

    _cldMemPool_exit(pMemPool);

    begin = clock();
    for(i = 0; i < NUM; i++){
        test1[i] =(test *) malloc(sizeof(test));
        if(!test1[i]){
            printf("malloc failed\n");
        }
    }
    end = clock();
    printf("malloc use time %d ms\n",(end-begin));
    begin = clock();
    for(i = 0 ; i < NUM; i++){
        free(test1[i]);
    }
    end = clock();
    printf("free use time %d ms\n",(end-begin));
#endif
}
