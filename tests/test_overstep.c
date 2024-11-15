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
    assert(ptr);
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

void test_normal_write() {
    printf("Testing normal write...\n");
    char *ptr = checked_alloc(SMALL_ALLOC);
    memset(ptr, 'A', SMALL_ALLOC);
    free_all();
}

void test_write_to_freed_memory() {
    printf("Testing write to freed memory...\n");
    char *ptr = checked_alloc(SMALL_ALLOC);
    mem_free(ptr);
    memset(ptr, 'A', SMALL_ALLOC);
    alloc_count--;
}

void test_write_to_next_allocation() {
    printf("Testing write into next allocation...\n");
    char *ptr1 = checked_alloc(MEDIUM_ALLOC);
    checked_alloc(MEDIUM_ALLOC);
    memset(ptr1 + MEDIUM_ALLOC + MEDIUM_ALLOC / 2, 'A', 1);
    free_all();
}

void test_write_one_byte_beyond() {
    printf("Testing write one byte beyond allocation...\n");
    char *ptr = checked_alloc(SMALL_ALLOC);
    ptr[SMALL_ALLOC] = 'A';
    free_all();
}

void test_large_overflow() {
    printf("Testing large overflow...\n");
    char *ptr = checked_alloc(MEDIUM_ALLOC);
    memset(ptr, 'A', LARGE_ALLOC);
    free_all();
}

void test_underflow() {
    printf("Testing underflow...\n");
    char *ptr = checked_alloc(SMALL_ALLOC);
    ptr[-1] = 'A';
    free_all();
}

int main() {
    mem_init();
    
    printf("Starting memory boundary violation tests...\n");
    
    test_normal_write();
    test_write_to_freed_memory();
    test_write_to_next_allocation();
    test_write_one_byte_beyond();
    test_large_overflow();
    test_underflow();
    
    printf("All tests completed. If no crashes occurred, the allocator handled the boundary violations.\n");
    
    return 0;
}