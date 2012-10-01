rm(list = ls())
setwd("/home/pmavrodiev/Documents/bats")
library(fields)
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
library(reldist)

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

tail_cdf = function(posixvector) {  
  v_sorted = sort(posixvector)
  v_unique = unique(v_sorted)
  return_vector = rep(NA,length(v_unique))
  for (i in 1:length(v_unique)) {
    return_vector[i] = length(which(v_sorted > v_unique[i])) / length(v_sorted)
  }
  return (return_vector)
}


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

##plot the distribution of chunk sizes only for those chunks which
#recored a leading-following event

setwd("/home/pmavrodiev/Documents/bats/result_files/")
fileName = "lf_chunks_times_span_2008.txt"
#all lead-follow events were recorded in chunks of less than 10 minutes.
#in this way phoney lf events from swarming are avoided.
#fileName = "lf_chunks_times_span_10mins.txt" 
chunks = read.table(fileName,colClasses="character")
times = as.POSIXct(strptime(chunks[,1], "%H:%M:%S"))
min_limit = min(times)
max_limit = max(times)
time_breaks=seq(min_limit,max_limit,length=length(times))

tcdf = tail_cdf(times)
#CairoPDF("lf_chunk_time_span_distr.pdf",width=20,height=20)
CairoPDF("lf_chunk_time_span_distr_2008.pdf",width=20,height=20)
par(mar=c(8,9,1,1),mgp=c(6,2,0),pty="s")
plot(times,ylab="Minutes",xlab="Chunk Number",cex.axis=4,cex.lab=4,type="p",pch=4,cex=3)
plot(unique(sort(times)),tcdf,type="l",lwd=4,
     xlab="Minutes",ylab="Tail CDF",cex.lab=4,cex.axis=4,cex=3)
legend("topright",paste("Sample Size=",length(times),sep=""),cex=3)

plot(unique(sort(times)),log(tcdf),type="l",lwd=4,
     xlab="Minutes",ylab="Tail CDF (log)",cex.lab=4,cex.axis=4,cex=3)
dev.off()

#plot the time differences between a leader and a follower for all pairs
setwd("/home/pmavrodiev/Documents/bats/result_files/")
#fileName = "lf_chunks_times_span.txt"
#all lead-follow events were recorded in chunks of less than 10 minutes.
#in this way phoney lf events from swarming are avoided.
fileName = "time_diff_lf_2008.txt" 
chunks = read.table(fileName,colClasses="character")
times = as.POSIXct(strptime(chunks[,1], "%H:%M:%S"))
min_limit = min(times)
max_limit = max(times)
time_breaks=seq(min_limit,max_limit,length=length(times))


tcdf = tail_cdf(times)
CairoPDF("lf_time_diff_2008.pdf",width=20,height=20)
par(mar=c(8,9,1,1),mgp=c(6,2,0),pty="s",yaxs="i",xaxs="i")
plot(unique(sort(times)),tcdf,type="l",cex.axis=4,cex.lab=4,lwd=4,cex=3,xlab="Time difference (minutes)",ylab="Tail CDF")
legend("topright",paste("Sample Size=",length(times),sep=""),cex=3)
#segments(as.POSIXct(strptime("00:05:04","%H:%M:%S")),0,as.POSIXct(strptime("00:05:04","%H:%M:%S")),tcdf[205],lwd=4,col="blue")
#segments(as.POSIXct(strptime("00:00:00","%H:%M:%S")),tcdf[205],as.POSIXct(strptime("00:05:04","%H:%M:%S")),tcdf[205],lwd=4,col="blue")
plot(unique(sort(times)),log(tcdf),type="l",cex.axis=4,cex.lab=4,lwd=4,cex=3,xlab="Time difference (minutes)",ylab="Tail CDF (log)")
legend("topright",paste("Sample Size=",length(times),sep=""),cex=3)

dev.off()


#plot the time differences between a leader and a follower for valid pairs only
setwd("/home/pmavrodiev/Documents/bats/result_files/")
fileName = "time_diff_valid_lf_3am_2008.txt" 
chunks = read.table(fileName,colClasses="character")
times = as.POSIXct(strptime(chunks[,1], "%H:%M:%S"))
min_limit = min(times)
max_limit = max(times)
time_breaks=seq(min_limit,max_limit,length=length(times))


tcdf = tail_cdf(times)
CairoPDF("lf_valid_time_diff_3am_2008.pdf",width=20,height=20)
par(mar=c(8,9,1,1),mgp=c(6,2,0),pty="s",yaxs="i",xaxs="i")
plot(unique(sort(times)),tcdf,type="l",cex.axis=4,cex.lab=4,lwd=4,cex=3,xlab="Time difference (minutes)",ylab="Tail CDF")
legend("topright",paste("Sample Size=",length(times),sep=""),cex=3)
#segments(as.POSIXct(strptime("00:05:04","%H:%M:%S")),0,as.POSIXct(strptime("00:05:04","%H:%M:%S")),tcdf[205],lwd=4,col="blue")
#segments(as.POSIXct(strptime("00:00:00","%H:%M:%S")),tcdf[205],as.POSIXct(strptime("00:05:04","%H:%M:%S")),tcdf[205],lwd=4,col="blue")
plot(unique(sort(times)),log(tcdf),type="l",cex.axis=4,cex.lab=4,lwd=4,cex=3,xlab="Time difference (minutes)",ylab="Tail CDF (log)")
legend("topright",paste("Sample Size=",length(times),sep=""),cex=3)

dev.off()


#plot all distributions of lf_chunks on 1 plot
setwd("/home/pmavrodiev/Documents/bats/result_files/output_files")
fileNamePatterns = "lf_chunk_time_span_distr_2008_" 
files = list.files(getwd(),pattern=fileNamePatterns)
time_data = matrix(as.POSIXct(strptime("00:00:00","%H:%M:%S")),
                   length(files),90) #90 >= length(tcdf) for the longest vector

CairoPDF("lf_chunk_time_span_distr_2008.pdf",width=20,height=20)
par(mar=c(8,9,4,1),mgp=c(6,2,0),pty="s",yaxs="i",xaxs="i")
chunk_boundaries = c(2,3,5,7,9)
y1 = NULL;y2 = NULL;y3 = NULL;y4 = NULL;
x1 = NULL;x2 = NULL;x3 = NULL;x4 = NULL;

for (j in 1:length(chunk_boundaries)) {
  counter = 1
  for (i in 1:length(files)) {
    tokenized_str = strsplit(files[i],"\\D")
    year = as.numeric(tokenized_str[[1]][26])
    time_chunk = as.numeric(tokenized_str[[1]][27])
    lf_delay = as.numeric(tokenized_str[[1]][28])
    occupation_deadline = tokenized_str[[1]][29]
    if (time_chunk == chunk_boundaries[j] && lf_delay==2) { #lf_delay is irrelevant
      chunks = read.table(files[i],colClasses="character")
      times = as.POSIXct(strptime(chunks[,1], "%H:%M:%S"))     
      tcdf = tail_cdf(times)      
      if (counter == 1) {x1=times;y1 = tcdf}
      else if (counter == 2){x2=times;y2 = tcdf}
      else if (counter == 3){x3=times;y3 = tcdf}
      else if (counter == 4) {x4=times;y4 = tcdf}
      counter = counter + 1
    }
    #legend("topright",paste("Sample Size=",length(times),sep=""),cex=3)
  }
  plot(unique(sort(x1)),y1,type="o",cex.axis=4,cex.lab=4,lwd=2,pch=6,
       cex=2,xlab="Chunk length (minutes)",ylab="Tail CDF",ylim=c(0,1),
       xlim=c(as.POSIXct(strptime("00:00:00","%H:%M:%S")),
              as.POSIXct(strptime("00:50:00","%H:%M:%S"))),
       main=paste("Chunks boundary = ",chunk_boundaries[j]," minutes",sep=""),cex.main=4)
  lines(unique(sort(x2)),y2,cex.axis=4,cex.lab=4,lwd=2,cex=2,pch=0,type="o")
  lines(unique(sort(x3)),y3,cex.axis=4,cex.lab=4,lwd=2,cex=2,pch=3,type="o")
  lines(unique(sort(x4)),y4,cex.axis=4,cex.lab=4,lwd=2,cex=2,pch=5,type="o")  
  legend("topright",c(paste("2am / ",length(y1),sep=""),
                      paste("3am / ",length(y2),sep=""),
                      paste("5am / ",length(y3),sep=""),
                      paste("8am / ",length(y4),sep="")),lwd=c(2,2,2,2),pch=c(6,0,3,5),cex=4)  
}
dev.off()



#plot all distributions of time differences b/n leader and follower for all pairs on 1 plot
setwd("/home/pmavrodiev/Documents/bats/result_files/output_files")
fileNamePatterns = "lf_time_diff_2008_" 
files = list.files(getwd(),pattern=fileNamePatterns)
time_data = matrix(as.POSIXct(strptime("00:00:00","%H:%M:%S")),
                   length(files),90) #90 >= length(tcdf) for the longest vector

CairoPDF("lf_time_diff_2008.pdf",width=20,height=20)
par(mar=c(8,9,4,1),mgp=c(6,2,0),pty="s",yaxs="i",xaxs="i")
chunk_boundaries = c(2,3,5,7,9)
lf_delays = c(2)
y1 = NULL;y2 = NULL;y3 = NULL;y4 = NULL;
x1 = NULL;x2 = NULL;x3 = NULL;x4 = NULL;

for (j in 1:length(chunk_boundaries)) {
  for (k in 1:length(lf_delays)) {
    counter = 1
    for (i in 1:length(files)) {
      tokenized_str = strsplit(files[i],"\\D")
      year = as.numeric(tokenized_str[[1]][14])
      time_chunk = as.numeric(tokenized_str[[1]][15])
      lf_delay = as.numeric(tokenized_str[[1]][16])
      occupation_deadline = tokenized_str[[1]][17]
      if (time_chunk == chunk_boundaries[j] && lf_delay==lf_delays[k]) {
        chunks = read.table(files[i],colClasses="character")
        times = as.POSIXct(strptime(chunks[,1], "%H:%M:%S"))     
        tcdf = tail_cdf(times)      
        if (counter == 1) {x1=times;y1 = tcdf}
        else if (counter == 2){x2=times;y2 = tcdf}
        else if (counter == 3){x3=times;y3 = tcdf}
        else if (counter == 4) {x4=times;y4 = tcdf}
        counter = counter + 1
      }
    #legend("topright",paste("Sample Size=",length(times),sep=""),cex=3)
    }
    plot(unique(sort(x1)),y1,type="o",cex.axis=4,cex.lab=4,lwd=2,pch=6,
         cex=2,xlab="Time difference (minutes)",ylab="Tail CDF",ylim=c(0,1),
         xlim=c(as.POSIXct(strptime("00:00:00","%H:%M:%S")),
              as.POSIXct(strptime("00:50:00","%H:%M:%S"))),
         main=paste("Chunks boundary = ",chunk_boundaries[j]," minutes",sep=""),cex.main=4)
    lines(unique(sort(x2)),y2,cex.axis=4,cex.lab=4,lwd=2,cex=2,pch=0,type="o")
    lines(unique(sort(x3)),y3,cex.axis=4,cex.lab=4,lwd=2,cex=2,pch=3,type="o")
    lines(unique(sort(x4)),y4,cex.axis=4,cex.lab=4,lwd=2,cex=2,pch=5,type="o")  
    legend("topright",c(paste("2am / ",length(y1),sep=""),
                        paste("3am / ",length(y2),sep=""),
                        paste("5am / ",length(y3),sep=""),
                        paste("8am / ",length(y4),sep="")),lwd=c(2,2,2,2),pch=c(6,0,3,5),cex=4)  
  }
}
dev.off()




#plot all distributions of time differences b/n leader and follower for all valid pairs
setwd("/home/pmavrodiev/Documents/bats/result_files/output_files/2008")
fileNamePatterns = "lf_valid_time_diff_2008_" 
files = list.files(getwd(),pattern=fileNamePatterns)

CairoPDF("lf_valid_time_diff_2008.pdf",width=20,height=20)
par(mar=c(8,9,4,1),mgp=c(6,2,0),pty="s",yaxs="i",xaxs="i")
chunk_boundaries = c(2,3,5,7,9)
lf_delays = c(2,3,5,7,9)
y1 = NULL;y2 = NULL;y3 = NULL;y4 = NULL;
x1 = NULL;x2 = NULL;x3 = NULL;x4 = NULL;

for (j in 1:length(chunk_boundaries)) {
  for (k in 1:length(lf_delays)) {
    counter = 1
    for (i in 1:length(files)) {
      tokenized_str = strsplit(files[i],"\\D")
      year = as.numeric(tokenized_str[[1]][20])
      time_chunk = as.numeric(tokenized_str[[1]][21])
      lf_delay = as.numeric(tokenized_str[[1]][22])
      occupation_deadline = tokenized_str[[1]][23]
      if (time_chunk == chunk_boundaries[j] && lf_delay==lf_delays[k]) {
        chunks = read.table(files[i],colClasses="character")
        times = as.POSIXct(strptime(chunks[,1], "%H:%M:%S"))     
        tcdf = tail_cdf(times)      
        if (counter == 1) {x1=times;y1 = tcdf}
        else if (counter == 2){x2=times;y2 = tcdf}
        else if (counter == 3){x3=times;y3 = tcdf}
        else if (counter == 4) {x4=times;y4 = tcdf}
        counter = counter + 1
      }
      #legend("topright",paste("Sample Size=",length(times),sep=""),cex=3)
    }
    plot(unique(sort(x1)),y1,type="o",cex.axis=4,cex.lab=4,lwd=2,pch=6,
         cex=2,xlab="Time difference (minutes)",ylab="Tail CDF",ylim=c(0,1),
         xlim=c(as.POSIXct(strptime("00:00:00","%H:%M:%S")),
                as.POSIXct(strptime(paste("00:0",lf_delays[k],":00",sep=""),"%H:%M:%S"))),
         main=paste("Chunks boundary = ",chunk_boundaries[j]," minutes",
                    " / LF dist. = ",lf_delays[k]," minutes",sep=""),cex.main=4)
    lines(unique(sort(x2)),y2,cex.axis=4,cex.lab=4,lwd=2,cex=2,pch=0,type="o")
    lines(unique(sort(x3)),y3,cex.axis=4,cex.lab=4,lwd=2,cex=2,pch=3,type="o")
    lines(unique(sort(x4)),y4,cex.axis=4,cex.lab=4,lwd=2,cex=2,pch=5,type="o")  
    legend("topright",c(paste("2am / ",length(y1),sep=""),
                        paste("3am / ",length(y2),sep=""),
                        paste("5am / ",length(y3),sep=""),
                        paste("8am / ",length(y4),sep="")),lwd=c(2,2,2,2),pch=c(6,0,3,5),cex=4)  
  }
}
dev.off()

#plot the distributions of the final page ranks for all parameter combinations
CairoPDF(file="final_pageranks.pdf",width=40,height=20)
par(mar=c(22,9,5,1),mgp=c(6,1,0),yaxs="i",xaxs="i")
setwd("/home/pmavrodiev/Documents/bats/result_files/output_files/2008/")
fileNamePatterns = "lead_follow_fpr_2008_2_\\d_020000"
files = list.files(getwd(),pattern=fileNamePatterns)
data = read.table(files[1],fill=TRUE,colClasses = "character")
data.sorted = data[order(data[,2],decreasing=TRUE),]
ginis = NULL
col_names_vec = NULL
for (i in 1:length(files)) {  
  tokenized_str = strsplit(files[i],"\\D")
  year = as.numeric(tokenized_str[[1]][17])
  time_chunk = as.numeric(tokenized_str[[1]][18])
  lf_delay = as.numeric(tokenized_str[[1]][19])
  occupation_deadline = tokenized_str[[1]][20]
  col_name = paste(time_chunk,"/",lf_delay,"/",occupation_deadline,sep="")
  col_names_vec = c(col_names_vec,col_name)
  data = read.table(files[i],fill=TRUE,colClasses = "character")
  data.sorted = data[order(data[,2],decreasing=TRUE),] 
  barplot(500*as.numeric(data.sorted[1:(nrow(data.sorted)-1),2]),
          names.arg=data.sorted[1:(nrow(data.sorted)-1),1],
          beside=TRUE,las=2,cex.axis=4,cex.names=4,ylab="Page Rank",
          cex.lab=4,cex.main=4,
          main=paste("Chunks boundary = ",time_chunk," minutes / LF dist. = ",
                     lf_delay," minutes / Occupation deadline = ",
                     occupation_deadline,sep=""))
  ginis=c(ginis,gini(as.numeric(data.sorted[1:(nrow(data.sorted)-1),2])))
}
plot(seq(1:length(files)),
     ginis,xaxt="n",yaxs="i",xaxs="i",xlab="",ylab="Gini coefficient",
     type="o",pch=4, cex.lab=4,cex.main=4,cex.axis=4,lwd=3,cex=3)
axis(1,at=seq(1:length(files)),
     labels=col_names_vec,las=2,cex.axis=3)

dev.off()


#plot individual page ranks over all parameter configurations
setwd("/home/pmavrodiev/Documents/bats/result_files/output_files")
fileNamePatterns = "lead_follow_fpr_2008_"
files = list.files(getwd(),pattern=fileNamePatterns)
col_names = NULL

#first loop through the files to get all parameter combinations
for (i in 1:length(files)) {
  tokenized_str = strsplit(files[i],"\\D")
  year = as.numeric(tokenized_str[[1]][17])
  time_chunk = as.numeric(tokenized_str[[1]][18])
  lf_delay = as.numeric(tokenized_str[[1]][19])
  occupation_deadline = tokenized_str[[1]][20]
  col_names = c(col_names,
                paste(time_chunk,"/",lf_delay,"/",occupation_deadline,sep=""))
}
data = read.table(files[1],fill=TRUE,colClasses = "character")
data.sorted = data[order(data[,2],decreasing=TRUE),]
row_names = data.sorted[1:(nrow(data.sorted)-1),1]
dataframe = matrix(NA,nrow=length(row_names),ncol=length(col_names))
rownames(dataframe)=row_names
colnames(dataframe)=col_names

for (i in 1:length(files)) {  
  tokenized_str = strsplit(files[i],"\\D")
  year = as.numeric(tokenized_str[[1]][17])
  time_chunk = as.numeric(tokenized_str[[1]][18])
  lf_delay = as.numeric(tokenized_str[[1]][19])
  occupation_deadline = tokenized_str[[1]][20]
  col_name = paste(time_chunk,"/",lf_delay,"/",occupation_deadline,sep="")
  data = read.table(files[i],fill=TRUE,colClasses = "character")
  data.sorted = data[order(data[,2],decreasing=TRUE),]
  dataframe[data.sorted[1:(nrow(data.sorted)-1),1],col_name] = 
    as.numeric(data.sorted[1:(nrow(data.sorted)-1),2])   
}



CairoPDF(file="final_pageranks_over_time.pdf",width=40,height=30)
par(mar=c(22,11,5,10),mgp=c(6,1,0),yaxs="i",xaxs="i")
#start preparing the mega cool plot
nbats = 35#nrow(data)-1
ylim_vec = c(0,nbats)
rownames_last4 = substr(row_names[1:35],7,10)

#calculate the rank of each bat over all parameter combinations
occupationDeadline = "2am"
select_pattern_2am=seq(1,length(col_names),by=4)
select_pattern_3am=seq(2,length(col_names),by=4)
select_pattern_5am=seq(3,length(col_names),by=4)
select_pattern_8am=seq(4,length(col_names),by=4)
dataframe_ranks = matrix(NA,nrow=nbats,
                         ncol=length(select_pattern_2am))
rownames(dataframe_ranks)=row_names[1:nbats]
colnames(dataframe_ranks)=col_names[select_pattern_2am]

#2am
for (j in 1:length(select_pattern_2am)) {
  dataframe_ranks[,j] = rank(-dataframe[1:nbats,select_pattern_2am[j]],
                             ties.method="first")
}
#plot the ranks of the bats now
for (j in 1:nbats) {
  if (j==1) { #set up the plot
    plot(seq(1:length(select_pattern_2am)),dataframe_ranks[j,]-0.5,
         xaxt="n",yaxs="i",xaxs="i",xlab="",yaxt="n",ylab="",
         ylim=c(0,nbats),type="o",pch=4,col=j,
         cex.axis=4,cex.lab=4,lwd=4,cex.main=4,
         main=paste("Occupation deadline = ",occupationDeadline,sep=""))
    axis(1,at=seq(1:length(select_pattern_2am)),labels=col_names[select_pattern_2am],las=2,cex.axis=4)
    axis(2,at=seq(1:nbats),labels=rownames_last4,las=2,padj=1.2,cex.axis=4)
    axis(4,at=seq(1:nbats),las=2,padj=1.2,cex.axis=4)
    mtext("Rank",side=4,line=8,cex=4)
    #show the lines now
    for (j in 1:nbats)
      abline(h=j,col="grey")
  }
  else {
    lines(seq(1:length(select_pattern_2am)),dataframe_ranks[j,]-0.5,
          type="o",pch=4,col=j,lwd=4,cex=3)
  }
}

#3am
occupationDeadline = "3am"
for (j in 1:length(select_pattern_3am)) {
  dataframe_ranks[,j] = rank(-dataframe[1:nbats,select_pattern_3am[j]],
                             ties.method="first")
}
#plot the ranks of the bats now
for (j in 1:nbats) {
  if (j==1) { #set up the plot
    plot(seq(1:length(select_pattern_3am)),dataframe_ranks[j,]-0.5,
         xaxt="n",yaxs="i",xaxs="i",xlab="",yaxt="n",ylab="",
         ylim=c(0,nbats),type="o",pch=4,col=j,
         cex.axis=4,cex.lab=4,lwd=4,cex.main=4,
         main=paste("Occupation deadline = ",occupationDeadline,sep=""))
    axis(1,at=seq(1:length(select_pattern_3am)),labels=col_names[select_pattern_3am],las=2,cex.axis=4)
    axis(2,at=seq(1:nbats),labels=rownames_last4,las=2,padj=1.2,cex.axis=4)
    axis(4,at=seq(1:nbats),las=2,padj=1.2,cex.axis=4)
    mtext("Rank",side=4,line=8,cex=4)
    #show the lines now
    for (j in 1:nbats)
      abline(h=j,col="grey")
  }
  else {
    lines(seq(1:length(select_pattern_3am)),dataframe_ranks[j,]-0.5,
          type="o",pch=4,col=j,lwd=4,cex=3)
  }
}

#5am
occupationDeadline = "5am"
for (j in 1:length(select_pattern_5am)) {
  dataframe_ranks[,j] = rank(-dataframe[1:nbats,select_pattern_5am[j]],
                             ties.method="first")
}
#plot the ranks of the bats now
for (j in 1:nbats) {
  if (j==1) { #set up the plot
    plot(seq(1:length(select_pattern_5am)),dataframe_ranks[j,]-0.5,
         xaxt="n",yaxs="i",xaxs="i",xlab="",yaxt="n",ylab="",
         ylim=c(0,nbats),type="o",pch=4,col=j,
         cex.axis=4,cex.lab=4,lwd=4,cex.main=4,
         main=paste("Occupation deadline = ",occupationDeadline,sep=""))
    axis(1,at=seq(1:length(select_pattern_5am)),labels=col_names[select_pattern_5am],las=2,cex.axis=4)
    axis(2,at=seq(1:nbats),labels=rownames_last4,las=2,padj=1.2,cex.axis=4)
    axis(4,at=seq(1:nbats),las=2,padj=1.2,cex.axis=4)
    mtext("Rank",side=4,line=8,cex=4)
    #show the lines now
    for (j in 1:nbats)
      abline(h=j,col="grey")
  }
  else {
    lines(seq(1:length(select_pattern_5am)),dataframe_ranks[j,]-0.5,
          type="o",pch=4,col=j,lwd=4,cex=3)
  }
}

#8am
occupationDeadline = "8am"
for (j in 1:length(select_pattern_8am)) {
  dataframe_ranks[,j] = rank(-dataframe[1:nbats,select_pattern_8am[j]],
                             ties.method="first")
}
#plot the ranks of the bats now
for (j in 1:nbats) {
  if (j==1) { #set up the plot
    plot(seq(1:length(select_pattern_8am)),dataframe_ranks[j,]-0.5,
         xaxt="n",yaxs="i",xaxs="i",xlab="",yaxt="n",ylab="",
         ylim=c(0,nbats),type="o",pch=4,col=j,
         cex.axis=4,cex.lab=4,lwd=4,cex.main=4,
         main=paste("Occupation deadline = ",occupationDeadline,sep=""))
    axis(1,at=seq(1:length(select_pattern_8am)),labels=col_names[select_pattern_8am],las=2,cex.axis=4)
    axis(2,at=seq(1:nbats),labels=rownames_last4,las=2,padj=1.2,cex.axis=4)
    axis(4,at=seq(1:nbats),las=2,padj=1.2,cex.axis=4)
    mtext("Rank",side=4,line=8,cex=4)
    #show the lines now
    for (j in 1:nbats)
      abline(h=j,col="grey")
  }
  else {
    lines(seq(1:length(select_pattern_8am)),dataframe_ranks[j,]-0.5,
          type="o",pch=4,col=j,lwd=4,cex=3)
  }
}

dev.off()



#plot heatmaps of the final page ranks for all parameter combinations
setwd("/home/pmavrodiev/Documents/bats/result_files/output_files/2008/")
CairoPDF(file="final_pageranks_heatmaps.pdf",width=40,height=20)
par(mar=c(12,20,10,1),mgp=c(6,2,0),yaxs="i",xaxs="i",mfrow=c(2,3))
nf = layout(matrix(c(1,2,3,4,5,6),2,3,byrow=TRUE),TRUE)
occ_deadline = c("020000","030000","050000","080000")
lf_delay_vec = c("2","3","5","7","9")
for (jj in occ_deadline) {
  for (j in lf_delay_vec) {
    fileNamePatterns = paste("lead_follow_fpr_2008_\\d_",j,"_",jj,sep="")
    files = list.files(getwd(),pattern=fileNamePatterns)
    data = read.table(files[1],fill=TRUE,colClasses = "character")
    data.sorted = data[order(data[,2],decreasing=TRUE),]
    data.sorted.nonzero=data.sorted[data.sorted[,3]!=0,]
    z = matrix(0,length(files),(nrow(data.sorted.nonzero)-1))
    nsample_data = matrix(0,1,length(files))
    col_names_vec = NULL
    for (i in 1:length(files)) {  
      tokenized_str = strsplit(files[i],"\\D")
      year = as.numeric(tokenized_str[[1]][17])
      time_chunk = as.numeric(tokenized_str[[1]][18])
      lf_delay = as.numeric(tokenized_str[[1]][19])
      occupation_deadline = tokenized_str[[1]][20]
      col_name = paste(time_chunk,"/",lf_delay,"/",occupation_deadline,sep="")
      col_names_vec = c(col_names_vec,col_name)
      data = read.table(files[i],fill=TRUE,colClasses = "character")
      data.sorted = data[order(data[,2],decreasing=TRUE),]
      data.sorted.nonzero=data.sorted[data.sorted[,3]!=0,]
      col_diff = (nrow(data.sorted.nonzero)-1)-ncol(z)
      if (col_diff>0) {
        z = cbind(z,matrix(NA,nrow(z),col_diff))
      }      
      ranks_i = 500*as.numeric(data.sorted.nonzero[1:(nrow(data.sorted.nonzero)-1),2])  
      z[i,]=ranks_i
      nsample_data[1,i] = data.sorted.nonzero[nrow(data.sorted.nonzero),1]
    }
    ramp = colorRamp(c("darkblue","blue","green","yellow"),bias=2)
    #ramp = colorRamp(c("black","grey","white"),bias=0.7)    
    cols = rgb(ramp(seq(0,1,length=128)),max=255)
    image(z,yaxt="n",xaxt="n",col=cols,breaks=seq(0,140,length.out=129),oldstyle=TRUE,
          xlab="Box activity limit",cex.lab=4)    
    title(main=paste("LF delay = ",lf_delay," minutes",sep=""),cex.main=3,
          line=5)
    grid(nx=5,ny=0)
    box()
    axis(1,seq(0,1,length=nrow(z)),lab=c(2,3,5,7,9),las=1,cex.lab=3,cex.axis=3)
    axis(2,seq(0,1,length=(nrow(data.sorted.nonzero)-1)),
         lab=data.sorted[1:(nrow(data.sorted.nonzero)-1),1],las=2,
         cex.lab=2.2,cex.axis=2.2)
    par(mar=c(12,20,10,1),mgp=c(6,1,0))
    axis(3,seq(0,1,length=nrow(z)),lab=nsample_data,las=1,cex.axis=3)
    par(mar=c(12,20,10,1),mgp=c(6,2,0))
    if (j == "9") {    
      par(mar=c(15,5,10,88),ylbias=0,yaxs="i")
      s = seq(1,128)
      plot(rep(50,128),s,cex=8,col=cols[s],pch=15,xlim=c(40,60),cex.lab=4,cex.axis=4,
           xaxt="n",yaxt="n",xlab="",ylab="",bty="n")
      axis(4,at=seq(1,128,length=8),lab=seq(0,140,length=8),cex.axis=4,las=2)
      par(mar=c(12,20,10,1),mgp=c(8,4,0))
    }  
  }    
}
dev.off()


#plot heatmaps of the final page ranks for all parameter combinations
setwd("/home/pmavrodiev/Documents/bats/result_files/output_files/2007/")
CairoPDF(file="final_pageranks_heatmaps.pdf",width=40,height=20)
par(mar=c(12,20,10,1),mgp=c(6,2,0),yaxs="i",xaxs="i",mfrow=c(2,3))
nf = layout(matrix(c(1,2,3,4,5,6),2,3,byrow=TRUE),TRUE)
occ_deadline = c("020000","030000","050000","080000")
lf_delay_vec = c("2","3","5","7","9")
for (jj in occ_deadline) {
  for (j in lf_delay_vec) {
    fileNamePatterns = paste("lead_follow_fpr_2007_\\d_",j,"_",jj,sep="")
    files = list.files(getwd(),pattern=fileNamePatterns)
    data = read.table(files[1],fill=TRUE,colClasses = "character")
    data.sorted = data[order(data[,2],decreasing=TRUE),]
    data.sorted.nonzero=data.sorted[data.sorted[,3]!=0,]
    z = matrix(0,length(files),(nrow(data.sorted.nonzero)-1))
    nsample_data = matrix(0,1,length(files))
    col_names_vec = NULL
    for (i in 1:length(files)) {  
      tokenized_str = strsplit(files[i],"\\D")
      year = as.numeric(tokenized_str[[1]][17])
      time_chunk = as.numeric(tokenized_str[[1]][18])
      lf_delay = as.numeric(tokenized_str[[1]][19])
      occupation_deadline = tokenized_str[[1]][20]
      col_name = paste(time_chunk,"/",lf_delay,"/",occupation_deadline,sep="")
      col_names_vec = c(col_names_vec,col_name)
      data = read.table(files[i],fill=TRUE,colClasses = "character")
      data.sorted = data[order(data[,2],decreasing=TRUE),]
      data.sorted.nonzero=data.sorted[data.sorted[,3]!=0,]
      col_diff = (nrow(data.sorted.nonzero)-1)-ncol(z)
      if (col_diff>0) {
        z = cbind(z,matrix(NA,nrow(z),col_diff))
      }      
      ranks_i = 500*as.numeric(data.sorted.nonzero[1:(nrow(data.sorted.nonzero)-1),2])  
      z[i,]=ranks_i
      nsample_data[1,i] = data.sorted.nonzero[nrow(data.sorted.nonzero),1]
    }
    ramp = colorRamp(c("darkblue","blue","green","yellow"),bias=2)
    #ramp = colorRamp(c("black","grey","white"),bias=0.7)    
    cols = rgb(ramp(seq(0,1,length=128)),max=255)
    image(z,yaxt="n",xaxt="n",col=cols,breaks=seq(0,140,length.out=129),oldstyle=TRUE,
          xlab="Box activity limit",cex.lab=4)    
    title(main=paste("LF delay = ",lf_delay," minutes",sep=""),cex.main=3,
          line=5)
    grid(nx=5,ny=0)
    box()
    axis(1,seq(0,1,length=nrow(z)),lab=c(2,3,5,7,9),las=1,cex.lab=3,cex.axis=3)
    axis(2,seq(0,1,length=(nrow(data.sorted.nonzero)-1)),
         lab=data.sorted[1:(nrow(data.sorted.nonzero)-1),1],las=2,
         cex.lab=2.2,cex.axis=2.2)
    par(mar=c(12,20,10,1),mgp=c(6,1,0))
    axis(3,seq(0,1,length=nrow(z)),lab=nsample_data,las=1,cex.axis=3)
    par(mar=c(12,20,10,1),mgp=c(6,2,0))
    if (j == "9") {    
      par(mar=c(15,5,10,88),ylbias=0,yaxs="i")
      s = seq(1,128)
      plot(rep(50,128),s,cex=8,col=cols[s],pch=15,xlim=c(40,60),cex.lab=4,cex.axis=4,
           xaxt="n",yaxt="n",xlab="",ylab="",bty="n")
      axis(4,at=seq(1,128,length=8),lab=seq(0,140,length=8),cex.axis=4,las=2)
      par(mar=c(12,20,10,1),mgp=c(8,4,0))
    }  
  }    
}
dev.off()



#plot the distributions of the final page ranks for all parameter combinations
CairoPDF(file="final_pageranks.pdf",width=40,height=20)
par(mar=c(22,9,5,1),mgp=c(6,1,0),yaxs="i",xaxs="i")
setwd("/home/pmavrodiev/Documents/bats/result_files/output_files/2008/")
fileNamePatterns = "lead_follow_fpr_2008"
files = list.files(getwd(),pattern=fileNamePatterns)
data = read.table(files[1],fill=TRUE,colClasses = "character")
data.sorted = data[order(data[,2],decreasing=TRUE),]
data.sorted.nonzero=data.sorted[data.sorted[,3]!=0,]
ginis = NULL
col_names_vec = NULL
for (i in 1:length(files)) {  
  tokenized_str = strsplit(files[i],"\\D")
  year = as.numeric(tokenized_str[[1]][17])
  time_chunk = as.numeric(tokenized_str[[1]][18])
  lf_delay = as.numeric(tokenized_str[[1]][19])
  occupation_deadline = tokenized_str[[1]][20]
  col_name = paste(time_chunk,"/",lf_delay,"/",occupation_deadline,sep="")
  col_names_vec = c(col_names_vec,col_name)
  data = read.table(files[i],fill=TRUE,colClasses = "character")
  data.sorted = data[order(data[,2],decreasing=TRUE),] 
  data.sorted.nonzero=data.sorted[data.sorted[,3]!=0,]
  
  barplot(500*as.numeric(data.sorted.nonzero[1:(nrow(data.sorted.nonzero)-1),2]),
          names.arg=data.sorted.nonzero[1:(nrow(data.sorted.nonzero)-1),1],
          beside=TRUE,las=2,cex.axis=4,cex.names=4,ylab="Page Rank",
          cex.lab=4,cex.main=4,
          main=paste("Chunks boundary = ",time_chunk," minutes / LF dist. = ",
                     lf_delay," minutes / Occupation deadline = ",
                     occupation_deadline,sep=""))
  ginis=c(ginis,gini(as.numeric(data.sorted.nonzero[1:(nrow(data.sorted.nonzero)-1),2])))
}
plot(seq(1:length(files)),
     ginis,xaxt="n",yaxs="i",xaxs="i",xlab="",ylab="Gini coefficient",
     type="o",pch=4, cex.lab=4,cex.main=4,cex.axis=4,lwd=3,cex=3)
axis(1,at=seq(1:length(files)),
     labels=col_names_vec,las=2,cex.axis=3)

dev.off()


#plot the distributions of the final page ranks for all parameter combinations
setwd("/home/pmavrodiev/Documents/bats/result_files/output_files/2007/")
CairoPDF(file="final_pageranks.pdf",width=40,height=20)
par(mar=c(22,9,5,1),mgp=c(6,1,0),yaxs="i",xaxs="i")
fileNamePatterns = "lead_follow_fpr_2007"
files = list.files(getwd(),pattern=fileNamePatterns)
data = read.table(files[1],fill=TRUE,colClasses = "character")
data.sorted = data[order(data[,2],decreasing=TRUE),]
data.sorted.nonzero=data.sorted[data.sorted[,3]!=0,]
ginis = NULL
col_names_vec = NULL
for (i in 1:length(files)) {  
  tokenized_str = strsplit(files[i],"\\D")
  year = as.numeric(tokenized_str[[1]][17])
  time_chunk = as.numeric(tokenized_str[[1]][18])
  lf_delay = as.numeric(tokenized_str[[1]][19])
  occupation_deadline = tokenized_str[[1]][20]
  col_name = paste(time_chunk,"/",lf_delay,"/",occupation_deadline,sep="")
  col_names_vec = c(col_names_vec,col_name)
  data = read.table(files[i],fill=TRUE,colClasses = "character")
  data.sorted = data[order(data[,2],decreasing=TRUE),] 
  data.sorted.nonzero=data.sorted[data.sorted[,3]!=0,]
  
  barplot(500*as.numeric(data.sorted.nonzero[1:(nrow(data.sorted.nonzero)-1),2]),
          names.arg=data.sorted.nonzero[1:(nrow(data.sorted.nonzero)-1),1],
          beside=TRUE,las=2,cex.axis=4,cex.names=4,ylab="Page Rank",
          cex.lab=4,cex.main=4,
          main=paste("Chunks boundary = ",time_chunk," minutes / LF dist. = ",
                     lf_delay," minutes / Occupation deadline = ",
                     occupation_deadline,sep=""))
  ginis=c(ginis,gini(as.numeric(data.sorted.nonzero[1:(nrow(data.sorted.nonzero)-1),2])))
}
plot(seq(1:length(files)),
     ginis,xaxt="n",yaxs="i",xaxs="i",xlab="",ylab="Gini coefficient",
     type="o",pch=4, cex.lab=4,cex.main=4,cex.axis=4,lwd=3,cex=3)
axis(1,at=seq(1:length(files)),
     labels=col_names_vec,las=2,cex.axis=3)

dev.off()




