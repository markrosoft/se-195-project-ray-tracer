#ifndef __COMMON_H__
#define __COMMON_H__


#define CHOOSE_GPU 0
#define CHOOSE_SCENE 0
// Need to change lines 265-280 of ratracer_kernel.cl

#include <CL/cl.h>

typedef cl_uchar4 Pixel;
typedef cl_float4 Color;

typedef enum {
	PLANE = 0,
	SPHERE = 1
} prim_type;

typedef struct {
	Color color; // material color
	cl_float refl; // material reflection factor
	cl_float diff; // mmaterial diffuse factor
	cl_float refr;
	cl_float refr_index;
	cl_float spec;
	cl_float dummy_3;
} Material;

typedef struct {
	Material material;
	prim_type type;
	cl_bool is_light;
	cl_float4 normal;
	cl_float4 center;
	cl_float depth;
	cl_float radius, sq_radius, r_radius;
} Primitive;

#endif