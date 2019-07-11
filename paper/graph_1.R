library(igraph)
library(mrfe)
source("util.R")

sample_graph <- function(n){
    A <- c(0L, 1L)
    x <- matrix(rep(0L,times=5*n), ncol=5, nrow=n)                             
    p1 <- c(.3, .7)
    P <- matrix(c(.5, .5, 1, 0), ncol=2, nrow=2)
    x[,1] <- sample(A,size=n,prob=p1,replace=T)
    for(i in 1:n) {
        for(j in 2:5) {
            x[i, j] <- sample(A, size=1, prob=P[x[i,j-1]+1,])
        }
    }
    return(x)
}

original_graph = matrix(c(0, 1, 0, 0, 0,
                          1, 0, 1, 0, 0,
                          0, 1, 0, 1, 0,
                          0, 0, 1, 0, 1,
                          0, 0, 0 , 1, 0), ncol=5, nrow=5)
print(original_graph)
consts <- c(.25, .5, 1, 1.5, 2)
sample_sizes <- c(25, 50, 100, 250, 500)

graph.const_sample(sample_sizes, consts, 2L, sample_graph,
                   "graph1_c_versus_sampleSize")
N = 400
step = 10
means_sds <- means_sds.errors(N, step, original_graph, 2L, sample_graph)
write(t(means_sds[[1]]), "graph1/means.ncons", ncolumns=ncol(means_sds[[1]]))
write(t(means_sds[[2]]), "graph1/sds.ncons", ncolumns=ncol(means_sds[[2]]))
write(t(means_sds[[3]]), "graph1/means.cons", ncolumns=ncol(means_sds[[3]]))
write(t(means_sds[[4]]), "graph1/sds.cons", ncolumns=ncol(means_sds[[4]]))

means.ncons <- as.matrix(read.table("graph1/means.ncons"))
sds.ncons <- as.matrix(read.table("graph1/sds.ncons"))
means.cons <- as.matrix(read.table("graph1/means.cons"))
sds.cons <- as.matrix(read.table("graph1/sds.cons"))

graph.errors(means.ncons, sds.ncons, means.cons, sds.cons,
             N, "graph1_errors", "Graph 1")
