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
int output_gaussian[M+2][N+2];
int Eo[M+2][N+2];
int Es[M+2][N+2];
int Igx[M+2][N+2];
int Igy[M+2][N+2];
int Edges[M+2][N+2];
int gaussianMask[3][3];
int GxMask[3][3];
int GyMask[3][3];
int NonMax[M][N];
int temp[M+2][N+2];
#pragma arm section

int i, j, newPixel, row, col, kernelRow, kernelCol;

int kernelSize = 5;
int searched[M+2][N+2]={0};
int limit=1;
int limit_gauss=1;
int T1=230;
int T2=245;	

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

  for(i=1;i<M+1;i++)
  {
    for(j=1;j<N+1;j++)
    { 
	  if(i==1 || j==1 || i==M || j==N)
      {
        fputc(0,frame_yuv);
	  }
	  else	
      fputc(Edges[i][j],frame_yuv);
    }
  }
 
  fclose(frame_yuv);
}

// ===================================================== 
void canny()
{

for(i=0;i<(M+2);++i)
 {
   for(j=0;j<(N+2);++j)
   {
     temp[i][j]=150;
   }
 }

  for(i=1;i<(M+1);++i)
  {
    for(j=1;j<(N+1);++j)
    {
      temp[i][j]=current[i-1][j-1];
    }
  }

	// Declare Gaussian mask 
	gaussianMask[0][0] = 1;	 gaussianMask[0][1] = 2;  gaussianMask[0][2] = 1;  	
	gaussianMask[1][0] = 2;	 gaussianMask[1][1] = 4;  gaussianMask[1][2] = 2;  	
	gaussianMask[2][0] = 1;	 gaussianMask[2][1] = 2;  gaussianMask[2][2] = 1;	
	
	for (col=limit_gauss;col<=(N+limit_gauss);col++){
		for (row=limit_gauss;row<=(M+limit_gauss);row++){		
			newPixel = 0;

			//printf("%d",row);
			for (kernelCol=-limit_gauss;kernelCol<=limit_gauss; kernelCol++){
				for (kernelRow=-limit_gauss; kernelRow<=limit_gauss; kernelRow++){
					newPixel = newPixel + temp[row+kernelRow][col+kernelCol]*gaussianMask[limit_gauss+kernelRow][limit_gauss+kernelCol];
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
	for (col=limit;col<=(N+limit);col++){
		for (row=limit;row<=(M+limit);row++){
				newPixel = 0;
				for (kernelCol=-limit;kernelCol<=limit; kernelCol++){
					for (kernelRow=-limit; kernelRow<=limit; kernelRow++){
					
						newPixel = newPixel + output_gaussian[row+kernelRow][col+kernelCol]*GxMask[limit+kernelRow][limit+kernelCol];
					}
				}
				Igx[row][col] = (int)(floor(newPixel));
			}
	}
	
	/* This is for Gy */
	for (col=limit;col<=(N+limit);col++){
		for (row=limit;row<=(M+limit);row++){
			newPixel = 0;
			for (kernelCol=-limit;kernelCol<=limit; kernelCol++){
				for (kernelRow=-limit; kernelRow<=limit; kernelRow++){
					newPixel = newPixel + output_gaussian[row+kernelRow][col+kernelCol]*GyMask[limit+kernelRow][limit+kernelCol];
				}
			}
			Igy[row][col] = (int)(floor(newPixel));
		}
	}

	/* This is for Es */
	for (i=limit;i<=N*M;i++) {
		*s = sqrt(pow(*igx, 2.0)+pow(*igy, 2.0));
		*o = atan2(*igy,*igx)/3.14159 * 180;
		
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
	
	for (col=0; col<N;col++) {	
		for (row=0;row<M;row++) { 		
        NonMax[row][col] = Es[row][col];
        
    	}
    }

    for (col=limit;col<(N+limit);col++){
		for (row=limit; row<(M+limit);row++) {
        
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

    for (col=limit;col<(N+limit);col++) {
		for (row=limit; row<(M+limit);row++) {      
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
