#include <stdio.h>
#include <R.h>
#include <Rinternals.h>
#include <omp.h>
/* #include <gsl/gsl_rng.h> */

void out() {
    SEXP ans = PROTECT(allocVector(REALSXP, 5));
    double *pans = REAL(ans);
    /* gsl_rng *rng = gsl_rng_alloc(gsl_rng_taus); */
    /* double u = gsl_rng_uniform(rng); */
    #pragma omp parallel for
    for (int i = 0; i < 5; i++) {
	pans[i] = i;
	/* printf("%d %lf\n", i, u);  */
	printf("%d\n", i);
    }
    UNPROTECT(1);    
}
