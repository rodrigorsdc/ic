#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include "mrfe.h"
#include "product.h"
#include "combination.h"
#include "array.h"
#include "util.h"

static void count_in_sample(int v, array *W, array* a,
			    array* aW, int **sample, int sample_size,
			    int *N_W, int *N_v_W) {
    *N_W = 0, *N_v_W = 0;
    int m = W->size;
    array* x_W = array_zeros(m);    
    for (int i = 0; i < sample_size; i++) {
        for (int j = 0; j < m; j++)
	    x_W->array[j] = sample[i][W->array[j]];
        if (array_equals(x_W, aW)) {
            if (sample[i][v] == a->array[0])
		*N_v_W = *N_v_W + 1;
            *N_W = *N_W + 1;
        }
    }
    array_destroy(x_W);        
}

static double likelihood_cv(int v, array* W, array *a,
			    array* aW, struct mrfe_data *data) {

    int N_W, N_v_W;
    double p_hat = 0.0;
    double alpha = 0.000001, pmin = 0.0000001;
    count_in_sample(v, W, a, aW, data->out_fold,
		    data->out_fold_size, &N_W, &N_v_W);
    if (N_W == 0)
	p_hat = 1.0 / data->A_size;
    else 
	p_hat = (double) N_v_W / N_W;
    p_hat = (1.0 - alpha) * p_hat + (alpha * pmin);
    /* in of fold */
    count_in_sample(v, W, a, aW, data->fold,
		    data->fold_size, &N_W, &N_v_W);
    return (double) N_v_W * log(p_hat);
}

static double penalized_factor(int W, struct mrfe_data *data) {
    return (data->c * pow(data->A_size, W) *
	    (log(data->sample_size) / log(data->A_size)));
}

static double likelihood(int v, array* W, array* a, array* aW,
			struct mrfe_data *data) {

    int N_W, N_v_W;
    double p_hat = 0.0;
    count_in_sample(v, W, a, aW, data->sample,
		    data->sample_size, &N_W, &N_v_W);
    if (N_W == 0)
	p_hat = 1.0 / data->A_size;
    else 
	p_hat = (double) N_v_W / N_W;
    if (N_v_W == 0)
	return 0.0;    
    return (double) N_v_W * log(p_hat);
}

static double L_vertex(int v, array* W, struct mrfe_data *data) {
    int m = W->size;
    double L_value = 0.0;
    product* p = product_init(data->A, 1);
    while (product_has_next(p)) {
        array* a = product_next(p);
        product* pW = product_init(data->A, m);
        while (product_has_next(pW)) {
            array* aW = product_next(pW);
            L_value += likelihood(v, W, a, aW, data);
            array_destroy(aW);            
        }
        array_destroy(a);
        product_finish(pW);
    }
    product_finish(p);
    return L_value - penalized_factor(W->size, data);
}
static array* estimate_neighborhood(int v, struct mrfe_data *data) {
    double best_value = -1 * INF;
    array* best_neighborhood = array_zeros(0);    
    array* V = array_erase(data->V, v);    
    for (int i = 0; i <= data->max_neigh; i++) {
        combination* c = combination_init(V, i);
        while (combination_has_next(c)) {
            array* W = combination_next(c);
            double PL_value = L_vertex(v, W, data);
            if (PL_value > best_value) {
                best_value = PL_value;
                array_destroy(best_neighborhood);
                best_neighborhood = W;
            } else 
                array_destroy(W);
        }
        combination_finish(c);
    }
    array_destroy(V);
    return best_neighborhood;
}

static void estimate_graph(struct mrfe_data *data) {
    array** ne_hat = array_matrix(data->V_size);
    #pragma omp parallel for
    for (int v = 0; v < data->V_size; v++)
        ne_hat[v] = estimate_neighborhood(v, data);
    data->adj = ne_hat;
}

static void cv_blocs(struct mrfe_data *data) {
    data->fold_bloc = array_zeros(data->k + 1);
    int q = data->sample_size / data->k;
    int r = data->sample_size % data->k;
    data->fold_bloc->array[0] = -1;
    for (int i = 1; i <= r; i++) {
        data->fold_bloc->array[i] = data->fold_bloc->array[i-1] + q + 1;
    }
    for (int i = r+1; i <= data->k; i++)
        data->fold_bloc->array[i] = data->fold_bloc->array[i-1] + q;
}

static void get_fold(int k, struct mrfe_data *data) {
    int a = data->fold_bloc->array[k];
    int b = data->fold_bloc->array[k-1];
    data->fold_size = a - b;
    for (int i = b+1, j = 0; i <= a; i++) {
	for (int k = 0; k < data->V_size; k++)
	    data->fold[j][k] = data->sample[i][k];
	j++;
    }
}

static void get_out_fold(int k, struct mrfe_data *data) {
    int a = data->fold_bloc->array[k];
    int b = data->fold_bloc->array[k-1];
    data->out_fold_size = data->sample_size - (a - b);
    for (int i = 0, j = 0; i < data->sample_size; i++) {
	if (!(i > b && i <= a)) {
	    for (k = 0; k < data->V_size; k++)
		data->out_fold[j][k] = data->sample[i][k];
	    j++;
	}
    }
}

static void sample_cv(struct mrfe_data *data) {
    matrixINTcpy(data->sample, data->out_fold,
		 data->out_fold_size, data->V_size);
    /* data->sample = data->out_fold; */    
    data->sample_size = data->out_fold_size;    
}

static void un_sample_cv(int **tmp, struct mrfe_data *data) {
    data->sample_size = data->out_fold_size + data->fold_size;
    matrixINTcpy(data->sample, tmp, data->sample_size, data->V_size);
}

static double L_vertex_cv(int v, struct mrfe_data *data) {
    double value = 0.0;
    product *p = product_init(data->A, 1);
    array *W = data->adj[v];
    while (product_has_next(p)) {
	array *a = product_next(p);
	product *pW = product_init(data->A, W->size);
	while(product_has_next(pW)) {
	    array *aW = product_next(pW);
	    value += likelihood_cv(v, W, a, aW, data);
	    array_destroy(aW);
	}
	product_finish(pW);
	array_destroy(a);
    }
    array_destroy(W);
    product_finish(p);
    return value;    
}

static double cv_value(struct mrfe_data *data) {
    double value = 0.0;    
    int **tmp = matrixINT(data->sample_size, data->V_size);
    matrixINTcpy(tmp, data->sample, data->sample_size, data->V_size);
    for (int i = 1; i <= data->k; i++) {
	get_fold(i, data);
	get_out_fold(i, data);
    	sample_cv(data); 
	estimate_graph(data); /* Estimate graph with K - 1 folds */
    	for (int v = 0; v < data->V_size; v++) {
	    value += L_vertex_cv(v, data);
    	}
    	un_sample_cv(tmp, data);
    }    
    free_matrixINT(tmp, data->sample_size);
    return value / data->k;
}

static void cross_validation(struct mrfe_data *data) {
    double best_value = -INF, best_c = 0.0;
    cv_blocs(data);
    for (double c = data->c_min ; c <= data->c_max; c += data->c_interval) {
    	data->c = c;
    	double value = cv_value(data);
    	if (value > best_value) {
    	    best_value = value;
    	    best_c = c;
    	}
    }
    data->c = best_c;
}

void mrfe(struct mrfe_data *data) {
    if(data->cv_enable)
	cross_validation(data);
    estimate_graph(data);
}
