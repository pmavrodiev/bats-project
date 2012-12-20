rm(list = ls())
setwd("/home/pmavrodiev/Dropbox/code")

library(png)
library(Cairo)
library(R.utils)
library(debug)
library(animation)
library(lattice)
library(grid)


boxes.data = read.table("box-locations.txt",fill=TRUE,header=TRUE)
#get coordinates of the rectangular map
x.min=min(boxes.data[,"x"],na.rm=TRUE);y.min=min(boxes.data[,"y"],na.rm=TRUE)
x.max=max(boxes.data[,"x"],na.rm=TRUE);y.max=max(boxes.data[,"y"],na.rm=TRUE)
#normalise the box coordinates w.r.t. x.min,y.min,x.max,y.max
boxes.data[,"x"] = boxes.data[,"x"]-x.min
boxes.data[,"y"] = boxes.data[,"y"]-y.min
x.min=min(boxes.data[,"x"],na.rm=TRUE);y.min=min(boxes.data[,"y"],na.rm=TRUE)
x.max=max(boxes.data[,"x"],na.rm=TRUE);y.max=max(boxes.data[,"y"],na.rm=TRUE)
####ggplot2
setwd("/home/pmavrodiev/Documents/bats/result_files")
CairoPDF(file="box-locations.pdf",width=10,height=10)
width=9
d=data.frame(x1=boxes.data[!is.na(boxes.data[,"x"]),"x"], 
             x2=boxes.data[!is.na(boxes.data[,"x"]),"x"]+width, 
             y1=boxes.data[!is.na(boxes.data[,"y"]),"y"],
             y2=boxes.data[!is.na(boxes.data[,"y"]),"y"]+width, 
             r=boxes.data[!is.na(boxes.data[,"box_id"]),"box_name"])
p=ggplot()
p=p+scale_x_continuous(name="x",limits=c(0,max(x.max+16,y.max+16)))
p=p+scale_y_continuous(name="y",limits=c(0,max(x.max+16,y.max+16)))+coord_fixed()

p=p+geom_rect(data=d, mapping=aes(xmin=x1, xmax=x2, ymin=y1, ymax=y2),
              color="blue",fill="blue")
p=p+geom_text(data=d, aes(x=x1+(x2-x1)/2, y=y1, label=r), size=4,
              fontface=1,vjust=1.2)
p=p+theme_grey()
p=p+theme(axis.text.x = element_text(size = rel(2)))
p=p+theme(axis.text.y = element_text(size = rel(2)))
p=p+theme(axis.title.x = element_text(size = rel(2)))
p=p+theme(axis.title.y = element_text(size = rel(2), angle = 0))
p
dev.off()



box_seq = c("0a","0b","0c","0d","0e","0f","0g","100a","100b","100c","100d","100e","33a","33b","33c","33d","33e","66a","66b","66c","66d")
boxes = matrix(0,nrow=21,ncol=4)
boxes = data.frame(boxes,row.names=box_seq)

colnames(boxes) = c("xleft","ybottom","xright","ytop")

box_width=0.03

#box coordinates
boxes["100d",] = c(1.2,1.21,1.2+box_width,1.21+box_width)
boxes["33c",] = c(1.17,1.25,1.17+box_width,1.25+box_width)
boxes["0c",] = c(1.13,1.29,1.13+box_width,1.29+box_width)
boxes["66c",] = c(1.1,1.33,1.1+box_width,1.33+box_width)
boxes["0a",] = c(1.19,1.54,1.19+box_width,1.54+box_width)
boxes["100c",] = c(1.26,1.58,1.26+box_width,1.58+box_width)
boxes["33b",] = c(1.27,1.62,1.27+box_width,1.62+box_width)
boxes["66b",] = c(1.42,1.6,1.42+box_width,1.6+box_width)
boxes["0e",] = c(1.49,1.61,1.49+box_width,1.61+box_width)
boxes["66d",] = c(1.542,1.605,1.542+box_width,1.605+box_width)
boxes["100b",] = c(1.59,1.63,1.59+box_width,1.63+box_width)
boxes["0d",] = c(1.58,1.685,1.58+box_width,1.685+box_width)
boxes["0b",] = c(1.58,1.5,1.58+box_width,1.5+box_width)
boxes["33d",] = c(1.74,1.65,1.74+box_width,1.65+box_width)
boxes["33a",] = c(1.81,1.66,1.81+box_width,1.66+box_width)
boxes["66a",] = c(1.9,1.69,1.9+box_width,1.69+box_width)
boxes["100a",] = c(1.94,1.689,1.94+box_width,1.689+box_width)

#add to existing plot
dome = function() {
  
  #create base plot
  map = readPNG("map.png")
  #CairoPDF("map.pdf",width=30,height=25)
  x11()
  plot(1:2, type='n', main="",xaxt="n",yaxt="n",xlab="", ylab="",bty="n",cex.axis=3)
  lim=par()
  rasterImage(map, lim$usr[1], lim$usr[3], lim$usr[2], lim$usr[4])
 
  for (i in box_seq)
    rect(xleft=boxes[i,"xleft"],ybottom=boxes[i,"ybottom"],xright=boxes[i,"xright"],ytop=boxes[i,"ytop"])
  
  Sys.sleep(15)
  #savePlot(paste("figures/",counter,".png",sep=""),type="png")
  #counter <<- counter + 1
  
  
  rownames(boxes_occupation) = box_seq
  
  duplicate_dates = duplicated(data_time_column)
  duplicate_dates = which(duplicate_dates == TRUE)
  consequtive_dates = seqToIntervals(duplicate_dates)
  extra_dates = consequtive_dates[,"from"]-1
  all_duplicate_dates = seqToIntervals(sort(c(extra_dates,duplicate_dates)))

  i=1
while (i <= length(data_time_column)) {
  print(i)
  current_date = data_time_column[i]
  d = which(all_duplicate_dates[,"from"] == i )
  search_result = i
  if (length(d) != 0 ) {
    search_result = all_duplicate_dates[d,"from"]:all_duplicate_dates[d,"to"]
  }
  i = i + length(search_result)  
  plotRecordings(search_result,current_date)    
  
}
}


ani.options("nmax"=T,"interval"=0.4,"outdir"=getwd())
saveVideo(dome(),video.name="test.mp4",img.name="Rplot",clean=FALSE)


#dev.off()

delme = function() {
plot(1, ann = FALSE, type = "n", axes = FALSE)
for (i in 1:length(s)) {  
  print(i)
  ani.start()
  text(1, 1, s[i], col = "blue", cex = 7)
  ani.stop()
  Sys.sleep(0.001)
}
}

saveVideo(delme(),video.name="test.mp4",img.name="Rplot",clean=FALSE)
