#include <stdio.h>
#include<stdlib.h>
#include<math.h>
#include<stdint.h>

#include "control_mode.h"
#include "gesture.h"
#include "tracking.h"

extern control_struct cinfo;
extern gesture_struct ginfo;
extern handtrack_t tinfo;

// tinfo -> cinfo -> molview 
void online_transform_for_molview(void)
{
// has ginfo bufptr been incremented yet? 
// ans -> yes 
    //int last_mode = cinfo.current_mode; 
	int i, count_grab = 0, count_letgo = 0;
	for (i = 0; i < GESTURE_LOOKBACK; i++) {
		int p = ginfo.bufptr - i;
		p = (p + 89) % 90;

		if (ginfo.gesture_history[p] == GESTURE_GRAB)
			count_grab++;

		if (ginfo.gesture_history[p] == GESTURE_LETGO)
			count_letgo++;
	}

	if (count_grab > GESTURE_COUNT_MIN
	    && ginfo.gesture_history[ginfo.bufptr - 1] == GESTURE_STILL)
		cinfo.current_mode = CTRLMODE_GRAB;

	if (count_letgo > GESTURE_COUNT_MIN)
		cinfo.current_mode = CTRLMODE_FREE;

    //chain mode in response to tap
    if (ginfo.current_gesture == GESTURE_TAP
        && cinfo.current_mode == CTRLMODE_FREE){ 
        
        cinfo.chain_mode = (cinfo.chain_mode + 1) % 3; 
    }



//only if we're going to limit max hand velocity will we check for it
	int condition = 1;
	if (LIMIT_MAX_HAND_VELOCITY) {
		float h1_speed = tinfo.hand1_vel_mag[tinfo_prev_bufptr()];
		float h2_speed = tinfo.hand2_vel_mag[tinfo_prev_bufptr()];
		condition = (h1_speed < MAX_HAND_VELOCITY
			     && h2_speed < MAX_HAND_VELOCITY);
	}


cinfo.hand1_vel = tinfo.hand1_vel_mag[tinfo_prev_bufptr()]; 
cinfo.hand2_vel = tinfo.hand2_vel_mag[tinfo_prev_bufptr()];

//int condition = hands_moving_on_center_line(); 

	if (cinfo.current_mode == CTRLMODE_GRAB && condition) {
        
		update_cinfo_w_tinfo();
		if (LOG_TRANSFORM_TO_FILE) {
			printf("%f,%f,%f,%f,%f,%f", tinfo.hand1.z,
			       tinfo.hand1.y, tinfo.hand2.z, cinfo.hand1_x_out,
			       cinfo.hand1_y_out, cinfo.hand2_z_out);
		}
	}
    /*if (last_mode == GESTURE_LETGO && cinfo.current_mode == GESTURE_STILL){
        set_buffer_to_current_hand_pos(); 
    }*/
}

void online_transform_for_molview_new(void)
{
// has ginfo bufptr been incremented yet? 
// ans -> yes 
    //int last_mode = cinfo.current_mode; 

	if(ginfo.current_gesture == GESTURE_GRAB)
		cinfo.current_mode = CTRLMODE_GRAB;
    else if (ginfo.current_gesture == GESTURE_LETGO)
		cinfo.current_mode = CTRLMODE_FREE;

    //chain mode in response to tap
    if (ginfo.current_gesture == GESTURE_TAP
        && cinfo.current_mode == CTRLMODE_FREE){ 
        cinfo.chain_mode = (cinfo.chain_mode + 1) % 3; 
    }



//only if we're going to limit max hand velocity will we check for it
	int condition = 1;
	if (LIMIT_MAX_HAND_VELOCITY) {
/*		float h1_speed = tinfo.hand1_vel_mag[tinfo_prev_bufptr()];
		float h2_speed = tinfo.hand2_vel_mag[tinfo_prev_bufptr()];
		condition = (h1_speed < MAX_HAND_VELOCITY
			     && h2_speed < MAX_HAND_VELOCITY);
*/
		condition = (tinfo.hand1_vel_mag[tinfo_prev_bufptr()] < MAX_HAND_VELOCITY
			      && tinfo.hand2_vel_mag[tinfo_prev_bufptr()] < MAX_HAND_VELOCITY);
	}


//int condition = hands_moving_on_center_line(); 

	if (cinfo.current_mode == CTRLMODE_GRAB && condition) {
        
		update_cinfo_w_tinfo();
		if (LOG_TRANSFORM_TO_FILE) {
			printf("%f,%f,%f,%f,%f,%f", tinfo.hand1.z,
			       tinfo.hand1.y, tinfo.hand2.z, cinfo.hand1_x_out,
			       cinfo.hand1_y_out, cinfo.hand2_z_out);
		}
	}
    /*if (last_mode == GESTURE_LETGO && cinfo.current_mode == GESTURE_STILL){
        set_buffer_to_current_hand_pos(); 
    }*/
}

void init_cinfo_values()
{
	cinfo.hand1_x_out = X_INIT_VALUE;
	cinfo.hand1_y_out = Y_INIT_VALUE;
	cinfo.hand2_z_out = Z_INIT_VALUE;
}

void update_cinfo_w_tinfo()
{
	int ptr = tinfo_prev_bufptr();
	//cinfo.hand1_x_out += X_SCALING_FACTOR * tinfo.hand1_velocity[ptr].x;
	//cinfo.hand1_y_out += Y_SCALING_FACTOR * tinfo.hand1_velocity[ptr].y;
	//cinfo.hand2_z_out += Z_SCALING_FACTOR * tinfo.hand2_velocity[ptr].z;

    cinfo.center_vx = (tinfo.hand_center_velocity[ptr].x);
    cinfo.center_vy = (tinfo.hand_center_velocity[ptr].y);
    cinfo.center_vz = (tinfo.hand_center_velocity[ptr].z);
}


int hands_moving_on_center_line()
{
	int curr_ptr = tinfo_curr_bufptr();
	int val =
	    ((tinfo.hand2_to_center_velocity[curr_ptr] > MAX_HAND_VELOCITY &&
	      tinfo.hand1_to_center_velocity[curr_ptr] <
	      (-1 * MAX_HAND_VELOCITY))
	     || (tinfo.hand2_to_center_velocity[curr_ptr] <
		 (-1 * MAX_HAND_VELOCITY)
		 && tinfo.hand1_to_center_velocity[curr_ptr] >
		 MAX_HAND_VELOCITY)
	    );

	return val;
}




