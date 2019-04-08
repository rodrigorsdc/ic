library('mrfe')
s = as.matrix(read.table('testing_samples/sample55'))
colnames(s) = NULL
rownames(s) = NULL
mrfe_neigh(2L, s, 5L, 4L)
print("SUCESSO!")
