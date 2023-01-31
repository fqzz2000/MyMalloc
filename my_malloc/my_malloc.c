#include "my_malloc.h"
// some useful macros
// adapted from Computer System : A Programmer's Perspective 3rd Edition Global Edition Section 9.9

#define WSIZE 4             /* single word size */
#define DSIZE 8             /* double word size */
#define MSIZE 24            /* minimum block size*/
#define CHUNKSIZE (1 << 12) /* size require by sbrk */

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define PACK(size, alloc) ((size) | (alloc))

/* read word from the given pointer */
// use unsigned int cast to make pointer 32 bits
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define PUT_LONG(p, val) (*(void **)(p) = (val))

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

/* Get pointer of free list */
#define GET_NEXT_FREE(bp) ((void **)(bp))
#define GET_PREV_FREE(bp) ((void **)((char *)(bp) + DSIZE))

/* jump through free list*/
#define PREV_FREE(bp) (*GET_PREV_FREE(bp))
#define NEXT_FREE(bp) (*GET_NEXT_FREE(bp))

static int FLAG = -1;
// head for all blocks
static char *HEAP_LISTP;
// head for free list
static char *FREE_HEAD;
// initialize memory structure for malloc
static int init();
// acquire more memory from system
static void *extend_heap(size_t words);
// coalesce current block with adjacent free blocks（if exists）
// return the pointer of the merged block
static void *merge(void *bp);
// allocate given size of memory, the selection policy is passed by find_fit function pointer
static void *my_alloc(size_t size, void *(*find_fit)(size_t));
// free the memory of given pointer
static void my_free(void *ptr);
// first fit find
static void *find_ff(size_t size);
// best fit find
static void *find_bf(size_t size);
// second fit find
static void *find_sf(size_t size);
// alloc a chunk of memory into the given free block, new free blocks after split would be
// added to the top of free list
static void place(void *ptr, size_t size);
// delete a free block from the free list
static void delete_fblock(void *bp);

static void delete_fblock(void *bp)
{
    if (bp != (void *)FREE_HEAD)
    {
        PUT_LONG(GET_NEXT_FREE(PREV_FREE(bp)), NEXT_FREE(bp));
        PUT_LONG(GET_PREV_FREE(NEXT_FREE(bp)), PREV_FREE(bp));
    }
    else
    {
        FREE_HEAD = PREV_FREE(bp);
    }
}
// require initial space and structure for malloc
static int init()
{
    // create prologue and epilogue
    if ((HEAP_LISTP = (char *)sbrk(6 * WSIZE)) == (void *)-1)
    {
        return -1;
    }
    PUT(HEAP_LISTP, PACK(0, 1));                   // header of free list tail
    PUT_LONG(HEAP_LISTP + (1 * WSIZE), 0);         // free list tail
    PUT(HEAP_LISTP + (3 * WSIZE), PACK(DSIZE, 1)); // prologue header
    PUT(HEAP_LISTP + (4 * WSIZE), PACK(DSIZE, 1)); // prologue footer
    PUT(HEAP_LISTP + (5 * WSIZE), PACK(0, 1));     // epilogue header
    FREE_HEAD = HEAP_LISTP + (1 * WSIZE);          // set free list tail
    HEAP_LISTP += (4 * WSIZE);                     // pointer to the epilogue of the footer, so that NEXT_BLOKP would be the first block
    // acquire some intial space
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    {
        return -1;
    }
    return 0;
}

static void *extend_heap(size_t words)
{
    char *bp;

    size_t size = MAX(ALIGN(words * WSIZE), MSIZE);
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
    size_t pre_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    // 1. prev and next all alloc: do nothing
    if (pre_alloc && next_alloc)
    {
    }
    // 2. prev is free and next alloc: merge with front, set prev header length to sum of two length and cur footer
    else if (!pre_alloc && next_alloc)
    { // delete pre from linked list
        // set pre prev next as pre next
        void *left = PREV_BLKP(bp);
        delete_fblock(left);
        // merge
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    // 3. prev is alloc and next is free : merge with next, set cur header and next footer length to length sum
    else if (pre_alloc && !next_alloc)
    {
        // delete next from linked list
        void *right = NEXT_BLKP(bp);
        delete_fblock(right);
        // merge
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    // 4. prev is free and next is free: merge with front and next, set prev header and next footer to the length sum
    else
    {
        void *left = PREV_BLKP(bp);
        delete_fblock(left);
        // delete next
        void *right = NEXT_BLKP(bp);
        // merge
        delete_fblock(right);
        // merge blocks
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    // add merged block to free list
    PUT_LONG(GET_NEXT_FREE(FREE_HEAD), bp);
    PUT_LONG(GET_PREV_FREE(bp), FREE_HEAD);
    FREE_HEAD = bp;
    return bp;
}
// free the memory
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
    size_t asize = MAX(ALIGN(DSIZE + size), MSIZE);
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
static void *find_bf(size_t size)
{
    size_t minDiff = size;
    char *ans = NULL;
    for (char *bp = FREE_HEAD; GET_ALLOC(HDRP(bp)) == 0; bp = PREV_FREE(bp))
    {
        // early stop if already exist a perfect match
        if (GET_SIZE(HDRP(bp)) == size)
        {
            return bp;
        }
        if (GET_SIZE(HDRP(bp)) >= size)
        {
            if ((GET_SIZE(HDRP(bp)) - size) <= minDiff)
            {
                minDiff = (GET_SIZE(HDRP(bp)) - size);
                ans = bp;
            }
        }
    }
    return ans;
}

// first fit find
static void *find_ff(size_t size)
{
    for (void *bp = FREE_HEAD; GET_ALLOC(HDRP(bp)) == 0; bp = PREV_FREE(bp))
    {
        if (GET_SIZE(HDRP(bp)) >= size)
        {
            return bp;
        }
    }
    return NULL;
}

// second fit find
static void *find_sf(size_t size)
{
    int flag = 0;
    for (void *bp = FREE_HEAD; GET_ALLOC(HDRP(bp)) == 0; bp = PREV_FREE(bp))
    {
        if (GET_SIZE(HDRP(bp)) >= size)
        {
            // skip the first matched free block
            if (flag == 0)
            {
                flag = 1;
            }
            else
            {
                return bp;
            }
        }
    }
    return NULL;
}

// place an alloc block on bp
static void place(void *bp, size_t size)
{
    size_t bsize = GET_SIZE(HDRP(bp));
    // if no place for a new block, padding
    // delete from free list
    if (bsize - size < MSIZE)
    {
        delete_fblock(bp);
        PUT(HDRP(bp), PACK(bsize, 1));
        PUT(FTRP(bp), PACK(bsize, 1));
        return;
    }
    // else split the block
    // delte free block
    delete_fblock(bp);
    PUT(HDRP(bp), PACK(size, 1));
    PUT(FTRP(bp), PACK(size, 1));
    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(bsize - size, 0));
    PUT(FTRP(bp), PACK(bsize - size, 0));
    // add new block to free list
    PUT_LONG(GET_NEXT_FREE(FREE_HEAD), bp);
    PUT_LONG(GET_PREV_FREE(bp), FREE_HEAD);
    FREE_HEAD = bp;
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

void *sf_malloc(size_t size)
{
    if (FLAG == -1)
    {
        if (init() == -1)
        {
            return NULL;
        }
        FLAG = 0;
    }
    return my_alloc(size, find_sf);
}

void ff_free(void *ptr)
{
    return my_free(ptr);
}

void bf_free(void *ptr)
{
    return my_free(ptr);
}

void sf_free(void *ptr)
{
    return my_free(ptr);
}

// iterate through the heap to find the largest
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

// iterate through the heap to add all together
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