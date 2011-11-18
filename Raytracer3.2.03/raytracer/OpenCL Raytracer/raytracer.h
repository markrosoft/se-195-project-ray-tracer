#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

#include "common.h"

#define WORK_GROUP_SIZE 64

/** Variable Declarations **/
// kernel input
Primitive * primitive_list;
cl_uint n_primitives;
// kernel output
Pixel * out_pixels;
uchar_4 * out_pixels_2;
Primitive_2 * primitive_list_2;

// image size
static cl_uint width = 800;
static cl_uint height = 600;

// memory buffers for opencl kernel
cl_mem primitive_buffer; // stores primitive list
cl_mem pixel_buffer; // stores traced pixels

cl_context			context;
cl_device_id		*devices;
cl_command_queue	command_queue;
cl_program			program;
cl_kernel			raytracer_kernel;

/** Function Declarations **/
// Host-related
int initialize_host(void);
void cleanup_host(void);

// OpenCL-related
int initialize_openCL(void);
int run_openCL_kernel(void);
int cleanup_openCL(void);
static char * read_kernel(const char * filename);

#endif