#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Test malloc
    char *str = (char *)malloc(20 * sizeof(char));
    if (str == NULL) {
        printf("malloc failed\n");
        return 1;
    }
    strcpy(str, "Hello, World!");
    printf("malloc: %s\n", str);

    // Test realloc
    str = (char *)realloc(str, 30 * sizeof(char));
    if (str == NULL) {
        printf("realloc failed\n");
        return 1;
    }
    strcat(str, " Extended!");
    printf("realloc: %s\n", str);

    // Test free
    free(str);
    printf("Memory freed\n");

    return 0;
}