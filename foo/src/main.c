#include <R.h>
#include <Rinternals.h>
/* #include "pml_graph.h" */

SEXP pml_graph(SEXP A_size, SEXP sample, SEXP c, SEXP k) {
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
    printf("dfwe");
    /* struct pml_graph_data *data = (struct pml_graph_data *) */
    /* 	malloc(sizeof(struct pml_graph_data)); */

    /* data->V = ncols(sample); */
    /* data->A_size = A_size; */
    /* data->sample_size = nrows(sample); */
    /* /\* data->sample = REAL(sample) *\/ */
    /* data->k = *(INTEGER(k)); */
	
    UNPROTECT(1);
    return ans;
}
