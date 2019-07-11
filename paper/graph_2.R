library(igraph)
library(mrfe)
source("util.R")

sample_graph<-function(n){
    A<-c(0L,1L,2L)
    x<-matrix(rep(0L,times=n*5),ncol=5,nrow=n)
    p1_3<-matrix(c(2,4,4,3,4,3,4,3,3)/10, nrow=3, ncol=3, byrow=T)
    p2_13<-matrix( c(1/2,1/2,0,2/4,1/4,1/4,1/4,1/4,2/4,1/3,0,2/3,1/4,
                     1/4,2/4,1/3,2/3,0,0,3/4,1/4,1/3,1/3,1/3,1/3,1/3,1/3),
                  nrow=9, ncol=3, byrow=T)
    p4_3<-matrix(c(1,4,5,2,7,1,3,6,1)/10, nrow=3, ncol=3, byrow=T)
    p5_3<-matrix(c(2,6,2,3,1,6,4,3,3)/10, nrow=3, ncol=3, byrow=T)
    p3=c(0.3,0.2,0.5)
    x[,3]<-sample(A,size=n,prob=p3,replace=TRUE)
    for(i in 1:n){
        x[i,1]<-sample(A,size=1,prob=p1_3[x[i,3]+1,])
        x[i,4]<-sample(A,size=1,prob=p4_3[x[i,3]+1,])
        x[i,5]<-sample(A,size=1,prob=p5_3[x[i,3]+1,])
        x[i,2]<-sample(A,size=1,prob=p2_13[3*x[i,3]+x[i,1]+1,])
    }
    return(x)
}

original_graph <- matrix(c(0, 1, 1, 0, 0,
                           1, 0, 1, 0, 0,
                           1, 1, 0, 1, 1,
                           0, 0, 1, 0, 0,
                           0, 0, 1, 0, 0), nrow=5)
consts <- c(.25, .5, 1, 1.5, 2)
sample_sizes <- c(100, 200, 500, 1000 , 2500)

graph.const_sample(sample_sizes, consts, 3L, sample_graph,
                   "graph2_c_versus_sampleSize")
N = 10000
step = 100
#means_sds <- means_sds.errors(N, step, original_graph, 3L, sample_graph)
#write(t(means_sds[[1]]), "graph2/means.ncons", ncolumns=ncol(means_sds[[1]]))
#write(t(means_sds[[2]]), "graph2/sds.ncons", ncolumns=ncol(means_sds[[2]]))
#write(t(means_sds[[3]]), "graph2/means.cons", ncolumns=ncol(means_sds[[3]]))
#write(t(means_sds[[4]]), "graph2/sds.cons", ncolumns=ncol(means_sds[[4]]))

means.ncons <- as.matrix(read.table("graph2/means.ncons"))
sds.ncons <- as.matrix(read.table("graph2/sds.ncons"))
means.cons <- as.matrix(read.table("graph2/means.cons"))
sds.cons <- as.matrix(read.table("graph2/sds.cons"))

graph.errors(means.ncons, sds.ncons, means.cons, sds.cons,
             N, "graph2_errors", "Graph 2")
#graph.errors(means_sds[[1]], means_sds[[2]], means_sds[[3]], means_sds[[4]],
#             N, "graph2_errors", "Graph 2")
