#include "array.h"
#include "combination.h"
#include "product.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include "simple.h"



typedef struct{
    double delta;
    double epsilon;
    int n;
    array *V;
    array *A;
    int V_size;
    int A_size;
    int **sample;
    int max_neig;    
}config;

static config* con;


static void config_init(const char* in);
static void config_destroy();
static void config_read_V(FILE* f);
static void config_read_A(FILE* f);
static void config_read_delta(FILE* f);
static void config_read_epsilon(FILE* f);
static void config_read_max_neig(FILE *f);
static void config_read_sample(FILE *f);
static array** estimate_graph();
static array* estimate_neighborhood(int v);
static int check_neighborhood(array* U, int v);
static double correlation_test(int v, array* U, int w, array* AvUw);
static void save_graph(const char* out, array** ne_hat);
static config* malloc_config(int n);

void simple(const char* in, const char* out) {
    config_init(in);
    array** ne_hat = estimate_graph();
    save_graph(out, ne_hat);
    array_matrix_destroy(ne_hat, con->V_size);
    config_destroy();
}

static array** estimate_graph() {
    array** ne_hat = array_matrix(con->V_size);
    for (int v = 0; v < con->V_size; v++)
        ne_hat[v] = estimate_neighborhood(v);

    return ne_hat;
}

static array* estimate_neighborhood(int v) {
    array* V = array_erase(con->V, v);
    for (int i = 0; i < con->max_neig; i++) {
        combination* c = combination_init(V, i);
        while (combination_has_next(c)) {
            array* U = combination_next(c);
            if (check_neighborhood(U, v)) {
                combination_finish(c);
                array_destroy(V);
                array_print(U);
                return U;
            }
            array_destroy(U);
        }
        combination_finish(c);
    }
    printf("Não era para ter chegado aqui!\n");
    exit(EXIT_FAILURE);
}

static int check_neighborhood(array *U, int v) {
    double best_value = -1.0;
    array* a_w = array_erase(array_remove(con->V, U), v);
    for (int i = 0; i < a_w->size; i++) {
        int w = a_w->array[i];
        // printf("w = %d\n", w);
        product* pUvww = product_init(con->A, U->size + 3);
        while(product_has_next(pUvww)) {
            array* AUvww = product_next(pUvww);
            double correlation_value = correlation_test(v, U, w, AUvww);
            if (correlation_value > best_value)
                best_value = correlation_value;
            array_destroy(AUvww);            
        }
        product_finish(pUvww);
    }
    array_destroy(a_w);
    if(best_value == -1.0) return 0;
    // printf("Best_value: %lf\n", best_value);
    // if (best_value == 0.0) return 0;
    if (best_value < con->epsilon / 2) return 1;
    return 0;
}

static double correlation_test(int v, array* U, int w, array* AUvww) {
    int NUw1 = 0, NUw2 = 0;
    int NvUw1 = 0, NvUw2 = 0;
    int av = AUvww->array[U->size];
    int aw1 = AUvww->array[U->size + 1];
    int aw2 = AUvww->array[U->size + 2];        
    array* XU = array_zeros(U->size);
    // printf("LL: %d", XU->size);
    // array_print(AUvww);
    for (int i = 0; i < con->n; i++) {
        for (int j = 0; j < U->size; j++)
            XU->array[j] = con->sample[i][U->array[j]];
        // array* t = array_sub(AUvww, U->size - 1);
        // printf("çç: %d\n", t->size);
        array *s = array_sub(AUvww, U->size - 1);
        if (array_equals(XU, s)) {
            // printf("oioiio\n");
            if (aw1 == con->sample[i][w]) {
                if (av == con->sample[i][v]) NvUw1++;
                NUw1++;
            }
            if (aw2 == con->sample[i][w]) {
                if (av == con->sample[i][v]) NvUw2++;
                NUw2++;
            }
        }
        array_destroy(s);
    }
    array_destroy(XU);
    // printf("%d %d\n", NUw1, NUw2);
    double PNUw1 = (double) NUw1 / con->n;
    double PNUw2 = (double) NUw2 / con->n;
    if (PNUw1 <= con->delta / 2.0 || PNUw2 <= con->delta / 2.0) return -1.0;
    return fabs((double) NvUw1 / NUw1 - (double) NvUw2 / NUw2);
    return 0.0;
}

static void save_graph(const char* out, array** ne_hat) {
    FILE* f = fopen(out, "w");
    for (int i = 0; i < con->V_size; i++) {
        for (int j = 0; j < ne_hat[i]->size; j++)
            fprintf(f, "%d ", ne_hat[i]->array[j]);
        fprintf(f, "\n");                
    }
    fclose(f);
}

static void config_destroy() {
    array_destroy(con->V);
    array_destroy(con->A);
    for (int i = 0; i < con->V_size; i++) {
        free(con->sample[i]);
        con->sample[i] = NULL;            
    }
    free(con->sample);
    con->sample = NULL;
    free(con);
    con = NULL;
}



static void config_read_V(FILE* f) {
    fscanf(f, "%d", &con->V_size);
    con->V = array_arange(con->V_size);
}

static void config_read_A(FILE* f) {
    fscanf(f, "%d", &con->A_size);
    con->A = array_arange(con->A_size);
}

static void config_read_delta(FILE *f) {
    fscanf(f, "%lf", &con->delta);    
}

static void config_read_epsilon(FILE* f) {
    fscanf(f, "%lf", &con->epsilon);
}

static void config_read_max_neig(FILE* f) {
    fscanf(f, "%d", &con->max_neig);
}

static void config_read_sample(FILE* f) {
    fscanf(f, "%d", &con->n);
    con->sample = (int**) malloc(con->n * sizeof(int*));
    for (int i = 0; i < con->n; i++) {
        con->sample[i] = (int*) malloc(con->V_size * sizeof(int));
        for (int j = 0; j < con->V_size; j++)
            fscanf(f, "%d", &con->sample[i][j]);
    }
}

static config* malloc_config(int n) {
    config* ptr = (config*) malloc(n * sizeof(config));
    if (ptr == NULL) {
        printf("malloc devolveu NULL\n");
        exit(EXIT_FAILURE);
    }
    return ptr;    
}

static void config_init(const char* in) {
    FILE *f = fopen(in, "r");
    con = malloc_config(1);
    config_read_V(f);
    config_read_A(f);
    config_read_delta(f);
    config_read_epsilon(f);
    config_read_max_neig(f);
    config_read_sample(f);
}
