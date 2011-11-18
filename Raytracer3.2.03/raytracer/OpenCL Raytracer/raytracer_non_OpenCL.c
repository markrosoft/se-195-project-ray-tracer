#include <stdlib.h>
#include <math.h>

#define TRACEDEPTH 5

// cant have dynamic allocation in the kernel
#if TRACEDEPTH == 0
#define MAX_RAY_COUNT 1
#elif TRACEDEPTH == 1
#define MAX_RAY_COUNT 2
#elif TRACEDEPTH == 2
#define MAX_RAY_COUNT 4
#elif TRACEDEPTH == 3
#define MAX_RAY_COUNT 8
#elif TRACEDEPTH == 4
#define MAX_RAY_COUNT 32
#elif TRACEDEPTH == 5
#define MAX_RAY_COUNT 64
#endif

// Intersection method return values
#define HIT		 1		// Ray hit primitive
#define MISS	 0		// Ray missed primitive
#define INPRIM	-1		// Ray started inside primitive

#define EPSILON 0.001f

// ray queue to simulate recursion

#define PUSH_RAY(q, r, c, n) \
	if (c >= MAX_RAY_COUNT) \
		c = 0; \
	q[c++] = r; \
	n++; 

#define POP_RAY(q, r, c, n) \
	if (c >= MAX_RAY_COUNT) \
		c = 0; \
	r = q[c++]; \
	n--;

typedef struct{
	float x,y,z,w;
}float_4;

typedef struct{
	unsigned char x,y,z,w;
}uchar_4;
	
#define dot(F,A,B)(F = A.x*B.x+A.y*B.y+A.z*B.z)

// typedefs - repeated from common.h, but for the kernel
typedef uchar_4 Pixel;
typedef float_4 Color_2;

typedef enum {
	ORIGIN = 0,
	REFLECTED = 1,
	REFRACTED = 2
} ray_type;

typedef enum  {
	PLANE = 0,
	SPHERE = 1
} prim_type;

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

typedef struct {
	float_4 origin;
	float_4 direction;
	float weight;
	float depth;
	int origin_primitive;
	ray_type type;
	float r_index;
	Color_2 transparency;
} Ray;

// functions
int plane_intersect(Primitive_2 * p, Ray * ray, float * cumu_dist )
{
	float d; dot(d, p->normal, ray->direction );
	if ( d != 0 )
	{
		float tempDot; dot(tempDot, p->normal, ray->origin );
		float dist = - ( tempDot + p->depth ) / d;
		if (dist > 0 && dist < *cumu_dist)
		{
			*cumu_dist = dist;
			return HIT;
		}
	}
	return MISS;
}

int sphere_intersect(Primitive_2 * p, Ray * ray, float * cumu_dist )
{
	float_4 v;
	v.x = ray->origin.x - p->center.x;
	v.y = ray->origin.y - p->center.y;
	v.z = ray->origin.z - p->center.z;
	float b; dot(b, v, ray->direction ); b = -b;
	float tempDot; dot(tempDot, v, v );
	float det = (b * b) - tempDot + p->sq_radius;
	int retval = MISS;
	if (det > 0)
	{
		//det = native_sqrt(det);
		det = sqrt(det);
		float i1 = b - det;
		float i2 = b + det;
		if (i2 > 0)
		{
			if (i1 < 0)
			{
				if (i2 < * cumu_dist)
				{
					*cumu_dist = i2;
					retval = INPRIM;
				}
			}
			else
			{
				if (i1 < * cumu_dist)
				{
					*cumu_dist = i1;
					retval = HIT;
				}
			}
		}
	}
	return retval;
}

int intersect(Primitive_2 * p, Ray * ray, float * cumu_dist )
{
	switch (p->type)
	{
		case PLANE:
			return plane_intersect(p, ray, cumu_dist);
		case SPHERE:
			return sphere_intersect(p, ray, cumu_dist);
	}
	return MISS;
}

float_4 get_normal(Primitive_2 * p, float_4 point)
{
	float_4 temp;
	switch (p->type)
	{
		case PLANE:
			return (p->normal);
		case SPHERE:
			temp.x = (point.x - p->center.x) * p->r_radius;
			temp.y = (point.y - p->center.y) * p->r_radius;
			temp.z = (point.z - p->center.z) * p->r_radius;
			return temp;
	}
	temp.x=temp.y=temp.z=temp.w=0.0f;
	return temp;
}

int raytrace(Ray * a_ray, Color_2 * a_acc, float * a_dist, float_4 * point_intersect, int * result, Primitive_2 * primitives, int n_primitives)
{
	*a_dist = 10000000.0f;
	int prim_index = -1;

	// find nearest intersection
	for ( int s = 0; s < n_primitives; s++ )
	{
		int res;
		if (res = intersect(&primitives[s], a_ray, a_dist))
		{			
			prim_index = s;
			* result = res;
		}
	}
	// no hit
	if (prim_index == -1) return -1;
	// handle hit
	if (primitives[prim_index].is_light)
	{
		*a_acc = primitives[prim_index].m_color;
	}
	else
	{
		point_intersect->x = a_ray->origin.x + (a_ray->direction.x * (*a_dist));
		point_intersect->y = a_ray->origin.y + (a_ray->direction.y * (*a_dist));
		point_intersect->z = a_ray->origin.z + (a_ray->direction.z * (*a_dist));
		// trace lights
		for (int l = 0; l < n_primitives; l++)
		{
			if (primitives[l].is_light)
			{
				// point light source shadows
				float shade = 1.0f;
				//float L_LEN = length(primitives[l].center - *point_intersect);
				float_4 temp;
				temp.x = primitives[l].center.x - point_intersect->x;
				temp.y = primitives[l].center.y - point_intersect->y;
				temp.z = primitives[l].center.z - point_intersect->z;
				float L_LEN = sqrt( temp.x * temp.x + temp.y * temp.y + temp.z * temp.z );
				//float_4 L = normalize(primitives[l].center - *point_intersect);
				float_4 L; 
				L.x = (1.0f / L_LEN)*temp.x; 
				L.y = (1.0f / L_LEN)*temp.y; 
				L.z = (1.0f / L_LEN)*temp.z; 
				if (primitives[l].type == SPHERE)
				{
					Ray r;
					r.origin.x = point_intersect->x + L.x * EPSILON;
					r.origin.y = point_intersect->y + L.y * EPSILON;
					r.origin.z = point_intersect->z + L.z * EPSILON;
					r.direction = L;
					int s = 0;
					while ( s < n_primitives )
					{
						if (!primitives[s].is_light && intersect(&primitives[s], &r, &L_LEN))
						{
							shade = 0;
							break;
						}
						s++;
					}
				}
				// Calculate diffuse shading
				float_4 N = get_normal(&primitives[prim_index], *point_intersect);
				if (primitives[prim_index].m_diff > 0)
				{
					float dot_prod; dot(dot_prod, N, L );
					if (dot_prod > 0)
					{
						float diff = dot_prod * primitives[prim_index].m_diff * shade;
						a_acc->x += diff * primitives[prim_index].m_color.x * primitives[l].m_color.x;
						a_acc->y += diff * primitives[prim_index].m_color.y * primitives[l].m_color.y;
						a_acc->z += diff * primitives[prim_index].m_color.z * primitives[l].m_color.z;
					}
				}
				// Calculate specular shading
				if (primitives[prim_index].m_spec > 0)
				{
					float_4 V;
					V.x = a_ray->direction.x;
					V.y = a_ray->direction.y;
					V.z = a_ray->direction.z;
					float tempDot; dot (tempDot, L, N );
					float_4 R;
					R.x = L.x - 2.0f * tempDot * N.x;
					R.y = L.y - 2.0f * tempDot * N.y;
					R.z = L.z - 2.0f * tempDot * N.z;
					float dot_prod; dot (dot_prod, V, R );
					if (dot_prod > 0)
					{
						float spec = pow( dot_prod, 20 ) * primitives[prim_index].m_spec * shade;
						a_acc->x += spec * primitives[l].m_color.x;
						a_acc->y += spec * primitives[l].m_color.y;
						a_acc->z += spec * primitives[l].m_color.z;
					}
				}
			}
		}
	}

	return prim_index;
}

// raytracing kernel

void raytracer_non_kernel ( 
	Pixel *pixels, 
	int width, 
	int height,		
	Primitive_2 * primitives, 		
	int n_primitives) {
		const float WX1 = -3.0f;
		const float WX2 = 3.0f;
		const float WY1 = 2.25f;
		const float WY2 = -2.25f;
		/*const float WX1 = -4.0f;
		const float WX2 = 4.0f;
		const float WY1 = 3.0f;
		const float WY2 = -3.0f;*/
		const float DX = (WX2 - WX1) / width;
		const float DY = (WY2 - WY1) / height;

		for(int y = 0; y<height; y++){
			for(int x = 0; x<width; x++){
				const float SY = WY1 + y * DY;
				const float SX = WX1 + x * DX;
				//float_4 camera = (float_4)( 0, 0.0f, -5.0f, 0 );
				float_4 camera; camera.x=0; camera.y=0.25f; camera.z=-7.0f; camera.w=0;

				Ray queue[MAX_RAY_COUNT];
				int rays_in_queue = 0;
				int front_ray_ptr = 0;
				int back_ray_ptr = 0;

				Color_2 acc; acc.x=acc.y=acc.z=acc.w=0;

				// supersample
				for (int tx = -1; tx < 2; tx++ )
					for (int ty = -1; ty < 2; ty++ )
					{
						// create primary ray
						//float_4 dir = normalize( (float_4)(SX + DX * (tx / 2.0f), SY + DY * (ty / 2.0f), 0, 0) - camera );
						float_4 dir;
						dir.x = SX + DX * (tx / 2.0f) - camera.x;
						dir.y = SY + DY * (ty / 2.0f) - camera.y;
						dir.z = 0 - camera.z;
						dir.w = 0 - camera.w;
						float len = 1.0f / sqrt(dir.x*dir.x+dir.y*dir.y+dir.z*dir.z);
						dir.x *= len; dir.y *= len; dir.z *= len;
						Ray r;
						r.origin = camera;
						r.direction = dir;
						r.weight = 1.0f;
						r.depth = 0;
						r.origin_primitive = -1;
						r.type = ORIGIN;
						r.r_index = 1.0f;
						r.transparency.x = r.transparency.y = r.transparency.z = 1;
						r.transparency.w = 0;

						PUSH_RAY(queue, r, back_ray_ptr, rays_in_queue)

							while (rays_in_queue > 0)
							{
								float dist;
								Ray cur_ray;
								POP_RAY(queue, cur_ray, front_ray_ptr, rays_in_queue)
									Color_2 ray_col; ray_col.x = ray_col.y = ray_col.z = ray_col.w = 0;
								float_4 point_intersect;
								int result;
								int prim_index = raytrace( &cur_ray, &ray_col, &dist, &point_intersect, &result, primitives, n_primitives);
								switch ( cur_ray.type )
								{
								case ORIGIN:
									acc.x += ray_col.x * cur_ray.weight;
									acc.y += ray_col.y * cur_ray.weight;
									acc.z += ray_col.z * cur_ray.weight;
									break;
								case REFLECTED:
									acc.x += ray_col.x * cur_ray.weight * primitives[cur_ray.origin_primitive].m_color.x * cur_ray.transparency.x;
									acc.y += ray_col.y * cur_ray.weight * primitives[cur_ray.origin_primitive].m_color.y * cur_ray.transparency.y;
									acc.z += ray_col.z * cur_ray.weight * primitives[cur_ray.origin_primitive].m_color.z * cur_ray.transparency.z;
									break;
								case REFRACTED:
									acc.x += ray_col.x * cur_ray.weight * cur_ray.transparency.x;
									acc.y += ray_col.y * cur_ray.weight * cur_ray.transparency.y;
									acc.z += ray_col.z * cur_ray.weight * cur_ray.transparency.z;
									break;
								}
								// handle reflection & refraction
								if (cur_ray.depth < TRACEDEPTH)
								{
									// reflection
									float refl = primitives[prim_index].m_refl;
									if (refl > 0.0f)
									{
										float_4 N = get_normal(&primitives[prim_index], point_intersect);
										float_4 R;
										float temp_dot; dot( temp_dot, cur_ray.direction, N );
										R.x = cur_ray.direction.x - 2.0f * temp_dot * N.x;
										R.y = cur_ray.direction.y - 2.0f * temp_dot * N.y;
										R.z = cur_ray.direction.z - 2.0f * temp_dot * N.z;
										Ray new_ray;
										new_ray.origin.x = point_intersect.x + R.x * EPSILON;
										new_ray.origin.y = point_intersect.y + R.y * EPSILON;
										new_ray.origin.z = point_intersect.z + R.z * EPSILON;
										new_ray.direction = R;
										new_ray.depth = cur_ray.depth + 1;
										new_ray.weight = refl * cur_ray.weight;
										new_ray.type = REFLECTED;
										new_ray.origin_primitive = prim_index;
										new_ray.r_index = cur_ray.r_index;
										new_ray.transparency = cur_ray.transparency;
										PUSH_RAY(queue, new_ray, back_ray_ptr, rays_in_queue)
									}
									// refraction
									float refr = primitives[prim_index].m_refr;
									if (refr > 0.0f)
									{
										float m_rindex = primitives[prim_index].m_refr_index;
										float n = cur_ray.r_index / m_rindex;
										float_4 temp = get_normal(&primitives[prim_index], point_intersect);
										float_4 N;
										N.x = temp.x * (float) result;
										N.y = temp.y * (float) result;
										N.z = temp.z * (float) result;
										float temp_dot;  dot( temp_dot, N, cur_ray.direction );
										float cosI = - temp_dot;
										float cosT2 = 1.0f - n * n * (1.0f - cosI * cosI);
										if (cosT2 > 0.0f)
										{
											float_4 T;
											T.x = (n * cur_ray.direction.x) + (n * cosI - sqrt( cosT2 )) * N.x;
											T.y = (n * cur_ray.direction.y) + (n * cosI - sqrt( cosT2 )) * N.y;
											T.z = (n * cur_ray.direction.z) + (n * cosI - sqrt( cosT2 )) * N.z;
											//float_4 T = (n * cur_ray.direction) + (n * cosI - sqrt( cosT2 )) * N;
											Ray new_ray;
											new_ray.origin.x = point_intersect.x + T.x * EPSILON;
											new_ray.origin.y = point_intersect.y + T.y * EPSILON;
											new_ray.origin.z = point_intersect.z + T.z * EPSILON;
											new_ray.direction = T;
											new_ray.depth = cur_ray.depth + 1;
											new_ray.weight = cur_ray.weight;
											new_ray.type = REFRACTED;
											new_ray.origin_primitive = prim_index;
											new_ray.r_index = m_rindex;
											new_ray.transparency.x = cur_ray.transparency.x * (exp(primitives[prim_index].m_color.x * 0.15f * (-dist)));
											new_ray.transparency.y = cur_ray.transparency.y * (exp(primitives[prim_index].m_color.y * 0.15f * (-dist)));
											new_ray.transparency.z = cur_ray.transparency.z * (exp(primitives[prim_index].m_color.z * 0.15f * (-dist)));
											PUSH_RAY(queue, new_ray, back_ray_ptr, rays_in_queue)
										}
									}
								}
							}
					}

					int red = (int)(acc.x * (256 / 9));
					int green = (int)(acc.y * (256 / 9));
					int blue = (int)(acc.z * (256 / 9));
					if (red > 255) red = 255;
					if (green > 255) green = 255;
					if (blue > 255) blue = 255;
					Pixel npix; 
					npix.x = (unsigned char)red; 
					npix.y = (unsigned char)green; 
					npix.z = (unsigned char)blue; 
					npix.w = 0;
					pixels[y * width + x] = npix;
			}
		}
}