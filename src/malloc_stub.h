//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#ifndef MEM_MALLOC_STUB_H
#define MEM_MALLOC_STUB_H

#include <stdlib.h>

void *malloc(size_t s);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#endif //MEM_MALLOC_STUB_H
