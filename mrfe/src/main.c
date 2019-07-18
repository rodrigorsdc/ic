#include <R.h>
#include <Rinternals.h>
#include "mrfe.h"
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

void setUp(struct mrfe_data *data, SEXP A,
		 SEXP sample, SEXP c, SEXP max_neigh, SEXP k) {
    data->V_size = ncols(sample);
    data->A_size = asInteger(A);
    data->sample_size = nrows(sample);
    data->sample = matrixINT(data->sample_size, data->V_size);
    flatten_to_matrix(data->sample, data->sample_size,
    		      data->V_size, INTEGER(sample));
    data->V = array_arange(data->V_size);
    data->A = array_arange(data->A_size);
    if (data->cv_enable == 1) {
	data->fold = matrixINT(data->sample_size, data->V_size);
	data->out_fold = matrixINT(data->sample_size, data->V_size);
	data->c_min = REAL(c)[0];
	data->c_max = REAL(c)[1];
	data->c_interval = REAL(c)[2];
	data->k = asInteger(k);
    } else 
	data->c = asReal(c);
    
    if (isNull(max_neigh))
	data->max_neigh = data->V_size - 1;
    else
	data->max_neigh = asInteger(max_neigh);
}

static SEXP array_to_vector(int i, struct mrfe_data *data) {
    SEXP v = PROTECT(allocVector(INTSXP, data->adj[i]->size));
    for (int j = 0; j < data->adj[i]->size; j++)
	INTEGER(v)[j] = data->adj[i]->array[j]+1;
    return v;
}

void input_checking(SEXP A, SEXP sample, SEXP c, SEXP max_neigh,
		    SEXP k) {
    if (!isNumeric(A) || length(A) != 1 || asInteger(A) <= 0)
	error("A argument must be a scalar positive integer");

    if (!isNumeric(sample) || !isMatrix(sample))
	error("sample argument must be a integer-entry matrix");

    if (!isNumeric(c))
	error("c argument must be double");

    if (!isNull(max_neigh) && (!isNumeric(max_neigh) ||
			       length(max_neigh) != 1 ||
			       ncols(sample) <= asInteger(max_neigh)))
	error("max_neigh, if used, must be a scalar integer and"
	      " be less than ncols(sample)");

    if (k != NULL && length(k) != 1 && !isNumeric(k))
	error("k argument must be a scalar integer");
}



SEXP Rmrfe(SEXP A, SEXP sample, SEXP c, SEXP max_neigh) {
    input_checking(A, sample, c, max_neigh, NULL);
    SEXP ans;
    ans = PROTECT(allocVector(VECSXP, ncols(sample)));
    PROTECT(sample = coerceVector(sample, INTSXP));
    PROTECT(c = coerceVector(c, REALSXP));
    struct mrfe_data *data = (struct mrfe_data *)
	R_alloc(1, sizeof(struct mrfe_data));
    data->cv_enable = 0;
    setUp(data, A, sample, c, max_neigh, NULL);
    mrfe(data);
    for (int i = 0; i < ncols(sample); i++)
	SET_VECTOR_ELT(ans, i, array_to_vector(i, data));
    UNPROTECT(ncols(sample) + 3);
    return ans;
}

SEXP Rmrfe_cv(SEXP A, SEXP sample, SEXP c, SEXP k,
	      SEXP max_neigh) {
    input_checking(A, sample, c, max_neigh, k);
    SEXP ans = PROTECT(allocVector(REALSXP, 1));
    PROTECT(sample = coerceVector(sample, INTSXP));
    PROTECT(c = coerceVector(c, REALSXP));
    struct mrfe_data *data = (struct mrfe_data *)
	R_alloc(1, sizeof(struct mrfe_data));
    data->cv_enable = 1;
    setUp(data, A, sample, c, max_neigh, k);
    mrfe_cv(data);
    REAL(ans)[0] = data->c;
    UNPROTECT(3);
    return ans;
}

