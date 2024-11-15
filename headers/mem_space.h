//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#ifndef MEM_SPACE_H
#define MEM_SPACE_H

#include <stdlib.h>
#include <stdio.h>

#if defined(DEBUG)
#define debug(...) fprintf(stderr, __VA_ARGS__)
#else
#define debug(...)
#endif

/** Retourne l'adresse de l'espace mémoire à utiliser dans l'allocateur. **/
void *mem_space_get_addr(void);
/** Retourne la taille de l'espace mémoire à utilier dans l'allocateur. **/
size_t mem_space_get_size(void);

#endif //MEM_SPACE_H
