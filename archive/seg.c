
#include<stdio.h>
#include<string.h>

#define N 10000

int idxmax = 0;
int pointx[N], pointy[N], pointcolor[N];

int img[100][100];
int xmax, ymax;

int getpixel(int x, int y)
{
	return img[x][y];
}

int setpixel(int x, int y, int c)
{
	img[x][y] = c;
}

void printgrid(void)
{
	int i, j;
	printf("   ");
	for (i = 0; i < xmax; i++)
		printf("%d", i % 10);
	printf("\n");
	for (j = 0; j < ymax; j++) {
		printf("%2d ", j);
		for (i = 0; i < xmax; i++) {
			char c;
			if (img[i][j] == 0)
				c = '.';
			else
				c = '0' + img[i][j];
			printf("%c", c);
		}
		printf("\n");
	}
}

void segmenter(void)
{

	int x, y;
	int idx_end, idx_start, i, j;
	int foundnewpoints;
	int x0, y0, c0, xx, yy;

//the 8 neighboring pixels
#define MAXNEIGHB 8
	int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

// mask contains 0/1 before segmentation
// after segmentation, segments are marked 2,3...
	int segmentindex = 1;

	int idxmax = 0;

	do {

//find a pixel of color 1 (not yet segmented)
		int flag = 0;
		for (j = 0; j < ymax && flag == 0; j++) {
			for (i = 0; i < xmax && flag == 0; i++) {
				if (getpixel(i, j) == 1) {
					flag = 1;
					x = i;
					y = j;
				}
			}
		}

		if (flag) {
			segmentindex++;
		} else
			break;

		pointx[idxmax] = x;
		pointy[idxmax] = y;
		pointcolor[idxmax] = segmentindex;
		setpixel(x, y, segmentindex);

		idx_start = idxmax;
		idxmax++;
		idx_end = idxmax;

//find all pixels continuously connected to (x,y)

		do {

			foundnewpoints = 0;

//go over the most recently added pixels
			for (i = idx_start; i < idx_end; i++) {

				x0 = pointx[i];
				y0 = pointy[i];
				c0 = pointcolor[i];

				for (j = 0; j < MAXNEIGHB; j++) {

					xx = x0 + dx[j];
					yy = y0 + dy[j];

					if (-1 < xx && xx < xmax && -1 < yy
					    && yy < ymax) {
						if (getpixel(xx, yy) == 1) {	// non-segmented, adjacent pixel

							//push on the stack as "recent" and paint 
							pointx[idxmax] = xx;
							pointy[idxmax] = yy;
							pointcolor[idxmax] = c0;
							setpixel(xx, yy, c0);
							idxmax++;
							foundnewpoints = 1;
						}

					}

				}	// end for j

			}	// end for i

			idx_start = idx_end;
			idx_end = idxmax;

		} while (foundnewpoints);

	} while (1);

}

int main(void)
{
	int x, y, i, j;
	char s[100];

	y = 0;
	FILE *f = fopen("seg.txt", "r");
	while (!feof(f)) {
		fgets(s, 100, f);
		xmax = strlen(s) - 1;
		for (i = 0; i < xmax; i++) {
			if (s[i] == '.')
				img[i][y] = 0;
			else
				img[i][y] = 1;
		}
		y++;
	}
	fclose(f);
	ymax = y - 1;

	printgrid();

	segmenter();

	printf("\n\n");

	printgrid();

}

int idxmax = 0;
int pointx[N], pointy[N], pointcolor[N];

int img[100][100];
int xmax, ymax;

int getpixel(int x, int y)
{
	return img[x][y];
}

int setpixel(int x, int y, int c)
{
	img[x][y] = c;
}

void printgrid(void)
{
	int i, j;
	printf("   ");
	for (i = 0; i < xmax; i++)
		printf("%d", i % 10);
	printf("\n");
	for (j = 0; j < ymax; j++) {
		printf("%2d ", j);
		for (i = 0; i < xmax; i++) {
			char c;
			if (img[i][j] == 0)
				c = '.';
			else
				c = '0' + img[i][j];
			printf("%c", c);
		}
		printf("\n");
	}
}
#define MAXNEIGHB 8

void segmenter(int * segment_array, int subsamp)
{

	int x, y;
	int idx_end, idx_start, i, j;
	int foundnewpoints;
	int x0, y0, c0, xx, yy;
    int xmax; 
    int ymax;
    xmax = 640/subsamp; 
    ymax = 480/subsamp; 

//the 8 neighboring pixels
	int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

// mask contains 0/1 before segmentation
// after segmentation, segments are marked 2,3...
	int segmentindex = 1;
	int idx_max = 0;

	do {

//find a pixel of color 1 (not yet segmented)
		int flag = 0;
		for (j = 0; j < ymax && flag == 0; j++) {
			for (i = 0; i < xmax && flag == 0; i++) {
                int idx = j * ymax + x; 
				if (segment_array[idx] == 1) {
					flag = 1;
					x = i;
					y = j;
				}
			}
		}

		if (flag) {
			segmentindex++;
		} else
			break;

		pointx[idx_max] = x;
		pointy[idx_max] = y;
		pointcolor[idx_max] = segmentindex;
        int idx = y * ymax + x; 
		segment_array[idx]=segmentindex;

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
				for (j = 0; j < MAXNEIGHB; j++) {

					x_neigh = x0 + dx[j];
					y_neigh = y0 + dy[j];

                    //check to make sure we aren't going over
					if (-1 < x_neigh && x_neigh < xmax && -1 < y_neigh
					    && y_neigh < ymax) {
                        int idx = y_neigh * ymax + x_neigh;
						if (segment_array[idx] == 1) {	// non-segmented, adjacent pixel

							//push on the stack as "recent" and paint 
							pointx[idx_max] = x_neigh;
							pointy[idx_max] = y_neigh;
							pointcolor[idx_max] = c0;
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

}
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

void segmenterT(int * segment_array, int subsamp)
{

	int x, y;
	int idx_end, idx_start, i, j;
	int foundnewpoints;
	int x0, y0, c0, xmax, ymax;
    int pointx[(640*480)/subsamp]; 
    int pointy[(640*480)/subsamp]; 
    int pointcolor[(640*480)/subsamp]; 

    xmax = 640/subsamp; 
    ymax = 480/subsamp; 

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

		if (flag) {
			segmentindex++;
		} else
            printf("max segment index = %i\n",segmentindex - 1); 
			break;

		pointx[idx_max] = x;
		pointy[idx_max] = y;
		pointcolor[idx_max] = segmentindex;

        int idx = y * xmax + x; 
		segment_array[idx]=segmentindex;

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
				for (j = 0; j < MAXNEIGHB; j++) {

					int x_neigh = x0 + dx[j];
					int y_neigh = y0 + dy[j];

                    //check to make sure we aren't going over
					if (-1 < x_neigh && x_neigh < xmax && -1 < y_neigh
					    && y_neigh < ymax) {
                        int idx = y_neigh * xmax + x_neigh;
						if (segment_array[idx] == 1) {	// non-segmented, adjacent pixel

							//push on the stack as "recent" and paint 
							pointx[idx_max] = x_neigh;
							pointy[idx_max] = y_neigh;
							pointcolor[idx_max] = c0;
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

}

