#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define N 420 /* frame dimension for QCIF format */
#define M 280 /* frame dimension for QCIF format */

#define filename "bus_420x280.yuv"
#define file_yuv "test.yuv"

/* code for armulator*/
// declare variables
#pragma arm section zidata="ram"
int current[M][N];
int output_gaussian[M][N];
int Eo[M][N];
int Es[M][N];
int Igx[M][N];
int Igy[M][N];
#pragma arm section

int i, j, newPixel, row, col, kernelRow, kernelCol;
int gaussianMask[5][5];
int GxMask[3][3];
int GyMask[3][3];
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
      fputc(Es[i][j],frame_yuv);
    }
  }
 
  fclose(frame_yuv);
}

// ===================================================== 
void canny()
{

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
			output_gaussian[row][col] = (int)(round(newPixel)/159);
		}
	}
	
}

void sobel()
{
	/* Declare Sobel masks */
	GxMask[0][0] = -1; GxMask[0][1] = 0; GxMask[0][2] = 1;
	GxMask[1][0] = -2; GxMask[1][1] = 0; GxMask[1][2] = 2;
	GxMask[2][0] = -1; GxMask[2][1] = 0; GxMask[2][2] = 1;
	
	GyMask[0][0] =  1; GyMask[0][1] =  2; GyMask[0][2] =  1;
	GyMask[1][0] =  0; GyMask[1][1] =  0; GyMask[1][2] =  0;
	GyMask[2][0] = -1; GyMask[2][1] = -2; GyMask[2][2] = -1;
	
	int limit=1;
	
	/* This is for Gx */
	for (row=limit;row<=(M-limit);row++){
		for (col=limit;col<=(N-limit);col++){
			newPixel = 0;
			for (kernelRow=-limit; kernelRow<=limit; kernelRow++){
				for (kernelCol=-limit;kernelCol<=limit; kernelCol++){
					newPixel = newPixel + output_gaussian[row+kernelRow][col+kernelCol]*GxMask[limit+kernelRow][limit+kernelCol];
				}
			}
			Igx[row][col] = (int)(round(newPixel));
		}
	}
	
	/* This is for Gy */
	for (row=limit;row<=(M-limit);row++){
		for (col=limit;col<=(N-limit);col++){
			newPixel = 0;
			for (kernelRow=-limit; kernelRow<=limit; kernelRow++){
				for (kernelCol=-limit;kernelCol<=limit; kernelCol++){
					newPixel = newPixel + output_gaussian[row+kernelRow][col+kernelCol]*GyMask[limit+kernelRow][limit+kernelCol];
				}
			}
			Igy[row][col] = (int)(round(newPixel));
		}
	}
	
	int a,b,c;
	/* This is for Es */
	for (row=limit;row<=(M-limit);row++){
		for (col=limit;col<=(N-limit);col++){
			a=pow(Igx[row][col], 2.0);
			b=pow(Igy[row][col], 2.0);
			c=a+b;				
			Es[row][col] = sqrt(c);
			
			if(Es[row][col]>=255)
			{
				Es[row][col] = 255;
			}
			printf("%d ",Es[row][col]);
		}
	}
	
	/* This is for Eo */
	for (row=limit;row<=(M-limit);row++){
		for (col=limit;col<=(N-limit);col++){					
			Eo[row][col] = atan(Igy[row][col]/Igy[row][col]);
		}
	}
	
	
	
}

int main() {

  read();

  canny();
  
  sobel();
  
  write();

  
}