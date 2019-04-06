library('bla')
s = as.matrix(read.table('testing_samples/sample4'))
pml_graph(2L, s, c(0.1, 2.0, 0.1), 5L)
print("SUCESSO!")
