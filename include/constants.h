#define BUFFER_SIZE 100
#define BUFFER_MAX_INDEX 90	//only indexes below this number
#define ONE 1
#define HAND_ONE 1
#define HAND_TWO 2

// kinect_window.c 
#define DEPTH_COLORING 0
#define H_ALGO 0
#define SUBSAMP 8 
#define MIRROR 1
#define BGDELTA 0.06
#define MOTION_DETECTION_THRESHOLD 40 //100
#define DISPLAY_SEGMENT 1
#define MIN_HAND_AREA_CUTOFF 1000

// see tracking.c:linfo
#define LOG_TRACKING_TO_FILE 1
#define LOG_TRANSFORM_TO_FILE 0
// #define LOGGING_FILE "/home/adam/work/kinect/analysis/data/tmp_save.csv"
 #define LOGGING_FILE "tmp_save.csv"

// tracking.c 
#define SMOOTHING_COEFF 5.0
#define SMOOTHING_ITERATIONS 80
#define LOCAL_SCAN_WIDTH 60
#define LOCAL_SCAN_HEIGHT 60
#define CG_SCAN_WIDTH_DC 60
#define CG_SCAN_HEIGHT_DC 60
#define CG_SCAN_WIDTH 60
#define CG_SCAN_HEIGHT 60
#define RESET_BUFFER_ON_INIT 1	//set center vel to [0,0,0] if hand just found
#define HAND_RANGE_MIN 500
#define HAND_RANGE_MAX 800
#define HAND_EXCLUSION_SET 1
#define HAND_EX_RADIUS 40

// tracking status
#define TRK_NO_TRACKING 0
#define TRK_WAIT_FOR_QUIET 1
#define TRK_ACQUIRING 2  
#define TRK_TRACKING_OK 3
#define TRK_TEST_AW 0 // 1 -> data from hand tracking by motion 0->nothing


//gesture.c 
// previous values

#define GESTURE_NONE 0
#define GESTURE_GRAB 1
#define GESTURE_LETGO 2
#define GESTURE_STILL 3
#define GESTURE_TAP 4
#define GESTURE_DEBUG 0

#define GRAB_TIME 7		//grab time in frames
#define GRAB_DIST 0.10		// distance threshold in m

#define GRAB_VELOCITY 0.20
#define LETGO_VELOCITY 0.25
#define STILL_VELOCITY 0.08	//both hands must be below in order for "still" gesture to be recognized

#define GESTURE_LOOKBACK 15	// 7 # of frame to look back for gestures
#define GESTURE_COUNT_MIN 2	//requires this many gestures in last GESTURE_LOOKBACK frames to set the gesture state

// "tap" gesture 
#define TAP_LOOKBACK 30
#define TAP_VELOCITY_F -0.30 //
#define TAP_VELOCITY_R 0.20 //
#define TAP_COUNTS_F 1  //symmetric
#define TAP_COUNTS_R 1  //symmetric

// tap gesture from R prototyping
#define LOOKBACK_tap 12
#define STILL_THRESHOLD_tap 0.05
#define MOVEMENT_VEL_tap  0.1
#define Y_VELOCITY_FORWARD_tap 0.05
#define Y_VELOCITY_REVERSE_tap 0.03
#define Z_VELOCITY_FORWARD_tap -0.01
#define Z_VELOCITY_REVERSE_tap 0.0
#define YZ_COUNT_FORWARD_tap 1
#define Y_COUNT_FORWARD_tap 2// count if h.y > _
#define Y_COUNT_REVERSE_tap 4 // count if h.y <  _
#define Z_COUNT_FORWARD_tap 1
#define Z_COUNT_REVERSE_tap 3
#define HAND1_STILL_COUNT_tap 1
#define HAND2_STILL_COUNT_tap 1
#define HAND1_STILL_THRESHOLD_tap 0.05
#define HAND2_STILL_THRESHOLD_tap 0.0015

//control_mode.c 

#define MAX_HAND_VELOCITY 0.35	//any motions faster than this won't be tracked
#define LIMIT_MAX_HAND_VELOCITY 1	//1 if limit max hand velocity, 0 otherwise

#define X_INIT_VALUE 0.0
#define Y_INIT_VALUE 0.0
#define Z_INIT_VALUE 0.7

#define X_SCALING_FACTOR  56.0
#define Y_SCALING_FACTOR  56.0
#define Z_SCALING_FACTOR  -0.13


//molview.c
#define NUMBER_OF_ATOMS 30
#define Z_VIEW_CORRECT 130
#define MAX_HAND_SPEED 0.35
#define ML_HIDE_MOL_WINDOW 0 //1 -> hide, 0 -> show....
#define PDB_MAX_ATOMS 400

    //translatate mol 
#define INSTANT_FACTOR 0.005
#define VCTRL 1.
#define ACC 4 //AW was 3
#define FRICTION 0.005
#define STATIC_THRESHOLD 0.002 // 0.0075 AW version with deadband
    //calc_rotation 
#define DEG_TO_RAD_FACTOR 0.0173542
#define ROTATIONAL_SCALING_FACTOR 0.5 //0.2 KZ
#define ACC_ROT 4
#define INSTANT_FACTOR_ROT 1 //0.005 KZ
#define COEFF_ROT_FRICTION 0
#define INSTANT_FACTOR_U 1
#define ACC_U 1

//1brs.pdb1 setting for molview 
#define MOVEMENT_ATOM_START 108
#define MOVEMENT_ATOM_END 195
#define MOVE_ALL_ATOMS 2


// segmenter.c 
#define SEG_PRINT_SEG_INDEX 0
