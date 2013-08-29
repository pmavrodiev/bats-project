library(Cairo)
library(Matching)
library(stats)
library(nortest)
library(igraph)

directory = "/home/pmavrodiev/Documents/bats/result_files/output_files_new_2/"

setwd(directory)

model = 5

original="combined-original-evs.txt"
shuffled=paste("combined-model-",model,".dat",sep="")

eigenvector.original = read.table(original)
eigenvector.original = sort(eigenvector.original[,2],decreasing=TRUE)

eigenvector.shuffled = read.table(shuffled)

eigenvector.shuffled.matrix = 
matrix(eigenvector.shuffled[,2],
    nrow = length(eigenvector.original),
    ncol = length(eigenvector.shuffled[,2]) / length(eigenvector.original))
X=seq(1,length(eigenvector.original))

eigenvector.shuffled.matrix.sorted = 
  apply(eigenvector.shuffled.matrix,2,sort,TRUE)

#remove cases when all centralities are 0
remove.idx = which(abs(eigenvector.shuffled.matrix.sorted[1,]) < 1e-10)

eigenvector.shuffled.matrix.sorted.cleaned = 
  eigenvector.shuffled.matrix.sorted
if (length(remove.idx)>0)
  eigenvector.shuffled.matrix.sorted.cleaned = 
    eigenvector.shuffled.matrix.sorted[,-remove.idx]
####
yminus = apply(eigenvector.shuffled.matrix.sorted.cleaned,1,
               quantile,c(0.025,0.975))[1,]

yplus = apply(eigenvector.shuffled.matrix.sorted.cleaned,1,
                       quantile,c(0.025,0.975))[2,]


ymedian = apply(eigenvector.shuffled.matrix.sorted.cleaned,1
                ,median)


# h0=hist(eigenvector.shuffled.matrix.sorted.cleaned[1,],breaks=50,plot=FALSE)
# plot(h0$mids,h0$counts/sum(h0$counts),type="o")
# 
# h=hist(rnorm(1000000,mean(eigenvector.shuffled.matrix.sorted.cleaned[1,]),sd(eigenvector.shuffled.matrix.sorted.cleaned[1,])),breaks=50,plot=FALSE)
# lines(h$mids,h$counts/sum(h$counts),col="blue")
# 
# ks.test(eigenvector.shuffled.matrix.sorted.cleaned[1,],"pnorm",mean(eigenvector.shuffled.matrix.sorted.cleaned[1,]),sd(eigenvector.shuffled.matrix.sorted.cleaned[1,]))
# 
# shapiro.test(eigenvector.shuffled.matrix.sorted.cleaned[1,])
# qqnorm(eigenvector.shuffled.matrix.sorted.cleaned[1,])
# ad.test(eigenvector.shuffled.matrix.sorted.cleaned[1,])
# 
# dlnorm(0.691963,mean(log(eigenvector.shuffled.matrix.sorted.cleaned[1,])),sd(log(eigenvector.shuffled.matrix.sorted.cleaned[1,])))
# 
# pnorm(0.385007,mean(eigenvector.shuffled.matrix.sorted.cleaned[2,]),sd(eigenvector.shuffled.matrix.sorted.cleaned[2,]))
# 


p.values = rep(NA,nrow(eigenvector.shuffled.matrix.sorted))
for (r in 1:nrow(eigenvector.shuffled.matrix.sorted)) {
  p.values[r] = length(which(eigenvector.shuffled.matrix.sorted.cleaned[r,] > 
           eigenvector.original[r])) / length(eigenvector.shuffled.matrix.sorted[r,])
                    
}

points.pch = ifelse(p.values<0.05,16,4)
CairoPDF(file=paste("combined-model-",model,".pdf",sep=""),width=12,height=10)
plot(X,eigenvector.original,type="o",ylim=c(0,1),lwd=3,cex=2,
     pch=points.pch,cex.axis=2,xlab="rank",ylab="eigenvector centrality",
     cex.lab=2)
polygon(x=c(X,rev(X)),
        y=c(yminus,rev(yplus)),
        col=rgb(0/255,0/255,255/255,alpha=0.3,maxColorValue=1),border=NA)
lines(X,ymedian,type="o",col="blue",lwd=3,cex=2,pch=4)
legend("topright",paste("Model",model,sep=""),cex=3)
dev.off()

_#now assortativity
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



g1=graph(c(1,2,1,3,2,1,2,3,4,3))
m=get.adjacency(g1,sparse=FALSE)
plot(g1)
#e=evcent(g1,directed=TRUE,scale=FALSE)
eigen(t(m))


g2=graph(c(1,2,1,3,2,1,2,3,4,3))
m=get.adjacency(g1,sparse=FALSE)
plot(g1)
#e=evcent(g1,directed=TRUE,scale=FALSE)
eigen(t(m))




