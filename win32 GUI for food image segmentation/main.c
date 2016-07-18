
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"
#include "myfunc.h"


#define SQR(x) ((x)*(x))
#define MAX_QUEUE 10000	/* max perimeter size (pixels) of border wavefront */
HANDLE hevent;
HINSTANCE hInst;
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					 LPTSTR lpCmdLine, int nCmdShow)

{
	MSG			msg;
	HWND		hWnd;
	WNDCLASS	wc;
	hevent=CreateEvent(NULL,TRUE,FALSE,NULL);

	wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc=(WNDPROC)WndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=LoadIcon(hInstance,"ID_PLUS_ICON");
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName="ID_MAIN_MENU";
	wc.lpszClassName="PLUS";

	if (!RegisterClass(&wc))
		return(FALSE);

	hWnd=CreateWindow("PLUS","plus program",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT,0,400,400,NULL,NULL,hInstance,NULL);

	if (!hWnd)
		return(FALSE);

	ShowScrollBar(hWnd,SB_BOTH,FALSE);
	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);
	MainWnd=hWnd;

	strcpy(filename,"");
	OriginalImage=NULL;
	ROWS=COLS=0;

	InvalidateRect(hWnd,NULL,TRUE);
	UpdateWindow(hWnd);

	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return(msg.wParam);
}

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char key[10]="1234";
	switch(Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_OK:
			if(GetTex(hwnd, key))
			{
				EndDialog(hwnd, IDC_OK);
				return TRUE;
			}
			else 
			{
				MessageBox(NULL,"Error Message","Error",MB_OK | MB_ICONINFORMATION);
				EndDialog(hwnd, 1);
			}
			break;
			/*case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;*/
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


BOOL GetTex(HWND hWnd, char key[10])
{
	HWND h1,h2;
	char temp1[255]={0},temp2[255]={0};
	//float absdif=0;
	h1=GetDlgItem(hWnd,IDC_ABSDIF);	
	h2=GetDlgItem(hWnd,IDC_CENTDIS);	
	PostMessage (h1,EM_SETPASSWORDCHAR,0,0);
	PostMessage (h2,EM_SETPASSWORDCHAR,0,0);
	Sleep(10);
	SendMessage(h1,WM_GETTEXT,255,(LPARAM)temp1);
	SendMessage(h2,WM_GETTEXT,255,(LPARAM)temp2);
	//if(!strcmp(key,temp))
	if (strcmp("",temp1)||strcmp("",temp2))
	{
		MessageBox(NULL,"New values are set.","Message",MB_OK);
		absdif = atof(temp1);
		centdis = atof(temp2);
		return TRUE;
	}
	else
	{
		//MessageBox(NULL,"Unable to open file",filename,MB_OK | MB_APPLMODAL);
		MessageBox(NULL,"You haven't specify new values","Message",MB_OK);
		return TRUE;
	}
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg,
						  WPARAM wParam, LPARAM lParam)

{
	HMENU				hMenu;
	HWND		        hwnd;
	OPENFILENAME		ofn;
	FILE				*fpt, *fpt2;
	HDC					hDC;
	char				header[320],text[320],key[10];
	int					BYTES,xPos,yPos;
	int					r,c,r2,c2,sum;
	int                 ret;


	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_DRAW:
			draw = 1;
			drag = 0;
			p = 0;
			//PaintImage();
			break;

		case ID_DISPLAY_REGIONGROW:

			regionGrow=(regionGrow+1)%2;
			if(regionGrow == 1)
			{
				DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DIALOG1),hWnd,AboutDlgProc);
			}
			//return 0;
			break;

		case ID_DISPLAY_CLEAR:
			for (i = 0; i < contnum; i++)
			{
				//clear sline
				sline[i][0] = 0;
				sline[i][1] = 0;
			}
			p = 0;
			PaintImage();
			break;

		case ID_REGIONGROW_RED:
			blue=0; green=0; yellow=0; cyan=0;
			red=(red+1)%2;
			break;

		case ID_REGIONGROW_GREEN:
			blue=0; red=0; yellow=0; cyan=0;
			green=(green+1)%2;
			break;

		case ID_REGIONGROW_BLUE:
			red=0; green=0; yellow=0; cyan=0;
			blue=(blue+1)%2;
			break;

		case ID_REGIONGROW_YELLOW:
			blue=0; green=0; red=0; cyan=0;
			yellow=(yellow+1)%2;
			break;

		case ID_REGIONGROW_CYAN:
			blue=0; green=0; yellow=0; red=0;
			cyan=(cyan+1)%2;
			break;

		case ID_DRAG:
			draw=0;
			lock=0;
			drag=(drag+1)%2;
			break;

			/*case ID_DISPLAY_TEST:
			CreateWindow("BUTTON", 0, WS_CHILD|WS_VISIBLE, 70, 70, 80, 25, hWnd, (HMENU)ID_DISPLAY_TEST, hInstance,0);
			break;*/


		case ID_FILE_LOAD:
			if (OriginalImage != NULL)
			{
				free(OriginalImage);
				free(data);
				free(smoothed);
				data=NULL;
				smoothed=NULL;
				OriginalImage=NULL;
			}
			memset(&(ofn),0,sizeof(ofn));
			ofn.lStructSize=sizeof(ofn);
			ofn.lpstrFile=filename;
			filename[0]=0;
			ofn.nMaxFile=MAX_FILENAME_CHARS;
			ofn.Flags=OFN_EXPLORER | OFN_HIDEREADONLY;
			ofn.lpstrFilter = "PNM files\0*.pnm\0All files\0*.*\0\0";
			if (!( GetOpenFileName(&ofn))  ||  filename[0] == '\0')
				break;		/* user cancelled load */
			if ((fpt=fopen(filename,"rb")) == NULL)
			{
				MessageBox(NULL,"Unable to open file",filename,MB_OK | MB_APPLMODAL);
				break;
			}
			fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
			if (strcmp(header,"P6") != 0  ||  BYTES != 255)
			{
				MessageBox(NULL,"Not a PNM (P6 Color) image",filename,MB_OK | MB_APPLMODAL);
				fclose(fpt);
				break;
			}
			total = COLS*ROWS*3;
			data = (unsigned char *)calloc(ROWS*COLS*3,1);
			OriginalImage=(unsigned char *)calloc(ROWS*COLS,1);
			smoothed=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

			header[0]=fgetc(fpt);	/* whitespace character after header */
			fread(data,1,ROWS*COLS*3,fpt);
			fclose(fpt);
			for (i = 0; i<=ROWS*COLS; i++)
			{
				OriginalImage[i] = (data[i*3]+data[i*3+1]+data[i*3+2])/3;
			}
			for (r=2; r<ROWS-2; r++){
				for (c=2; c<COLS-2; c++)
				{
					sum=0;
					for (r2=-2; r2<=2; r2++)
						for (c2=-2; c2<=2; c2++)
							sum+=OriginalImage[(r+r2)*COLS+(c+c2)];
					smoothed[r*COLS+c]=sum/25;
				}
			}

			SetWindowText(hWnd,filename);
			PaintImage();
			break;

		case ID_FILE_QUIT:
			DestroyWindow(hWnd);
			break;
		}
		break;
	case WM_SIZE:		  /* could be used to detect when window size changes */
		PaintImage();
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_PAINT:
		PaintImage();
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;

	case WM_LBUTTONDOWN:
		if(drag==1 && OriginalImage!=NULL && done == 1){
			hDC=GetDC(MainWnd);
			xPos=LOWORD(lParam);
			yPos=HIWORD(lParam);
			for(i=0; i<contnum; i++){
				if ((sline[i][0]==xPos) && (sline[i][1]==yPos)){
					lockindex = i;
					lockx = xPos;
					locky = yPos;
					lock = 1;
					for (j = -3; j <= 3; j++){
						value = OriginalImage[yPos*COLS+xPos+j];
						SetPixel(hDC,xPos+j,yPos,RGB(value, value, value));
						value = OriginalImage[(yPos+j)*COLS+xPos];
						SetPixel(hDC,xPos,yPos+j,RGB(value, value, value));
						// [(m+j)*COLS+n] = 0;
						// imagecopy[m*COLS+(n+j)] = 0;       
					}
					//SetPixel(hDC,xPos,yPos,RGB(value, value, value));
				}
			}			
		}

		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;

	case WM_RBUTTONDOWN:
		if(OriginalImage!= NULL)
		{
			clickx=LOWORD(lParam);
			clicky=HIWORD(lParam);
			done = 0;
			balloon(clickx, clicky);
		}
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_LBUTTONUP:
		if ((draw == 1)&& (OriginalImage!=NULL))
		{
			fpt=fopen("contour.txt","w");
			fpt2=fopen("test.txt","w");
			contnum = 0;
			for (i=0; i<p-1; i++)
			{
				if (i % 5 == 0){
					contnum++;
					fprintf(fpt,"%d %d\n",line[i][0], line[i][1]);
					sline[i/5][0] = line[i][0];
					sline[i/5][1] = line[i][1];
				}
				fprintf(fpt2,"%d %d\n",line[i][0], line[i][1]);
			}
			//fprintf(fpt,"%d\n",q);
			fclose(fpt);
			fclose(fpt2);
			if (contnum!=0){
				done = 0;
				_beginthread(AnimationThread,0,MainWnd);
			}
			p = 0;		
		}

		if(drag == 1 && lock == 1 && done == 1){
			hDC=GetDC(MainWnd);
			xPos=LOWORD(lParam);
			yPos=HIWORD(lParam);
			sline[lockindex][0] = xPos;
			sline[lockindex][1] = yPos;
			for (j = -3; j <= 3; j++){
				SetPixel(hDC,xPos+j,yPos,RGB(255,0,0));
				SetPixel(hDC,xPos,yPos+j,RGB(255,0,0));
						// [(m+j)*COLS+n] = 0;
						// imagecopy[m*COLS+(n+j)] = 0;       
			}
			
			//SetPixel(hDC,xPos,yPos,RGB(255,0,0));
			ReleaseDC(MainWnd,hDC);
			done = 0;
			Manual();
			lock = 0;
		}
		break;
	case WM_MOUSEMOVE:
		if ((draw == 1) && (wParam & WM_LBUTTONDOWN) && (OriginalImage!=NULL))
		{
			done = 0;
			xPos=LOWORD(lParam);
			yPos=HIWORD(lParam);
			if (xPos >= 0  &&  xPos < COLS  &&  yPos >= 0  &&  yPos < ROWS)
			{
				hDC=GetDC(MainWnd);
				line[p][0] = xPos;
				line[p][1] = yPos;
				p++;
				//draw larger points 3x3
				for (i = -1; i <= 1; i++)
				{
					for (j = -1; j <=1; j++)
					{
						SetPixel(hDC,xPos+i,yPos+j,RGB(255,0,0));
					}
				}
			}

		}


		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;

	case WM_KEYDOWN:
		if (wParam == 's'  ||  wParam == 'S')
			PostMessage(MainWnd,WM_COMMAND,ID_SHOWPIXELCOORDS,0);	  /* send message to self */
		if ((TCHAR)wParam == '1')
		{
			//TimerRow=TimerCol=0;
			SetTimer(MainWnd,TIMER_SECOND,10,NULL);	/* start up 10 ms timer */
		}

		if ((TCHAR)wParam == '2')
		{
			KillTimer(MainWnd,TIMER_SECOND);			/* halt timer, stopping generation of WM_TIME events */
			PaintImage();								/* redraw original image, erasing animation */
		}

		if ((TCHAR)wParam == '3')
		{
			ThreadRunning=1;
			//_beginthread(AnimationThread,0,MainWnd);	/* start up a child thread to do other work while this thread continues GUI */
		}

		if ((TCHAR)wParam == '4')
		{
			ThreadRunning=0;							/* this is used to stop the child thread (see its code below) */
		}

		if ((TCHAR)wParam == 0x4A)
		{
			SetEvent(hevent);							/* this is used to stop the child thread (see its code below) */
		}

		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_TIMER:	  /* this event gets triggered every time the timer goes off */
		hDC=GetDC(MainWnd);
		SetPixel(hDC,TimerCol,TimerRow,RGB(0,0,255));	/* color the animation pixel blue */
		ReleaseDC(MainWnd,hDC);
		TimerRow++; 
		TimerCol+=2;
		break;
	case WM_HSCROLL:	  /* this event could be used to change what part of the image to draw */
		PaintImage();	  /* direct PaintImage calls eliminate flicker; the alternative is InvalidateRect(hWnd,NULL,TRUE); UpdateWindow(hWnd); */
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_VSCROLL:	  /* this event could be used to change what part of the image to draw */
		PaintImage();
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	}

	hMenu=GetMenu(MainWnd);
	if (draw == 1)
		CheckMenuItem(hMenu,ID_DRAW,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_DRAW,MF_UNCHECKED);

	if (drag == 1)
		CheckMenuItem(hMenu,ID_DRAG,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_DRAG,MF_UNCHECKED);
	if (red == 1)
		CheckMenuItem(hMenu,ID_REGIONGROW_RED,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_REGIONGROW_RED,MF_UNCHECKED);
	if (green == 1)
		CheckMenuItem(hMenu,ID_REGIONGROW_GREEN,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_REGIONGROW_GREEN,MF_UNCHECKED);
	if (yellow == 1)
		CheckMenuItem(hMenu,ID_REGIONGROW_YELLOW,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_REGIONGROW_YELLOW,MF_UNCHECKED);
	if (blue == 1)
		CheckMenuItem(hMenu,ID_REGIONGROW_BLUE,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_REGIONGROW_BLUE,MF_UNCHECKED);
	if (cyan == 1)
		CheckMenuItem(hMenu,ID_REGIONGROW_CYAN,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_REGIONGROW_CYAN,MF_UNCHECKED);
	if (playmode == 1)
		CheckMenuItem(hMenu,ID_SELECTREGIONGROWMODE_PLAY,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_SELECTREGIONGROWMODE_PLAY,MF_UNCHECKED);
	if (stepmode == 1)
		CheckMenuItem(hMenu,ID_SELECTREGIONGROWMODE_STEP,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_SELECTREGIONGROWMODE_STEP,MF_UNCHECKED);


	DrawMenuBar(hWnd);
	return(0L);
}




void PaintImage()

{
	PAINTSTRUCT			Painter;
	HDC					hDC;
	BITMAPINFOHEADER	bm_info_header;
	BITMAPINFO			*bm_info;
	int					i,r,c,DISPLAY_ROWS,DISPLAY_COLS;
	unsigned char		*DisplayImage;

	if (OriginalImage == NULL)
		return;		/* no image to draw */

	/* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
	DISPLAY_ROWS=ROWS;
	DISPLAY_COLS=COLS;
	if (DISPLAY_ROWS % 4 != 0)
		DISPLAY_ROWS=(DISPLAY_ROWS/4+1)*4;
	if (DISPLAY_COLS % 4 != 0)
		DISPLAY_COLS=(DISPLAY_COLS/4+1)*4;
	DisplayImage=(unsigned char *)calloc(DISPLAY_ROWS*DISPLAY_COLS,1);
	for (r=0; r<ROWS; r++)
		for (c=0; c<COLS; c++)
			DisplayImage[r*DISPLAY_COLS+c]=OriginalImage[r*COLS+c];

	BeginPaint(MainWnd,&Painter);
	hDC=GetDC(MainWnd);
	bm_info_header.biSize=sizeof(BITMAPINFOHEADER); 
	bm_info_header.biWidth=DISPLAY_COLS;
	bm_info_header.biHeight=-DISPLAY_ROWS; 
	bm_info_header.biPlanes=1;
	bm_info_header.biBitCount=8; 
	bm_info_header.biCompression=BI_RGB; 
	bm_info_header.biSizeImage=0; 
	bm_info_header.biXPelsPerMeter=0; 
	bm_info_header.biYPelsPerMeter=0;
	bm_info_header.biClrUsed=256;
	bm_info_header.biClrImportant=256;
	bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
	bm_info->bmiHeader=bm_info_header;
	for (i=0; i<256; i++)
	{
		bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
		bm_info->bmiColors[i].rgbReserved=0;
	} 

	SetDIBitsToDevice(hDC,0,0,DISPLAY_COLS,DISPLAY_ROWS,0,0,
		0, /* first scan line */
		DISPLAY_ROWS, /* number of scan lines */
		DisplayImage,bm_info,DIB_RGB_COLORS);
	ReleaseDC(MainWnd,hDC);
	EndPaint(MainWnd,&Painter);

	free(DisplayImage);
	free(bm_info);
}


/*HDC hDC; 
//LPARAM lParam;
int xPos, yPos;
unsigned char	*labels;
int		*indices,RegionSize, TotalRegions;
indices=(int *)calloc(ROWS*COLS,sizeof(int));
labels=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));	
xPos = clickx;
yPos = clicky;
//xPos=LOWORD(lParam);
//yPos=HIWORD(lParam);
TotalRegions = 128;
if (xPos >= 0  &&  xPos < COLS  &&  yPos >= 0  &&  yPos < ROWS)
{
//RegionGrow(OriginalImage,labels,ROWS,COLS,xPos,yPos,0,TotalRegions,indices,&RegionSize);
{
int	r2,c2,r,c;
int	queue[MAX_QUEUE],qh,qt;
int	average=0,total=0,rtotal=0,ctotal=0,ravrg=0,cavrg=0,distance=0;	/* average and total intensity in growing region */
/*		int paint_over_label=0;
int new_label=128;
COLORREF rcol=RGB(255,0,0),gcol=RGB(0,255,0),bcol=RGB(0,0,255),ycol=RGB(255,255,0),ccol=RGB(0,255,255);
//unsigned char *image;
//image=OriginalImage;
int count=1;
r=yPos;
c=xPos;
if (labels[r*COLS+c] != paint_over_label)
return;

//labels[r*COLS+c]=new_label;
//SetTimer(MainWnd,TIMER_SECOND,1000,NULL);
//---------------------------------------
hDC=GetDC(MainWnd);
if (red == 1){SetPixel(hDC,xPos,yPos,rcol);}
else if (yellow == 1){SetPixel(hDC,xPos,yPos,ycol);}
else if (green == 1){SetPixel(hDC,xPos,yPos,gcol);}
else if (blue == 1){SetPixel(hDC,xPos,yPos,bcol);}
else if (cyan == 1){SetPixel(hDC,xPos,yPos,ccol);}
else {SetPixel(hDC,xPos,yPos,rcol);}
//---------------------------------------------------
average=total=(int)OriginalImage[r*COLS+c];
rtotal=r;
ctotal=c;
ravrg=r;
cavrg=c;
if (indices != NULL)
indices[0]=r*COLS+c;
queue[0]=r*COLS+c;
qh=1;	/* queue head */
//		qt=0;	/* queue tail */
//(*count)=1;

/*	while (qt != qh)
{
if ((count)%50 == 0)	/* recalculate average after each 50 pixels join */
/*		{
average=total/(count);
ravrg=rtotal/(count);
cavrg=ctotal/(count);
// printf("new avg=%d\n",average);
}
for (r2=-1; r2<=1; r2++)
for (c2=-1; c2<=1; c2++)
{
if (r2 == 0  &&  c2 == 0)
continue;
if ((queue[qt]/COLS+r2) < 0  ||  (queue[qt]/COLS+r2) >= ROWS  ||
(queue[qt]%COLS+c2) < 0  ||  (queue[qt]%COLS+c2) >= COLS)
continue;
if (labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=paint_over_label)
continue;
/* test criteria to join region */
/*					distance = sqrt((float)((queue[qt]/COLS-ravrg)*(queue[qt]/COLS-ravrg)
+(queue[qt]%COLS-cavrg)*(queue[qt]%COLS-cavrg)));
if ((abs((int)(OriginalImage[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2])
-average) > absdif)||(distance> centdis))//  
continue;
//if pressed 'j'
labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]=new_label;
if (ThreadRunning == 0)
{
if (red == 1){SetPixel(hDC,queue[qt]%COLS+c2,queue[qt]/COLS+r2,rcol);}
else if (yellow == 1){SetPixel(hDC,queue[qt]%COLS+c2,queue[qt]/COLS+r2,ycol);}
else if (green == 1){SetPixel(hDC,queue[qt]%COLS+c2,queue[qt]/COLS+r2,gcol);}
else if (blue == 1){SetPixel(hDC,queue[qt]%COLS+c2,queue[qt]/COLS+r2,bcol);}
else if (cyan == 1){SetPixel(hDC,queue[qt]%COLS+c2,queue[qt]/COLS+r2,ccol);}
else {SetPixel(hDC,queue[qt]%COLS+c2,queue[qt]/COLS+r2,rcol);}
}
if (playmode == 1)
{
Sleep(1);//Sleep for 1ms
}
if (stepmode == 1)
{
ResetEvent(hevent);
WaitForSingleObject(hevent,INFINITE);
}
if (indices != NULL)
indices[count]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
total+=OriginalImage[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2];
rtotal+=queue[qt]/COLS+r2;
ctotal+=queue[qt]%COLS+c2;
(count)++;
queue[qh]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;

qh=(qh+1)%MAX_QUEUE;
if (qh == qt)
{
printf("Max queue size exceeded\n");
exit(0);
}
}
qt=(qt+1)%MAX_QUEUE;
}
}
}
ReleaseDC(MainWnd,hDC);
}*/

