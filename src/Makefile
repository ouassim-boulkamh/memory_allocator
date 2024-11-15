##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2023-2024
##------------------------------------------------------------------------------

.SUFFIXES:

CC=gcc

# uncomment to compile in 32bits mode (require gcc-*-multilib packages
# on Debian/Ubuntu)
#HOST32= -m32

CXXFLAGS= $(HOST32) -Wall -Werror -std=c++20 -g
CFLAGS= $(HOST32) -Wall -std=c17 -g -DMEMORY_SIZE=128000 #-Werror
CFLAGS+= -DDEBUG
CFLAGS+= -I../headers
LDFLAGS= $(HOST32)
INCLUDE=.
LIB_DIR=../libs

PROGRAMS=mem_shell mem_shell2 libmalloc.so libmalloc_core_static.a 

.PHONY: all place_libs

all: $(PROGRAMS) place_libs

# dépendances des binaires
DEPS=mem.o mem_space.o

%:  $(DEPS) %.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -fPIC -c $< $(CFLAGS)

# Create static lib to embed in tests
libmalloc_core_static.a: $(DEPS)
	$(AR) -rcs $@ $^

place_libs: libmalloc_core_static.a libmalloc.so
	mkdir -p ${LIB_DIR};\
	mv libmalloc_core_static.a libmalloc.so ${LIB_DIR};

# test avec des programmes existant
# création d'une librairie partagée
# pour MAC OS utiliser -install_name à la place de soname
libmalloc.so: malloc_stub.o $(DEPS)
	$(CC) -fPIC -shared -Wl,-soname,$@ $^ -o $@


# nettoyage
clean:
	rm -f *.o $(PROGRAMS)

distclean: clean
	rm -rf ${LIB_DIR}

# deps
mem.c: ../headers/mem.h ../headers/mem_os.h ../headers/mem_space.h
mem_os.c: ../headers/mem_os.h
mem_space.c: ../headers/mem_space.h
