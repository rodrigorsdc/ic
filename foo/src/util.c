#include <stdlib.h>
#include <stdio.h>
#include "util.h"

const double INF = 1.0 / 0.0;

static void check_mcalloc(void *ptr) {
    if (ptr == NULL) {
	printf("malloc/calloc return NULL!\n");
	exit(EXIT_FAILURE);
    }
}

void free_matrixINT(int **A, int n) {
    for (int i = 0; i < n; i++)
	free(A[i]);
    free(A);
}

void matrixINTcpy(int **A, int **B, int n, int m) {
    for (int i = 0; i < n; i++)
	for (int j = 0; j < m; j++)
	    A[i][j] = B[i][j];
}

int* malloc_int(int n) {
    int* ptr;
    ptr = (int*) malloc(n * sizeof(int));
    check_mcalloc(ptr);
    return ptr;
}

int **matrixINT(int n, int m) {
    int **ptr;
    ptr = (int **) calloc(n, sizeof(int *));
    check_mcalloc(ptr);
    for (int i = 0; i < n; i++) {
	ptr[i] = (int *) calloc(m, sizeof(int));
	check_mcalloc(ptr[i]);
    }
    return ptr;    
}
