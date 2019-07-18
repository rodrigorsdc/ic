list_to_matrix.cons <- function(list.adj) {
    n <- length(list.adj)
    matrix.adj <- matrix(rep(0, n*n), ncol=n, nrow=n)
    for (i in 1:n) {
        for (j in 1:n) {
            if (is.element(j, list.adj[[i]]) &&
            is.element(i, list.adj[[j]])) {
                matrix.adj[i,j] <- 1
                matrix.adj[j,i] <- 1
            }
        }            
    }
    return (matrix.adj)
}

list_to_matrix.ncons <- function(list.adj) {
    n <- length(list.adj)
    matrix.adj <- matrix(rep(0, n*n), ncol=n, nrow=n)
    for (i in 1:n) {
        for (j in 1:n) {
            if (is.element(j, list.adj[[i]])) {
                matrix.adj[i, j] <- 1
                matrix.adj[j, i] <- 1
            }
        }
    }
    return (matrix.adj)
}

errors <- function(g, eg){
    n<-length(g)
    error.under<-0
    error.over<-0
    for(i in 1:n){
        if(g[i]!=eg[i]){
            if(g[i]<eg[i]){
                error.over<-error.over+1
            }
            else{
                error.under<-error.under+1
            }
        }
        i<-i+1
    }
    error.total<-(error.under+error.over)/(n - ncol(g))
    if (sum(g) == 0)
        error.under <- 0
    else 
        error.under<-error.under/sum(g)
    if((n - ncol(g)) - sum(g) == 0)
        error.over <- 0
    else
        error.over<-error.over/((n - ncol(g))-sum(g))
    results<-cbind(error.under,error.over,error.total)
    return(results)
}


graph.const_sample <- function(sample_sizes, consts, A,
                               sample_graph, file_name) {
    pdf(paste(file_name,"_ncons.pdf", sep=""))
    par(mfcol=c(5,5), mai=c(0,0.55,0.2,0),
    font.main=1, cex.lab=0.9, cex.axis=0.9,
    family="serif", font.lab=2, font.axis=2)
    for (s in sample_sizes) {
        for (c in consts) {
            sample = sample_graph(s)
            list.adj = mrfe(A, sample, c)
            matrix.adj = list_to_matrix.ncons(list.adj)
            grafo<-graph_from_adjacency_matrix(matrix.adj, mode="undirect")
            #grafo$layout <- layout_on_grid(grafo, 5)
            grafo$layout <- layout.circle
            plot(grafo, vertex.color="black", vertex.label=NA)
            if(s == sample_sizes[1])
                title(ylab=paste("c=", c, sep=""))
            if(c == consts[1])
                title(main=paste("n=", s, sep=""))
        }
    }
    pdf(paste(file_name,"_cons.pdf", sep=""))
    par(mfcol=c(5,5), mai=c(0,0.55,0.2,0),
    font.main=1, cex.lab=0.9, cex.axis=0.9,
    family="serif", font.lab=2, font.axis=2)
    for (s in sample_sizes) {
        for (c in consts) {
            sample = sample_graph(s)
            list.adj = mrfe(A, sample, c)
            matrix.adj = list_to_matrix.cons(list.adj)
            grafo<-graph_from_adjacency_matrix(matrix.adj, mode="undirect")
            #grafo$layout <- layout_on_grid(grafo, 5)
            grafo$layout <- layout.circle
            plot(grafo, vertex.color="black", vertex.label=NA)
            if(s == sample_sizes[1])
                title(ylab=paste("c=", c, sep=""))
            if(c == consts[1])
                title(main=paste("n=", s, sep=""))
        }
    }
}

means_sds.errors <- function(N, step, original_graph, A,
                         sample_graph) {

    s = N / step
    means.ncons <- matrix(c(rep(0, 3*s)), ncol=s, nrow=3)
    sds.ncons <- matrix(c(rep(0, 3*s)), ncol=s, nrow=3)
    means.cons <- matrix(c(rep(0, 3*s)), ncol=s, nrow=3)
    sds.cons <- matrix(c(rep(0, 3*s)), ncol=s, nrow=3)
    j <- 1
    for (n in seq(step, N, by=step)) {
        e.ncons = matrix(c(rep(0, 90)), ncol=30, nrow=3)
        e.cons = matrix(c(rep(0, 90)), ncol=30, nrow=3)
        for (i in 1:30) {
            sample <- sample_graph(n)
            list.adj <- mrfe(A, sample, 1.0)
            matrix.adj.ncons <- list_to_matrix.ncons(list.adj)
            matrix.adj.cons <- list_to_matrix.cons(list.adj)
            error.ncons <- errors(original_graph, matrix.adj.ncons)
            error.cons <- errors(original_graph, matrix.adj.cons)
            e.ncons[,i] <- error.ncons
            e.cons[,i] <- error.cons
        }
        means.ncons[,j] <- apply(e.ncons, 1, mean)
        sds.ncons[,j] <- apply(e.ncons, 1, sd)
        means.cons[,j] <- apply(e.cons, 1, mean)
        sds.cons[,j] <- apply(e.cons, 1, sd)
        j <- j + 1
    }
    result <- list(means.ncons, sds.ncons, means.cons, sds.cons)
    return (result)
}

graph.errors <- function(means.ncons, sds.ncons, means.cons,
                         sds.cons, N, file_name, title_name) {
    main.texts <- c(paste(title_name, "underestimation error\n"),
                paste(title_name, "overestimation error\n"),
                paste(title_name, "total error\n"))
    errors.texts <- c("underestimation error",
                  "overestimation error",
                  "total error")
    pdf(paste(file_name, "_ncons.pdf", sep=""), width=12, height=6)
    par(mfrow=c(1, 3), family='serif', cex.axis=1.2, cex.lab=1.2)
    for (i in 1:3) {
        plot(seq(step, N, step), means.ncons[i,], xlab='n',
             ylab=errors.texts[i], xlim=c(10, N), ylim=c(-0.1, 1.0),
             type='l', col='green',
             main=paste(main.texts[i], "Non-conservative approach"))
        lines(seq(step, N, step), means.ncons[i,] + sds.ncons[i,],
              col='red', type='l', lty=2)
        lines(seq(step, N, step), means.ncons[i,] - sds.ncons[i,],
              col='red', type='l', lty=2)
        legend("topright", legend=c('mean', 'mean +- std'),
               col=c('green', 'red'), inset=c(0.04, 0.04),
               lty=c(1, 2), cex=1.2)
    }
    pdf(paste(file_name, "_cons.pdf", sep=""), width=12, height=6)
    par(mfrow=c(1, 3), family='serif', cex.axis=1.2, cex.lab=1.2)
    for (i in 1:3) {
        plot(seq(step, N, step), means.cons[i,], xlab='n',
             ylab=errors.texts[i], xlim=c(10, N), ylim=c(-0.1, 1.0),
             type='l', col='green',
             main=paste(main.texts[i], "Conservative approach"))
        lines(seq(step, N, step), means.cons[i,] + sds.cons[i,],
              col='red', type='l', lty=2)
        lines(seq(step, N, step), means.cons[i,] - sds.cons[i,],
              col='red', type='l', lty=2)
        legend("topright", legend=c('mean', 'mean +- std'),
               col=c('green', 'red'), inset=c(0.04, 0.04),
               lty=c(1, 2), cex=1.2)
    }
}

