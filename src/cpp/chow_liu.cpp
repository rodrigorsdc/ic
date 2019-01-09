#include "array.h"
#include "combination.h"
#include "product.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


typedef struct{
    int v;
    int w;
    double weight;    
}edge;

typedef struct{
    edge *edges;
    int m;
}edges;

typedef struct{
    array* V;
    array* A;
    int V_size;
    int A_size;
    int n;
    int** sample;
}config;

static config *con; 
static int* pa;
static int* size;

static void config_init(const char* in);
static void config_destroy();
static void config_read_V(FILE* f);
static void config_read_A(FILE* f);
static void config_read_sample(FILE *f);
static edges* calcule_mutual_informations();
static double estimate_L(array* W, array* aW);
static edges* kruskal(edges* e);
static void union_find_init();
static void union_find_destroy();
static int find(int v);
static void join(int v, int w);
static edges* edges_init(int n);
static void edges_destroy(edges* e);
static void save_graph(edges* e, const char* out);
static void chow_liu(const char* in, const char* out);


static void chow_liu(const char* in, const char* out) {
    config_init(in);
    edges* total_edges = calcule_mutual_informations();
    edges* correct_edges = kruskal(total_edges);
    save_graph(correct_edges, out);    
    edges_destroy(total_edges);
    edges_destroy(correct_edges);
    config_destroy();
}

/* Função de comparação que compara os pesos de duas arestas */
static int cmp(const void* a, const void* b) {
    double da = ((edge*)a)->weight;
    double db = ((edge*)b)->weight;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

/* Essa função recebe um conjunto de arestas "e" e devolve um outro
conjunto de arestas que corresponde à árvore geradora máxima */
static edges* kruskal(edges* e) {
    edges* result = edges_init(con->V_size - 1);
    union_find_init();
    qsort(e->edges, e->m, sizeof(edge), cmp);
    for (int i = 0, j = 0; i < e->m; i++) {
        if (find(e->edges[i].v) != find(e->edges[i].w)) {
            result->edges[j++] = e->edges[i];
            join(e->edges[i].v, e->edges[i].w);
        }        
    }
    union_find_destroy();
    return result;   
}

/* Inicializador do Union-Find */
static void union_find_init() {
    pa = malloc_int(con->V_size);
    size = malloc_int(con->V_size);
    for (int i = 0; i < con->V_size; i++) {
        pa[i] = i;
        size[i] = 1;
    }    
}

/* Desalocação dos vetores usados no Union-Find */
static void union_find_destroy() {
    free(pa);
    pa = NULL;
    free(size);
    size = NULL;
}

/* A função Find do Union-Find. */
static int find(int v) {
    if (pa[v] != v) {
        return pa[v] = find(pa[v]);        
    }
    return v;
}

/* A função Join do Union-Find. */
static void join(int v, int w) {
    v = find(v);
    w = find(w);
    if (v == w) return;
    if (size[v] < size[w]) {
        pa[v] = w;
        size[w] = size[w] + size[v];
    } else {
        pa[w] = v;
        size[v] = size[v] + size[w];
    }
}

/* Esse função devolve a medida de informação mútua de todos os pares
   de vértices da árvore. */
static edges* calcule_mutual_informations() {
    combination* c = combination_init(con->V, 2);
    int num_edges = (con->V_size * (con->V_size-1)) / 2;
    edges* es = edges_init(num_edges);
    int i = 0;
    while (combination_has_next(c)) {
        array* W = combination_next(c);
        product* p = product_init(con->A, 2);
        edge e;
        e.v = W->array[0];
        e.w = W->array[1];
        e.weight = 0.0;
        while(product_has_next(p)) {
            array* aW = product_next(p);
            e.weight -= estimate_L(W, aW);
            array_destroy(aW);
        }
        array_destroy(W);
        es->edges[i++] = e;
        product_finish(p);
    }
    combination_finish(c);
    return es;
}

/* Essa função recebe um array de vértice W e um array de alfabeto aW
e devolve o valor parcial da informação mútua. */
static double estimate_L(array* W, array* aW) {
    int N0 = 0, N1 = 0;
    int NaW = 0;
    array* XW = array_zeros(2);
    for (int i = 0; i < con->n; i++) {
        XW->array[0] = con->sample[i][W->array[0]];
        XW->array[1] = con->sample[i][W->array[1]];
        if (array_equals(XW, aW)) NaW++;
        if (XW->array[0] == aW->array[0]) N0++;
        if (XW->array[1] == aW->array[1]) N1++;        
    }
    array_destroy(XW);
    double p0 = (double) N0 / con->n;
    double p1 = (double) N1 / con->n;
    double paW = (double) NaW / con->n;
    return paW * log((paW) / (p0 * p1));
}

/* Essa função desaloca a estrutura config */
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

/* Essa função lê um inteiro do arquivo "f" e atribui ao valor de
   "V_size" do config */
static void config_read_V(FILE* f) {
    fscanf(f, "%d", &con->V_size);
    con->V = array_arange(con->V_size);
}

/* Essa função lê um inteiro do arquivo "f" e atribui ao valor de
   "V_size" do config */
static void config_read_A(FILE* f) {
    fscanf(f, "%d", &con->A_size);
    con->A = array_arange(con->A_size);
}

/* Essa função lê um inteiro do arquivo "f" e atribui ao valor de "n"
   do config e lê uma matrix de tamanho (n x V_size) e atribui ao
   valor de "sample" do config. */
static void config_read_sample(FILE* f) {
    fscanf(f, "%d", &con->n);
    con->sample = (int**) malloc(con->n * sizeof(int*));
    if (con->sample == NULL) {
        printf("malloc() devolveu NULL!\n");
        exit(EXIT_FAILURE);
    } 
    for (int i = 0; i < con->n; i++) {
        con->sample[i] = malloc_int(con->V_size);
        for (int j = 0; j < con->V_size; j++)
            fscanf(f, "%d", &con->sample[i][j]);
    }
}

/* Função de alocação do config. */
static config* malloc_config(int n) {
    config* ptr = (config*) malloc(n * sizeof(config));
    if (ptr == NULL) {
        printf("malloc devolveu NULL\n");
        exit(EXIT_FAILURE);
    }
    return ptr;    
}

/* Função de preenchimento do config a partir do arquivo "in". */
static void config_init(const char* in) {
    FILE *f = fopen(in, "r");
    con = malloc_config(1);
    config_read_V(f);
    config_read_A(f);
    config_read_sample(f);
    fclose(f);
}

/* Função de alocação de "n" da estrutura edge. */
static edge* edge_malloc(int n) {
    edge* ptr = (edge*) malloc(n * sizeof(edge));
    if (ptr == NULL) {
        printf("malloc devolveu NULL!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/* Função de alocação de "n" elementos da estrutura edges. */
static edges* edges_init(int n) {
    edges* result = (edges*) malloc(sizeof(edges));
    if (result == NULL) {
        printf("malloc() devolveu NULL!\n");
        exit(EXIT_FAILURE);
    }
    result->m = n;
    result->edges = edge_malloc(n);
    return result;    
}

/* Função de desalocação de edges*/
static void edges_destroy(edges* e) {
    free(e->edges);
    e->edges = NULL;
    free(e);
    e = NULL;    
}

/* Essa função recebe uma estrutura de edges "e" e grava em "out" em
   formato de grafo. */
static void save_graph(edges* e, const char* out) {
    FILE *f = fopen(out, "w");
    fprintf(f, "%d\n", con->V_size);
    for (int i = 0; i < e->m; i++) 
        fprintf(f, "%d %d\n", e->edges[i].v, e->edges[i].w);
    fclose(f);    
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Falta argumentos!\n");
        return 1;        
    } else 
        chow_liu(argv[1], argv[2]);
    return 0;
} 
