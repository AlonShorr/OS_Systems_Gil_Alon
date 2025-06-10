#ifndef CUSTOM_ALLOCATOR
#define CUSTOM_ALLOCATOR

/*=============================================================================
* Do no edit lines below!
=============================================================================*/
#include <stddef.h> //for size_t

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
    bool free;
    Block* next;
} Block;

#endif // CUSTOM_ALLOCATOR
