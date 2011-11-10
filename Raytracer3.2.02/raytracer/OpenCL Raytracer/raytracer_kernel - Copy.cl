#ifdef DEBUGGING
#pragma OPENCL EXTENSION cl_amd_printf : enable
#define DEBUG(st, i) \
			if (get_global_id(0) == 288133) \
				printf("%s : f - %f ~ d - %d\r\n", st, i, i);
#endif


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

#define EPSILON 0.01f

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
	
// typedefs - repeated from common.h, but for the kernel
typedef uchar4 Pixel;
typedef float4 Color;

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
	Color m_color;
	float m_refl;
	float m_diff;
	prim_type type;
	bool is_light;
	float4 normal;
	float4 center;
	float depth;
	float radius, sq_radius, r_radius;
} Primitive;

#define SPECULAR(p) (1.0f - p.m_diff)

typedef struct {
	float4 origin;
	float4 direction;
	float weight;
	float depth;
	int origin_primitive;
	ray_type type;
} Ray;

// functions
int plane_intersect(global Primitive * p, Ray * ray, float * cumu_dist )
{
	float d = dot( p->normal, ray->direction );
	if ( d != 0 )
	{
		float dist = - ( dot( p->normal, ray->origin ) + p->depth ) / d;
		if (dist > 0 && dist < *cumu_dist)
		{
			*cumu_dist = dist;
			return HIT;
		}
	}
	return MISS;
}

int sphere_intersect(global Primitive * p, Ray * ray, float * cumu_dist )
{
	float4 v = ray->origin - p->center;
	float b = - dot( v, ray->direction );
	float det = (b * b) - dot(v, v) + p->sq_radius;
	int retval = MISS;
	if (det > 0)
	{
		det = native_sqrt(det);
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

int intersect(global Primitive * p, Ray * ray, float * cumu_dist )
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

float4 get_normal(global Primitive * p, float4 point)
{
	switch (p->type)
	{
		case PLANE:
			return (p->normal);
		case SPHERE:
			return (point - p->center) * p->r_radius;
	}
	return (float4) (0, 0, 0, 0);
}

int raytrace(Ray * a_ray, Color * a_acc, float a_rindex, float * a_dist, float4 * point_intersect, global Primitive * primitives, int n_primitives)
{
	*a_dist = MAXFLOAT;
	int result;
	int prim_index = -1;

	// find nearest intersection
	for ( int s = 0; s < n_primitives; s++ )
	{
		int res;
		if (res = intersect(&primitives[s], a_ray, a_dist))
		{			
			prim_index = s;
			result = res;
		}
	}
	// no hit
	if (prim_index == -1) return -1;
	// handle hit
	if (primitives[prim_index].is_light)
	{
		*a_acc = (Color) (1, 1, 1, 0);
	}
	else
	{
		*point_intersect = a_ray->origin + (a_ray->direction * (*a_dist));
		// trace lights
		for (int l = 0; l < n_primitives; l++)
		{
			if (primitives[l].is_light)
			{
				// point light source shadows
				float shade = 1.0f;
				float L_LEN = length(primitives[l].center - *point_intersect);
				float4 L = fast_normalize(primitives[l].center - *point_intersect);
				if (primitives[l].type == SPHERE)
				{
					Ray r;
					r.origin = *point_intersect + L * EPSILON;
					r.direction = L;
					for (int s = 0; s < n_primitives; s++ )
					{
						if (&primitives[s] != &primitives[l] && intersect(&primitives[s], &r, &L_LEN))
						{
							shade = 0.0f;
							break;
						}
					}
				}
				// Calculate diffuse shading
				float4 N = get_normal(&primitives[prim_index], *point_intersect);
				if (primitives[prim_index].m_diff > 0)
				{
					float dot_prod = dot( N, L );
					if (dot_prod > 0)
					{
						float diff = dot_prod * primitives[prim_index].m_diff * shade;
						* a_acc += diff * primitives[prim_index].m_color * primitives[l].m_color;
					}
				}
				// Calculate specular shading
				if (SPECULAR(primitives[prim_index]) > 0)
				{
					float4 V = a_ray->direction;
					float4 R = L - 2.0f * dot ( L, N ) * N;
					float dot_prod = dot ( V, R );
					if (dot_prod > 0)
					{
						float spec = native_powr( dot_prod, 20 ) * SPECULAR(primitives[prim_index]) * shade;
						* a_acc += spec * primitives[l].m_color;
					}
				}
			}
		}
	}

	return prim_index;
}

// raytracing kernel

__kernel void raytracer_kernel ( 
		global Pixel *pixels, 
		const int width, 
		const int height,		
		global Primitive *primitives, 		
		const int n_primitives) {

		const int gid = get_global_id(0);
		const int x = gid % width;
		const int y = gid / width;	

		if (x >= width || y >= height)
			return;

		const float WX1 = -4.0f;
		const float WX2 = 4.0f;
		const float WY1 = 3.0f;
		const float WY2 = -3.0f;
		const float DX = (WX2 - WX1) / width;
		const float DY = (WY2 - WY1) / height;
		const float SX = WX1 + x * DX;
		const float SY = WY1 + y * DY;

		float4 camera = (float4)( 0, 0, -5, 0 );

		Ray queue[MAX_RAY_COUNT];
		int rays_in_queue = 0;
		int front_ray_ptr = 0;
		int back_ray_ptr = 0;

		// create primary ray
		float4 dir = fast_normalize( (float4)(SX, SY, 0, 0) - camera );
		Ray r;
		r.origin = camera;
		r.direction = dir;
		r.weight = 1.0f;
		r.depth = 0;
		r.origin_primitive = -1;
		r.type = ORIGIN;
		Color acc = (Color)( 0, 0, 0, 0 );

		PUSH_RAY(queue, r, back_ray_ptr, rays_in_queue)

		while (rays_in_queue > 0)
		{
			float dist;
			Ray cur_ray;
			POP_RAY(queue, cur_ray, front_ray_ptr, rays_in_queue)
			Color ray_col = (Color)( 0, 0, 0, 0 );
			float4 point_intersect;
			int prim_index = raytrace( &cur_ray, &ray_col, 1.0f, &dist, &point_intersect, primitives, n_primitives);
			switch ( cur_ray.type )
			{
				case ORIGIN:
					acc += ray_col * cur_ray.weight;
					break;
				case REFLECTED:
					acc += ray_col * cur_ray.weight * primitives[cur_ray.origin_primitive].m_color;
					break;
			}
			// handle reflection & refraction
			if (cur_ray.depth < TRACEDEPTH)
			{
				// reflection
				float refl = primitives[prim_index].m_refl;
				if (refl > 0.0f)
				{
					float4 N = get_normal(&primitives[prim_index], point_intersect);
					float4 R = cur_ray.direction - 2.0f * dot( cur_ray.direction, N ) * N;
					Ray new_ray;
					new_ray.origin = point_intersect + R * EPSILON;
					new_ray.direction = R;
					new_ray.depth = cur_ray.depth + 1;
					new_ray.weight = refl * cur_ray.weight;
					new_ray.type = REFLECTED;
					new_ray.origin_primitive = prim_index;
					PUSH_RAY(queue, new_ray, back_ray_ptr, rays_in_queue)
				}
			}
		}
				
		uchar red = clamp(acc.x * 256, 0, 255);
		uchar green = clamp(acc.y * 256, 0, 255);
		uchar blue = clamp(acc.z * 256, 0, 255);

		pixels[y * width + x] = (Pixel)(red, green, blue, 0);
}