#ifndef __COMMON_H__
#define __COMMON_H__


#define CHOOSE_GPU 1
#define CHOOSE_SCENE 0
// Need to change lines 265-280 of ratracer_kernel.cl

#include <CL/cl.h>

typedef struct{
	float x,y,z,w;
}float_4;

typedef struct{
	unsigned char x,y,z,w;
}uchar_4;

typedef cl_uchar4 Pixel;
typedef cl_float4 Color;
typedef float_4 Color_2;


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

typedef struct {
	Color_2 m_color;
	float m_refl;
	float m_diff;
	float m_refr;
	float m_refr_index;
	float m_spec;
	float dummy_3;
	prim_type type;
	bool is_light;
	float_4 normal;
	float_4 center;
	float depth;
	float radius, sq_radius, r_radius;
} Primitive_2;

#endif