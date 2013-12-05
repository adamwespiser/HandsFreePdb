#include "constants.h"
#define CTRLMODE_FREE 0
#define CTRLMODE_GRAB 1

//chain modes 
#define CHAIN_ONE 0 
#define CHAIN_TWO 1 
#define ALL_CHAINS 2 
//#define MOVE_ALL 2

typedef struct ci {
	int current_mode;

    int chain_mode; 

	float hand1_x_out;
	float hand1_y_out;
	float hand1_z_out;

	float hand2_x_out;
	float hand2_y_out;
	float hand2_z_out;


    float hand1_vel; 
    float hand2_vel; 

    float center_vx;
    float center_vy;
    float center_vz;

    float total_theta; 
    float total_xcg;
    float total_ycg;
    float total_zcg;

    float cg_vx;
    float cg_vy;
    float cg_vz;

    float cg_ax;
    float cg_ay;
    float cg_az;

    float omega;
    float ux; 
    float uy; 
    float uz; 

    float slowdown;

} control_struct;

void online_transform_for_molview(void);
void online_transform_for_molview_new(void);
void update_cinfo_w_tinfo(void);
void init_cinfo_values(void);
int hands_moving_on_center_line();
//void scale_input_for_molview(void); 
