rm(list = ls())
setwd("/home/pmavrodiev/Documents/bats/result_files/assortativity/2008")


library(Cairo)
library(R.utils)
library(lattice)
library(vioplot)

original=dir(pattern="2008_\\d_\\d_\\d{6}.txt.gz")
shuffled=dir(pattern="2008_\\d_\\d_\\d{6}_shuffled.txt.gz")

CairoPDF("2008_assortativity.pdf",width=24,height=16)
par(mfrow=c(1,2))
for (i in 1:length(original)) {
  #tokenize the filename  
  file.original=gzfile(original[i])  
  file.shuffled=gzfile(shuffled[i])
  list.original=strsplit(readLines(file.original), " ")
  list.shuffled=strsplit(readLines(file.shuffled), " ")
  #the assortativity coefficient in both graphs
  assort.coeff.original = as.numeric(list.original[[1]])
  assort.coeff.shuffled = as.numeric(list.shuffled[[1]])
  #create the variables
  list.original.length=length(list.original)
  violin.names=NULL  
  for (j in 2:list.original.length) {
    number=paste(j,sep="")
    if (j < 10) number=paste("0",j,sep="")
    assign(paste("A",number,sep=""),as.numeric(list.original[[j]][2:length(list.original[[j]])]))
    assign(paste("B",number,sep=""),as.numeric(list.shuffled[[j]][2:length(list.shuffled[[j]])]))
    violin.names=c(violin.names,as.numeric(list.original[[j]][1])) 
  }
  original.args=lapply(ls(pat="A\\d"),get)
  original.args[["col"]]="lightblue"
  original.args[["colMed"]]="blue"
  original.args[["names"]]=violin.names
  original.args[["ylim"]]=c(0,22)
  v=do.call(vioplot,original.args)
  lines(seq(1,list.original.length-1),v$median,type="o",pch=19,cex=1,col="blue",lwd=2)
  title(paste(assort.coeff.original,sep=""),cex.main=2)
  mtext(side=1,"<k>",line=3,cex=2)
  mtext(side=2,"<k>_nn",cex=2,line=2)
  ##
  shuffled.args=lapply(ls(pat="B\\d"),get)
  shuffled.args[["col"]]="lightblue"
  shuffled.args[["colMed"]]="blue"
  shuffled.args[["names"]]=violin.names
  shuffled.args[["ylim"]]=c(0,22)
  v2=do.call(vioplot,shuffled.args)
  lines(seq(1,list.original.length-1),v2$median,type="o",pch=19,cex=1,col="blue",lwd=2)
  title(paste(assort.coeff.shuffled,sep=""),cex.main=2)
  mtext(side=1,"<k>",line=3,cex=2)
  mtext(side=2,"<k>_nn",cex=2,line=2)   
  #
  do.call(rm,list(ls(pat="A\\d")))
  do.call(rm,list(ls(pat="B\\d")))
  #
  close(file.original)
  close(file.shuffled)
}
dev.off()


v=vioplot(A02,A03,A04,A05,A06,A07,A08,A09,A10,A11,
        col="lightblue",colMed="blue",names=violin.names,ylim=c(0,22))


v=vioplot(B2,B3,B4,B5,B6,B7,B8,B9,B10,B11,
          col="lightblue",colMed="blue",names=violin.names,ylim=c(0,22))
lines(seq(1,10),v$median,type="o",pch=19,cex=1,col="blue",lwd=2)
title("knowledge-delay_2_lf-delay_2_deadline-020000-shuffled",cex.main=2)
mtext(side=1,"<k>",line=3,cex=2)
mtext(side=2,"<k>_nn",cex=2,line=2)
dev.off()