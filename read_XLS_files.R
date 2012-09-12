library("gdata")

setwd("/home/pmavrodiev/Documents/bats/data/Daniela_GB2_2007/")
#get the box directories
bdirs=list.files(getwd())

for (i in 1:length(bdirs)) {
  box_dir = paste(getwd(),"/",bdirs[i],sep="")
  files = list.files(box_dir)
  for (j in 1:length(files)) {
    file_ext = substr(files[j],nchar(files[j])-2,nchar(files[j]))
    if (file_ext != "txt") {
      txtFileName = paste(substr(files[j],1,nchar(files[j])-4),".txt",sep="")
      data = NULL
      data = read.xls(paste(box_dir,"/",files[j],sep=""),sheet=1,
                          header=FALSE,colClasses = "character")    
      write.table(data,paste(box_dir,"/",txtFileName,sep=""),quote=FALSE,row.names=FALSE,
                             col.names=FALSE,sep=";")    
    }
    
  }
}


data = read.xls("20070716_0037_0a.xls", sheet=1,header=FALSE,colClasses = "character")

