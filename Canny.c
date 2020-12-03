#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define N 144 /* frame dimension for QCIF format */
#define M 176 /* frame dimension for QCIF format */
#define s 2   /*scaling factor*/

#define W 144 /* width of current frame [pixels] */
#define H 176 /* height of current frame [pixels] */

/* code for armulator*/
// declare variables
#pragma arm section zidata="ram"
int data[N][M];
double conV[N][M],conH[N][M],con2[N][M];
int final[N*s][M*s];
#pragma arm section

// declare more variables

int i,j,k,kernelRow,kernelCol;
unsigned int row, col;	// Pixel's row and col positions
int rowOffset;			// Row offset from the current pixel
int colOffset;			// Col offset from the current pixel
int kernelSize = 5;		// Size of Gaussin blur kernel
int SxMask[3][3];		// Sobel mask in the x direction
int SyMask[3][3];		// Sobel mask in the y direction
int gaussianMask[5][5];	// Gaussian mask
int newPixel;			// Sum pixel values for gaussian
int output[W][H];		// Output image from every step

// read image
void read()
{
  FILE *frame_c;
  if((frame_c=fopen("akiyo1.y","rb"))==NULL)
  {
    printf("current frame doesn't exist\n");
    exit(-1);
  }

  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j++)
    {
      data[i][j]=fgetc(frame_c);
    }
  }
}

void canny() {
	
	/* Declare Sobel masks */
	SxMask[0][0] = -1; SxMask[0][1] = 0; SxMask[0][2] = 1;
	SxMask[1][0] = -2; SxMask[1][1] = 0; SxMask[1][2] = 2;
	SxMask[2][0] = -1; SxMask[2][1] = 0; SxMask[2][2] = 1;
	
	SyMask[0][0] =  1; SyMask[0][1] =  2; SyMask[0][2] =  1;
	SyMask[1][0] =  0; SyMask[1][1] =  0; SyMask[1][2] =  0;
	SyMask[2][0] = -1; SyMask[2][1] = -2; SyMask[2][2] = -1;
	
	/* Declare Gaussian mask */
	gaussianMask[0][0] = 2;	 gaussianMask[0][1] = 4;  gaussianMask[0][2] = 5;  gaussianMask[0][3] = 4;  gaussianMask[0][4] = 2;	
	gaussianMask[1][0] = 4;	 gaussianMask[1][1] = 9;  gaussianMask[1][2] = 12; gaussianMask[1][3] = 9;  gaussianMask[1][4] = 4;	
	gaussianMask[2][0] = 5;	 gaussianMask[2][1] = 12; gaussianMask[2][2] = 15; gaussianMask[2][3] = 12; gaussianMask[2][4] = 2;	
	gaussianMask[3][0] = 4;	 gaussianMask[3][1] = 9;  gaussianMask[3][2] = 12; gaussianMask[3][3] = 9;  gaussianMask[3][4] = 4;	
	gaussianMask[4][0] = 2;	 gaussianMask[4][1] = 4;  gaussianMask[4][2] = 5;  gaussianMask[4][3] = 4;  gaussianMask[4][4] = 2;	
	
	/* Gaussian blur */
	int limit = (int) floor(kernelSize/2);
	for (row = limit; row < (W-limit); row++) {
		for (col = limit; col < (W-limit); col++) {
			newPixel = 0;
			for (kernelRow=-limit;kernelRow<limit;kernelRow++) {
				for (kernelCol=-limit;kernelCol<limit;kernelCol++) {
					
					newPixel = newPixel + gaussianMask[limit+kernelRow,limit+kernelCol]*data[row+kernelRow,col+kernelCol];
				}
			}
		output[row,col] = (int)(round(newPixel/kernelWeight));
		}
	}
	
	/* Find gradients using Sobel masks */
	
}
