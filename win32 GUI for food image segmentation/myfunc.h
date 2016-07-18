void balloon(int xPo, int yPo)
{
	FILE *fpt;
	float sobelx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
	float sobely[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
	float Gx, Gy;
	int r, c, p, q;
	int m, n;
	float midmax=-100000, midmin=100000;
	int iter=0,move=0;
	float n1, m1, inter1[5][5], inter2[5][5], exter[5][5], energy[5][5];//declare a 1D array to store the energy in the 7*7 window
	float red[5][5], green[5][5], blue[5][5], colordif[5][5];
	int point=0;
	float maxi1, mini1, maxi2, mini2, maxex, minex, maxcdif, mincdif;
	float sumx, sumy, sumr, sumg, sumb;
	int centx, centy, si, sj, sn, sm;
	float sumdis,ave,scale1,scale2,scale3,scale4,maxtotal;
	int n0,m0,pr,pc;
	//declare memory for tempory images
	float * Gradient;
	float ** middle;
	HDC  hDC;
	int newx = xPo, newy = yPo, curx, cury;
	float dif,dif2,dif3;
	int pix, nnum = 0;
	hDC=GetDC(MainWnd);
	x = 10;
	y = 0;
	decisionOver2 = 1 - x;   // Decision criterion divided by 2 evaluated at x=r, y=0
	p = 0;
	contnum = 0;
	//Get initial contour
	line[nnum][0] = xPo;
	line[nnum][1] = yPo-10;
	while((newx!=xPo)||(newy!=yPo-10)){
		//	while(contnum<50){
		curx = line[nnum][0];
		cury = line[nnum][1];
		if((curx>=xPo)&&(cury<=yPo)){
			newx = curx+1; 
			newy = cury;
			dif=abs((curx+1-xPo)*(curx+1-xPo)+(cury-yPo)*(cury-yPo)-10*10);
			dif2=abs((curx-xPo)*(curx-xPo)+(cury+1-yPo)*(cury+1-yPo)-10*10);
			if(dif2<dif){
				newx = curx; 
				newy = cury+1;
				dif = dif2;
			}
			dif3=abs((curx+1-xPo)*(curx+1-xPo)+(cury+1-yPo)*(cury+1-yPo)-10*10);
			if(dif3<dif){
				newx = curx+1; 
				newy = cury+1;
				//dif = dif3;
			}
		}
		if((curx>=xPo)&&(cury>=yPo)){
			newx = curx-1; 
			newy = cury;
			dif=abs((curx-1-xPo)*(curx-1-xPo)+(cury-yPo)*(cury-yPo)-10*10);
			dif2=abs((curx-xPo)*(curx-xPo)+(cury+1-yPo)*(cury+1-yPo)-10*10);
			if(dif2<dif){
				newx = curx; 
				newy = cury+1;
				dif = dif2;
			}
			dif3=abs((curx-1-xPo)*(curx-1-xPo)+(cury+1-yPo)*(cury+1-yPo)-10*10);
			if(dif3<dif){
				newx = curx-1; 
				newy = cury+1;
				dif = dif3;
			}
		}
		if((curx<=xPo)&&(cury>=yPo)){
			newx = curx-1; 
			newy = cury;
			dif=abs((curx-1-xPo)*(curx-1-xPo)+(cury-yPo)*(cury-yPo)-10*10);
			dif2=abs((curx-xPo)*(curx-xPo)+(cury-1-yPo)*(cury-1-yPo)-10*10);
			if(dif2<dif){
				newx = curx; 
				newy = cury-1;
				dif = dif2;
			}
			dif3=abs((curx-1-xPo)*(curx-1-xPo)+(cury-1-yPo)*(cury-1-yPo)-10*10);
			if(dif3<dif){
				newx = curx-1; 
				newy = cury-1;
				//dif = dif3;
			}
		}
		if((curx<=xPo)&&(cury<=yPo)){
			newx = curx+1; 
			newy = cury;
			dif=abs((curx+1-xPo)*(curx+1-xPo)+(cury-yPo)*(cury-yPo)-10*10);
			dif2=abs((curx-xPo)*(curx-xPo)+(cury-1-yPo)*(cury-1-yPo)-10*10);
			if(dif2<dif){
				newx = curx; 
				newy = cury-1;
				dif = dif2;
			}
			dif3=abs((curx+1-xPo)*(curx+1-xPo)+(cury-1-yPo)*(cury-1-yPo)-10*10);
			if(dif3<dif){
				newx = curx+1; 
				newy = cury-1;
				//dif = dif3;
			}
		}
		nnum++;
		line[nnum][0] = newx;
		line[nnum][1] = newy;
		//SetPixel(hDC,newx,newy,RGB(255,0,255));
	}

	//down sample to every third pixel
	fpt=fopen("contourball.txt","w");
	for (i=0; i<nnum; i++)
	{
		if (i % 3 == 0){
			contnum++;
					fprintf(fpt,"%d %d\n",line[i][0], line[i][1]);
					sline[i/3][0] = line[i][0];
					sline[i/3][1] = line[i][1];
					SetPixel(hDC,sline[i/3][0], sline[i/3][1],RGB(255,0,255));
				}
	}
	//fprintf(fpt,"%d\n",q);
	fclose(fpt);

	//calculate the gradient image 
	Gradient = (float *)calloc(ROWS*COLS,sizeof(float));
	middle = (float **)calloc(COLS, sizeof(float *));
	for(i=0; i<COLS; i++)
	{
		middle[i] = (float *)calloc(ROWS, sizeof(float));
	}
	//compute the gradient image
	for(r=1; r<ROWS-1; r++){
		for (c=1; c<COLS-1; c++){
			Gx = 0; Gy = 0;
			for(p=-1; p<=1; p++){
				for(q=-1; q<=1; q++){
					Gx = Gx + (float)smoothed[(r+p)*COLS+c+q] * sobelx[1+q][1+p];
					Gy = Gy + (float)smoothed[(r+p)*COLS+c+q] * sobely[1+q][1+p];
				}
			}
			//middle[r][c] = (Gx * Gx + Gy * Gy);
			middle[c][r] = abs(Gx) + abs(Gy);
			if (middle[c][r]<midmin){midmin=middle[c][r];}
			if (middle[c][r]>midmax){midmax=middle[c][r];}
		}
	}
	for(r=1; r<ROWS-1; r++){
		for (c=1; c<COLS-1; c++){
			Gradient[r*COLS+c] = (middle[c][r]-midmin)*255/(midmax-midmin);
		}
	}
	sumx = 0; sumy = 0;
	sumr = 0; sumb = 0; sumg = 0;
	for (r = -4; r <= 4; r++){
		for (c = -4; c<= 4; c++){
			sumr = sumr + (float)data[((r+yPo)*COLS+c+xPo)*3];
			sumg = sumg + (float)data[((r+yPo)*COLS+c+xPo)*3+1];
			sumb = sumb + (float)data[((r+yPo)*COLS+c+xPo)*3+2];
			//SetPixel(hDC,xPo+c,yPo+r,RGB(data[((r+yPo)*COLS+c+xPo)*3],data[((r+yPo)*COLS+c+xPo)*3+1],data[((r+yPo)*COLS+c+xPo)*3+2]));
		}
	}
	sumr = sumr/81.0;
	sumg = sumg/81.0;
	sumb = sumb/81.0;
	//active contour
	for (iter = 0; iter <=70; iter++){
		//PaintImage();
		move =0;
		sumdis = 0;
		n0 = sline[0][0];
		m0 = sline[0][1];
		//first calculate the average of all the distances
		//meanwhile, compute the center point of the contour
		for (point=0; point<contnum; point++){
			n = sline[point][0];
			m = sline[point][1];
			for (j = -3; j <= 3; j++){
				pix = OriginalImage[m*COLS+n+j];
				SetPixel(hDC,n+j,m,RGB(pix, pix, pix));
				pix = OriginalImage[(m+j)*COLS+n];
				SetPixel(hDC,n,m+j,RGB(pix, pix, pix)); 
			}
			//draw the initial contour
			newx = line[point*3][0];
			newy = line[point*3][1];
			SetPixel(hDC,newx,newy,RGB(255,0,255));
			if (point==contnum-1){
				n1=n0;
				m1=m0;
			}else{
				n1 = sline[(point+1)][0];
				m1 = sline[(point+1)][1];
			}
			sumdis = sumdis + (n-n1)*(n-n1)+(m-m1)*(m-m1);
		}
		ave = sumdis/contnum;
		//for each point, calculate the energies
		for (point=0; point<contnum; point++){
			n = sline[point][0];
			m = sline[point][1];
			if (point==contnum-1){
				//n1=n0;
				//m1=m0;
				n1=sline[0][0];
				m1=sline[0][1];
			}else{
				n1 = sline[(point+1)][0];
				m1 = sline[(point+1)][1];
			}

			//printf("n %d, m %d",n,m);// n1 %f, m1 %f.\n",n,m,n1,m1); 
			maxi1 = -10000000; mini1 = 10000000;
			maxi2 = -10000000; mini2 = 10000000;
			maxex = -10000000; minex = 10000000;
			maxcdif = -10000000; mincdif = 10000000;
			//for each pixel in the 7*7 winballow
			for (r = (-1)*winball; r <= winball; r++){
				for (c = (-1)*winball; c <= winball; c++){
					if((n+c>0) && (m+r>0) && (n+c<COLS) && (m+r<ROWS) && (((r+m)*COLS+c+n)*3+2>0) && (((r+m)*COLS+c+n)*3+2<total)){

						inter1[winball+c][winball+r] = (float)((n+c-n1)*(n+c-n1)+(m+r-m1)*(m+r-m1));
						//inter1[winball+c][winball+r] = 0;
						//calculate internel energy 2 averaging the points using inter1
						inter2[winball+c][winball+r] = (float)((ave - inter1[winball+c][winball+r])*(ave - inter1[winball+c][winball+r]));
						
						//recalculate internal energy 1, calculate distance of each point towards the center of the circle
						inter1[winball+c][winball+r] = (float)((n+c-xPo)*(n+c-xPo)+(m+r-yPo)*(m+r-yPo));

						//calculate the externel energy using gradience values
						exter[winball+c][winball+r] = middle[n+c][m+r];

						//calculate the red, green and blue colors
						red[winball+c][winball+r] = (abs((float)data[((r+m)*COLS+c+n)*3]-sumr));
						green[winball+c][winball+r] = (abs((float)data[((r+m)*COLS+c+n)*3+1]-sumg));
						blue[winball+c][winball+r] = (abs((float)data[((r+m)*COLS+c+n)*3+2]-sumb));
						colordif[winball+c][winball+r] = red[winball+c][winball+r]+green[winball+c][winball+r]+blue[winball+c][winball+r];
						//printf ("exter %f\n",exter[3+i][3+j]);
						//printf ("energy %f\n",energy[3+i][3+j]);
						//find the minimum and maximum value
						if (inter1[winball+c][winball+r] > maxi1){ maxi1 = inter1[winball+c][winball+r];}
						if (inter1[winball+c][winball+r] < mini1){ mini1 = inter1[winball+c][winball+r];}
						if (inter2[winball+c][winball+r] > maxi2){ maxi2 = inter2[winball+c][winball+r];}
						if (inter2[winball+c][winball+r] < mini2){ mini2 = inter2[winball+c][winball+r];}
						if (exter[winball+c][winball+r] > maxex){ maxex = exter[winball+c][winball+r];}
						if (exter[winball+c][winball+r] < minex){ minex = exter[winball+c][winball+r];}
						if (colordif[winball+c][winball+r] > maxcdif){ maxcdif = colordif[winball+c][winball+r];}
						if (colordif[winball+c][winball+r] < mincdif){ mincdif = colordif[winball+c][winball+r];}				
					}
				}      
			}

			scale1 = maxi1 - mini1;
			scale2 = maxi2 - mini2;
			scale3 = maxex - minex;
			scale4 = maxcdif - mincdif;
			maxtotal = -1000000;
			pr = 0, pc = 0;
			//normalize the values
			for (r = -winball; r <= winball; r++){
				for (c = -winball; c <= winball; c++){
					if((n+c>0) && (m+r>0) && (n+c<COLS) && (m+r<ROWS) && (((r+m)*COLS+c+n)*3+2>0) && (((r+m)*COLS+c+n)*3+2<total)){
						inter1[winball+c][winball+r] = (inter1[winball+c][winball+r]-mini1)/scale1;
						inter2[winball+c][winball+r] = (inter2[winball+c][winball+r]-mini2)/scale2;
						exter[winball+c][winball+r] = (exter[winball+c][winball+r]-minex)/scale3;
						colordif[winball+c][winball+r] = (colordif[winball+c][winball+r]-mincdif)/scale4;
						// calculate the total energy
						energy[winball+c][winball+r] =(inter1[winball+c][winball+r])-4*(inter2[winball+c][winball+r])-1.9*(colordif[winball+c][winball+r])+1.8*(exter[winball+c][winball+r]);
						// find the maximum energy and its position
						if (energy[winball+c][winball+r] > maxtotal) { 
							maxtotal = energy[winball+c][winball+r];
							pr = r;
							pc = c;
							// printf("%d %d\n",pc,pr);
						}
					}
				}
			}

			// move the point in the contour
			if (n+pc>0 && n+pc<COLS && m+pr>0 && m+pr<ROWS){
				sline[point][0] = n + pc;
				sline[point][1] = m + pr;
			}
			// check if contour moves
			if ((pc!=0)||(pr!=0)){
				move++;
			} 

		}

		//for every pixel on the contour, draw a crossing
		for (si = 0; si < contnum; si++){
			sn = sline[si][0];//c
			sm = sline[si][1];//r
			for (sj = -3; sj <= 3; sj++){
				SetPixel(hDC,sn+sj,sm,RGB(0,0,255));
				SetPixel(hDC,sn,sm+sj,RGB(0,0,255));       
			}
		}
		//printf("move %d\n",move);

		Sleep(100);

	}

	done = 1;
	ReleaseDC(MainWnd,hDC);
	free(Gradient);
	for(i=0; i<ROWS; i++)
	{
		free(middle[i]);
	}
	free(middle);
	return;
}




void AnimationThread(HWND AnimationWindowHandle)
{
	HDC hDC; 
	int xPos, yPos;
	//sobel template
	float sobelx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
	float sobely[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
	float Gx, Gy;
	int r, c, p, q;
	int m, n;
	int pix;
	float midmax=-100000, midmin=100000;
	int iter=0,move=0;
	float n1, m1, inter1[11][11]={0}, inter2[11][11]={0}, exter[11][11]={0}, energy[11][11]={0};//declare a 1D array to store the energy in the 7*7 window
	float red[11][11]={0}, green[11][11]={0}, blue[11][11]={0}, colordif[11][11]={0};
	int point=0;
	float maxi1, mini1, maxi2, mini2, maxex, minex, maxcdif, mincdif;
	float sumx, sumy, sumr, sumg, sumb;
	int centx, centy;
	float sumdis,ave,scale1,scale2,scale3,scale4,mintotal;
	int n0,m0,pr,pc,sm,sn,sj,si;
	//declare memory for tempory images
	unsigned char * Gradient;
	float ** middle;
	int newx, newy;
	Gradient = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
	middle = (float **)calloc(COLS, sizeof(float *));
	for(i=0; i<COLS; i++)
	{
		middle[i] = (float *)calloc(ROWS, sizeof(float));
	}
	hDC=GetDC(MainWnd);
	//compute the gradient image
	for(r=1; r<ROWS-1; r++){
		for (c=1; c<COLS-1; c++){
			Gx = 0; Gy = 0;
			for(p=-1; p<=1; p++){
				for(q=-1; q<=1; q++){
					Gx = Gx + (float)smoothed[(r+p)*COLS+c+q] * sobelx[1+q][1+p];
					Gy = Gy + (float)smoothed[(r+p)*COLS+c+q] * sobely[1+q][1+p];
				}
			}
			//middle[r][c] = (Gx * Gx + Gy * Gy);
			middle[c][r] = abs(Gx) + abs(Gy);
			if (middle[c][r]<midmin){midmin=middle[c][r];}
			if (middle[c][r]>midmax){midmax=middle[c][r];}
		}
	}
	for(r=1; r<ROWS-1; r++){
		for (c=1; c<COLS-1; c++){
			Gradient[r*COLS+c] = (middle[c][r]-midmin)*255/(midmax-midmin);
		}
	}
	//find the color around the center point
	sumr = 0; sumb = 0; sumg = 0;
	sumx = 0; sumy = 0;
	for (point=0; point<contnum; point++){
		n = sline[point][0];
		m = sline[point][1];
		sumx = sumx + n;
		sumy = sumy + m;
	}
	centx = sumx/contnum;//x of center point
	centy = sumy/contnum;//y of center point
	for (r = -4; r <= 4; r++){
		for (c = -4; c<= 4; c++){
			sumr = sumr + (float)data[((r+centy)*COLS+c+centx)*3];
			sumg = sumg + (float)data[((r+centy)*COLS+c+centx)*3+1];
			sumb = sumb + (float)data[((r+centy)*COLS+c+centx)*3+2];
			SetPixel(hDC,centx+c,centy+r,RGB(data[((r+centy)*COLS+c+centx)*3],data[((r+centy)*COLS+c+centx)*3+1],data[((r+centy)*COLS+c+centx)*3+2]));
		}
	}
	sumr = sumr/81.0;
	sumg = sumg/81.0;
	sumb = sumb/81.0;
	//active contour
	for (iter = 0; iter <=60; iter++){
		//PaintImage();
		move =0;
		sumdis = 0;
		n0 = sline[0][0];
		m0 = sline[0][1];
		//first calculate the average of all the distances
		//meanwhile, compute the center point of the contour
		for (point=0; point<contnum; point++){
			n = sline[point][0];
			m = sline[point][1];
			for (j = -3; j <= 3; j++){
				pix = OriginalImage[m*COLS+n+j];
				SetPixel(hDC,n+j,m,RGB(pix, pix, pix));
				pix = OriginalImage[(m+j)*COLS+n];
				SetPixel(hDC,n,m+j,RGB(pix, pix, pix));    
			}
			//draw the initial contour
			newx = line[point*5][0];
			newy = line[point*5][1];
			for (i = -1; i <= 1; i++)
				{
					for (j = -1; j <=1; j++)
					{
						SetPixel(hDC,newx+i,newy+j,RGB(255,0,0));
					}
				}
			//SetPixel(hDC,newx,newy,RGB(255,0,255));

			if (point==contnum-1){
				n1=n0;
				m1=m0;
			}else{
				n1 = sline[(point+1)][0];
				m1 = sline[(point+1)][1];
			}
			sumdis = sumdis + (n-n1)*(n-n1)+(m-m1)*(m-m1);
		}
		ave = sumdis/contnum;

		//printf("ave %f",ave);
		//for each point, calculate the energies
		for (point=0; point<contnum; point++){
			n = sline[point][0];
			m = sline[point][1];
			if (point==contnum-1){
				//n1=n0;
				//m1=m0;
				n1=sline[0][0];
				m1=sline[0][1];
			}else{
				n1 = sline[(point+1)][0];
				m1 = sline[(point+1)][1];
			}
			//printf("n %d, m %d",n,m);// n1 %f, m1 %f.\n",n,m,n1,m1); 
			maxi1 = -10000000; mini1 = 10000000;
			maxi2 = -10000000; mini2 = 10000000;
			maxex = -10000000; minex = 10000000;
			maxcdif = -10000000; mincdif = 10000000;
			//for each pixel in the 7*7 window
			for (r = (-1)*wind; r <= wind; r++){
				for (c = (-1)*wind; c <= wind; c++){
					if((n+c>0) && (m+r>0) && (n+c<COLS) && (m+r<ROWS) && (((r+m)*COLS+c+n)*3+2>0) && (((r+m)*COLS+c+n)*3+2<total)){
						//calculate inter energy 1
						inter1[wind+c][wind+r] = (float)((n+c-n1)*(n+c-n1)+(m+r-m1)*(m+r-m1));

						//calculate inter energy 2
						//inter2[3+r][3+c] = 0;
						inter2[wind+c][wind+r] = (float)((ave - inter1[wind+c][wind+r])*(ave - inter1[wind+c][wind+r]));
						//inter2[3+r][3+c] = (float)(abs(ave - inter1[3+r][3+c]));

						//calculate the externel energy by convolution
						exter[wind+c][wind+r] = middle[n+c][m+r];

						//calculate the red, green and blue colors
						red[wind+c][wind+r] = (abs((float)data[((r+m)*COLS+c+n)*3]-sumr));
						green[wind+c][wind+r] = (abs((float)data[((r+m)*COLS+c+n)*3+1]-sumg));
						blue[wind+c][wind+r] = (abs((float)data[((r+m)*COLS+c+n)*3+2]-sumb));
						colordif[wind+c][wind+r] = red[wind+c][wind+r]+green[wind+c][wind+r]+blue[wind+c][wind+r];
						//printf ("exter %f\n",exter[3+i][3+j]);
						//printf ("energy %f\n",energy[3+i][3+j]);
						//find the minimum and maximum value
						if (inter1[wind+c][wind+r] > maxi1){ maxi1 = inter1[wind+c][wind+r];}
						if (inter1[wind+c][wind+r] < mini1){ mini1 = inter1[wind+c][wind+r];}
						if (inter2[wind+c][wind+r] > maxi2){ maxi2 = inter2[wind+c][wind+r];}
						if (inter2[wind+c][wind+r] < mini2){ mini2 = inter2[wind+c][wind+r];}
						if (exter[wind+c][wind+r] > maxex){ maxex = exter[wind+c][wind+r];}
						if (exter[wind+c][wind+r] < minex){ minex = exter[wind+c][wind+r];}
						if (colordif[wind+c][wind+r] > maxcdif){ maxcdif = colordif[wind+c][wind+r];}
						if (colordif[wind+c][wind+r] < mincdif){ mincdif = colordif[wind+c][wind+r];}
					}
				}      
			}

			scale1 = maxi1 - mini1;
			scale2 = maxi2 - mini2;
			scale3 = maxex - minex;
			scale4 = maxcdif - mincdif;
			if(scale1 == 0){scale1 = 1;}
			if(scale2 == 0){scale2 = 1;}
			if(scale3 == 0){scale3 = 1;}
			if(scale4 == 0){scale4 = 1;}
			mintotal = 1000000;
			pr = 0, pc = 0;
			//normalize the values

			for (r = -wind; r <= wind; r++){
				for (c = -wind; c <= wind; c++){
					if((n+c>0) && (m+r>0) && (n+c<COLS) && (m+r<ROWS) && (((r+m)*COLS+c+n)*3+2>0) && (((r+m)*COLS+c+n)*3+2<total)){
						inter1[wind+c][wind+r] = (inter1[wind+c][wind+r]-mini1)/scale1;
						inter2[wind+c][wind+r] = (inter2[wind+c][wind+r]-mini2)/scale2;
						exter[wind+c][wind+r] = (exter[wind+c][wind+r]-minex)/scale3;
						colordif[wind+c][wind+r] = (colordif[wind+c][wind+r]-mincdif)/scale4;
						// calculate the total energy
						energy[wind+c][wind+r] =5*(inter1[wind+c][wind+r])+(inter2[wind+c][wind+r])+(colordif[wind+c][wind+r])-(exter[wind+c][wind+r])*2;
						// find the minimum energy and its position
						if (energy[wind+c][wind+r] < mintotal) { 
							mintotal = energy[wind+c][wind+r];
							pr = r;
							pc = c;
							// printf("%d %d\n",pc,pr);
						}
					}
				}
			}

			// move the point in the contour
			sline[point][0] = n + pc;
			sline[point][1] = m + pr;
			// check if contour moves
			if ((pc!=0)||(pr!=0)){
				move++;
			} 

		}
		//printf("move %d\n",move);
		if (move == 0){
			printf("stop %d\n",iter);
			break;
		}

		//for every pixel on the contour, draw a crossing
		for (si = 0; si < contnum; si++){
			sn = sline[si][0];//c
			sm = sline[si][1];//r
			for (sj = -3; sj <= 3; sj++){
				SetPixel(hDC,sn+sj,sm,RGB(0,0,255));
				SetPixel(hDC,sn,sm+sj,RGB(0,0,255));
				// [(m+j)*COLS+n] = 0;
				// imagecopy[m*COLS+(n+j)] = 0;       
			}
		}
		Sleep(100);
	}
	done = 1;
	ReleaseDC(MainWnd,hDC);
	free(Gradient);
	for(i=0; i<ROWS; i++)
	{
		free(middle[i]);
	}
	free(middle);
}



void Manual()
{
	HDC hDC; 
	int xPos, yPos;
	//sobel template
	float sobelx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
	float sobely[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
	float Gx, Gy;
	int r, c, p, q;
	int m, n;
	float midmax=-100000, midmin=100000;
	int iter=0,move=0;
	float inter1[11][11]={0}, inter2[11][11]={0}, inter3[11][11]={0}, exter[11][11]={0}, energy[11][11]={0};//declare a 1D array to store the energy in the 7*7 window
	float red[11][11]={0}, green[11][11]={0}, blue[11][11]={0}, colordif[11][11]={0};
	int point=0;
	float maxi1, mini1, maxi2, mini2, mini3, maxi3, maxex, minex, maxcdif, mincdif;
	float sumx, sumy, sumr, sumg, sumb;
	int centx, centy;
	float sumdis,ave,scale1,scale2,scale3,scale4,scale5,mintotal;
	int n0,m0,n1,m1,n01,m01,pr,pc,sm,sn,sj,si;
	//declare memory for tempory images
	unsigned char * Gradient;
	float ** middle;
	FILE *fpt;
	int pix;
	Gradient = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
	middle = (float **)calloc(COLS, sizeof(float *));
	for(i=0; i<COLS; i++)
	{
		middle[i] = (float *)calloc(ROWS, sizeof(float));
	}
	hDC=GetDC(MainWnd);
	//compute the gradient image
	for(r=1; r<ROWS-1; r++){
		for (c=1; c<COLS-1; c++){
			Gx = 0; Gy = 0;
			for(p=-1; p<=1; p++){
				for(q=-1; q<=1; q++){
					Gx = Gx + (float)smoothed[(r+p)*COLS+c+q] * sobelx[1+q][1+p];
					Gy = Gy + (float)smoothed[(r+p)*COLS+c+q] * sobely[1+q][1+p];
				}
			}
			//middle[r][c] = (Gx * Gx + Gy * Gy);
			middle[c][r] = abs(Gx) + abs(Gy);
			if (middle[c][r]<midmin){midmin=middle[c][r];}
			if (middle[c][r]>midmax){midmax=middle[c][r];}
		}
	}
	for(r=1; r<ROWS-1; r++){
		for (c=1; c<COLS-1; c++){
			Gradient[r*COLS+c] = (middle[c][r]-midmin)*255/(midmax-midmin);
		}
	}
	//find the color around the center point
	sumr = 0; sumb = 0; sumg = 0;
	sumx = 0; sumy = 0;
	fpt=fopen("mancontour.txt","w");
	for (point=0; point<contnum; point++){
		n = sline[point][0];
		m = sline[point][1];
		fprintf(fpt,"%d %d\n",n, m);
		sumx = sumx + n;
		sumy = sumy + m;
	}
	fclose(fpt);
	centx = sumx/contnum;//x of center point
	centy = sumy/contnum;//y of center point
	for (r = -4; r <= 4; r++){
		for (c = -4; c<= 4; c++){
			sumr = sumr + (float)data[((r+centy)*COLS+c+centx)*3];
			sumg = sumg + (float)data[((r+centy)*COLS+c+centx)*3+1];
			sumb = sumb + (float)data[((r+centy)*COLS+c+centx)*3+2];
			//SetPixel(hDC,centx+c,centy+r,RGB(data[((r+centy)*COLS+c+centx)*3],data[((r+centy)*COLS+c+centx)*3+1],data[((r+centy)*COLS+c+centx)*3+2]));
		}
	}
	sumr = sumr/81.0;
	sumg = sumg/81.0;
	sumb = sumb/81.0;
	//active contour
	for (iter = 0; iter <=60; iter++){
		//PaintImage();
		move =0;
		sumdis = 0;
		n0 = sline[0][0];
		m0 = sline[0][1];
		//first calculate the average of all the distances
		//meanwhile, compute the center point of the contour
		for (point=0; point<contnum; point++){
			n = sline[point][0];
			m = sline[point][1];
			for (j = -3; j <= 3; j++){
				pix = OriginalImage[m*COLS+n+j];
				SetPixel(hDC,n+j,m,RGB(pix, pix, pix));
				pix = OriginalImage[(m+j)*COLS+n];
				SetPixel(hDC,n,m+j,RGB(pix, pix, pix));    
			}
			//the next point in the contour
			if (point==contnum-1){
				n1=n0;
				m1=m0;
			}else{
				n1 = sline[(point+1)][0];
				m1 = sline[(point+1)][1];
			}
			//the previous point in the contour 
			if (point==0){
				n01 = sline[contnum-1][0];
				m01 = sline[contnum-1][1];
			}else{
				n01 = sline[(point-1)][0];
				m01 = sline[(point-1)][1];
			}
			sumdis = sumdis + (n-n1)*(n-n1)+(m-m1)*(m-m1);
		}
		ave = sumdis/contnum;

		//printf("ave %f",ave);
		//for each point, calculate the energies
		for (point=0; point<contnum; point++){
			n = sline[point][0];
			m = sline[point][1];
			if (point==contnum-1){
				//n1=n0;
				//m1=m0;
				n1=sline[0][0];
				m1=sline[0][1];
			}else{
				n1 = sline[(point+1)][0];
				m1 = sline[(point+1)][1];
			}
			//printf("n %d, m %d",n,m);// n1 %f, m1 %f.\n",n,m,n1,m1); 
			maxi1 = -10000000; mini1 = 10000000;
			maxi2 = -10000000; mini2 = 10000000;
			maxex = -10000000; minex = 10000000;
			maxcdif = -10000000; mincdif = 10000000;
			//for each pixel in the 7*7 window
			for (r = (-1)*wind; r <= wind; r++){
				for (c = (-1)*wind; c <= wind; c++){
					if((n+c>0) && (m+r>0) && (n+c<COLS) && (m+r<ROWS) && (((r+m)*COLS+c+n)*3+2>0) && (((r+m)*COLS+c+n)*3+2<total)){
						//calculate inter energy 1
						inter1[wind+c][wind+r] = (float)((n+c-n1)*(n+c-n1)+(m+r-m1)*(m+r-m1));

						//calculate inter energy 2
						//inter2[3+r][3+c] = 0;
						inter2[wind+c][wind+r] = (float)((ave - inter1[wind+c][wind+r])*(ave - inter1[wind+c][wind+r]));
						//inter2[3+r][3+c] = (float)(abs(ave - inter1[3+r][3+c]));

						//average distance (using the prvious point)
						inter3[wind+c][wind+r] = (float)(SQR(ave - (float)(SQR(n+c-n01)+SQR(m+r-m01))));

						//calculate the externel energy by convolution
						exter[wind+c][wind+r] = middle[n+c][m+r];

						//calculate the red, green and blue colors
						red[wind+c][wind+r] = (abs((float)data[((r+m)*COLS+c+n)*3]-sumr));
						green[wind+c][wind+r] = (abs((float)data[((r+m)*COLS+c+n)*3+1]-sumg));
						blue[wind+c][wind+r] = (abs((float)data[((r+m)*COLS+c+n)*3+2]-sumb));
						colordif[wind+c][wind+r] = red[wind+c][wind+r]+green[wind+c][wind+r]+blue[wind+c][wind+r];
						//printf ("exter %f\n",exter[3+i][3+j]);
						//printf ("energy %f\n",energy[3+i][3+j]);
						//find the minimum and maximum value
						if (inter1[wind+c][wind+r] > maxi1){ maxi1 = inter1[wind+c][wind+r];}
						if (inter1[wind+c][wind+r] < mini1){ mini1 = inter1[wind+c][wind+r];}
						if (inter2[wind+c][wind+r] > maxi2){ maxi2 = inter2[wind+c][wind+r];}
						if (inter2[wind+c][wind+r] < mini2){ mini2 = inter2[wind+c][wind+r];}
						if (inter3[wind+c][wind+r] > maxi1){ maxi3 = inter3[wind+c][wind+r];}
						if (inter3[wind+c][wind+r] < mini1){ mini3 = inter3[wind+c][wind+r];}
						if (exter[wind+c][wind+r] > maxex){ maxex = exter[wind+c][wind+r];}
						if (exter[wind+c][wind+r] < minex){ minex = exter[wind+c][wind+r];}
						if (colordif[wind+c][wind+r] > maxcdif){ maxcdif = colordif[wind+c][wind+r];}
						if (colordif[wind+c][wind+r] < mincdif){ mincdif = colordif[wind+c][wind+r];}
					}
				}      
			}

			scale1 = maxi1 - mini1;
			scale2 = maxi2 - mini2;
			scale3 = maxex - minex;
			scale5 = maxi3 - mini3;
			scale4 = maxcdif - mincdif;
			if(scale1 == 0){scale1 = 1;}
			if(scale2 == 0){scale2 = 1;}
			if(scale3 == 0){scale3 = 1;}
			if(scale4 == 0){scale4 = 1;}
			if(scale5 == 0){scale5 = 1;}
			mintotal = 1000000;
			pr = 0, pc = 0;
			//normalize the values

			for (r = -wind; r <= wind; r++){
				for (c = -wind; c <= wind; c++){
					if((n+c>0) && (m+r>0) && (n+c<COLS) && (m+r<ROWS) && (((r+m)*COLS+c+n)*3+2>0) && (((r+m)*COLS+c+n)*3+2<total)){
						inter1[wind+c][wind+r] = (inter1[wind+c][wind+r]-mini1)/scale1;
						inter2[wind+c][wind+r] = (inter2[wind+c][wind+r]-mini2)/scale2;
						inter3[wind+c][wind+r] = (inter3[wind+c][wind+r]-mini3)/scale5;
						exter[wind+c][wind+r] = (exter[wind+c][wind+r]-minex)/scale3;
						colordif[wind+c][wind+r] = (colordif[wind+c][wind+r]-mincdif)/scale4;
						// calculate the total energy
						energy[wind+c][wind+r] =(inter1[wind+c][wind+r])+2*(inter3[wind+c][wind+r])+(inter2[wind+c][wind+r])+(colordif[wind+c][wind+r])-(exter[wind+c][wind+r])/3;
						//energy[wind+c][wind+r] =(inter2[wind+c][wind+r])+(colordif[wind+c][wind+r])-(exter[wind+c][wind+r])/3;
						// find the minimum energy and its position
						if (energy[wind+c][wind+r] < mintotal) { 
							mintotal = energy[wind+c][wind+r];
							pr = r;
							pc = c;
							// printf("%d %d\n",pc,pr);
						}
					}
				}
			}

			// move the point in the contour
			if(point!=lockindex){
				sline[point][0] = n + pc;
				sline[point][1] = m + pr;
			}
			// check if contour moves
			if ((pc!=0)||(pr!=0)){
				move++;
			} 

		}
		//printf("move %d\n",move);
		if (move == 0){
			printf("stop %d\n",iter);
			break;
		}

		//for every pixel on the contour, draw a crossing
		for (si = 0; si < contnum; si++){
			sn = sline[si][0];//c
			sm = sline[si][1];//r
			for (sj = -3; sj <= 3; sj++){
				if(si==lockindex){
					SetPixel(hDC,sn+sj,sm,RGB(255,0,0));
					SetPixel(hDC,sn,sm+sj,RGB(255,0,0));
				}else{
					SetPixel(hDC,sn+sj,sm,RGB(0,0,255));
					SetPixel(hDC,sn,sm+sj,RGB(0,0,255));
				}
				// [(m+j)*COLS+n] = 0;
				// imagecopy[m*COLS+(n+j)] = 0;       
			}
		}
		Sleep(100);
	}
	done = 1;
	ReleaseDC(MainWnd,hDC);
	free(Gradient);
	for(i=0; i<ROWS; i++)
	{
		free(middle[i]);
	}
	free(middle);
}