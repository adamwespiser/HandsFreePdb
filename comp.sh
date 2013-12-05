#KININCDIR=/home/zeld/work/OpenKinect-libfreenect-2402fcd/include 
#KINLIBDIR=/home/zeld/work/OpenKinect-libfreenect-2402fcd/build/lib
#kinlibdir also included in /etc/ld.so.conf

KININCDIR=/home/adam/bin/libfreenect/include
KINLIBDIR=/home/adam/bin/libfreenect/build/lib/

GLLIBS="-lGLU -lGL -lglut -lpthread -lX11 -lfreenect"
SRCS="./src/main.c ./src/kinect_window.c ./src/molview.c ./src/vector3d.c ./src/tracking.c ./src/gesture.c ./src/control_mode.c ./src/pdblib.c ./src/segmenter.c"
gcc -Wall -g $SRCS -lm $GLLIBS -I$KININCDIR -L$KINLIBDIR -I./include -o kineprot
