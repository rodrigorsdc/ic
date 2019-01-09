#include <stdlib.h>
#include <stdio.h>
#include "util.h"

using namespace std;

extern const double INF = 1.0 / 0.0;

int* malloc_int(int n) {
    int* ptr;
    ptr = (int*) malloc(n * sizeof(int));
    if (ptr == NULL) {
        printf("malloc devolveu NULL!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

