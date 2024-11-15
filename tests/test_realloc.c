#include "mem_space.h"
#include "mem.h"
#include "mem_os.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ALLOC 100
#define NB_TESTS 5

void *checked_alloc(size_t size) {
    void *ptr = mem_alloc(size);
    assert(ptr != NULL);
    debug("Allocated %zu bytes at %p\n", size, ptr);
    return ptr;
}

void *checked_realloc(void *ptr, size_t size) {
    void *new_ptr = mem_realloc(ptr, size);
    assert(new_ptr);
    debug("Reallocated to %zu bytes at %p\n", size, new_ptr);
    return new_ptr;
}

void test_normal_realloc() {
    debug("\n--- Testing normal reallocation ---\n");
    void *ptr = checked_alloc(50);
    memset(ptr, 'A', 50);
    ptr = checked_realloc(ptr, 100);
    assert(memcmp(ptr, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 50) == 0);
    ptr = checked_realloc(ptr, 25);
    assert(memcmp(ptr, "AAAAAAAAAAAAAAAAAAAAAAAAA", 25) == 0);
    mem_free(ptr);
}

void test_zero_size_realloc() {
    debug("\n--- Testing reallocation to zero size ---\n");
    void *ptr = checked_alloc(50);
    void *new_ptr = mem_realloc(ptr, 0);
    assert(new_ptr);
}

void test_null_ptr_realloc() {
    debug("\n--- Testing reallocation of NULL pointer ---\n");
    void *ptr = checked_realloc(NULL, 100);
    mem_free(ptr);
}

void test_large_realloc() {
    debug("\n--- Testing large reallocation ---\n");
    void *ptr = checked_alloc(1000);
    ptr = checked_realloc(ptr, mem_space_get_size() - 1000);
    mem_free(ptr);
}

void test_multiple_realloc() {
    debug("\n--- Testing multiple reallocations ---\n");
    void *ptrs[MAX_ALLOC];
    int num_allocs = 0;
    
    while (num_allocs < MAX_ALLOC && (ptrs[num_allocs] = mem_alloc(10)) != NULL) {
        num_allocs++;
    }
    
    for (int i = 0; i < num_allocs; i++) {
        ptrs[i] = checked_realloc(ptrs[i], 20);
        ptrs[i] = checked_realloc(ptrs[i], 5);
        ptrs[i] = checked_realloc(ptrs[i], 15);
    }
    
    for (int i = 0; i < num_allocs; i++) {
        mem_free(ptrs[i]);
    }
}

int main() {
    mem_init();
    fprintf(stderr, "Testing memory reallocation functionality\n"
                    "Crashes indicate test failures\n"
                    "Define DEBUG at compilation for verbose output\n\n");
    
    for (int i = 0; i < NB_TESTS; i++) {
        debug("=== Test iteration %d ===\n", i + 1);
        test_normal_realloc();
        test_zero_size_realloc();
        test_null_ptr_realloc();
        test_large_realloc();
        test_multiple_realloc();
    }
    
    printf("All reallocation tests completed successfully!\n");
    return 0;
}