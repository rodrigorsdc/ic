#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "product.h"
#include "array.h"
#include "pml_graph.h"
#include "util.h"

#define max_edges(V) ((V*(V-1)) / 2)

static void edges_offset(int **adj, int V, array *e) {
    int k = 0;
    for (int i = 0; i < V; i++)
	for (int j = i+1; j < V; j++) {
	    adj[i][j] = e->array[k++];
	    adj[j][i] = adj[i][j];
	}
}

static double likelihood(int v, array* W, array* a,
			 array* aW, struct pml_graph_data *data) {
    int N_W = 0, N_v_W = 0;
    double p_hat = 0.0;
    int m = W->size;
    array* x_W = array_zeros(m);    
    for (int i = 0; i < data->sample_size; i++) {
        for (int j = 0; j < m; j++)
	    x_W->array[j] = data->sample[i][W->array[j]];
        if (array_equals(x_W, aW)) {
            if (data->sample[i][v] == a->array[0])
		N_v_W++;
            N_W++;
        }
    }
    array_destroy(x_W);
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

static int ***gen_all_adj(struct pml_graph_data *data) {
    int ***ans = (int ***) malloc(data->num_graphs * sizeof(int **));
    for (int i = 0; i < data->num_graphs; i++)
	ans[i] = matrix_int(data->V, data->V);
    array *a = array_arange(2);
    product *p = product_init(a, data->max_edges);
    int i = 0;
    while(product_has_next(p)) {
	array *g = product_next(p);
	edges_offset(ans[i++], data->V, g);
    }
    return ans;
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
    data->adj = best_edges;
}

void pml_graph(struct pml_graph_data *data) {
    data->adj = matrix_int(data->V, data->V);
    data->max_edges = max_edges(data->V);
    data->A = array_arange(data->A_size);
    data->num_graphs = 1 << data->max_edges;
    estimate_graph(data);
}

void test(struct pml_graph_data *data) {
    data->V = 5;
    data->A_size = 2;
    data->c = 1.0;
    data->sample_size = 1000;
    data->sample = (int**) malloc(1000 * sizeof(int*));
    for (int i = 0; i < 1000; i++)
	data->sample[i] = (int*) malloc(data->V * sizeof(int));
}

int main() {
    struct pml_graph_data *data = (struct pml_graph_data *)
	malloc(sizeof(struct pml_graph_data));
    test(data);
    FILE *f = fopen("sample3", "r");
    if (f == NULL) printf("deu ruim\n");
    for (int i = 0; i < data->sample_size; i++)
	for (int j = 0; j < data->V; j++) {
	    fscanf(f, "%d", &data->sample[i][j]);
	}
    pml_graph(data);
    for (int i = 0; i < data->V; i++) {
	for (int j = 0; j < data->V; j++)
	    printf("%d ", data->adj[i][j]);
	printf("\n");
    }
    return 0;
}

