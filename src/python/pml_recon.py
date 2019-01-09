import multiprocessing as mp
import numpy as np
import itertools as it
import time
import util
from config import PML_config
from sample_generator import *
from graph_tool.all import *
import math

word_table = mp.Manager().dict()
word = mp.Manager().dict()
va = mp.Manager().dict()
va[1] = 0

def estimate_graph():
    ne_hat = mp.Manager().dict()
    pool = mp.Pool(processes=mp.cpu_count())
    [pool.apply(__estimate_neighborhood, (v, ne_hat)) for v in config.V]
    pool.close()
    pool.join()
    g_hat = __recon(dict(ne_hat))
        
    return g_hat

def __estimate_neighborhood(v, ne_hat):
    best_value = -np.inf
    best_neighborhood = None
    V = config.V - {v}
    for d in range(config.d + 1):
        for W in it.combinations(V, d):
            PL_value = __estimate_PML(v, W)
            if (PL_value > best_value):
                (best_value, best_neighborhood) = (PL_value, W)
                
    print('v ' + str(best_value))
    print('wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww')        
    ne_hat[v] = set(best_neighborhood)

def __penalized_factor(W):
    m = len(W)
    
    return (config.c * pow(config.A_len, m) * math.log(config.n, config.A_len))
    

def __estimate_PML(v, W):
    m = len(W)        
    PL_value = 0
    A = config.A
    k = 0
    for a in it.product(A, repeat=1):
        for aW in it.product(A, repeat=m):
            PL_value += __MLE_cond(v, W, a, aW)
            k += 1
    # print(__penalized_factor(W), " ", len(W))
    # print(PL_value)
    PL_value -= __penalized_factor(W)
    return PL_value

def __MLE_cond(v, W, a, aW):
    N_aW = 0
    N_a_aW = 0
    v = (v,)
    vW = v + W
    aaW = a + aW

    if (vW, aaW) not in word_table:
        word_table[(vW, aaW)] = __word_count(vW, aaW)
    else:
        va[1] += 1
    if ((W, aW) not in word_table):
        word_table[(W, aW)] = __word_count(W, aW)
    else:
        va[1] += 1
    N_a_aW = word_table[(vW, aaW)]
    N_aW = word_table[(W, aW)]
    if (N_aW == 0): p_hat = 1.0 / config.A_len
    else:           p_hat = N_a_aW / N_aW
    if (N_a_aW == 0.0): return 0.0
    
    return N_a_aW * math.log(p_hat)

def __word_count(W, aW):
    N_aW = 0
    for i in range(config.n):
        x_W =  tuple(sample[i][w] for w in W)
        if (x_W == aW): N_aW += 1
    return N_aW
    
def __recon(ne_hat):
    g_hat = Graph(directed=False)
    g_hat.add_vertex(len(config.V))
    print(ne_hat)
    if (config.conservative):
        for v in ne_hat:
            for w in ne_hat[v]:
                if (v in ne_hat[w]): g_hat.add_edge(v, w)
            remove_parallel_edges(g_hat)
    else:
        for v in ne_hat:
            for w in ne_hat[v]: g_hat.add_edge(v, w)
            remove_parallel_edges(g_hat)    

    return g_hat

def rand():
    return np.random.choice([0, 1])


if __name__ == '__main__':
    g = Graph(directed=False)
    sample = np.loadtxt("ent", dtype=np.int32)
    config = PML_config({0, 1, 2}, sample, 1.5, 4, conservative=False)
    g_hat = estimate_graph()
    pos = sfdp_layout(g_hat)
    pos2 = sfdp_layout(g)
    graph_draw(g_hat, pos)
