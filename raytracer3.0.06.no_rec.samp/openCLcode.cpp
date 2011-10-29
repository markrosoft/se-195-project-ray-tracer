#define _CRT_SECURE_NO_WARNINGS 1

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "raytracer.h"
#include "scene.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

extern int useGPU;

static int forceWorkSize = 0;
std::string outputLine1 = "1:";
std::string outputLine2 = "2:";
std::string outputLine3 = "3:";
std::string outputLine4 = "4:";
std::string outputLine5 = "5:";
std::string outputLine6 = "6:";
std::string outputLine7 = "7:";
std::string outputLine8 = "8:";
std::string outputLine9 = "9:";

/* OpenCL variables */
static cl_context context;
static cl_mem colorBuffer;
static cl_mem m_DestBuffer;
static cl_mem primitiveBuffer;
static cl_mem tracedRayBuffer;
static cl_mem cameraBuffer;
static cl_command_queue commandQueue;
static cl_program program;
static cl_kernel kernel;
static unsigned int workGroupSize = 1;
static char *kernelFileName = "openCLcode.cl";

extern int m_Width, m_Height;
extern Pixel* m_Dest;
//extern vector3 * camera;
static vector3 * colors;

static void FreeBuffers() {
	cl_int status = clReleaseMemObject(m_DestBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to release OpenCL pixel buffer: %d\n", status);
		exit(-1);
    }

	status = clReleaseMemObject(primitiveBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to release OpenCL pixel buffer: %d\n", status);
		exit(-1);
    }
}

void AllocateBuffers() {
	const int pixelCount = m_Width * m_Height;
	cl_int status;	

	cl_uint sizeBytes = sizeof(unsigned int) * pixelCount;
    m_DestBuffer = clCreateBuffer(
            context,
            CL_MEM_READ_WRITE,
            sizeBytes,
            NULL,
            &status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL output buffer: %d\n", status);
		exit(-1);
    }
	status = clEnqueueWriteBuffer(
			commandQueue,
			m_DestBuffer,
			CL_TRUE,
			0,
			sizeBytes,
			m_Dest,
			0,
			NULL,
			NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL seeds buffer: %d\n", status);
		exit(-1);
	}	

	int primCount = m_Scene->m_Primitives;
	sizeBytes = sizeof(Primitive) * primCount;
	primitiveBuffer = clCreateBuffer(
            context,
			CL_MEM_READ_WRITE,
            sizeBytes,
            NULL,
            &status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL scene buffer: %d\n", status);
		exit(-1);
    }
	status = clEnqueueWriteBuffer(
			commandQueue,
			primitiveBuffer,
			CL_TRUE,
			0,
			sizeBytes,
			m_Scene->m_Primitive,
			0,
			NULL,
			NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL scene buffer: %d\n", status);
		exit(-1);
	}
}

static char *ReadSources(const char *fileName) {
	FILE *file = fopen(fileName, "rb");
	if (!file) {
		fprintf(stderr, "Failed to open file '%s'\n", fileName);
		exit(-1);
	}

	if (fseek(file, 0, SEEK_END)) {
		fprintf(stderr, "Failed to seek file '%s'\n", fileName);
		exit(-1);
	}

	long size = ftell(file);
	if (size == 0) {
		fprintf(stderr, "Failed to check position on file '%s'\n", fileName);
		exit(-1);
	}

	rewind(file);

	char *src = (char *)malloc(sizeof(char) * size + 1);
	if (!src) {
		fprintf(stderr, "Failed to allocate memory for file '%s'\n", fileName);
		exit(-1);
	}

	fprintf(stderr, "Reading file '%s' (size %ld bytes)\n", fileName, size);
	size_t res = fread(src, 1, sizeof(char) * size, file);
	if (res != sizeof(char) * size) {
		fprintf(stderr, "Failed to read file '%s' (read %ld)\n", fileName, res);
		exit(-1);
	}
	src[size] = '\0'; /* NULL terminated */

	fclose(file);

	return src;

}

char* openCLcode() {

	cl_device_type dType;
	char buffer[1024];

	if (useGPU)
		dType = CL_DEVICE_TYPE_GPU;
	else
		dType = CL_DEVICE_TYPE_CPU;

	// Select the platform

    cl_uint numPlatforms;
	cl_platform_id platform = NULL;
	cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to get OpenCL platforms\n");
		exit(-1);
	}

	if (numPlatforms > 0) {
		cl_platform_id *platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * numPlatforms);
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
			if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL platform IDs\n");
			exit(-1);
		}

		unsigned int i;
		for (i = 0; i < numPlatforms; ++i) {
			char pbuf[100];
			status = clGetPlatformInfo(platforms[i],
					CL_PLATFORM_VENDOR,
					sizeof(pbuf),
					pbuf,
					NULL);

			status = clGetPlatformIDs(numPlatforms, platforms, NULL);
			if (status != CL_SUCCESS) {
				fprintf(stderr, "Failed to get OpenCL platform IDs\n");
				exit(-1);
			}

			fprintf(stderr, "OpenCL Platform %d: %s\n", i, pbuf);
			sprintf_s(buffer, "OpenCL Platform %d: %s      ", i, pbuf);
			outputLine1 += buffer;
		}

		platform = platforms[0];
		free(platforms);
	}

	// Select the device

	cl_device_id devices[32];
	cl_uint deviceCount;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 32, devices, &deviceCount);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to get OpenCL device IDs\n");
		exit(-1);
	}

	int deviceFound = 0;
	cl_device_id selectedDevice;
	unsigned int i;
	for (i = 0; i < deviceCount; ++i) {
		cl_device_type type = 0;
		status = clGetDeviceInfo(devices[i],
				CL_DEVICE_TYPE,
				sizeof(cl_device_type),
				&type,
				NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		char *stype;
		switch (type) {
			case CL_DEVICE_TYPE_ALL:
				stype = "TYPE_ALL";
				break;
			case CL_DEVICE_TYPE_DEFAULT:
				stype = "TYPE_DEFAULT";
				break;
			case CL_DEVICE_TYPE_CPU:
				stype = "TYPE_CPU";
				if (!useGPU && !deviceFound) {
					selectedDevice = devices[i];
					deviceFound = 1;
				}
				break;
			case CL_DEVICE_TYPE_GPU:
				stype = "TYPE_GPU";
				if (useGPU && !deviceFound) {
					selectedDevice = devices[i];
					deviceFound = 1;
				}
				break;
			default:
				stype = "TYPE_UNKNOWN";
				break;
		}
		fprintf(stderr, "OpenCL Device %d: Type = %s\n", i, stype);
		sprintf_s(buffer, "OpenCL Device %d: Type = %s    ", i, stype);
		outputLine2 += buffer;

		char buf[256];
		status = clGetDeviceInfo(devices[i],
				CL_DEVICE_NAME,
				sizeof(char[256]),
				&buf,
				NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "OpenCL Device %d: Name = %s\n", i, buf);
		sprintf_s(buffer, "OpenCL Device %d: Name = %s    ", i, buf);
		outputLine3 += buffer;

		cl_uint units = 0;
		status = clGetDeviceInfo(devices[i],
				CL_DEVICE_MAX_COMPUTE_UNITS,
				sizeof(cl_uint),
				&units,
				NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "OpenCL Device %d: Compute units = %u\n", i, units);
		sprintf_s(buffer, "OpenCL Device %d: Compute units = %u    ", i, units);
		outputLine4 += buffer;

		size_t gsize = 0;
		status = clGetDeviceInfo(devices[i],
				CL_DEVICE_MAX_WORK_GROUP_SIZE,
				sizeof(size_t),
				&gsize,
				NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "OpenCL Device %d: Max. work group size = %d\n", i, (unsigned int)gsize);
		sprintf_s(buffer, "OpenCL Device %d: Max. work group size = %d    ", i, (unsigned int)gsize);
		outputLine5 += buffer;
	}

	if (!deviceFound) {
		fprintf(stderr, "Unable to select an appropriate device\n");
		exit(-1);
	}

	// Create the context

	cl_context_properties cps[3] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties) platform,
		0
	};

	cl_context_properties *cprops = (NULL == platform) ? NULL : cps;
	context = clCreateContext(
			cprops,
			1,
			&selectedDevice,
			NULL,
			NULL,
			&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to open OpenCL context\n");
		exit(-1);
	}

    /* Get the device list data */
	size_t deviceListSize;
    status = clGetContextInfo(
            context,
            CL_CONTEXT_DEVICES,
            32,
            devices,
            &deviceListSize);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to get OpenCL context info: %d\n", status);
		exit(-1);
    }

	/* Print devices list */
	for (i = 0; i < deviceListSize / sizeof(cl_device_id); ++i) {
		cl_device_type type = 0;
		status = clGetDeviceInfo(devices[i],
				CL_DEVICE_TYPE,
				sizeof(cl_device_type),
				&type,
				NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		char *stype;
		switch (type) {
			case CL_DEVICE_TYPE_ALL:
				stype = "TYPE_ALL";
				break;
			case CL_DEVICE_TYPE_DEFAULT:
				stype = "TYPE_DEFAULT";
				break;
			case CL_DEVICE_TYPE_CPU:
				stype = "TYPE_CPU";
				break;
			case CL_DEVICE_TYPE_GPU:
				stype = "TYPE_GPU";
				break;
			default:
				stype = "TYPE_UNKNOWN";
				break;
		}
		fprintf(stderr, "[SELECTED] OpenCL Device %d: Type = %s\n", i, stype);
		sprintf_s(buffer, "[SELECTED] OpenCL Device %d: Type = %s    ", i, stype);
		outputLine6 += buffer;

		char buf[256];
		status = clGetDeviceInfo(devices[i],
				CL_DEVICE_NAME,
				sizeof(char[256]),
				&buf,
				NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "[SELECTED] OpenCL Device %d: Name = %s\n", i, buf);
		sprintf_s(buffer, "[SELECTED] OpenCL Device %d: Name = %s    ", i, buf);
		outputLine7 += buffer;

		cl_uint units = 0;
		status = clGetDeviceInfo(devices[i],
				CL_DEVICE_MAX_COMPUTE_UNITS,
				sizeof(cl_uint),
				&units,
				NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "[SELECTED] OpenCL Device %d: Compute units = %u\n", i, units);
		sprintf_s(buffer, "[SELECTED] OpenCL Device %d: Compute units = %u    ", i, units);
		outputLine8 += buffer;

		size_t gsize = 0;
		status = clGetDeviceInfo(devices[i],
				CL_DEVICE_MAX_WORK_GROUP_SIZE,
				sizeof(size_t),
				&gsize,
				NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "[SELECTED] OpenCL Device %d: Max. work group size = %d\n", i, (unsigned int)gsize);
		sprintf_s(buffer, "[SELECTED] OpenCL Device %d: Max. work group size = %d    ", i, (unsigned int)gsize);
		outputLine9 += buffer;
	}

	cl_command_queue_properties prop = 0;
	commandQueue = clCreateCommandQueue(
			context,
			devices[0],
			prop,
			&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL command queue: %d\n", status);
		exit(-1);
    }

	/*------------------------------------------------------------------------*/

	// here was the the create 
	// sphereBuffer, 
	// cameraBuffer

	/*------------------------------------------------------------------------*/

	/* Create the kernel program */
	const char *sources = ReadSources(kernelFileName);
	program = clCreateProgramWithSource(
        context,
        1,
        &sources,
        NULL,
        &status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to open OpenCL kernel sources: %d\n", status);
		exit(-1);
    }

#ifdef __APPLE__
	status = clBuildProgram(program, 1, devices, "-I. -D__APPLE__", NULL, NULL);
#else
	status = clBuildProgram(program, 1, devices, "-I. ", NULL, NULL);
#endif
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to build OpenCL kernel: %d\n", status);

        size_t retValSize;
		status = clGetProgramBuildInfo(
				program,
				devices[0],
				CL_PROGRAM_BUILD_LOG,
				0,
				NULL,
				&retValSize);
        if (status != CL_SUCCESS) {
            fprintf(stderr, "Failed to get OpenCL kernel info size: %d\n", status);
			exit(-1);
		}

        char *buildLog = (char *)malloc(retValSize + 1);
        status = clGetProgramBuildInfo(
				program,
				devices[0],
				CL_PROGRAM_BUILD_LOG,
				retValSize,
				buildLog,
				NULL);
		if (status != CL_SUCCESS) {
            fprintf(stderr, "Failed to get OpenCL kernel info: %d\n", status);
			exit(-1);
		}
        buildLog[retValSize] = '\0';

		fprintf(stderr, "OpenCL Programm Build Log: %s\n", buildLog);
		exit(-1);
    }

	kernel = clCreateKernel(program, "raytrace_kernel", &status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL kernel: %d\n", status);
		exit(-1);
    }

	// LordCRC's patch for better workGroupSize
	size_t gsize = 0;
	status = clGetKernelWorkGroupInfo(kernel,
			devices[0],
			CL_KERNEL_WORK_GROUP_SIZE,
			sizeof(size_t),
			&gsize,
			NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to get OpenCL kernel work group size info: %d\n", status);
		exit(-1);
	}

	workGroupSize = (unsigned int) gsize;
	fprintf(stderr, "OpenCL Device 0: kernel work group size = %d\n", workGroupSize);

	if (forceWorkSize > 0) {
		fprintf(stderr, "OpenCL Device 0: forced kernel work group size = %d\n", forceWorkSize);
		workGroupSize = forceWorkSize;
	}

	char* realOutput = _strdup(outputLine1.c_str());
	return realOutput;
}

void ExecuteKernel() {
	/* Enqueue a kernel run call */
	size_t globalThreads[1];
	globalThreads[0] = m_Width * m_Height;
	if (globalThreads[0] % workGroupSize != 0)
		globalThreads[0] = (globalThreads[0] / workGroupSize + 1) * workGroupSize;
	size_t localThreads[1];
	localThreads[0] = workGroupSize;

	cl_int status = clEnqueueNDRangeKernel(
			commandQueue,
			kernel,
			1,
			NULL,
			globalThreads,
			localThreads,
			0,
			NULL,
			NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to enqueue OpenCL work: %d\n", status);
		exit(-1);
	}
}

void SetKernelArguments(){

	/* Set kernel arguments */
	cl_int status = clSetKernelArg(
			kernel,
			0,
			sizeof(int),
			(void *)&m_Height);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #0: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
			kernel,
			1,
			sizeof(int),
			(void *)&m_Width);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #1: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
			kernel,
			2,
			sizeof(cl_mem),
			(void *)&m_DestBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #2: %d\n", status);
		exit(-1);
	}
	status = clSetKernelArg(
			kernel,
			3,
			sizeof(cl_mem),
			(void *)&primitiveBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #3: %d\n", status);
		exit(-1);
	}

	int primitiveCount = m_Scene->m_Primitives;
	status = clSetKernelArg(
			kernel,
			4,
			sizeof(int),
			(void *)&primitiveCount);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #4: %d\n", status);
		exit(-1);
	}

	/*status = clSetKernelArg(
			kernel,
			5,
			sizeof(cl_mem),
			(void *)&colorBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #5: %d\n", status);
		exit(-1);
	}*/
}

void ReadKernelBuffer(){
	/* Enqueue readBuffer */
	cl_int status = clEnqueueReadBuffer(
			commandQueue,
			m_DestBuffer,
			CL_TRUE,
			0,
			m_Width * m_Height * sizeof(unsigned int),
			m_Dest,
			0,
			NULL,
			NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to read the OpenCL pixel buffer: %d\n", status);
		exit(-1);
	}

}