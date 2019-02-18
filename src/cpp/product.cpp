
/* Código de geração de produto cartesiano */

#include <stdio.h>
#include <utility>
#include <stdlib.h>
#include "array.h"
#include "util.h"

using namespace std;
#include "product.h"

static int pow(int a, int b);    
static void product_step_pointer(product* p);
static product* malloc_product(int n);

static int pow(int a, int b) {
    int result = 1;
    for (int i = 0; i < b; i++) result *= a;
    return result;
}

product* product_init(array* a, int repeat) {
    product* p = malloc_product(1);
    p->pointer = array_zeros(repeat);
    p->A = array_copy(a);
    p->counter = 0;
    p->END = pow(p->A->size, p->pointer->size);
    return p;

}

int product_has_next(product* p) {
    if (p->counter == p->END) return false;
    return true;
}

static void product_step_pointer(product* p) {
    if (p->pointer->size > 0) {
        p->pointer->array[0]++;
        for (int i = 0; i < p->pointer->size-1 ; i++)
            if (p->pointer->array[i] % p->A->size == 0) {
                p->pointer->array[i] = 0;
                p->pointer->array[i+1]++;
            } else
                break;
    }
}

void product_finish(product* p) {
    array_destroy(p->A);
    array_destroy(p->pointer);
    free(p);
    p = NULL;
}

array* product_next(product* p) {
    array* result = array_zeros(p->pointer->size);
    for (int i = 0; i < p->pointer->size; i++)
        result->array[i] = p->A->array[p->pointer->array[i]];
    product_step_pointer(p);
    p->counter++;
    array_reverse(result);
    return result;
}

static product* malloc_product(int n) {
    product* ptr = (product*) malloc(n * sizeof(product));
    if (ptr == NULL) {
        printf("malloc devolveu NULL!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

