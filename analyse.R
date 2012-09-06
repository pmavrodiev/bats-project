rm(list = ls())
setwd("/home/pmavrodiev/Dropbox/bats")

library(png)
library(Cairo)
library(R.utils)
library(debug)
library(animation)
library(lattice)
library(grid)

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

#read input file with records
data = read.table("temp")
data[,1] = paste(data[,1],data[,2],sep=" ")
data[,2] = data[,3]
data[,3] = data[,4]
data[,4] = data[,5]


#occupation of boxes over time
boxes_occupation = matrix(0,nrow=21,ncol=nrow(data))
rownames(boxes_occupation) = box_seq

data_time_column=strptime(paste(data[,1],data[,2],sep=" "),"%Y-%b-%d %H:%M:%S")
time_span = seq(min(data_time_column),max(data_time_column),by=1)

#dummy
counter = 1

#plot the recordings for a particular date
#a graphics device must have been opened already
plotRecordings = function(recordings,current_date) {
  
  which_boxes = rep("",length(recordings))
  for (i in 1:length(recordings)) {
    which_boxes[i] = as.character(data[recordings[i],4])
  }
  #calculate the coordinates within each box
  plotting_coordinates = calcBoxCoord(which_boxes,recordings)
  points(plotting_coordinates[,"xcoords"],plotting_coordinates[,"ycoords"],pch=".",col="blue")
  #savePlot(paste("figures/",counter,".png",sep=""))
  #counter <<- counter + 1
  
}



#calculate the coordinates within each box - MAX 9 BATS IN A BOX AT ANY GIVEN TIME
calcBoxCoord = function(which__boxes,recordings) {  
 
  #auxillary to calculate the x/y positions of bats in case more than 1 bat occupies a given box. 
  #coordinates are RELATIVE to the box coordinates
  offset = function(new_bat) {
    xy=c(0,0)
    base_offsets = matrix(c(-1,1,0,-1,1,-1,1,0,1,1,0,1,-1,1,-1,0),nrow=8,ncol=2,byrow=TRUE)
    
    #determine the level at which the new bat is to be inserted. ask PM about the formulas below
    lower_bound = 2/4 + sqrt(1+new_bat)/2
    upper_bound = 6/4 + sqrt(new_bat)/2
    #mathematically -> 1<upper_bound-lower_bound<2, which makes the call to ceiling sane. ask PM for details
    level = ceiling(lower_bound)
    #start/end of level and side lenght
    start = 4*(level-2)*(level-1)+1
    end = 4*level*(level-1)
    side = 2*level - 1
    #determine the position offset of the new bat within the level it belongs to
    #calculated as (bat number - start of level). ask PM for the formula
    position_offset = new_bat - start 
    #level corners
    topleft = start
    topright = topleft + side - 1
    bottomright = topright + side - 1
    bottomleft = bottomright + side - 1
    #topleft, topright, bottomleft and bottomright offsets for the calculated level
    off_topleft = c(-level+1,level-1)
    off_topright = c(level-1,level-1)
    off_bottomleft = c(-level+1,-level+1)
    off_bottomright = c(level-1,-level+1)
    #translate the position_offset into exact x,y coordinate
    if (new_bat == topleft) {
      xy = off_topleft
    } else if (new_bat > topleft && new_bat <= topright) {
      xy[1] = off_topleft[1] + position_offset   #x
      xy[2] = off_topright[2]                    #y
    } else if (new_bat > topright && new_bat <= bottomright) {
      xy[1] = off_topright[1]                                #x
      xy[2] = off_topright[2] - (position_offset - side + 1) #y
    } else if (new_bat > bottomright && new_bat <= bottomleft) {
      xy[1] = off_bottomright[1] - (position_offset - 2*side + 2) #x
      xy[2] = off_bottomright[2]                                  #y  
    } else if (new_bat > bottomleft && new_bat > topleft) { #note the second boolean clause is ">=" not "<="
      xy[1] = off_bottomleft[1]                                   #x
      xy[2] = off_bottomleft[2] + (position_offset - 3*side + 3) #y
    }
    
    return (xy)
  }
  
  #create return data frame
  df = matrix(0,nrow=length(which__boxes),ncol=2)
  temp = seq(1:length(which__boxes))
  rowNames = paste(data[recordings,2],temp,sep=".")
  df_coordinates = data.frame(df,row.names=rowNames)
  
  colnames(df_coordinates) = c("xcoords","ycoords")
  
  for (i in 1:length(which__boxes)) {
    #already_in = length(which(alreadyDone == which__boxes[i]))
    idx = which(boxes_occupation[which__boxes[i],]==0)[1]
    #if (idx != 1) idx = idx - 1
    already_in = idx - 1
    if (already_in != 0) { #this box has already been calculated at least once
        #how many bats are already in the current box?
        #howmany = boxes_occupation[which__boxes[i],which(boxes_occupation[which__boxes[i]]==0)[1]-1]
        granularity = 20
        Offsets = offset(already_in)
        x = boxes[which__boxes[i],"xleft"] + box_width / 3 + box_width / 6 + Offsets[1]*(box_width/ granularity)
        y = boxes[which__boxes[i],"ybottom"] + box_width / 3 + box_width / 6 + Offsets[2]*(box_width/ granularity)
        df_coordinates[i,] = c(x,y)
        boxes_occupation[which__boxes[i],idx] <<-  1
    } else { #first recording for this box
      #calculate x and y coordinates
      x = boxes[which__boxes[i],"xleft"] + box_width / 3 + box_width / 6
      y = boxes[which__boxes[i],"ybottom"] + box_width / 3 + box_width / 6
      df_coordinates[i,] = c(x,y)     
      boxes_occupation[which__boxes[i],idx] <<- 1
    }
  }  
  
  return (df_coordinates)
}

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
