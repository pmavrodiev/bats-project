axp <- par("xaxp") #-> powers of 10 *inside* ?usr?
axp[3] <- 1 # such that only 10^. are labeled
aX <- axTicks(1, axp = 10^axp)
xu <- 10 ^ par("usr")[1:2]
e10 <- c(-1,1) + round(axp[1:2]) ## exponents of 10 *outside* ?usr?
v <- c(outer(1:9, e10[1]:e10[2], function(x,E) x * 10 ^ E))
v <- v[xu[1] <= v & v <= xu[2]]


ayp <- par("yaxp") #-> powers of 10 *inside* ?usr?
ayp[3] <- 1 # such that only 10^. are labeled
aY <- axTicks(2, axp = ayp)
yu <- 10 ^ par("usr")[1:2]
e10 <- c(-1,1) + round(ayp[1:2]) ## exponents of 10 *outside* ?usr?
w <- c(outer(1:9, e10[1]:e10[2], function(x,E) x * 10 ^ E))
#w <- w[yu[1] <= w & w <= yu[2]]