typedef struct v {
	float x;
	float y;
	float z;
} vector3d;

vector3d init_zero_vector();
vector3d midpoint_vector(vector3d v, vector3d n);
vector3d single_frame_velocity(vector3d t0, vector3d t1, int frame_rate);
vector3d unit_vector(vector3d v);
vector3d projection_a_on_b(vector3d a, vector3d b);
vector3d vector_from_coord_pair(vector3d a, vector3d b);
vector3d vector_subtract(vector3d a, vector3d b);
vector3d vector_add_scalar(vector3d a, float scalar);
vector3d vector_crossproduct(vector3d a, vector3d b); 
vector3d scalar_multiply(vector3d v, float a);
float component_a_on_b(vector3d a, vector3d b);
float dot_product(vector3d v, vector3d n);
float vector_magnitude(vector3d v);

int vector_mag_below_threshold(vector3d v, float threshold);
