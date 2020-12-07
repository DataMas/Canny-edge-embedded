#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define N 368 /* frame dimension for QCIF format */
#define M 276 /* frame dimension for QCIF format */

#define filename "house_368x276.yuv"
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
int Edges[M][N];
int searched[M][N]={0};
int limit = 1;
int limit_gauss = 2;
int T1=230;
int T2=245;
#pragma arm section

int i, j, newPixel, row, col, kernelRow, kernelCol;
int gaussianMask[5][5];
int GxMask[3][3];
int GyMask[3][3];
int kernelSize = 5;

int NonMax[M][N];

int *s = &Es[0][0];
int *o = &Eo[0][0];
int *igx = &Igx[0][0];
int *igy = &Igy[0][0];
int temp_current;
int temp_output;



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
      fputc(Edges[i][j],frame_yuv);
    }
  }
 
  fclose(frame_yuv);
}

// ===================================================== 
void canny()
{

	// Declare Gaussian mask 
	gaussianMask[0][0] = 0;	 gaussianMask[0][1] = 0;  gaussianMask[0][2] = 0;  gaussianMask[0][3] = 0;  gaussianMask[0][4] = 0;	
	gaussianMask[1][0] = 0;	 gaussianMask[1][1] = 1;  gaussianMask[1][2] = 2; gaussianMask[1][3] = 1;  gaussianMask[1][4] = 0;	
	gaussianMask[2][0] = 0;	 gaussianMask[2][1] = 2; gaussianMask[2][2] = 4; gaussianMask[2][3] = 2; gaussianMask[2][4] = 0;	
	gaussianMask[3][0] = 0;	 gaussianMask[3][1] = 1;  gaussianMask[3][2] = 2; gaussianMask[3][3] = 1;  gaussianMask[3][4] = 0;	
	gaussianMask[4][0] = 0;	 gaussianMask[4][1] = 0;  gaussianMask[4][2] = 0;  gaussianMask[4][3] = 0;  gaussianMask[4][4] = 0;	
	

	for (row=limit_gauss;row<=(M-limit_gauss);row++){
		for (col=limit_gauss;col<=(N-limit_gauss);col++){
			newPixel = 0;
			//printf("%d",row);
			
			for (kernelRow=-limit_gauss; kernelRow<=limit_gauss; kernelRow++){
				for (kernelCol=-limit_gauss;kernelCol<=limit_gauss; kernelCol++){
					newPixel = newPixel + current[row+kernelRow][col+kernelCol]*gaussianMask[limit_gauss+kernelRow][limit_gauss+kernelCol];
				}
			}
			output_gaussian[row][col] = (int)(floor(newPixel)/16);
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
	
	/* This is for Gx */
	for (row=limit;row<=(M-limit);row++){
		for (col=limit;col<=(N-limit);col++){
			newPixel = 0;
			for (kernelRow=-limit; kernelRow<=limit; kernelRow++){
				for (kernelCol=-limit;kernelCol<=limit; kernelCol++){
					newPixel = newPixel + output_gaussian[row+kernelRow][col+kernelCol]*GxMask[limit+kernelRow][limit+kernelCol];
				}
			}
			Igx[row][col] = (int)(floor(newPixel));
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
			Igy[row][col] = (int)(floor(newPixel));
		}
	}

	/* This is for Es */
	for (i=limit;i<=N*M;i++) {
		*s = sqrt(pow(*igx, 2.0)+pow(*igy, 2.0));
		*o = atan2(*igx,*igy)/3.14159 * 180;
		
			if(*s>=255)
			{
				*s = 255;
			}
		*s++ ;
		*o++ ;
		*igx++ ;
		*igy++ ;
	}

}

void nonSup() {
		
	for (row=0;row<M;row++) {
    	for (col=0; col<N;col++) {
    		
        NonMax[row][col] = Es[row][col];
        
    	}
    }
    

    for (row=limit; row<(M-limit);row++) {
        for (col=limit;col<(N-limit);col++)
        {
            //Horizontal Edge
            if (((-22.5 < Eo[row][col]) && (Eo[row][col] <= 22.5)) || ((157.5 < Eo[row][col]) && (Eo[row][col] <= -157.5)))
            {
                if ((Es[row][col] < Es[row][col+1]) || (Es[row][col] < Es[row][col-1]))
                	{
                    NonMax[row][col] = 0;
                }
            }


            //Vertical Edge
            if (((-112.5 < Eo[row][col]) && (Eo[row][col] <= -67.5)) || ((67.5 < Eo[row][col]) && (Eo[row][col] <= 112.5)))
            {
                if ((Es[row][col] < Es[row+1][col]) || (Es[row][col] < Es[row-1][col]))
                {
                    NonMax[row][col] = 0;
                }
            }

            //+45 Degree Edge
            if (((-67.5 < Eo[row][col]) && (Eo[row][col] <= -22.5)) || ((112.5 < Eo[row][col]) && (Eo[row][col] <= 157.5)))
            {
                if ((Es[row][col] < Es[row+1][col+1]) || (Es[row][col] < Es[row-1][col+1]))
                {
			
                    NonMax[row][col] = 0;
                }
            }

            //-45 Degree Edge
            if (((-157.5 < Eo[row][col]) && (Eo[row][col] <= -112.5)) || ((67.5 < Eo[row][col]) && (Eo[row][col] <= 22.5)))
            {
                if ((Es[row][col] < Es[row+1][col+1]) || (Es[row][col] < Es[row-1][col-1]))
                {
			
                    NonMax[row][col] = 0;
                }
            }

        }
    }

}			

void neighbor_loop(int Row, int Col){	
			
            //Horizontal Edge
            if (((-22.5 < Eo[Row][Col]) && (Eo[Row][Col] <= 22.5)) || ((157.5 < Eo[Row][Col]) && (Eo[Row][Col] <= -157.5)))
            {
               if(Es[Row][Col+1]>T1 && searched[Row][Col+1]!=0)
               {
               	Edges[Row][Col+1]=255;
               	neighbor_loop(Row,Col+1);
               	searched[Row][Col+1]=1;
			   }
			   
			   else if(Es[Row][Col-1]>T1 && searched[Row][Col-1]!=0)
               {
               	Edges[Row][Col-1]=255;
               	neighbor_loop(Row,Col-1);
               	searched[Row][Col-1]=1;
			   }
            }

            //Vertical Edge
            else if (((-112.5 < Eo[Row][Col]) && (Eo[Row][Col] <= -67.5)) || ((67.5 < Eo[Row][Col]) && (Eo[Row][Col] <= 112.5)))
            {
            	if(Es[Row+1][Col]>T1 && searched[Row+1][Col]!=0)
               {
               	Edges[Row+1][Col]=255;
               	neighbor_loop(Row+1,Col);
               	searched[Row+1][Col]=1;
			   }
			   
			   else if(Es[Row-1][Col]>T1 && searched[Row-1][Col]!=0)
               {
               	Edges[Row-1][Col]=255;
               	neighbor_loop(Row-1,Col);
               	searched[Row-1][Col]=1;
			   }
                
            }

            //+45 Degree Edge
            else if (((-67.5 < Eo[Row][Col]) && (Eo[Row][Col] <= -22.5)) || ((112.5 < Eo[Row][Col]) && (Eo[Row][Col] <= 157.5)))
            {
                if(Es[Row-1][Col+1]>T1 && searched[Row-1][Col+1]!=0)
               {
               	Edges[Row-1][Col+1]=255;
               	neighbor_loop(Row-1,Col+1);
               	searched[Row-1][Col+1]=1;
			   }
			   
			   else if(Es[Row+1][Col-1]>T1 && searched[Row+1][Col-1]!=0)
               {
               	Edges[Row+1][Col-1]=255;
               	neighbor_loop(Row+1,Col-1);
               	searched[Row+1][Col-1]=1;
			   }
            }
          

            //-45 Degree Edge
            else if (((-157.5 < Eo[Row][Col]) && (Eo[Row][Col] <= -112.5)) || ((67.5 < Eo[Row][Col]) && (Eo[Row][Col] <= 22.5)))
            {
            	if(Es[Row+1][Col+1]>T1 && searched[Row+1][Col+1]!=0)
               {
               	Edges[Row+1][Col+1]=255;
               	neighbor_loop(Row+1,Col+1);
               	searched[Row+1][Col+1]=1;
			   }
			   
			else if(Es[Row-1][Col-1]>T1 && searched[Row-1][Col-1]!=0)
               {
               	Edges[Row-1][Col-1]=255;
               	neighbor_loop(Row-1,Col-1);
               	searched[Row-1][Col-1]=1;
			   }
                
            }
            
            return;
               
}

void four(){


    for (row=limit; row<(M-limit);row++) {
        for (col=limit;col<(N-limit);col++)
        {
        	if(Es[row][col]<T1)
        	{
        		Edges[row][col]=0;
			}
			
			else if(Es[row][col]>T2)
        	{
        		Edges[row][col]=255;
        		neighbor_loop(row,col);
			}
			
			else 
			{
				Edges[row][col]=0;
			}
        	
		}
	}
}


int main() {

  read();

  canny();
  
  sobel();
  
  nonSup();
  
  four();
  
  write();
  
  printf("ok");

  
}
