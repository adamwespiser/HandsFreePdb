#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>


int imgx=640, imgy = 480, templx=150, temply = 150;

void sq_diff(unsigned char *img, unsigned char *templ, float *res)
{
float d,s;
int i,j, ofsx, ofsy;

for(ofsy=0; ofsy<(imgy-temply-1); ofsy++)
for(ofsx=0; ofsx<(imgx-templx-1); ofsx++)
//for(ofsy=0; ofsy<(imgy-temply-1); ofsy++)
{

s =0;
for(j=0;j<temply;j++)
for(i=0;i<templx;i++)
//for(j=0;j<temply;j++)
{
d = templ[j*templx+i] - img[ (j+ofsy)*imgx + ofsx  ];
s+=d*d;
}

res[ ofsy * templx + ofsx  ] = s;

}


} 

int main(void)
{
unsigned char *img, *templ;
float *r;
double dt;
clock_t t1, t2;

img = (unsigned char *)malloc( imgx*imgy*sizeof(unsigned char));
templ=(unsigned char *)malloc( templx*temply*sizeof(unsigned char));

r = (float *)malloc( imgx*imgy*sizeof(float));

t1 = clock();

sq_diff(img, templ, r);

t2 = clock();
dt = ((double)(t2-t1))/CLOCKS_PER_SEC;
printf("%f s\n",dt/3);


}

