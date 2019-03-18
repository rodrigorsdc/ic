/* Código gerador de combinação a partir de um array */

#include <gsl/gsl_combination.h>
#include <stdlib.h>
#include "array.h"
#include "combination.h"

static combination* malloc_combination(unsigned int n);

combination* combination_init(array* a, int size) {
    combination* c = malloc_combination(1);
    if (size == 0)
    c->n = a->size;
    c->size = size;
    // printf("A: %d\n", a->size);
    c->gsl_c = gsl_combination_calloc(a->size, size);
    c->data = array_copy(a);
    c->END = 0;
    return c;
}

array* combination_next(combination* c) {
    array* result = array_zeros(c->size);
    for (int i = 0; i < c->size; i++) {
        int indice = gsl_combination_get(c->gsl_c, i);
        result->array[i] = c->data->array[indice];
    }
    if (gsl_combination_next(c->gsl_c) != GSL_SUCCESS)
        c->END = 1;
    return result;    
}

int combination_has_next(combination* c) {
    return !c->END;
}

void combination_finish(combination* c) {
    array_destroy(c->data);
    gsl_combination_free(c->gsl_c);
    free(c);
    c = NULL;
}

static combination* malloc_combination(unsigned int n) {
    combination* ptr = (combination*) malloc(n * sizeof(combination));
    if (ptr == NULL) {
        printf("malloc devolveu NULL!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}


