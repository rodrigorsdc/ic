\name{mrfe}
\alias{mrfe}
%- Also NEED an '\alias' for EACH other topic documented here.
\title{
  Frondana's Markov Random Field estimator
}
\description{
  Returns a Markovian neighborhood for each graph vertex
}
\usage{
mrfe(A, sample, c, max_neigh=NULL)
mrfe(A, sample, c)
mrfe(A, sapmle, c, max_neigh)
}

\arguments{
  \item{A}{
    Size of the alphabet.
}
  \item{sample}{
    A integer-valued matrix. Each value may be belong range 0 and 'A' - 1.
}
  \item{c}{
    The penalization constant. May be positive.
}
  \item{max_neigh}{
    The maximum length of a candidated Markovian neighborhood.
}

}
\details{
%%  ~~ If necessary, more details than the description above ~~
}
\value{
  A list filled with Markov neighborhood estimated for each graph vertex
}
\references{
  FRONDANA, Iara Moreira. Model selection for discrete Markov random
  fields on graphs. São Paulo : Instituto de Matemática e Estatística,
  University of São Paulo, 2016. Doctoral Thesis in Estatística.
  \url{http://www.teses.usp.br/teses/disponiveis/45/45133/tde-02022018-151123/publico/tese_Iara_Frondana.pdf}
}
\author{
  Rodrigo Carvalho
}
\note{
%%  ~~further notes~~
}

%% ~Make other sections like Warning with \section{Warning }{....} ~

\seealso{
%% ~~objects to See Also as \code{\link{help}}, ~~~
}
\examples{
library(mrfe)
A = c(0, 1)
s = matrix(sample(A, size=1000), ncol=5)
mrfe(length(A), s, 1.0)
}


