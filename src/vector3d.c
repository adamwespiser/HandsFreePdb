#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<stdint.h>

#include"vector3d.h"

vector3d init_zero_vector()
{
	vector3d zero = { 0.0, 0.0, 0.0 };
	return zero;
}

vector3d midpoint_vector(vector3d v, vector3d n)
{
	vector3d midpoint =
	    { (v.x + n.x) / 2, (v.y + n.y) / 2, (v.z + n.z) / 2 };
	return midpoint;
}

vector3d single_frame_velocity(vector3d t0, vector3d t1, int frame_rate)
{

	float x = (t1.x - t0.x) * frame_rate;
	float y = (t1.y - t0.y) * frame_rate;
	float z = (t1.z - t0.z) * frame_rate;
	vector3d v = { x, y, z };
	return v;
}

vector3d scalar_multiply(vector3d v, float a)
{
	vector3d vec = { v.x * a, v.y * a, v.z * a };
	return vec;
}

vector3d vector_add_scalar(vector3d a, float scalar)
{
	vector3d v = { a.x + scalar, a.y + scalar, a.z + scalar };
	return v;
}

vector3d vector_subtract(vector3d a, vector3d b)
{
	vector3d v = { a.x - b.x, a.y - b.y, a.z - b.z };
	return v;
}

vector3d unit_vector(vector3d v)
{
	float mag = vector_magnitude(v);
    vector3d v_unit; 
    if (mag != 0){ 
	v_unit.x =  v.x / mag;
	v_unit.y =  v.y / mag;
	v_unit.z =  v.z / mag;
    }
    else {

	v_unit.x =  0;
	v_unit.y = 0;
	v_unit.z =0;
    }
	return v_unit;
}

vector3d projection_a_on_b(vector3d a, vector3d b)
{
    float dot_bb = dot_product(b,b); 
    if (dot_bb != 0){
    	return scalar_multiply(b, (dot_product(a, b) / dot_bb));
    }else {
     return init_zero_vector(); 
    }
}

vector3d vector_from_coord_pair(vector3d a, vector3d b)
{
	vector3d ab = { a.x - b.x, a.y - b.y, a.z - b.z };
	return ab;

}

float component_a_on_b(vector3d a, vector3d b)
{
    float vb = vector_magnitude(b); 
    if (vb != 0){
	return (dot_product(a, b) / vector_magnitude(b));
    } 
    else 
    return 0; 
}

float dot_product(vector3d v, vector3d n)
{
	return (v.x * n.x + v.y * n.y + v.z * n.z);
}

float vector_magnitude(vector3d v)
{

	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

int vector_mag_below_threshold(vector3d v, float threshold)
{
	float mag_2 = (v.x * v.x + v.y * v.y + v.z * v.z);
	// compare the threshold squared to avoid having to take a sqrt
	return (mag_2 < (threshold * threshold)) ? 1 : 0;
}

vector3d vector_crossproduct(vector3d a, vector3d b){
/*
    vector3d v; 
    v.x = a.y * b.z - a.z * b.y; 
    v.y = a.z * b.x - a.x * b.z; 
    v.z = a.x * b.y - a.y * b.x; 
    return v; 
    a.x = a.x + 100;
    a.y = a.y + 100;
    a.z = a.z + 100;

    b.x = b.x + 100;
    b.y = b.y + 100;
    b.z = b.z + 100;
*/

    vector3d v = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; 
    return v; 
}


