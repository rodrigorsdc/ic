<<<<<<< HEAD
=======
# Código que gera as simulações para os algoritmos
# de máxima verossimilhança penalizada e de Chow-Liu

>>>>>>> a756c6795a95e1038195346aa4cd75f8a2347186
import numpy as np
import subprocess
import util
import matplotlib.pyplot as plt
import os
import graph_tool.all as gt
import csv

def simulate_pml(regularizer, sample_min, sample_max, sample_interval,
                 samples_per_size, A_size, type_graph=None,
                 d_min=None, d_max=None, n=None, filename_g=None):
    A = set(np.arange(A_size))
    if (filename_g is None):
        if (type_graph == "random"): g = util.random_DAG(n, d_min, d_max)
        else:                        g = util.random_tree(n)
        gen = util.SampleGenerator(A, g)
        pml_simulation_case(A, regularizer, sample_min, sample_max,
                      sample_interval, samples_per_size, g, gen)            
    else:
        g = util.file_to_graph(filename_g, True)
        gen = util.SampleGenerator(A, g) 
        pml_simulation_case(A, regularizer, sample_min, sample_max,
                      sample_interval, samples_per_size, g, gen)

def simulate_chow(sample_min, sample_max, sample_interval,
                  samples_per_size, A_size, n=None, filename_g=None):
    A = set(np.arange(A_size))
    if (filename_g is None): g = util.random_tree(n)
    else:                    g = util.file_to_graph(filename_g, True)
    gen = util.SampleGenerator(A, g)
    chow_simulation_case(A, sample_min, sample_max, sample_interval,
                   samples_per_size, g, gen)

def chow_simulation_case(A, sample_min, sample_max, sample_interval,
                   samples_per_size, g, gen):
    dirname = create_simulation_dir()
    i = sample_min
    g_markov = util.bayesian_to_markov(g)
    save_graph(g, dirname, 'DAG_original')
    save_graph(g_markov, dirname, 'Markov_original')
    g_markov = util.simplifiqued_graph(g_markov)
    history = {}
    history['ue'], history['oe'], history['te'] = [],[],[]
    history['sample'] = []
    chow_create_history_csv(dirname)
    while (i <= sample_max):
        errors = np.zeros(3)
        for j in range(samples_per_size):
            sample = gen.sample(i)
            util.gen_input_chow(A, sample)
            subprocess.call(['bin/chow', 'tmp/in', 'tmp/out'])
            g_hat = util.file_to_graph('tmp/out')
            save_graph(g_hat, dirname, str(i) + '_' + str(j))
            g_hat = util.simplifiqued_graph(g_hat)
            errors += util.errors(g_markov, g_hat)
        errors /= samples_per_size
        history['ue'].append(errors[0])
        history['oe'].append(errors[1])
        history['te'].append(errors[2])
        history['sample'].append(i)
        i += sample_interval
        chow_add_history_csv(history, dirname)
<<<<<<< HEAD
    # chow_plot_graphics(history, dirname)
=======
>>>>>>> a756c6795a95e1038195346aa4cd75f8a2347186
    chow_add_info(history, g_markov['num_vertices'], dirname, A, gen)

def chow_add_info(history, V_size, dir_name, A, gen):
    path = dir_name + "/info.txt"
    with open(path, 'wb') as f:
        f.write(("Number of vertices: " + str(V_size) + '\n').encode())
        f.write(("Last underestimate error: " + str(history['ue'][-1]) +
                 '\n').encode())
        f.write(("Last overestimate error: " + str(history['oe'][-1])
                 + '\n').encode())
        f.write(("Last total error: " + str(history['te'][-1]) + '\n').encode())
        f.write(("Alphabet size: " + str(len(A)) + '\n').encode())
<<<<<<< HEAD
        f.write(("Vertices' probability distribution\n\n").encode())
=======
        f.write(("Probability distribution of the vertices\n\n").encode())
>>>>>>> a756c6795a95e1038195346aa4cd75f8a2347186
        gen.print_p_cond(f)

def pml_simulation_case(A, regularizer, sample_min, sample_max,
                  sample_interval, samples_per_size, g, gen):
    dir_name = create_simulation_dir()
    i = sample_min
    g_markov = util.bayesian_to_markov(g)    
    d_max = g_markov.graph_properties['d_max']
<<<<<<< HEAD
    print(d_max)
=======
>>>>>>> a756c6795a95e1038195346aa4cd75f8a2347186
    save_graph(g, dir_name, 'DAG_original')
    save_graph(g_markov, dir_name, 'Markov_original')
    g_markov = util.simplifiqued_graph(g_markov)
    history = {}
    history['ue_cons'], history['oe_cons'], history['te_cons'] = [],[],[]
    history['ue_ncons'], history['oe_ncons'], history['te_ncons'] = [],[],[]
    history['sample'] = []
    pml_create_history_csv(dir_name)
    while (i <= sample_max):
        erros_cons = np.zeros(3)
        erros_ncons = np.zeros(3)
        for j in range(samples_per_size):
            sample = gen.sample(i)
            util.gen_input_pml(A, regularizer, sample, d_max)
            subprocess.call(['bin/pml', 'tmp/in', 'tmp/cons_out',
                             'tmp/ncons_out'])
            g_hat_cons = util.file_to_graph('tmp/cons_out')
            g_hat_ncons = util.file_to_graph('tmp/ncons_out')
            save_graph(g_hat_cons, dir_name, 'cons_' + str(i) +
                       '_' + str(j))
            save_graph(g_hat_ncons, dir_name, 'ncons_' + str(i) +
                       '_' + str(j))
            g_hat_cons = util.simplifiqued_graph(g_hat_cons)
            g_hat_ncons = util.simplifiqued_graph(g_hat_ncons)
            erros_cons += util.errors(g_markov, g_hat_cons)
            erros_ncons += util.errors(g_markov, g_hat_ncons)
        erros_cons /= samples_per_size
        erros_ncons /= samples_per_size
        history['sample'].append(i)
        history['ue_cons'].append(erros_cons[0])
        history['oe_cons'].append(erros_cons[1])
        history['te_cons'].append(erros_cons[2])
        history['ue_ncons'].append(erros_ncons[0])
        history['oe_ncons'].append(erros_ncons[1])
        history['te_ncons'].append(erros_ncons[2])
        i += sample_interval
        pml_add_history_csv(history, dir_name)

    # pml_plot_graphics(history, dir_name)
    pml_add_info(history, g_markov['num_vertices'], d_max, dir_name,
                 regularizer, A, gen)

def pml_add_info(history, V_size, d_max, dir_name, regularizer, A, gen):
    path = dir_name + '/info.txt'
    with open(path, 'wb') as f:
        f.write(('Number of vertices: ' + str(V_size) + '\n').encode())
        f.write(('Maximum degree: ' + str(d_max) + '\n').encode())
        f.write(('Constant value: ' + str(regularizer) + '\n').encode())
        f.write(('A size: ' + str(len(A)) + '\n').encode())
        f.write(('Last subestimate error non-conservative: ' +
                 str(history['ue_ncons'][-1]) + '\n').encode())
        f.write(('Last overestimate error non-conservative: ' +
                 str(history['oe_ncons'][-1]) + '\n').encode())
        f.write(('Last total error non-conservative: ' +
                 str(history['te_ncons'][-1]) + '\n').encode())
        f.write(('Last subestimate error conservative: ' +
                 str(history['ue_cons'][-1]) + '\n').encode())
        f.write(('Last overestimate error conservative: ' +
                 str(history['oe_cons'][-1]) + '\n').encode())
        f.write(('Last total error conservative: ' +
                 str(history['te_cons'][-1]) + '\n').encode())        
        f.write(('Vertices probability distribution:\n').encode())
        gen.print_p_cond(f)

def chow_plot_graphics(history):
    f = plt.figure(figsize=(17, 10))
    path = 'erros.png'
    sp = plt.subplot(1, 3, 1)
    plt.plot(history['sample'], history['ue'])
    plt.xlabel("Tamanho da amostra", fontsize=11, labelpad=20)
    plt.ylabel("Erro de subestimação", fontsize=11, labelpad=20)
    plt.title("Erro de subestimação", fontsize=13, fontweight='bold')
    plt.ylim(-0.04, 1.0)

    sp = plt.subplot(1, 3, 2)
    plt.plot(history['sample'], history['oe'])
    plt.xlabel("Tamanho da amostra", fontsize=11, labelpad=20)
    plt.ylabel("Erro de sobrestimação", fontsize=11, labelpad=20)
    plt.title("Erro de sobrestimação", fontsize=13, fontweight='bold')
    plt.ylim(-0.04, 1.0)

    sp = plt.subplot(1, 3, 3)
    plt.plot(history['sample'], history['te'])
    plt.xlabel("Tamanho da amostra", fontsize=11, labelpad=20)
    plt.ylabel("Erro total", fontsize=11, labelpad=20)
    plt.title("Erro total", fontsize=13, fontweight='bold')
    plt.ylim(-0.04, 1.0)

    plt.tight_layout()
    f.savefig(path)    

def pml_plot_graphics(i):
    history = util.csv_to_dict(i)
    f = plt.figure(figsize=(17, 10))
    path = 'simulations/simulation_' + str(i) + '/erros.png'
    
    sp = plt.subplot(1, 3, 1)
    plt.plot(history['sample_size'], history['ue_ncons'],
             label="não-conservativo")
    plt.plot(history['sample_size'], history['ue_cons'],
             label="Conservativo", linestyle='--')
    plt.xlabel("Tamanho da amostra", fontsize=11, labelpad=20)
    plt.ylabel("Erro de subestimação", fontsize=11, labelpad=20)
    plt.title("Erro de subestimação", fontsize=13, fontweight='bold')
    plt.ylim(-0.04, 1.0)
    plt.legend()
    
    sp = plt.subplot(1, 3, 2)
    plt.plot(history['sample_size'], history['oe_ncons'],
             label="não-conservativo")
    plt.plot(history['sample_size'], history['oe_cons'],
             label="conservativo", linestyle='--')
    plt.xlabel("Tamanho da amostra", fontsize=11, labelpad=20)
    plt.ylabel("Erro de sobrestimação", fontsize=11, labelpad=20)
    plt.title("Erro de sobrestimação", fontsize=13, fontweight='bold')
    plt.ylim(-0.04, 1.0)
    plt.legend()


    sp = plt.subplot(1, 3, 3)
    plt.plot(history['sample_size'], history['te_ncons'],
             label='não-conservativo')
    plt.plot(history['sample_size'], history['te_cons'],
             label='conservativo', linestyle='--')
    plt.xlabel("Tamanho da amostra", fontsize=11, labelpad=20)
    plt.ylabel("Erro total", fontsize=11, labelpad=20)
    plt.title("Erro total", fontsize=13, fontweight='bold')
    plt.ylim(-0.04, 1.0)
    plt.legend()

    plt.tight_layout()
    f.savefig(path)



def save_graph(g, dir_name, filename):
    g.save(dir_name + '/' + filename + '.xml.gz')
    
def create_simulation_dir():
    try:
        f = open('simulations/simulation_counter', 'r')
    except FileNotFoundError:
        f = open('simulations/simulation_counter', 'w')
        f.write('0')
        f.close()
        return create_simulation_dir()
    num_case = int(f.read())
    dir_name = 'simulations/simulation_' + str(num_case)
    subprocess.call(['mkdir', dir_name])
    f.close()
    f = open('simulations/simulation_counter', 'w')
    f.write(str(num_case + 1))
    f.close()
    return dir_name

def actual_simulation_dir():
    try:
        f = open('simulations/simulation_counter', 'r')
    except FileNotFoundError:
        return create_simulation_dir
    num_case = int(f.read())-1
    return 'simulations/simulation_' + str(num_case) 

def chow_add_history_csv(h, dirname):
    path = dirname + '/erros.csv'
    row = [h['sample'][-1], h['ue'][-1], h['oe'][-1], h['te'][-1]]
    row = map(str, row)
    with open(path, 'a') as f:
        writer = csv.writer(f, delimiter=',', quotechar='"',
                            quoting=csv.QUOTE_MINIMAL)
        writer.writerow(row)

def pml_add_history_csv(h, dirname):
    path = dirname + '/history.csv'
    row = [h['sample'][-1], h['ue_cons'][-1], h['oe_cons'][-1],
           h['te_cons'][-1], h['ue_ncons'][-1], h['oe_ncons'][-1],
           h['te_ncons'][-1]]
    row = map(str, row)    
    with open(path, 'a') as f:
        writer = csv.writer(f, delimiter=',', quotechar='"',
                            quoting=csv.QUOTE_MINIMAL)
        writer.writerow(row)                
    
def pml_create_history_csv(dirname):
    path = dirname + '/history.csv'
    with open(path, 'w') as f:
        writer = csv.writer(f, delimiter=',', quotechar='"',
                            quoting=csv.QUOTE_MINIMAL)
        writer.writerow(['sample', 'ue_cons', 'oe_cons', 'te_cons',
                         'ue_ncons', 'oe_ncons', 'te_ncons'])

def chow_create_history_csv(dirname):
    path = dirname + '/history.csv'
    with open(path, 'w') as f:
        writer = csv.writer(f, delimiter=',', quotechar='"',
                            quoting=csv.QUOTE_MINIMAL)
        writer.writerow(['sample', 'ue', 'oe', 'te'])
