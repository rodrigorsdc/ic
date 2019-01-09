<<<<<<< HEAD
=======
# Código main

>>>>>>> a756c6795a95e1038195346aa4cd75f8a2347186
import subprocess as sp
import sys
import simulation
import cProfile
import time


<<<<<<< HEAD
# algo [regularizer] sample_min sample_max sample_interval samples_per_size A_size type_graph [filename] [d_min d_max n] [n]

=======
>>>>>>> a756c6795a95e1038195346aa4cd75f8a2347186
if __name__ == "__main__":
    arg = sys.argv[1:len(sys.argv)]
    if (arg[0] == "pml"):
        if (arg[7] == "self"):
            simulation.simulate_pml(float(arg[1]), int(arg[2]), int(arg[3]),
                                    int(arg[4]), int(arg[5]), int(arg[6]),
                                    filename_g=arg[8])
        else:
            simulation.simulate_pml(float(arg[1]), int(arg[2]), int(arg[3]),
                         int(arg[4]), int(arg[5]), int(arg[6]), arg[7],
                                    int(arg[8]), int(arg[9]), int(arg[10]))

    elif (arg[0] == "chow-liu"):
        if (arg[6] == "self"):
            simulation.simulate_chow(int(arg[1]), int(arg[2]), int(arg[3]),
                                     int(arg[4]), int(arg[5]),
                                     filename_g=arg[7])
        else:
            simulation.simulate_chow(int(arg[1]), int(arg[2]), int(arg[3]),
                                     int(arg[4]), int(arg[5]), n=int(arg[7]))
            
<<<<<<< HEAD
    sp.call(['rm', 'tmp/cons_out', 'tmp/in', 'tmp/ncons_out', 'tmp/out'])
=======
    sp.call(['rm', 'tmp/cons_out', 'tmp/in', 'tmp/ncons_out', 'tmp/out'],
            stderr=sp.DEVNULL)
>>>>>>> a756c6795a95e1038195346aa4cd75f8a2347186
