library("data.table")
library("Matching")
library(Cairo)
setwd("/home/pmavrodiev/Documents/bats/result_files/output_files_new_param_sweep/2008_GB2")

#setwd("/home/pmavrodiev/Documents/bats/result_files/output_files_new_param_sweep/2007_BS")

data=read.table("parameter-sweep-GB2-2008.dat",colClasses="character")

#data=read.table("parameter-sweep-GB2-2007.dat",colClasses="character")

dtable=data.table(data)

setnames(dtable,old=c("V1","V2","V3","V4"),
         new=c("lf_delay","turnaround_time",
               "occupation_deadline","lf_time_diff"))

setkeyv(dtable,cols=c("lf_delay","turnaround_time",
                      "occupation_deadline"))

all.occupation.deadlines=unique(dtable$occupation_deadline)
all.lf.delays = sort(as.numeric(unique(dtable$lf_delay)))
all.turnaround.times=sort(as.numeric(unique(dtable$turnaround_time)))

#compare occupation time-pairs for fixed lf_delay
for (tt in all.turnaround.times) {
  cat("turnaround_time = ",tt,"\n",sep="")
  for (lfd in 3:3){#all.lf.delays) {
    cat("\t lf_delay = ",lfd,"\t",sep="")
    for (i in 1:(length(all.occupation.deadlines)-1)) {
      x1=subset(dtable,turnaround_time==tt&
                lf_delay==lfd&
                occupation_deadline==all.occupation.deadlines[i])
      for (j in (i+1):length(all.occupation.deadlines)) {
        x2=subset(dtable,turnaround_time==tt&
                         lf_delay==lfd&
                  occupation_deadline==all.occupation.deadlines[j])
        ks.pvalue=wilcox.test(as.numeric(x1$lf_time_diff),
                        as.numeric(x2$lf_time_diff),alternative="less")$p.value
        cat(round(ks.pvalue,3),"\t",sep="")
      }      
    }
    cat("\n",sep="")
  }
}
#get the sample sizes
for (tt in all.turnaround.times) {
  cat("turnaround_time = ",tt,"\n",sep="")
  for (lfd in 3:3){#all.lf.delays) {
    for (i in 1:(length(all.occupation.deadlines))) {
      x1=subset(dtable,turnaround_time==tt&
                  lf_delay==lfd&
                  occupation_deadline==all.occupation.deadlines[i])
      cat("N = ",nrow(x1),"\t")
    }
    cat("\n",sep="")
  }
}



plot.histogram = function(data,file.name,b=20,y.max) {  
  histogram = hist(as.numeric(data),breaks=b,plot=FALSE)
  mids = histogram$mids
  counts = histogram$counts
  counts = counts / sum(counts) #normalize
  xmin = floor(min(mids)); xmax = ceiling(max(mids))
  bin.width=mids[2]-mids[1]
  ymax = y.max
  CairoPDF(file=file.name,width=12,height=10)
  par(mar=c(8,11,1,1),mgp=c(1,2.5,0))
  plot(NA,xlim=c(xmin,xmax),ylim=c(0,ymax),xaxt="n",yaxt="n",xlab="",ylab="")
  rect(xleft=mids-bin.width/2,ybottom=0,xright=mids+bin.width/2,ytop=counts,
       col="lightgoldenrod",border="navy",lwd=3)
  axis(1,at=seq(xmin,xmax),label=seq(xmin,xmax),cex.axis=3,cex.lab=3)
  axis(2,at=pretty(seq(0,ymax,by=0.01)),label=pretty(seq(0,ymax,by=0.01)),cex.axis=3,
       cex.lab=3,las=2,hadj=0.8)
  mtext("L/F time difference [minutes]",side=1,cex=3,line=6.3)
  mtext("Probability of occurrence",side=2,cex=3,line=8.5)
  legend("topright",paste("N = ",length(data),sep=""),cex=2,bty="n")
  dev.off()  
}

plot.histogram()

x1=subset(dtable,turnaround_time==3&
            lf_delay==3&occupation_deadline=="020000")
file.name="/home/pmavrodiev/Documents/bats/result_files/output_files_new_param_sweep/2008_GB2/lf-time-differences-lf-3-tt-3-od-2.pdf"
plot.histogram(as.numeric(x1$lf_time_diff),file.name,b=20,y.max=0.3)

x2=subset(dtable,turnaround_time==3&
            lf_delay==3&occupation_deadline=="080000")
file.name="/home/pmavrodiev/Documents/bats/result_files/output_files_new_param_sweep/2008_GB2/lf-time-differences-lf-3-tt-3-od-8.pdf"
plot.histogram(as.numeric(x2$lf_time_diff),file.name,b=20,y.max=0.3)


x3=subset(dtable,turnaround_time==3&
            lf_delay==5&occupation_deadline=="080000")

hist(as.numeric(x1$lf_time_diff))
hist(as.numeric(x2$lf_time_diff))
hist(as.numeric(x3$lf_time_diff))

mean(as.numeric(x1$lf_time_diff))
mean(as.numeric(x2$lf_time_diff))
mean(as.numeric(x3$lf_time_diff))

median(as.numeric(x1$lf_time_diff))
median(as.numeric(x2$lf_time_diff))
median(as.numeric(x3$lf_time_diff))
