library(Cairo)


setwd("/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/2007")

eigenvector.original = read.table("eigenvector_original_2007.dat")
eigenvector.original = sort(eigenvector.original[,2],decreasing=TRUE)

eigenvector.shuffled = read.table("eigenvector_shuffled_2007.dat")


eigenvector.shuffled.matrix = 
matrix(eigenvector.shuffled[,2],
    nrow = length(eigenvector.original),
    ncol = length(eigenvector.shuffled[,2]) / length(eigenvector.original))
X=seq(1,length(eigenvector.original))

eigenvector.shuffled.matrix.sorted = 
  apply(eigenvector.shuffled.matrix,2,sort,TRUE)

#remove cases when all centralities are 0
remove.idx = which(abs(eigenvector.shuffled.matrix.sorted[1,]) < 
                     .Machine$double.eps)
eigenvector.shuffled.matrix.sorted.cleaned = 
  eigenvector.shuffled.matrix.sorted
if (length(remove.idx)>0)
  eigenvector.shuffled.matrix.sorted.cleaned = 
    eigenvector.shuffled.matrix.sorted[,-remove.idx]
####
yminus = apply(eigenvector.shuffled.matrix.sorted.cleaned,1,
               quantile,c(0.025,0.95))[1,]

yplus = apply(eigenvector.shuffled.matrix.sorted.cleaned,1,
                       quantile,c(0.025,0.95))[2,]


ymedian = apply(eigenvector.shuffled.matrix.sorted.cleaned,1
                ,median)

plot(X,eigenvector.original,type="o",ylim=c(0,0.8))
polygon(x=c(X,rev(X)),
        y=c(yminus,rev(yplus)),
        col=rgb(0/255,0/255,0/255,alpha=0.3,maxColorValue=1),border=NA)
lines(X,ymedian,type="o",col="blue")

#now assortativity
setwd("/home/pmavrodiev/Documents/bats/result_files/assortativity")
assortativity.original=read.table("2008.txt")

assortativity.shuffled=read.table("2008_shuffled.txt")

assort_minus = quantile(assortativity.shuffled[,1],
                        probs=c(0.025,0.095))[1]
assort_plus = quantile(assortativity.shuffled[,1],
                        probs=c(0.025,0.095))[2]

larger = length(assortativity.shuffled[
         assortativity.shuffled[,1]>=assortativity.original[,1],1])/ nrow(assortativity.shuffled)

smaller = length(assortativity.shuffled[
  assortativity.shuffled[,1]<assortativity.original[,1],1])/ nrow(assortativity.shuffled)

