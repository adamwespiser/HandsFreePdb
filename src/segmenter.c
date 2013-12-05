#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<stdint.h>

#include "constants.h"

#define N 10000

#define MAXNEIGHB 8

/*
*  segementer(integer array, subsamp size) 
*  accepts: an array with the following characteristics: 
*    1) values corresponding to potential segmentation must be equal to 1
*    2) all other values must be 0
*  returns: void, 
*    1) mutates the input array, replacing all values of '1' with a segmentation index, 
*       of integer value greater than 2.  For a given segmentation index of value > 1, 
*       all array entries with that value are neighbours 
*
*
*/

int segmenter(int *segment_array, int subsamp)
{

	int x, y;
	int idx_end, idx_start, i, j;
	int foundnewpoints;
	int x0, y0, c0, xmax, ymax;

	int pointx[(640 * 480) / subsamp];
	int pointy[(640 * 480) / subsamp];
	int pointcolor[(640 * 480) / subsamp];

	xmax = 640 / subsamp;
	ymax = 480 / subsamp;

//the 8 neighboring pixels
	int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

// mask contains 0/1 before segmentation
// after segmentation, segments are marked 2,3...
	int segmentindex = 1;
	int idx_max = 0;

	do {

//find a pixel of color 1 (not yet segmented)
//and mark it for BFS coloring
		int flag = 0;
		for (j = 0; j < ymax && flag == 0; j++) {
			for (i = 0; i < xmax && flag == 0; i++) {
				int idx = j * xmax + i;
				if (segment_array[idx] == 1) {
					flag = 1;
					x = i;
					y = j;
				}
			}
		}

		if (flag == 1) {
			segmentindex++;
		} else {
			if (SEG_PRINT_SEG_INDEX)
                printf("max segment index = %i\n", segmentindex - 1);
			break;
		}
		pointx[idx_max] = x;
		pointy[idx_max] = y;
		pointcolor[idx_max] = segmentindex;

		int idx = y * xmax + x;
		segment_array[idx] = segmentindex;

		idx_start = idx_max;
		idx_max++;
		idx_end = idx_max;

//find all pixels continuously connected to (x,y)

		do {

			foundnewpoints = 0;

//go over the most recently added pixels
			//over the added pixels    
			for (i = idx_start; i < idx_end; i++) {

				//take a pixel
				x0 = pointx[i];
				y0 = pointy[i];
				c0 = pointcolor[i];

				//over the taken pixel's neighbours
				// 8 is the number of neighbours(constants)
				for (j = 0; j < 8; j++) {

					int x_neigh = x0 + dx[j];
					int y_neigh = y0 + dy[j];

					//check to make sure we aren't going over
					if (-1 < x_neigh && x_neigh < xmax
					    && -1 < y_neigh && y_neigh < ymax) {
						int idx =
						    y_neigh * xmax + x_neigh;
						if (segment_array[idx] == 1) {	// non-segmented, adjacent pixel

							//push on the stack as "recent" and paint 
							pointx[idx_max] =
							    x_neigh;
							pointy[idx_max] =
							    y_neigh;
							pointcolor[idx_max] =
							    c0;
							segment_array[idx] = c0;
							idx_max++;
							foundnewpoints = 1;
						}

					}

				}	// end for j

			}	// end for i

			idx_start = idx_end;
			idx_end = idx_max;

		} while (foundnewpoints);

	} while (1);

	return segmentindex;
}

void findlargestsegs(int *segment_array, int subsamp, int nseg)
{
	int segsize[nseg + 1];
	int i, j;

	for (i = 0; i < nseg + 1; i++)
		segsize[i] = 0;

	for (i = 0; i < 640 * 480 / (subsamp * subsamp); i++)
		if (segment_array[i] > 0)
			segsize[segment_array[i]]++;

	int max1 = -1;
	int maxidx1 = -1;
	int max2 = -1;
	int maxidx2 = -1;
	float x1 = 0, x2 = 0;
	for (i = 2; i < nseg + 1; i++)
		if (segsize[i] > max1) {
			max1 = segsize[i];
			maxidx1 = i;
		}

	for (i = 2; i < nseg + 1; i++)
		if (segsize[i] > max2 && segsize[i] != max1) {
			max2 = segsize[i];
			maxidx2 = i;
		}

	for (i = 0; i < 640 * 480 / (subsamp * subsamp); i++) {
		if (segment_array[i] == maxidx1)
			x1 = x1 + (i % (640 / subsamp));
		if (segment_array[i] == maxidx2)
			x2 = x2 + (i % (640 / subsamp));
	}

	x1 /= max1;
	x2 /= max2;
	int idx1, idx2;
	if (x2 < x1) {
		idx1 = 2;
		idx2 = 1;
	} else {
		idx1 = 1;
		idx2 = 2;
	}

	for (i = 0; i < 640 * 480 / (subsamp * subsamp); i++) {
		if (segment_array[i] == maxidx1)
			j = idx1;
		else if (segment_array[i] == maxidx2)
			j = idx2;
		else
			j = 0;

		segment_array[i] = j;

	}

}

void expandmask(int *segment_array, int subsamp)
{

	int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	int x, y, i, idx, idx2, xx, yy;

	for (y = 0; y < 480 / subsamp; y++)
		for (x = 0; x < 640 / subsamp; x++) {
			idx = x + y * (640 / subsamp);

			for (i = 0; i < 9; i++) {
				xx = x + dx[i];
				yy = y + dy[i];
				idx2 =
				    x + dx[i] + (y + dy[i]) * (640 / subsamp);

				if (-1 < xx && xx < 640 / subsamp - 1 && -1 < yy
				    && yy < 480 / subsamp - 1
				    && segment_array[idx] == 1
				    && segment_array[idx2] == 0)
					segment_array[idx2] = 3;

				if (-1 < xx && xx < 640 / subsamp - 1 && -1 < yy
				    && yy < 480 / subsamp - 1
				    && segment_array[idx] == 2
				    && segment_array[idx2] == 0)
					segment_array[idx2] = 4;
			}

		}

	for (i = 0; i < 640 * 480 / (subsamp * subsamp); i++) {

		if (segment_array[i] == 3)
			segment_array[i] = 1;
		if (segment_array[i] == 4)
			segment_array[i] = 2;

	}

}

int findhgramcutoff(int *segment_array, uint16_t * depth, int subsamp,
		    int handidx)
{
	int i, j, idx1, idx2;
	int hgram[4096];
	for (i = 0; i < 4096; i++)
		hgram[i] = 0;

	for (j = 0; j < 480; j++)
		for (i = 0; i < 640; i++) {

			idx1 = i + 640 * j;
			idx2 = i / subsamp + (j / subsamp) * (640 / subsamp);

			if (segment_array[idx2] == handidx)
				hgram[(int)depth[idx1]]++;

		}

	int s = 0;
	for (i = 0; i < 4096; i++)
		s += hgram[i];

	int scutoff = s * 0.4;
	s = 0;
	for (i = 0; i < 4096; i++) {
		s += hgram[i];
		if (s > scutoff) {
			j = i;
			break;
		};
	}

	return j;
}
