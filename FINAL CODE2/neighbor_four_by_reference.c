void neighbor_loop(int *Row, int *Col){	
			
			
            //Horizontal Edge
            if (((-22.5 < Eo[*Row][*Col]) && (Eo[*Row][*Col] <= 22.5)) || ((157.5 < Eo[*Row][*Col]) && (Eo[*Row][*Col] <= -157.5)))
            {
               if(NonMax[*Row][*Col+1]>T1 && searched[*Row][*Col+1]!=0)
               {
               	Edges[*Row][*Col+1]=255;
               	neighbor_loop(Row,Col+1);
               	searched[*Row][*Col+1]=1;
			   }
			   
			   else if(NonMax[*Row][*Col-1]>T1 && searched[*Row][*Col-1]!=0)
               {
               	Edges[*Row][*Col-1]=255;
               	neighbor_loop(Row,Col-1);
               	searched[*Row][*Col-1]=1;
			   }
            }

            //Vertical Edge
            else if (((-112.5 < Eo[*Row][*Col]) && (Eo[*Row][*Col] <= -67.5)) || ((67.5 < Eo[*Row][*Col]) && (Eo[*Row][*Col] <= 112.5)))
            {
            	if(NonMax[*Row+1][*Col]>T1 && searched[*Row+1][*Col]!=0)
               {
               	Edges[*Row+1][*Col]=255;
               	neighbor_loop(Row+1,Col);
               	searched[*Row+1][*Col]=1;
			   }
			   
			   else if(NonMax[*Row-1][*Col]>T1 && searched[*Row-1][*Col]!=0)
               {
               	Edges[*Row-1][*Col]=255;
               	neighbor_loop(Row-1,Col);
               	searched[*Row-1][*Col]=1;
			   }
                
            }

            //+45 Degree Edge
            else if (((-67.5 < Eo[*Row][*Col]) && (Eo[*Row][*Col] <= -22.5)) || ((112.5 < Eo[*Row][*Col]) && (Eo[*Row][*Col] <= 157.5)))
            {
                if(NonMax[*Row-1][*Col+1]>T1 && searched[*Row-1][*Col+1]!=0)
               {
               	Edges[*Row-1][*Col+1]=255;
               	neighbor_loop(Row-1,Col+1);
               	searched[*Row-1][*Col+1]=1;
			   }
			   
			   else if(NonMax[*Row+1][*Col-1]>T1 && searched[*Row+1][*Col-1]!=0)
               {
               	Edges[*Row+1][*Col-1]=255;
               	neighbor_loop(Row+1,Col-1);
               	searched[*Row+1][*Col-1]=1;
			   }
            }
          

            //-45 Degree Edge
            else if (((-157.5 < Eo[*Row][*Col]) && (Eo[*Row][*Col] <= -112.5)) || ((67.5 < Eo[*Row][*Col]) && (Eo[*Row][*Col] <= 22.5)))
            {
            	if(NonMax[*Row+1][*Col+1]>T1 && searched[*Row+1][*Col+1]!=0)
               {
               	Edges[*Row+1][*Col+1]=255;
               	neighbor_loop(Row+1,Col+1);
               	searched[*Row+1][*Col+1]=1;
			   }
			   
			else if(NonMax[*Row-1][*Col-1]>T1 && searched[*Row-1][*Col-1]!=0)
               {
               	Edges[*Row-1][*Col-1]=255;
               	neighbor_loop(Row-1,Col-1);
               	searched[*Row-1][*Col-1]=1;
			   }
                
            }
            
            return;
               
}

void four(){


    for (row=limit; row<(M+limit);row++) {
        for (col=limit;col<(N+limit);col+=4)
        {
        //	col1=col+1;
        //	col2=col+2;
        //	col3=col+3;
        	
        	if(NonMax[row][col]<T1)
        	{
        		Edges[row][col]=0;
			}
			
			else if(NonMax[row][col]>T2)
        	{
        		Edges[row][col]=255;
        		neighbor_loop(&row,&col);
			}
			
			else 
			{
				Edges[row][col]=0;
			}
			
			
			if(NonMax[row][col+1]<T1)
        	{
        		Edges[row][col+1]=0;
			}
			
			else if(NonMax[row][col+1]>T2)
        	{
        		Edges[row][col+1]=255;
        		neighbor_loop(&row,&col+1);
			}
			
			else 
			{
				Edges[row][col+1]=0;
			}
			
			
			if(NonMax[row][col+2]<T1)
        	{
        		Edges[row][col+2]=0;
			}
			
			else if(NonMax[row][col+2]>T2)
        	{
        		Edges[row][col+2]=255;
        		neighbor_loop(&row,&col+2);
			}
			
			else 
			{
				Edges[row][col+2]=0;
			}
			
			if(NonMax[row][col+3]<T1)
        	{
        		Edges[row][col+3]=0;
			}
			
			else if(NonMax[row][col+3]>T2)
        	{
        		Edges[row][col+3]=255;
        		neighbor_loop(&row,&col+3);
			}
			
			else 
			{
				Edges[row][col+3]=0;
			}
        	
		}
	}
}
