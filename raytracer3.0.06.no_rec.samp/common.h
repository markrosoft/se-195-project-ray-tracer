// -----------------------------------------------------------
// common.h
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#ifndef I_COMMON_H
#define I_COMMON_H

#include "math.h"
#include "stdlib.h"

typedef unsigned int Pixel;

inline float Rand( float a_Range ) { return ((float)rand() / RAND_MAX) * a_Range; }


#define DOT(A,B)		(A.x*B.x+A.y*B.y+A.z*B.z)
//#define DOT(A,B)		(A->x*B->x+A->y*B->y+A->z*B->z)
#define NORMALIZE(A)	{float l=1/sqrtf(A.x*A.x+A.y*A.y+A.z*A.z);A.x*=l;A.y*=l;A.z*=l;}
#define LENGTH(A)		(sqrtf(A.x*A.x+A.y*A.y+A.z*A.z))
#define SQRLENGTH(A)	(A.x*A.x+A.y*A.y+A.z*A.z)
#define SQRDISTANCE(A,B) ((A.x-B.x)*(A.x-B.x)+(A.y-B.y)*(A.y-B.y)+(A.z-B.z)*(A.z-B.z))

#define EPSILON			0.001f
#define TRACEDEPTH		4

#define PI				3.141592653589793238462f

struct vector3{
public:
	float x;
	float y;
	float z;
};

inline void vector3_Set(vector3 * v, float x, float y, float z) { v->x = x; v->y = y; v->z = z; }
#define vector3_Copy(v1, v2) { (v1).x = (v2).x; (v1).y = (v2).y; (v1).z = (v2).z; }
#define vector3_Dot(v1, v2) (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)
#define vector3_plusEquals(v1, v2) { v1.x += v2.x; v1.y += v2.y; v1.z += v2.z; }
#define vector3_minusEquals(v1, v2) { v1.x -= v2.x; v1.y -= v2.y; v1.z -= v2.z; }
#define vector3_scalarMult(v, s) { v.x *=  s; v.y *=  s; v.z *= s;}
#define vector3_vectorMult(v1, v2) { v1.x *= v2.x; v1.y *= v2.y; v1.z *= v2.z; }

inline void vector3_negate(vector3 * v) {
	 v->x = -v->x; v->y = -v->y; v->z = -v->z;
}
#define vector3_minus(v1, v2, v3){v1.x = v2.x-v3.x; v1.y = v2.y-v3.y; v1.z = v2.z-v3.z;}

struct plane{
	vector3 N;
	float D;
	float cell[4];
};

inline plane * plane_Create(float normal_x, float normal_y, float normal_z, float a_D) {
	plane * p = (plane *)malloc(sizeof(plane));
	p->N.x = normal_x;
	p->N.y = normal_y;
	p->N.z = normal_z;
	p->D = a_D;
	return p;
}

typedef vector3 Color;

#endif