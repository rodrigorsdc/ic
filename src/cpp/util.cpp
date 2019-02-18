#include <stdlib.h>
#include <stdio.h>
#include "util.h"

using namespace std;

extern const double INF = 1.0 / 0.0;

static void check_mcalloc(void *ptr) {
    if (ptr == NULL) {
	printf("malloc/calloc return NULL!\n");
	exit(EXIT_FAILURE);
    }
}

int* malloc_int(int n) {
    int* ptr;
    ptr = (int*) malloc(n * sizeof(int));
    check_mcalloc(ptr);
    return ptr;
}

int **matrix_int(int n, int m) {
    int **ptr;
    ptr = (int **) calloc(n, sizeof(int *));
    check_mcalloc(ptr);
    for (int i = 0; i < n; i++) {
	ptr[i] = (int *) calloc(m, sizeof(int));
	check_mcalloc(ptr[i]);
    }
    return ptr;    
}
