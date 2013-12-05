
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<stdint.h>
#include "gesture.h"
#include "tracking.h"
#include "constants.h"

extern gesture_struct ginfo;
extern handtrack_t tinfo;

// a function that iterates over the history of tinfo, and fills in the current gesture array if the
// the sufficient actions have been meet 
void gesture_recognition()
{
	gesture_rec_for_grab_and_letgo_newTAP();
	gesture_inc_bufptr();
}

void gesture_rec_for_grab_and_letgo_newTAP()
{
	int tinfo_ptr = tinfo_prev_bufptr();
    int ginfo_ptr =  ginfo_bufptr(); 
    int current_still = 0 ;
	int i;

	ginfo.current_gesture = GESTURE_NONE;

// primary gestures 
    int letgo_motion = 0, grab_motion = 0, still_motion = 0, tap_forward_motion = 0;
	if (tinfo.hand2_to_center_velocity[tinfo_ptr] > LETGO_VELOCITY &&
	    tinfo.hand1_to_center_velocity[tinfo_ptr] > LETGO_VELOCITY
	    )
        letgo_motion = 1; 

    if (tinfo.hand1_to_center_velocity[tinfo_ptr] > LETGO_VELOCITY && 
        tinfo.hand2_vel_mag[tinfo_ptr] < STILL_VELOCITY)
        tap_forward_motion = 1;



	if (tinfo.hand2_to_center_velocity[tinfo_ptr] < (-1* GRAB_VELOCITY) &&
	    tinfo.hand1_to_center_velocity[tinfo_ptr] < (-1 *GRAB_VELOCITY))
		grab_motion = 1;

	if ((tinfo.hand1_vel_mag[tinfo_prev_bufptr()] < STILL_VELOCITY) &&
	    (tinfo.hand2_vel_mag[tinfo_prev_bufptr()] < STILL_VELOCITY)
	    ){
		still_motion = 1;
        current_still = 1; 
    }


    ginfo.grab_motion[ginfo_ptr]  = grab_motion; 
    ginfo.letgo_motion[ginfo_ptr] = letgo_motion; 
    ginfo.still[ginfo_ptr]        = still_motion; 
    ginfo.tap_forward[ginfo_ptr] = tap_forward_motion; 
    //ginfo.tap_reverse[ginfo_ptr] = tap_reverse_motion; 
    ginfo.tap_detected[ginfo_ptr] = 0; 

// secondary gesture: 
// grab, letgo, tap 
	int count_grab = 0, count_letgo = 0, count_tapout = 0, taps_found = 0;
	for (i = 0; i < GESTURE_LOOKBACK; i++) {
		int p = ginfo_ptr - i;
		p = (p + 89) % 90;
		if (ginfo.grab_motion[p] == 1)
			count_grab++;
		if (ginfo.letgo_motion[p] == 1)
			count_letgo++;
        if (ginfo.tap_forward[p] == 1) 
            count_tapout++;
        if (ginfo.tap_detected[p] == 1)
            taps_found++;
	}




	if (current_still && count_tapout > 2 && taps_found == 0){
        ginfo.current_gesture = GESTURE_TAP; 
        ginfo.tap_detected[ginfo.bufptr] = 1; 
        block_out_tap_detected(20);
    } else{
        ginfo.tap_detected[ginfo.bufptr] = 0;
    }

	if (current_still && count_grab > GESTURE_COUNT_MIN){
		ginfo.current_gesture = GESTURE_GRAB;
    }
    else if (count_letgo > GESTURE_COUNT_MIN){
        ginfo.current_gesture = GESTURE_LETGO; 
    }


    if(GESTURE_DEBUG)
    	printf("gesture is ------------ %4d -- %d\n", ginfo.bufptr,  ginfo.current_gesture);
    if (GESTURE_TAP == ginfo.current_gesture){
        printf("________T___A___P__________\n");
    }

	ginfo.gesture_history[ginfo.bufptr] = ginfo.current_gesture;

}


void gesture_inc_bufptr()
{
	ginfo.bufptr++;
	if (ginfo.bufptr > (BUFFER_MAX_INDEX - 1))
		ginfo.bufptr = 0;
}
int ginfo_bufptr(){
return ginfo.bufptr; 
}

int ginfo_prev_bufptr(){
    return (ginfo.bufptr + 89) % 90; 

}

void ginfo_reset_tap(int backwards){
int i = 0;  
int p = ginfo.bufptr; 
for (; i < backwards; i++){
  p = (p + 89) % 90;  
  ginfo.tap_detected[p] = 0; 
}
}

void block_out_tap_detected(int frames){
    int i; 
    for (i = 0 ; i < frames; i++){
        int ptr =( ginfo.bufptr + i + 90) % 90;
        ginfo.tap_detected[ptr] = 1;
    } 
}
