//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "mem_space.h"

// Vous pouvez changer la taille de la mémoire ici,
// si vous n'utilisez pas l'option -DMEMORY_SIZE=...
// lors de la compilation
#if !defined(MEMORY_SIZE)
#define MEMORY_SIZE 4096
#endif

static char memory[MEMORY_SIZE];

void *mem_space_get_addr()
{
    return memory;
}

size_t mem_space_get_size()
{
    return MEMORY_SIZE; 
}
