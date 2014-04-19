rm(list = ls())
library(Cairo)
library(ggplot2)
#read the health info
setwd("/home/pmavrodiev/Documents/bats/data/activity-health/")
health=read.csv("GB2_health_status.csv",header=TRUE,sep="\t",
                colClasses="character",na.strings="")

#read activity information
setwd("/home/pmavrodiev/Documents/bats/data/activity-health/")
a1=read.table("activities_2007.dat",colClasses="character")
a2=read.table("activities_2008.dat",colClasses="character")
a3=read.table("activities_2009.dat",colClasses="character")
a4=read.table("activities_2010.dat",colClasses="character")
a5=read.table("activities_2011.dat",colClasses="character")
a1=cbind(a1,rep(2007,nrow(a1)))
a1=cbind(a1,rep("GB2",nrow(a1)))
a2=cbind(a2,rep(2008,nrow(a2)))
a2=cbind(a2,rep("GB2",nrow(a2)))
a3=cbind(a3,rep(2009,nrow(a3)))
a3=cbind(a3,rep("GB2",nrow(a3)))
a4=cbind(a4,rep(2010,nrow(a4)))
a4=cbind(a4,rep("GB2",nrow(a4)))
a5=cbind(a5,rep(2011,nrow(a5)))
a5=cbind(a5,rep("GB2",nrow(a5)))
colnames(a1)=c("ID","raw.activity","total.activity","norm.activity","year","colony")
colnames(a2)=c("ID","raw.activity","total.activity","norm.activity","year","colony")
colnames(a3)=c("ID","raw.activity","total.activity","norm.activity","year","colony")
colnames(a4)=c("ID","raw.activity","total.activity","norm.activity","year","colony")
colnames(a5)=c("ID","raw.activity","total.activity","norm.activity","year","colony")


activity=data.frame(rbind(a1,a2,a3,a4,a5))


activity.column=rep(0,nrow(health))
raw.activity = rep(0,nrow(health))

for (i in 1:nrow(health)) {
  found=FALSE
  id1 = health[i,"ID"]
  for (j in 1:nrow(activity)) {
    id2 = substr(activity[j,"ID"],7,10)   
    if (id1 == id2 & activity[j,"year"] == health[i,"Year"]) {
      found=TRUE
      break
    }
  }
  if (found == TRUE) {
    activity.column[i]=as.numeric(activity[j,"norm.activity"])
    raw.activity[i]=as.numeric(activity[j,"raw.activity"])
  }
}

health=cbind(health,activity.column)
health=cbind(health,raw.activity)
head#
# health=cbind(health,as.numeric(health[,"Weight_15_05"])/as.numeric(health[,"FOM"]))
# colnames(health)=c(colnames(health)[1:10],"BI_15_05")
# health=cbind(health,as.numeric(health[,"Weight_24_08"])/as.numeric(health[,"FOM"]))
# colnames(health)=c(colnames(health)[1:11],"BI_24_08")
health=cbind(health,as.numeric(health[,"Weight_31_07"])/as.numeric(health[,"FOM"]))
colnames(health)=c(colnames(health)[1:13],"BI_31_07")

#
health[,"Age"]=as.numeric(health[,"Age"])
health[,"FOM"]=as.numeric(health[,"FOM"])
health[,"Weight_31_07"]=as.numeric(health[,"Weight_31_07"])
health.subset = subset(health,raw.activity>0)

glm.family=poisson(link="log")
m1=glm(raw.activity~Age,data=health.subset,family=glm.family)
m2=glm(raw.activity~FOM,data=health.subset,family=glm.family)
m3=glm(raw.activity~Weight_31_07,data=health.subset,family=glm.family)
m4=glm(raw.activity~BI_31_07,data=health.subset,family=glm.family)
m5=glm(raw.activity~Age*FOM,data=health.subset,family=glm.family)
m6=glm(raw.activity~Age*BI_31_07,data=health.subset,family=glm.family)
m7=glm(raw.activity~Age*Weight_31_07,data=health.subset,family=glm.family)
m8=glm(raw.activity~FOM*Weight_31_07,data=health.subset,family=glm.family)


glm.family=gaussian()
m1=glm(activity.column~Age,data=health.subset,family=glm.family)
m2=glm(activity.column~FOM,data=health.subset,family=glm.family)
m3=glm(activity.column~Weight_31_07,data=health.subset,family=glm.family)
m4=glm(activity.column~BI_31_07,data=health.subset,family=glm.family)
m5=glm(activity.column~Age*FOM,data=health.subset,family=glm.family)
m6=glm(activity.column~Age*BI_31_07,data=health.subset,family=glm.family)
m7=glm(activity.column~Age*Weight_31_07,data=health.subset,family=glm.family)
m8=glm(activity.column~FOM*Weight_31_07,data=health.subset,family=glm.family)


#

plot(as.numeric(health[,"Age"]),as.numeric(health[,"activity.column"]),type="p")
plot(as.numeric(health[,"FOM"]),as.numeric(health[,"activity.column"]),type="p")
# plot(as.numeric(health[,"BI_15_05"]),as.numeric(health[,"activity.column"]),type="p")
# plot(as.numeric(health[,"BI_24_08"]),as.numeric(health[,"activity.column"]),type="p")
plot(as.numeric(health[,"BI_31_07"]),as.numeric(health[,"activity.column"]),type="p")


plot(as.numeric(health[,"Age"]),as.numeric(health[,"raw.activity"]),type="p")
plot(as.numeric(health[,"FOM"]),as.numeric(health[,"raw.activity"]),type="p")
plot(as.numeric(health[,"BI_31_07"]),as.numeric(health[,"raw.activity"]),type="p")

##
library("vars")
data("Canada")
plot(Canada,nc=2)
adf1 <- summary(ur.df(Canada[, "prod"], type = "trend", lags = 2))

