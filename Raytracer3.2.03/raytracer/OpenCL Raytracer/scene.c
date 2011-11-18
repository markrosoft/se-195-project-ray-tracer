#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Material create_material(cl_float r, cl_float g, cl_float b, cl_float refl, cl_float refr, cl_float refr_index, cl_float diff, cl_float spec)
{
	Material m;
	m.color.s[0] = r;
	m.color.s[1] = g;
	m.color.s[2] = b;
	m.refl = refl;
	m.diff = diff;
	m.refr = refr;
	m.refr_index = refr_index;
	m.spec = spec;
	return m;
}

Primitive create_plane(Material m, cl_bool is_light, cl_float normal_x, cl_float normal_y, cl_float normal_z, cl_float depth)
{
	Primitive p;
	p.type = PLANE;
	p.material = m;
	p.is_light = is_light;
	p.normal.s[0] = normal_x;
	p.normal.s[1] = normal_y;
	p.normal.s[2] = normal_z;
	p.depth = depth;
	return p;
}

Primitive create_sphere(Material m, cl_bool is_light, cl_float center_x, cl_float center_y, cl_float center_z, cl_float radius)
{
	Primitive p;
	p.type = SPHERE;
	p.material = m;
	p.is_light = is_light;
	p.center.s[0] = center_x;
	p.center.s[1] = center_y;
	p.center.s[2] = center_z;
	p.radius = radius;
	p.sq_radius = radius * radius;
	p.r_radius = 1.0f/radius;
	return p;
}

Primitive * create_scene(cl_uint & n_primitives)
{	
	int curr_prim = 0;
	Primitive * primitive_list;

	if (CHOOSE_SCENE == 0){

		n_primitives = 17;
		primitive_list = (Primitive *) malloc(sizeof(Primitive) * n_primitives);
		memset(primitive_list, 0, sizeof(Primitive) * n_primitives);

		float light = 0.85f;
		//create_plane(create_material(r, g, b, refl, refr, refr_index, diff, spec), is_light, x, y, z, depth/radius);
		// floor plane
		primitive_list[curr_prim++] = create_plane(create_material(0.6f, 0.6f, 0.6f, 0.0f, 0.0f, 0.0f, 0.4f, 1.8f), false, 0.0f, 0.75f, 0.0f, 4.4f);
		// big sphere
		primitive_list[curr_prim++] = create_sphere(create_material(0.08f, 0.08f, 0.08f, 0.2f, 1.0f, 1.4f, 0.0f, 0.0f), false, 3.4f, -3.4f, 23.0f, 2.5f);
		// small sphere 5
		primitive_list[curr_prim++] = create_sphere(create_material(0.07f, 0.17f, 0.07f, 0.1f, 1.0f, 1.2f, 0.0f, 0.0f), false, -0.7f, -4.90f, 27.0f, 1.0f);
		// small sphere
		primitive_list[curr_prim++] = create_sphere(create_material(1.0f, 1.0f, 1.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f), false, -3.4f, -3.4f, 29.0f, 2.5f);
		// small sphere 2
		primitive_list[curr_prim++] = create_sphere(create_material(1.5f, 0.7f, 0.7f, 0.1f, 0.0f, 0.0f, 0.2f, 0.2f), false, 0.5f, -4.1f, 29.0f, 1.5f);
		// small sphere 3
		primitive_list[curr_prim++] = create_sphere(create_material(0.7f, 0.7f, 1.7f, 0.2f, 0.0f, 0.0f, 0.2f, 0.2f), false, -6.0f, -4.1f, 32.0f, 1.5f);
		// small sphere 4
		primitive_list[curr_prim++] = create_sphere(create_material(0.07f, 0.17f, 0.07f, 0.3f, 1.0f, 1.2f, 0.2f, 0.8f), false, -6.7f, -4.90f, 29.0f, 1.0f);
		// small sphere 6 up front
		primitive_list[curr_prim++] = create_sphere(create_material(0.08f, 0.08f, 0.08f, 0.7f, 1.0f, 1.3f, 0.8f, 0.0f), false, 6.4f, -4.9f, 18.0f, 1.0f);
		// left wall
		primitive_list[curr_prim++] = create_plane(create_material(1.0f, 0.6f, 0.6f, 0.0f, 0.0f, 0.0f, 0.8f, 1.5f), false, 0.7f, 0.0f, 0.0f, 5.4f);
		// right wall
		primitive_list[curr_prim++] = create_plane(create_material(0.7f, 0.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.8f, 0.8f), false, -0.7f, 0.0f, 0.0f, 5.4f);
		// top wall
		primitive_list[curr_prim++] = create_plane(create_material(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.2f, 0.8f), false, 0.0f, -0.8f, 0.0f, 5.4f);
		// back wall
		primitive_list[curr_prim++] = create_plane(create_material(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 1.2f, 0.8f), false, 0.0f, 0.0f, -0.14f, 5.4f);
		// front wall
		primitive_list[curr_prim++] = create_plane(create_material(0.1f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f), false, 0.0f, 0.0f, 0.72f, 5.4f);
		// light source center
		primitive_list[curr_prim++] = create_sphere(create_material(light, light, light, 0.0f, 0.0f, 0.0f, 0.0f, 1.8f), true, 0.0f, 6.5f, 22.0f, 0.35f);
		// light source right
		primitive_list[curr_prim++] = create_sphere(create_material(light, light, light, 0.0f, 0.0f, 0.0f, 0.0f, 1.8f), true, -3.0f, 6.5f, 22.0f, 0.35f);
		// light source left
		primitive_list[curr_prim++] = create_sphere(create_material(light, light, light, 0.0f, 0.0f, 0.0f, 0.0f, 1.8f), true, 3.0f, 6.5f, 22.0f, 0.35f);	
		/*	
		// light source ground back
		primitive_list[curr_prim++] = create_sphere(create_material(0.85f, 0.25f, 0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), false, -5.8f, -5.55f, 31.0f, 0.35f);
		*/

	}else if(CHOOSE_SCENE == 1){		
		n_primitives = 64;
		primitive_list = (Primitive *) malloc(sizeof(Primitive) * n_primitives);
		memset(primitive_list, 0, sizeof(Primitive) * n_primitives);
		//create_plane(create_material(r, g, b, refl, refr, refr_index, diff, spec), is_light, x, y, z, depth/radius);
		// floor plane
		primitive_list[curr_prim++] = create_plane(create_material(0.4f, 0.3f, 0.3f, 0.0f, 0.0f, 1.0f, 1.0f, 0.8f), false, 0.0f, 1.0f, 0.0f, 4.4f);
		// big sphere
		primitive_list[curr_prim++] = create_sphere(create_material(0.7f, 0.7f, 1.0f, 0.0f, 1.0f, 1.3f, 0.2f, 0.8f), false, 2.0f, 0.8f, 3.0f, 2.5f);
		// small sphere
		primitive_list[curr_prim++] = create_sphere(create_material(0.7f, 0.7f, 1.0f, 0.5f, 0.0f, 1.0f, 0.1f, 0.8f), false, -5.5f, -0.5f, 7.0f, 2.0f);
		// light source 1
		primitive_list[curr_prim++] = create_sphere(create_material(0.4f, 0.4f, 0.4f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f), true, 0.0f, 5.0f, 5.0f, 0.1f);
		// light source 2
		primitive_list[curr_prim++] = create_sphere(create_material(0.6f, 0.6f, 0.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f), true, -3.0f, 5.0f, 1.0f, 0.1f);
		// sphere 3
		primitive_list[curr_prim++] = create_sphere(create_material(1.0f, 0.4f, 0.4f, 0.5f, 0.0f, 1.0f, 0.2f, 0.8f), false, -1.5f, -3.8f, 1.0f, 1.5f);
		// back plane
		primitive_list[curr_prim++] = create_plane(create_material(0.5f, 0.3f, 0.5f, 0.0f, 0.0f, 1.0f, 0.6f, 0.0f), false, 0.4f, 0.0f, -1.0f, 12.0f);
		// ceiling plane
		primitive_list[curr_prim++] = create_plane(create_material(0.4f, 0.7f, 0.7f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f), false, 0.0f, -1.0f, 0.0f, 7.4f);

		for (int x = 0; x < 8; x++)
			for (int y = 0; y < 7; y++)
			{
				primitive_list[curr_prim++] = create_sphere(create_material(0.3f, 1.0f, 0.4f, 0.0f, 0.0f, 1.0f, 0.6f, 0.6f), false, -4.5f + x * 1.5f, -4.3f + y * 1.5f, 10.0f, 0.3f);
			}	
	}

	return primitive_list;
}