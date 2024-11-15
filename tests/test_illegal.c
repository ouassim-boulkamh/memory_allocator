#include "mem_space.h"
#include "mem.h"
#include "mem_os.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SMALL_ALLOC 16
#define MEDIUM_ALLOC 128
#define LARGE_ALLOC 1024

void *allocs[100];
int alloc_count = 0;

void *checked_alloc(size_t size) {
    void *ptr = mem_alloc(size);
    assert(ptr != NULL);
    allocs[alloc_count++] = ptr;
    return ptr;
}

void free_all() {
    for (int i = 0; i < alloc_count; i++) {
        if (allocs[i] != NULL) {
            mem_free(allocs[i]);
            allocs[i] = NULL;
        }
    }
    alloc_count = 0;
}

void test_normal_allocation_free() {
    printf("Testing normal allocation and free...\n");
    void *ptr = checked_alloc(SMALL_ALLOC);
    mem_free(ptr);
    alloc_count--;
}

void test_free_null() {
    printf("Testing free of NULL pointer...\n");
    mem_free(NULL);
}

void test_double_free() {
    printf("Testing double free...\n");
    void *ptr = checked_alloc(SMALL_ALLOC);
    mem_free(ptr);
    mem_free(ptr);
    alloc_count--;
}

void test_free_unallocated() {
    printf("Testing free of unallocated memory...\n");
    void *ptr = (void*)0x12345678;  // Some arbitrary address
    mem_free(ptr);  // Illegal: freeing unallocated memory
}

void test_free_middle_of_allocation() {
    printf("Testing free of middle of allocation...\n");
    char *ptr = checked_alloc(MEDIUM_ALLOC);
    mem_free(ptr + SMALL_ALLOC);  // Illegal: freeing from middle of allocation
    alloc_count--;
}

int main() {
    mem_init();
    
    printf("Starting illegal allocator calls tests...\n");
    
    test_normal_allocation_free();
    test_free_null();
    test_double_free();
    test_free_unallocated();
    test_free_middle_of_allocation();
    
    printf("All tests completed. If no crashes occurred, the allocator handled the illegal calls.\n");
    
    return 0;
}