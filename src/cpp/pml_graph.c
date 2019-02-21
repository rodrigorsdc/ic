#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "product.h"
#include "array.h"
#include "pml_graph.h"
#include "util.h"

#define max_edges(V) ((V*(V-1)) / 2)

static void print_adj(struct pml_graph_data *data) {
    for(int i = 0; i < data->V; i++) {
	for (int j = 0; j < data->V; j++)
	    printf("%d ", data->adj[i][j]);
	printf("\n");
    }
}

static void print_sample(struct pml_graph_data *data) {
    for (int i = 0; i < data->sample_size; i++) {
	for (int j = 0; j < data->V; j++)
	    printf("%d ", data->sample[i][j]);
	printf("\n");
    }
   
}

static void edges_offset(int **adj, int V, array *e) {
    int k = 0;
    for (int i = 0; i < V; i++)
	for (int j = i+1; j < V; j++) {
	    adj[i][j] = e->array[k++];
	    adj[j][i] = adj[i][j];
	}
}

/* Count operators N(W) and N(v, W) */
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
			    array* aW, struct pml_graph_data *data) {

    int N_W, N_v_W;
    double p_hat = 0.0;
    double alpha = 0.000001, pmin = 0.000001;
    count_in_sample(v, W, a, aW, data->fold,
		    data->fold_size, &N_W, &N_v_W);
    if (N_W == 0)
	p_hat = 1.0 / data->A_size;
    else 
	p_hat = (double) N_v_W / N_W;
    p_hat = (1.0 - alpha) * p_hat + alpha * pmin;
    /* Out of fold */
    count_in_sample(v, W, a, aW, data->out_fold,
		    data->out_fold_size, &N_W, &N_v_W);
    return (double) N_v_W * log(p_hat);
}

static double likelihood(int v, array* W, array* a,
			 array* aW, struct pml_graph_data *data) {
    int N_W = 0, N_v_W = 0;
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

static int count_one(int *v, int n) {
    int ans = 0;
    for (int i = 0; i < n; i++)
	if (v[i] == 1)
	    ans++;
    return ans;
}

static array *adj_to_list(int *W, int n) {
    int num_neigh = count_one(W, n);
    array *a = array_zeros(num_neigh);
    for (int i = 0, j = 0; i < n; i++) {
	if (W[i] == 1)
	    a->array[j++] = i;
    }
    return a;
}

/* Return the likelihood of a vertex given its
 * neighborhood */
static double L_vertex(int v, struct pml_graph_data *data, int **adj) {
    double L_value = 0.0;
    array *W = adj_to_list(adj[v], data->V);
    int m = W->size;
    product *p = product_init(data->A, 1);
    while(product_has_next(p)) {
	array *a = product_next(p);
	product *pW = product_init(data->A, m);
	while(product_has_next(pW)) {
	    array *aW = product_next(pW);
	    L_value += likelihood(v, W, a, aW, data);
	    array_destroy(aW);
	}
	array_destroy(a);
	product_finish(pW);	    
    }
    array_destroy(W);
    product_finish(p);
    return L_value;
}

static int get_num_edges(int **adj, int V) {
    int ans = 0;
    for (int i = 0; i < V; i++)
	for (int j = 0; j < V; j++)
	    if (adj[i][j] == 1)
		ans++;
    return ans / 2;
}

static double penalized_factor(struct pml_graph_data *data,
			       int **adj) {
    int num_edges = get_num_edges(adj, data->V);
    return data->c * pow(data->A_size, num_edges) *
	(log(data->sample_size) / log(data->A_size));
}

/* Return the penalized likelihood of a given graph */
static double estimate_PL(struct pml_graph_data *data,
			  int **adj) {
    double value = 0.0;
    for (int v = 0; v < data->V; v++)
	value += L_vertex(v, data, adj);
    value -= penalized_factor(data, adj);	
    return value;    
} 

/* All possible graph with V vertices */
static int ***gen_all_adj(struct pml_graph_data *data) {
    int ***ans = (int ***) malloc(data->num_graphs * sizeof(int **));
    for (int i = 0; i < data->num_graphs; i++)
	ans[i] = matrixINT(data->V, data->V);
    array *a = array_arange(2);
    product *p = product_init(a, data->max_edges);
    int i = 0;
    while(product_has_next(p)) {
	array *g = product_next(p);
	edges_offset(ans[i++], data->V, g);
    }
    return ans;
}

static void free_all_adj(int ***all_adj, struct pml_graph_data *data) {
    for (int i = 0 ; i < data->num_graphs; i++)
	free_matrixINT(all_adj[i], data->V);    
}

static void estimate_graph(struct pml_graph_data *data) {
    double best_value = -INF;
    int **best_edges = NULL;
    array *a = array_arange(2);
    int ***all_adj = gen_all_adj(data);
    product *p = product_init(a, data->max_edges);
    #pragma omp parallel for
    for (int i = 0; i < data->num_graphs; i++) {
	double value = estimate_PL(data, all_adj[i]);
	#pragma omp critical
	if (value > best_value) {
	    best_value = value;
	    best_edges = all_adj[i];
	}
    }
    array_destroy(a);
    product_finish(p);
    matrixINTcpy(data->adj, best_edges, data->V, data->V);
    free_all_adj(all_adj, data);
}

static void cv_blocs(struct pml_graph_data *data) {
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

static void get_fold(int k, struct pml_graph_data *data) {
    int a = data->fold_bloc->array[k];
    int b = data->fold_bloc->array[k-1];
    data->fold_size = a - b;
    for (int i = b+1, j = 0; i <= a; i++)
	data->fold[j++] = data->sample[i];
}

static void get_out_fold(int k, struct pml_graph_data *data) {
    int a = data->fold_bloc->array[k];
    int b = data->fold_bloc->array[k-1];
    data->out_fold_size = data->sample_size - (a - b);
    for (int i = 0, j = 0; i < data->sample_size; i++) {
	if (!(i > b && i <= a))
	    data->out_fold[j++] = data->sample[i];
    }
}

static void sample_cv(int **tmp, struct pml_graph_data *data) {
    matrixINTcpy(data->sample, data->out_fold, data->out_fold_size, data->V);
    /* data->sample = data->out_fold; */    
    data->sample_size = data->out_fold_size;    
}

static void un_sample_cv(int **tmp, struct pml_graph_data *data) {
    matrixINTcpy(data->sample, tmp, data->sample_size, data->V);
    data->sample_size = data->out_fold_size + data->fold_size;
}

static double L_vertex_cv(int v, struct pml_graph_data *data) {
    double value = 0.0;
    product *p = product_init(data->A, 1);
    array *W = adj_to_list(data->adj[v], data->V);
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

static double cv_value(struct pml_graph_data *data) {
    double value = 0.0;    
    int **tmp = matrixINT(data->sample_size, data->V);
    matrixINTcpy(tmp, data->sample, data->sample_size, data->V);
    for (int i = 1; i <= data->k; i++) {
	get_fold(i, data);
	get_out_fold(i, data);
    	sample_cv(tmp, data); 
	estimate_graph(data); /* Estimate graph with K - 1 folds */
    	for (int v = 0; v < data->V; v++) {
	    value += L_vertex_cv(v, data);
    	}
    	un_sample_cv(tmp, data);
    }    
    free_matrixINT(tmp, data->sample_size);
    return value / data->k;
}

static void setUp(struct pml_graph_data *data) {
    data->adj = matrixINT(data->V, data->V);
    data->max_edges = max_edges(data->V);
    data->A = array_arange(data->A_size);
    data->num_graphs = 1 << data->max_edges;
}

static void cross_validation(struct pml_graph_data *data) {
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

void test(struct pml_graph_data *data) {
    data->V = 5;
    data->A_size = 2;
    data->c = 1.0;
    data->k = 2;
    data->sample_size = 5000;
    data->sample = (int**) malloc(data->sample_size * sizeof(int*));
    for (int i = 0; i < data->sample_size; i++)
	data->sample[i] = (int*) malloc(data->V * sizeof(int));
    data->fold = matrixINT(data->sample_size, data->V);
    data->out_fold = matrixINT(data->sample_size, data->V);
    data->cv_enable = 1;
    data->c_min = 0.1;
    data->c_max = 2.0;
    data->c_interval = 0.05;	
}

int main() {
    struct pml_graph_data *data = (struct pml_graph_data *)
	malloc(sizeof(struct pml_graph_data));
    test(data);
    setUp(data);
    FILE *f = fopen("sample3", "r");
    if (f == NULL) printf("deu ruim\n");
    for (int i = 0; i < data->sample_size; i++)
	for (int j = 0; j < data->V; j++) {
	    fscanf(f, "%d", &data->sample[i][j]);
	}
    cross_validation(data);
    printf("c: %lf\n", data->c);
    estimate_graph(data);
    print_adj(data);
    return 0;
}

