//#include <math.h>
//#include <stdlib.h>

// Intersection method return values
#define HIT				1		// Ray hit primitive
#define MISS			0		// Ray missed primitive
#define INPRIM			-1		// Ray started inside primitive
#define TRACEDEPTH		6
#define EPSILON			0.001f

#define PI				3.141592653589793238462f

enum PRIMTYPE {
	SPHERE = 1,
	PLANE = 2,
	BOX = 3
};

typedef struct{
	float x,y,z;
}vector3;

typedef struct{
	vector3 N;
	float D;
	float cell[4];
}plane;

typedef struct{
	vector3 m_Color;
	float m_Refl, m_Refr, m_Diff, m_Spec, m_RIndex;
}Material;

typedef struct{
	enum PRIMTYPE type;
	int m_Light;
	// for sphere
	vector3 m_Centre;
	float m_SqRadius, m_Radius, m_RRadius;
	// for PlanePrim
	plane m_Plane;
	Material m_Material;
}Primitive;

typedef struct {
	vector3 m_Origin;
	vector3 m_Direction;
}Ray;

#define vector3_Dot(v1, v2) ((v1).x * (v2).x + (v1).y * (v2).y + (v1).z * (v2).z)
#define vector3_minusEquals(v1, v2) { v1.x -= v2.x; v1.y -= v2.y; v1.z -= v2.z; }

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

// -----------------------------------------------------------
// Sphere primitive methods
// -----------------------------------------------------------

//int Primitive_Intersect(/*global*/ Primitive * p, Ray * a_Ray, float * a_Dist) {
int Primitive_Intersect(global Primitive * p, Ray * a_Ray, float * a_Dist) {
	vector3 dummy_vector; 
	dummy_vector.x = 0; 
	dummy_vector.y = 0; 
	dummy_vector.z = 0;
	if (p->type == SPHERE) { 
		dummy_vector.x = a_Ray->m_Origin.x; 
		dummy_vector.y = a_Ray->m_Origin.y; 
		dummy_vector.z = a_Ray->m_Origin.z;
		dummy_vector.x -= p->m_Centre.x; 
		dummy_vector.y -= p->m_Centre.y; 
		dummy_vector.z -= p->m_Centre.z;
		//vector3 * v = vector3_minus(Ray_GetOrigin(a_Ray), p->m_Centre);
		float b = -vector3_Dot(dummy_vector, a_Ray->m_Direction);
		float det = (b * b) - vector3_Dot(dummy_vector, dummy_vector) + p->m_SqRadius;
		//free(v);
		int retval = MISS;
		if (det > 0)
		{
			det = sqrt( det );
			//det = 0.01f;
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
			//vector3_set(dummy_vector, (p->m_Plane)->x, (p->m_Plane)->y, (p->m_Plane)->z);
			float d = vector3_Dot( p->m_Plane.N , a_Ray->m_Direction);
			if (d != 0)
			{
				float dist = -(vector3_Dot(p->m_Plane.N, a_Ray->m_Origin ) + p->m_Plane.D) / d; 
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

//// -----------------------------------------------------------
// Engine::Raytrace
// Naive ray tracing: Intersects the ray with every primitive
// in the scene to determine the closest intersection
// -----------------------------------------------------------
//int Engine_Raytrace(Ray *a_Ray , vector3 *a_Acc, int a_Depth, float * a_RIndex, float *a_Dist, int primitiveCount,/*global*/ Primitive * primitiveBuffer, float * a_refl, int * a_refl_Index, Ray *a_refl_Ray, float * a_refr, int * a_refr_Index, Ray *a_refr_Ray){
int Engine_Raytrace(Ray *a_Ray , vector3 *a_Acc, int a_Depth, float * a_RIndex, float *a_Dist, int primitiveCount,global Primitive * primitiveBuffer, float * a_refl, int * a_refl_Index, Ray *a_refl_Ray, float * a_refr, int * a_refr_Index, Ray *a_refr_Ray){
	if (a_Depth > TRACEDEPTH) return -1;
	// trace primary ray
	*a_Dist = 1000000.0f;
	//vector3 pi;
	//Primitive * prim;// = 0;
	int prim_index = 0;
	int result;
	int hitOnce = 0;
	// find the nearest intersection
	for ( int s = 0; s < primitiveCount; s++ )
	{
		int res;
		if (res = Primitive_Intersect( &primitiveBuffer[s], a_Ray, a_Dist )) 	
		{
			hitOnce = 1;
			prim_index = s;
			//Primitive * x = prim;
			result = res; // 0 = miss or hit something further than last hit, 1 = hit closer than last hit, -1 = hit from inside primitive
		}		
	}
	// no hit, terminate ray
	if (hitOnce == 0) {return -1;}
	// handle intersection
	if (primitiveBuffer[prim_index].m_Light >0)
	{
		// we hit a light, stop tracing
		//a_Acc->x += 1; a_Acc->y += 1; a_Acc->z += 1;
		a_Acc->x += primitiveBuffer[prim_index].m_Material.m_Color.x;
		a_Acc->y += primitiveBuffer[prim_index].m_Material.m_Color.y;
		a_Acc->z += primitiveBuffer[prim_index].m_Material.m_Color.z;
	}
	else
	{
		// determine color at point of intersection
		Ray r = *a_Ray;
		vector3 pi; //Ray_GetDirection(dir3, *a_Ray)
		pi.x = r.m_Direction.x;
		pi.y = r.m_Direction.y;
		pi.z = r.m_Direction.z;
		//vector3_scalarMult(pi, *a_Dist);
		pi.x *= *a_Dist;
		pi.y *= *a_Dist;
		pi.z *= *a_Dist;
		//vector3_plusEquals(pi, r.m_Origin);
		pi.x += r.m_Origin.x;
		pi.y += r.m_Origin.y;
		pi.z += r.m_Origin.z;	

		// trace lights
		for ( int p_index = 0; p_index < primitiveCount; p_index++ ) // iterate thru primitives
		{			
			Primitive p = primitiveBuffer[p_index]; // get the particular primitive
			if (primitiveBuffer[p_index].m_Light>0) // only look for lights
			{
				Primitive light = p;// create local variable
				// added to for Version 2 ////////////////////////////////////////////
				float shade = 1.0f;
				if (light.type == SPHERE)
				{
					//vector3 L = ((Sphere*)light)->GetCentre() - pi;
					vector3 L;
					L.x = light.m_Centre.x - pi.x;
					L.y = light.m_Centre.y - pi.y;
					L.z = light.m_Centre.z - pi.z;
					//float tdist = LENGTH( L );
					float tdist = sqrt(L.x * L.x + L.y * L.y + L.z * L.z);
					//L *= (1.0f / tdist);					
					L.x *= (1.0f / tdist);
					L.y *= (1.0f / tdist);
					L.z *= (1.0f / tdist);
					//Ray r = Ray( pi + L * EPSILON, L );
					Ray r;
					r.m_Origin.x = pi.x + L.x * EPSILON;
					r.m_Origin.y = pi.y + L.y * EPSILON;
					r.m_Origin.z = pi.z + L.z * EPSILON;
					r.m_Direction.x = L.x;
					r.m_Direction.y = L.y;
					r.m_Direction.z = L.z;
					for ( int s = 0; s < primitiveCount; s++ )
					{
						//Primitive pr = primitiveBuffer[s];
						//if ((!pr.m_Light) && (pr->Intersect( r, tdist )))
						if ((primitiveBuffer[s].m_Light<1) && (Primitive_Intersect(&primitiveBuffer[s], &r, &tdist)))
						{
							shade = 0;
							break;
						}
					}
				}
				// end of version 2 add /////////////////////////////////////////////////
				if(shade > 0){
					// calculate diffuse shading
					vector3 L = light.m_Centre;
					vector3_minusEquals(L, pi);
					//vector3 L = ((Sphere*)light)->GetCentre() - pi; // get the vector from center to pi
					float L_len = sqrt( L.x * L.x + L.y * L.y + L.z * L.z );
					//vector3_Normalize(L, L_len); // normalize the vector
					if(L_len > 0.0f){
						L.x *= (1.0f / L_len); L.y *= (1.0f / L_len); L.z *= (1.0f / L_len);
					}else{
						L.x = L.y = L.z = 0;		
					}
					vector3 N; Primitive_GetNormal(&N, primitiveBuffer[prim_index], pi ); // get the normal vector @ primitive intersect
					if (primitiveBuffer[prim_index].m_Material.m_Diff > 0)// check if there is any diffusion
					{
						float dot = N.x * L.x + N.y * L.y + N.z * L.z; 
						if (dot > 0)
						{
							float diff = dot * primitiveBuffer[prim_index].m_Material.m_Diff * shade;
							// add diffuse component to ray color
							//a_Acc += diff * prim->GetMaterial()->GetColor() * light->GetMaterial()->GetColor();
							vector3 dummy_vector;
							dummy_vector.x = primitiveBuffer[prim_index].m_Material.m_Color.x;
							dummy_vector.y = primitiveBuffer[prim_index].m_Material.m_Color.y;
							dummy_vector.z = primitiveBuffer[prim_index].m_Material.m_Color.z;					
							dummy_vector.x *= light.m_Material.m_Color.x;
							dummy_vector.y *= light.m_Material.m_Color.y;
							dummy_vector.z *= light.m_Material.m_Color.z;
							dummy_vector.x *= diff;
							dummy_vector.y *= diff;
							dummy_vector.z *= diff;
							//vector3_vectorMult(dummy_vector4, primitiveBuffer[l_index].m_Material->m_Color);
							//vector3_scalarMult(dummy_vector4, diff);
							a_Acc->x += dummy_vector.x; 
							a_Acc->y += dummy_vector.y; 
							a_Acc->z += dummy_vector.z;
							//vector3_plusEquals(*a_Acc, dummy_vector4);						
						}
					}
					//  added to for Version 2 /////////////////////////
					// determine specular component
					//if (prim->GetMaterial()->GetSpecular() > 0)
					if (primitiveBuffer[prim_index].m_Material.m_Spec > 0.0f)
					{
						// point light source: sample once for specular highlight
						vector3 V = a_Ray->m_Direction;
						//vector3 R = L - 2.0f * DOT( L, N ) * N;
						vector3 R;
						float tempDot = (L.x * N.x + L.y * N.y + L.z * N.z);
						R.x = L.x - 2.0f * tempDot * N.x;
						R.y = L.y - 2.0f * tempDot * N.y;
						R.z = L.z - 2.0f * tempDot * N.z;
						//float dot = DOT( V, R );
						float dot = (V.x * R.x + V.y * R.y + V.z * R.z);
						if (dot > 0)
						{
							//float spec = powf( dot, 20 ) * prim->GetMaterial()->GetSpecular() * shade;
							float primSpec = primitiveBuffer[prim_index].m_Material.m_Spec;
							float spec = pow( dot, 20 ) * primSpec * shade;
							// add specular component to ray color
							//a_Acc += spec * light->GetMaterial()->GetColor();
							a_Acc->x += spec * light.m_Material.m_Color.x;
							a_Acc->y += spec * light.m_Material.m_Color.y;
							a_Acc->z += spec * light.m_Material.m_Color.z;
						}
					}
					// end of version 2 add /////////////////////////////////////////////
				}
			}
		}


		//  added to for Version 3 /////////////////////////
		// calculate refraction
		//float refr = prim->GetMaterial()->GetRefraction();
		*a_refr = primitiveBuffer[prim_index].m_Material.m_Refr;
		if ((*a_refr > 0) && (a_Depth < TRACEDEPTH))
		{
			//float rindex = prim->GetMaterial()->GetRefrIndex();
			float rindex = primitiveBuffer[prim_index].m_Material.m_RIndex;
			float n = * a_RIndex / rindex;
			* a_RIndex = rindex;  
			//vector3 N = prim->GetNormal( pi ) * (float)result;
			vector3 N; Primitive_GetNormal(&N, primitiveBuffer[prim_index], pi ); 
			//vector3_scalarMult( N, (float)result );
			N.x *= (float)result;
			N.y *= (float)result;
			N.z *= (float)result;
			//float cosI = -DOT( N, a_Ray.GetDirection() );
			float cosI = vector3_Dot( N, a_Ray->m_Direction );
			cosI = -cosI;
			//vector3 dirForCosI = a_Ray->m_Direction;
			//float cosI = -(dirForCosI.x * N.x + dirForCosI.y * N.y + dirForCosI.z * N.z);
			float cosT2 = 1.0f - n * n * (1.0f - cosI * cosI);
			if (cosT2 > 0.0f)
			{
				//vector3 T = (n * a_Ray.GetDirection()) + (n * cosI - sqrtf( cosT2 )) * N;
				vector3 dirForT = a_Ray->m_Direction;
				vector3 T;
				float fForT = (n * cosI - sqrt( cosT2 ));
				T.x = (n * dirForT.x) + fForT * N.x;
				T.y = (n * dirForT.y) + fForT * N.y;
				T.z = (n * dirForT.z) + fForT * N.z;
				//Color rcol( 0, 0, 0 );
				a_refr_Ray->m_Origin.x = pi.x + T.x * EPSILON;
				a_refr_Ray->m_Origin.y = pi.y + T.y * EPSILON;
				a_refr_Ray->m_Origin.z = pi.z + T.z * EPSILON;
				a_refr_Ray->m_Direction.x = T.x;
				a_refr_Ray->m_Direction.y = T.y;
				a_refr_Ray->m_Direction.z = T.z;
				*a_refr_Index = prim_index;
			}else{
				*a_refr_Index = -1;
			}
		}else{
			*a_refr_Index = -1;
		}
		//  end of Version 3 ADD /////////////////////////

		//  added to for Version 2 /////////////////////////
		// calculate reflection
		*a_refl = primitiveBuffer[prim_index].m_Material.m_Refl;
		if (*a_refl > 0.0f)
		{
			//vector3 N = prim->GetNormal( pi );
			vector3 N; Primitive_GetNormal(&N, primitiveBuffer[prim_index], pi );
			//vector3 R = a_Ray.GetDirection() - 2.0f * DOT( a_Ray.GetDirection(), N ) * N;
			vector3 R; // Check the next 4 lines for programming accuracy
			vector3 dirForR = a_Ray->m_Direction;
			float dotForR = (dirForR.x * N.x + dirForR.y * N.y + dirForR.z * N.z);			
			R.x = dirForR.x - 2.0f * dotForR * N.x;
			R.y = dirForR.y - 2.0f * dotForR * N.y;
			R.z = dirForR.z - 2.0f * dotForR * N.z;
			//Color rcol( 0, 0, 0 );
			//Raytrace( Ray( pi + R * EPSILON, R ), rcol, a_Depth + 1, a_RIndex, dist );
			a_refl_Ray->m_Origin.x = pi.x + R.x * EPSILON;
			a_refl_Ray->m_Origin.y = pi.y + R.y * EPSILON;
			a_refl_Ray->m_Origin.z = pi.z + R.z * EPSILON;
			a_refl_Ray->m_Direction.x = R.x;
			a_refl_Ray->m_Direction.y = R.y;
			a_refl_Ray->m_Direction.z = R.z;
			*a_refl_Index = prim_index;
		}else{
			*a_refl_Index = -1;
		}
		// end of version 2 add /////////////////////////////////////////////
	}
	return prim_index;
}