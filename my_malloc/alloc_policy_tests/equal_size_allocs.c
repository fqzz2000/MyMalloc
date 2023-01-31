#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "my_malloc.h"

#define NUM_ITERS 10000
#define NUM_ITEMS 10000
#define ALLOC_SIZE 128
#define UNIT 1000

#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p) ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p) bf_free(p)
#endif
#ifdef SF
#define MALLOC(sz) sf_malloc(sz)
// #define MALLOC(sz) malloc(sz)
#define FREE(p) sf_free(p)
#endif

double calc_time(struct timespec start, struct timespec end)
{
  double start_sec = (double)start.tv_sec * 1000000000.0 + (double)start.tv_nsec;
  double end_sec = (double)end.tv_sec * 1000000000.0 + (double)end.tv_nsec;

  if (end_sec < start_sec)
  {
    return 0;
  }
  else
  {
    return end_sec - start_sec;
  }
};

int main(int argc, char *argv[])
{
  int i, j;
  int *array[NUM_ITEMS];
  int *spacing_array[NUM_ITEMS];
  unsigned long largest_free_block;
  unsigned long data_segment_free_space;
  struct timespec start_time, end_time;

  if (NUM_ITEMS < UNIT)
  {
    printf("Error: NUM_ITEMS must be >= 1000\n");
    return -1;
  } // if

  for (i = 0; i < NUM_ITEMS; i++)
  {
    array[i] = (int *)MALLOC(ALLOC_SIZE);
    spacing_array[i] = (int *)MALLOC(ALLOC_SIZE);
  } // for i

  for (i = 0; i < NUM_ITEMS; i++)
  {
    FREE(array[i]);
  } // for i

  // printf("here\n");
  // Start Time
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  for (i = 0; i < NUM_ITERS; i++)
  {
    // printf("%d\n", i);
    for (j = 0; j < UNIT; j++)
    {
      array[j] = (int *)MALLOC(ALLOC_SIZE);
    } // for j
    // check_memory();
    // check_free();

    for (j = UNIT; j < NUM_ITEMS; j++)
    {
      array[j] = (int *)MALLOC(ALLOC_SIZE);
      // check_memory();
      // check_free();
      FREE(array[j - UNIT]);

      if ((i == NUM_ITERS / 2) && (j == NUM_ITEMS / 2))
      {
        // Record fragmentation halfway through (try to repsresent steady state)
        largest_free_block = get_largest_free_data_segment_size();
        data_segment_free_space = get_total_free_size();
      } // if
    }   // for j

    for (j = NUM_ITEMS - UNIT; j < NUM_ITEMS; j++)
    {
      FREE(array[j]);
    } // for j
  }   // for i

  // Stop Time
  clock_gettime(CLOCK_MONOTONIC, &end_time);

  double elapsed_ns = calc_time(start_time, end_time);
  printf("Execution Time = %f seconds\n", elapsed_ns / 1e9);
  printf("Fragmentation  = %f\n", 1.0 - largest_free_block / (float)data_segment_free_space);

  for (i = 0; i < NUM_ITEMS; i++)
  {
    FREE(spacing_array[i]);
  }

  return 0;
}
