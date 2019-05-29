import networkx as nx
import numpy as np
import time

class SampleGenerator:

    def __init__(self, A, g):
        self.A = list(A)
        self.g = g
        self.cond_neigh = self._gen_cond_neigh(g) 
        self.p_cond = self._gen_p_cond(self.cond_neigh, len(A))

    # Gera uma amostra de tamanho n de acordo com as probabilidades condicionais mantidas na variavel self.p_cond    
    def sample(self, n):
        nv = self.g.number_of_nodes()
        smpl = np.zeros((n, nv), dtype=np.int32)
        V = tuple(self.cond_neigh.keys())
        a_choice = self._a_choice
        for i in range(n):
            for v in reversed(V):
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

    def _gen_p_cond(self, cond_neigh, n):
        p_cond = {}
        for i in cond_neigh:
            m = len(cond_neigh[i])
            nrow = pow(n, m)
            ncol = n
            p = np.zeros((nrow, ncol))
            for j in range(nrow): p[j] = np.random.dirichlet(np.ones(n))
            p_cond[i] = p
        return p_cond
    
    def _gen_cond_neigh(self, g):
        cond_neigh = {}
        for v in g.nodes():
            v_cond = []
            for w in g.neighbors(v):
                if (int(w) > int(v)): v_cond.append(int(w))
            cond_neigh[int(v)] = sorted(v_cond)
        return cond_neigh



def rand():
    return np.random.randint(4)

def gen_in_file(A, c, sample, d):
    f = open("in.txt", "w")
    f.write(str(sample.shape[1]) + '\n\n')
    f.write(str(len(A)) +'\n\n')
    f.write(str(c) + '\n\n')
    f.write(str(d) + '\n\n')
    f.write(str(sample.shape[0]) + '\n\n')
    # print(sample)
    np.savetxt(f, sample, fmt="%d")
    f.close()

if __name__ == '__main__':
    g = nx.Graph()
    g.add_edge(0, 4)
    g.add_edge(0, 2)
    g.add_edge(1, 3)
    g.add_edge(2, 3)
    g.add_edge(2, 4)
    gen = SampleGenerator({0, 1}, g)
    sample = gen.sample(750)
    gen_in_file({0, 1}, 0.5, sample, 2)
    
