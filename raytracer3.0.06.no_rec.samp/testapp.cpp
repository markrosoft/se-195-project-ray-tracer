// 3.0.02
#define _CRT_SECURE_NO_WARNINGS 1

#include <iostream> // added
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "raytracer.h"
#include "scene.h"
#include "surface.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define SCRWIDTH	800
#define SCRHEIGHT	600

Surface * surface = 0;
Pixel* buffer = 0;

/* Options */
// either one of the CPU or GPU versions on Open CL
int useOpenCL = 1;
// If using OpenCL, set value to use the GPU ( 1 ) or CPU ( 0 )
int useGPU = 1;

char* openCLcode();
void AllocateBuffers();
void SetKernelArguments();
void ExecuteKernel();
void ReadKernelBuffer();

extern std::string outputLine1;
extern std::string outputLine2;
extern std::string outputLine3;
extern std::string outputLine4;
extern std::string outputLine5;
extern std::string outputLine6;
extern std::string outputLine7;
extern std::string outputLine8;
extern std::string outputLine9;

void DrawWindow(int runCnt);

void GetPixelColor(Pixel pixel, int & r, int & g, int & b) {
 b = pixel & 0x000000ff;
 g = (pixel & 0x0000ff00) >> 8;
 r = (pixel & 0x00ff0000) >> 16;
}


int main(int argc, char * argv[]) {
	// prepare output canvas
	surface = Surface_Create( SCRWIDTH, SCRHEIGHT );
	buffer = Surface_GetBuffer(surface);
	Surface_Clear(surface, 0);
	Surface_InitCharset(surface);
	Surface_Print(surface, "timings:", 2, 2, 0xffffffff );
	// prepare renderer
	Engine_Constructor();
	//tracer = new Raytracer::Engine();
	TracedRays_init();
	Scene_InitScene();
	Engine_SetTarget(Surface_GetBuffer(surface), SCRWIDTH, SCRHEIGHT );	

	// Added
	if(useOpenCL){
	char* retFromOpenCLcode = openCLcode();
	char charBuffer[1024];
	sprintf(charBuffer, "%s", retFromOpenCLcode);
	Surface_Print(surface, charBuffer, 5, 530, 0xffffffff);
	sprintf(charBuffer, "%s", outputLine2.c_str());
	Surface_Print(surface, charBuffer, 5, 538, 0xffffffff);
	sprintf(charBuffer, "%s", outputLine3.c_str());
	Surface_Print(surface, charBuffer, 5, 546, 0xffffffff);
	sprintf(charBuffer, "%s", outputLine4.c_str());
	Surface_Print(surface, charBuffer, 5, 554, 0xffffffff);
	sprintf(charBuffer, "%s", outputLine5.c_str());
	Surface_Print(surface, charBuffer, 5, 562, 0xffffffff);
	sprintf(charBuffer, "%s", outputLine6.c_str());
	Surface_Print(surface, charBuffer, 5, 570, 0xffffffff);
	sprintf(charBuffer, "%s", outputLine7.c_str());
	Surface_Print(surface, charBuffer, 5, 578, 0xffffffff);
	sprintf(charBuffer, "%s", outputLine8.c_str());
	Surface_Print(surface, charBuffer, 5, 586, 0xffffffff);
	sprintf(charBuffer, "%s", outputLine9.c_str());
	Surface_Print(surface, charBuffer, 5, 594, 0xffffffff);
	}
	// end of added

	// go
	int xpos = 60;
	int ypos = 2;
	int maxx = 1;
	int maxy = 1;
	int maxrun = maxx*maxy;
	int runCnt = 0;
	int totftime = 0;
	float avftime = 0;
	 
	//------------------------------------------------------
	if(useOpenCL){
		// Allocate global memory for GPU of m_Dest
		AllocateBuffers(); 
		//// Create kernel arguments -> m_Dest copy, height, width
		SetKernelArguments();
		//// execute kernel
		//ExecuteKernel();
		//// read and copy kernel buffer to m_Dest
		//ReadKernelBuffer();
	}
	//------------------------------------------------------
	while (runCnt < maxrun)  // draw only runCnt times, not forever
	//while (1)
	{
		int fstart = GetTickCount();
		Engine_InitRender();
		// get rid of timeout, draw the damned picture
		if(!useOpenCL) { Engine_Render();}
		//Engine_Render_Simple(); // fill m_Dest with red pixels
		//DrawWindow(); 
		//------------------------------------------------------
		if(useOpenCL){
			// Allocate global memory for GPU of m_Dest
			AllocateBuffers(); 
			//// Create kernel arguments -> m_Dest copy, height, width
			SetKernelArguments();
			//// execute kernel
			ExecuteKernel();
			//// read and copy kernel buffer to m_Dest
			ReadKernelBuffer();
		}
		//------------------------------------------------------

		//DrawWindow(runCnt); //needs m_Dest populated
		// end
		int ftime = GetTickCount() - fstart;
		totftime += ftime;
		char t[] = "00:00.000";
		t[6] = (ftime / 100) % 10 + '0';
		t[7] = (ftime / 10) % 10 + '0';
		t[8] = (ftime % 10) + '0';
		int secs = (ftime / 1000) % 60;
		int mins = (ftime / 60000) % 100;
		t[3] = ((secs / 10) % 10) + '0';
		t[4] = (secs % 10) + '0';
		t[1] = (mins % 10) + '0';
		t[0] = ((mins / 10) % 10) + '0';
		Surface_Print(surface,  t, xpos, ypos, 0xffffffff );
		printf("%s\n", t);
		if(runCnt%maxx==maxx-1){ypos+=8;xpos=60;}
		else{xpos+=65;}

		runCnt++;
	}
	avftime = (float)totftime /(float)maxrun;
	char k[] = "00:00.000";
	k[6] = ((int)avftime / 100) % 10 + '0';
	k[7] = ((int)avftime / 10) % 10 + '0';
	k[8] = ((int)avftime % 10) + '0';
	int secs = ((int)avftime / 1000) % 60;
	int mins = ((int)avftime / 60000) % 100;
	k[3] = ((secs / 10) % 10) + '0';
	k[4] = (secs % 10) + '0';
	k[1] = (mins % 10) + '0';
	k[0] = ((mins / 10) % 10) + '0';
	Surface_Print(surface, "average: ", xpos, ypos, 0xffffffff );
	Surface_Print(surface,  k, xpos+60, ypos, 0xffffffff );
	printf("average of %d runs: %s\n", runCnt, k);
	DrawWindow(runCnt);
	//system("PAUSE");
	return 1;
}

void DrawWindow(int runCnt)
{	
	char fname[1024];
	sprintf(fname, "output%d.ppm", runCnt);
	FILE * outfile = fopen(fname, "w");
	fprintf(outfile, "P3\n800 600\n255\n");
	int i = 0;
	for (int r = 0; r < 600; r++) {
		for (int c = 0; c < 800; c++) {
			int r, g, b;
			GetPixelColor(buffer[i], r, g, b);
			fprintf(outfile, "%d %d %d ", r, g, b);
			++i;
			if (c % 5 == 0)
				fprintf(outfile, "\n");
		}
		fprintf(outfile, "\n");
	}
	fclose(outfile);
}
