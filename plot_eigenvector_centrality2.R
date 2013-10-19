library(Cairo)
library(ggplot2)
library(grid)


rm(list=ls())

args = commandArgs(trailingOnly = TRUE)
if (length(args) != 1) {
  stop("Insufficient arguments. \n\n <dirs>: path to result files\n\n")
}

path = args[1]

list.dirs = function(path=".", pattern=NULL, all.dirs=FALSE,
                     full.names=TRUE, ignore.case=FALSE) {
  
  
  all = list.files(path, pattern, all.dirs,
                   full.names, recursive=FALSE, ignore.case)
  all[file.info(all)$isdir]
}

dirs=list.dirs(path,pattern="_")
for (d in dirs) {
  print(paste("Entering ",d,sep=""))
  setwd(d)
  #tokenize dir
  s = strsplit(d,"_")  
  year=strsplit(s[[1]][5],"/")[[1]][2] # 2008
  colony=paste("_",s[[1]][6],sep="") # "_GB2"   
#   original.ev=paste("eigenvector_original_",year,".dat",sep="")
#   original.in=paste("indegree_original_",year,".dat",sep="")
  original.sd=paste("second-indegree_original_",year,".dat",
                    sep="")  
#   eigenvector.original[,1] = substr(eigenvector.original[,1],7,10)
#   eigenvector.original = eigenvector.original[order(eigenvector.original[,1]),]
#   
#   indegree.original = read.table(original.in)
#   indegree.original[,1] = substr(indegree.original[,1],7,10)
#   indegree.original = indegree.original[order(indegree.original[,1]),]
  
  sd.original = read.table(original.sd)
  sd.original[,1] = substr(sd.original[,1],7,10)
  sd.original = sd.original[order(sd.original[,1]),]
  
  options(warn=3)
  for (model in   1:6) { #c(1,2,3,5)) {
#     shuffled.ev=paste("eigenvector_shuffled_",year,"_model-",model,".dat.gz",sep="")
#     shuffled.in=paste("indegree_shuffled_",year,"_model-",model,".dat.gz",sep="")   
    shuffled.sd=paste("second-indegree_shuffled_",year,
                      "_model-",model,".dat.gz",sep="")
    
    sd.shuffled = read.table(shuffled.sd)
    sd.shuffled.matrix = 
      matrix(sd.shuffled[,2],
             nrow = nrow(sd.original),
             ncol = length(sd.shuffled[,2]) / nrow(sd.original))
{
    rownames(sd.shuffled.matrix) = 
      substr(sd.shuffled[1:nrow(sd.original),1],7,10)
    sd.shuffled.matrix = 
      sd.shuffled.matrix[order(rownames(sd.shuffled.matrix)),]    
    sd.shuffled.matrix = 
      sd.shuffled.matrix[,colSums(sd.shuffled.matrix)!=0]    
    
}   
    
#     eigenvector.shuffled = read.table(shuffled.ev)
#     eigenvector.shuffled.matrix = 
#       matrix(eigenvector.shuffled[,2],
#              nrow = nrow(eigenvector.original),
#              ncol = length(eigenvector.shuffled[,2]) / nrow(eigenvector.original))
# {
#     rownames(eigenvector.shuffled.matrix) = substr(eigenvector.shuffled[1:nrow(eigenvector.original),1],7,10)
#     eigenvector.shuffled.matrix = eigenvector.shuffled.matrix[order(rownames(eigenvector.shuffled.matrix)),]    
#     eigenvector.shuffled.matrix = 
#       eigenvector.shuffled.matrix[,colSums(eigenvector.shuffled.matrix)!=0]
# }   
#     indegree.shuffled = read.table(shuffled.in)
#     indegree.shuffled.matrix = 
#       matrix(indegree.shuffled[,2],
#              nrow = nrow(indegree.original),
#              ncol = length(indegree.shuffled[,2]) / nrow(indegree.original))
#     
# {    
#     rownames(indegree.shuffled.matrix) = substr(indegree.shuffled[1:nrow(indegree.original),1],7,10)
#     indegree.shuffled.matrix = indegree.shuffled.matrix[order(rownames(indegree.shuffled.matrix)),]    
#     indegree.shuffled.matrix = 
#       indegree.shuffled.matrix[,colSums(indegree.shuffled.matrix)!=0]
# }   
    X.lab=sd.original[,1]          
    X.ints = seq(1,length(X.lab)) 

    yminus.sd = apply(sd.shuffled.matrix,1,quantile,
                      c(0.025,0.975),TRUE)[1,]    
    yplus.sd = apply(sd.shuffled.matrix,1,quantile,
                     c(0.025,0.975),TRUE)[2,]      
    ymedian.sd = apply(sd.shuffled.matrix,1,median,TRUE)
#     
#     yminus.ev = apply(eigenvector.shuffled.matrix,1,quantile,c(0.025,0.975),TRUE)[1,]    
#     yplus.ev = apply(eigenvector.shuffled.matrix,1,quantile,c(0.025,0.975),TRUE)[2,]      
#     ymedian.ev = apply(eigenvector.shuffled.matrix,1,median,TRUE)
#     yminus.in = apply(indegree.shuffled.matrix,1,quantile,c(0.025,0.975),TRUE)[1,]    
#     yplus.in = apply(indegree.shuffled.matrix,1,quantile,c(0.025,0.975),TRUE)[2,]      
#     ymedian.in = apply(indegree.shuffled.matrix,1,median,TRUE)    
#     
    df=data.frame(x.ints=X.ints,                  
                  #y.empirical.ev=eigenvector.original[,2],
                  #y.empirical.in=indegree.original[,2],
                  y.empirical.sd=sd.original[,2]                   
                  )
    
    df_error = 
      data.frame(x=c(X.ints,rev(X.ints)),
#                  y.ev=c(as.numeric(yminus.ev),
#                         rev(as.numeric(yplus.ev))),
#                  y.in=c(as.numeric(yminus.in),
#                         rev(as.numeric(yplus.in))),
                 y.sd=c(as.numeric(yminus.sd),
                        rev(as.numeric(yplus.sd))),                 
                 y.median.sd=c(as.numeric(ymedian.sd),
                               rev(as.numeric(ymedian.sd)))                   
#                  y.median.ev=c(as.numeric(ymedian.ev),
#                                rev(as.numeric(ymedian.ev))),                       
#                  y.median.in=c(as.numeric(ymedian.in),
#                                rev(as.numeric(ymedian.in)))
                )
    
    Ymax=max(c(sd.original[,2],as.numeric(yplus.sd)))
    CairoPDF(file=paste(year,colony,"-seconddegree-model-",
                        model,".pdf",sep=""),width=16,height=10)    
    g=ggplot(data=df,aes(x=x.ints,y=y.empirical.sd/Ymax))+
      geom_point(size=7,shape=4)+xlab("")+
      ylab("second-degree centrality")+
      scale_x_discrete(labels=X.lab)+
      scale_y_continuous(limits=c(0,1))+
      geom_polygon(data=df_error,mapping=aes(x=x,y=y.sd/Ymax),
                   fill="blue",alpha=0.4)+
      theme(axis.text.x = element_text(angle=90,
                                       size = rel(3),vjust=0.5),
            axis.text.y = element_text(size = rel(3)),
            axis.title.y = element_text(size=rel(2.5),vjust=0.1),
            plot.margin = unit(c(0.2,0.2,0.1,1),"cm"))+
      geom_text(label=paste("MODEL ",model,sep=""),
                size=rel(12),x=0.92*max(X.ints),y=0.97)    
    theme_set(theme_bw())
    print(g)  
    dev.off() 
#     
#     CairoPDF(file=paste(year,colony,"-model-",model,".pdf",sep=""),width=16,height=10)
#     Y.max=1
#     g=ggplot(data=df,aes(x=x.ints,y=y.empirical.ev/Y.max))+
#         geom_point(size=7,shape=4)+xlab("")+
#         ylab("eigenvector centrality")+
#         scale_x_discrete(labels=X.lab)+
#         scale_y_continuous(limits=c(0,1))+
#         geom_polygon(data=df_error,mapping=aes(x=x,y=y.ev/Y.max),fill="blue",alpha=0.4)+
#       theme(axis.text.x = element_text(angle=90,size = rel(3),vjust=0.5),
#             axis.text.y = element_text(size = rel(3)),
#             axis.title.y = element_text(size=rel(2.5),vjust=0.1),
#             plot.margin = unit(c(0.2,0.2,0.1,1),"cm"))+
#             geom_text(label=paste("MODEL ",model,sep=""),
#               size=rel(12),x=0.92*max(X.ints),y=0.97)    
#     theme_set(theme_bw())
#     print(g)  
#     dev.off() 
#     
#     CairoPDF(file=paste(year,colony,"-indegree-model-",model,".pdf",sep=""),width=16,height=10)    
#     Y.max=max(c(yplus.in,indegree.original[,2]))
#     g=ggplot(data=df,aes(x=x.ints,y=y.empirical.in/Y.max))+
#       geom_point(size=7,shape=4)+xlab("")+
#       ylab("indegree centrality")+
#       scale_x_discrete(labels=X.lab)+
#       scale_y_continuous(limits=c(0,1))+
#       geom_polygon(data=df_error,mapping=aes(x=x,y=y.in/Y.max),fill="blue",alpha=0.4)+
#       theme(axis.text.x = element_text(angle=90,size = rel(3),vjust=0.5),
#             axis.text.y = element_text(size = rel(3)),
#             axis.title.y = element_text(size=rel(2.5),vjust=0.1),
#             plot.margin = unit(c(0.2,0.2,0.1,1),"cm"))+ 
#       geom_text(label=paste("MODEL ",model,sep=""),
#                 size=rel(12),x=0.92*max(X.ints),y=0.97)
#     
#     theme_set(theme_bw())
#     print(g)  
#     dev.off()      
    
#     save("ymedian.ev","yplus.ev","ymedian.ev","ymedian.in","yplus.in","ymedian.in",
#          file=paste("data-model-",model,".RData",sep="")) 
    save("ymedian.sd","yplus.sd","ymedian.sd",
         file=paste("data-model-",model,".RData",sep="")) 
  }
  
#   #calculate the ranks
  eigenvector.original.sorted = eigenvector.original[order(eigenvector.original[,2],decreasing=TRUE),]
  indegree.original.sorted = indegree.original[order(indegree.original[,2],decreasing=TRUE),]
  X.lab=eigenvector.original.sorted[,1]  
  ymax=rep(NA,length(X.lab)); ymin=rep(NA,length(X.lab));pch_min=rep(NA,length(X.lab));
  pch_max=rep(NA,length(X.lab));
  #eigenvector to indegree rank change
  #eigenvector is an empty square, indegree is full circle
  for (i in 1:length(X.lab)) {
    ev.rank = which(eigenvector.original.sorted[,1] == X.lab[i])
    indegree.rank = which(indegree.original.sorted[,1] == X.lab[i])
    ymin[i] = min(ev.rank,indegree.rank)
    ymax[i] = max(ev.rank,indegree.rank)
    if (ev.rank < indegree.rank) {
      pch_min[i]=0 ; pch_max[i]=16
    } else {
      pch_min[i]=16; pch_max[i]=0
    }     
  }
  
  df.rank.change = data.frame(x=X.ints,ymin=ymin,ymax=ymax,pch_min=pch_min,
                              pch_max=pch_max)
  
  CairoPDF(file=paste(year,colony,"-rankchange.pdf",sep=""),width=16,height=10) 
  g=ggplot(data=df.rank.change,aes(x=x))+
    geom_linerange(aes(x=x,ymin=ymin,ymax=ymax))+
    geom_point(aes(x=x,y=ymin),shape=pch_min,size=7)+
    geom_point(aes(x=x,y=ymax),shape=pch_max,size=7)+xlab("")+
    ylab("rank change")+scale_x_discrete(labels=X.lab)+
    theme(axis.text.x = element_text(angle=90,size = rel(3),vjust=0.5),
          axis.text.y = element_text(size = rel(3)),
          axis.title.y = element_text(size=rel(2.5),vjust=0.1),
          plot.margin = unit(c(0.2,0.2,0.1,1),"cm"))  
  theme_set(theme_bw())
  print(g)
  dev.off()
 
}
  

#calcualte level of heterogeneity

calc.heterogeneity = function(indegree.vector) {
  return (sum(indegree.vector^2) / sum(indegree.vector))
}

for (d in dirs) {
  print(paste("Entering ",d,sep=""))
  setwd(d)
  #tokenize dir
  s = strsplit(d,"_")  
  year=strsplit(s[[1]][5],"/")[[1]][2] # 2008
  colony=paste("_",s[[1]][6],sep="") # "_GB2"   
  original.in=paste("indegree_original_",year,".dat",sep="")
  
  indegree.original = read.table(original.in)
  indegree.original[,1] = substr(indegree.original[,1],7,10)
  indegree.original = indegree.original[order(indegree.original[,1]),]  
  empirical_heterogeneity = calc.heterogeneity(as.numeric(indegree.original[,2]))
 
  cat(empirical_heterogeneity); cat("\n")
  sink(file=paste(year,"-heterogeneity.dat",sep=""))
  cat(paste("heterogeneity \t ",empirical_heterogeneity,sep=""))
  cat(paste("molloy reed \t ",
  (sum(indegree.original[,2]^2)-2*sum(indegree.original[,2]))/nrow(indegree.original),sep=""))
  sink()
  options(warn=3)
#   CairoPDF(file=paste(year,colony,"-heterogeneity.pdf",sep=""),width=10,height=10)
#   for (model in  c(1,2,3,5)) {    
#     shuffled.in=paste("indegree_shuffled_",year,"_model-",model,".dat.gz",sep="")      
#     indegree.shuffled = read.table(shuffled.in)
#     indegree.shuffled.matrix = 
#       matrix(indegree.shuffled[,2],
#              nrow = nrow(indegree.original),
#              ncol = length(indegree.shuffled[,2]) / nrow(indegree.original))
#     
# 
#         rownames(indegree.shuffled.matrix) = substr(indegree.shuffled[1:nrow(indegree.original),1],7,10)
#         indegree.shuffled.matrix = indegree.shuffled.matrix[order(rownames(indegree.shuffled.matrix)),]    
#         indegree.shuffled.matrix = 
#           indegree.shuffled.matrix[,colSums(indegree.shuffled.matrix)!=0]
#     
#     heterogeneity.vector=apply(indegree.shuffled.matrix,2,calc.heterogeneity)
#     
#     hist(heterogeneity.vector,freq=FALSE,breaks=50,
#          main=paste(year," - Model ",model,sep=""),xlab="simulation")        
#   }
#   dev.off()
}