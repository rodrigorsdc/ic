#ifndef PML_GRAPH_H
#define PML_GRAPH_H

struct pml_graph {
    int V;
    int A;
    double c;
    int **sample;    
};

/* Return the |V|x|V| adjacency matrix */
int **estimate_graph(struct pml_graph *data);

#endif
