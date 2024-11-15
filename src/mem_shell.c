//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "mem_space.h"
#include "mem.h"
#include "mem_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define TAILLE_BUFFER 128
#define MAX_ALLOCATIONS 128

void aide() {
    fprintf(stderr, "Aide :\n");
    fprintf(stderr, "Saisir l'une des commandes suivantes\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "a taille  :   allouer un bloc de la taille souhaitee\n");
    fprintf(stderr,
            "l adresse :   librer un bloc alloue precedemment a adresse\n");
    fprintf(
        stderr,
        "f n       :   librer le bloc alloue lors de la n-ieme allocation\n");
    fprintf(stderr, "i         :   afficher la liste des emplacements memoire "
                    "inoccupes\n");
    fprintf(stderr, "o         :   afficher la liste des emplacements memoire "
                    "occupes\n");
    fprintf(stderr, "M         :   afficher la liste de tous les emplacements "
                    "memoire (libres et occupes)\n");
    fprintf(stderr, "m         :   afficher le dump de la memoire\n");
    fprintf(stderr, "h         :   afficher cette aide\n");
    fprintf(stderr, "q         :   quitter ce programme\n");
    fprintf(stderr, "\n");
}

void afficher_zone(void *adresse, size_t taille, int free) {
    printf("Zone %s, Adresse : %lu, Taille : %lu\n", free ? "libre" : "occupee",
           (unsigned long)adresse, (unsigned long)taille);
}

void afficher_zone_libre(void *adresse, size_t taille, int free) {
    if (free)
        afficher_zone(adresse, taille, 1);
}

void afficher_zone_occupee(void *adresse, size_t taille, int free) {
    if (!free)
        afficher_zone(adresse, taille, 0);
}

int main(int argc, char **argv) {
    char buffer[TAILLE_BUFFER];
    char commande;
    char *adresse;
    void *ptr;
    int offset;
    int taille, i;
    void *allocations[MAX_ALLOCATIONS];
    int nb_alloc = 0;

    memset(allocations, 0, sizeof(allocations));

    aide();
    mem_init();

    while (1) {
        fprintf(stderr, "? ");
        fflush(stdout);
        commande = getchar();
        switch (commande) {
        case 'a':
            scanf("%d", &taille);
            ptr = mem_alloc(taille);
            allocations[nb_alloc] = ptr;
            nb_alloc++;
            if (ptr == NULL)
                printf("Echec de l'allocation\n");
            else
                printf("Memoire allouee en %d\n",
                       (int)(ptr - mem_space_get_addr()));
            break;
        case 'l':
            scanf("%d", &offset);
            mem_free(mem_space_get_addr() + offset);
            printf("Memoire liberee\n");
            break;
        case 'f':
            scanf("%d", &offset);
            assert(offset < MAX_ALLOCATIONS);
            mem_free(allocations[offset - 1]);
            allocations[offset - 1] = NULL;
            printf("Memoire liberee\n");
            break;
        case 'i':
            mem_show(afficher_zone_libre);
            break;
        case 'o':
            mem_show(afficher_zone_occupee);
            break;
        case 'M':
            mem_show(afficher_zone);
            break;
        case 'm':
            printf("[ ");
            adresse = mem_space_get_addr();
            for (i = 0; i < mem_space_get_size(); i++)
                printf("%d ", adresse[i]);
            printf("]\n");
            break;
        case 'h':
            aide();
            break;
        case 'q':
            exit(0);
        default:
            fprintf(stderr, "Commande inconnue !\n");
        }
        /* vide ce qu'il reste de la ligne dans le buffer d'entree */
        fgets(buffer, TAILLE_BUFFER, stdin);
    }
    return 0;
}
