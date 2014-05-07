library("data.table")
library("Matching")
library(Cairo)

tail_cdf = function(data) { 
  data.sorted=sort(data,decreasing=TRUE)
  return (1-ecdf(data)(data.sorted))  
}

setwd("/home/pmavrodiev/Documents/bats/result_files/")

datfiles=list.files(path=getwd(),pattern="lf_seqs_summary", 
                    all.files=FALSE,
                    full.names=TRUE)

datfiles.merged=data.table(do.call("rbind",lapply(datfiles,
                                FUN=function(files)
                          {read.table(files,header=FALSE, 
                                      colClasses="numeric",sep="\t")})))
                        

setnames(datfiles.merged,old=c("V1","V2"),new=c("seq_length","number"))

max_length=max(datfiles.merged$seq_length)
sequences=seq(1,max_length)
freqs=rep(NA,max_length)

for (i in 1:max_length) {
  sub_data = subset(datfiles.merged,seq_length==i)
  freqs[i] = sum(sub_data$number)
}

file.name=paste(getwd(),"/lf-sequences.pdf",sep="")  
CairoPDF(file=file.name,width=13,height=10)
par(mar=c(7,8,1,1),mgp=c(1,2,0))
plot(seq(1,max_length),freqs/sum(freqs),xlim=c(1,max_length),
     ylim=c(0,0.7),xaxt="n",yaxt="n",type="c",xlab="",ylab="",lwd=3)
points(seq(1,max_length),freqs/sum(freqs),bg="grey",pch=21,cex=3)
axis(1,at=seq(1,16),label=seq(1,16),cex.axis=2,cex.lab=3)
axis(2,at=seq(0,0.7,by=0.1),label=seq(0,0.7,by=0.1),cex.axis=2,
     cex.lab=3,las=2,hadj=0.8)
mtext("L/F chain length",side=1,cex=2,line=5.5)
mtext("Probability of occurrence",side=2,cex=2,line=5.5)
dev.off()  






