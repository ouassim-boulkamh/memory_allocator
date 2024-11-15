//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#ifndef MEM_OS_H
#define MEM_OS_H

//include stdlib pour definition du type size_t
#include <stdlib.h>

//Definie la structure the bloc libre
typedef struct mem_free_block_s mem_free_block_t;

/* -----------------------------------------------*/
/* Interface de gestion de votre allocateur       */
/* -----------------------------------------------*/
// Initialisation
void mem_init(void);

// Définition du type mem_fit_function_t
// type des fonctions d'allocation
typedef mem_free_block_t *(mem_fit_function_t)(mem_free_block_t *, size_t);

// Choix de la fonction d'allocation
// = choix de la stratégie de l'allocation
void mem_set_fit_handler(mem_fit_function_t *);

// Stratégies de base (fonctions) d'allocation
mem_fit_function_t mem_first_fit;
mem_fit_function_t mem_worst_fit;
mem_fit_function_t mem_best_fit;

#endif /* MEM_OS_H */
