#include <R.h>
#include <Rinternals.h>
#include "mrfe_graph.h"
#include "util.h"

void flatten_to_matrix(int **M, int n, int m, int *V) {
    int k = 0;
    for (int i = 0; i < n; i++)
	for (int j = 0; j < m; j++)
	    M[i][j] = V[k++];
}

void matrix_to_flatten(int *V, int **M, int n, int m) {
    int k = 0;
    for (int i = 0; i < n; i++)
	for (int j = 0; j < m; j++)
	    V[k++] = M[i][j];
}

void print_matrix(int **M, int n, int m) {
    for (int i = 0; i < n; i++) {
	for (int j = 0; j < m; j++)
	    printf("%d ", M[i][j]);
	printf("\n");
    }
}

void graph_setUp(struct mrfe_graph_data *data, SEXP A_size,
		 SEXP sample, SEXP c, SEXP k) {
    data->V = ncols(sample);
    data->A_size = *(INTEGER(A_size));
    data->sample_size = nrows(sample);
    data->sample = (int **) malloc(data->sample_size * sizeof(int *));
    for (int i = 0; i < data->sample_size; i++)
	data->sample[i] = (int *) malloc(data->V * sizeof(int));
    flatten_to_matrix(data->sample, data->sample_size,
    		      data->V, INTEGER(sample));
    data->adj = matrixINT(data->V, data->V);
    data->max_edges = (data->V * (data->V - 1)) / 2;
    data->A = array_arange(data->A_size);
    data->num_graphs = 1 << data->max_edges;
    if (length(c) == 3) {
	data->fold = matrixINT(data->sample_size, data->V);
	data->out_fold = matrixINT(data->sample_size, data->V);
	data->cv_enable = 1;
	data->c_min = REAL(c)[0];
	data->c_max = REAL(c)[1];
	data->c_interval = REAL(c)[2];
	data->k = *(INTEGER(k));
    } else {
	data->cv_enable = 0;
	data->c = *(REAL(c));
    }    
}


/* MRFE GRAPH */
SEXP mrfe_gr(SEXP A_size, SEXP sample, SEXP c, SEXP k) {
    SEXP ans;
    if (!isInteger(A_size) || length(A_size) != 1)
	error("A_size argument must be a scalar integer");
    if (!isInteger(sample))
	error("sample argument must be a integer-entry matrix");
    if (!isReal(c) || (length(c) != 1 &&  length(c) != 3))
	error("c argument must be a scalar double or 3-length double vector");
    if (length(c) == 3 && !isInteger(k))
	error("k argument must be a scalar integer");
    ans = PROTECT(allocMatrix(INTSXP, ncols(sample), ncols(sample)));
    
    struct mrfe_graph_data *data = (struct mrfe_graph_data *)
    	malloc(sizeof(struct mrfe_graph_data));
    graph_setUp(data, A_size, sample, c, k);
    mrfe_graph(data);
    if (length(c) == 3)
	printf("best regularizer: %lf\n", data->c);
    matrix_to_flatten(INTEGER(ans), data->adj, data->V, data->V);
    UNPROTECT(1);
    return ans;
}
