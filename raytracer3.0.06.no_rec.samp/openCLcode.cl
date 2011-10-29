#define GPU_KERNEL

#include "openCLcode.h"

__kernel void raytrace_kernel( 
		const int height, 
		const int width, 
		global int *pixels, 
		global Primitive *primitiveBuffer, 		
		const int primitiveCount) {
			
    const int gid = get_global_id(0);
	const int x = gid % width;
	const int y = gid / width;	

	const float WX1 = -3.0f;
	const float WX2 = 3.0f;
	const float WY1 = 2.25f;
	const float WY2 = -2.25f;
	const float DX = (WX2 - WX1) / width;
	const float DY = (WY2 - WY1) / height;
	const float SX = WX1 + x * DX;
	const float SY = WY1 + y * DY;

	//// fire primary ray
	////struct vector3 *  acc = vector3_Create( 0, 0, 1 );
	vector3 camera; camera.x = 0.0f; camera.y = 0.25f; camera.z = -7.0f;
	vector3 acc; acc.x = acc.y = acc.z = 0.0f;
	vector3 total_acc; total_acc.x = 0; total_acc.y = 0; total_acc.z = 0;
	
	////struct vector3 * dir = vector3_Create( m_SX, m_SY, 0 );
	vector3 dir; dir.x = SX; dir.y = SY; dir.z = 0.0f;

	////vector3_minusEquals(dir , camera);
	dir.x -= camera.x; dir.y -= camera.y; dir.z -= camera.z;

	////vector3_Normalize( dir );
	const float dir_len = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z );
	if(dir_len > 0.0f){
		dir.x *= (1.0f / dir_len); dir.y *= (1.0f / dir_len); dir.z *= (1.0f / dir_len);
	}else{
		dir.x = dir.y = dir.z = 0;		
	}

	////Ray * r = Ray_Create_FromVec( camera, dir );
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
				
	if (y >= 20){
		if(y < 530){
			for ( int tx = -1; tx < 1; tx++ ) for ( int ty = -1; ty < 1; ty++ )
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
				dir.x = ( SX + DX * tx / 2.0f ) - camera.x;
				dir.y = ( SY + DY * ty / 2.0f ) - camera.y;
				dir.z = ( 0 ) - camera.z;
				//NORMALIZE( dir );
				const float dir_len = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z );
				if(dir_len > 0.0f){
					dir.x *= (1.0f / dir_len); dir.y *= (1.0f / dir_len); dir.z *= (1.0f / dir_len);
				}else{
					dir.x = dir.y = dir.z = 0;		
				}
				o_Ray.m_Direction = dir;
				o_Ray.m_Origin = camera;
				tr_o_Ray[0] = o_Ray;		 
				refl_Ray.m_Direction = dir;
				refl_Ray.m_Origin = camera;				
				tr_refl_Ray[0] = o_Ray;
				refr_Ray.m_Direction = dir;
				refr_Ray.m_Origin = camera;
				tr_refr_Ray[0] = o_Ray;
				float dist = 0;
				float refl = 0;
				float refr = 0;
				int refl_index = -1;
				int refr_index = -1;
				acc.x = acc.y = acc.z = 0;
				float input_Rindex = 1.0f;
				int pindex = Engine_Raytrace( &o_Ray, &acc, 1, &input_Rindex, &dist, primitiveCount, primitiveBuffer, 
											&refl, &refl_index, &refl_Ray, &refr, &refr_index, &refr_Ray);				
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
						pindex = Engine_Raytrace( &o_Ray, &acc, 1, &input_Rindex, &dist, primitiveCount, primitiveBuffer, 
												&refl, &refl_index, &refl_Ray, &refr, &refr_index, &refr_Ray);						
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
						pindex = Engine_Raytrace( &o_Ray, &acc, 1, &input_Rindex, &dist, primitiveCount, primitiveBuffer, 
												&refl, &refl_index, &refl_Ray, &refr, &refr_index, &refr_Ray);
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
				for(int i = NodeCount-1; i >= 2; i-=2){	
					acc.x = tr_Color_x[i-1]; 
					acc.y = tr_Color_y[i-1]; 
					acc.z = tr_Color_z[i-1];

					if ((tr_Refl_index[(i-1)/2] > -1) && (tr_Refl[(i-1)/2] > 0)) {
						acc.x = tr_Color_x[i-1] * primitiveBuffer[tr_Refl_index[(i-1)/2]].m_Material.m_Color.x * tr_Refl[(i-1)/2];
						acc.y = tr_Color_y[i-1] * primitiveBuffer[tr_Refl_index[(i-1)/2]].m_Material.m_Color.y * tr_Refl[(i-1)/2];
						acc.z = tr_Color_z[i-1] * primitiveBuffer[tr_Refl_index[(i-1)/2]].m_Material.m_Color.z * tr_Refl[(i-1)/2];
					}				
					tr_Color_x[(i-1)/2] += acc.x; 
					tr_Color_y[(i-1)/2] += acc.y; 
					tr_Color_z[(i-1)/2] += acc.z;	

					acc.x = tr_Color_x[i]; 
					acc.y = tr_Color_y[i]; 
					acc.z = tr_Color_z[i];

					if ((tr_Refr_index[(i-1)/2] > -1) && (tr_Refr[(i-1)/2] > 0)) {
						vector3 absorbance;
						absorbance.x = primitiveBuffer[tr_Refr_index[(i-1)/2]].m_Material.m_Color.x * 0.15f * -tr_dist[(i-1)/2];
						absorbance.y = primitiveBuffer[tr_Refr_index[(i-1)/2]].m_Material.m_Color.y * 0.15f * -tr_dist[(i-1)/2];
						absorbance.z = primitiveBuffer[tr_Refr_index[(i-1)/2]].m_Material.m_Color.z * 0.15f * -tr_dist[(i-1)/2];
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
				}
				total_acc.x += tr_Color_x[0];
				total_acc.y += tr_Color_y[0];
				total_acc.z += tr_Color_z[0];
			}
			int red = (int)(total_acc.x * (256/4));
			int green = (int)(total_acc.y * (256/4));
			int blue = (int)(total_acc.z * (256/4));
			if (red > 255) red = 255;
			if (green > 255) green = 255;
			if (blue > 255) blue = 255;

			pixels[y * width + x] = red  << 16 | green << 8 | blue;
		}
	}
}