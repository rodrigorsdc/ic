mrfe_graph <- function(A_size, sample, c, k=NULL) {
    f <- function(A_size, sample, c, k).Call('mrfe_gr', A_size,
                                             sample, c, k);
    return (f(A_size, sample, c, k));
}

"mrfe_neigh" <- function(A_size, sample, c, max_neigh=NULL,k=10) {
     return (.Call('mrfe_ne', A_size,
                    sample, c,
                   max_neigh, k))
}
