library("scatterplot3d")

scatterplot3d(c(19,397,325),c(300,304,242),c(0.716,2.41,0.23),xlab="x1",ylab="y1",zlab="effect",
              pch=19)

P.matrix = matrix(c(0.14,0.07,0,0.07,0.07,0,0.43,0.07,0.07,0,0.07,0),3,4)
#P.matrix = matrix(c(0.12,0.06,0.02,0.06,0.03,0.01,0.36,0.18,0.06,0.06,0.03,0.01),3,4)

Ie = 0
for (s in 1:4) {
  for (d in 1:3) {
    if (P.matrix[d,s] != 0) {
      Ie = P.matrix[d,s]*log(P.matrix[d,s] / (sum(P.matrix[,s])*sum(P.matrix[d,])))
    }
  }
}