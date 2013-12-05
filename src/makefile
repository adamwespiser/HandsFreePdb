P=../bin/kineprot
LDLIBS= -lm -lGLU -lGL -lglut -lpthread -lX11 -lfreenect
SOURCES=main.c kinect_window.c molview.c vector3d.c tracking.c gesture.c control_mode.c pdblib.c segmenter.c 

KININCDIR=/home/adam/bin/libfreenect/include
KINLIBDIR=/home/adam/bin/libfreenect/build/lib

CC=gcc

IDIR =../include
_DEPS=constants.h control_mode.h kinect_vars.h segmenter.h vector3d.h control.h gesture.h pdblib.h tracking.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

CFLAGS= -Wall -g -I$(IDIR) -I.

ODIR=../obj
LDIR =../lib
_OBJ= kinect_window.o gesture.o control_mode.o molview.o pdblib.o tracking.o vector3d.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -lm $(LDLIBS) -I$(KININCDIR) -L$(KINLIBDIR) 

$(P): $(OBJ)
	$(CC) $(CFLAGS) $(SOURCES) -lm $(LDLIBS) -I$(KININCDIR) -L$(KINLIBDIR) -o $(P)

.PHONY: clean

clean:
	rm -f ../bin/kineprot $(ODIR)/*.o *~ core $(INCDIR)/*~ 
