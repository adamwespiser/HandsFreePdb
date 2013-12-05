#include "constants.h"
#include "vector3d.h"

void hand_determine_pos(float x, float y, float z, int h, uint16_t *d);
void htrack_set_current_time_points(void);
void set_handpos_xyz_from_cg(float cgx, float cgy, float cgz, int hand);
void set_buffer_to_current_hand_pos(void); 
void report_xyz_for_two_hands(void);

vector3d get_hand_rotation_crossproduct(); 

int tinfo_curr_bufptr(void);
int tinfo_prev_bufptr(void);
int bufptr_minus_one(int ptr);
int check_for_hand_conv(void);

float raw_depth_to_meters(int depth_value);


typedef struct ht_t {

    int trackingstatus; // current status of the capture/tracking
    int trackingstatustime; // timestamp for temporal events
    int trackingframecounter; // kinect frame counter from program start    

	vector3d hand1;		//output for tracking algo
	vector3d hand2;		//output for tracking algo 

// current pos in rw coords (meters) and pix - set by set_handpos_xyz_from_cg 
// immediate output from tracking routine, BEFORE smoothing
	vector3d handpos1;
	vector3d handpos2;
    vector3d handpos1_pix;
    vector3d handpos2_pix;

//pixels 
	int handfirst1_x;
	int handfirst1_y;
    int handfirst1_z;
	int depthfirst1;

	int handfirst2_x;
	int handfirst2_y;
    int handfirst2_z;
	int depthfirst2;

    int hand1_cgx;
    int hand1_cgy;
    int hand2_cgx;
    int hand2_cgy;
    

	int has_tracking;	// 1 if tracking info is valid

	int hand1_has_tracking;
	int hand2_has_tracking;

// based off center of gravity of hand position 
	vector3d hand1_cg[BUFFER_SIZE];
	vector3d hand2_cg[BUFFER_SIZE];

// represents current.frame(x,y,z) - old.frame(x,y,z) 
//hand {1,2} velocity
	vector3d hand1_velocity[BUFFER_SIZE];
	vector3d hand2_velocity[BUFFER_SIZE];

// smoothed values for hand positions  
	vector3d hand1_smooth[BUFFER_SIZE];
	vector3d hand2_smooth[BUFFER_SIZE];

// center of two hands
	vector3d hand_center_pos[BUFFER_SIZE];

// center of two hands velocity
	vector3d hand_center_velocity[BUFFER_SIZE];

	float hand_distance[BUFFER_SIZE];

	float hand1_to_center_velocity[BUFFER_SIZE];
	float hand2_to_center_velocity[BUFFER_SIZE];

	float hand1_vel_mag[BUFFER_SIZE];
	float hand2_vel_mag[BUFFER_SIZE];

	int bufptr;

//scaled output for molview
	float hand1_x_out;
	float hand1_y_out;
	float hand1_z_out;

	float hand2_x_out;
	float hand2_y_out;
	float hand2_z_out;

	int time_point;

	int hand_just_reset;

// array for hand tracking

    char tracking_mask[640*480];

} handtrack_t;


// struct for printing out data
// init'd in main, passed on from there to gestures and tracking.c
typedef struct li {
	FILE *fp;
	char *file_out;
	char status;
    int cmd_flag;
    //char *cmd_line_file;
} log_info;
