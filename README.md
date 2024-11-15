
# malloc, free, realloc

Implements simplified versions of `malloc`, `free`, and `realloc`. Includes an integrity check through a magic number in the beginning and end of allocated blocks. Works using a linked list for free blocks.
Could use an improvement to handle alignment and just better overall code structure.
Tests and mem.c were written by me, other files might have minor changes, but were all provided by my professor in the context of a university project.
Is not optimised or intended for any real world use.
