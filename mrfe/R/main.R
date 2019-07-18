"mrfe" <- function(A, sample, c, max_neigh=NULL,k=10) {
     return (.Call('Rmrfe', A,
                    sample, c,
                   max_neigh, k))
}

"cv.mrfe" <- function(A, sample, c_lower, c_upper, c_step, k=10,
                      max_neigh=NULL) {
    return (.Call('Rmrfe_cv', A, sample, c(c_lower, c_upper, c_step),
                                           max_neigh, k))
}
