#include "array.h"

#ifndef MRFE_H
#define MRFE_H

struct mrfe_data {
    int V_size;
    array *V;
    int A_size;
    array *A;
    double c;
    int max_neigh;
    int max_edges;
    array **adj;
    int **sample;
    int sample_size;
    int num_graphs;
    /* For Cross Validation use */
    int k;
    array *fold_bloc;
    int **fold;
    int fold_size;
    int **out_fold;
    int out_fold_size;
    int cv_enable;
    double *c_values;
    int c_values_size;
};

void mrfe(struct mrfe_data *data);
void mrfe_cv(struct mrfe_data *data);

#endif
