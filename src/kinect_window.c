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

#include "control.h"
#include "control_mode.h"
#include "tracking.h"
#include "gesture.h"
#include "kinect_vars.h"
#include "segmenter.h"

// include </usr/include/glib-2.0/glib.h>

extern handtrack_t tinfo;

#define LOG_STATUS 1

void draw_square(int xPos, int yPos, int len);
void draw_square_color(int xPos, int yPos, int len, int r, int g, int b,
		       int mirror);
void draw_center_line(int width, int r, int g, int b);

#define FREENECT_FRAME_PIX (640)*480

int within_range(int val, int low, int high)
{
	return (val > low && val < high);
}

static void
make_window_for_kinect(Display * dpy, const char *name,
		       int x, int y, int width, int height,
		       Window * winRet, GLXContext * ctxRet)
{
	int attrib[] = { GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		None
	};
	int scrnum;
	XSetWindowAttributes attr;
	unsigned long mask;
	Window root;
	Window win;
	GLXContext ctx;
	XVisualInfo *visinfo;

	scrnum = DefaultScreen(dpy);
	root = RootWindow(dpy, scrnum);

	visinfo = glXChooseVisual(dpy, scrnum, attrib);
	if (!visinfo) {
		printf("Error: couldn't get an RGB, Double-buffered visual\n");
		exit(1);
	}

	/* window attributes */
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	win = XCreateWindow(dpy, root, 0, 0, width, height,
			    0, visinfo->depth, InputOutput,
			    visinfo->visual, mask, &attr);

	/* set hints and properties */
	{
		XSizeHints sizehints;
		sizehints.x = x;
		sizehints.y = y;
		sizehints.width = width;
		sizehints.height = height;
		sizehints.flags = USSize | USPosition;
		XSetNormalHints(dpy, win, &sizehints);
		XSetStandardProperties(dpy, win, name, name,
				       None, (char **)NULL, 0, &sizehints);
	}

	ctx = glXCreateContext(dpy, visinfo, NULL, True);
	if (!ctx) {
		printf("Error: glXCreateContext failed\n");
		exit(1);
	}

	XFree(visinfo);

	*winRet = win;
	*ctxRet = ctx;
}

void DrawGLScene()
{
	pthread_mutex_lock(&gl_backbuf_mutex);

	while (!got_depth || !got_rgb) {
		pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
	}

	if (requested_format != current_format) {
		pthread_mutex_unlock(&gl_backbuf_mutex);
		return;
	}

	void *tmp;

	if (got_depth) {
		tmp = depth_front;
		depth_front = depth_mid;
		depth_mid = tmp;
		got_depth = 0;

		//tracking_handpos_paint( depth_front );

	}
	if (got_rgb) {
		tmp = rgb_front;
		rgb_front = rgb_mid;
		rgb_mid = tmp;
		got_rgb = 0;
	}

	pthread_mutex_unlock(&gl_backbuf_mutex);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE,
		     depth_front);

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(255.0f, 255.0f, 255.0f, 255.0f);
	glTexCoord2f(0, 0);
	glVertex3f(0, 0, 0);
	glTexCoord2f(1, 0);
	glVertex3f(640, 0, 0);
	glTexCoord2f(1, 1);
	glVertex3f(640, 480, 0);
	glTexCoord2f(0, 1);
	glVertex3f(0, 480, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
	if (current_format == FREENECT_VIDEO_RGB)
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB,
			     GL_UNSIGNED_BYTE, rgb_front);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, 1, 640, 480, 0, GL_LUMINANCE,
			     GL_UNSIGNED_BYTE, rgb_front + 640 * 4);

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(255.0f, 255.0f, 255.0f, 255.0f);
	glTexCoord2f(0, 0);
	glVertex3f(640, 0, 0);
	glTexCoord2f(1, 0);
	glVertex3f(1280, 0, 0);
	glTexCoord2f(1, 1);
	glVertex3f(1280, 480, 0);
	glTexCoord2f(0, 1);
	glVertex3f(640, 480, 0);
	glEnd();

// KZ: now done in the event loop
//      glutSwapBuffers();  
}

void ReSizeGLScene(int Width, int Height)
{
	glViewport(0, 0, Width, Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1280, 480, 0, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
}

void InitGL(int Width, int Height)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);
	glGenTextures(1, &gl_depth_tex);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenTextures(1, &gl_rgb_tex);
	glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	ReSizeGLScene(Width, Height);
}

static void event_loop_kindraw(Display * dpy, Window win)
{
	while (1) {

		while (XPending(dpy) > 0) {
			XEvent event;
			XNextEvent(dpy, &event);

			switch (event.type) {
/*       
         case MotionNotify:
         {
         needredraw =1;
         break;
         }
*/

/*
	 case ButtonPress:
	 {
	 if (event.xbutton.button==1) { track1_pos_x = event.xmotion.x; track1_pos_y = event.xmotion.y; }
         if (event.xbutton.button==2) { track2_pos_x = event.xmotion.x; track2_pos_y = event.xmotion.y; }
	 break;
	 }
*/

			case Expose:
				{	/* we'll redraw below */
					break;
				}
			case ConfigureNotify:
				{
					ReSizeGLScene(event.xconfigure.width,
						      event.xconfigure.height);
					break;
				}

			case KeyPress:
				{
					char key, buffer[100];
					//int r =XLookupString(&event.xkey, buffer,sizeof(buffer), NULL,NULL);
					key = buffer[0];
					if (key == 27) {
						die = 1;
						pthread_join(freenect_thread,
							     NULL);
						glutDestroyWindow(window);
						free(depth_mid);
						free(depth_front);
						free(rgb_back);
						free(rgb_mid);
						free(rgb_front);
						pthread_exit(NULL);
						return;
					}

					if (key == 'w') {
						freenect_angle++;
						if (freenect_angle > 30)
							freenect_angle = 30;
					}
					if (key == 's') {
						freenect_angle = 0;
					}
					if (key == 'x') {
						freenect_angle--;
						if (freenect_angle < -30)
							freenect_angle = -30;
					}
					freenect_set_tilt_degs(f_dev,
							       freenect_angle);


//                    if (key=='y') freenect_set_led(f_dev, LED_YELLOW);
//                    if (key=='u') freenect_set_led(f_dev, LED_GREEN);


					if (key == 'f') {
						if (requested_format ==
						    FREENECT_VIDEO_IR_8BIT)
							requested_format =
							    FREENECT_VIDEO_RGB;
						else
							requested_format =
							    FREENECT_VIDEO_IR_8BIT;
					}
				}	// end case keypress

			}	// end switch event.type
		}		//end while XPending(dpy)

		// always redraw kinect views
		DrawGLScene();
		glXSwapBuffers(dpy, win);

	}			//end while(1)
}				// end event_loop_for_kinect;

void *gl_threadfunc(void *arg)
{
	Display *dpy;
	Window win;
	GLXContext ctx;
	char *dpyName = NULL;
	//GLboolean printInfo = GL_FALSE;
	//int i;
	//int screen;
	//long fg, bg;
	//XFontStruct *font_info;

	printf("starting gl thread\n");
	fflush(stdout);

// only one glutInit, in main
// glutInit(&g_argc, g_argv);

	dpy = XOpenDisplay(dpyName);
	if (!dpy) {
		printf("Error: couldn't open display %s\n", dpyName);

		//return -1;

		return NULL;
	}

	make_window_for_kinect(dpy, "libfreenct", 0, 0, 1280, 480, &win, &ctx);

	XMapWindow(dpy, win);
	glXMakeCurrent(dpy, win, ctx);
//        reshape_for_kinect(1280, 640);

/*
       glutInitWindowSize(1280, 480);
        glutInitWindowPosition(0, 0);
        

       glutDisplayFunc(&DrawGLScene);
       glutIdleFunc(&DrawGLScene);
       glutReshapeFunc(&ReSizeGLScene);
       glutKeyboardFunc(&keyPressed);
*/

	XSelectInput(dpy, win,
		     KeyPressMask | ButtonPressMask | PointerMotionMask);

	InitGL(1280, 480);

//glutMainLoop();
	event_loop_kindraw(dpy, win);

	printf("kinect window main loop ended\n");

	glXDestroyContext(dpy, ctx);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return NULL;
}

void depth_cb(freenect_device * dev, void *v_depth, uint32_t timestamp)
{
	int i;
	uint16_t *depth = v_depth;
	int led_r, led_g, led_b;

	pthread_mutex_lock(&gl_backbuf_mutex);

	tinfo.trackingframecounter++;

	//histogram depth distribution
	float *depthcoarse = subbuffer;
	float *background = subbuffer2;
	float *motionimg = subbuffer3;
	float *maskimg = subbuffer4;
	int *segimg = subbuffer5;

    //start of motion based tracking 
//vars are determined by motion tracking, then sent to physical tracking to seed
//tracking
double h1cgx, h1cgy, h1cgz,h2cgx, h2cgy, h2cgz = 0;
    int nsegs = 0;
	int areahand1 = 0;
    int areahand2 = 0;
	int hashand1 = 0;
	int hashand2 = 0;
	int maskarea = 0;
    int hand1cut = 0;
    int hand2cut = 0;
// motion based tracking to acquire intial points
if (tinfo.trackingstatus != TRK_TRACKING_OK ) {

	memset(depthcoarse, 0, 640 * 480 * 3);

	//set depthcoarse
	//essentially a mapping of idx1(dpeth) to idx2(depthcoarse) based on the size of the sub sampling variable SUBSAMP
	for (i = 0; i < 480; i++) {
		int j;
		for (j = 0; j < 640; j++) {

			int ii = i / SUBSAMP;
			int jj = j / SUBSAMP;
			int idx1 = i * 640 + j;
			int idx2 = ii * 640 / SUBSAMP + jj;

			depthcoarse[idx2] += depth[idx1];	//+ (1-LTIME) * background2[idx2];

	}}

	//normalize depthcourse values
	//by the square SUBSAMP value
	for (i = 0; i < 480 / SUBSAMP; i++) {
		int j;
		for (j = 0; j < 640 / SUBSAMP; j++) {
			int idx2 = i * 640 / SUBSAMP + j;
			depthcoarse[idx2] /= (SUBSAMP * SUBSAMP);
	}}

	int depthhgram[4096];	//represents the range of possible depth values in the depth array
// 0 being closet, 4096 furthest away
// q: what is the largest practically observed depth value?
	int hgramtotal = 0;
	int minvalue = 100000;

	for (i = 0; i < 4096; i++)
		depthhgram[i] = 0;

	// determine the depth distribution, where hgramtotal represents the sum of depth values
	for (i = 0; i < 480 / SUBSAMP; i++) {
		int j;
		for (j = 0; j < 640 / SUBSAMP; j++) {
			int idx2 = i * 640 / SUBSAMP + j;
			if (depthcoarse[idx2] < minvalue)
				minvalue = depthcoarse[idx2];
			if (depthcoarse[idx2] < 4095) {
				depthhgram[(int)(depthcoarse[idx2])]++;
				hgramtotal++;
			};
	}}

	minvalue = 350;

	int cumul = 0;
	int hgramcutoff = 0;
//over all possible depth values, determine the cumulative depth found(cumul), if its
//greater than FRAC * hgramtotal, mark the hgramcutoff and break
	for (i = 0; i < 4096; i++) {
		cumul += depthhgram[i];
		if (cumul > 0.2 * hgramtotal) {
			hgramcutoff = i;
			break;
		}
	}

	//foreach pixel in the depthcoarse array, if value is less then hgramcutoff set to hgramcutoff, else set to 0
	//adjustment: keep all depthcoarse array values less than the histogram cutoff, all else set to 0
	for (i = 0; i < 480 / SUBSAMP; i++) {
		int j;
		for (j = 0; j < 640 / SUBSAMP; j++) {
			int idx2 = i * 640 / SUBSAMP + j;
			if (depthcoarse[idx2] < hgramcutoff) {
				;;	//depthcoarse[idx2] = hgramcutoff;
			} else {
				depthcoarse[idx2] = 0;
			};
		}
	}

// motion detection
// background is an accumlator array
// motionimg is the difference between the current frame(depthcoarse) and the accumulator array(background) 
// if that difference is greater than MOTOIN_DETECTION_THRESHOLD, maskimg[current index] := 1, otherwise := 0
	for (i = 0; i < 480 / SUBSAMP; i++) {
		int j;
		for (j = 0; j < 640 / SUBSAMP; j++) {
			int idx2 = i * 640 / SUBSAMP + j;
			background[idx2] =
			    background[idx2] * (1 - BGDELTA) +
			    BGDELTA * depthcoarse[idx2];
			motionimg[idx2] =
			    fabs(depthcoarse[idx2] - background[idx2]);

			//motion detection threshold
			if (depthcoarse[idx2] > 0
			    && motionimg[idx2] > MOTION_DETECTION_THRESHOLD)
				maskimg[idx2] = 1;
			else
				maskimg[idx2] = 0;

		}
	}

//mask isolated pixels, (noise)
	// for i,j in depthcoarse
	for (i = 0; i < 480 / SUBSAMP; i++) {
		int j;
		for (j = 0; j < 640 / SUBSAMP; j++) {
			int idx2 = i * 640 / SUBSAMP + j;

			int ii, jj;
			int neighbors = 0;

			// for pixel neighbours (ii,jj)
			for (ii = -1; ii < 2; ii++) {
				for (jj = -1; jj < 2; jj++) {

					//protect against three cases: 1) ii=jj=0(original pixel) 2) ii || jj is a value beyond the end of the array(out of bounds error will segfault)
					if (ii == 0 && jj == 0)
						continue;

					if (i + ii < 0
					    || i + ii >= 480 / SUBSAMP)
						continue;
					if (j + jj < 0
					    || j + jj >= 640 / SUBSAMP)
						continue;

					int idx3 =
					    (i + ii) * 640 / SUBSAMP + j + jj;

					if (maskimg[idx3] > 0)
						neighbors++;

				}
			}

			//set maskimg to the number of neighbours w/ value > 0 found
			if (maskimg[idx2] > 0)
				maskimg[idx2] = neighbors + 1;

		}
	}

	// go through maskimg, take all points with less than 7 neighbours and set them to 0 
	// in both the maskimg and the segimg arrays
	for (i = 0; i < 480 / SUBSAMP; i++) {
		int j;
		for (j = 0; j < 640 / SUBSAMP; j++) {
			int idx2 = i * 640 / SUBSAMP + j;
			if (maskimg[idx2] < 7) {
				maskimg[idx2] = 0;
				segimg[idx2] = 0;
			} else {
				maskimg[idx2] = 1;
				segimg[idx2] = 1;
				maskarea++;
			}
		}
	}
	//printf("%s","this far\n");
	//segment the segimg, where values of 1 correspond to motion detection.
	// assigns segment number to each of the arrays of motions.
	nsegs = segmenter(segimg, SUBSAMP);

	//find the largest two moving segments, delete others
	findlargestsegs(segimg, SUBSAMP, nsegs);

	//if a pixel is neighbour to a segment, it is now part of that segments
	// repeat process
	expandmask(segimg, SUBSAMP);
	expandmask(segimg, SUBSAMP);

	//determine the depth value cutoff for each hand
	hand1cut = findhgramcutoff(segimg, depth, SUBSAMP, 1);
	 hand2cut = findhgramcutoff(segimg, depth, SUBSAMP, 2);

//check for presence of hands
	//float h1cgx, h1cgy, h1cgz;
	//float h2cgx, h2cgy, h2cgz;
    // for all pixels -> map idx1 for full array, and idx2 for subsamp'd array
    // determine the center of gravity(x,y,z) for hand 1 and 2:
    //    (x,y,z) counts towards mean iff segimg == (1 || 2) and depth[index] < cutoff
	for (i = 0; i < 480; i++) {
		int j;
		for (j = 0; j < 640; j++) {

			int ii = i / SUBSAMP;
			int jj = j / SUBSAMP;
			int idx1 = i * 640 + j;
			int idx2 = ii * 640 / SUBSAMP + jj;
			if (depth[idx1] < hand1cut && segimg[idx2] == 1) {
				areahand1++;
				h1cgx += j;
				h1cgy += i;
				h1cgz += depth[idx1];
			}
			if (depth[idx1] < hand2cut && segimg[idx2] == 2) {
				areahand2++;
				h2cgx += j;
				h2cgy += i;
				h2cgz += depth[idx1];
			}

		}
	}

        if(TRK_TEST_AW){
    		printf("<pre> cg1/2(x,y,z) %.1f %.1f %.1f ---  %.1f %.1f %.1f\n",h1cgx, h1cgy, h1cgz, h2cgx, h2cgy, h2cgz);

            printf("<pre> area h1/h2 %d %d\n", areahand1, areahand2); 
        }

	if (areahand1) { //protect against illegal division
		h1cgx /= areahand1;
		h1cgy /= areahand1;
		h1cgz /= areahand1;
	}
	if (areahand2) {
		h2cgx /= areahand2;
		h2cgy /= areahand2;
		h2cgz /= areahand2;
	}

    //set cuttoffs for the min. hand size
	if (areahand1 > MIN_HAND_AREA_CUTOFF)
		hashand1 = 1;
	if (areahand2 > MIN_HAND_AREA_CUTOFF)
		hashand2 = 1;

} //not exectuted if TRK_TRACKING_OK == 1


//tracking status logic:
// TRK_NO_TRACKING -> TRK_WAIT_FOR_QUIET -> TRK_ACQUIRED -> TRK_TRACKING_OK
//      ^                                        |
//      L-----------------------------------------
//

	if (tinfo.trackingstatus == TRK_NO_TRACKING) {
//wait 0.5 s then switch to waiting

		tinfo.hand1_has_tracking = 0;
		tinfo.hand2_has_tracking = 0;

		int dt = tinfo.trackingframecounter - tinfo.trackingstatustime;
		if (dt > 30) {
			tinfo.trackingstatus = TRK_WAIT_FOR_QUIET;
			tinfo.trackingstatustime = tinfo.trackingframecounter;
		}
	}

	if (tinfo.trackingstatus == TRK_WAIT_FOR_QUIET) {
		if (maskarea < 2) {
			tinfo.trackingstatus = TRK_ACQUIRING;
			tinfo.trackingstatustime = tinfo.trackingframecounter;
		}
	}



	if (tinfo.trackingstatus == TRK_ACQUIRING) {
		// if not ack'ed within 5 sec, go back to no tracking
		int dt = tinfo.trackingframecounter - tinfo.trackingstatustime;
		if (dt > 5 * 30) {
			tinfo.trackingstatus = TRK_NO_TRACKING;
			tinfo.trackingstatustime = tinfo.trackingframecounter;
		}
		if (hashand1 && hashand2 && nsegs > 1) {
			tinfo.trackingstatus = TRK_TRACKING_OK;
			tinfo.trackingstatustime = tinfo.trackingframecounter;
		}
	}
// set status LED color
	switch (tinfo.trackingstatus) {
	case TRK_NO_TRACKING:{
			led_r = 255;
			led_g = 0;
			led_b = 0;


requested_kinect_led = LED_RED;

			break;
		}
	case TRK_WAIT_FOR_QUIET:{
			led_r = 200;
			led_g = 200;
			led_b = 0;

requested_kinect_led = LED_YELLOW;


			break;
		}
	case TRK_ACQUIRING:{
			led_r = 50;
			led_g = 50;
			led_b = 250; // blue on screen

    requested_kinect_led = LED_BLINK_GREEN;

			break;
		}
	case TRK_TRACKING_OK:{
			led_r = 0;
			led_g = 255;
			led_b = 0;

      requested_kinect_led = LED_GREEN;
        
			break;
		}
	}


    // print out tracking information
	if (tinfo.trackingstatus == TRK_TRACKING_OK) {


    memset((void*) tinfo.tracking_mask, 0, 640*480);

        if (TRK_TEST_AW){
		    printf("--- cg1/2(x,y,z) %.1f %.1f %.1f ---  %.1f %.1f %.1f\n",h1cgx, h1cgy, h1cgz, h2cgx, h2cgy, h2cgz);
            printf("area 1/2 %d %d\n", areahand1, areahand2);
        }
        //tracking.c for hand_determine_pos
		hand_determine_pos(h1cgx, h1cgy, h1cgz, HAND_ONE, depth);
		hand_determine_pos(h2cgx, h2cgy, h2cgz, HAND_TWO, depth);
	
        //KZ : check for convergence; if 

        //check_convergence(); // set trackingstatus to 

        }
    if(check_for_hand_conv() == 1){
        tinfo.trackingstatus = TRK_NO_TRACKING;
    } 


	if (tinfo.trackingstatus == TRK_TRACKING_OK)
		htrack_set_current_time_points();	// smoothing // increments tinfo

// DRAWING THE DEPTH DISPLAY
	//for values in depth array, map maskimg value, if corresponding value set to 0(depthcourse value greater than cutoff)
	//then set the depth array value to 0

	for (i = 0; i < 480; i++) {
		int j;
		for (j = 0; j < 640; j++) {

			int ii = i / SUBSAMP;
			int jj = j / SUBSAMP;
			int idx1 = i * 640 + j;
			int idx2 = ii * 640 / SUBSAMP + jj;

// baseline depth gray image
			int level = 255 - (int)(depth[idx1] / 2048. * 254.);
			level = 128 + (level - 128) * 0.5;
			int lb_r = level;
			int lb_g = level;
			int lb_b = level;

// color based on motion image
			lb_r += segimg[idx2] * 64;
			lb_g -= segimg[idx2] * 64;

			if (depth[idx1] < hand1cut && segimg[idx2] == 1) {
				lb_r = 255;
				lb_g = 255;
				lb_b = 200;
				if (hashand1) {
					lb_g = 0;
					lb_b = 0;
				}
			}
			if (depth[idx1] < hand2cut && segimg[idx2] == 2) {
				lb_r = 200;
				lb_g = 255;
				lb_b = 255;
				if (hashand2) {
					lb_r = 0;
					lb_b = 0;
				}
			}
//                if (depth[idx1]<hand1cut) depth[idx1]=0;

//  if ()       depth[idx1] = depthcoarse[idx2];
			if (DISPLAY_SEGMENT == 1) {
				//depth[idx1] = (float)segimg[idx2];
			}

			if (depth[idx1] > hand1cut)
				depth[idx1] = 0;


            if (tinfo.trackingstatus == TRK_TRACKING_OK) {


//            switch (tinfo.tracking_mask[idx1]) {
//           case 1+10: { lb_r= level;  }
//           }            

            lb_r = level + tinfo.tracking_mask[idx1]*2;
            lb_g = level - tinfo.tracking_mask[idx1]*2.5;
            lb_b = level + tinfo.tracking_mask[idx1]*3;


            }


			//if (maskimg[idx2] == 0){ depth[idx1] = 0;}

			int inverse_index = i * 640 + 639 - j;
			if (MIRROR) {
				idx1 = inverse_index;
			}
			depth_mid[3 * idx1 + 0] = lb_r;
			depth_mid[3 * idx1 + 1] = lb_g;
			depth_mid[3 * idx1 + 2] = lb_b;

		}
	}



    //end of of motion tracking;









    // draw status LED
	draw_square_color(50, 50, 20, led_r, led_g, led_b, 0);

	//htrack_find_hand_new(depth, HAND_ONE);
	//htrack_find_hand_new(depth, HAND_TWO);
//    htrack_find_hand_dc(depthcoarse, HAND_ONE, SUBSAMP);
//    htrack_find_hand_dc(depthcoarse, HAND_TWO, SUBSAMP);

//      check_for_hand_convergence();

	// add the gesture recognition stuff here... 
      gesture_recognition();

	// transform tinfo into cinfo for molview 
	online_transform_for_molview_new();

	if (LOG_STATUS == 1) {

		//report_xyz_for_two_hands();
	}

/*
	for (i = 0; i < FREENECT_FRAME_PIX; i++) {

		//convert i to x and y coords 
		int hand1y_curr = i / 640;
		int hand1x_curr = i % 640;

		// set RGB colors to black 
		int lb_r = 0, lb_g = 0, lb_b = 0;
		if (DEPTH_COLORING != 1) {

			if (depth[i] < 350) {

				lb_r = 0, lb_g = 0, lb_b = 0;

			}
			// "close" depth is red 
			if (depth[i] < 500 && depth[i] > 350) {
				lb_r = 255;
			}
			// "medium" depth is green              
			if (500 < depth[i] && depth[i] < 800) {
				lb_r = 0;
				lb_g = 255;
				lb_b = 0;
			}
			// "far" depth is dark green 

			if (800 < depth[i] && depth[i] < 900) {
				lb_r = 0;
				lb_g = 100;
				lb_b = 0;
			}
		}

		int min_value = 350;
		int max_value = hgramcutoff;

		if (DEPTH_COLORING == 1 && DISPLAY_SEGMENT == 0) {
			if (min_value < depth[i] && depth[i] < max_value) {
				int intensity =
				    255 -
				    (((depth[i] -
				       (float)min_value) / (max_value -
							    min_value)) * 255) /
				    2;
				lb_r = 0;
				lb_g = intensity;
				lb_b = 0;
			} else if (depth[i] > max_value && depth[i] < 1200) {
				int intensity =
				    255 -
				    ((depth[i] - (float)max_value) / (1200. -
								      max_value))
				    * 255;
				if (intensity < 0) {

					intensity = 0;
				}

				lb_r = intensity;
				lb_g = intensity;
				lb_b = intensity;
			}
		}
		//if (900 < depth[i] && depth[i] < 1000){lb_r=60; lb_g=60; lb_b=0;}
		//if (1000 < depth[i] && depth[i] < 1100){lb_r=0; lb_g=30; lb_b=0;}
		// for screen inversion, if used later...
        if (DISPLAY_SEGMENT == 1){
            if (depth[i] == 0.0){
                    lb_r =  lb_g =  lb_b = 255;
            } 
            else { 
            switch( ((int) depth[i]) % 8){
                case 0: 
                    lb_r = 125; lb_g = 125; lb_b = 125;
                    break;
                case 1:
                    lb_r = 255; lb_g = 255; lb_b = 0;
                    break;
                case 2:
                    lb_r = 255; lb_g = 125; lb_b = 76;
                    break;
                case 3: 
                    lb_r = 255; lb_g = 0; lb_b = 0;
                    break;
                case 4:
                    lb_r = 255; lb_g = 0; lb_b = 255;
                    break;
                case 5: 
                    lb_r = 0; lb_g = 255; lb_b = 0;
                    break;
                case 6:
                    lb_r = 0; lb_g = 255; lb_b = 255;
                    break;
                case 7: 
                    lb_r = 0; lb_g = 0; lb_b = 255;
                    break;
                default: 
                    lb_r = 125; lb_g = 125; lb_b = 125;
            } 
            }
        }

		int j = i;
		int inverse_index = hand1y_curr * 640 + 639 - hand1x_curr;
		if (MIRROR) {
			j = inverse_index;
		}
		//if (depth[i] == 2047){lb_r = 255;  }
		depth_mid[3 * j + 0] = lb_r;
		depth_mid[3 * j + 1] = lb_g;
		depth_mid[3 * j + 2] = lb_b;

	}

*/

	//printf("min = %i, max = %i",min,max); 
	got_depth++;

	int h1_x = tinfo.handpos1_pix.x;
	int h1_y = tinfo.handpos1_pix.y;
	int h2_x = tinfo.handpos2_pix.x;
	int h2_y = tinfo.handpos2_pix.y;

//printf("%i %i %i %i\n",h1_x,h1_y,h2_x,h2_y); 

	/* h1_x = h1cgx;
	   h1_y = h1cgy;
	   h2_x = h2cgx;
	   h2_y = h2cgy; */

	if (tinfo.trackingstatus == TRK_TRACKING_OK) {
		//draw_square_color(h1_x, h1_y, 20, 255, 127, 0, MIRROR);	//orange
		//draw_square_color(h2_x, h2_y, 20, 72, 118, 255, MIRROR);	//blue
		draw_square_color(h1_x, h1_y, 10, 255, 255, 255, MIRROR);	//orange
		draw_square_color(h2_x, h2_y, 10, 255, 255, 225, MIRROR);	//blue
	}
	//draw_square_color(tinfo.handfirst1_x, tinfo.handfirst1_y, 20, 255, 255,255, MIRROR);
	//draw_square_color(tinfo.handfirst1_x, tinfo.handfirst1_y, 20, 255, 127,0, MIRROR);
	//draw_square_color(tinfo.handfirst2_x, tinfo.handfirst2_y, 20, 72, 118,
//                        255, MIRROR);

	draw_center_line(2, 198, 226, 255);

	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

void draw_center_line(int width, int r, int g, int b)
{
	int y, x, xStart, xEnd;
	xStart = 320 - width / 2;
	xEnd = xStart + width;
	for (y = 0; y < 480; y++)
		for (x = xStart; x < xEnd; x++) {
			int imgptr = y * 640 + x;
			depth_mid[3 * imgptr + 0] = r;
			depth_mid[3 * imgptr + 1] = g;
			depth_mid[3 * imgptr + 2] = b;
		}
}

void draw_square_color(int xPos, int yPos, int len, int r, int g, int b,
		       int mirror)
{

//    printf("%i\n",xPos); 
	int i;
	if (xPos == 0) {
		xPos = 1;
	};
	if (xPos - len < 0) {
		xPos = len;
	}
	if (yPos >= (480 - len)) {
		yPos = 480 - len;
	}

	for (i = 0; i < len * len; i++) {

		int imgptr = (yPos + i / len) * 640 + xPos + i % len;
		if (mirror) {
			imgptr = (yPos + i / len) * 640 + 639 - xPos + i % len;
		}
		if (imgptr < 640 * 480 && imgptr > 0) {
			depth_mid[3 * imgptr + 0] = r;
			depth_mid[3 * imgptr + 1] = g;
			depth_mid[3 * imgptr + 2] = b;
		}
	}
}

void draw_square(int xPos, int yPos, int len)
{
	int i;
	if (xPos == 0) {
		xPos = 1;
	};
	if (yPos >= (480 - len)) {
		yPos = 480 - len;
	}

	for (i = 0; i < len * len; i++) {
		int imgptr = (yPos + i / len) * 640 + xPos + i % len;

		if (imgptr < 640 * 480 && imgptr > 0) {
			depth_mid[3 * imgptr + 0] = 255;
			depth_mid[3 * imgptr + 1] = 255;
			depth_mid[3 * imgptr + 2] = 255;
		}
	}
}

void rgb_cb(freenect_device * dev, void *rgb, uint32_t timestamp)
{
	pthread_mutex_lock(&gl_backbuf_mutex);

	// swap buffers
	assert(rgb_back == rgb);
	rgb_back = rgb_mid;
	freenect_set_video_buffer(dev, rgb_back);
	rgb_mid = rgb;

	got_rgb++;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

void *freenect_threadfunc(void *arg)
{
/*
//	freenect_set_tilt_degs(f_dev,freenect_angle);
	freenect_set_led(f_dev,LED_RED);
	freenect_set_depth_callback(f_dev, depth_cb);
	freenect_set_video_callback(f_dev, rgb_cb);
	freenect_set_video_format(f_dev, current_format);
	freenect_set_depth_format(f_dev, FREENECT_DEPTH_11BIT);
	freenect_set_video_buffer(f_dev, rgb_back);
*/

//new version:
//      freenect_set_tilt_degs(f_dev,freenect_angle);
//	freenect_set_led(f_dev, LED_RED);
	freenect_set_depth_callback(f_dev, depth_cb);
	freenect_set_video_callback(f_dev, rgb_cb);
	freenect_set_video_mode(f_dev,
				freenect_find_video_mode
				(FREENECT_RESOLUTION_MEDIUM, current_format));
	freenect_set_depth_mode(f_dev, freenect_find_depth_mode
				//(FREENECT_RESOLUTION_HIGH,
				(FREENECT_RESOLUTION_MEDIUM,
				 FREENECT_DEPTH_11BIT));
	freenect_set_video_buffer(f_dev, rgb_back);

	freenect_start_depth(f_dev);
	freenect_start_video(f_dev);

	tinfo.trackingstatus = TRK_NO_TRACKING;
	tinfo.trackingframecounter = 0;
	tinfo.trackingstatustime = 0;

	printf
	    ("'w'-tilt up, 's'-level, 'x'-tilt down, '0'-'6'-select LED mode, 'f'-video format\n");

	while (!die && freenect_process_events(f_ctx) >= 0) {
		freenect_raw_tilt_state *state;
		freenect_update_tilt_state(f_dev);
		state = freenect_get_tilt_state(f_dev);
		double dx, dy, dz;
		freenect_get_mks_accel(state, &dx, &dy, &dz);
		//      printf("\r raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f", state->accelerometer_x, state->accelerometer_y, state->accelerometer_z, dx, dy, dz);
		fflush(stdout);

//        freenect_set_led(f_dev, LED_BLINK_GREEN);

        if (requested_kinect_led != current_kinect_led) {
            freenect_stop_video(f_dev);
            freenect_set_led(f_dev, requested_kinect_led);
            freenect_start_video(f_dev);
            current_kinect_led = requested_kinect_led;
            }

		if (requested_format != current_format) {
			freenect_stop_video(f_dev);
			//freenect_set_video_format(f_dev, requested_format);

			freenect_set_video_mode(f_dev,
						freenect_find_video_mode
						(FREENECT_RESOLUTION_MEDIUM,
						 requested_format));

			freenect_start_video(f_dev);
			current_format = requested_format;
		}
	}

	printf("\nshutting down streams...\n");

	freenect_stop_depth(f_dev);
	freenect_stop_video(f_dev);

	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);

	printf("-- done!\n");
	return NULL;
}
