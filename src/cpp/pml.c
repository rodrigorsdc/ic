#include <stdio.h>
#include <vector>
#include <set>
#include <limits>
#include <math.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

using namespace std;
#include "product.h"
#include "combination.h"
#include "pml.h"
#include "array.h"
#include "util.h"

typedef struct{
    array* V;
    array* A;
    double c;
    int V_size;
    int A_size;
    int n;
    int** sample;
    int max_neig;
}config;

typedef struct{
    int a;
    int b;
}pair;

static config* con;
static int cv = 0;

static array* estimate_neighborhood(int v);
static double estimate_PL(int v, array* W);
static double penalized_factor(int W);
static double get_p_hat(int v, array* W, array* a, array* aW);
static array** estimate_graph();
static void save_graph(const char* cons_out, const char* ncons_out, array** ne_hat);
static void config_read_V(FILE* f);
static void config_read_A(FILE* f);
static void config_read_c(FILE* f);
static void config_read_sample(FILE* f);
static void config_read_max_neig(FILE* f);
static config* malloc_config(unsigned int n);
static void config_destroy();
static void config_init(const char *in);
static void save_graph_cv(const char* out, array** ne_hat);

void pml(const char* in, const char* cons_out, const char* ncons_out) {
    config_init(in);
    array** ne_hat = estimate_graph();
    if (cv == 1) save_graph_cv(ncons_out, ne_hat);
    else         save_graph(cons_out, ncons_out, ne_hat);
    array_matrix_destroy(ne_hat, con->V_size);
    config_destroy();
}

/* Função que devolve a vizinhança
   estimada de cada vértice */
static array** estimate_graph() {
    array** ne_hat = array_matrix(con->V_size);
    #pragma omp parallel for
    for (int v = 0; v < con->V_size; v++)
        ne_hat[v] = estimate_neighborhood(v);
    return ne_hat;
}

/* Estima cada vizinhança do vértice "v". */
static array* estimate_neighborhood(int v) {
    double best_value = -1 * INF;
    array* best_neighborhood = array_zeros(0);
    array* V = array_erase(con->V, v);
    for (int i = 0; i <= con->max_neig; i++) {
        combination* c = combination_init(V, i);
        while (combination_has_next(c)) {
            array* W = combination_next(c);
            double PL_value = estimate_PL(v, W);
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

/* Devolve o valor de máxima verossimilnha penalizada do vértice v
   dado W. */
static double estimate_PL(int v, array* W) {
    int m = W->size;
    double L_value = 0.0;
    product* p = product_init(con->A, 1);
    while (product_has_next(p)) {
        array* a = product_next(p);
        product* pW = product_init(con->A, m);
        while (product_has_next(pW)) {
            array* aW = product_next(pW);
            L_value += get_p_hat(v, W, a, aW);
            array_destroy(aW);            
        }
        array_destroy(a);
        product_finish(pW);
    }
    product_finish(p);
    return L_value - penalized_factor(W->size);
}

/* Devolve o valor de penalização de um conjunto
   de tamanho W de vizinhos. */
static double penalized_factor(int W) {
    return (con->c * pow(con->A_size, W) * (log(con->n) / log(con->A_size)));
}

/* Devolve o valor de p_hat(v = a | W = aW) */
static double get_p_hat(int v, array* W, array* a, array* aW) {
    int N_W = 0, N_v_W = 0;
    double p_hat = 0.0;
    int m = W->size;
    array* x_W = array_zeros(m);    
    for (int i = 0; i < con->n; i++) {
        for (int j = 0; j < m; j++) x_W->array[j] = con->sample[i][W->array[j]];
        if (array_equals(x_W, aW)) {
            if (con->sample[i][v] == a->array[0]) N_v_W++;
            N_W++;
        }
    }
    array_destroy(x_W);
    if (N_W == 0) p_hat = 1.0 / con->A_size;
    else          p_hat = (double) N_v_W / N_W;
    if (N_v_W == 0) return 0.0;
    
    return (double) N_v_W * log(p_hat);

}

/* Grava no arquivo "out" as informação de cada vizinhança estimada da
seguinte forma: na i-ésima linha haverá um inteiro "s" que indicará
quantos vizinhos o vértices "i" tem. Os próximos "s" inteiros contém
os vértices da vizinhança. */
static void save_graph_cv(const char* out, array** ne_hat) {
    FILE* f_cv = fopen(out, "w");
    for (int i = 0; i < con->V_size; i++) {
        fprintf(f_cv, "%d ", ne_hat[i]->size);
        for (int j = 0; j < ne_hat[i]->size; j++) {
            fprintf(f_cv, "%d ", ne_hat[i]->array[j]);
        }
        fprintf(f_cv, "\n");
    }
    fclose(f_cv);
}

/* Grava nos arquivos "ncons_out" e "cons_out" os grafos na abordagem
não-conservativa e conservativa, respectivamente.  As vizinhanças de
cada nó se encontra no parâmetro "ne_hat". */
static void save_graph(const char* cons_out, const char* ncons_out,
                       array** ne_hat) {
    FILE* f_cons = fopen(cons_out, "w");
    FILE* f_ncons = fopen(ncons_out, "w");
    fprintf(f_cons, "%d\n", con->V_size);
    fprintf(f_ncons, "%d\n", con->V_size);
    for (int i = 0; i < con->V_size; i++) {
        for (int j = 0; j < ne_hat[i]->size; j++) {
            fprintf(f_ncons, "%d %d\n", i, ne_hat[i]->array[j]);
            if(array_contains(ne_hat[ne_hat[i]->array[j]], i))
                fprintf(f_cons, "%d %d\n", i, ne_hat[i]->array[j]);
        }
    }    
    fclose(f_cons);
    fclose(f_ncons);
}

static void config_read_V(FILE* f) {
    int V_size;
    fscanf(f, "%d", &V_size);
    con->V = array_arange(V_size);
    con->V_size = V_size;
}

static void config_read_c(FILE* f) {
    double c;
    fscanf(f, "%lf", &c);
    con->c = c;
}

static void config_read_sample(FILE* f) {
    int n;
    fscanf(f, "%d", &n);
    con->sample = (int**) malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        con->sample[i] = (int*) malloc(con->V_size * sizeof(int));
        for (int j = 0; j < con->V_size; j++)
            fscanf(f, "%d", &con->sample[i][j]);
    }
    con->n = n;
}

static void config_read_A(FILE* f) {
    int A_size;
    fscanf(f, "%d", &A_size);
    con->A = array_arange(A_size);
    con->A_size = A_size;
}

static void config_read_max_neig(FILE* f) {
    int max_neig;
    fscanf(f, "%d", &max_neig);
    con->max_neig = max_neig;
}
         
static void config_init(const char *in) {
    con = malloc_config(1);
    FILE* f = fopen(in, "r");
    config_read_V(f);
    config_read_A(f);
    config_read_c(f);
    config_read_max_neig(f);
    config_read_sample(f);
    fclose(f);    
}

static void config_destroy() {
    array_destroy(con->V);
    array_destroy(con->A);
    for (int i = 0; i < con->n; i++) {
        free(con->sample[i]);
        con->sample[i] = NULL;
    }
    free(con->sample);
    con->sample = NULL;
    free(con);
    con = NULL;        
}

static config* malloc_config(unsigned int n) {
    config* ptr = (config*) malloc(n * sizeof(config));
    if (ptr == NULL) {
        printf("malloc devolveu NULL!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 5) {
        printf("Falta argumentos!");
        return 1;
    } else if (argc == 5){
        cv = 1;        
    }
    pml(argv[1], argv[2], argv[3]);
    return 0;    
}

