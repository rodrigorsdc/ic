#include "array.h"

#ifndef MRFE_GRAPH_H
#define MRFE_GRAPH_H

struct mrfe_graph_data {
    int V;
    int A_size;
    array *A;
    double c;
    int max_edges;
    int **adj;
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

/* The adjacency matrix graph estimated will be stored at
 * data->adj*/
void mrfe_graph(struct mrfe_graph_data *data);

#endif
