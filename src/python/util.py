<<<<<<< HEAD
=======
# Código com funções e classe utilitários

>>>>>>> a756c6795a95e1038195346aa4cd75f8a2347186
import numpy as np
import graph_tool.all as gt
import requests
import itertools as it
import csv

def errors(g, g_hat):
    error_under = 0
    error_over = 0
    n = g["num_vertices"];
    m = (n * (n-1))/2
    for e in g["edges"]:
        if e not in g_hat["edges"]: error_under += 1
    for e in g_hat["edges"]:
        if e not in g["edges"]: error_over += 1

    error_total = (error_under + error_over) / m
    if (len(g["edges"]) > 0):
        error_under = error_under / len(g["edges"])
    if (len(g["edges"]) < m):
        error_over = error_over / (m - len(g["edges"]))
    return np.array([error_under, error_over, error_total])

def random_DAG(n, d_min, d_max):
    f = np.random.randint
    g = gt.random_graph(n, lambda:(f(d_min,d_max+1)), directed=False)
    g.set_directed(True)
    g.set_fast_edge_removal(True)
    for edge in g.get_edges():
        s, t = edge[0], edge[1]
        if s < t: g.remove_edge(g.edge(s, t))
    gt.remove_parallel_edges(g)
    add_edge_prop(g)
    add_vertex_label(g)
    return g

def random_tree(n):
    f = np.random.randint
    g = gt.complete_graph(n, directed=False)
    tree = gt.random_spanning_tree(g)
    g = gt.GraphView(g, efilt=tree)
    f = gt.Graph()
    for e in gt.bfs_iterator(g, np.random.randint(0, n), array=True): f.add_edge(e[0], e[1])
    add_vertex_label(f)
    return f    

def simplifiqued_graph(g):
    d = {}
    d["num_vertices"] = g.num_vertices()
    d["edges"] = set()
    d['d_max'] = max_out_degree(g)
    for edge in g.get_edges():
        if (edge[0] < edge[1]):
            d["edges"].add((edge[0], edge[1]))
        else:
            d["edges"].add((edge[1], edge[0]))
    return d

def gen_input_pml(A, regularizer, sample, d_max):
    f = open('tmp/in', 'wb')
    f.write((str(sample.shape[1]) + '\n\n').encode())
    f.write((str(len(A)) + '\n\n').encode())
    f.write((str(regularizer) + '\n\n').encode())
    f.write((str(d_max) + '\n\n').encode())
    f.write((str(sample.shape[0]) + '\n\n').encode())
    np.savetxt(f, sample, fmt="%d")
    f.close()

def gen_input_chow(A, sample):
    f = open('tmp/in', 'wb')
    f.write((str(sample.shape[1]) + '\n\n').encode())
    f.write((str(len(A)) + '\n\n').encode())
    f.write((str(sample.shape[0]) + '\n\n').encode())
    np.savetxt(f, sample, fmt='%d')
    f.close()

def graph_to_file(g, filename):
    f = open(filename, "w")
    f.write(str(g.num_vertices()) + '\n')
    for e in g.get_edges():
        f.write(str(e[0]) + ' ' + str(e[1]) + '\n')
    f.close()    

def file_to_graph(filename, direct=False):
    f  = open(filename, 'r')
    g = gt.Graph(directed=direct)
    g.add_vertex(int(f.readline()))
    for line in f:
        line = line.split()
        g.add_edge(int(line[0]), int(line[1]))
    f.close()
    add_vertex_label(g)
    gt.remove_parallel_edges(g)
    return g



def bayesian_to_markov(g):
    g_markov = gt.Graph(g)
    g_markov.set_directed(False)
    eprop = g_markov.new_edge_property('bool')
    for v in g.vertices():
        neigh = g.get_in_neighbors(v)
        for w, v in it.combinations(neigh, r=2):
            w, v = int(w), int(v)
            g_markov.add_edge(w, v)
            eprop[g_markov.edge(w, v)] = True
    g_markov.edge_properties['strong'] = eprop
    add_vertex_label(g_markov)
    g_markov.graph_properties['d_max'] = g_markov.new_graph_property('int')
    gt.remove_parallel_edges(g_markov)
    g_markov.graph_properties['d_max'] = max_out_degree(g_markov)
    return g_markov

def max_out_degree(g):
    degrees = g.get_out_degrees(g.get_vertices())
    return np.amax(degrees)

def min_out_degree(g):
    degrees = g.get_out_degrees(g.get_vertices())
    return np.amin(degrees)

def min_in_degree(g):
    degrees = g.get_in_degrees(g.get_vertices())
    return np.amin(degrees)

def add_edge_prop(g):
    eprop = g.new_edge_property('bool')
    g.edge_properties['strong'] = eprop

def add_vertex_label(g):
    vprop = g.new_vertex_property('string')
    for i, v in enumerate(g.get_vertices()): vprop[v] = i
    g.vertex_properties['name']= vprop

def csv_to_dict(i):
    path = 'tests/test_' + str(i) + '/history.csv'
    dic = {}
    with open(path) as f:
        csv_reader = csv.DictReader(f)
        for field in csv_reader.fieldnames: dic[field] = []
        for row in csv_reader:
            for key in row: dic[key].append(float(row[key]))
    return dic            

class SampleGenerator:

    def __init__(self, A, g):
        self.A = list(A)
        self.g = g
        self.cond_neigh = self._gen_cond_neigh(g) 
        self.p_cond = self._gen_p_cond(self.cond_neigh, len(A))

    # Gera uma amostra de tamanho n de acordo com as probabilidades condicionais mantidas na variavel self.p_cond    
    def sample(self, n):
        nv = self.g.num_vertices()
        smpl = np.zeros((n, nv), dtype=np.int32)
        V = gt.topological_sort(self.g)
        a_choice = self._a_choice
        for i in range(n):
            for v in V:
                if (len(self.cond_neigh[v]) == 0):
                    smpl[i][v] = a_choice(self.p_cond[v])
                else:
                    k = self._map([smpl[i][j] for j in self.cond_neigh[v]])
                    smpl[i][v] = a_choice(self.p_cond[v][k])
        return smpl
    
    def _map(self, l):
        n = len(self.A)
        m = len(l)        
        vl = 0
        for i in range(m): vl += l[i] * pow(n, m-i-1)
        return vl - 1
    
    def _a_choice(self, prob):
        prob = np.squeeze(prob)
        ch = np.random.choice(self.A, p=prob)
        return ch

    def print_p_cond(self, f):
        for v in self.g.vertices():
            f.write((str(v) + " given " + str(self.cond_neigh[v]) +
                     "\n").encode())
            np.savetxt(f, self.p_cond[v], fmt="%.2lf")
            f.write(('\n\n').encode())

    def _gen_p_cond(self, cond_neigh, A_size):
        p_cond = {}
        for i in cond_neigh:
            m = len(cond_neigh[i])
            nrow = pow(A_size, m)
            ncol = A_size
            p = np.zeros((nrow, ncol))
            for j in range(nrow): p[j] = np.random.dirichlet(np.ones(A_size))
            p_cond[i] = p
        return p_cond
    
    def _gen_cond_neigh(self, g):
        cond_neigh = {}
        for v in g.vertices():
            v_cond = []
            for w in g.get_in_neighbors(v): v_cond.append(int(w))
            cond_neigh[int(v)] = sorted(v_cond)
        return cond_neigh

