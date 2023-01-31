#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
// malloc with first fit finding policy
void *ff_malloc(size_t size);
// malloc with best fit finding policy
void *bf_malloc(size_t size);
// malloc with second fit finding policy
void *sf_malloc(size_t size);
// free for ff_maloc
void ff_free(void *ptr);
// free for bf_malloc
void bf_free(void *ptr);
// free for sf_malloc
void sf_free(void *ptr);
// compute the largest free block size in bytes
unsigned long get_largest_free_data_segment_size(); // in bytes
// compute the total free blocks size in bytes
unsigned long get_total_free_size(); // in bytes