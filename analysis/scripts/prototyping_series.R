library(ggplot2) 
library(reshape)

source("./r-lib-kinect.R")
source("./r-constants-h.R")

saveFile <- "../data/proto_series_zrc.csv"

## need to be determined ahead of time...


param_df <- expand.grid(
                        #lookback=seq(10,30,by=5),
                        lookback=c(35),
                        #still_thresh=seq(0.00,0.2,by=0.02),
                        still_thresh=c(0.1),
                        #zfv=seq(-0.35,-0.2,by=0.05),
                        zfv=c(-0.2),
                        zrv=c(0.1),
                        yfv=c(0.1),
                        yrv=c(-0.1),
                        zfc=c(1),
                        zrc=c(4),
                        yfc=c(1),
                        yrc=c(4),
                        sf=c(1),
                        h2s=c(0.0015)
                    
                        ) 
param_df$control_region_taps   <- 0 
param_df$canon_taps_found      <- 0
param_df$frac_taps_found       <- 0
param_df$total_taps_found      <- 0

tap_detect_window <- 15
#df <- read.csv(file="~/work/kinect/analysis/data/accel_test_losttracking.csv",stringsAsFactors=FALSE) 
#df <- read.csv(file="/home/adam/work/kinect/analysis/data/tap_gesture_AW.csv",stringsAsFactors=FALSE) 
df <- read.csv(file="/home/adam/tmpOut.csv",stringsAsFactors=FALSE) 
start_site=175
canonical_taps <-c(200,240,280,319,355,400,435,476,513,553,589,635,669,715,745,787,821,862,902,940,982,1020,1060,1100,1180,1220,1255,1290,1330,1370,1410,1450,1485,1425,1570,1605,1645,1680,1720,1760,1800,1840,1880,1920,1955,1990,1935,2065,2112,2150,2190) - 160
# canonical_taps are now by row, not time point
param_df$canon_taps <- length(canonical_taps)
print(paste("canonical taps annotated = ", length(canonical_taps)))
control_region <-c(2081,3414) 

df <- transform(df, center_vel = (center_x_velocity^2 + center_y_velocity^2 + center_z_velocity^2)^0.5)
df <- transform(df, h1vel = (h1x_velocity^2 + h1y_velocity^2 + h1z_velocity^2)^0.5)
df <- transform(df, h2vel = (h2x_velocity^2 + h2y_velocity^2 + h2z_velocity^2)^0.5)
df <- transform(df, still = ifelse(h1vel < 0.05, "still", "moving"))
df <- transform(df, tap_detected = 0)


for(j in 1:dim(param_df)[1]){ 
lookback            <- param_df$lookback[j]
still_threshold     <- param_df$still_thresh[j]
z_tap_count_forward <- param_df$zfc[j]
z_tap_count_reverse <- param_df$zrc[j]
z_tap_vel_forward   <- param_df$zfv[j]# -0.3
z_tap_vel_reverse   <- param_df$zrv[j] # 0.2

y_tap_count_forward <- param_df$yfc[j]
y_tap_count_reverse <- param_df$yrc[j]
y_tap_vel_forward   <- param_df$yfv[j]
y_tap_vel_reverse   <- param_df$yrv[j] 
still_count         <- param_df$sf[j]
h2_still            <- param_df$h2s[j]



start_time <- proc.time()[[3]]
df$tap_detected <- 0
for(i in (lookback+start_site):nrow(df)){
#print(i)
result <- check_for_tap(df        = df,
                        i_end     = i,
                        lookback  = lookback,
                        still_t   = still_threshold,
                        ztap_ct_f = z_tap_count_forward,
                        ztap_ct_r = z_tap_count_reverse,
                        ytap_ct_f = y_tap_count_forward,
                        ytap_ct_r = y_tap_count_reverse,
                        ztap_vel_f= z_tap_vel_forward,
                        ztap_vel_r= z_tap_vel_reverse,
                        ytap_vel_f= y_tap_vel_forward,
                        ytap_vel_r= y_tap_vel_reverse,
                        still_c   = still_count,
                        h2_still_frames = lookback * 0.9,
                        h2_still_threshold = h2_still)

df$tap_detected[i] <- result
}
elapsed_time <- proc.time()[[3]] - start_time
print(elapsed_time)

total_taps <- sum(df$tap_detected) 
tap_list   <- df[which(df$tap_detected == 1),]$time

print(paste("taps detected = ",total_taps))
#print(paste("locations:",tap_list))


c_taps_f_vec <- rep(0,length(canonical_taps)) 
for (ii in 1:length(canonical_taps)){
    tap_index <- canonical_taps[ii] 
    i_start   <- tap_index - tap_detect_window 
    i_end     <- ifelse( tap_index + tap_detect_window < nrow(df),  tap_index + tap_detect_window, nrow(df))
    
    if (sum(df$tap_detected[i_start:i_end]) > 0){ 
        c_taps_f_vec[ii] <- 1 
    } 
    
}
c_taps_found <- sum(c_taps_f_vec)
print(paste("canoncial taps found =", c_taps_found))
control_violations <- sum(df$tap_detected[control_region[1]:control_region[2]])
print(paste("control regions violations = ",control_violations)) 

param_df$total_taps_found[j]     <- sum(df$tap_detected)
param_df$control_region_taps[j]  <-  control_violations
param_df$canon_taps_found[j]     <- c_taps_found 
param_df$frac_taps_found[j]      <- c_taps_found / param_df$canon_taps[j]

df.plot <- melt(df[c("h1x_velocity","h1y_velocity","h1z_velocity","h1vel","h2vel","tap_detected","time","still")],id.var=c("time","still"))
plot1<-ggplot(df.plot,aes(x=time,y=value,color=still))+
    geom_point() + 
    facet_grid(scales="free",variable ~ .) + 
    theme_bw() 

}

seePoint <- function(x){
    plot1 + xlim(x-30, x + 30) 
}

write.csv(param_df,file=saveFile)
#df <- transform(df, h1acc = (h1x_accel^2 + h1y_accel^2 + h1z_accel^2)^0.5)
#df <- transform(df, h2acc = (h2x_accel/abs(h2x_accel) * h2y_accel/abs(h2y_accel) * h2z_accel/abs(h2z_accel))*((h2x_accel^2 + h2y_accel^2 + h2z_accel^2)^0.5))


#dfm <- melt(df[c("h1x_velocity","h1y_velocity","h1z_velocity","h1vel","h2vel","time","still")],id.var=c("time","still"))
#g2 <- ggplot(dfm,aes(x=time,y=value,color=still))+geom_point()+facet_grid(scales="free",variable ~ .)+theme_bw()


