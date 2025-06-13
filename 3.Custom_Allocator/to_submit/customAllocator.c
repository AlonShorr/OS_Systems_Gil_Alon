#define _GNU_SOURCE
//
#include "customAllocator.h"
Block* heap_blocks_head = NULL;
void* heap_start = NULL;
/*=============================================================================
  Helper Functions
=============================================================================*/
void handleSbrkError(void *sbrk_result) {
    if (sbrk_result == (void*)-1) {
        if (errno == ENOMEM) {
            fprintf(stderr, "<sbrk error>: out of memory\n");
        } else if (errno == EINVAL) {
            fprintf(stderr, "<sbrk error>: invalid parameter\n"); // this print is for us to debug
        }
        brk(heap_start);
        exit(1);
    }
}

void handlebrkError(int brk_result) {
    if (brk_result == -1) {
        if (errno == ENOMEM) {
            fprintf(stderr, "<brk error>: out of memory\n");
        } else if (errno == EINVAL) {
            fprintf(stderr, "<brk error>: invalid parameter\n"); // this print is for us to debug
        }
        brk(heap_start);
        exit(1);
    }
}

Block* find_free_place(size_t size){
    Block *current = heap_blocks_head;
    Block *prev = NULL;
    if (current!=NULL){
        // gap befor first block
        if ((char*)current - (char*)heap_start >= size + sizeof(Block)) {
            Block *new_block = (Block*)heap_start;
            new_block->size = size; // Update the size
            new_block->next = current; // Ensure next points to the first block
            heap_blocks_head = new_block; // Update the head of the list
            return new_block;
        }
        // no gap before first block
        prev = current;
        current = current->next;
    }
    // go over the list to find a free block
    while (current != NULL) {
        if ((char*)current - ((char*)prev + sizeof(Block) + prev->size) >= size + sizeof(Block)) {
            // Found a free block with enough size
            Block *new_block = (Block*)((char*)prev + sizeof(Block) + prev->size);
            new_block->size = size; // Update the size
            new_block->next = current; // Ensure next is NULL for the new block
            prev->next = new_block; // Link the previous block to the new block
            return new_block;
        }
        prev = current;
        current = current->next;
    }

    // If no suitable free block found, return NULL
    return NULL;
}


static inline Block *payload_to_block(void *payload) {
    return (Block *)((char *)payload - sizeof(Block));
}

static inline void *block_to_payload(Block *b) {
    return (char *)b + sizeof(Block);
}

bool is_valid_malloc_ptr(void *payload)
{
    if (!payload) return false;
    Block *cur = heap_blocks_head;
    while (cur) {
        if (block_to_payload(cur) == payload)
            return true;              // now the comparison matches correctly
        cur = cur->next;
    }
    return false;
}

/*=============================================================================
  Excersize Functions
=============================================================================*/
void* customMalloc(size_t size){
    if (size <= 0) {
        fprintf(stderr, "<customMalloc error>: passed nonpositive size\n"); // this print is for us to debug
        return NULL;
    }
    if (heap_blocks_head == NULL) {
        heap_start = sbrk(0);
        handleSbrkError(heap_start);
        size_t needed_size = ALIGN_TO_MULT_OF_4(size + sizeof(Block));
        heap_blocks_head = sbrk(needed_size);
        heap_blocks_head->size = size; // Initialize the first block with size 
        heap_blocks_head->next = NULL; // Initialize the first block's next pointer to NULL
        return (char*)heap_blocks_head + sizeof(Block);
    }

    Block *block = find_free_place(size);
    //total size needed for the new block
    size_t new_block_size = ALIGN_TO_MULT_OF_4(size+sizeof(Block));
    if (block == NULL) {
        // didnt find a free block with enough size
        block = sbrk(new_block_size);
        handleSbrkError(block);
        block->size = size; // Update the size
        block->next = NULL; // Ensure next is NULL for the new block
        Block* last_block = heap_blocks_head;
        while (last_block->next != NULL) {
            last_block = last_block->next; // Move to the end of the list
        }
        last_block->next = block; // Link the last block to the new block
        //return (char*)block + sizeof(*Block);
    }
    return (char*)block + sizeof(Block);
}

void customFree(void* ptr) {
    if (ptr == NULL){ // Block list is empty
        fprintf(stderr, "<free error>: passed null pointer\n");
        return; 
    }
    if(!is_valid_malloc_ptr(ptr)) {
        fprintf(stderr, "<free error>: passed non-heap pointer\n");
        return;
    }
    Block* before_block = heap_blocks_head;
    Block *block_to_free = payload_to_block(ptr);
    int brk_result;
    if (before_block == block_to_free){
        // If the block to be freed is the first block
        heap_blocks_head = before_block->next; // Update head to the next block
        if (heap_blocks_head == NULL) {
            // If the block to be freed is the last block
            brk_result = brk(heap_start);
            handlebrkError(brk_result);
        }
        return;
    }
    else {
        while (before_block->next != block_to_free) {
            before_block = before_block->next; // Move to the next block
        }
        before_block->next = before_block->next->next; // Remove the block from the list
    }
    if (before_block->next == NULL) {
        // If the block to be freed is the last block
        int brk_result = brk((char*)before_block + sizeof(Block) + before_block->size);
        handlebrkError(brk_result);
    }
}

//FIX PRINTS AND CHECK VALID INPUT
void* customCalloc(size_t nmemb, size_t size){
    size_t total_memSize = ALIGN_TO_MULT_OF_4(nmemb * size);
    if (nmemb <= 0 || size <= 0) {
        fprintf(stderr, "<calloc error>: passed nonpositive value\n"); // this print is for us to debug
        return NULL;
    }
    void* ptr = customMalloc(total_memSize);
    if (ptr == NULL) {
        return NULL;
    }
    // write 0 to all bytes in the allocated memory
    for (size_t i = 0; i < total_memSize; i++) {
        *((char*)ptr + i) = 0;
    }
    return ptr;
}

void* customRealloc(void* ptr, size_t size) {
    
    if (ptr == NULL) return customMalloc(size);
    
    if(!is_valid_malloc_ptr(ptr)) {
        fprintf(stderr, "<realloc error>: passed non-heap pointer\n");
        return NULL;
    }
    
    Block *realloced_block = payload_to_block(ptr);
    size_t old_size = realloced_block->size;
 
    if(size < old_size) {
        realloced_block->size = size;
        return ptr;
    }
    else { // old_size < size --> allocate new larger block
        void* newMem = customMalloc(size);
        if(newMem == NULL) return NULL;

        for(size_t i = 0; i < old_size; i++) {
            *((char*)newMem + i) = *((char*)ptr + i);
        }
        
        customFree(ptr);
        return newMem;
    }   
}


// REMEMBER TO ERASE THIS
void print_heap_blocks(const char *tag)
{
    printf("\n--- heap blocks (%s) ---\n", tag);
    Block *cur = heap_blocks_head;
    int idx = 0;
    while (cur) {
        void *payload = (char *)cur + sizeof(Block);
        printf("[%02d] header=%p  payload=%p  size=%zu  next=%p\n",
               idx, (void*)cur, payload, cur->size, (void*)cur->next);
        cur = cur->next;
        ++idx;
    }
    printf("------------------------\n");
}

