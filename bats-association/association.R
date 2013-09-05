library(RMySQL)

#for year 2008
Year=2010
control.boxes = as.character(c( 
  10001,
  10002,
  10003,
  100011,
  100033,
  1000111 
))

stimulus.boxes = as.character(c( 
  331,
  332,
  3311,  
  33111,
  661,  
  663,
  6611,
  1001,
  1002,
  1003  
)) 
  
normal.boxes = NULL

db = dbConnect(MySQL(),host="localhost", # your host, usually localhost
                       user="root", # your username
                       password="1234567890", # your password
                       dbname="GB2_bats_new", # name of the data base
                       unix.socket="/tmp/mysql.sock"
)

rs = dbSendQuery(db,paste("SELECT DISTINCT id_bat FROM Findings where fin_date>\"",Year,"-05-05\" and fin_date<\"",Year,"-31-12\"",sep=""))
#
unique.bats=fetch(rs,n=-1)
mat.control.boxes=matrix(0,nrow=nrow(unique.bats),ncol=nrow(unique.bats))
mat.stimulus.boxes=matrix(0,nrow=nrow(unique.bats),ncol=nrow(unique.bats))
mat.normal.boxes=matrix(0,nrow=nrow(unique.bats),ncol=nrow(unique.bats))
mat.all.same.date.boxes=matrix(0,nrow=nrow(unique.bats),ncol=nrow(unique.bats))
mat.different.boxes.same.date=matrix(0,nrow=nrow(unique.bats),ncol=nrow(unique.bats))
rownames(mat.control.boxes) = unique.bats[,1]
colnames(mat.control.boxes) = unique.bats[,1]
rownames(mat.stimulus.boxes) = unique.bats[,1]
colnames(mat.stimulus.boxes) = unique.bats[,1]
rownames(mat.normal.boxes) = unique.bats[,1]
colnames(mat.normal.boxes) = unique.bats[,1]
rownames(mat.all.same.date.boxes) = unique.bats[,1]
colnames(mat.all.same.date.boxes) = unique.bats[,1]
rownames(mat.different.boxes.same.date) = unique.bats[,1]
colnames(mat.different.boxes.same.date) = unique.bats[,1]

#query how many times each pair of bats have been detected on the same date but not
#necessarily in the same box
for (b1 in 1:(length(unique.bats[,1])-1)) {
  for (b2 in (b1+1):length(unique.bats[,1])) {
    query_string = paste("SELECT count(*) as count FROM Findings as s1, (SELECT id_bat, fin_date,fin_box FROM Findings WHERE id_bat=\"",unique.bats[b1,1],"\" and fin_date>\"",Year,"-05-05\" and fin_date<\"",Year,"-31-12\") as s2 WHERE  s1.id_bat=\"",unique.bats[b2,1],"\" and s1.fin_date=s2.fin_date",sep="")
    rs=dbSendQuery(db,query_string)
    count = fetch(rs,n=-1)
    mat.different.boxes.same.date[unique.bats[b1,1],unique.bats[b2,1]] = count[,1]
    mat.different.boxes.same.date[unique.bats[b2,1],unique.bats[b1,1]] = count[,1]
  }
}



#
rs = dbSendQuery(db,"SELECT fin_date FROM Findings where fin_date>\"",Year,"-05-05\" and fin_date<\"",Year,"-31-12\" group by fin_date ORDER BY fin_date  ASC")
unique.dates = fetch(rs,n=-1)

for (date in unique.dates[,1]) {
  rs = dbSendQuery(db,paste("SELECT * FROM Findings where fin_date=\"",date,"\" order by fin_date asc, fin_box",sep=""))
  all.bats.for.date=fetch(rs,n=-1)
  f = factor(all.bats.for.date[,"fin_box"])
  bats.per.box = split(all.bats.for.date,f)
  for (i in 1:length(bats.per.box)) {
    bats.in.box = bats.per.box[[i]][,"id_bat"]
    which.box = bats.per.box[[i]][1,"fin_box"]    
    if (length(bats.in.box) == 1)
      next
    for (b1 in 1:(length(bats.in.box)-1)) {
      for (b2 in (b1+1):length(bats.in.box)) {   
        if (length(which(control.boxes == which.box)) > 0) {
          mat.control.boxes[bats.in.box[b1],bats.in.box[b2]] = 
            mat.control.boxes[bats.in.box[b1],bats.in.box[b2]] + 1
          mat.control.boxes[bats.in.box[b2],bats.in.box[b1]] = 
            mat.control.boxes[bats.in.box[b2],bats.in.box[b1]] + 1
        }
        else if (length(which(stimulus.boxes == which.box)) > 0) {
          mat.stimulus.boxes[bats.in.box[b1],bats.in.box[b2]] = 
            mat.stimulus.boxes[bats.in.box[b1],bats.in.box[b2]] + 1
          mat.stimulus.boxes[bats.in.box[b2],bats.in.box[b1]] = 
            mat.stimulus.boxes[bats.in.box[b2],bats.in.box[b1]] + 1          
        }
        else {
          mat.normal.boxes[bats.in.box[b1],bats.in.box[b2]] = 
            mat.normal.boxes[bats.in.box[b1],bats.in.box[b2]] + 1
          mat.normal.boxes[bats.in.box[b2],bats.in.box[b1]] = 
            mat.normal.boxes[bats.in.box[b2],bats.in.box[b1]] + 1   
          normal.boxes = c(normal.boxes,which.box)
        }
        mat.all.same.date.boxes[bats.in.box[b1],bats.in.box[b2]] = 
          mat.all.same.date.boxes[bats.in.box[b1],bats.in.box[b2]] + 1
        mat.all.same.date.boxes[bats.in.box[b2],bats.in.box[b1]] = 
          mat.all.same.date.boxes[bats.in.box[b2],bats.in.box[b1]] + 1
      }
    }
  }
}
normal.boxes = unique(normal.boxes)
dbDisconnect(db)

fraction.control.to.normal.boxes = length(control.boxes) / 
  (length(control.boxes) + length(normal.boxes))

fraction.experimental.to.normal.boxes = 
  (length(control.boxes)+length(stimulus.boxes)) / 
  (length(control.boxes)+length(stimulus.boxes)+length(normal.boxes))

myqhyper = function(X,prob,fraction) {
  return (qbinom(prob,X,fraction))
}
#c2n = control.to.normal
mat.upper.c2n = structure(vapply(mat.control.boxes+mat.normal.boxes,
                                 myqhyper, numeric(1),0.975,
                                 fraction.control.to.normal.boxes), 
                          dim=dim(mat.control.boxes))
mat.lower.c2n = structure(vapply(mat.control.boxes+mat.normal.boxes,
                             myqhyper, numeric(1),0.025,
                             fraction.control.to.normal.boxes), 
                      dim=dim(mat.control.boxes))
mat.upper.e2n = structure(vapply(mat.control.boxes+mat.normal.boxes+
                                 mat.stimulus.boxes,
                                 myqhyper, numeric(1),0.975,
                                 fraction.experimental.to.normal.boxes), 
                          dim=dim(mat.control.boxes))
mat.lower.e2n = structure(vapply(mat.control.boxes+mat.normal.boxes+
                                 mat.stimulus.boxes,
                                 myqhyper, numeric(1),0.025,
                                 fraction.experimental.to.normal.boxes), 
                          dim=dim(mat.control.boxes))

make.difference.c2n = function() {
  return.matrix = matrix(NA,nrow(mat.control.boxes),ncol(mat.control.boxes))
  rownames(return.matrix) = rownames(mat.control.boxes)
  colnames(return.matrix) = colnames(mat.control.boxes)
  for (i in 1:nrow(mat.control.boxes)) {
    for (j in 1:ncol(mat.control.boxes)) {
      if (i==j) {
        return.matrix[i,j] = "."
      }
      else if (mat.control.boxes[i,j] == 0) {
        return.matrix[i,j] = "o"
      } 
      else {
        upper=mat.control.boxes[i,j]-mat.upper.c2n[i,j] 
        lower=mat.lower.c2n[i,j] - mat.control.boxes[i,j]
        if (upper > 0) {
          return.matrix[i,j] = "+"
        }
        if (lower > 0) {
          return.matrix[i,j] = "-"
        }
        else {
          return.matrix[i,j] = "x"
        }      
      }
    }
  }
  return (return.matrix)
}


make.difference.e2n = function() {
  return.matrix = matrix(NA,nrow(mat.control.boxes),ncol(mat.control.boxes))
  rownames(return.matrix) = rownames(mat.control.boxes)
  colnames(return.matrix) = colnames(mat.control.boxes)
  for (i in 1:nrow(mat.control.boxes)) {
    for (j in 1:ncol(mat.control.boxes)) {
      if (i==j) {
        return.matrix[i,j] = "."
      }
      else if (mat.control.boxes[i,j]+mat.stimulus.boxes[i,j] == 0) {
        return.matrix[i,j] = "o"
      } 
      else {
        upper=mat.control.boxes[i,j]+mat.stimulus.boxes[i,j]-mat.upper.e2n[i,j] 
        lower=mat.lower.e2n[i,j]-mat.control.boxes[i,j]-mat.stimulus.boxes[i,j]
        if (upper > 0) {
          return.matrix[i,j] = "+"
        }
        if (lower > 0) {
          return.matrix[i,j] = "-"
        }
        else {
          return.matrix[i,j] = "x"
        }      
      }
    }
  }
  return (return.matrix)
}



