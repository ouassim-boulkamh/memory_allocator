##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2023-2024
##------------------------------------------------------------------------------

###############################################################################
# commands
CC=gcc

# build flags
CFLAGS= -Wall -g -DMEMORY_SIZE=128000 #-Werror
CFLAGS+= -I../headers
CFLAGS+= -DDEBUG
LDFLAGS=

###############################################################################
# targets
TESTS=test_init test_fusion test_fusion_bzero test_frag test_base test_base_bzero test_cheese test_cheese_bzero test_realloc test_overstep test_illegal
PROGRAMS=$(TESTS)
LIB_DIR=../libs

###############################################################################
all: $(PROGRAMS)

%: %.c ${LIB_DIR}/libmalloc_core_static.a
	$(CC) $(CFLAGS) --std=c17 -L${LIB_DIR} -o $@ $< -lmalloc_core_static

test:
	for file in $(TESTS);do echo; echo ================== $$file =================; echo; ./$$file || exit; done

test_all:
	for file in $(TESTS);do echo "\033[0;35mRUNNING  $$file...\033[0m"; ./$$file > /dev/null 2> /dev/null && echo "\033[0;32mSUCCESS $$file\033[0m" || echo "\033[0;31mFAILURE $$file !!!\033[0m"; done

googletest:
	echo "\033[0;35mYou need the advaced branch of the project \033[0m"

clean:
	$(RM) -f $(PROGRAMS)

distclean: clean

.PHONY: clean test test_all distclean

###############################################################################
#include deps
%.c: ../headers/mem_os.h ../headers/mem_space.h ../headers/mem.h
