#include "my_malloc.h"
// some useful macros
// adapted from Computer System : A Programmer's Perspective 3rd Edition Global Edition Section9.9
#define WSIZE 4             /* single word size */
#define DSIZE 8             /* double word size */
#define CHUNKSIZE (1 << 12) /* size require by sbrk */

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define PACK(size, alloc) ((size) | (alloc))

/* read word from the given pointer */
// use unsigned int cast to make pointer 32 bits
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* functions to read header*/
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* functions to get header address and footer address*/
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* round up to the nearest align */
#define ALIGN(len) (((len) + DSIZE - 1) & ~0x7)

/* compute ptr to next and prev block*/
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))
static int FLAG = -1;
static char *HEAP_LISTP;
static int init();
static void *extend_heap(size_t words);
static void *merge(void *bp);
static void *my_alloc(size_t size, void *(*find_fit)(size_t));
static void my_free(void *ptr);
static void *find_ff(size_t size);
static void *find_bf(size_t size);
static void place(void *ptr, size_t size);
// require initial space and structure for malloc
static int init()
{
    // create prologue and epilogue
    if ((HEAP_LISTP = (char *)sbrk(4 * WSIZE)) == (void *)-1)
    {
        return -1;
    }
    PUT(HEAP_LISTP, 0);                            // allignment padding
    PUT(HEAP_LISTP + (1 * WSIZE), PACK(DSIZE, 1)); // prologue header
    PUT(HEAP_LISTP + (2 * WSIZE), PACK(DSIZE, 1)); // prologue footer
    PUT(HEAP_LISTP + (3 * WSIZE), PACK(0, 1));     // epilogue header
    HEAP_LISTP += (2 * WSIZE);                     // pointer to the epilogue of the footer, so that NEXT_BLOKP would be the first block
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    {
        return -1;
    }
    return 0;
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size = ALIGN(words * WSIZE);
    if ((long)(bp = (char *)sbrk(size)) == -1)
    {
        return NULL;
    }
    // set new free block, replace old epilogue
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue
    // merge free block if previous is free block
    return merge(bp);
}

// merge a free block with adjacent free blocks
static void *merge(void *bp)
{
    size_t pre_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(FTRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    // 1. prev and next all alloc: do nothing
    if (pre_alloc && next_alloc)
    {
        return bp;
    }
    // 2. prev is free and next alloc: merge with front, set prev header length to sum of two length and cur footer
    else if (!pre_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    // 3. prev is alloc and next is free : merge with next, set cur header and next footer length to length sum
    else if (pre_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    // 4. prev is free and next is free: merge with front and next, set prev header and next footer to the length sum
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

static void my_free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }
    // set header and footer
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    // merge with adjacent blocks
    merge(ptr);
}

static void *my_alloc(size_t size, void *(*find_fit)(size_t))
{
    if (size == 0)
    {
        return NULL;
    }
    size_t asize = ALIGN(DSIZE + size);
    char *bp;
    // find a place that can hold total size
    // if exist, allocate
    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }
    // else call for more memories and put it into the last chunk
    size_t extend_size = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extend_size / WSIZE)) == NULL)
    {
        return NULL;
    }
    place(bp, asize);
    return bp;
}

// best fit find
// static void *find_bf(size_t size)
// {
//     size_t minDiff = size;
//     char *ans = NULL;
//     for (char *bp = HEAP_LISTP; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
//     {
//         if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= size)
//         {
//             if (GET_SIZE(HDRP(bp)) == size)
//             {
//                 return bp;
//             }
//             if ((GET_SIZE(HDRP(bp)) - size) < minDiff)
//             {
//                 minDiff = (GET_SIZE(HDRP(bp)) - size);
//                 ans = bp;
//             }
//         }
//     }
//     return ans;
// }

static void *find_bf(size_t size)
{
    int flag = 0;
    for (char *bp = HEAP_LISTP; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= size)
        {
            if (flag == 0)
            {
                flag == 0;
            }
            else
            {
                return bp;
            }
        }
    }
    return NULL;
}

// first fit find
static void *find_ff(size_t size)
{
    for (char *bp = HEAP_LISTP; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= size)
        {
            return bp;
        }
    }
    return NULL;
}

// place an alloc block on bp
static void place(void *bp, size_t size)
{
    size_t bsize = GET_SIZE(HDRP(bp));
    // if no place for a new block, padding
    if (bsize - size < (2 * DSIZE))
    {
        PUT(HDRP(bp), PACK(bsize, 1));
        PUT(FTRP(bp), PACK(bsize, 1));
        return;
    }
    // else split the block
    PUT(HDRP(bp), PACK(size, 1));
    PUT(FTRP(bp), PACK(size, 1));
    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(bsize - size, 0));
    PUT(FTRP(bp), PACK(bsize - size, 0));
    return;
}

void *ff_malloc(size_t size)
{
    // initialize prologue and epilogue of the malloc
    if (FLAG == -1)
    {
        if (init() == -1)
        {
            return NULL;
        }
        FLAG = 0;
    }
    return my_alloc(size, find_ff);
}

void *bf_malloc(size_t size)
{
    // initialize prologue and epilogue of the malloc
    if (FLAG == -1)
    {
        if (init() == -1)
        {
            return NULL;
        }
        FLAG = 0;
    }
    return my_alloc(size, find_bf);
}

void ff_free(void *ptr)
{
    return my_free(ptr);
}

void bf_free(void *ptr)
{
    return my_free(ptr);
}

unsigned long get_largest_free_data_segment_size() // in bytes
{
    size_t largest_size = 0;
    char *bp;
    for (bp = HEAP_LISTP; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) > largest_size)
        {
            largest_size = GET_SIZE(HDRP(bp));
        }
    }
    return largest_size;
}
unsigned long get_total_free_size() // in bytes
{
    size_t total_free_size = 0;
    char *bp;
    for (bp = HEAP_LISTP; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)))
        {
            total_free_size += GET_SIZE(HDRP(bp));
        }
    }
    return total_free_size;
}