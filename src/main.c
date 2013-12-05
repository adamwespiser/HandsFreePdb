/*
 * This file is part of the OpenKinect Project. http://www.openkinect.org
 *
 * Copyright (c) 2010 individual OpenKinect contributors. See the CONTRIB file
 * for details.
 *
 * This code is licensed to you under the terms of the Apache License, version
 * 2.0, or, at your option, the terms of the GNU General Public License,
 * version 2.0. See the APACHE20 and GPL2 files for the text of the licenses,
 * or the following URLs:
 * http://www.apache.org/licenses/LICENSE-2.0
 * http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * If you redistribute this file in source form, modified or unmodified, you
 * may:
 *   1) Leave this header intact and distribute it under the same terms,
 *      accompanying it with the APACHE20 and GPL20 files, or
 *   2) Delete the Apache 2.0 clause and accompany it with the GPL2 file, or
 *   3) Delete the GPL v2 clause and accompany it with the APACHE20 file
 * In all cases you must keep the copyright notice intact and include a copy
 * of the CONTRIB file.
 *
 * Binary distributions must follow the binary distribution requirements of
 * either License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "libfreenect.h"

#include <pthread.h>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#include <math.h>
#include <errno.h>
#include "tracking.h"
#include "control.h"
#include "control_mode.h"
#include "gesture.h"
#include "kinect_vars.h"
#include "pdblib.h"
#include "getopt.h"

void *molecular_threadfunc_mol(void *arg);
void *gl_threadfunc(void *arg);
void *freenect_threadfunc(void *arg);

handtrack_t tinfo;
log_info linfo;
control_struct cinfo;
gesture_struct ginfo;
mol_thread_struct molinfo;

#define ONE 1

//struct for getopt_long
static struct option long_options[] = {
	{"firstChain", required_argument, NULL, 'f'},
	{"secondChain", required_argument, NULL, 's'},
	{"pdbFile", required_argument, NULL, 'p'},
	{"kinectDevice", required_argument, NULL, 'k'},
	{"verbose", no_argument, NULL, 'v'},
	{"log", required_argument, NULL, 'l'},
	{NULL, 0, NULL, 0}
};



void usage(void)
{

	fprintf(stderr,
		"Project Kineport: A protein visualization interface for Xbox Kinect\n\n\
USAGE: kineport [ --pdbFile ] [--firstChain] [--secondChain]\n\n\
EXAMPLE: kineprot --pdbFile ./pdbFiles/1brsD.pdb1 --firstChain A --secondChain D\n");
}

int main(int argc, char **argv)
{


    int has_one_chain = 0;
    int has_two_chain = 0;
    int has_pdb_file = 0;
	char ch;
    char *cmd_log_file = NULL;
	int user_device_number = 0;
	FILE *file;
	while ((ch =
		getopt_long_only(argc, argv, "fspk:v:l:", long_options,
			    NULL)) != -1) {
		switch (ch) {
		case 'f':
			molinfo.chain_one = *(optarg);
			//molinfo.chain_one = optarg[0];
			//molinfo.chain_one = optarg;
           // molinfo.chain_one = *(argv[optind]);
            has_one_chain = 1;
			break;
			// short option 'a'
		case 's':
            //printf("optarg for -s:%s argv[optind]: %s\n",optarg,argv[optind]);
			molinfo.chain_two = *(optarg);
			//molinfo.chain_two = optarg[0];
			//molinfo.chain_two = optarg;
            has_two_chain = 1;
            //exit(1);
			break;

		case 'p':
			molinfo.pdb_file = optarg;
            has_pdb_file = 1;
			break;
		case 'k':
			user_device_number = atoi(optarg);
			break;
        //note -> once logging is set, nothing else happens...
        case 'l':
            linfo.cmd_flag = 1;
            cmd_log_file = optarg;
            printf("log file is: %s\n",cmd_log_file);
            exit(1);
            break;
		default:
			usage();
		}
	}
	printf("File=%s firstChain=%c secondChain=%c\n", molinfo.pdb_file, molinfo.chain_one,
	       molinfo.chain_two);

    if (!(has_one_chain && has_two_chain && has_pdb_file)){
        fprintf(stderr, "ERROR: please specify both first and second chain as well as pdbFile. Thank you.\n\n");
        usage();
        exit(1);
    }
	if ((file = fopen(molinfo.pdb_file, "r")) == NULL) {
		fprintf(stderr,
			"ERROR: pdb file cannot be opened or doesn't exist\n");
		usage();
		exit(1);
	} else {
		fclose(file);
	}
	subbuffer  = malloc(640 * 480 * 3 * sizeof(float));
	subbuffer2 = malloc(640 * 480 * 3 * sizeof(float));
	subbuffer3 = malloc(640 * 480 * 3 * sizeof(float));
	subbuffer4 = malloc(640 * 480 * 3 * sizeof(float));
	subbuffer5 = malloc(640 * 480 * 3 * sizeof(int));

	init_cinfo_values();
    
    if (linfo.cmd_flag && cmd_log_file){
        printf("log file cmd flag is set\nlog file=%s",cmd_log_file);
        linfo.file_out = cmd_log_file;
    }
    else{
	    linfo.file_out = LOGGING_FILE;
    }
	linfo.fp = fopen(linfo.file_out, "w");

	if (!linfo.fp) {
		fprintf(stderr, "cannot write to %s\nPlease specify a file with --log=[ log file]\n", linfo.file_out);
        usage();
		exit(1);
	};
	if (LOG_TRANSFORM_TO_FILE) {
		fprintf(linfo.fp,
			"h1x_pos,h1y_pos,h2z_pos,h1x_out,h1y_out,h2z_out\n");
	}
	if (LOG_TRACKING_TO_FILE) {
		fprintf(linfo.fp,
			"h1x_velocity,h1y_velocity,h1z_velocity,h2x_velocity,h2y_velocity,h2z_velocity,center_x,center_y,center_z,center_x_velocity,center_y_velocity,center_z_velocity,h1x_position,h1y_position,h1z_position,h2x_position,h2y_position,h2z_position,hand1_to_center,hand2_to_center,h1x_cg,h1y_cg,h1z_cg,h2x_cg,h2y_cg,h2z_cg,hand_distance,time\n");
	}
	int res;
	tinfo.hand1_has_tracking = 0;
	tinfo.hand2_has_tracking = 0;
	depth_mid = malloc(640 * 480 * 3);
	depth_front = malloc(640 * 480 * 3);
	rgb_back = malloc(640 * 480 * 3);
	rgb_mid = malloc(640 * 480 * 3);
	rgb_front = malloc(640 * 480 * 3);

	printf("Kinect hand tracking test\n");

	int i;
	for (i = 0; i < 2048; i++) {
		float v = i / 2048.0;
		v = powf(v, 3) * 6;
		t_gamma[i] = v * 6 * 256;
	}

//      track1_pos_x=200; track1_pos_y=240;
//      track2_pos_x=400; track2_pos_y=240;

	g_argc = argc;
	g_argv = argv;

	die = 0;

//// this is a macro which expands to something??
//   they both expand to mutex's and conditional variables, as perscribed by pthreads.h

	//pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
	//pthread_cond_t gl_frame_cond = PTHREAD_COND_INITIALIZER;

	requested_format = FREENECT_VIDEO_RGB;
	current_format = FREENECT_VIDEO_RGB;
	got_rgb = 0;
	got_depth = 0;
	freenect_angle = 0;

	// f_ctx is a freenect_context struct as declared in kinect_vars
	if (freenect_init(&f_ctx, NULL) < 0) {
		printf("freenect_init() failed\n");
		return 1;
	}

	freenect_set_log_level(f_ctx, FREENECT_LOG_DEBUG);

	int nr_devices = freenect_num_devices(f_ctx);
	printf("Number of devices found: %d\n", nr_devices);

// AW: appearantly the first argument of for the glview project is the device number...
	//if (argc > 1)
	//      user_device_number = atoi(argv[1]);
	//user_device_number = atoi(0);

	if (nr_devices < 1)
		return 1;

	if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
		printf("Could not open device\n");
		return 1;
	}

	XInitThreads();

	res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL);
	if (res) {
		printf("pthread_create failed\n");
		return 1;
	}

	glutInit(&argc, argv);

	res =
	    pthread_create(&molecular_thread_mol, NULL,
			   molecular_threadfunc_mol, &molinfo);
	if (res) {
		printf("pthread_create failed\n");
		return 1;
	}
	// OS X requires GLUT to run on the main thread
	gl_threadfunc(NULL);

	return 0;
}
