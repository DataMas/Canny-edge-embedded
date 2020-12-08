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
double lo, hi, mid;
#pragma arm section

int i, j, newPixel, newPixel2, newPixel3,newPixel4, newPixely, newPixel2y,newPixel3y,newPixel4y, row, col, kernelRow, kernelCol;

int searched[M+2][N+2]={0};
int limit=1;
int limit_gauss=1;
int T1=230;
int T2=245;
int *s = &Es[0][0];
int *o = &Eo[0][0];
int *tmp = &temp[0][0];
int *curr = &current[0][0];
int *edge = &Edges[0][0];
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

  for(i=0;i<M*N;i+=4)
  {
  	*curr = fgetc(frame_c);
  	*curr++ = fgetc(frame_c);
  	*curr++ = fgetc(frame_c);
  	*curr++ = fgetc(frame_c);
  }

  fclose(frame_c);
}

//write image
void write()
{
  FILE *frame_yuv;
  frame_yuv=fopen(file_yuv,"wb");

  for(i=1;i<M*N+2;i++)
  {

      fputc(*edge,frame_yuv);
      fputc(*++edge,frame_yuv);
      fputc(*++edge,frame_yuv);
      fputc(*++edge,frame_yuv);

  }
  
  fclose(frame_yuv);
}

// ===================================================== 
void canny()
{

	for(i=0;i<(M*N+4);++i)
 
   {
   	*tmp = 0;
	*tmp++ = 0;
	*tmp++ = 0;
	*tmp++ = 0;
   }
 

  for(i=1;i<(M*+2);++i) {
      temp[i][j]=current[i-1][j-1];
      temp[i][j+1]=current[i-1][j];
      temp[i][j+2]=current[i-1][j+1];
      temp[i][j+3]=current[i-1][j+2];
    }
  

	// Declare Gaussian mask 
	gaussianMask[0][0] = 1;	 gaussianMask[0][1] = 2;  gaussianMask[0][2] = 1;  	
	gaussianMask[1][0] = 2;	 gaussianMask[1][1] = 4;  gaussianMask[1][2] = 2;  	
	gaussianMask[2][0] = 1;	 gaussianMask[2][1] = 2;  gaussianMask[2][2] = 1;
		
	for (row=limit_gauss;row<=M+limit_gauss;row++){
		for (col=limit_gauss;col<=N+limit_gauss;col+=4){
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
			output_gaussian[row][col] = (int)(floor(newPixel)/16);
			output_gaussian[row][col+1] = (int)(floor(newPixel2)/16);
			output_gaussian[row][col+2] = (int)(floor(newPixel3)/16);
			output_gaussian[row][col+3] = (int)(floor(newPixel4)/16);
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
	for (row=limit;row<=(M+limit);row++){
		for (col=2;col<=(N+limit);col+=4){
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
			
			/*
			Es[row][col] = sqrt(Igx[row][col]*Igx[row][col]+Igy[row][col]*Igy[row][col]);
			if(Es[row][col]>=255)
			{
				Es[row][col] = 255;
			}
			Eo[row][col] = (atan2(Igy[row][col],Igx[row][col])/3.14159) * 180.0;
			
			
			Es[row][col+1] = sqrt(Igx[row][col+1]*Igx[row][col+1]+Igy[row][col+1]*Igy[row][col+1]);
			if(Es[row][col+1]>=255)
			{
				Es[row][col+1] = 255;
			}
			Eo[row][col+1] = (atan2(Igy[row][col+1],Igx[row][col+1])/3.14159) * 180.0;
			
			
			Es[row][col+2] = sqrt(Igx[row][col+2]*Igx[row][col+2]+Igy[row][col+2]*Igy[row][col+2]);
			if(Es[row][col+2]>=255)
			{
				Es[row][col+2] = 255;
			}
			Eo[row][col+2] = (atan2(Igy[row][col+2],Igx[row][col+2])/3.14159) * 180.0;
			
			
			Es[row][col+3] = sqrt(Igx[row][col+3]*Igx[row][col+3]+Igy[row][col+3]*Igy[row][col+3]);
			if(Es[row][col+3]>=255)
			{
				Es[row][col+3] = 255;
			}
			Eo[row][col+3] = (atan2(Igy[row][col+3],Igx[row][col+3])/3.14159) * 180.0;
			*/	
	 	}
	}
	
	
	for (i=limit;i<=N*M;i+=4) {
		*s = sqrt((*igx)*(*igx)+(*igy)*(*igy));
		*o = atan2(*igx,*igy)/3.14159 * 180;
		
		if(*s>=255)
		{
			*s = 255;
		}
		*s++ ;
		*o++ ;
		*igx++ ;
		*igy++ ;
		
		*s = sqrt((*igx)*(*igx)+(*igy)*(*igy));
		*o = atan2(*igx,*igy)/3.14159 * 180;
		
		if(*s>=255)
		{
			*s = 255;
		}
		*s++ ;
		*o++ ;
		*igx++ ;
		*igy++ ;

		*s = sqrt((*igx)*(*igx)+(*igy)*(*igy));
		*o = atan2(*igx,*igy)/3.14159 * 180;
		
		if(*s>=255)
		{
			*s = 255;
		}
		*s++ ;
		*o++ ;
		*igx++ ;
		*igy++ ;

		*s = sqrt((*igx)*(*igx)+(*igy)*(*igy));
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
    	for (col=0; col<N;col+=4) {
    		
        NonMax[row][col] = Es[row][col];
        NonMax[row][col+1] = Es[row][col+1];
        NonMax[row][col+2] = Es[row][col+2];
        NonMax[row][col+3] = Es[row][col+3];
        
    	}
    }
    

    for (row=limit; row<(M+limit);row++) {
        for (col=limit;col<(N+limit);col++)
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


    for (row=limit; row<(M+limit);row++) {
        for (col=limit;col<(N+limit);col+=4)
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
			
			
			if(Es[row][col+1]<T1)
        	{
        		Edges[row][col+1]=0;
			}
			
			else if(Es[row][col+1]>T2)
        	{
        		Edges[row][col+1]=255;
        		neighbor_loop(row,col+1);
			}
			
			else 
			{
				Edges[row][col+1]=0;
			}
			
			
			if(Es[row][col+2]<T1)
        	{
        		Edges[row][col+2]=0;
			}
			
			else if(Es[row][col+2]>T2)
        	{
        		Edges[row][col+2]=255;
        		neighbor_loop(row,col+2);
			}
			
			else 
			{
				Edges[row][col+2]=0;
			}
			
			if(Es[row][col+3]<T1)
        	{
        		Edges[row][col+3]=0;
			}
			
			else if(Es[row][col+3]>T2)
        	{
        		Edges[row][col+3]=255;
        		neighbor_loop(row,col+3);
			}
			
			else 
			{
				Edges[row][col+3]=0;
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

  
}
