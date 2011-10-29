// -----------------------------------------------------------
// raytracer.h
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#ifndef I_RAYTRACER_H
#define I_RAYTRACER_H

#include <math.h>
#include "common.h"

enum PRIMTYPE{
	SPHERE = 1,
	PLANE = 2,
	BOX = 3
};

typedef struct {
	vector3 m_Color;
	float m_Refl, m_Refr, m_Diff, m_Spec, m_RIndex;
}Material;

typedef struct {
	enum PRIMTYPE type;
	int m_Light;
	// for Sphere
	vector3 m_Centre;
	float m_SqRadius, m_Radius, m_RRadius;
	// for PlanePrim
	plane m_Plane;
	Material m_Material;	
}Primitive;

struct Scene{
	int m_Primitives;
	Primitive * m_Primitive;
};

struct Ray{
	vector3 m_Origin;
	vector3 m_Direction;
};

typedef struct{
	vector3 color;
	float refl;
	int refl_index;
}TracedRay;

extern float m_WX1, m_WY1, m_WX2, m_WY2, m_DX, m_DY, m_SX, m_SY;
extern Scene* m_Scene;
extern Pixel* m_Dest;
extern TracedRay TracedRays[];
extern int m_Width, m_Height, m_CurrLine, m_PPos;
//extern Primitive** m_LastRow;
void Engine_Constructor();

void Engine_SetTarget( Pixel* a_Dest, int a_Width, int a_Height );
inline Scene* Engine_GetScene() { return m_Scene; }
//Primitive* Engine_Raytrace( Ray& a_Ray, Color& a_Acc, int a_Depth, float a_RIndex, float& a_Dist );
//void Engine_Raytrace( Ray * a_Ray, Color * a_Acc, int a_Depth, float a_RIndex, float * a_Dist );
int Engine_Raytrace( Ray * a_Ray, Color * a_Acc, int a_Depth, float a_RIndex, float * a_Dist, float * refl, int * a_PrimIndex );
void Engine_InitRender();
bool Engine_Render();
bool Engine_Render_Simple();


#endif