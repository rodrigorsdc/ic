#ifndef COMBINATION_H
#define COMBINATION_H

#include <gsl/gsl_combination.h>
#include "array.h"

typedef struct{
    int n;
    int size;
    bool END;
    gsl_combination* gsl_c;
    array* data;
}combination;

combination* combination_init(array* a, int size);
array* combination_next(combination* c);
bool combination_has_next(combination* c);
void combination_finish(combination* c);
#endif










