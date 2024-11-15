//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#ifndef MEM_H
#define MEM_H

#include <stddef.h>

/* -----------------------------------------------*/
/* Interface utilisateur de votre allocateur      */
/* -----------------------------------------------*/
void *mem_alloc(size_t);
void mem_free(void *);
size_t mem_get_size(void *);
void *mem_realloc(void *, size_t);

/* Itérateur sur le contenu de l'allocateur */
void mem_show(void (*print)(void *, size_t, int free));

/* Enable logging for debugging */
void mem_set_logging(int enabled);

#endif // MEM_H
