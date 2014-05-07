library("data.table")
library("Matching")
library(Cairo)



wilcox.boot = function (Tr, Co, nboots = 1000, alternative = c("two.sided", 
                                                                "less", "greater"), print.level = 0)  {
  #options(warn=-3)
  alternative = match.arg(alternative)
  tol <- sqrt(.Machine$double.eps)
  Tr <- Tr[!is.na(Tr)]
  Co <- Co[!is.na(Co)]
  w <- c(Tr, Co)
  obs <- length(w)
  n.x <- length(Tr)
  n.y <- length(Co)
  cutp <- n.x
  wcox.boot.pval <- NULL
  bbcount <- 0
  if (nboots < 10) {
    nboots <- 10
    warning("At least 10 'nboots' must be run; seting 'nboots' to 10")
  }
  if (nboots < 500) 
    warning("For publication quality p-values it is recommended that 'nboots'\n be set equal to at least 500 (preferably 1000)")  
  
  wcox <- wilcox.test(Tr, Co, alternative = alternative,exact=FALSE)

  if (alternative == "two.sided") {
    if (print.level > 0) 
      cat("Wilcoxon rank sum test with continuity correction: two.sided test\n")
    for (bb in 1:nboots) {
      if (print.level > 1) 
        cat("s:", bb, "\n")
      sindx <- sample(1:obs, obs, replace = TRUE)
      X1tmp <- w[sindx[1:cutp]]      
      X2tmp <- w[sindx[(cutp + 1):obs]]      
      s.wilcox <- wilcox.test(X1tmp, X2tmp,exact=FALSE)
      if (s.wilcox$statistic <= (wcox$statistic - tol)) 
        bbcount <- bbcount + 1
    }
  }
  else {
    if (print.level > 0) 
      cat("Wilcoxon rank sum test with continuity correction:", alternative, "test\n")
    for (bb in 1:nboots) {
      if (print.level > 1) 
        cat("s:", bb, "\n")
      sindx <- sample(1:obs, obs, replace = TRUE)
      X1tmp <- w[sindx[1:cutp]]
      X2tmp <- w[sindx[(cutp + 1):obs]]
      s.wilcox <- wilcox.test(X1tmp, X2tmp, alternative = alternative,exact=FALSE)
      if (s.wilcox$statistic <= (wcox$statistic - tol)) 
        bbcount <- bbcount + 1
    }
  }
  wilcox.boot.pval = bbcount/nboots
  ret = list(wilcox.boot.pvalue = wilcox.boot.pval, wcox.original = wcox, nboots = nboots)
  class(ret) <- "wilcox.boot"
  options(warn=0)
  return(ret)
}


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
        ks.pvalue=wilcox.boot(as.numeric(x1$lf_time_diff),
                        as.numeric(x2$lf_time_diff))$wilcox.boot.pvalue
        cat(round(ks.pvalue,3),"\t",sep="")
      }      
    }
    cat("\n",sep="")
  }
}


#compare occupation time-pairs for fixed turnaround_time and occ_deadline
for (tt in 5:5){#all.turnaround.times) {
  cat("turnaround_time = ",tt,"\n",sep="")
  for (i in 3:3) {
    for (lfd in 1:4) {
      
      x1=subset(dtable,turnaround_time==tt&
                  lf_delay==all.lf.delays[lfd]&
                  occupation_deadline==all.occupation.deadlines[i])
      for (lfd2 in (lfd+1):5) {
        x2=subset(dtable,turnaround_time==tt&
                    lf_delay==all.lf.delays[lfd2]&
                    occupation_deadline==all.occupation.deadlines[i])
        ks.pvalue=wilcox.test(as.numeric(x1$lf_time_diff),
                              as.numeric(x2$lf_time_diff))$p.value
        cat(round(ks.pvalue,3),"\t",sep="")
      }
    }
    cat("\n",sep="")
  }
}
  
#get the sample sizes
for (tt in all.turnaround.times) {
  cat("turnaround_time = ",tt,"\n",sep="")
  for (lfd in 5:5){#all.lf.delays) {
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

x1=subset(dtable,turnaround_time==2&
            lf_delay==5&occupation_deadline=="050000")
file.name="/home/pmavrodiev/Documents/bats/result_files/output_files_new_param_sweep/2008_GB2/lf-time-differences-lf-5-tt-2-od-5.pdf"
plot.histogram(as.numeric(x1$lf_time_diff),file.name,b=20,y.max=0.3)

x2=subset(dtable,turnaround_time==2&
            lf_delay==5&occupation_deadline=="080000")
file.name="/home/pmavrodiev/Documents/bats/result_files/output_files_new_param_sweep/2008_GB2/lf-time-differences-lf-5-tt-2-od-8.pdf"
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
