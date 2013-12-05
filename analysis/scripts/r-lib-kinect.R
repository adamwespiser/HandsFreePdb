source("./r-constants-h.R")





check_for_tap <- function(  df,
                            i_end,
                            lookback,
                            still_threshold,
                            ztap_ct_f=1,
                            ztap_ct_r=1,
                            ytap_ct_f,
                            ytap_ct_r,
                            ztap_vel_f=TAP_VELOCITY_F,
                            ztap_vel_r=TAP_VELOCITY_R,
                            ytap_vel_f,
                            ytap_vel_r,
                            still_c   ,
                            h2_still_frames=0,
                            h2_still_threshold=0){ 

    return_value <- 0
    i_start <- i_end - lookback + 1
    #print(still_c)
    #print(i_stop)

    if(df$h1vel[i_end] < still_threshold) {
        ztap_forward_found <- 0
        ztap_reverse_found <- 0
        ytap_forward_found <- 0
        ytap_reverse_found <- 0
        h1_still_found     <- 0
        h2_still_found     <- 0
        for(j in i_start:i_end ){

            z_speed  <- df$h1z_velocity[j]
            y_speed  <- df$h1y_velocity[j]
            h1_speed <- df$h1vel[j] 
            h2_speed <- df$h2vel[j]

            if(h2_speed  < still_threshold){
                h2_still_found <- h2_still_found + 1
            } 

            if(h1_speed  < still_threshold){
                h1_still_found <- h1_still_found + 1
            } 

            if(z_speed  < ztap_vel_f){
                ztap_forward_found <- ztap_forward_found + 1
            } 

            if(z_speed  > ztap_vel_r){
                ztap_reverse_found <- ztap_reverse_found + 1
            }

            if(y_speed  > ytap_vel_f){
                ytap_forward_found <- ytap_forward_found + 1
            } 

            if(y_speed  < ytap_vel_r){
                ytap_reverse_found <- ytap_reverse_found + 1
            }

        } # end of for loop 
        if(    ztap_forward_found >= ztap_ct_f  
            && ztap_reverse_found >= ztap_ct_r 
            && ytap_forward_found >= ytap_ct_f  
            && ytap_reverse_found >= ytap_ct_r
            && sum(df$tap_detected[i_start:i_end]) == 0
            && h1_still_found >= still_c
            && h2_still_found >= h2_still_threshold ) {
            #print("found a tap!!!!")
            return_value<- 1
        } 
    } # end of if movinG

    return_value
}



