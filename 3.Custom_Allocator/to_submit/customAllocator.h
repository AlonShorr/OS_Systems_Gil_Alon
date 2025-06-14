#ifndef CUSTOM_ALLOCATOR
#define CUSTOM_ALLOCATOR
//
/*=============================================================================
* Do no edit lines below!
=============================================================================*/
#include <unistd.h> //for sbrk, brk
#include <stdint.h> //for uintptr_t
#include <stdlib.h> //for malloc, free, calloc, realloc
#include <stdio.h>  //for printf, fprintf
#include <string.h> //for memset, memcpy
#include <errno.h>  //for errno
#include <stddef.h> //for size_t
#include <stdbool.h> //for bool



void* customMalloc(size_t size);
void customFree(void* ptr);
void* customCalloc(size_t nmemb, size_t size);
void* customRealloc(void* ptr, size_t size);


/*=============================================================================
* Do no edit lines above!
=============================================================================*/




// Heap creation and destruction functions - 
void heapCreate()
{
    return;
}
void heapKill()
{
    return;
}

/*=============================================================================
* If writing bonus - uncomment lines below
=============================================================================*/
// #ifndef BONUS
// #define BONUS
// #endif
// void* customMTMalloc(size_t size);
// void customMTFree(void* ptr);
// void* customMTCalloc(size_t nmemb, size_t size);
// void* customMTRealloc(void* ptr, size_t size);

// void heapCreate();
// void heapKill();

/*=============================================================================
* Defines
=============================================================================*/
#define SBRK_FAIL (void*)(-1)
#define ALIGN_TO_MULT_OF_4(x) (((((x) - 1) >> 2) << 2) + 4)

/*=============================================================================
* Block 
=============================================================================*/
// Suggestion for block usage - feel free to change this
typedef struct Block {
    size_t size;
    //bool free;
    struct Block* next;
} Block;



/*=============================================================================
* added functions
=============================================================================*/
int brk(void *end_data_segment);
void handleSbrkError(void *sbrk_result);
void handlebrkError(int brk_result);
bool is_valid_malloc_ptr (void *ptr);
Block* find_free_place(size_t size);
//inline Block *payload_to_block(void *payload);
//inline void *block_to_payload(Block *b);


#endif // CUSTOM_ALLOCATOR
