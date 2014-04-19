library(Cairo)
library(ggplot2)
library(grid)


rm(list=ls())

args = commandArgs(trailingOnly = TRUE)
if (length(args) != 1) {
  stop("Insufficient arguments. \n\n <dirs>: path to result files\n\n")
}

#path = args[1]
path="/home/pmavrodiev/Documents/bats/result_files/output_files_new_4"

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
  original.ev=paste("eigenvector_original_",year,".dat",sep="")
  original.in=paste("indegree_original_",year,".dat",sep="")
  original.sd=paste("second-indegree_original_",year,".dat",
                    sep="")  
  eigenvector.original = read.table(original.ev)
  eigenvector.original[,1] = substr(eigenvector.original[,1],
                                    7,10)
  eigenvector.original = eigenvector.original[
                           order(eigenvector.original[,1]),]
  
  indegree.original = read.table(original.in)
  indegree.original[,1] = substr(indegree.original[,1],7,10)
  indegree.original = indegree.original[order(indegree.original[,1]),]
  
  sd.original = read.table(original.sd)
  sd.original[,1] = substr(sd.original[,1],7,10)
  sd.original = sd.original[order(sd.original[,1]),]
  
  options(warn=3)
  density.list=list()
  sink("density-estimates.dat")  
  for (model in 1:6) {
    skip.indegree=FALSE
    if (model == 4 | model == 6)
      skip.indegree=TRUE
    shuffled.ev=paste("eigenvector_shuffled_",year,"_model-",model,".dat.gz",sep="")
    shuffled.in=paste("indegree_shuffled_",year,"_model-",model,".dat.gz",sep="")   
    shuffled.sd=paste("second-indegree_shuffled_",year,
                      "_model-",model,".dat.gz",sep="")    
    density.estimates.this.model = matrix(NA,nrow=nrow(sd.original),ncol=3)
    rownames(density.estimates.this.model) = sd.original[,1]
    colnames(density.estimates.this.model) = c("eigenvector","second.indegree",
                                               "indegree")
    
{
    nlines=as.numeric(system(paste("zcat ",shuffled.sd," | wc -l",sep=""),
                             intern=TRUE))
    sd.shuffled = read.table(shuffled.sd,nrows=nlines,colClasses=NA)
    sd.shuffled.matrix = 
      matrix(sd.shuffled[,2],
             nrow = nrow(sd.original),
             ncol = length(sd.shuffled[,2]) / nrow(sd.original))    
    rownames(sd.shuffled.matrix) = 
      substr(sd.shuffled[1:nrow(sd.original),1],7,10)
    sd.shuffled.matrix = 
      sd.shuffled.matrix[order(rownames(sd.shuffled.matrix)),]    
    sd.shuffled.matrix = 
      sd.shuffled.matrix[,colSums(sd.shuffled.matrix)!=0]
    rm(sd.shuffled); gc();
    Ymax=max(c(sd.original[,2],max(sd.shuffled.matrix)))
    sd.original.normed = sd.original[,2] / Ymax
    sd.shuffled.matrix.normed=apply(sd.shuffled.matrix,2,"/",Ymax)    
}   

{
    nlines=as.numeric(system(paste("zcat ",shuffled.ev," | wc -l",sep=""),
                          intern=TRUE))
    eigenvector.shuffled = read.table(shuffled.ev,nrows=nlines,colClasses=NA)
    eigenvector.shuffled.matrix = 
      matrix(eigenvector.shuffled[,2],
             nrow = nrow(eigenvector.original),
             ncol = length(eigenvector.shuffled[,2]) / nrow(eigenvector.original))    
    rownames(eigenvector.shuffled.matrix) = 
      substr(eigenvector.shuffled[1:nrow(eigenvector.original),1],7,10)
    eigenvector.shuffled.matrix = 
      eigenvector.shuffled.matrix[order(rownames(eigenvector.shuffled.matrix)),]    
    eigenvector.shuffled.matrix = 
      eigenvector.shuffled.matrix[,colSums(eigenvector.shuffled.matrix)!=0]
    rm(eigenvector.shuffled); gc();
}   
    
    if (!skip.indegree)
{
    nlines=as.numeric(system(paste("zcat ",shuffled.in," | wc -l",sep=""),
                               intern=TRUE))    
    indegree.shuffled = read.table(shuffled.in,nrows=nlines,colClasses=NA)
    indegree.shuffled.matrix = 
      matrix(indegree.shuffled[,2],
             nrow = nrow(indegree.original),
             ncol = length(indegree.shuffled[,2]) / nrow(indegree.original))
        
    rownames(indegree.shuffled.matrix) = 
      substr(indegree.shuffled[1:nrow(indegree.original),1],7,10)
    indegree.shuffled.matrix = 
      indegree.shuffled.matrix[order(rownames(indegree.shuffled.matrix)),]    
    indegree.shuffled.matrix = 
      indegree.shuffled.matrix[,colSums(indegree.shuffled.matrix)!=0]
    rm(indegree.shuffled); gc();
    Ymax=max(c(indegree.original[,2],max(indegree.shuffled.matrix)))
    indegree.original.normed = indegree.original[,2] / Ymax
    indegree.shuffled.matrix.normed=apply(indegree.shuffled.matrix,2,"/",Ymax)    
}   
    
    counter=1
    for (r in rownames(density.estimates.this.model)) {
      density.estimates.this.model[r,"eigenvector"]=
        dnorm(eigenvector.original[counter,2],
          mean(eigenvector.shuffled.matrix[r,]),sd(eigenvector.shuffled.matrix[r,]))
      density.estimates.this.model[r,"second.indegree"]=
        dnorm(sd.original.normed[counter],
         mean(sd.shuffled.matrix.normed[r,]),sd(sd.shuffled.matrix.normed[r,]))
      if (!skip.indegree) {
      density.estimates.this.model[r,"indegree"]=
        dnorm(indegree.original.normed[counter],
  mean(indegree.shuffled.matrix.normed[r,]),sd(indegree.shuffled.matrix.normed[r,]))      
      }
      counter=counter+1
    }
    density.estimates.this.model[is.infinite(density.estimates.this.model[,1]),2]=0
    density.estimates.this.model[is.infinite(density.estimates.this.model[,2]),2]=0
    density.estimates.this.model[is.infinite(density.estimates.this.model[,3]),2]=0
    save(density.estimates.this.model,
         file=paste("density-estimates-model-",model,"-detailed.RData",sep=""))    
    cat("Model\tEigenvector\tSecond-indegree\tIndegree\n")
    cat(model,sum(density.estimates.this.model[,"eigenvector"]),
        sum(density.estimates.this.model[,"second.indegree"]),
        sum(density.estimates.this.model[,"indegree"]),"\n",sep="\t")    
#     X.lab=sd.original[,1]          
#     X.ints = seq(1,length(X.lab)) 
# 
#     yminus.sd = apply(sd.shuffled.matrix,1,quantile,
#                       c(0.025,0.975),TRUE)[1,]    
#     yplus.sd = apply(sd.shuffled.matrix,1,quantile,
#                      c(0.025,0.975),TRUE)[2,]      
#     ymedian.sd = apply(sd.shuffled.matrix,1,median,TRUE)
# #     
#     yminus.ev = apply(eigenvector.shuffled.matrix,1,quantile,c(0.025,0.975),TRUE)[1,]    
#     yplus.ev = apply(eigenvector.shuffled.matrix,1,quantile,c(0.025,0.975),TRUE)[2,]      
#     ymedian.ev = apply(eigenvector.shuffled.matrix,1,median,TRUE)
#     yminus.in = apply(indegree.shuffled.matrix,1,quantile,c(0.025,0.975),TRUE)[1,]    
#     yplus.in = apply(indegree.shuffled.matrix,1,quantile,c(0.025,0.975),TRUE)[2,]      
#     ymedian.in = apply(indegree.shuffled.matrix,1,median,TRUE)    
#     
#     df=data.frame(x.ints=X.ints,                  
#                   y.empirical.ev=eigenvector.original[,2],
#                   #y.empirical.in=indegree.original[,2],
#                   y.empirical.sd=sd.original[,2]                   
#                   )
#     
#     df_error = 
#       data.frame(x=c(X.ints,rev(X.ints)),
#                  y.ev=c(as.numeric(yminus.ev),
#                         rev(as.numeric(yplus.ev))),
# #                  y.in=c(as.numeric(yminus.in),
# #                         rev(as.numeric(yplus.in))),
#                  y.sd=c(as.numeric(yminus.sd),
#                         rev(as.numeric(yplus.sd))),                 
#                  y.median.sd=c(as.numeric(ymedian.sd),
#                                rev(as.numeric(ymedian.sd))),                   
#                  y.median.ev=c(as.numeric(ymedian.ev),
#                                rev(as.numeric(ymedian.ev)))                      
#                  y.median.in=c(as.numeric(ymedian.in),
#                                rev(as.numeric(ymedian.in)))
#                 )
    
#     Ymax=max(c(sd.original[,2],max(sd.shuffled.matrix)))
#     CairoPDF(file=paste(year,colony,"-seconddegree-model-",
#                         model,".pdf",sep=""),width=16,height=10)    
#     g=ggplot(data=df,aes(x=x.ints,y=y.empirical.sd/Ymax))+
#       geom_point(size=7,shape=4)+xlab("")+
#       ylab("second-degree centrality")+
#       scale_x_discrete(labels=X.lab)+
#       scale_y_continuous(limits=c(0,1))+
#       geom_polygon(data=df_error,mapping=aes(x=x,y=y.sd/Ymax),
#                    fill="blue",alpha=0.4)+
#       theme(axis.text.x = element_text(angle=90,
#                                        size = rel(3),vjust=0.5),
#             axis.text.y = element_text(size = rel(3)),
#             axis.title.y = element_text(size=rel(2.5),vjust=0.1),
#             plot.margin = unit(c(0.2,0.2,0.1,1),"cm"))+
#       geom_text(label=paste("MODEL ",model,sep=""),
#                 size=rel(12),x=0.92*max(X.ints),y=0.97)    
#     theme_set(theme_bw())
#     print(g)  
#     dev.off() 
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
#     save("ymedian.sd","yplus.sd","ymedian.sd",
#          file=paste("data-model-",model,".RData",sep="")) 
  }
  sink()
  
  
  #calculate the ranks
#   eigenvector.original.sorted = 
#     eigenvector.original[order(eigenvector.original[,2],
#                                decreasing=TRUE),]
#   indegree.original.sorted = 
#     indegree.original[order(indegree.original[,2],
#                             decreasing=TRUE),]
#   
#   secondindegree.original.sorted = 
#     sd.original[order(sd.original[,2],
#                             decreasing=TRUE),]
#   
#   main.axis = indegree.original.sorted
#   auxillary.axis = secondindegree.original.sorted
#   
#   X.lab=main.axis[,1]  
#   X.ints = seq(1,length(X.lab))
#   ymax=rep(NA,length(X.lab)); ymin=rep(NA,length(X.lab));pch_min=rep(NA,length(X.lab));
#   pch_max=rep(NA,length(X.lab));
  #main.axis to auxillary.axis rank change
  #main.axis is an empty square, auxillary.axis is full circle
#   for (i in 1:length(X.lab)) {
#     ev.rank = which(main.axis[,1] == X.lab[i])
#     indegree.rank = which(auxillary.axis[,1] == X.lab[i])
#     ymin[i] = min(ev.rank,indegree.rank)
#     ymax[i] = max(ev.rank,indegree.rank)
#     if (ev.rank < indegree.rank) {
#       pch_min[i]=0 ; pch_max[i]=16
#     } else {
#       pch_min[i]=16; pch_max[i]=0
#     }     
#   }
#   
#   df.rank.change = data.frame(x=X.ints,ymin=ymin,
#                               ymax=ymax,pch_min=pch_min,
#                               pch_max=pch_max)
#   
#   CairoPDF(file=paste(year,colony,"-rankchange2.pdf",sep=""),width=16,height=10) 
#   g=ggplot(data=df.rank.change,aes(x=x))+
#     geom_linerange(aes(x=x,ymin=ymin,ymax=ymax))+
#     geom_point(aes(x=x,y=ymin),shape=pch_min,size=7)+
#     geom_point(aes(x=x,y=ymax),shape=pch_max,size=7)+xlab("")+
#     ylab("rank change")+scale_x_discrete(labels=X.lab)+
#     theme(axis.text.x = element_text(angle=90,size = rel(3),vjust=0.5),
#           axis.text.y = element_text(size = rel(3)),
#           axis.title.y = element_text(size=rel(2.5),vjust=0.1),
#           plot.margin = unit(c(0.2,0.2,0.1,1),"cm"))  
#   theme_set(theme_bw())
#   print(g)
#   dev.off()
#  
}
  

#check the existence of a giant cluster 



# #calcualte level of heterogeneity
# 
# calc.heterogeneity = function(indegree.vector) {
#   return (sum(indegree.vector^2) / sum(indegree.vector))
# }
# 
# for (d in dirs) {
#   print(paste("Entering ",d,sep=""))
#   setwd(d)
#   #tokenize dir
#   s = strsplit(d,"_")  
#   year=strsplit(s[[1]][5],"/")[[1]][2] # 2008
#   colony=paste("_",s[[1]][6],sep="") # "_GB2"   
#   original.in=paste("indegree_original_",year,".dat",sep="")
#   
#   indegree.original = read.table(original.in)
#   indegree.original[,1] = substr(indegree.original[,1],7,10)
#   indegree.original = indegree.original[order(indegree.original[,1]),]  
#   empirical_heterogeneity = calc.heterogeneity(as.numeric(indegree.original[,2]))
#  
#   cat(empirical_heterogeneity); cat("\n")
#   sink(file=paste(year,"-heterogeneity.dat",sep=""))
#   cat(paste("heterogeneity \t ",empirical_heterogeneity,sep=""))
#   cat(paste("molloy reed \t ",
#   (sum(indegree.original[,2]^2)-2*sum(indegree.original[,2]))/nrow(indegree.original),sep=""))
#   sink()
#   options(warn=3)
# #   CairoPDF(file=paste(year,colony,"-heterogeneity.pdf",sep=""),width=10,height=10)
# #   for (model in  c(1,2,3,5)) {    
# #     shuffled.in=paste("indegree_shuffled_",year,"_model-",model,".dat.gz",sep="")      
# #     indegree.shuffled = read.table(shuffled.in)
# #     indegree.shuffled.matrix = 
# #       matrix(indegree.shuffled[,2],
# #              nrow = nrow(indegree.original),
# #              ncol = length(indegree.shuffled[,2]) / nrow(indegree.original))
# #     
# # 
# #         rownames(indegree.shuffled.matrix) = substr(indegree.shuffled[1:nrow(indegree.original),1],7,10)
# #         indegree.shuffled.matrix = indegree.shuffled.matrix[order(rownames(indegree.shuffled.matrix)),]    
# #         indegree.shuffled.matrix = 
# #           indegree.shuffled.matrix[,colSums(indegree.shuffled.matrix)!=0]
# #     
# #     heterogeneity.vector=apply(indegree.shuffled.matrix,2,calc.heterogeneity)
# #     
# #     hist(heterogeneity.vector,freq=FALSE,breaks=50,
# #          main=paste(year," - Model ",model,sep=""),xlab="simulation")        
# #   }
# #   dev.off()
  
  
}