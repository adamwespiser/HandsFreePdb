library(ggplot2) 
library(reshape)

#df <- read.csv(file="~/work/kinect/analysis/data/tap_gesture_control_AW.csv",stringsAsFactors=FALSE) 
#df <- read.csv(file="~/tmpOut.csv",stringsAsFactors=FALSE) 
df<-read.csv(file="/home/adam/work/kinect/analysis/data/tmp_save.csv",stringsAsFactors=FALSE)
#df <- read.csv(file="~/work/kinect/analysis/data/tap_gesture_AW.csv",stringsAsFactors=FALSE) 
#df <- read.csv(file="~/work/kinect/analysis/data/accel_test_losttracking.csv",stringsAsFactors=FALSE) 
#df <- read.csv(file="~/work/kinect/analysis/data/kz_hm_grab.csv",stringsAsFactors=FALSE) 
#df <- df[500:1600,]
df <- transform(df, center_vel = (center_x_velocity^2 + center_y_velocity^2 + center_z_velocity^2)^0.5)
df <- transform(df, h1vel = (h1x_velocity^2 + h1y_velocity^2 + h1z_velocity^2)^0.5)
df <- transform(df, h2vel = (h2x_velocity^2 + h2y_velocity^2 + h2z_velocity^2)^0.5)
df <- transform(df, still = ifelse(h1vel < 0.08, "still", "moving"))

#df <- transform(df, h1acc = (h1x_accel^2 + h1y_accel^2 + h1z_accel^2)^0.5)
#df <- transform(df, h2acc = (h2x_accel/abs(h2x_accel) * h2y_accel/abs(h2y_accel) * h2z_accel/abs(h2z_accel))*((h2x_accel^2 + h2y_accel^2 + h2z_accel^2)^0.5))
g1 <- ggplot(df, aes(y=h1x_velocity,x=time)) + geom_line()


dfm <- melt(df[c("h1x_velocity","h1y_velocity","h1z_velocity","h1vel","h2vel","time","still")],id.var=c("time","still"))
g2 <- ggplot(dfm,aes(x=time,y=value,color=still))+geom_point()+facet_grid(scales="free",variable ~ .)+theme_bw()

dfm <- melt(df[c("h2x_velocity","h2y_velocity","h2z_velocity","h1vel","h2vel","time","still")],id.var=c("time","still"))
g22 <- ggplot(dfm,aes(x=time,y=value,color=still))+geom_point()+facet_grid(scales="free",variable ~ .)+theme_bw()



dfm <- melt(df[c("h1x_cg", "h1y_cg", "h1z_cg","h2x_cg", "h2y_cg", "h2z_cg","time","still")],id.var=c("time","still"))
g33 <- ggplot(dfm,aes(x=time,y=value,color=still))+geom_point()+facet_grid(scales="free",variable ~ .)+theme_bw()





quit("c")





dfm <- melt(df[c("hand_distance","center_vel","hand1_to_center","hand2_to_center","time")],id.var="time")
g3 <- ggplot(dfm,aes(x=time,y=value))+geom_line()+facet_grid(scales="free",variable ~ .)+ theme_bw()


dfm <- melt(df[c("center_vel","center_x","center_y","center_z","time")],id.var="time")
g4 <- ggplot(dfm,aes(x=time,y=value))+geom_point()+facet_grid(scales="free"
,variable ~ .)+theme_bw()


dfm <- melt(df[c("center_vel","hand1_to_center","hand2_to_center","time")],id.var="time")
g5 <- ggplot(dfm,aes(x=time,y=value))+geom_point()+facet_grid(variable ~ .)+theme_bw()


dfm <- melt(df[c("center_vel","center_y_velocity","center_x_velocity","center_y_velocity","time")],id.var="time")
g6 <- ggplot(dfm,aes(x=time,y=value))+geom_line()+facet_grid(scales="free",variable ~ .) + theme_bw()


dd <- df[c("h1x_position","h1y_position","h2x_position","h2y_position","time")]

h1 <-  ggplot(dd,aes(y=h1y_position*-1,x=h1x_position,color=time))+geom_path()+theme_bw()+scale_color_gradient()+theme_bw()
h2 <-  ggplot(dd,aes(y=h2y_position*-1,x=h2x_position,color=time))+geom_path()+theme_bw()+scale_color_gradient() +theme_bw()

dd1 <- df[c("h1x_position","h1y_position","time")]
dd2 <- df[c("h2x_position","h2y_position","time")]
colnames(dd1) <- c("x","y","time") 
colnames(dd2) <- c("x","y","time") 
dd1$hand <- "1"
dd2$hand <- "2"
ddb <- rbind(dd1,dd2)
h3 <-  ggplot(ddb,aes(y=y*-1,x=x,color=time))+geom_path()+theme_bw()+scale_color_gradient(low="red",high="blue") + facet_grid(hand ~ .)+theme_bw()



dfm <- melt(df[c("h1x_position","h1y_position","h2x_position","h2y_position","time")],id.var="time")
g7 <- ggplot(dfm,aes(x=time,y=value))+geom_line()+facet_grid(scales="free",variable ~ .) + theme_bw()

dfm <- melt(df[c("hand_distance","center_vel","hand1_to_center","hand2_to_center","h1vel","h2vel","time")],id.var="time")
g8 <- ggplot(dfm,aes(x=time,y=value))+geom_line()+facet_grid(scales="free",variable ~ .)+theme_bw()


#dfm <- melt(df[c("h1x_accel","h1y_accel","h1z_accel","h2x_accel" ,"h2y_accel","h2z_accel","time")],id.var="time")
#a1 <- ggplot(dfm,aes(x=time,y=value))+geom_line()+geom_point()+facet_grid(scales="free",variable ~ .) + theme_bw()

#dfm <- melt(df[c("h1acc","h1vel","h2acc","h2vel","time")],id.var="time")
#a2 <- ggplot(dfm,aes(x=time,y=value))+geom_line()+geom_point()+facet_grid(scales="free",variable ~ .) + theme_bw()

mm1 <- melt(df,id.var="time")
ggplot(mm1,aes(x=time,y=value))+geom_point()+facet_grid(scales="free",variable ~ .)
ggsave("../data/allVars_mostRecent.pdf",height=20)



