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
  
  original=paste("eigenvector_original_",year,".dat",sep="")
  CairoPDF(file=paste(year,colony,".pdf",sep=""),width=16,height=10)
  for (model in 1:6) {
    
    shuffled=paste("eigenvector_shuffled_",year,"_model-",model,".dat.gz",sep="")
    
    eigenvector.original = read.table(original)
    eigenvector.original[,1] = substr(eigenvector.original[,1],7,10)
    eigenvector.original = eigenvector.original[order(eigenvector.original[,1]),]
   
    eigenvector.shuffled = read.table(shuffled)    
    eigenvector.shuffled.matrix = 
      matrix(eigenvector.shuffled[,2],
             nrow = nrow(eigenvector.original),
             ncol = length(eigenvector.shuffled[,2]) / nrow(eigenvector.original))

    
    rownames(eigenvector.shuffled.matrix) = substr(eigenvector.shuffled[1:nrow(eigenvector.original),1],7,10)
    eigenvector.shuffled.matrix = eigenvector.shuffled.matrix[order(rownames(eigenvector.shuffled.matrix)),]    
    eigenvector.shuffled.matrix = 
      eigenvector.shuffled.matrix[,colSums(eigenvector.shuffled.matrix)!=0]
    
    X=seq(1,nrow(eigenvector.original))
    X.lab = rownames(eigenvector.shuffled.matrix)
    X.ints = seq(1,length(X.lab))

    yminus = apply(eigenvector.shuffled.matrix,1,quantile,c(0.025,0.975))[1,]
    
    yplus = apply(eigenvector.shuffled.matrix,1,quantile,c(0.025,0.975))[2,]  
    
    ymedian = apply(eigenvector.shuffled.matrix,1,median)
    
    
    df=data.frame(x.ints=X.ints,                  
                  y.empirical=eigenvector.original[,2])
    
    df_error = data.frame(x=c(X.ints,rev(X.ints)),
                          y=c(as.numeric(yminus),rev(as.numeric(yplus))),
                          y.median=c(as.numeric(ymedian),rev(as.numeric(ymedian))))
    

    g=ggplot(data=df,aes(x=x.ints,y=y.empirical))+
        geom_point(size=7,shape=4)+xlab("")+ylab("eigenvector centrality")+
        scale_x_discrete(labels=X.lab)+
        scale_y_continuous(limits=c(0,1))+
        geom_polygon(data=df_error,mapping=aes(x=x,y=y),fill="blue",alpha=0.4)+
      theme(axis.text.x = element_text(angle=90,size = rel(3),vjust=0.5),
            axis.text.y = element_text(size = rel(3)),
            axis.title.y = element_text(size=rel(2.5),vjust=0.1),
            plot.margin = unit(c(0.2,0.2,0.1,1),"cm"))
    theme_set(theme_bw())
    print(g)  
    #save("ymedian","yplus","ymedian",file=paste("data-model-",model,".RData",sep=""))    
  }
  dev.off() 
}


