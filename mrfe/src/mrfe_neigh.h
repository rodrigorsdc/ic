#include "array.h"

#ifndef PML_H
#define PML_H

struct mrfe_neigh_data {
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
    double c_min;
    double c_max;
    double c_interval;
};

void mrfe_neigh(struct mrfe_neigh_data *data);

#endif
