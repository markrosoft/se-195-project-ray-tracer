// -----------------------------------------------------------
// raytracer.cpp
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#include "raytracer.h"
#include "scene.h"
#include "common.h"
#include "windows.h"
#include "winbase.h"

void Engine_Constructor(){
	m_Scene = (Scene *) malloc (sizeof(Scene));
}

void Engine_SetTarget( Pixel* a_Dest, int a_Width, int a_Height )
{
	// set pixel buffer address & size
	m_Dest = a_Dest;
	m_Width = a_Width;
	m_Height = a_Height;
}


//// -----------------------------------------------------------
// Engine_Raytrace
// Naive ray tracing: Intersects the ray with every primitive
// in the scene to determine the closest intersection
// -----------------------------------------------------------
int Engine_Raytrace( Ray * a_Ray, Color * a_Acc, int a_Depth, float * a_RIndex, float * a_Dist, float * a_refl, int * a_refl_Index, Ray *a_refl_Ray, float * a_refr, int * a_refr_Index, Ray *a_refr_Ray )
{
	if (a_Depth > TRACEDEPTH) return -1;
	// trace primary ray
	* a_Dist = 1000000.0f;
	int prim_index = 0;
	int result;
	int hitOnce = 0;
	// find the nearest intersection
	for ( int s = 0; s < m_Scene->m_Primitives; s++ )
	{
		//Primitive pr = m_Scene->m_Primitive[s];// Scene_GetPrimitive( m_Scene, s );
		int res;
		if (res = Primitive_Intersect( &m_Scene->m_Primitive[s], a_Ray, a_Dist )) 
		{
			hitOnce = 1;
			prim_index = s;
			result = res; // 0 = miss or hit something further than last hit, 1 = hit closer than last hit, -1 = hit from inside primitive
		}
	}
	// no hit, terminate ray
	if (hitOnce == 0) return -1;
	// handle intersection
	if (m_Scene->m_Primitive[prim_index].m_Light > 0) 
	{
		// we hit a light, stop tracing
		a_Acc->x += 1; a_Acc->y += 1; a_Acc->z += 1;
	}
	else
	{
		// determine color at point of intersection
		Ray r = *a_Ray;
		vector3 pi;
		vector3_Copy(pi, r.m_Direction);
		vector3_scalarMult(pi, *a_Dist);
		vector3_plusEquals(pi, r.m_Origin);	

		// trace lights
		for ( int p_index = 0; p_index < m_Scene->m_Primitives; p_index++ ) // iterate thru primitives
		{	
			Primitive p = m_Scene->m_Primitive[p_index]; // get the particular primitive		
			if (p.m_Light > 0) // only look for lights
			{
				Primitive light = p; // create local variable
				// added to for Version 2 ////////////////////////////////////////////
				float shade = 1.0f;
				if (light.type == SPHERE)
				{
					//vector3 L = ((Sphere*)light)->GetCentre() - pi;
					vector3 L;
					vector3_Set(&L, light.m_Centre.x - pi.x, light.m_Centre.y - pi.y, light.m_Centre.z - pi.z);
					//float tdist = LENGTH( L );
					float tdist = sqrtf(L.x * L.x + L.y * L.y + L.z * L.z);
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

					for ( int s = 0; s < m_Scene->m_Primitives; s++ )
					{
						Primitive pr = m_Scene->m_Primitive[ s ];
						//if ((!pr.m_Light) && (pr->Intersect( r, tdist )))
						if (pr.m_Light==0){
							if(Primitive_Intersect(&pr, &r, &tdist)){
								shade = 0;//////////////////////////////////////
								//vector3_Set(a_Acc, 1.0f, 1.0f, 1.0f);
								break;
							}else{
								//shade = 1.0f/tdist/0.1f;
							}
						}
					}
				}
				// end of version 2 add /////////////////////////////////////////////////
				if(shade > 0){
					////////////////////////////////////////////////////////////////////////
					// calculate diffuse shading
					vector3 L = light.m_Centre; 
					vector3_minusEquals(L, pi);
					//vector3 L = ((Sphere*)light)->GetCentre() - pi; // get the vector from center to pi
					float L_len = sqrt( L.x * L.x + L.y * L.y + L.z * L.z );
					//vector3_Normalize(L, len); // normalize the vector
					if(L_len > 0.0f){
						L.x *= (1.0f / L_len); L.y *= (1.0f / L_len); L.z *= (1.0f / L_len);
					}else{
						L.x = L.y = L.z = 0;		
					}
					vector3 N; Primitive_GetNormal(&N, m_Scene->m_Primitive[prim_index], pi ); // get the normal vector @ primitive intersect
					if ( m_Scene->m_Primitive[prim_index].m_Material.m_Diff > 0) // check if there is any diffusion
					{
						float dot = vector3_Dot(L , N );
						if (dot > 0)
						{
							float diff = dot * m_Scene->m_Primitive[prim_index].m_Material.m_Diff * shade;
							// add diffuse component to ray color
							//a_Acc += diff * prim->GetMaterial()->GetColor() * light->GetMaterial()->GetColor();
							vector3 dummy_vector;
							vector3_Copy(dummy_vector, m_Scene->m_Primitive[prim_index].m_Material.m_Color);
							vector3_vectorMult(dummy_vector, light.m_Material.m_Color);
							vector3_scalarMult(dummy_vector, diff);
							//vector3_vectorMult(dummy_vector, light.m_Material.m_Color);
							//vector3_scalarMult(dummy_vector, diff);
							a_Acc->x += dummy_vector.x; 
							a_Acc->y += dummy_vector.y; 
							a_Acc->z += dummy_vector.z;
							//vector3_plusEquals(*a_Acc, dummy_vector);
						}
					}
					//  added to for Version 2 /////////////////////////
					// determine specular component
					//if (prim->GetMaterial()->GetSpecular() > 0)
					if (m_Scene->m_Primitive[prim_index].m_Material.m_Spec > 0)
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
							float primSpec = m_Scene->m_Primitive[prim_index].m_Material.m_Spec;
							float spec = powf( dot, 20 ) * primSpec * shade;
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
		*a_refr = m_Scene->m_Primitive[prim_index].m_Material.m_Refr;
		if ((*a_refr > 0) && (a_Depth < TRACEDEPTH))
		{
			//float rindex = prim->GetMaterial()->GetRefrIndex();
			float rindex = m_Scene->m_Primitive[prim_index].m_Material.m_RIndex;
			float n = *a_RIndex / rindex;
			*a_RIndex = rindex;  /////////////////////////////////////////////////////////
			//vector3 N = prim->GetNormal( pi ) * (float)result;
			vector3 N; Primitive_GetNormal(&N, m_Scene->m_Primitive[prim_index], pi ); 
			vector3_scalarMult( N, (float)result );
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
				T.x = (n * dirForT.x) + (n * cosI - sqrtf( cosT2 )) * N.x;
				T.y = (n * dirForT.y) + (n * cosI - sqrtf( cosT2 )) * N.y;
				T.z = (n * dirForT.z) + (n * cosI - sqrtf( cosT2 )) * N.z;
				//Color rcol( 0, 0, 0 );
				/////////vector3 rcol; rcol.x = 0; rcol.y = 0; rcol.z = 0;
				/////////float dist = 0.0f;
				/////Ray r;
				a_refr_Ray->m_Origin.x = pi.x + T.x * EPSILON;
				a_refr_Ray->m_Origin.y = pi.y + T.y * EPSILON;
				a_refr_Ray->m_Origin.z = pi.z + T.z * EPSILON;
				a_refr_Ray->m_Direction = T;
				*a_refr_Index = prim_index;
				//Engine_Raytrace( Ray( pi + T * EPSILON, T ), rcol, a_Depth + 1, rindex, dist );
				/////////Engine_Raytrace( &r, &rcol, a_Depth + 1, rindex, &dist, refl, a_PrimIndex );
				// apply Beer's law
				//Color absorbance = prim->GetMaterial()->GetColor() * 0.15f * -dist;
				/////////vector3 absorbance;
				/////////absorbance.x = m_Scene->m_Primitive[prim_index].m_Material.m_Color.x * 0.15f * -dist;
				/////////absorbance.y = m_Scene->m_Primitive[prim_index].m_Material.m_Color.y * 0.15f * -dist;
				/////////absorbance.z = m_Scene->m_Primitive[prim_index].m_Material.m_Color.z * 0.15f * -dist;
				//Color transparency = Color( expf( absorbance.r ), expf( absorbance.g ), expf( absorbance.b ) );
				/////////vector3 transparency;
				/////////transparency.x = expf( absorbance.x );
				/////////transparency.y = expf( absorbance.y );
				/////////transparency.z = expf( absorbance.z );
				//a_Acc += rcol * transparency;
				/////////a_Acc->x += rcol.x * transparency.x;
				/////////a_Acc->y += rcol.y * transparency.y;
				/////////a_Acc->z += rcol.z * transparency.z;
			}else{
				*a_refr_Index = -1;
			}
		}else{
			*a_refr_Index = -1;
		}
		//  end of Version 3 ADD /////////////////////////

		//  added to for Version 2 /////////////////////////
		// calculate reflection
		*a_refl = m_Scene->m_Primitive[prim_index].m_Material.m_Refl;
		if ( *a_refl > 0.0f)
		{
			//vector3 N = prim->GetNormal( pi );
			vector3 N; Primitive_GetNormal(&N, m_Scene->m_Primitive[prim_index], pi );
			//vector3 R = a_Ray.GetDirection() - 2.0f * DOT( a_Ray.GetDirection(), N ) * N;
			vector3 R; // Check the next 4 lines for programming accuracy
			vector3 dirForR = a_Ray->m_Direction;
			float dotForR = (dirForR.x * N.x + dirForR.y * N.y + dirForR.z * N.z);			
			R.x = dirForR.x - 2.0f * dotForR * N.x;
			R.y = dirForR.y - 2.0f * dotForR * N.y;
			R.z = dirForR.z - 2.0f * dotForR * N.z;
				//Color rcol( 0, 0, 0 );
				//vector3 rcol; rcol.x = 0; rcol.y = 0; rcol.z = 0;
				//float dist;
				//Raytrace( Ray( pi + R * EPSILON, R ), rcol, a_Depth + 1, a_RIndex, dist );
				//Ray r;
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

// -----------------------------------------------------------
// Engine::InitRender
// Initializes the renderer, by resetting the line / tile
// counters and precalculating some values
// -----------------------------------------------------------
void Engine_InitRender()
{
	// set first line to draw to
	m_CurrLine = 20;
	// set pixel buffer address of first pixel
	m_PPos = 20 * m_Width;
	//m_PPos = 0;
	// screen plane in world space coordinates
	m_WX1 = -3, m_WX2 = 3, m_WY1 = m_SY = 2.25f, m_WY2 = -2.25f;
	// calculate deltas for interpolation
	m_DX = (m_WX2 - m_WX1) / m_Width;
	m_DY = (m_WY2 - m_WY1) / m_Height;
	m_SY += 20 * m_DY;
	// allocate space to store pointers to primitives for previous line
	//m_LastRow = new Primitive*[m_Width];
	//memset( m_LastRow, 0, m_Width * 4 );
}

// -----------------------------------------------------------
// Engine::Render
// Fires rays in the scene one scanline at a time, from left
// to right
// -----------------------------------------------------------
bool Engine_Render()
{
	// render scene
	// reset last found primitive pointer
	Primitive* lastprim = 0;
	// render remaining lines
	for ( int y = m_CurrLine; y < (m_Height - 70); y++ )
	{
		m_SX = m_WX1;
		// render pixels for current line
		for ( int x = 0; x < m_Width; x++ )
		{
			// fire primary ray
			vector3 camera; // really just the orgin
			camera.x = 0.0f; camera.y = 0.25f; camera.z = -7.0f; 
			Color acc; acc.x = acc.y = acc.z = 0;
			Color total_acc; total_acc.x = 0; total_acc.y = 0; total_acc.z = 0;
			vector3 dir;
			dir.x = m_SX; dir.y = m_SY; dir.z = 0; //  vector3_Create( m_SX, m_SY, 0 );			
			//vector3_minusEquals(dir , camera);
			dir.x -= camera.x; dir.y -= camera.y; dir.z -= camera.z;
			//float len = vector3_Length(dir);
			float len = (sqrt( dir.x * dir.x + dir.y * dir.y + dir.z * dir.z ));
			//vector3_Normalize(dir, len);
			if(len > 0.0f){
				dir.x *= (1.0f / len); dir.y *= (1.0f / len); dir.z *= (1.0f / len);
			}else{
				dir.x = dir.y = dir.z = 0;		
			}
			//Ray_Create_FromVec( camera, dir );
			Ray o_Ray;
			Ray refl_Ray ;
			Ray refr_Ray;

			//const int NodeCount = 63;
			const int NodeCount = 63;

			Ray tr_o_Ray[NodeCount];
			float tr_Color_x[NodeCount];
			float tr_Color_y[NodeCount];
			float tr_Color_z[NodeCount];
			Ray tr_refl_Ray[NodeCount];
			float tr_Refl[NodeCount];
			int tr_Refl_index[NodeCount];
			float tr_Refr_Rindex[NodeCount];
			Ray tr_refr_Ray[NodeCount];
			float tr_Refr[NodeCount];
			int tr_Refr_index[NodeCount];
			float tr_dist[NodeCount];

			for ( int tx = -1; tx < 2; tx++ ) for ( int ty = -1; ty < 2; ty++ )
			{
				for(int i = 0; i < NodeCount; i++){ 
					tr_Color_x[i] = 0; 
					tr_Color_y[i] = 0; 
					tr_Color_z[i] = 0; 
					tr_Refl[i] = 0; 
					tr_Refl_index[i] = -1;
					tr_Refr_Rindex[i] = 1.0f;
					tr_Refr[i] = 0;
					tr_Refr_index[i] = -1;
					tr_dist[i] = 0;
				}
				dir.x = ( m_SX + m_DX * tx / 2.0f ) - camera.x;
				dir.y = ( m_SY + m_DY * ty / 2.0f ) - camera.y;
				dir.z = ( 0 ) - camera.z;
				NORMALIZE( dir );
				o_Ray.m_Direction = dir;
				o_Ray.m_Origin = camera;		 
				refl_Ray.m_Direction = dir;
				refl_Ray.m_Origin = camera;				
				tr_refl_Ray[0] = o_Ray;
				refr_Ray.m_Direction = dir;
				refr_Ray.m_Origin = camera;

				float dist = 0;
				float refl = 0;
				float refr = 0;
				int refl_index = -1;
				int refr_index = -1;
				acc.x = acc.y = acc.z = 0;
				float input_Rindex = 1.0f;
				int pindex = Engine_Raytrace( &o_Ray, &acc, 1, &input_Rindex, &dist, &refl, &refl_index, &refl_Ray, &refr, &refr_index, &refr_Ray);				
				tr_o_Ray[0] = o_Ray;
				tr_Color_x[0] = acc.x; 
				tr_Color_y[0] = acc.y; 
				tr_Color_z[0] = acc.z;					
				tr_refl_Ray[0] = refl_Ray;
				tr_Refl[0] = refl; 
				tr_Refl_index[0] = refl_index;
				tr_refr_Ray[0] = refr_Ray;
				tr_Refr_Rindex[0] = input_Rindex;
				tr_Refr[0] = refr;
				tr_Refr_index[0] = refr_index;
				tr_dist[0] = dist;
				int refl_done = 0;
				int refr_done = 0;
				for(int i = 1; i < NodeCount; i+=2){
					if (tr_Refl[(i-1)/2] > 0) {
						refl = 0; 
						refl_index = -1;
						o_Ray = tr_refl_Ray[(i-1)/2];
						acc.x = tr_Color_x[i];
						acc.y = tr_Color_y[i];
						acc.z = tr_Color_z[i];
						dist = 0;
						refl = 0;
						refr = 0;
						refl_index = -1;
						refr_index = -1;
						//acc.x = 0; acc.y = 0; acc.z = 0;
						input_Rindex = tr_Refr_Rindex[(i-1)/2];
						pindex = Engine_Raytrace( &o_Ray, &acc, 1, &input_Rindex, &dist, &refl, &refl_index, &refl_Ray, &refr, &refr_index, &refr_Ray);						
						tr_o_Ray[i] = o_Ray;
						tr_Color_x[i] = acc.x; 
						tr_Color_y[i] = acc.y; 
						tr_Color_z[i] = acc.z;				
						tr_refl_Ray[i] = refl_Ray;
						tr_Refl[i] = refl; 
						tr_Refl_index[i] = refl_index;				
						tr_refr_Ray[i] = refr_Ray;
						tr_Refr[i] = refr; 
						tr_Refr_index[i] = refr_index;
						tr_Refr_Rindex[i] = input_Rindex;
						tr_dist[i] = dist;
					} else {
						tr_Color_x[i] = 0; 
						tr_Color_y[i] = 0; 
						tr_Color_z[i] = 0; 
						tr_Refl[i] = 0; 
						tr_Refl_index[i] = -1;
						tr_Refr[i] = 0; 
						tr_Refr_index[i] = -1;
					}

					if(tr_Refr[(i-1)/2]>0){
						o_Ray = tr_refr_Ray[(i-1)/2];
						acc.x = tr_Color_x[i+1];
						acc.y = tr_Color_y[i+1];
						acc.z = tr_Color_z[i+1];
						dist = 0;
						refl = 0;
						refr = 0;
						refl_index = -1;
						refr_index = -1;
						input_Rindex = tr_Refr_Rindex[(i-1)/2];
						//acc.x = 0; acc.y = 0; acc.z = 0;
						//if(tr_Refr[(i-1)/2]>0){
						pindex = Engine_Raytrace( &o_Ray, &acc, 1, &input_Rindex, &dist, &refl, &refl_index, &refl_Ray, &refr, &refr_index, &refr_Ray);
						tr_o_Ray[i+1] = o_Ray;
						tr_Color_x[i+1] = acc.x; 
						tr_Color_y[i+1] = acc.y; 
						tr_Color_z[i+1] = acc.z;				
						tr_refl_Ray[i+1] = refl_Ray;
						tr_Refl[i+1] = refl; 
						tr_Refl_index[i+1] = refl_index;				
						tr_Refr_Rindex[i+1] = input_Rindex;
						tr_refr_Ray[i+1] = refr_Ray;
						tr_Refr[i+1] = refr; 
						tr_Refr_index[i+1] = refr_index;
						tr_dist[i+1] = dist;
					} else {

						tr_Color_x[i+1] = 0; 
						tr_Color_y[i+1] = 0; 
						tr_Color_z[i+1] = 0; 
						tr_Refl[i+1] = 0; 
						tr_Refl_index[i+1] = -1;
						tr_Refr[i+1] = 0; 
						tr_Refr_index[i+1] = -1;
					}
				}

				acc.x = acc.y = acc.z = 0;
				//for(int i = NodeCount-1; i >= 0; i--){					
				for(int i = NodeCount-1; i >= 2; i-=2){					
					acc.x = tr_Color_x[i]; 
					acc.y = tr_Color_y[i]; 
					acc.z = tr_Color_z[i];

					if ((tr_Refr_index[(i-1)/2] > -1) && (tr_Refr[(i-1)/2] > 0)) {
						vector3 absorbance;
						absorbance.x = m_Scene->m_Primitive[tr_Refr_index[(i-1)/2]].m_Material.m_Color.x * 0.15f * -tr_dist[(i-1)/2];
						absorbance.y = m_Scene->m_Primitive[tr_Refr_index[(i-1)/2]].m_Material.m_Color.y * 0.15f * -tr_dist[(i-1)/2];
						absorbance.z = m_Scene->m_Primitive[tr_Refr_index[(i-1)/2]].m_Material.m_Color.z * 0.15f * -tr_dist[(i-1)/2];
						vector3 transparency;
						transparency.x = exp( absorbance.x );
						transparency.y = exp( absorbance.y );
						transparency.z = exp( absorbance.z );		
						acc.x = tr_Color_x[i] *transparency.x; 
						acc.y = tr_Color_y[i] *transparency.y; 
						acc.z = tr_Color_z[i] *transparency.z;	
					}				
					tr_Color_x[(i-1)/2] += acc.x; 
					tr_Color_y[(i-1)/2] += acc.y; 
					tr_Color_z[(i-1)/2] += acc.z;


					acc.x = tr_Color_x[i-1]; 
					acc.y = tr_Color_y[i-1]; 
					acc.z = tr_Color_z[i-1];

					if ((tr_Refl_index[(i-1)/2] > -1) && (tr_Refl[(i-1)/2] > 0)) {
						acc.x = tr_Color_x[i-1] * m_Scene->m_Primitive[tr_Refl_index[(i-1)/2]].m_Material.m_Color.x * tr_Refl[(i-1)/2];
						acc.y = tr_Color_y[i-1] * m_Scene->m_Primitive[tr_Refl_index[(i-1)/2]].m_Material.m_Color.y * tr_Refl[(i-1)/2];
						acc.z = tr_Color_z[i-1] * m_Scene->m_Primitive[tr_Refl_index[(i-1)/2]].m_Material.m_Color.z * tr_Refl[(i-1)/2];
					}				
					tr_Color_x[(i-1)/2] += acc.x; 
					tr_Color_y[(i-1)/2] += acc.y; 
					tr_Color_z[(i-1)/2] += acc.z;
				}

				total_acc.x += tr_Color_x[0];
				total_acc.y += tr_Color_y[0];
				total_acc.z += tr_Color_z[0];
			}
			int red = (int)(total_acc.x * (256/9));
			int green = (int)(total_acc.y * (256/9));
			int blue = (int)(total_acc.z * (256/9));
			if (red > 255) red = 255;
			if (green > 255) green = 255;
			if (blue > 255) blue = 255;
			m_Dest[m_PPos++] = (red << 16) + (green << 8) + blue;
			m_SX += m_DX;
		}
		m_SY += m_DY;
	}
	// all done
	return true;
}


bool Engine_Render_Simple()
{
	// render remaining lines	
	for ( int y = m_CurrLine; y < (m_Height - 70); y++ )
	{
		m_SX = m_WX1;
		for ( int x = 0; x < m_Width; x++ )
		{
			vector3 acc_newcolor;
			acc_newcolor.x = 0.5f; acc_newcolor.y = 0; acc_newcolor.z = 0;
			//Primitive* prim = Engine_Raytrace( r, acc_newcolor, 1, 1.0f, &dist );
			int red = (int)(acc_newcolor.x * 256);
			int green = (int)(acc_newcolor.y * 256);
			int blue = (int)(acc_newcolor.z * 256);
			if (red > 255) red = 255;
			if (green > 255) green = 255;
			if (blue > 255) blue = 255;
			m_Dest[m_PPos++] = (red << 16) + (green << 8) + blue;

			m_SX += m_DX;
		}
		m_SY += m_DY;
	}
	return true;
}