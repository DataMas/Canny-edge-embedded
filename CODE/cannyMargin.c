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
int output_gaussian[M+4][N+4];
int Eo[M+4][N+4];
int Es[M+4][N+4];
int Igx[M+4][N+4];
int Igy[M+4][N+4];
int Edges[M+4][N+4];
int searched[M+4][N+4]={0};
int limit = 1;
int limit_gauss = 2;
int T1=230;
int T2=245;
#pragma arm section

int i, j, newPixel, newPixel2, newPixel3,newPixel4, newPixely, newPixel2y,newPixel3y,newPixel4y, row, col, kernelRow, kernelCol;
int temp[M+4][N+4];
int gaussianMask[5][5];
int GxMask[3][3];
int GyMask[3][3];
int kernelSize = 5;

int NonMax[M+4][N+4];

int *s = &Es[0][0];
int *o = &Eo[0][0];
int *nonmax = &NonMax[0][0];
int *igx = &Igx[0][0];
int *igy = &Igy[0][0];

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

  for(i=4;i<M+2;i++)
  {
    for(j=4;j<N+2;j++)
    {
      fputc(Edges[i][j],frame_yuv);
    }
  }
 
  fclose(frame_yuv);
}

// ===================================================== 
void canny()
{
	
for(i=0;i<(M+4);++i)
 {
   for(j=0;j<(N+4);++j)
   {
     temp[i][j]=255;
   }
  }

  for(i=2;i<(M+2);++i)
  {
    for(j=2;j<(N+2);++j)
    {
      temp[i][j]=current[i-2][j-2];
    }
  }

	// Declare Gaussian mask 
	gaussianMask[0][0] = 2;	 gaussianMask[0][1] = 4;  gaussianMask[0][2] = 5;  gaussianMask[0][3] = 4;  gaussianMask[0][4] = 2;	
	gaussianMask[1][0] = 4;	 gaussianMask[1][1] = 9;  gaussianMask[1][2] = 12; gaussianMask[1][3] = 9;  gaussianMask[1][4] = 4;	
	gaussianMask[2][0] = 5;	 gaussianMask[2][1] = 12; gaussianMask[2][2] = 15; gaussianMask[2][3] = 12; gaussianMask[2][4] = 2;	
	gaussianMask[3][0] = 4;	 gaussianMask[3][1] = 9;  gaussianMask[3][2] = 12; gaussianMask[3][3] = 9;  gaussianMask[3][4] = 4;	
	gaussianMask[4][0] = 2;	 gaussianMask[4][1] = 4;  gaussianMask[4][2] = 5;  gaussianMask[4][3] = 4;  gaussianMask[4][4] = 2;	
		
	for (row=limit_gauss;row<=M+2;row++){
		for (col=limit_gauss;col<=N+2;col+=4){
			newPixel = 0;
			newPixel2 = 0;
			newPixel3 = 0;
			newPixel4 = 0;
			//printf("%d",row);
			for (kernelRow=-limit_gauss; kernelRow<=limit_gauss; kernelRow++){
				for (kernelCol=-limit_gauss;kernelCol<=limit_gauss; kernelCol++){
					//newPixel = newPixel + temp[row+kernelRow][col+kernelCol]*gaussianMask[limit_gauss+kernelRow][limit_gauss+kernelCol];
					newPixel = newPixel + current[row+kernelRow][col+kernelCol]*gaussianMask[limit_gauss+kernelRow][limit_gauss+kernelCol];
					newPixel2 = newPixel2 + current[row+kernelRow][col+kernelCol+1]*gaussianMask[limit_gauss+kernelRow][limit_gauss+kernelCol];
					newPixel3 = newPixel3 + current[row+kernelRow][col+kernelCol+2]*gaussianMask[limit_gauss+kernelRow][limit_gauss+kernelCol];
					newPixel4 = newPixel4 + current[row+kernelRow][col+kernelCol+3]*gaussianMask[limit_gauss+kernelRow][limit_gauss+kernelCol];
				}
			}
			output_gaussian[row][col] = (int)(floor(newPixel)/159);
			output_gaussian[row][col+1] = (int)(floor(newPixel2)/159);
			output_gaussian[row][col+2] = (int)(floor(newPixel3)/159);
			output_gaussian[row][col+3] = (int)(floor(newPixel4)/159);
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
	for (row=2;row<=(M+2);row++){
		for (col=2;col<=(N+2);col+=4){
			newPixel = 0;
			newPixel2 = 0;
			newPixel3 = 0;
			newPixel4 = 0;
			
			newPixely = 0;
			newPixel2y = 0;
			newPixel3y = 0;
			newPixel4y = 0;
			for (kernelRow=-limit; kernelRow<=limit; kernelRow++){
				for (kernelCol=-limit;kernelCol<=limit; kernelCol++){
					newPixel = newPixel + output_gaussian[row+kernelRow][col+kernelCol]*GxMask[limit+kernelRow][limit+kernelCol];
					newPixel2 = newPixel2 + output_gaussian[row+kernelRow][col+kernelCol+1]*GxMask[limit+kernelRow][limit+kernelCol];
					newPixel3 = newPixel3 + output_gaussian[row+kernelRow][col+kernelCol+2]*GxMask[limit+kernelRow][limit+kernelCol];
					newPixel4 = newPixel4 + output_gaussian[row+kernelRow][col+kernelCol+3]*GxMask[limit+kernelRow][limit+kernelCol];
					
					newPixely = newPixely + output_gaussian[row+kernelRow][col+kernelCol]*GyMask[limit+kernelRow][limit+kernelCol];
					newPixel2y = newPixel2y + output_gaussian[row+kernelRow][col+kernelCol+1]*GyMask[limit+kernelRow][limit+kernelCol];
					newPixel3y = newPixel3y + output_gaussian[row+kernelRow][col+kernelCol+2]*GyMask[limit+kernelRow][limit+kernelCol];
					newPixel4y = newPixel4y + output_gaussian[row+kernelRow][col+kernelCol+3]*GyMask[limit+kernelRow][limit+kernelCol];
	
				}
			}
			Igx[row][col] = (int)(floor(newPixel));
			Igx[row][col+1] = (int)(floor(newPixel2));
			Igx[row][col+2] = (int)(floor(newPixel3));
			Igx[row][col+3] = (int)(floor(newPixel4));
			
			Igy[row][col] = (int)(floor(newPixely));
			Igy[row][col+1] = (int)(floor(newPixel2y));
			Igy[row][col+2] = (int)(floor(newPixel3y));
			Igy[row][col+3] = (int)(floor(newPixel4y));	
	 	}
	}
	
	for (i=2;i<=N*M;i++) {
		*s = sqrt((*igx)*(*igx)+(*igy)*(*igy));
		*o = sqrt((*igx)*(*igx)+(*igy)*(*igy));
		
		if(*s>=255)
		{
			*s = 255;
		}
		*s++ ;
		*igx++ ;
		*igy++ ;
	}
}

void nonSup() {
	//****
	for (row=0;row<M;row++) {
    	for (col=0; col<N;col++) {
    		
        NonMax[row][col] = Es[row][col];
        
    	}
    }
    

    for (row=limit; row<(M+limit);row++) {
        for (col=limit;col<(N+limit);col++)
        {
			/*
            if (DerivativeX[i, j] == 0)
                Tangent = 90F;
            else
                Tangent = (float)(Math.Atan(DerivativeY[i, j] / DerivativeX[i, j]) * 180 / Math.PI); //rad to degree
			*/


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


    for (row=limit; row<(M+limit);row++) {
        for (col=limit;col<(N+limit);col++)
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
