#include <R.h>
#include <Rinternals.h>
#include "mrfe_graph.h"
#include "mrfe_neigh.h"
#include "array.h"
#include "util.h"

void flatten_to_matrix(int **M, int n, int m, int *V) {
    int k = 0;
    for (int i = 0; i < n; i++)
	for (int j = 0; j < m; j++)
	    M[i][j] = V[n*j + i];
}

void matrix_to_flatten(int *V, int **M, int n, int m) {
    int k = 0;
    for (int i = 0; i < n; i++)
	for (int j = 0; j < m; j++)
	    V[n*j + i] = M[i][j];
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
    data->sample = matrixINT(data->sample_size, data->V);
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

void neigh_setUp(struct mrfe_neigh_data *data, SEXP A_size,
		 SEXP sample, SEXP c, SEXP max_neigh, SEXP k) {
    data->V_size = ncols(sample);
    data->A_size = asInteger(A_size);
    data->sample_size = nrows(sample);
    data->sample = matrixINT(data->sample_size, data->V_size);
    flatten_to_matrix(data->sample, data->sample_size,
    		      data->V_size, INTEGER(sample));
    data->V = array_arange(data->V_size);
    data->A = array_arange(data->A_size);
    if (length(c) == 3) {
	data->fold = matrixINT(data->sample_size, data->V_size);
	data->out_fold = matrixINT(data->sample_size, data->V_size);
	data->cv_enable = 1;
	data->c_min = REAL(c)[0];
	data->c_max = REAL(c)[1];
	data->c_interval = REAL(c)[2];
	data->k = asInteger(k);
    } else {
	data->cv_enable = 0;
	data->c = asReal(c);
    }
    if (isNull(max_neigh))
	data->max_neigh = data->V_size - 1;
    else
	data->max_neigh = asInteger(max_neigh);
}

static SEXP array_to_vector(int i, struct mrfe_neigh_data *data) {
    SEXP v = PROTECT(allocVector(INTSXP, data->adj[i]->size));
    for (int j = 0; j < data->adj[i]->size; j++)
	INTEGER(v)[j] = data->adj[i]->array[j]+1;
    return v;


}
/* MRFE GRAPH */
SEXP mrfe_gr(SEXP A_size, SEXP sample, SEXP c, SEXP k) {    
    if (!isInteger(A_size) || length(A_size) != 1)
	error("A_size argument must be a scalar integer");
    if (!isInteger(sample))
	error("sample argument must be a integer-entry matrix");
    if (!isReal(c) || (length(c) != 1 &&  length(c) != 3))
	error("c argument must be a scalar double or 3-length double vector");
    if (length(c) == 3 && !isInteger(k))
	error("k argument must be a scalar integer");
    
    SEXP ans = PROTECT(allocMatrix(INTSXP, ncols(sample), ncols(sample)));    
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


void input_checking(SEXP A_size, SEXP sample, SEXP c, SEXP max_neigh,
		    SEXP k) {
    if (!isNumeric(A_size) || length(A_size) != 1)
	error("A_size argument must be a scalar integer");

    if (!isNumeric(sample) || !isMatrix(sample))
	error("sample argument must be a integer-entry matrix");

    if (!isNumeric(c) || (length(c) != 1 &&  length(c) != 3))
	error("c argument must be a scalar double or 3-length "
	      "double vector");

    if (!isNull(max_neigh) && (!isNumeric(max_neigh) ||
			       length(max_neigh) != 1 ||
			       ncols(sample) <= asInteger(max_neigh)))
	error("max_neigh, if used, must be a scalar integer and"
	      " be less than ncols(sample)");

    if (length(k) != 1 && !isNumeric(k))
	error("k argument must be a scalar integer");
}

SEXP mrfe_ne(SEXP A_size, SEXP sample, SEXP c, SEXP max_neigh, SEXP k) {
    input_checking(A_size, sample, c, max_neigh, k);
    SEXP ans;
    ans = PROTECT(allocVector(VECSXP, ncols(sample)));
    PROTECT(sample = coerceVector(sample, INTSXP));
    PROTECT(c = coerceVector(c, REALSXP));
    struct mrfe_neigh_data *data = (struct mrfe_neigh_data *)
	R_alloc(1, sizeof(struct mrfe_graph_data));
    neigh_setUp(data, A_size, sample, c, max_neigh, k);
    mrfe_neigh(data);
    if (length(c) == 3)
	printf("best regularizer: %lf\n", data->c);
    for (int i = 0; i < ncols(sample); i++)
	SET_VECTOR_ELT(ans, i, array_to_vector(i, data));
    UNPROTECT(ncols(sample) + 3);
    return ans;
}
