/*
 * main.c – stand-alone sanity tests for the custom allocator
 *
 *   gcc -std=c99 -Wall -Werror -pedantic-errors -DNDEBUG \
 *       main.c customAllocator.c -o allocator_tests
 *   ./allocator_tests
 */
//
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "testFunc.h"

/* ─── allocator API (forward declarations only) ─────────────── */
void *customMalloc (size_t size);
void  customFree   (void *ptr);
void *customCalloc (size_t nmemb, size_t size);
void *customRealloc(void *ptr, size_t size);

/* ────────────────────────────────────────────────────────────── */
int cm = 0; // for debug prints, set to 1 to enable debug prints
/* ────────────────────────────────────────────────────────────── */

#define ALLOC_COUNT 5          /* used in Test 2            */
#define UNUSED(x) (void)(x)    /* silence -Wunused-variable */

static void verify_pattern(const void *mem, size_t len, unsigned char pat)
{
    const unsigned char *p = mem;
    for (size_t i = 0; i < len; ++i)
        if (p[i] != pat) {
            fprintf(stderr,
                    "pattern mismatch at byte %zu: "
                    "expected 0x%02X, got 0x%02X\n",
                    i, pat, p[i]);
            abort();
        }
}

int main(void)
{
    puts("=== custom allocator quick-tests ===");

    /* ----------------------------------------------------- */
    /* 1 – basic malloc / free                               */
    /* ----------------------------------------------------- */
    {
        const size_t SZ = 16;
        void *p = customMalloc(SZ);
        assert(p);
        memset(p, 0xA5, SZ);
        verify_pattern(p, SZ, 0xA5);
        customFree(p);
        puts("  [1] basic malloc/free ✔");
    }

    /* ----------------------------------------------------- */
    /* 2 – multiple allocations & data integrity             */
    /* ----------------------------------------------------- */
    {
        puts("  [2] multiple allocations");
        const size_t sizes[ALLOC_COUNT]   = {1, 32, 100, 250, 500};
        const unsigned char pats[ALLOC_COUNT] = {0xAA, 0x55, 0x77, 0x33, 0xCC};
        void *ptrs[ALLOC_COUNT] = {0};

        for (int i = 0; i < ALLOC_COUNT; ++i) {
            ptrs[i] = customMalloc(sizes[i]);
            assert(ptrs[i]);
            memset(ptrs[i], pats[i], sizes[i]);
        }
        for (int i = 0; i < ALLOC_COUNT; ++i)
            verify_pattern(ptrs[i], sizes[i], pats[i]);

        /* free in mixed order */
        customFree(ptrs[1]);
        customFree(ptrs[3]);
        customFree(ptrs[0]);
        customFree(ptrs[4]);
        customFree(ptrs[2]);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 3 – calloc zero-initialisation                        */
    /* ----------------------------------------------------- */
    {
        puts("  [3] calloc zero-init");
        const size_t N = 10;
        int *arr = (int *)customCalloc(N, sizeof(int));
        assert(arr);
        for (size_t i = 0; i < N; ++i) assert(arr[i] == 0);
        customFree(arr);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 4 – realloc expand                                    */
    /* ----------------------------------------------------- */
    {
        puts("  [4] realloc expand");
        char *p = (char *)customMalloc(8);
        assert(p);
        strcpy(p, "Hi");
        char *q = (char *)customRealloc(p, 32);
        assert(q && strcmp(q, "Hi") == 0);
        memset(q + 2, 'X', 30);
        customFree(q);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 5 – realloc shrink                                    */
    /* ----------------------------------------------------- */
    {
        puts("  [5] realloc shrink");
        char *p = (char *)customMalloc(32);
        memset(p, 'Y', 32);
        char *q = (char *)customRealloc(p, 16);
        assert(q);
        verify_pattern(q, 16, 'Y');
        customFree(q);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 6 – realloc(NULL, size)                               */
    /* ----------------------------------------------------- */
    {
        puts("  [6] realloc(NULL,size)");
        void *p = customRealloc(NULL, 64);
        assert(p);
        memset(p, 0x99, 64);
        customFree(p);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 7 – realloc(ptr, 0) frees                             */
    /* ----------------------------------------------------- */
    {
        puts("  [7] realloc(ptr,0)");
        void *p = customMalloc(40);
        assert(p);
        void *q = customRealloc(p, 0);   /* may return NULL or not */
        if (q) customFree(q);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 8 – free(NULL) no-op                                  */
    /* ----------------------------------------------------- */
    {
        puts("  [8] free(NULL)");
        customFree(NULL);                /* need to print error */
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 9 – double free detection                             */
    /* ----------------------------------------------------- */
    {
        puts("  [9] double free (allocator should warn or ignore)");
        void *p = customMalloc(24);
        customFree(p);
        customFree(p);                   /* first free do free, the second need to print error */
        puts("      ✔ (survived)");
    }

    /* ----------------------------------------------------- */
    /* 10 – free invalid (stack) pointer                     */
    /* ----------------------------------------------------- */
    {
        puts(" [10] free non-heap pointer");
        int on_stack = 42;
        customFree(&on_stack);           /* need to print error */
        puts("      ✔ (survived)");
    }

    /* ----------------------------------------------------- */
    /* 11 – stress: 1 000 tiny allocations                   */
    /* ----------------------------------------------------- */
    {
        puts(" [11] stress: 1 000 allocations 1–128 B");
        enum { N = 1000 };
        void *ptrs[N];

        for (int i = 0; i < N; ++i) {
            size_t sz = (i % 128) + 1;
            ptrs[i] = customMalloc(sz);
            assert(ptrs[i]);
            memset(ptrs[i], (unsigned char)(i & 0xFF), sz);
        }

        for (int i = 0; i < N; i += 2)
            customFree(ptrs[i]);

        for (int i = 0; i < N; i += 2) {
            ptrs[i] = customMalloc(64);
            assert(ptrs[i]);
            memset(ptrs[i], 0xEE, 64);
        }

        for (int i = 0; i < N; ++i)
            customFree(ptrs[i]);

        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 12 – alignment (payload % 4 == 0)                     */
    /* ----------------------------------------------------- */
    {
        puts(" [12] alignment: each payload %% 4 == 0");
        for (size_t sz = 1; sz <= 32; ++sz) {
            void *p = customMalloc(sz);
            assert(p);
            assert(((uintptr_t)p % 4) == 0);
            customFree(p);
        }
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 13 – realloc may move a big block                     */
    /* ----------------------------------------------------- */
    {
        puts(" [13] realloc move (big → bigger)");
        size_t big = 4096, bigger = 8192;

        char *p = customMalloc(big);
        assert(p);
        memset(p, 0xA7, big);

        char *q = customRealloc(p, bigger);
        assert(q);
        verify_pattern(q, big, 0xA7);
        memset(q + big, 0x5C, bigger - big);

        customFree(q);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 14 – large calloc zero-fill                           */
    /* ----------------------------------------------------- */
    {
        puts(" [14] calloc 4 096 ints zero-init");
        size_t n = 4096;
        int *arr = (int *)customCalloc(n, sizeof(int));
        assert(arr);
        for (size_t i = 0; i < n; ++i) assert(arr[i] == 0);
        customFree(arr);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 15 – gap reuse                                        */
    /* ----------------------------------------------------- */
    {
        puts(" [15] gap reuse behaviour");
        void *a = customMalloc(64);
        void *b = customMalloc(128);
        void *c = customMalloc(64);
        assert(a && b && c);

        customFree(b);

        void *d = customMalloc(96);
        assert(d);
        assert((char *)d > (char *)a && (char *)d < (char *)c);

        customFree(a);
        customFree(c);
        customFree(d);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 16 – calloc nmemb==0 or size==0                       */
    /* ----------------------------------------------------- */
    {
        puts(" [16] calloc edge-cases (nmemb or size zero)");
        void *p1 = customCalloc(0, 10);             // should print error and return NULL (well.. its undefined its to do malloc with size 0)  
        void *p2 = customCalloc(5, 0);              // should print error and return NULL (well.. its undefined its to do malloc with size 0)
        assert(p1 == NULL && p2 == NULL);
        UNUSED(p1);
        UNUSED(p2);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 17 – calloc alignment & uniqueness                    */
    /* ----------------------------------------------------- */
    {
        puts(" [17] calloc alignment and uniqueness");
        void *prev = NULL;
        for (size_t elems = 1; elems <= 32; ++elems) {
            int *arr = (int *)customCalloc(elems, sizeof(int));
            assert(arr);
            assert(((uintptr_t)arr % 4) == 0);
            if (prev) assert(arr != prev);
            prev = arr;
            for (size_t i = 0; i < elems; ++i) assert(arr[i] == 0);
            customFree(arr);
        }
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 18 – realloc on calloc’d block                        */
    /* ----------------------------------------------------- */
    {
        puts(" [18] realloc on calloc'd memory");
        char *p = (char *)customCalloc(128, 1);
        assert(p);
        for (int i = 0; i < 128; ++i) assert(p[i] == 0);

        char *q = (char *)customRealloc(p, 256);
        assert(q);
        for (int i = 0; i < 128; ++i) assert(q[i] == 0);

        memset(q + 128, 0xAB, 128);
        char *r = (char *)customRealloc(q, 64);
        assert(r);
        for (int i = 0; i < 64; ++i) assert(r[i] == 0);

        customFree(r);
        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 19 – allocate-free-allocate pattern                   */
    /* ----------------------------------------------------- */
    {
        puts(" [19] allocate–free–allocate pattern");
        enum { N = 10 };
        void *ptrs[N] = {0};

        for (int i = 0; i < N; ++i) {
            size_t sz = 32 * (i + 1);
            ptrs[i] = customMalloc(sz);
            assert(ptrs[i]);
            memset(ptrs[i], (unsigned char)(0xF0 + i), sz);
        }

        for (int i = 1; i < N; i += 2) {
            customFree(ptrs[i]);
            ptrs[i] = NULL;
        }

        for (int i = 1; i < N; i += 2) {
            ptrs[i] = customMalloc(64);
            assert(ptrs[i]);
            memset(ptrs[i], 0x5A, 64);
        }

        for (int i = 0; i < N; i += 2) {
            unsigned char *p = (unsigned char *)ptrs[i];
#ifndef NDEBUG
            size_t sz = 32 * (i + 1);
            for (size_t j = 0; j < sz; ++j)
                assert(p[j] == (unsigned char)(0xF0 + i));
#else
            UNUSED(p);
#endif
        }

        for (int i = 0; i < N; ++i)
            customFree(ptrs[i]);

        puts("      ✔");
    }

    /* ----------------------------------------------------- */
    /* 20 – 4-byte, 4-byte, free, 2-byte, 2-byte             */
    /* ----------------------------------------------------- */
    {
        puts(" [20] pattern 4-B / 4-B / free / 2-B / 2-B");

        void *p1 = customMalloc(4);
        void *p2 = customMalloc(4);
        assert(p1 && p2);
        print_heap_blocks("after two 4-byte mallocs"); 
        memset(p2, 0xC3, 4);

        customFree(p1);

        void *p3 = customMalloc(2);
        void *p4 = customMalloc(2);
        assert(p3 && p4);
        print_heap_blocks("after two 2-byte mallocs");

#ifndef NDEBUG
        unsigned char *q = (unsigned char *)p2;
        for (int i = 0; i < 4; ++i) assert(q[i] == 0xC3);
#else
        UNUSED(p2);
#endif

        customFree(p2);
        customFree(p3);
        customFree(p4);
        print_heap_blocks("after final cleanup");  
        puts("      ✔");
    }

    puts("=== all tests completed ===");
    return 0;
}
