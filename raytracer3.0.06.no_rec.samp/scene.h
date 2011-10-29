#ifndef I_SCENE_H
#define I_SCENE_H

#include <math.h>

// Intersection method return values
#define HIT		 1		// Ray hit primitive
#define MISS	 0		// Ray missed primitive
#define INPRIM	-1		// Ray started inside primitive

void Material_Set(Material m);

void Material_SetColor(Material * m, float r, float g, float b);

float Material_GetSpecular(Material * m);

Material * Primitive_GetMaterial(Primitive *p);

PRIMTYPE Primitive_GetType(Primitive *p);

struct Ray;
// be sure to accomodate all types
int Primitive_Intersect(Primitive * p, Ray * ray, float * dist);

//// always returns a NEW vector3
//vector3 * Primitive_GetNormal(Primitive * p, vector3 * a_Pos );
void Primitive_GetNormal(vector3 *newvec, Primitive p, vector3 a_Pos );

Color * Primitive_GetColor( Primitive * p, vector3 * );

float Primitive_GetSqRadius(Primitive *p);

float Primitive_GetD(Primitive *p);

Scene * Scene_Create(void);

void Scene_InitScene();

int Scene_GetNrPrimitives(Scene *s);

Primitive * Scene_GetPrimitive(Scene *s, int idx);

void TracedRays_init();

#endif