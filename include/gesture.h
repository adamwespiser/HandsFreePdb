#include "constants.h"

void gesture_recognition(void);
void ginfo_reset_tap(int backwards); 
void gesture_rec_for_grab_and_letgo_newTAP(void);
void gesture_inc_bufptr(void);
void block_out_tap_detected(int frames);

int ginfo_prev_bufptr(void); 
int ginfo_bufptr(void); 

typedef struct gi {
	int grab_motion[BUFFER_SIZE];
	int letgo_motion[BUFFER_SIZE];
	int still[BUFFER_SIZE];

    int tap_forward[BUFFER_SIZE];
    int tap_reverse[BUFFER_SIZE];
    int tap_detected[BUFFER_SIZE]; 
	int gesture_history[BUFFER_SIZE];

	int bufptr;
	int current_gesture;

} gesture_struct;
