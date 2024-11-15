/*
 * code for the remove_block function has been adapted from Linus Torvalds' TED talk:
 * https://www.youtube.com/watch?v=o8NPllzkFhE
 * minute 14:35
 *
 * idea for magic number / guards:
 * https://www.cs.princeton.edu/courses/archive/spr09/cos217/lectures/19DynamicMemory2.pdf
 * https://en.wikipedia.org/wiki/Magic_number_(programming)#DEADBEEF
 *
 *
 * decided on 64 instead of 32 for guards number size because compiler pads header to 16 bytes anyways for alignment purposes
 * for 12 byte headers, define alloc block as packed (struct __attribute__((packed)) mem_allocated_block_s)
 * and change guard size to 4 and guard number to unit32_t (might cause worse performance)
 */

#include "mem.h"
#include "mem_space.h"
#include "mem_os.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define SECRET_SIZE 8 // used to compute guards

// definition of the structure for memory blocks
struct mem_free_block_s
{
  // use of this structure is mandatory for the project, we are *not* allowed to switch to a doubly linked list :/
  size_t size;                   // size of the block
  struct mem_free_block_s *next; // pointer to the next block
};

struct mem_allocated_block_s
{
  size_t size;    // size of the block
  uint64_t guard; // guard to check the integrity of the block
};

typedef struct mem_free_block_s mem_free_block_t;
typedef struct mem_allocated_block_s mem_allocated_block_t;

// memory-size-related pre-calculated values
const static size_t MEM_FREE_BLOCK_SIZE = sizeof(mem_free_block_t);                                                             // size of the free block structure
const static size_t MEM_ALL_BLOCK_SIZE = sizeof(mem_allocated_block_t);                                                         // size of the allocated block structure
const static size_t MEM_MAX_BLOCK_SIZE = (MEM_FREE_BLOCK_SIZE > MEM_ALL_BLOCK_SIZE) ? MEM_FREE_BLOCK_SIZE : MEM_ALL_BLOCK_SIZE; // maximum size of a header
const static size_t MEM_MIN_BLOCK_SIZE = (MEM_FREE_BLOCK_SIZE < MEM_ALL_BLOCK_SIZE) ? MEM_FREE_BLOCK_SIZE : MEM_ALL_BLOCK_SIZE; // minimum size of a header
const static size_t MEM_FREE_ALL_DIFF = -MEM_FREE_BLOCK_SIZE + MEM_ALL_BLOCK_SIZE;                                              // difference between the size of a free block and an allocated block

static void *MEM_SPACE_MIN = NULL; // minimum address of the memory space
static void *MEM_SPACE_MAX = NULL; // maximum address of the memory space
static void *memory_area = NULL;   // pointer to the memory area
static uint64_t secret_number = 0; // secret number to compute guards

// linked list for free & allocated blocks
static mem_free_block_t *free_block_list = NULL;

// function pointer for the allocation strategy
static mem_fit_function_t *current_fit_function = mem_first_fit;

// function prototypes
void remove_block(mem_free_block_t *, mem_free_block_t **);
void insert_block(mem_free_block_t *, mem_free_block_t **);
void search_block(mem_free_block_t *, mem_free_block_t ***);

void mem_init()
{
  // first address & size of the memory space
  static size_t total_memory_size = 0;

  // Get the address and size of the memory space
  memory_area = mem_space_get_addr();
  total_memory_size = mem_space_get_size();

  // set memory allowed values
  MEM_SPACE_MIN = memory_area + MEM_MIN_BLOCK_SIZE;
  MEM_SPACE_MAX = (void *)(((char *)memory_area) + total_memory_size - 1);

  // Initialize the first free block
  free_block_list = (mem_free_block_t *)memory_area;
  free_block_list->size = total_memory_size - MEM_FREE_BLOCK_SIZE;
  free_block_list->next = NULL;

  // TODO: replace with logic to compute good secret number
  secret_number = 0xDEADBEEFFEEDFACE; // initialize the secret number

  // Set default allocation strategy
  current_fit_function = mem_first_fit;
}

void mem_set_fit_handler(mem_fit_function_t *mff)
{
  current_fit_function = mff; // set the specified allocation strategy
}

// performs necessary splits/size changes, and returns usable allocated block
// does not return NULL for 0 size allocations anymore, returns 0 sized allocated block instead
void *mem_alloc(size_t size)
{
  size += SECRET_SIZE; // add guard size

  //***************************************STEP 1 : Find suitable block
  mem_free_block_t *block = current_fit_function(free_block_list, size + MEM_FREE_ALL_DIFF);

  if (!block)
    return NULL; // No suitable block found

  //***************************************STEP 2 : Remove block from the free block list
  mem_free_block_t **list = &free_block_list;
  search_block(block, &list);
  remove_block(block, list);

  //***************************************STEP 3 : Split block if needed
  // if the remaining size after the allocation is less than the size of a block, we don't split the block, allocate more instead
  // this is because the splitting operation requires the minimum size of a block
  if ((block->size - size - MEM_FREE_ALL_DIFF) <= MEM_MAX_BLOCK_SIZE + SECRET_SIZE)
  {
    size = block->size - MEM_FREE_ALL_DIFF;
  }
  else
  {
    // Split the block if there's excess space
    mem_free_block_t *new_free_block = (mem_free_block_t *)(((char *)block) + size + MEM_ALL_BLOCK_SIZE);
    new_free_block->size = block->size - size - MEM_ALL_BLOCK_SIZE;
    new_free_block->next = NULL;
    insert_block(new_free_block, list); // Update the free block list
  }

  //***************************************STEP 4 : Initialize block appropriate metadata and return
  // Initialize the allocated block
  *((mem_allocated_block_t *)block) = (mem_allocated_block_t){size, ((uint64_t)block) ^ secret_number};
  // Initialize the last byte with the guard
  *((uint64_t *)(((char *)block) + MEM_ALL_BLOCK_SIZE + size - SECRET_SIZE)) = ((uint64_t)block) ^ secret_number;

  return (void *)(((char *)block) + MEM_ALL_BLOCK_SIZE); // adjust for block header
}

void mem_free(void *zone)
{
  if (!zone || zone < MEM_SPACE_MIN || zone >= MEM_SPACE_MAX)
    return; // refuse NULL pointers and pointers outside the memory area

  mem_allocated_block_t *block_to_free = (mem_allocated_block_t *)(((char *)zone) - MEM_ALL_BLOCK_SIZE); // adjust for block header

  //***************************************STEP 1 : Perform validity checks
  uint64_t guard = ((uint64_t)block_to_free) ^ secret_number;
  assert(block_to_free->guard == guard); // check first guard

  assert(*((uint64_t *)(((char *)block_to_free) + MEM_ALL_BLOCK_SIZE + block_to_free->size - SECRET_SIZE)) == guard); // check last guard

  //***************************************STEP 2 : Initialize block with appropriate metadata
  mem_free_block_t *new_free_block = (mem_free_block_t *)block_to_free;
  new_free_block->size = block_to_free->size - MEM_FREE_ALL_DIFF;
  new_free_block->next = NULL;

  //***************************************STEP 3 : Insert block into free block list
  mem_free_block_t **list = &free_block_list;
  search_block(new_free_block, &list);

  insert_block(new_free_block, list);
}

size_t mem_get_size(void *zone)
{
  if (!zone || zone < MEM_SPACE_MIN || zone >= MEM_SPACE_MAX)
    return 0; // refuse NULL pointers and pointers outside the memory area

  mem_allocated_block_t *block = (mem_allocated_block_t *)(((char *)zone) - MEM_ALL_BLOCK_SIZE); // adjust for block header

  uint64_t guard = ((uint64_t)block) ^ secret_number;
  if (block->guard != guard) // check first guard
    return 0;

  if (*((uint64_t *)(((char *)block) + MEM_ALL_BLOCK_SIZE + block->size - SECRET_SIZE)) != guard) // check last guard
    return 0;

  return block->size - SECRET_SIZE; // return the size of the block
}

void mem_show(void (*print)(void *, size_t, int free))
{
  mem_free_block_t *free_current = free_block_list;
  mem_allocated_block_t *allocated_current = memory_area;

  // this first loop prints any potential allocated blocks at the beginning of the memory space (before first free space)
  while (allocated_current != free_current && allocated_current < MEM_SPACE_MAX)
  {
    print((void *)(allocated_current + 1), allocated_current->size - SECRET_SIZE, 0);
    allocated_current = (mem_allocated_block_t *)(((char *)allocated_current) + MEM_ALL_BLOCK_SIZE + allocated_current->size);
  }

  // this loop prins free and allocated blocks in the memory space
  while (free_current)
  {
    print((void *)(free_current + 1), free_current->size - SECRET_SIZE, 1);
    allocated_current = (mem_allocated_block_t *)(((char *)free_current) + MEM_FREE_BLOCK_SIZE + free_current->size);
    while (allocated_current != free_current->next && allocated_current < MEM_SPACE_MAX)
    {
      print((void *)(allocated_current + 1), allocated_current->size - SECRET_SIZE, 0);
      allocated_current = (mem_allocated_block_t *)(((char *)allocated_current) + MEM_ALL_BLOCK_SIZE + allocated_current->size);
    }
    free_current = free_current->next;
  }
}

mem_free_block_t *mem_first_fit(mem_free_block_t *list, size_t size)
{
  while (list)
  {
    if (list->size >= size)
    {
      return list; // Return the first block that fits
    }
    list = list->next;
  }
  return NULL; // No suitable block found
}

mem_free_block_t *mem_best_fit(mem_free_block_t *list, size_t size)
{
  mem_free_block_t *best_fit = NULL;
  size_t best_size = SIZE_MAX; // Initialize to maximum size

  while (list)
  {
    if (list->size >= size && list->size < best_size)
    {
      best_size = list->size;
      best_fit = list; // Update best_fit if this block is smaller
    }
    list = list->next;
  }
  return best_fit; // Return the best fitting block, or NULL if none found
}

mem_free_block_t *mem_worst_fit(mem_free_block_t *list, size_t size)
{
  mem_free_block_t *worst_fit = NULL;
  size_t worst_size = 0; // Initialize to zero

  while (list)
  {
    if (list->size >= size && list->size > worst_size)
    {
      worst_size = list->size;
      worst_fit = list; // Update worst_fit if this block is larger
    }
    list = list->next;
  }
  return worst_fit; // Return the worst fitting block, or NULL if none found
}

// remove block from sorted list, if it exists
void remove_block(mem_free_block_t *block, mem_free_block_t **list)
{
  // THIS FUNCTION ASSUMES ALL GIVEN BLOCKS AND POINTERS ARE VALID (NO VALIDITY CHECKS!)
  // FOR EFFICIENCY REASONS, THIS FUNCTION NO LONGER PERFORMS FREE BLOCK LIST SEARCHES, IT EXPECTS THE LIST PARAMETER TO BE ON THE CORRECT POSITION, CALL SEARCH BLOCK BEFOREHAND

  if (*list == block)
  {
    // remove block from list
    *list = block->next;
    block->next = NULL;
  }
}

// function that takes a block and inserts it into the free block list (sorted by address)
void insert_block(mem_free_block_t *block, mem_free_block_t **list)
{
  // THIS FUNCTION ASSUMES ALL GIVEN BLOCKS AND POINTERS ARE VALID (NO VALIDITY CHECKS!)
  // FOR EFFICIENCY REASONS, THIS FUNCTION NO LONGER PERFORMS FREE BLOCK LIST SEARCHES, IT EXPECTS THE LIST PARAMETER TO BE ON THE CORRECT POSITION, CALL SEARCH BLOCK BEFOREHAND

  // false only if free block list is empty or we are at the last block
  if (*list)
  {
    // insert block in the middle of the list

    // check if block on the right is contiguous
    if ((((char *)block) + block->size + MEM_FREE_BLOCK_SIZE) == (char *)(*list))
    {
      // fuse blocks
      block->size += MEM_FREE_BLOCK_SIZE + (*list)->size;
      block->next = (*list)->next;
    }
    else
    {
      // not contiguous, regular insertion
      block->next = (*list);
    }
  }
  else
  {
    // insert at the end of the list
    block->next = NULL;
  }

  // calculating the address of the previous (left) free block
  mem_free_block_t *prev = (mem_free_block_t *)(((char *)list) - offsetof(mem_free_block_t, next));

  // flase only if free block list is empty
  if (prev >= memory_area)
  {
    // check if block on the left is contiguous a
    if ((char *)prev + MEM_FREE_BLOCK_SIZE + prev->size == (char *)block)
    {
      // fuse blocks
      prev->size += MEM_FREE_BLOCK_SIZE + block->size;
      prev->next = block->next;
      block = prev;
    }
    else
    {
      // not contiguous, regular insertion
      prev->next = block;
    }
  }
  else
  {
    {
      // inserting into an empty list
      *list = block;
    }
  }
}

// function that searches for a block in the free block list, and updates the list pointer to the correct position
void search_block(mem_free_block_t *block, mem_free_block_t ***list)
{
  // searching for the block in list, break on NULL, or on the right address
  while ((**list) && ((**list) < block))
  {
    *list = &(**list)->next;
  }
}


// function that takes a zone that has been allocated with mem_alloc, and a size, returns pointer with address of resized block or NULL
void *mem_realloc(void *zone, size_t size)
{
  // allocate new block if the pointer is NULL
  if (!zone)
    return mem_alloc(size);

  // STEP 0 : validity checks and pre-calcs
  if (zone < MEM_SPACE_MIN || zone >= MEM_SPACE_MAX)
    return NULL; // refuse pointers outside the memory area

  //*************************case 1 : user is trying to free ----> free and return
  if (!size)
  {
    mem_free(zone);
    return mem_alloc(size); // free the block if the new size is 0
  }

  size += SECRET_SIZE; // add guard size

  mem_allocated_block_t *block_to_realloc = (mem_allocated_block_t *)(((char *)zone) - MEM_ALL_BLOCK_SIZE); // adjust for block header

  uint64_t guard = ((uint64_t)block_to_realloc) ^ secret_number;
  assert(block_to_realloc->guard == guard); // check first guard

  assert(*((uint64_t *)(((char *)block_to_realloc) + MEM_ALL_BLOCK_SIZE + block_to_realloc->size - SECRET_SIZE)) == guard); // check last guard

  //*************************case 2 : same size requested, nothing to do
  if (size == block_to_realloc->size)
  {
    return zone; // return if the size is the same
  }

  // finding the block on the right
  mem_free_block_t *right_block = (mem_free_block_t *)((char *)block_to_realloc + block_to_realloc->size + MEM_ALL_BLOCK_SIZE);

  mem_free_block_t **list = &free_block_list;
  search_block(right_block, &list);

  //*************************case 3 : user is trying to reduce size
  if (size < block_to_realloc->size)
  {
    if (right_block == *list)
    {
      //*************************case 3.1: the block on the right is free
      mem_free_block_t *new_free_block = (mem_free_block_t *)(((char *)block_to_realloc) + size + MEM_ALL_BLOCK_SIZE);
      *new_free_block = (mem_free_block_t){right_block->size + (block_to_realloc->size - size), NULL};

      remove_block(right_block, list);
      insert_block(new_free_block, list);

      // reduce the size of the block
      block_to_realloc->size = size;
      // Initialize the last byte with the guard
      *((uint64_t *)(((char *)block_to_realloc) + MEM_ALL_BLOCK_SIZE + size - SECRET_SIZE)) = ((uint64_t)block_to_realloc) ^ secret_number;

      return zone;
    }
    else
    {
      //*************************case 3.2: the block on the right is not free
      if (block_to_realloc->size - size <= MEM_MAX_BLOCK_SIZE + SECRET_SIZE)
      {
        //*************************case 3.2.1 : remaining size is less than the size of a smallest possible block

        return zone; // return if remaining size is less than the size of a smallest possible block
      }
      else
      {
        //*************************case 3.2.2 : remaining size is more than the size of a smallest possible block

        mem_free_block_t *new_free_block = (mem_free_block_t *)(((char *)block_to_realloc) + size + MEM_ALL_BLOCK_SIZE);
        *new_free_block = (mem_free_block_t){block_to_realloc->size - size - MEM_FREE_BLOCK_SIZE, NULL};
        insert_block(new_free_block, list);

        block_to_realloc->size = size;
        // Initialize the last byte with the guard
        *((uint64_t *)(((char *)block_to_realloc) + MEM_ALL_BLOCK_SIZE + size - SECRET_SIZE)) = ((uint64_t)block_to_realloc) ^ secret_number;
        return zone;
      }
    }
  }

  //*************************case 4 : user is trying to increase size
  if (right_block != *list || right_block->size + MEM_FREE_BLOCK_SIZE < size - block_to_realloc->size)
  {
    //*************************case 4.1 : block on the right is not free or does not have enough space
    char *result;

    // allocate new block
    result = (char *) mem_alloc(size - SECRET_SIZE);

    // return NULL if allocation failed
    if (!result)
      return NULL;

    // copy the data
    memcpy(result, zone, block_to_realloc->size - SECRET_SIZE);

    // on libère l'ancien
    mem_free(zone);

    return (void *) result;
  
  }
  else
  {
    //*************************case 4.2 : block on the right is free and has enough space

    remove_block(right_block, list);

    if (right_block->size - size + block_to_realloc->size + MEM_FREE_BLOCK_SIZE <= MEM_MAX_BLOCK_SIZE + SECRET_SIZE)
    {
      //*************************case 4.2.1 : remaining size is less than the size of smallest possible block

      // fuse blocks
      block_to_realloc->size += right_block->size + MEM_FREE_BLOCK_SIZE;
      *((uint64_t *)(((char *)block_to_realloc) + MEM_ALL_BLOCK_SIZE + block_to_realloc->size - SECRET_SIZE)) = ((uint64_t)block_to_realloc) ^ secret_number;

      return zone;
    }
    else
    {
      //*************************case 4.2.2 : remaining size is more than the size of smallest possible block

      // split the block
      mem_free_block_t *new_free_block = (mem_free_block_t *)(((char *)block_to_realloc) + size + MEM_ALL_BLOCK_SIZE);
      new_free_block->size = right_block->size - size + block_to_realloc->size;
      new_free_block->next = NULL;

      block_to_realloc->size = size;
      *((uint64_t *)(((char *)block_to_realloc) + MEM_ALL_BLOCK_SIZE + block_to_realloc->size - SECRET_SIZE)) = ((uint64_t)block_to_realloc) ^ secret_number;

      insert_block(new_free_block, list);

      return zone;
    }
  }
}
