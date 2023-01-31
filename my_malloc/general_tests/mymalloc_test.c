#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"

#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p) ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p) bf_free(p)
#endif

void test1()
{

  const unsigned NUM_ITEMS = 10;
  int i;
  int size;
  int sum = 0;
  int expected_sum = 0;
  int *array[NUM_ITEMS];

  size = 4;
  expected_sum += size * size;
  array[0] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[0][i] = size;
  } // for i
  for (i = 0; i < size; i++)
  {
    sum += array[0][i];
  } // for i

  size = 16;
  expected_sum += size * size;
  array[1] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[1][i] = size;
  } // for i
  for (i = 0; i < size; i++)
  {
    sum += array[1][i];
  } // for i

  size = 8;
  expected_sum += size * size;
  array[2] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[2][i] = size;
  } // for i
  for (i = 0; i < size; i++)
  {
    sum += array[2][i];
  } // for i

  size = 32;
  expected_sum += size * size;
  array[3] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[3][i] = size;
  } // for i
  for (i = 0; i < size; i++)
  {
    sum += array[3][i];
  } // for i

  FREE(array[0]);
  FREE(array[2]);

  size = 7;
  expected_sum += size * size;
  array[4] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[4][i] = size;
  } // for i
  for (i = 0; i < size; i++)
  {
    sum += array[4][i];
  } // for i

  size = 256;
  expected_sum += size * size;
  array[5] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[5][i] = size;
  } // for i
  for (i = 0; i < size; i++)
  {
    sum += array[5][i];
  } // for i

  FREE(array[5]);
  FREE(array[1]);
  FREE(array[3]);

  size = 23;
  expected_sum += size * size;
  array[6] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[6][i] = size;
  } // for i
  for (i = 0; i < size; i++)
  {
    sum += array[6][i];
  } // for i

  size = 4;
  expected_sum += size * size;
  array[7] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[7][i] = size;
  } // for i
  for (i = 0; i < size; i++)
  {
    sum += array[7][i];
  } // for i

  FREE(array[4]);

  size = 10;
  expected_sum += size * size;
  array[8] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[8][i] = size;
  } // for i
  for (i = 0; i < size; i++)
  {
    sum += array[8][i];
  } // for i

  size = 32;
  expected_sum += size * size;
  array[9] = (int *)MALLOC(size * sizeof(int));
  for (i = 0; i < size; i++)
  {
    array[9][i] = size;
  } // for i
  for (i = 0; i < size; i++)
  {
    sum += array[9][i];
  } // for i

  FREE(array[6]);
  FREE(array[7]);
  FREE(array[8]);
  FREE(array[9]);

  if (sum == expected_sum)
  {
    printf("Calculated expected value of %d\n", sum);
    printf("Test passed\n");
  }
  else
  {
    printf("Expected sum=%d but calculated %d\n", expected_sum, sum);
    printf("Test failed\n");
  } // else
}
void test3()
{
  char *ptr0 = MALLOC((2 << 10) - 8);
  // char *ptr1 = MALLOC((2 << 8) - 8);
  // char *ptr2 = MALLOC((2 << 6) - 8);
  char *ptr3 = MALLOC((2 << 10) - 8);
  char *ptr4 = MALLOC((2 << 10) - 8);
  char *ptr5 = MALLOC((2 << 10) - 8);
  size_t largest_free = get_largest_free_data_segment_size();
  size_t total_free = get_total_free_size();
  printf("init largest >> %lu\n", largest_free);
  printf("init total >> %lu\n", total_free);
}
void test2()
{

  char *ptr1 = MALLOC((2 << 10) - 8);
  char *ptr2 = MALLOC((2 << 10) - 8);
  char *ptr3 = MALLOC((2 << 10) - 8);
  char *ptr4 = MALLOC((2 << 10) - 8);
  char *ptr5 = MALLOC((2 << 10) - 8);
  size_t largest_free = get_largest_free_data_segment_size();
  size_t total_free = get_total_free_size();
  printf("init largest >> %lu\n", largest_free);
  printf("init total >> %lu\n", total_free);
  FREE(ptr1);
  largest_free = get_largest_free_data_segment_size();
  total_free = get_total_free_size();
  printf("after free ptr1 largest >> %lu\n", largest_free);
  printf("after free ptr1 total >> %lu\n", total_free);
  FREE(ptr3);
  largest_free = get_largest_free_data_segment_size();
  total_free = get_total_free_size();
  printf("after free ptr3 largest >> %lu\n", largest_free);
  printf("after free ptr3 total >> %lu\n", total_free);
  FREE(ptr4);
  largest_free = get_largest_free_data_segment_size();
  total_free = get_total_free_size();
  printf("after free ptr4 largest >> %lu\n", largest_free);
  printf("after free ptr4 total >> %lu\n", total_free);
  FREE(ptr2);
  largest_free = get_largest_free_data_segment_size();
  total_free = get_total_free_size();
  printf("after free ptr2 largest >> %lu\n", largest_free);
  printf("after free ptr2 total >> %lu\n", total_free);
  FREE(ptr5);
  largest_free = get_largest_free_data_segment_size();
  total_free = get_total_free_size();
  printf("after free ptr5 largest >> %lu\n", largest_free);
  printf("after free ptr5 total >> %lu\n", total_free);
  // char *ptr2 = MALLOC(128);
  // char *ptr3 = MALLOC(128);
  // largest_free = get_largest_free_data_segment_size();
  // total_free = get_total_free_size();
  // printf("after largest >> %lu\n", largest_free);
  // printf("after total >> %lu\n", total_free);
  return;
}
int main(int argc, char *argv[])
{
  test1();
  // test2();
  // test3();
  return 0;
}
