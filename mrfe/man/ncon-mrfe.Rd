\name{ncon.mrfe}
\encoding{utf-8}
\alias{ncon.mrfe}
%- Also NEED an '\alias' for EACH other topic documented here.
\title{
  Non-conservative approach for mrfe
}
\description{
  Non-conservative construction of the estimated Markov Random
  Field graph.
}
\usage{
ncon.mrfe(A, sample, c, max_neigh = NULL)

}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{A}{
    Size of the alphabet.
}
  \item{sample}{
    A integer-valued matrix. Each value must belong range 0 and \code{A} - 1.
}
  \item{c}{
      The penalization constant. Must be positive.
}
  \item{max_neigh}{
    The maximum length of a candidate Markovian neighborhood. Must be
    less or equal than \code{ncol(sample)}.
}
}
\value{
  A adjacency matrix of the estimated Markov Random Field graph.
}
\references{
  FRONDANA, Iara Moreira. \emph{Model selection for discrete Markov random
  fields on graphs}. São Paulo : Instituto de Matemática e Estatística,
  University of São Paulo, 2016. Doctoral Thesis in Estatística.
  \url{http://www.teses.usp.br/teses/disponiveis/45/45133/tde-02022018-151123/publico/tese_Iara_Frondana.pdf}

}
\author{
  Rodrigo Carvalho
}
\examples{
library(mrfe)
A = c(0, 1)
s = matrix(sample(A, size=1000, replace=TRUE), ncol=5)
ncon.mrfe(length(A), s, 1.0)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
%\keyword{ ~kwd1 }% use one of  RShowDoc("KEYWORDS")
%\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
