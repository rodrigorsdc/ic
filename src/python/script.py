import util
import simulation
import graph_tool.all as gt
import sys


g = util.random_DAG(100, 0, 1)
gen = util.SampleGenerator({0, 1, 2}, g)
sample = gen.sample(266)
l = util.bayesian_to_markov(g)
util.gen_input_pml({0, 1, 2}, 1.0, sample, util.max_out_degree(l))


def 


if __name__ == '__main__':
    n = int(sys.argv[1])
    for i in range(0, n+1):
        simulation.pml_plot_graphics(i)
    
