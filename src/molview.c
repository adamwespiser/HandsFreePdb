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
#include "pdblib.h"
//#include "vector3d.h"

static GLfloat view_rotx = 20.0, view_roty = 30.0;
static GLint myatomsphere, unitbond, unitbond2, yellowbond;
static GC gc;
static XGCValues gcval;

int nbonds_inter = 0;		//intermolecular bonds
int numatoms = NUMBER_OF_ATOMS;
int movement_atom[PDB_MAX_ATOMS] = { 0 };	// 1 if movement is yes, 0 otherwise(still) 
int globmousex, globmousey;

GLfloat x_mol[PDB_MAX_ATOMS], y_mol[PDB_MAX_ATOMS], z_mol[PDB_MAX_ATOMS];

extern control_struct cinfo;

void init_molecule_rot_trans();

void fill_atom_coord_mock_molecule()
{
	int i;
	for (i = 0; i < numatoms; i++) {
		x_mol[i] = 5 * cos(6.28 * i / 10);
		y_mol[i] = 5 * sin(6.28 * i / 10);
		z_mol[i] = 1 * i - numatoms / 2;
	}

}


void fill_atom_coord_new(mol_thread_struct * molinfo)
{
	int chain_spacer = 0;
	int aatype[3000], i;
	double x_pdb_read[3000], y_pdb_read[3000], z_pdb_read[3000];
	double x_pdb[3000], y_pdb[3000], z_pdb[3000];
	char chain_pdb_read[3000];
	FILE *fp = fopen(molinfo->pdb_file, "r");
	int pdb_i = 0;;
	int first_chain_found = 0;
	int second_chain_found = 0;
	char current_chain = '-';
	char first_chain = '-';
	char second_chain = '-';

	if (fp == NULL) {
		fprintf(stderr,
			"Can't open pdb file. Please check the location of file.\n");
		exit(1);
	}
	//printf("inside fill_atom_coord_new()...\n");

	numatoms =
	    ReadPDB_All_CA_Chain(fp, aatype, x_pdb_read, y_pdb_read, z_pdb_read,
				 chain_pdb_read);


    int chain_one_present = 0;
    int chain_two_present = 0;
    for (i = 0; i < numatoms; i++){
        if (chain_pdb_read[i] == molinfo->chain_one)
            chain_one_present = 1;
        if (chain_pdb_read[i] == molinfo->chain_two)
            chain_two_present = 1;
    }
    if (!chain_one_present && ! chain_two_present){
        fprintf(stderr, "ERROR: Neither chain %c nor %c were discovered in the pdb file\n",molinfo->chain_one, molinfo->chain_two);
        exit(1);
    }
    if (!chain_one_present){
        fprintf(stderr, "ERROR: Chain %c not discovered in the pdb file\n",molinfo->chain_one);}
    if (!chain_two_present){
        fprintf(stderr, "ERROR: Chain %c not discovered in the pdb file\n",molinfo->chain_two);}

	for (i = 0; (i < numatoms && pdb_i <= PDB_MAX_ATOMS); i++) {
		char cc = chain_pdb_read[i];
		// init case
		if ((cc == molinfo->chain_one || cc == molinfo->chain_two)
		    && first_chain_found == 0) {
			if (molinfo->chain_one == cc) {
				first_chain = molinfo->chain_one;
				second_chain = molinfo->chain_two;
			} else {
				first_chain = molinfo->chain_two;
				second_chain = molinfo->chain_one;
			}
			current_chain = first_chain;
			first_chain_found = 1;
			x_pdb[pdb_i] = x_pdb_read[i];
			y_pdb[pdb_i] = y_pdb_read[i];
			z_pdb[pdb_i] = z_pdb_read[i];
			movement_atom[pdb_i] = 1;
			pdb_i++;
			continue;
		}
		//read through first chain
		if (cc == first_chain && cc == current_chain
		    && first_chain_found && !second_chain_found) {

			x_pdb[pdb_i] = x_pdb_read[i];
			y_pdb[pdb_i] = y_pdb_read[i];
			z_pdb[pdb_i] = z_pdb_read[i];
			movement_atom[pdb_i] = 1;
			pdb_i++;
			continue;
		}
		//intermitten chain // or end chain
		if (cc != first_chain && cc != second_chain) {
			if (current_chain == first_chain && !second_chain_found) {
				chain_spacer = pdb_i - 1;
				current_chain = '-';
				continue;
			}
			//break off case, both chains found
			else if (first_chain_found && second_chain_found) {
				i = numatoms + 1;
				continue;
			}

		}
		//init second chain
		if (cc == second_chain && !second_chain_found) {
			if (current_chain == first_chain) {
				chain_spacer = pdb_i - 1;
				second_chain_found = 1;
			}

			current_chain = second_chain;
			x_pdb[pdb_i] = x_pdb_read[i];
			y_pdb[pdb_i] = y_pdb_read[i];
			z_pdb[pdb_i] = z_pdb_read[i];
			movement_atom[pdb_i] = 0;
			pdb_i++;
			continue;
		}
		if (cc == second_chain && second_chain_found) {
			x_pdb[pdb_i] = x_pdb_read[i];
			y_pdb[pdb_i] = y_pdb_read[i];
			z_pdb[pdb_i] = z_pdb_read[i];
			movement_atom[pdb_i] = 0;
			pdb_i++;
			continue;
		}
	}

	numatoms = pdb_i;
    if (numatoms > PDB_MAX_ATOMS){
        fprintf(stderr, "WARNING: combined chain size is larger than maximum allowed\nCurrent of chains %c %c is %i\n",molinfo->chain_one,molinfo->chain_two,numatoms);}

	printf("chain spacer: %d numatoms: %d\n", chain_spacer, numatoms);
	printf("first chain: %c second chain: %c\n", first_chain, second_chain);
	for (i = 0; i <= chain_spacer; i++) {
		movement_atom[i] = 1;
	}

	double cgx = 0, cgy = 0, cgz = 0;
	for (i = 0; i < numatoms; i++) {
		cgx += x_pdb[i];
		cgy += y_pdb[i];
		cgz += z_pdb[i];
	}
	cgx /= numatoms;
	cgy /= numatoms;
	cgz /= numatoms;
	for (i = 0; i < numatoms; i++) {
		x_pdb[i] -= cgx;
		y_pdb[i] -= cgy;
		z_pdb[i] -= cgz + Z_VIEW_CORRECT;
	}

	for (i = 0; i < numatoms; i++) {
		x_mol[i] = x_pdb[i];
		y_mol[i] = y_pdb[i];
		z_mol[i] = z_pdb[i];
	}
	init_molecule_rot_trans();
	fclose(fp);
}

static void reshape(int width, int height)
{
	GLfloat h = (GLfloat) height / (GLfloat) width;	// h  = 1.00000

	glViewport(0, 0, (GLint) width, (GLint) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(25.0, 1, h * 20, -h * 20);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
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

vector3d calculate_cg_movement_chain()
{
	vector3d v = { 0, 0, 0 };
	int i;
	int count = 0;
	for (i = 0; i < numatoms; i++) {
		if (movement_atom[i] == cinfo.chain_mode
		    || cinfo.chain_mode == MOVE_ALL_ATOMS) {
			//  if (movement_atom[i] == 1){ 
			count++;
			v.x += x_mol[i];
			v.y += y_mol[i];
			v.z += z_mol[i];
		}

	}
	if (count > 0) {
		v.x /= count;
		v.y /= count;
		v.z /= count;
	} else {
		v.x = 0;
		v.y = 0;
		v.z = 0;
	}
	return v;
}

vector3d calculate_cg()
{
	vector3d v = { 0, 0, 0 };
	int i;
	for (i = 0; i < numatoms; i++) {
		v.x += x_mol[i];
		v.y += y_mol[i];
		v.z += z_mol[i];

	}
	v.x /= numatoms;
	v.y /= numatoms;
	v.z /= numatoms;
	return v;
}

void global_to_intertial_ref_movement_chain(vector3d cg)
{
	int i;
	for (i = 0; i < numatoms; i++) {
		if (movement_atom[i] == cinfo.chain_mode
		    || cinfo.chain_mode == MOVE_ALL_ATOMS) {
			x_mol[i] -= cg.x;
			y_mol[i] -= cg.y;
			z_mol[i] -= cg.z;
		}
	}
}

void global_to_intertial_ref(vector3d cg)
{
	int i;
	for (i = 0; i < numatoms; i++) {
		x_mol[i] -= cg.x;
		y_mol[i] -= cg.y;
		z_mol[i] -= cg.z;
	}
}

void inertial_to_global_ref_movement_chain(vector3d cg)
{
	int i;
	for (i = 0; i < numatoms; i++) {

		if (movement_atom[i] == cinfo.chain_mode
		    || cinfo.chain_mode == MOVE_ALL_ATOMS) {
			x_mol[i] += cg.x;
			y_mol[i] += cg.y;
			z_mol[i] += cg.z;
		}
	}

}

void inertial_to_global_ref(vector3d cg)
{
	int i;
	for (i = 0; i < numatoms; i++) {

		x_mol[i] += cg.x;
		y_mol[i] += cg.y;
		z_mol[i] += cg.z;
	}

}

void translate_mol_linear(int move_one_chain)
{
	float dt = 2.5;

	dt /= cinfo.slowdown;
//printf("%f",cinfo.slowdown);

	float dx = (-1) * cinfo.center_vx * dt;
	float dy = (-1) * cinfo.center_vy * dt;
	float dz = cinfo.center_vz * dt;
	int i;
	//printf("%.4f,%.4f\n",cinfo.hand1_vel, cinfo.hand2_vel); 
	if (cinfo.hand1_vel < MAX_HAND_SPEED
	    && cinfo.hand2_vel < MAX_HAND_SPEED) {
		if (move_one_chain) {
			for (i = 0; i < numatoms; i++) {
				if (movement_atom[i] == cinfo.chain_mode
				    || cinfo.chain_mode == MOVE_ALL_ATOMS) {
					x_mol[i] += dx;
					y_mol[i] += dy;
					z_mol[i] += dz;
				}
			}
		} else {

			for (i = 0; i < numatoms; i++) {

				x_mol[i] += dx;
				y_mol[i] += dy;
				z_mol[i] += dz;
			}
		}

	}
}

void translate_movement_chain_by_vector3d(vector3d v)
{
	int i;
	for (i = 0; i < numatoms; i++) {

		if (movement_atom[i] == cinfo.chain_mode
		    || cinfo.chain_mode == MOVE_ALL_ATOMS) {
			x_mol[i] += v.x;
			y_mol[i] += v.y;
			z_mol[i] += v.z;
		}
	}

}

void translate_by_vector3d(vector3d v)
{
	int i;
	for (i = 0; i < numatoms; i++) {
		x_mol[i] += v.x;
		y_mol[i] += v.y;
		z_mol[i] += v.z;
	}

}

///
float determine_max_xy(float zdepth)
{
	return (0.221695 * zdepth);
}

void move_mol_into_bounds_movement_chain()
{
	vector3d trans = { 0, 0, 0 };
	vector3d vcg = calculate_cg_movement_chain();
	float max_xy = determine_max_xy(-vcg.z);

	if (vcg.x < -max_xy) {
		trans.x = -max_xy - vcg.x;
	} else if (vcg.x > max_xy) {
		trans.x = max_xy - vcg.x;
	}

	if (vcg.y < -max_xy) {
		trans.y = -max_xy - vcg.y;
	} else if (vcg.y > max_xy) {
		trans.y = max_xy - vcg.y;
	}
	translate_movement_chain_by_vector3d(trans);
}

void move_mol_into_bounds()
{
	vector3d trans = { 0, 0, 0 };
	vector3d vcg = calculate_cg();
	float max_xy = determine_max_xy(-vcg.z);

	if (vcg.x < -max_xy) {
		trans.x = -max_xy - vcg.x;
	} else if (vcg.x > max_xy) {
		trans.x = max_xy - vcg.x;
	}

	if (vcg.y < -max_xy) {
		trans.y = -max_xy - vcg.y;
	} else if (vcg.y > max_xy) {
		trans.y = max_xy - vcg.y;
	}
	translate_by_vector3d(trans);

}

void translate_mol()
{
	float dt = 2.5;
	cinfo.cg_vx =
	    cinfo.cg_vx * (1 - INSTANT_FACTOR) +
	    INSTANT_FACTOR * ((-1) * cinfo.center_vx * VCTRL * ACC +
			      cinfo.cg_ax * dt * (1 - VCTRL));
	cinfo.cg_vy =
	    cinfo.cg_vy * (1 - INSTANT_FACTOR) +
	    INSTANT_FACTOR * ((-1) * cinfo.center_vy * VCTRL * ACC +
			      cinfo.cg_ay * dt * (1 - VCTRL));
	cinfo.cg_vz =
	    cinfo.cg_vz * (1 - INSTANT_FACTOR) +
	    INSTANT_FACTOR * (cinfo.center_vz * VCTRL * ACC +
			      cinfo.cg_az * dt * (1 - VCTRL));

	float vmag =
	    (cinfo.cg_vx * cinfo.cg_vx) + (cinfo.cg_vy * cinfo.cg_vy) +
	    (cinfo.cg_vz * cinfo.cg_vz);
	float attenuation =
	    1. - 0.5 * exp(-vmag / (STATIC_THRESHOLD * STATIC_THRESHOLD));

	cinfo.cg_vx *= (1 - FRICTION) * attenuation;
	cinfo.cg_vy *= (1 - FRICTION) * attenuation;
	cinfo.cg_vz *= (1 - FRICTION) * attenuation;

	float dx = cinfo.cg_vx * dt;
	float dy = cinfo.cg_vy * dt;
	float dz = cinfo.cg_vz * dt;

	int i;
	for (i = 0; i < numatoms; i++) {
		x_mol[i] += dx;
		y_mol[i] += dy;
		z_mol[i] += dz;
	}

}

void calc_rotation_simple()
{
	vector3d u_raw = get_hand_rotation_crossproduct();
	cinfo.omega = vector_magnitude(u_raw) * ROTATIONAL_SCALING_FACTOR;
	vector3d u = unit_vector(u_raw);
	cinfo.ux = u.x;
	cinfo.uy = u.y;
	cinfo.uz = u.z;
}

void calc_rotation_theta_and_u()
{

// calculates cinfo.u{x,y,z} && cinfo.omega; 
	vector3d u_raw = get_hand_rotation_crossproduct();
	float theta = vector_magnitude(u_raw) * ROTATIONAL_SCALING_FACTOR;

	cinfo.omega =
	    cinfo.omega * (1 - INSTANT_FACTOR_ROT) +
	    INSTANT_FACTOR_ROT * theta * ACC_ROT;
	cinfo.omega *= (1 - 0.01);
	vector3d u = unit_vector(u_raw);
	cinfo.ux =
	    cinfo.ux * (1 - INSTANT_FACTOR_U) + INSTANT_FACTOR_U * u.x * ACC_U;
	cinfo.uy =
	    cinfo.uy * (1 - INSTANT_FACTOR_U) + INSTANT_FACTOR_U * u.y * ACC_U;
	cinfo.uz =
	    cinfo.uz * (1 - INSTANT_FACTOR_U) + INSTANT_FACTOR_U * u.z * ACC_U;

}

void apply_rotation_to_movement_chain()
{
#define DTROT 1
// frrom cinfo.u{x,y,z} && cinfo.omega; 
	float ux = cinfo.ux;
	float uy = cinfo.uy;
	float uz = cinfo.uz;

	float theta = cinfo.omega / cinfo.slowdown;
	float cos_theta = cos(theta);
	float one_minus_cos_theta = 1.0 - cos_theta;
	float sin_theta = sin(theta);

	float rxx, rxy, rxz;
	float ryx, ryy, ryz;
	float rzx, rzy, rzz;

	//assign rotational matrix for thete and vector u. 
	rxx = cos_theta + ux * ux * one_minus_cos_theta;
	rxy = ux * uy * one_minus_cos_theta + uz * sin_theta;
	rxz = ux * uz * one_minus_cos_theta - uy * sin_theta;

	ryx = uy * ux * one_minus_cos_theta - uz * sin_theta;
	ryy = cos_theta + uy * uy * one_minus_cos_theta;
	ryz = uy * uz * one_minus_cos_theta + ux * sin_theta;

	rzx = uz * ux * one_minus_cos_theta + uy * sin_theta;
	rzy = uz * uy * one_minus_cos_theta - ux * sin_theta;
	rzz = cos_theta + uz * uz * one_minus_cos_theta;

	int i;
	for (i = 0; i < numatoms; i++) {
		if (movement_atom[i] == cinfo.chain_mode
		    || cinfo.chain_mode == MOVE_ALL_ATOMS) {
			float xtmp = x_mol[i] * DTROT, ytmp =
			    y_mol[i] * DTROT, ztmp = z_mol[i] * DTROT;

			x_mol[i] = rxx * xtmp + rxy * ytmp + rxz * ztmp;
			y_mol[i] = ryx * xtmp + ryy * ytmp + ryz * ztmp;
			z_mol[i] = rzx * xtmp + rzy * ytmp + rzz * ztmp;
		}

	}

}

void apply_rotation_to_molecule()
{
// frrom cinfo.u{x,y,z} && cinfo.omega; 
	float ux = cinfo.ux;
	float uy = cinfo.uy;
	float uz = cinfo.uz;

	float theta = cinfo.omega;
	float cos_theta = cos(theta);
	float one_minus_cos_theta = 1.0 - cos_theta;
	float sin_theta = sin(theta);

	float rxx, rxy, rxz;
	float ryx, ryy, ryz;
	float rzx, rzy, rzz;

	//assign rotational matrix for thete and vector u. 
	rxx = cos_theta + ux * ux * one_minus_cos_theta;
	rxy = ux * uy * one_minus_cos_theta + uz * sin_theta;
	rxz = ux * uz * one_minus_cos_theta - uy * sin_theta;

	ryx = uy * ux * one_minus_cos_theta - uz * sin_theta;
	ryy = cos_theta + uy * uy * one_minus_cos_theta;
	ryz = uy * uz * one_minus_cos_theta + ux * sin_theta;

	rzx = uz * ux * one_minus_cos_theta + uy * sin_theta;
	rzy = uz * uy * one_minus_cos_theta - ux * sin_theta;
	rzz = cos_theta + uz * uz * one_minus_cos_theta;

	int i;
	for (i = 0; i < numatoms; i++) {

		float xtmp = x_mol[i] * DTROT, ytmp = y_mol[i] * DTROT, ztmp =
		    z_mol[i] * DTROT;

		x_mol[i] = rxx * xtmp + rxy * ytmp + rxz * ztmp;
		y_mol[i] = ryx * xtmp + ryy * ytmp + ryz * ztmp;
		z_mol[i] = rzx * xtmp + rzy * ytmp + rzz * ztmp;

	}

}

void init_molecule_rot_trans()
{
	int i;
	float x_start = 0;
	float y_start = 0;
	float z_start = -60;
	for (i = 0; i < numatoms; i++) {
		x_mol[i] += x_start;
		y_mol[i] += y_start;
		z_mol[i] += z_start;
	}
	vector3d cg_vector = calculate_cg();
	global_to_intertial_ref(cg_vector);
//90,90,220
	float x_rot_init = DEG_TO_RAD_FACTOR * 50.;
	float y_rot_init = DEG_TO_RAD_FACTOR * 30.;
	float z_rot_init = DEG_TO_RAD_FACTOR * 160.;
	vector3d u_raw_x = { 1, 0, 0 };
	vector3d u_raw_y = { 0, 1, 0 };
	vector3d u_raw_z = { 0, 0, 1 };
	vector3d u;

	vector3d cg_vector1 = calculate_cg_movement_chain();

	printf("x: %.4f y: %.4f z: %.4f\n", (x_mol[109] - cg_vector1.x) / 360,
	       (y_mol[109] - cg_vector1.y) / 360,
	       (z_mol[109] - cg_vector1.z) / 360);
	//x rotation
	cinfo.omega = x_rot_init;
	u = unit_vector(u_raw_x);
	cinfo.ux = u.x;
	cinfo.uy = u.y;
	cinfo.uz = u.z;
	apply_rotation_to_molecule();

	//y rotation
	cinfo.omega = y_rot_init;
	u = unit_vector(u_raw_y);
	cinfo.ux = u.x;
	cinfo.uy = u.y;
	cinfo.uz = u.z;
	apply_rotation_to_molecule();

	//z rotation
	cinfo.omega = z_rot_init;
	u = unit_vector(u_raw_z);
	cinfo.ux = u.x;
	cinfo.uy = u.y;
	cinfo.uz = u.z;
	apply_rotation_to_molecule();

	inertial_to_global_ref(cg_vector);

	cg_vector = calculate_cg_movement_chain();
	cg_vector = unit_vector(cg_vector);
	printf("x: %.4f y: %.4f z: %.4f\n", (x_mol[109] - cg_vector.x) / 360,
	       (y_mol[109] - cg_vector.y) / 360, z_mol[109] - cg_vector.z);
}

static void draw(void)
{
	int i;
	GLfloat brm[16], cp, ct, sp, st, dx, dy, dz, dl, len;
	//GLfloat brm[16], cp, ct, sp, st, dx, dy, dz, dl, len,  theta;
	char buf[32];

	cinfo.slowdown = 1 + 0.08 * nbonds_inter;
	if (cinfo.slowdown > 6)
		cinfo.slowdown = 6;

	if (cinfo.current_mode == CTRLMODE_GRAB) {

		translate_mol_linear(1);
		vector3d cg_vector = calculate_cg_movement_chain();
		//printf("x: %.4f y: %.4f z: %.4f\n",x_mol[109] - cg_vector.x, y_mol[109] - cg_vector.y, z_mol[109] - cg_vector.z); 
		global_to_intertial_ref_movement_chain(cg_vector);
		//calc_rotatation_theta_and_u(); 
		calc_rotation_simple();
		apply_rotation_to_movement_chain();
		inertial_to_global_ref_movement_chain(cg_vector);
		move_mol_into_bounds_movement_chain();
	}			// end of if control mode is 'grab'

	for (i = 0; i < 16; i++)
		brm[i] = 0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	glScalef(0.7, 0.7, 0.7);

	//overall rotation of the scene (change viewing angle)
//      glRotatef(view_rotx, 1.0, 0.0, 0.0);
//j     glRotatef(view_roty, 0.0, 1.0, 0.0);
//      glRotatef(view_rotz, 0.0, 0.0, 1.0);

	glPushMatrix();

	//glScalef(cinfo.hand2_z_out, cinfo.hand2_z_out, cinfo.hand2_z_out);

//      glScalef(0.3, 0.3, 0.3);
	glCallList(myatomsphere);
	glPopMatrix();

//glCallList(unitbond);

// text label
	glPushMatrix();
	glRasterPos3f(-25, 25, -Z_VIEW_CORRECT);	//in model coords!

	if (cinfo.current_mode == GESTURE_GRAB) {
		sprintf(buf, "   GRAB");
	} else {
		sprintf(buf, "NO GRAB");
	}

	if (cinfo.chain_mode == 0) {
		strcat(buf, " SEL CHAIN 1");
	} else if (cinfo.chain_mode == 1) {
		strcat(buf, " SEL CHAIN 2");
	} else {
		strcat(buf, " SEL ALL");
	}

	char b2[32];
	sprintf(b2, " %.2f %d", cinfo.slowdown, nbonds_inter);
	strcat(buf, b2);

	glColor3f(2.0, 2.0, 2.0);
	print_bitmap_string(GLUT_BITMAP_HELVETICA_18, buf);
	glPopMatrix();

	nbonds_inter = 0;

//the molecule
	for (i = 0; i < numatoms; i++) {

//clashed atoms & intermolecular bonds
		int ii;
		for (ii = i + 1; ii < numatoms; ii++) {

			dx = x_mol[ii] - x_mol[i];
			dy = y_mol[ii] - y_mol[i];
			dz = z_mol[ii] - z_mol[i];

			len = (dx * dx + dy * dy + dz * dz);
			if (len < 5 * 5
			    &&
			    ((movement_atom[i] == 0 && movement_atom[ii] == 1)
			     || (movement_atom[i] == 1
				 && movement_atom[ii] == 0))) {

#define CLASH_SPHERE 2.5

				glPushMatrix();
				glTranslatef(x_mol[i], y_mol[i], z_mol[i]);
				glScalef(CLASH_SPHERE, CLASH_SPHERE,
					 CLASH_SPHERE);
				glCallList(myatomsphere);
				glPopMatrix();

				glPushMatrix();
				glTranslatef(x_mol[ii], y_mol[ii], z_mol[ii]);
				glScalef(CLASH_SPHERE, CLASH_SPHERE,
					 CLASH_SPHERE);
				glCallList(myatomsphere);
				glPopMatrix();
			}

			if (len < 8 * 8 && len > 5 * 5
			    &&
			    ((movement_atom[i] == 0 && movement_atom[ii] == 1)
			     || (movement_atom[i] == 1
				 && movement_atom[ii] == 0))) {

				nbonds_inter++;

				glPushMatrix();

				len = sqrt(dx * dx + dy * dy + dz * dz);
				dl = sqrt(dx * dx + dy * dy);
				st = dz / len;
				ct = dl / len;
				cp = dx / dl;
				sp = dy / dl;

				glTranslatef(x_mol[i], y_mol[i], z_mol[i]);

				// rotate cylinder "up" around y axis then around z axis
				brm[0] = cp * ct;
				brm[1] = sp * ct;
				brm[2] = st;
				brm[4] = -sp;
				brm[5] = cp;
				brm[6] = 0;
				brm[8] = -cp * st;
				brm[9] = sp * st;
				brm[10] = ct;
				brm[15] = 1;
				glMultMatrixf(brm);

				//stretch x-wise to bond length
				glScalef(len + 0.2, 1, 1);

				//only for glut cylinder which is along Z axis

				float bond_dia;

				bond_dia = 0.1;

				//TODO: bond diameter via Lennard-Jones!
				bond_dia = 0.4 - (len - 5) / 3. * 0.35;

				glRotatef(90, 0, 1, 0);
				glScalef(bond_dia, bond_dia, 1);	// transient yellow bonds are thin

				glCallList(yellowbond);
				glPopMatrix();

			}

		}

		//intra-chain bonds
		if (i < numatoms - 1) {

			dx = x_mol[i + 1] - x_mol[i];
			dy = y_mol[i + 1] - y_mol[i];
			dz = z_mol[i + 1] - z_mol[i];
            
			len = sqrt(dx * dx + dy * dy + dz * dz);
            //should take care of any far away sequences...
            if (len > 10){
                //printf("found length greater than 10\n");
                continue; 
            } 
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
			brm[4] = -sp;
			brm[5] = cp;
			brm[6] = 0;
			brm[8] = -cp * st;
			brm[9] = sp * st;
			brm[10] = ct;
			brm[15] = 1;
			glMultMatrixf(brm);

			//stretch x-wise to bond length
			glScalef(len + 0.2, 1, 1);

			//only for glut cylinder which is along Z axis
			glRotatef(90, 0, 1, 0);
			glScalef(0.3, 0.3, 1);

			if (movement_atom[i] == 1) {
				glCallList(unitbond);
			} else {
				glCallList(unitbond2);
			}

//glColor4f(1.0, 0.0, 0.0, 1.0);
//RasterPos3f(cinfo.u.x,cinfo.u.y,cinfo.uz );   // x and y in the model coordinates, not on image plane!
			// if (cinfo.current_mode == CTRLMODE_GRAB) {
//printf(buf, "GRAB"); 
//rint_bitmap_string(GLUT_BITMAP_9_BY_15, buf);

//float zzz[4];
//glGetFloatv(GL_CURRENT_RASTER_POSITION, zzz);
//printf("%f %f %f\n", zzz[0],zzz[1],zzz[2]);

			glPopMatrix();
		}

	}

	glPopMatrix();

}

static void init(void)
{
	static GLfloat pos[4] = { 5.0, 5.0, 10.0, 0.0 };
	static GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };	// ... 0.5 for semi-transp spheres
	static GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
	static GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };
	static GLfloat yellow[4] = { 1.0, 1.0, 0.1, 1.0 };

	GLUquadricObj *quadr;

	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	static GLfloat pos2[4] = { -25, 25, +40 - Z_VIEW_CORRECT, 0 };
	glLightfv(GL_LIGHT1, GL_POSITION, pos2);
	static GLfloat pos4[4] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, pos4);
	glEnable(GL_LIGHT1);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

// transparency
//      glEnable(GL_BLEND);
//      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	myatomsphere = glGenLists(1);
	glNewList(myatomsphere, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, red);

	quadr = gluNewQuadric();
	gluQuadricDrawStyle(quadr, GLU_FILL);
	gluSphere(quadr, 1, 10, 10);
	gluDeleteQuadric(quadr);

// MyAtomSphere( 1, 30, 30);
	glEndList();

	unitbond = glGenLists(1);
	glNewList(unitbond, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
//  UnitBond( 1, 0.4, 30 );
	quadr = gluNewQuadric();
	gluQuadricDrawStyle(quadr, GLU_FILL);
	gluDisk(quadr, 0, 1, 10, 4);
	gluCylinder(quadr, 1, 1, 1, 10, 10);
	glTranslatef(0, 0, 1);
	gluDisk(quadr, 0, 1, 10, 4);
	gluDeleteQuadric(quadr);
	glEndList();

	unitbond2 = glGenLists(1);
	glNewList(unitbond2, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
	quadr = gluNewQuadric();
	gluQuadricDrawStyle(quadr, GLU_FILL);
	gluDisk(quadr, 0, 1, 10, 4);
	gluCylinder(quadr, 1, 1, 1, 10, 10);
	glTranslatef(0, 0, 1);
	gluDisk(quadr, 0, 1, 10, 4);
	gluDeleteQuadric(quadr);
	glEndList();

	yellowbond = glGenLists(1);
	glNewList(yellowbond, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
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

//                                      x_mol[4] = event.xmotion.x * 0.01;
//                                      y_mol[4] = event.xmotion.y * 0.01;

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
					//int r;
					int code;
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
						//r = XLookupString(&event.xkey,buffer,sizeof(buffer),NULL, NULL);
						if (buffer[0] == 27) {
							/* escape */
							return;
						}
						/*if (buffer[0] == 'a') {
						   x_mol[3] += 0.5;
						   }
						   if (buffer[0] == 's') {
						   x_mol[3] -= 0.5;
						   } */

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

			//sprintf(buf,"%.1f %.1f   %.1f", hand1_z,hand2_z, hand1_z - hand2_z);
			// XDrawLine(dpy,win,gc, 200, 200, globmousex, globmousey);
			XDrawString(dpy, win, gc, 20, 20, buf, strlen(buf));

			glXSwapBuffers(dpy, win);

//                      XDrawString(dpy, win, gc, 20, 20, buf, strlen(buf));

//needredraw = 0;

		}		//end while XPending(dpy)
	}			//end while(1)
}				// end event_loop;

/*

what to do if:
[xcb] Unknown sequence number while processing reply
[xcb] Most likely this is a multi-threaded client and XInitThreads has not been called
[xcb] Aborting, sorry about that.
a.out: ../../src/xcb_io.c:634: _XReply: Assertion `!xcb_xlib_th9reads_sequence_lo

or typically
a.out: ../../src/xcb_io.c:528: _XAllocID: Assertion `ret != inval_id' failed.
Aborted (core dumped)


*/

void *molecular_threadfunc_mol(void *arg)
{
	mol_thread_struct *molinfo = (mol_thread_struct *) arg;
	printf("Passed to molview thread:\n%s %c %c\n", molinfo->pdb_file,
	       molinfo->chain_one, molinfo->chain_two);

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

	fill_atom_coord_new(molinfo);

	dpy = XOpenDisplay(dpyName);
	if (!dpy) {
		printf("Error: couldn't open display %s\n", dpyName);
		//return -1;
		return NULL;	//"return -1;" was giving an error, going to try this...
	}
//glutInit(NULL, NULL);
	if (ML_HIDE_MOL_WINDOW) {
		return NULL;
	}
	make_window(dpy, "Protein Viewer", 400, 0, 900, 900, &win, &ctx);

	screen = DefaultScreen(dpy);
	bg = BlackPixel(dpy, screen);
	fg = WhitePixel(dpy, screen);
	gcval.foreground = fg;
	gcval.background = bg;
	gc = XCreateGC(dpy, win, GCForeground | GCBackground, &gcval);

//  if ( (font_info = XLoadQueryFont(dpy, "-*-helvetica-*-r-*-*-*-180-*-*-*-*-*-*")) == NULL)

	if ((font_info =
	     XLoadQueryFont(dpy,
			    "-bitstream-courier 10 pitch-bold-r-normal--0-0-0-0-m-0-ascii-0"))
	    == NULL) {

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
