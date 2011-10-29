// -----------------------------------------------------------
// scene.cpp
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#define _CRT_SECURE_NO_WARNINGS 1

#include "common.h"
#include "string.h"
#include "raytracer.h"
#include "scene.h"

float m_WX1, m_WY1, m_WX2, m_WY2, m_DX, m_DY, m_SX, m_SY;
Scene* m_Scene;
Pixel* m_Dest = NULL;
TracedRay TracedRays[TRACEDEPTH];
int m_Width, m_Height, m_CurrLine, m_PPos;
//Primitive** m_LastRow;

//vector3 * dummy_vector = NULL;

vector3 * camera = NULL;

// -----------------------------------------------------------
// Primitive class implementation
// -----------------------------------------------------------

//void Primitive_SetName( Primitive * p, char* a_Name )
//{
//	free(p->m_Name);
//	p->m_Name = _strdup(a_Name);
//}

void Primitive_GetNormal(vector3 *newvec, Primitive p, vector3 a_Pos ) {
	if (p.type == SPHERE) {
		newvec->x = a_Pos.x - p.m_Centre.x;
		newvec->y = a_Pos.y - p.m_Centre.y;
		newvec->z = a_Pos.z - p.m_Centre.z;		
		newvec->x *= p.m_RRadius;
		newvec->y *= p.m_RRadius;
		newvec->z *= p.m_RRadius;
	} else {
		if (p.type == PLANE) {
			newvec->x = p.m_Plane.N.x;
			newvec->y = p.m_Plane.N.y;
			newvec->z = p.m_Plane.N.z;
		} else {
			newvec->x = 0;
			newvec->y = 0;
			newvec->z = 0;
		}
	}
}

void Primitive_Create(Primitive *p, PRIMTYPE type, 
	float center_normal_x, float center_normal_y, float center_normal_z,
	float radius_depth,
	float color_r, float color_g, float color_b,
	float refl, float refr, float rIndex, float diff, float spec,
	bool isLight)
{
	p->type = type;
	p->m_Light = isLight ? 1 : 0;
	p->m_Material.m_Color.x = color_r;
	p->m_Material.m_Color.y = color_g;
	p->m_Material.m_Color.z = color_b;
	p->m_Material.m_Refl = refl;
	p->m_Material.m_Refr = refr;
	p->m_Material.m_RIndex = rIndex;
	p->m_Material.m_Diff = diff;
	p->m_Material.m_Spec = spec;
	bool isSphere = (type == SPHERE);
	p->m_Centre.x = isSphere ? center_normal_x : 0;
	p->m_Centre.y = isSphere ? center_normal_y : 0;
	p->m_Centre.z = isSphere ? center_normal_z : 0;
	p->m_Radius = isSphere ? radius_depth : 0;
	p->m_SqRadius = isSphere ? radius_depth*radius_depth : 0;
	p->m_RRadius = isSphere ? ((radius_depth > 0) ? 1.0f/radius_depth : 0) : 0;
	p->m_Plane.D = isSphere ? 0 : radius_depth;
	p->m_Plane.N.x = isSphere ? 0 : center_normal_x;
	p->m_Plane.N.y = isSphere ? 0 : center_normal_y;
	p->m_Plane.N.z = isSphere ? 0 : center_normal_z;
}

vector3 Primitive_GetCentre(Primitive *p) {
	return p->m_Centre;
}

float Primitive_GetSqRadius(Primitive *p) { return p->m_SqRadius; }


float Primitive_GetD(Primitive *p) { return p->m_Plane.D; }


// -----------------------------------------------------------
// Material class implementation
// -----------------------------------------------------------

void Material_Set(Material *m) {
	m->m_Color.x = 0.2f;
	m->m_Color.y = 0.2f;
	m->m_Color.z = 0.2f;
	m->m_Refl = 0.0f;
	m->m_Diff = 0.2f;
	m->m_RIndex = 1.5f;
}

void Material_SetColor(Material * m, float r, float g, float b) {
	m->m_Color.x = r;
	m->m_Color.y = g;
	m->m_Color.z = b;
}

float Material_GetSpecular(Material * m) { return 1.0f - m->m_Diff; }

//Material * Primitive_GetMaterial(Primitive *p) { return p->m_Material; }

PRIMTYPE Primitive_GetType(Primitive *p) { return p->type; }


// -----------------------------------------------------------
// Sphere primitive methods
// -----------------------------------------------------------

int Primitive_Intersect(Primitive *p, Ray * a_Ray, float * a_Dist) {
	vector3 dummy_vector;
	dummy_vector.x = 0;
	dummy_vector.y = 0;
	dummy_vector.z = 0;
	if (p->type == SPHERE) {
		dummy_vector.x = a_Ray->m_Origin.x;
		dummy_vector.y = a_Ray->m_Origin.y;
		dummy_vector.z = a_Ray->m_Origin.z;
		//vector3_minusEquals(dummy_vector, p->m_Centre);
		//vector3 * v = vector3_minus(Ray_GetOrigin(a_Ray), p->m_Centre);		
		dummy_vector.x -= p->m_Centre.x;
		dummy_vector.y -= p->m_Centre.y;
		dummy_vector.z -= p->m_Centre.z;
		float b = vector3_Dot(dummy_vector, a_Ray->m_Direction);
		b = -b;
		float det = (b * b) - vector3_Dot( dummy_vector, dummy_vector ) + p->m_SqRadius;
		//free(v);
		int retval = MISS;
		if (det > 0)
		{
			det = sqrtf( det );
			float i1 = b - det;
			float i2 = b + det;
			if (i2 > 0)
			{
				if (i1 < 0) 
				{
					if (i2 < *a_Dist) 
					{
						*a_Dist = i2;
						retval = INPRIM;
					}
				}
				else
				{
					if (i1 < *a_Dist)
					{
						*a_Dist = i1;
						retval = HIT;
					}
				}
			}
		}
		return retval;
	} else {
		if (p->type == PLANE) {
			float d = vector3_Dot( p->m_Plane.N, a_Ray->m_Direction);
			if (d != 0)
			{
				float dist = -(vector3_Dot( p->m_Plane.N, a_Ray->m_Origin ) + p->m_Plane.D) / d;
				if (dist > 0)
				{
					if (dist < *a_Dist) 
					{
						*a_Dist = dist;
						return HIT;
					}
				}
			}			
			return MISS;
		} else {
			return MISS;
		}
	}
}



// -----------------------------------------------------------
// Scene class implementation
// -----------------------------------------------------------

Scene * Scene_Create(void) {
	Scene *s = (Scene *) malloc(sizeof(Scene));
	return s;
}

void TracedRays_init(void){
	for (int i = 0; i < TRACEDEPTH; ++i) {
		TracedRays[i].color.x = 0;
		TracedRays[i].color.y = 0;
		TracedRays[i].color.z = 0;
		TracedRays[i].refl = 0;
		TracedRays[i].refl_index = 0;
	}
}


int Scene_GetNrPrimitives(Scene *s) { return s->m_Primitives; }


void Scene_InitScene()
{
	m_Scene = Scene_Create();
	// set number of primitives
	int maxx, maxy;
	maxx = maxy = 0;
	//maxx = 30; maxy = 15;
	//maxx = 10; maxy = 2;
	m_Scene->m_Primitive = (Primitive *) malloc (sizeof(Primitive)*50);
	int pc = 0;
	// floor plane
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), PLANE , 0.0f, 0.75f, 0.0f, 4.4f, 0.6f, 0.6f, 0.6f, 0.0f, 0.0f, 0.0f, 0.4f, 1.8f, false);
	// light source center
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, 0.0f, 6.5f, 22.0f, 0.35f, 0.85f, 0.85f, 0.85f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, true);
	// big sphere
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, 3.4f, -3.40f, 23.0f, 2.5f, 0.08f, 0.08f, 0.08f, 1.9f, 1.0f, 2.3f, 0.0f, 0.0f, false);
	//Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, 03.4f, -3.40f, 23.0f, 2.5f, 0.08f, 0.08f, 0.08f, 1.5f, 1.0f, 2.3f, 0.2f, 0.2f, false);
	// small sphere 5
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, -0.7f, -4.90f, 27.0f, 1.0f, 0.07f, 0.17f, 0.07f, 0.1f, 1.5f, 2.3f, 0.2f, 0.8f, false);
	// small sphere
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, -3.4f, -3.40f, 29.0f, 2.5f, 1.0f, 1.0f, 1.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, false);
	// small sphere 2
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, 0.5f, -4.10f, 29.0f, 1.5f, 1.5f, 0.7f, 0.7f, 0.1f, 0.0f, 0.0f, 0.2f, 0.2f, false);
	// small sphere 3
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, -6.0f, -4.10f, 32.0f, 1.5f, 0.7f, 0.7f, 1.7f, 0.2f, 0.0f, 0.0f, 0.2f, 0.2f, false);
	// small sphere 4
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, -6.7f, -4.90f, 29.0f, 1.0f, 0.07f, 0.17f, 0.07f, 0.1f, 1.5f, 2.3f, 0.2f, 0.8f, false);
	// small sphere 6
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, 6.4f, -4.90f, 18.0f, 1.0f, 0.18f, 0.18f, 0.18f, 1.7f, 1.0f, 2.6f, 1.8f, 0.0f, false);
	// left wall
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), PLANE, 0.7f, 0.0f, 0.0f, 5.4f, 1.0f, 0.6f, 0.6f, 0.0f, 0.0f, 0.0f, 0.8f, 1.5f, false);
	// right wall
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), PLANE, -0.7f, 0.0f, 0.0f, 5.4f, 0.7f, 0.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.8f, 0.8f, false);
	// top wall
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), PLANE, 0.0f, -0.8f, 0.0f, 5.4f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.2f, 0.8f, false);
	// back wall
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), PLANE, 0.0f, 0.0f, -0.14f, 5.4f, 2.5f, 2.5f, 2.5f, 0.0f, 0.0f, 0.0f, 1.2f, 0.8f, false);
	// front wall
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), PLANE, 0.0f, 0.0f, 0.72f, 5.4f, 0.1f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false);
	// light source right
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, -3.0f, 6.5f, 22.0f, 0.35f, 0.85f, 0.85f, 0.85f, 0.0f, 0.0f, 0.0f, 0.0f, 1.8f, true);
	// light source left
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, 3.0f, 6.5f, 22.0f, 0.35f, 0.85f, 0.85f, 0.85f, 0.0f, 0.0f, 0.0f, 0.0f, 1.8f, true);
	// light source ground back
	Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, -5.8f, -5.55f, 31.0f, 0.35f, 1.15f, 0.35f, 0.35f, 1.0f, 1.0f, 2.3f, 0.0f, 1.8f, true);

	int x = 0; int y = 0;
	for (y = 0; y < maxy; ++y) {
		for (x = 0; x < maxx; x++) {
			int index = 5 + x + (y*maxx);
			Primitive_Create(&(m_Scene->m_Primitive[pc++]), SPHERE, 
				-15.0f+(x*1.0f), 10.0f-(y*1.0f), 23, 0.5f, 0.8f+0.3f*x, 2.6f-0.02f*x+0.05f*y, 0.5f+0.01f*y, 0.0f, 0.0f, 1.5f, 0.1f, 0.1f, false);	
		}
	}
	m_Scene->m_Primitives = pc + (maxx*maxy);
}


