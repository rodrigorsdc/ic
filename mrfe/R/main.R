mrfe_graph <- function(A_size, sample, c, k=NULL) {
    f <- function(A_size, sample, c, k).Call('mrfe_gr', A_size,
                                             sample, c, k);
    return (f(A_size, sample, c, k));
}
