rm(list = ls())
setwd("/home/pmavrodiev/Documents/bats")

library(png)
library(Cairo)
library(R.utils)
library(debug)
library(animation)
library(lattice)
library(latticeExtra)        
library(grid)
library(gdata)
library(gplots)

  load("/mnt/ethz/home/Documents/bats/Grobs.RData")
  setwd("/home/pmavrodiev/Documents/bats/")
  #read the data frame holding the programming of the boxes
  programming = read.xls("explanations_GB2.xlsx",sheet=4,row.names=1,
                         col.names=c("","33A","33E","66A","66D","33B","33D","66B","33C","66C"),
                         colClasses="character")



  #main viewport
  #CairoPNG(file="Rplot%05d.png",width=1300,height=700,res=600) 
  setwd("/home/pmavrodiev/Documents/bats/graphs")
  min_travel_frequency=1
  max_travel_frequency=0
  graphFiles = dir(pattern="*.graph")
  #read all graph files once to determine the maximum travel frequency
  for (f in graphFiles) {
    graph=read.table(f,col.names=c("from","to","frequency"))
    print(paste("-",f,"-",sep=""))
    for (i in 1:nrow(graph)) {        
        local_max = max(graph[,3])
        if (local_max > max_travel_frequency) {
          max_travel_frequency = local_max
          print(f)
        }
    }
  }


  CairoPDF(file="movement_statistics.pdf",width=40,height=25)
  for (f in graphFiles) {
    grid.newpage()
    pushViewport(mainvp)    
    #add title
    titleGrob = textGrob(substr(f,1,10),x=0.5,y=1.47,name="title",gp=gpar(fontsize=15,cex=3))
    grid.draw(titleGrob)
    #add color legend
    N = 64 #number of colors
    colors=rich.colors(N)
    legendGrob = rasterGrob(rev(colors),x=0.95,y=0.55,height=0.6,width=0.015,just="bottom",                       
                            interpolate=FALSE,name="legend")
    grid.draw(legendGrob)
    colorIndex = seq(0,1,length=(N+1))
    #add numbers to legend
    numbersGrob = textGrob(c(0,0.2,0.4,0.6,0.8,1),x=0.97,y=seq(0.558,1.145,length=6),just="left",name="numbers",
                           gp=gpar(fontsize=10,cex=3))
    grid.draw(numbersGrob)
    #read files
    graph=read.table(f,col.names=c("from","to","frequency"))
    if (nrow(graph)==0) break
    max_lwd=15
    min_lwd=1.5
    for (i in 1:nrow(graph)) {
      box_from = toupper(as.character(graph[i,1]))
      box_to = toupper(as.character(graph[i,2]))
      total_activity = sum(graph[,3])
      weight = graph[i,3]/total_activity
      width = graph[i,3]
      #determine the color of this line based on the weight
      col_index = 0
      for (j in 1:(length(colorIndex)-1)) {
        if (colorIndex[j] <= weight && colorIndex[j+1] >= weight) {
          col_index = j
          break
        }
      }  
      box_from_idx = seq_along(boxes)[sapply(boxes, FUN=function(X) box_from %in% X)]
      box_to_idx = seq_along(boxes)[sapply(boxes, FUN=function(X) box_to %in% X)]
      if (length(box_from_idx) > 0 && length(box_to_idx) > 0) {
        box_from_x = boxes[[box_from_idx]]$x
        box_from_y = boxes[[box_from_idx]]$y
        box_to_x = boxes[[box_to_idx]]$x
        box_to_y = boxes[[box_to_idx]]$y
        #logarithmic line width
        C=log(max_travel_frequency)/(1-min_lwd/max_lwd)
        line_width=(C+log(width)-log(max_travel_frequency))/C          
        #gline = linesGrob(x=c(box_from_x,box_to_x),y=c(box_from_y,box_to_y),
        #                  name=paste("line-",box_from,"-",box_to,sep=""),
        #                  gp=gpar(lwd=max_lwd*line_width,col=colors[col_index]))
        gline = curveGrob(box_from_x,box_from_y,box_to_x,box_to_y,
                          name=paste("line-",box_from,"-",box_to,sep=""),
                          gp=gpar(lwd=max_lwd*line_width,col=colors[col_index]),
                          angle=110,curvature=0.6,shape=1)
        
        grid.draw(gline)
      }
    }
    
    for (j in 1:17) {
      batTag=substr(f,1,10)
      editedBoxGrob=boxes[[j]]
      if ((substr(boxes[[j]]$name,1,1) != "0" && 
        substr(programming[batTag,paste("X",boxes[[j]]$name,sep="")],1,1) == "Y") 
               || substr(boxes[[j]]$name,1,3) == "100") 
        editedBoxGrob=editGrob(boxes[[j]],gp=gpar(col="black",lwd=3))      
      grid.draw(editedBoxGrob)
      boxName = textGrob(boxes[[j]]$name,x=boxes[[j]]$x,y=boxes[[j]]$y,gp=gpar(cex=2))
      grid.draw(boxName)
    }
  } 
  for (j in 1:17) {
    batTag=substr(f,1,10)
    editedBoxGrob=boxes[[j]]
    if ((substr(boxes[[j]]$name,1,1) != "0" && 
      substr(programming[batTag,paste("X",boxes[[j]]$name,sep="")],1,1) == "Y"
         ) || substr(boxes[[j]]$name,1,3) == "100") 
        editedBoxGrob=editGrob(boxes[[j]],gp=gpar(col="black",lwd=3))
    
    grid.draw(editedBoxGrob)
    boxName = textGrob(boxes[[j]]$name,x=boxes[[j]]$x,y=boxes[[j]]$y,gp=gpar(cex=2))
    grid.draw(boxName)
  }
  dev.off()


#plot histograms of activities in each box
setwd("/home/pmavrodiev/Documents/bats/")
activity=read.table("aggregated_data.txt",colClasses="character")
activity.matrix=matrix(0,nrow=35,ncol=21,dimnames=list(unique(activity[,3]),unique(activity[,5])))
#activity.sorted = activity[order(activity[,3],activity[,5]),]
for (i in 1:nrow(activity)) {
  activity.matrix[activity[i,3],activity[i,5]] = activity.matrix[activity[i,3],activity[i,5]] + as.numeric(activity[i,4])
}


gg=function(i) {
  barcolors = rep("green",21)
  barcolors[which(substr(colnames(activity.matrix),1,3)=="100")] = "red" #mark the "all" boxes
  #the subst is needed to deal with a stupid empty space that appears sometimes, e.g. "Y "
  m=colnames(programming)[which(substr(programming[rownames(activity.matrix)[i],],1,1)=="Y")]
  mm=match(paste("X",toupper(colnames(activity.matrix)),sep="") ,m)
  barcolors[which(mm != "NA")] = "red"
  barchart((activity.matrix[i,]/sum(activity.matrix[i,]))~colnames(activity.matrix),main=list(rownames(activity.matrix)[i],cex=4),
           ylab=list("Frequency",cex=4),
           scales=list(cex=4),col=barcolors)
  
}

CairoPDF(file="movement_frequency.pdf",width=40,height=20)
gg(1)
gg(2)
gg(3)
gg(4)
gg(5)
gg(6)
gg(7)
gg(8)
gg(9)
gg(10)
gg(11)
gg(12)
gg(13)
gg(14)
gg(15)
gg(16)
gg(17)
gg(18)
gg(19)
gg(20)
gg(21)
gg(22)
gg(23)
gg(24)
gg(25)
gg(26)
gg(27)
gg(28)
gg(29)
gg(30)
gg(31)
gg(32)
gg(33)
gg(34)
gg(35)
gg(36)
gg(37)
dev.off()


#read and plot the leading following statistics
setwd("/home/pmavrodiev/Documents/bats/")
fileName = "lead-follow.txt"

lfdata = read.table(fileName)
empirical_cdf = ecdf(lfdata[,2])
tail_cdf = 1-empirical_cdf(sort(lfdata[,2]))
plot(sort(lfdata[,2]),tail_cdf,type="l")
normal_deviates = rnorm(1000,mean=mean(lfdata[,2]),sd=sd(lfdata[,2]))
normal_cdf = ecdf(normal_deviates)
normal_tail_cdf=1-normal_cdf(sort(normal_deviates))
lines(sort(normal_deviates),normal_tail_cdf)


hist(lfdata[,2],breaks=30)

CairoPDF(file="lead-follow-stats-3-minutes.pdf",width=10,height=10)
par(mar=c(8,9,1,1),mgp=c(6,2,0),pty="s")
plot(lfdata[,2],lfdata[,3],type="p",xlab="Number of Leading Events",ylab="Number of Following Events",cex.lab=4,cex.axis=4,pch=19,cex=4,asp=1,xlim=c(0,50),ylim=c(0,50))
dev.off()

##plot the chunk size distribution
setwd("/home/pmavrodiev/Documents/bats/")
fileName = "chunks_times_span.txt"
chunks = read.table(fileName,colClasses="character")
times = as.POSIXct(strptime(chunks[,1], "%H:%M:%S"))
min_limit = min(times)
max_limit = max(times)
time_breaks=seq(min_limit,max_limit,length=length(times))
hist(times,breaks=time_breaks,freq=FALSE)
mids=hist(times,breaks=time_breaks)$mids
counts=hist(times,breaks=time_breaks)$counts / sum(counts)


CairoPDF("chunk_time_span_distr.pdf",width=20,height=20)
par(mar=c(8,9,1,1),mgp=c(6,2,0),pty="s")
plot(times,ylab="Minutes",xlab="Chunk Number",cex.axis=4,cex.lab=4,type="p",pch=4,cex=3)
plot(time_breaks[which(counts!=0)],counts[counts!=0],type="p",pch=4,
     xlab="Minutes",ylab="PDF",cex.lab=4,cex.axis=4,cex=3)

plot(time_breaks[which(counts!=0)],log(counts[counts!=0]),type="p",pch=4,
     xlab="Minutes",ylab="log(PDF)",cex.lab=4,cex.axis=4,cex=3)
dev.off()

##plot the distribution of chunk sizes only for those chunk which
#recored a leading-following event

setwd("/home/pmavrodiev/Documents/bats/")
fileName = "lf_chunks_times_span.txt"
chunks = read.table(fileName,colClasses="character")
times = as.POSIXct(strptime(chunks[,1], "%H:%M:%S"))
min_limit = min(times)
max_limit = max(times)
time_breaks=seq(min_limit,max_limit,length=length(times))

hist(times,breaks=time_breaks,freq=FALSE,plot=FALSE)
mids=hist(times,breaks=time_breaks)$mids
counts=hist(times,breaks=time_breaks)$counts
counts =  counts / sum(counts)


CairoPDF("lf_chunk_time_span_distr.pdf",width=20,height=20)
par(mar=c(8,9,1,1),mgp=c(6,2,0),pty="s")
plot(times,ylab="Minutes",xlab="Chunk Number",cex.axis=4,cex.lab=4,type="p",pch=4,cex=3)
plot(time_breaks[which(counts!=0)],counts[counts!=0],type="p",pch=4,
     xlab="Minutes",ylab="PDF",cex.lab=4,cex.axis=4,cex=3)

plot(time_breaks[which(counts!=0)],log(counts[counts!=0]),type="p",pch=4,
     xlab="Minutes",ylab="log(PDF)",cex.lab=4,cex.axis=4,cex=3)
dev.off()





