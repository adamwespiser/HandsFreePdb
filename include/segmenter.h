#include "constants.h"


int segmenter(int * segment_array, int subsamp);
void findlargestsegs(int * segment_array, int subsamp, int nseg);
void expandmask(int *segment_array, int subsamp);


int findhgramcutoff(int *segment_array, uint16_t *depth, int subsamp, int hand); 
