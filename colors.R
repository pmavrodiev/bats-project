library("rgl")
library(R.utils)
library(plotrix)
library(fields)
library(Cairo)
library(colorRamps)


nColors = 128
ramp = colorRamp(c("yellow","violet","blue","red","green"))
ramp = topo.colors(nColors)
#cols = rgb(ramp(seq(0,1,length=nColors)),max=255)
cols=ramp
colindex = as.integer(cut(seq(0,56,length=128),breaks=nColors))
#colors = tim.colors(nColors)[colindex]
setwd("/home/pmavrodiev/Documents/bats/")

CairoPNG(filename="color_scale_indegree.png",width=1200,height=1200,pointsize=14)
par(mar=c(5,6,4,2),ylbias=0,yaxs="i")
plot(rep(50,128),seq(0,56,length=128),cex=8,col=cols[colindex],pch=15,xlim=c(45,55),cex.lab=4,cex.axis=4,ylab="",xlab="",xaxt="n")
dev.off()
colors = cols[colindex]

for (i in 1:129)
  print(paste(seq(0,56,length=129)[i],",",sep=""))

t=col2rgb(cols)/255
for (i in 1:128)
  print(paste(t["blue",i],",",sep=""))
