<<<<<<< HEAD
=======
/* Código para o calculo da validação cruzada do algoritmo
   de máxima verossimilhança */

>>>>>>> a756c6795a95e1038195346aa4cd75f8a2347186
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <omp.h>
#include <sys/wait.h>
#include <string>
#include <stdarg.h>
#include "array.h"
#include "util.h"
#include "product.h"

using namespace std;

typedef struct {
    array* V;
    array* A;
    int V_size;
    int A_size;
    int n;
    double c_min;
    double c_max;
    double c_inter;
    int** sample;
    int max_neig;
    array* blocs;
    int k;
}config_cv;

static config_cv* con;

static void pml_cv(const char* in);
static void config_init(const char* in);
static config_cv* malloc_config(unsigned int n);
static void config_read_V(FILE *f);
static void config_read_A(FILE *f);
static void config_read_max_neig(FILE *f);
static void config_read_sample(FILE *f);
static void config_read_c(FILE *f);
static void config_read_k(FILE *f);
static double cv_value(double c);
static void insert_sample(FILE *f, int k);
static void cv_blocs();
static void execute_pml();
static void create_input_pml(int k, double c);
static array** neighbourhoods(const char* out);
static double log_likelihood(array** ne_hat, int k);
static double get_p_hat(int v, array* W, array* a, array* aW, int k);
static void clean_tmp_dir();



static void pml_cv(const char* in) {
    config_init(in);
    double best_value = -1.0d  * INF;
    double best_c = con->c_min;
    FILE *serie = fopen("serie.csv", "w");
    fprintf(serie, "CV(c),c\n");
    for (double c = con->c_min; c <= con->c_max; c += con->c_inter) {
        double value = cv_value(c);
        if (value > best_value) {
            best_c = c;
            best_value = value;
        }
        fprintf(serie, "%lf,%lf\n", value, c);
    }
    printf("Best_value = %lf\n", best_value);
    printf("best_c = %lf\n", best_c);
    
    create_input_pml(0, best_c);
    fclose(serie);
}
static void cv_blocs() {
    con->blocs = array_zeros(con->k + 1);
    int q = con->n / con->k;
    int r = con->n % con->k;
    con->blocs->array[0] = -1;
    for (int i = 1; i <= r; i++) {
        con->blocs->array[i] = con->blocs->array[i-1] + q + 1;
    }
    for (int i = r+1; i <= con->k; i++)
        con->blocs->array[i] = con->blocs->array[i-1] + q;
}

static void execute_pml() {
    if (fork() == 0)
        execlp("./bin/pml", "./bin/pml", "tmp/in", "tmp/dummy",
               "tmp/ncons", "-cv", (char*) NULL);
    wait(NULL);
}
static double cv_value(double c) {
    double value = 0.0d;
    for (int k = 1; k <= con->k; k++) {
        create_input_pml(k, c);
        execute_pml();
        array** ne_hat = neighbourhoods("tmp/ncons");
        value += log_likelihood(ne_hat, k);
    }
    return value / (double) con->k;
}

static array** neighbourhoods(const char* out) {
    array** ne_hat = array_matrix(con->V_size);
    FILE* f = fopen(out, "r");
    for (int i = 0; i < con->V_size; i++) {
        int neigh_size;
        fscanf(f, "%d", &neigh_size);
        ne_hat[i] = array_zeros(neigh_size);
        for (int j = 0; j < neigh_size; j++)
            fscanf(f, "%d", &ne_hat[i]->array[j]);
    }
    fclose(f);
    return ne_hat;
}

static double log_likelihood(array** ne_hat, int k) {
    double value = 0.0d;
    for (int v = 0; v < con->V_size; v++) {
        product* p_ne = product_init(con->A, ne_hat[v]->size);
        while (product_has_next(p_ne)) {
            array* aW = product_next(p_ne);
            product* p = product_init(con->A, 1);
            while (product_has_next(p)) {
                array* a = product_next(p);
                value += get_p_hat(v, ne_hat[v], a, aW, k);
                array_destroy(a);
            }
            product_finish(p);
            array_destroy(aW);
        }
        product_finish(p_ne);
    }
    return value;
}

static double get_p_hat(int v, array* W, array* a, array* aW, int k) {
    int N_W = 0, N_v_W = 0, N_v_W_cv = 0;
    double p_hat = 0.0;
    int m = W->size;
    double alpha = 0.000001;
    double pmin = 0.0000001;        
    array* x_W = array_zeros(m);
    for (int i = 0; i < con->n; i++) {
        for (int j = 0; j < m; j++) x_W->array[j] = con->sample[i][W->array[j]];
        if (i >= con->blocs->array[k-1] + 1 && i <= con->blocs->array[k]) {
            if (array_equals(x_W, aW) && con->sample[i][v] == a->array[0])
                N_v_W_cv++;
                
        } else {
            if (array_equals(x_W, aW)) {
                if (con->sample[i][v] == a->array[0]) N_v_W++;
                N_W++;
            }            
        }
    }
    array_destroy(x_W);
    if (N_W == 0) p_hat = 1.0 / (double) con->A_size;
    else          p_hat = (double) N_v_W / (double) N_W;
    p_hat = (1.0 - alpha) * p_hat + (alpha * pmin);
    return (double) N_v_W_cv * log(p_hat);
}

static void create_input_pml(int k, double c) {
    FILE* f;
    if (k == 0) f = fopen("in", "w");
    else f = fopen("tmp/in", "w");
    
    fprintf(f, "%d\n\n", con->V_size);
    fprintf(f, "%d\n\n", con->A_size);
    fprintf(f, "%lf\n\n", c);
    fprintf(f, "%d\n\n", con->max_neig);
    insert_sample(f, k);
    fclose(f);
}

static void insert_sample(FILE* f, int k) {
    int n = con->n;
    if (k > 0) n = n - (con->blocs->array[k] - con->blocs->array[k-1]);
    fprintf(f, "%d\n\n", n);
    for (int i = 0; i < con->n; i++) {
        if (k == 0 || (i <= con->blocs->array[k-1] ||
                       i > con->blocs->array[k])) {
            for (int j = 0; j < con->V_size; j++)
                fprintf(f, "%d ", con->sample[i][j]);
            fprintf(f, "\n");
        }        
    }
}

static void config_read_c(FILE *f) {
    fscanf(f, "%lf", &con->c_min);
    fscanf(f, "%lf", &con->c_max);
    fscanf(f, "%lf", &con->c_inter);
}

static void config_read_k(FILE *f) {
    fscanf(f, "%d", &con->k);
}


static void config_read_V(FILE *f) {
    fscanf(f, "%d", &con->V_size);
    con->V = array_arange(con->V_size);    
}

static void config_read_A(FILE *f) {
    fscanf(f, "%d", &con->A_size);
    con->A = array_arange(con->A_size);        
}

static void config_read_max_neig(FILE *f) {
    fscanf(f, "%d", &con->max_neig);
}    

static void config_read_sample(FILE *f) {
    fscanf(f, "%d", &con->n);
    con->sample = (int**) malloc(con->n * sizeof(int*));
    for (int i = 0; i < con->n; i++) {
        con->sample[i] = (int*)malloc(con->V_size * sizeof(int));
        for (int j = 0; j < con->V_size; j++)
            fscanf(f, "%d", &con->sample[i][j]);
    }
}

static void config_init(const char* in) {
    FILE* f = fopen(in, "r");
    con = malloc_config(1);
    config_read_c(f);
    config_read_k(f);
    config_read_V(f);
    config_read_A(f);
    config_read_max_neig(f);
    config_read_sample(f);
    cv_blocs();
    fclose(f);            
}

static config_cv* malloc_config(unsigned int n) {
    config_cv* ptr = (config_cv*) malloc(n * sizeof(config_cv));
    if (ptr == NULL) {
        printf("malloc error!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static void clean_tmp_dir(){
    if (fork() == 0)
        execlp("rm", "rm", "tmp/in", "tmp/ncons", (char*) NULL);
    wait(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Falta argumentos!\n");
        exit(EXIT_FAILURE);
    }
    pml_cv(argv[1]);
    clean_tmp_dir();
    return 0;
}
