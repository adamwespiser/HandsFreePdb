pthread_t molecular_thread_mol;
pthread_t molecular_thread;
pthread_t freenect_thread;

int window;

pthread_mutex_t gl_backbuf_mutex;

// back: owned by libfreenect (implicit for depth)
// mid: owned by callbacks, "latest frame ready"  
// front: owned by GL, "currently being drawn"  
uint8_t *depth_mid, *depth_front;
uint8_t *rgb_back, *rgb_mid, *rgb_front;
float *subbuffer;
float *subbuffer2;
float *subbuffer3;
float *subbuffer4;
int *subbuffer5;
GLuint gl_depth_tex;
GLuint gl_rgb_tex;

freenect_context *f_ctx;
freenect_device *f_dev;
int freenect_angle;
int freenect_led;

freenect_video_format requested_format;
freenect_video_format current_format;

freenect_led_options current_kinect_led;
freenect_led_options requested_kinect_led;

pthread_cond_t gl_frame_cond;
int got_rgb;
int got_depth;

uint16_t t_gamma[2048];
