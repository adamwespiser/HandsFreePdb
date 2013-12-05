#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <math.h>

#include "constants.h"

#include "tracking.h"
#include "control.h"
#include "control_mode.h"
//#include "vector3d.h"

#include "pdblib.h"

static GLfloat view_rotx = 20.0, view_roty = 30.0, view_rotz = 0.0;
static GLint myatomsphere, unitbond;

static GC gc;
static XGCValues gcval;

int numatoms = NUMBER_OF_ATOMS;
GLfloat x_mol[100], y_mol[100], z_mol[100];
int globmousex, globmousey;

//extern handtrack_t tinfo;
extern control_struct cinfo;

void fill_atom_coord_mock_molecule()
{
	int i;

	for (i = 0; i < numatoms; i++) {
		x_mol[i] = 5 * cos(6.28 * i / 10);
		y_mol[i] = 5 * sin(6.28 * i / 10);
		z_mol[i] = 1 * i - numatoms / 2;
	}

}

void fill_atom_coord() 
{
    int aatype[3000],i;
    double x_pdb_read[3000], y_pdb_read[3000], z_pdb_read[3000];
    FILE *fp = fopen("1UBQ.pdb", "r");
    numatoms = ReadPDB_All_CA(fp, aatype, x_pdb_read,y_pdb_read,z_pdb_read );

    double cgx=0,cgy=0,cgz=0;
    for(i=0;i<numatoms;i++) {
    cgx += x_pdb_read[i]; cgy+=y_pdb_read[i]; cgz+=z_pdb_read[i];
    }
    cgx/=numatoms; cgy/=numatoms;cgz/=numatoms;
    for(i=0;i<numatoms;i++) {
    x_pdb_read[i]-=cgx;y_pdb_read[i]-=cgy;z_pdb_read[i]-=cgz+Z_VIEW_CORRECT; //20A is the middle of the viewinig box
    }


//    float scalefactor = 1;
//    for(i=0;i<numatoms;i++) {
//    x_pdb_read[i]*=scalefactor;y_pdb_read[i]*=scalefactor;z_pdb_read[i]*=scalefactor;
//    }



    for(i=0;i<numatoms;i++) {
        x_mol[i]=x_pdb_read[i]; y_mol[i]=y_pdb_read[i]; z_mol[i]=z_pdb_read[i];
        }


fclose(fp);
}

static void reshape(int width, int height)
{
	GLfloat h = (GLfloat) height / (GLfloat) width; // h  = 1.00000

	glViewport(0, 0, (GLint) width, (GLint) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	glFrustum(-1.0, 1.0, -h, h, 0.1, 60.0);



//http://www.cprogramming.com/tutorial/opengl_projections.html

// box is 40 angstroms each way
//    glOrtho(-20, 20, -h*20, h*20, 1, 41);
//glFrustum(-20, 20, -h*20, h*20, 10, 50);

//http://profs.sci.univr.it/~colombar/html_openGL_tutorial/en/03basic_transforms_018.html
    gluPerspective(25.0, 1, h*20, -h*20);   
    printf("_____________ %f \n",h); 	
    exit;
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
//	glTranslatef(0.0, 0.0, -40.0);
}

static void
make_window(Display * dpy, const char *name,
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

//from glutfonts.c
void print_bitmap_string(void *font, char *s)
{
	if (s && strlen(s)) {
		while (*s) {
			glutBitmapCharacter(font, *s);
			s++;
		}
	}
}

static void draw(void)
{
	int i;
	GLfloat brm[16], cp, ct, sp, st, dx, dy, dz, dl, len,  theta;
	char buf[32];

	//float x_mol0[100], y_mol0[100], z_mol0[100];
    int xcg= 0; 
    int ycg= 0; 
    int zcg= 0; 
    

    if (cinfo.current_mode==CTRLMODE_GRAB && cinfo.hand1_vel < MAX_HAND_SPEED && cinfo.hand2_vel < MAX_HAND_SPEED ) {

    float dt = 2.5;
    float dx = (-1)*cinfo.center_vx * dt;
    float dy = (-1)*cinfo.center_vy * dt;
    float dz = cinfo.center_vz * dt; 

/*
#define ACCEL_FACTOR 0.001
#define VDECAY 0.3
#define VCTRL 1
#define INSTANT_FACTOR 1

    cinfo.cg_ax = (-1)*cinfo.center_vx * ACCEL_FACTOR;
    cinfo.cg_ay = (-1)*cinfo.center_vy * ACCEL_FACTOR;
    cinfo.cg_az =      cinfo.center_vz * ACCEL_FACTOR;

    cinfo.cg_vx = cinfo.cg_vx *(1-INSTANT_FACTOR) + INSTANT_FACTOR *( (-1)*cinfo.center_vx*VCTRL + cinfo.cg_ax * dt*(1-VCTRL) );
    cinfo.cg_vy = cinfo.cg_vy *(1-INSTANT_FACTOR) + INSTANT_FACTOR *( (-1)*cinfo.center_vy*VCTRL + cinfo.cg_ay * dt*(1-VCTRL) );
    cinfo.cg_vz = cinfo.cg_vz *(1-INSTANT_FACTOR) + INSTANT_FACTOR *(      cinfo.center_vz*VCTRL + cinfo.cg_az * dt*(1-VCTRL) );

    cinfo.cg_vx *= (1-VDECAY);
    cinfo.cg_vy *= (1-VDECAY);
    cinfo.cg_vz *= (1-VDECAY);
    
    float dx = cinfo.cg_vx * dt;
    float dy = cinfo.cg_vy * dt;
    float dz = cinfo.cg_vz * dt; 
*/
//AW code below

    cinfo.total_xcg += dx; 
    cinfo.total_ycg += dy; 
    cinfo.total_zcg += dz; 
    //printf("%f,%f,%f\n", cinfo.total_xcg, cinfo.total_ycg, cinfo.total_zcg); 

	for (i = 0; i < numatoms; i++) { 

        //TRANSLATION
        x_mol[i] +=  cinfo.total_xcg;
        y_mol[i] +=  cinfo.total_ycg; 
        z_mol[i] +=  cinfo.total_zcg;
        //Do counting for cg
        xcg += x_mol[i];
        ycg += y_mol[i];
        zcg += z_mol[i];
    }        
    
    xcg /= numatoms; 
    ycg /= numatoms; 
    zcg /= numatoms; 

	for (i = 0; i < numatoms; i++){ 
        x_mol[i] -= xcg;
		y_mol[i] -= ycg; 
		z_mol[i] -= zcg;
	}
    // now x_mol has been translated from the global view to the intertial view...
    // and the roation matrix can now be applied
    // calc center of gravity
    // r{row} {column}
    vector3d u_raw = get_hand_rotation_crossproduct(); 
    theta = vector_magnitude(u_raw) * ROTATIONAL_SCALING_FACTOR; 
    vector3d u = unit_vector(u_raw); 
    float ux = u.x;
    float uy = u.y;
    float uz = u.z;
    //printf("%f,%f,%f,= %f\n",u.x,u.y,u.z,(u.x*u.x + u.y*u.y+u.z*u.z));// works
    
    //theta += 3.145926/2.;
//    cinfo.total_theta += theta; 
 //   printf("theta: %f\n",cos(cinfo.total_theta)); 
    float cos_theta = cos(theta); 
    float one_minus_cos_theta = 1.0 - cos_theta; 
    float sin_theta = sin(theta); 
    //rotational matrix declared
    float rxx,rxy,rxz; 
    float ryx,ryy,ryz; 
    float rzx,rzy,rzz; 

    //assign rotational matrix for thete and vector u. 
    rxx = cos_theta  + ux * ux * one_minus_cos_theta;
    rxy = ux * uy * one_minus_cos_theta + uz * sin_theta ;
    rxz = ux * uz * one_minus_cos_theta - uy * sin_theta;

    ryx = uy * ux * one_minus_cos_theta - uz * sin_theta; 
    ryy = cos_theta + uy * uy * one_minus_cos_theta; 
    ryz = uy * uz * one_minus_cos_theta + ux * sin_theta; 

    rzx = uz * ux * one_minus_cos_theta + uy * sin_theta; 
    rzy = uz * uy * one_minus_cos_theta - ux * sin_theta; 
    rzz = cos_theta + uz * uz * one_minus_cos_theta; 

    //ROTAION 
	for (i = 0; i < numatoms; i++){ 
        
        #define DTROT 1

        float xtmp = x_mol[i]*DTROT, ytmp=y_mol[i]*DTROT, ztmp=z_mol[i]*DTROT;

        x_mol[i] =  rxx * xtmp + rxy * ytmp + rxz * ztmp ;
        y_mol[i] =  ryx * xtmp + ryy * ytmp + ryz * ztmp ;
        z_mol[i] =  rzx * xtmp + rzy * ytmp + rzz * ztmp ; 

    }

    for (i = 0; i < numatoms; i++) { 
        x_mol[i]+=xcg; 
        y_mol[i]+=ycg;
        z_mol[i]+=zcg;
    }


    } // end of if control mode is 'grab'

//XDrawString( dpy, win, gc, 100, 100, "XXX", strlen("XXX"));

	for (i = 0; i < 16; i++)
		brm[i] = 0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glScalef(0.7, 0.7, 0.7);

    //overall rotation of the scene (change viewing angle)
	glRotatef(view_rotx, 1.0, 0.0, 0.0);
	glRotatef(view_roty, 0.0, 1.0, 0.0);
	glRotatef(view_rotz, 0.0, 0.0, 1.0);

	glPushMatrix();

	glScalef(cinfo.hand2_z_out, cinfo.hand2_z_out, cinfo.hand2_z_out);

	glScalef(0.3, 0.3, 0.3);
	glCallList(myatomsphere);
	glPopMatrix();

//glCallList(unitbond);

//atoms
	for (i = 0; i < numatoms; i++) {
		glPushMatrix();
		glTranslatef(x_mol[i], y_mol[i], z_mol[i]);
//		if (i == 0)
			glScalef(0.3, 0.3, 0.3);
//		glCallList(myatomsphere);
		glPopMatrix();

//bonds
		if (i < numatoms - 1) {

			dx = x_mol[i + 1] - x_mol[i];
			dy = y_mol[i + 1] - y_mol[i];
			dz = z_mol[i + 1] - z_mol[i];

			len = sqrt(dx * dx + dy * dy + dz * dz);
			dl = sqrt(dx * dx + dy * dy);
			st = dz / len;
			ct = dl / len;
			cp = dx / dl;
			sp = dy / dl;

			glPushMatrix();
			glTranslatef(x_mol[i], y_mol[i], z_mol[i]);

// rotate cylinder "up" around y axis then around z axis
			brm[0] = cp * ct;
			brm[1] = sp * ct;
			brm[2] = st;
			brm[4] =-sp;
			brm[5] = cp;
			brm[6] = 0;
			brm[8] =-cp * st;
			brm[9] = sp * st;
			brm[10] = ct;
			brm[15] = 1;
			glMultMatrixf(brm);

//stretch x-wise to bond length
			glScalef(len, 1, 1);

//only for glut cylinder which is along Z axis
			glRotatef(90, 0, 1, 0);
			glScalef(0.3, 0.3, 1);

			glCallList(unitbond);
			glPopMatrix();
		}

	}

/* text */
	glColor4f(0.0, 1.0, 0.0, 1.0);
	glRasterPos2f(-3, -3);	// x and y in the model coordinates, not on image plane!
	sprintf(buf, "ZZ %d %d %.4f  MODE %d", globmousex, globmousey,
		cinfo.hand2_z_out, cinfo.current_mode);
	print_bitmap_string(GLUT_BITMAP_9_BY_15, buf);

	glPopMatrix();
}

static void init(void)
{
	static GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };
	static GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
	static GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
//	static GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };

	GLUquadricObj *quadr;

	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	myatomsphere = glGenLists(1);
	glNewList(myatomsphere, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);

	quadr = gluNewQuadric();
	gluQuadricDrawStyle(quadr, GLU_FILL);
	gluSphere(quadr, 1, 10, 10);
	gluDeleteQuadric(quadr);

// MyAtomSphere( 1, 30, 30);
	glEndList();

	unitbond = glGenLists(1);
	glNewList(unitbond, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
//  UnitBond( 1, 0.4, 30 );

	quadr = gluNewQuadric();
	gluQuadricDrawStyle(quadr, GLU_FILL);
	gluDisk(quadr, 0, 1, 10, 4);
	gluCylinder(quadr, 1, 1, 1, 10, 10);
	glTranslatef(0, 0, 1);
	gluDisk(quadr, 0, 1, 10, 4);
	gluDeleteQuadric(quadr);

	glEndList();

	glEnable(GL_NORMALIZE);
}

static void event_loop(Display * dpy, Window win)
{

	int needredraw = 0;

	while (1) {

		while (XPending(dpy) > 0) {
			XEvent event;
			XNextEvent(dpy, &event);

			needredraw = 0;

			switch (event.type) {
			case MotionNotify:
				{

//					x_mol[4] = event.xmotion.x * 0.01;
//					y_mol[4] = event.xmotion.y * 0.01;

					// in pixels, (0,0) is top left corner of the window

					globmousex = event.xmotion.x;
					globmousey = event.xmotion.y;

					needredraw = 1;

					break;
				}

			case ButtonPress:
				{

/*					if (event.xbutton.button == 1)
						x_mol[3] += 0.3;
					if (event.xbutton.button == 2)
						x_mol[3] -= 0.3;
*/
					needredraw = 1;

					break;
				}
			case Expose:
				{	/* we'll redraw below */
					needredraw = 1;
					break;
				}
			case ConfigureNotify:
				reshape(event.xconfigure.width,
					event.xconfigure.height);
				needredraw = 1;
				break;
			case KeyPress:
				{
					char buffer[10];
					int r, code;
					code = XLookupKeysym(&event.xkey, 0);
					if (code == XK_Left) {
						view_roty += 5.0;
					} else if (code == XK_Right) {
						view_roty -= 5.0;
					} else if (code == XK_Up) {
						view_rotx += 5.0;
					} else if (code == XK_Down) {
						view_rotx -= 5.0;
					}

					else {
						r = XLookupString(&event.xkey,
								  buffer,
								  sizeof
								  (buffer),
								  NULL, NULL);
						if (buffer[0] == 27) {
							/* escape */
							return;
						}
						/*if (buffer[0] == 'a') {
							x_mol[3] += 0.5;
						}
						if (buffer[0] == 's') {
							x_mol[3] -= 0.5;
						}*/

					}
					needredraw = 1;
					break;

				}
			}
		}

//       XDrawString( dpy, win, gc, 20, 20, "XXX", strlen("XXX"));
//      glXWaitX();
//       draw();
//       glXSwapBuffers(dpy, win);

		if (needredraw) {

//view_roty = -(hand1_z -hand2_z)*0.3;

			view_rotx = cinfo.hand1_y_out * 2;
			view_roty = cinfo.hand1_x_out * 1.2;

//x_mol[5] = hand_z / 100;

			draw();

			char buf[32];
			sprintf(buf, "%d %d   STAT %d", globmousex, globmousey,
				cinfo.current_mode);

			//sprintf(buf,"%.1f %.1f   %.1f", hand1_z,hand2_z, hand1_z - hand2_z);
			// XDrawLine(dpy,win,gc, 200, 200, globmousex, globmousey);

			XDrawString(dpy, win, gc, 20, 20, buf, strlen(buf));
			glXSwapBuffers(dpy, win);
//needredraw = 0;

		}		//end while XPending(dpy)
	}			//end while(1)
}				// end event_loop;


/*

what to do if:
[xcb] Unknown sequence number while processing reply
[xcb] Most likely this is a multi-threaded client and XInitThreads has not been called
[xcb] Aborting, sorry about that.
a.out: ../../src/xcb_io.c:634: _XReply: Assertion `!xcb_xlib_threads_sequence_lo

or typically
a.out: ../../src/xcb_io.c:528: _XAllocID: Assertion `ret != inval_id' failed.
Aborted (core dumped)


*/

void *molecular_threadfunc_mol(void *arg)
{
	Display *dpy;
	Window win;
	GLXContext ctx;
	char *dpyName = NULL;
	//GLboolean printInfo = GL_FALSE;
	int screen;
	long fg, bg;
	XFontStruct *font_info;

	printf("starting molecular thread\n");

// glutInit(&g_argc, g_argv);

	fill_atom_coord();

	dpy = XOpenDisplay(dpyName);
	if (!dpy) {
		printf("Error: couldn't open display %s\n", dpyName);
		//return -1;
		return NULL;		//"return -1;" was giving an error, going to try this...
	}
//glutInit(NULL, NULL);

	make_window(dpy, "glxgears", 400, 0, 900, 900, &win, &ctx);

	screen = DefaultScreen(dpy);
	bg = BlackPixel(dpy, screen);
	fg = WhitePixel(dpy, screen);
	gcval.foreground = fg;
	gcval.background = bg;
	gc = XCreateGC(dpy, win, GCForeground | GCBackground, &gcval);

//  if ( (font_info = XLoadQueryFont(dpy, "-*-helvetica-*-r-*-*-*-180-*-*-*-*-*-*")) == NULL)

	if ((font_info = XLoadQueryFont(dpy, "-urw-nimbus sans l-*-r-normal-*-*-*-*-*-*-*-*-*")) == NULL) {	// use xfontsel to choose the font
		fprintf(stderr, "XLoadQueryFont\n");
		exit(1);
	}
	XSetFont(dpy, gc, font_info->fid);

	XMapWindow(dpy, win);
	glXMakeCurrent(dpy, win, ctx);
	reshape(900, 900);

// pass only keyboard and 
	XSelectInput(dpy, win,
		     KeyPressMask | ButtonPressMask | PointerMotionMask);

	init();

	event_loop(dpy, win);

	glXDestroyContext(dpy, ctx);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return NULL;

}
