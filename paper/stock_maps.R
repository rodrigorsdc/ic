library(maps)
library(mapdata)
library(mrfe)
source("util.R")

data <- as.matrix(read.table("stock_data_700"))
pdf("stock_non-conservative.pdf", width=13)
map('worldHires',c("USA","India","Brazil","France","Japan", "UK"),
    col="gray90",fill=TRUE, ylim=c(-40,80),
    xlim=c(-170,160),boundary=FALSE)
title("Stock market indexes - Graph\nNon-conservative approach")
text(-54,8,"Brazil")
text(130,45,"Japan")
text(-90, 55, "USA")
text(-12, 62, "UK")
text(5, 40, "France")
text(75, 0, "India")

list.adj <- mrfe_neigh(2L, data, 0.5)
matrix.adj.ncons <- list_to_matrix.ncons(list.adj)
matrix.adj.cons <- list_to_matrix.cons(list.adj)
print(matrix.adj.cons)

#Non-conservative
# Brazil - USA
lines(c(-49, -100), c(-10, 42))
# Brazil - UK
lines(c(-49, -1), c(-10, 53))
# UK - India
lines(c(-1, 80), c(53, 25))
# UK - France
lines(c(-1, 2), c(53, 45))
# UK - Japan
lines(c(-1, 140), c(53, 38))
# France - USA
lines(c(2, -100), c(45, 42))
# Japan - India
lines(c(140, 80), c(38, 25))

pdf("stock_conservative.pdf", width=13)
map('worldHires',c("USA","India","Brazil","France","Japan", "UK"),
    col="gray90",fill=TRUE, ylim=c(-40,80),
    xlim=c(-170,160),boundary=FALSE)
title("Stock market indexes - Graph\nConservative approach")
text(-54,8,"Brazil")
text(130,45,"Japan")
text(-90, 55, "USA")
text(-12, 62, "UK")
text(5, 40, "France")
text(75, 0, "India")

#Conservative
# Brazil - USA
lines(c(-49, -100), c(-10, 42))
# UK - India
lines(c(-1, 80), c(53, 25))
# UK - France
lines(c(-1, 2), c(53, 45))
# France - USA
lines(c(2, -100), c(45, 42))
# Japan - India
lines(c(140, 80), c(38, 25))

