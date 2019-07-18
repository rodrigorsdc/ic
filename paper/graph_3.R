library(igraph)
library(mrfe)
source("util.R")

sample_graph <- function(n) {
    A <- c(0L, 1L)
    x <- matrix(rep(0, times=n*5), ncol=5)
    p1 <- c(.3, .7)
    p2 <- c(.8, .2)
    p3 <- c(.4, .6)
    p4 <- c(.5, .5)
    p5_1234 <- matrix(c(.7, .3,
                        .2, .8,
                        .1, .9,
                        .9, .1,
                        .75, .25,
                        .5, .5,
                        .3, .7,
                        .0, 1,
                        .8, .25,
                        .5, .5,
                        .6, .4,
                        .85, .15,
                        .45, .55,
                        .7, .3,
                        .95, .05,
                        .35, .65), ncol=2, byrow=T)

    x[,1] <- sample(A, size=n, prob=p1, replace=T)
    x[,2] <- sample(A, size=n, prob=p2, replace=T)
    x[,3] <- sample(A, size=n, prob=p3, replace=T)
    x[,4] <- sample(A, size=n, prob=p4, replace=T)
    for (i in 1:n) {
        index <- x[i, 1] + 2 * x[i, 2] + 4 * x[i, 3] + 8 * x[i, 4] + 1
        x[i,5] <- sample(A, size=1, prob=p5_1234[index,], replace=T)
    }
    return (x)                      
}

original_graph <- matrix(c(0, 1, 1, 1, 1,
                           1, 0, 1, 1, 1,
                           1, 1, 0, 1, 1,
                           1, 1, 1, 0, 1,
                           1, 1, 1, 1, 0), ncol=5)

consts <- c(.25, .5, 1, 1.5, 2.0)
sample_sizes <- c(100, 500, 1000, 2500, 5000)
graph.const_sample(sample_sizes, consts, 2L, sample_graph,
                   "graph3_c_versus_sampleSize")

N=10000
step=100
#means_sds <- means_sds.errors(N, step, original_graph, 2L, sample_graph)
#write(t(means_sds[[1]]), "graph3/means.ncons", ncolumns=ncol(means_sds[[1]]))
#write(t(means_sds[[2]]), "graph3/sds.ncons", ncolumns=ncol(means_sds[[2]]))
#write(t(means_sds[[3]]), "graph3/means.cons", ncolumns=ncol(means_sds[[3]]))
#write(t(means_sds[[4]]), "graph3/sds.cons", ncolumns=ncol(means_sds[[4]]))

means.ncons <- as.matrix(read.table("graph3/means.ncons"))
sds.ncons <- as.matrix(read.table("graph3/sds.ncons"))
means.cons <- as.matrix(read.table("graph3/means.cons"))
sds.cons <- as.matrix(read.table("graph3/sds.cons"))

#graph.errors(means.ncons, sds.ncons, means.cons, sds.cons,
#             N, "graph3_errors", "Graph 3")

