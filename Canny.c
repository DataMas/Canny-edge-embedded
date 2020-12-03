#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define N 420 /* frame dimension for QCIF format */
#define M 280 /* frame dimension for QCIF format */
//#define s 2   /*scaling factor*/

#define filename "bus_420x280.yuv"
#define file_yuv "test_420x280.yuv"

/* code for armulator*/
// declare variables
#pragma arm section zidata="ram"
int current[M][N];
int output[M][N];

#pragma arm section

int i, j, newPixel, row, col, kernelRow, kernelCol;
int gaussianMask[5][5];
int kernelSize = 5;

// ===================================================== 
// read image
void read()
{
  FILE *frame_c;
  if((frame_c=fopen(filename,"rb"))==NULL)
  {
    printf("current frame doesn't exist\n");
    exit(-1);
  }

  for(i=0;i<M;i++)
  {
    for(j=0;j<N;j++)
    {
      current[i][j]=fgetc(frame_c);
    }
  }

  fclose(frame_c);
}

//write image
void write()
{
  FILE *frame_yuv;
  frame_yuv=fopen(file_yuv,"wb");

  for(i=0;i<M;i++)
  {
    for(j=0;j<N;j++)
    {
      fputc(output[i][j],frame_yuv);
    }
  }
 
  fclose(frame_yuv);
}

// ===================================================== 
void canny()
{
	printf("ok");
	// Declare Gaussian mask 
	gaussianMask[0][0] = 2;	 gaussianMask[0][1] = 4;  gaussianMask[0][2] = 5;  gaussianMask[0][3] = 4;  gaussianMask[0][4] = 2;	
	gaussianMask[1][0] = 4;	 gaussianMask[1][1] = 9;  gaussianMask[1][2] = 12; gaussianMask[1][3] = 9;  gaussianMask[1][4] = 4;	
	gaussianMask[2][0] = 5;	 gaussianMask[2][1] = 12; gaussianMask[2][2] = 15; gaussianMask[2][3] = 12; gaussianMask[2][4] = 2;	
	gaussianMask[3][0] = 4;	 gaussianMask[3][1] = 9;  gaussianMask[3][2] = 12; gaussianMask[3][3] = 9;  gaussianMask[3][4] = 4;	
	gaussianMask[4][0] = 2;	 gaussianMask[4][1] = 4;  gaussianMask[4][2] = 5;  gaussianMask[4][3] = 4;  gaussianMask[4][4] = 2;	
	
	int limit = (int) floor(kernelSize/2);
	
	for (row=limit;row<=(M-limit);row++){
		for (col=limit;col<=(N-limit);col++){
			newPixel = 0;
			//printf("%d",row);
			for (kernelRow=-limit; kernelRow<=limit; kernelRow++){
				for (kernelCol=-limit;kernelCol<=limit; kernelCol++){
					newPixel = newPixel + current[row+kernelRow][col+kernelCol]*gaussianMask[limit+kernelRow][limit+kernelCol];
				}
			}
			output[row][col] = (int)(round(newPixel)/159);
		}
	}
	
printf("ok");
}

int main() {

  read();

  canny();
  
  write();

  
}
