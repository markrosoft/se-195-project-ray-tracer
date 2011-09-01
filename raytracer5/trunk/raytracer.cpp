// -----------------------------------------------------------
// raytracer.cpp
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#include "raytracer.h"
#include "scene.h"
#include "common.h"
#include "windows.h"
#include "winbase.h"

namespace Raytracer {

Ray::Ray( vector3& a_Origin, vector3& a_Dir, int a_ID ) : 
	m_Origin( a_Origin ), 
	m_Direction( a_Dir ),
	m_ID( a_ID )
{
}

Engine::Engine()
{
	m_Scene = new Scene();
}

Engine::~Engine()
{
	delete m_Scene;
}

// -----------------------------------------------------------
// Engine::SetTarget
// Sets the render target canvas
// -----------------------------------------------------------
void Engine::SetTarget( Pixel* a_Dest, int a_Width, int a_Height )
{
	// set pixel buffer address & size
	m_Dest = a_Dest;
	m_Width = a_Width;
	m_Height = a_Height;
	// precalculate 1 / size of a cell (for x, y and z)
	m_SR.x = GRIDSIZE / m_Scene->GetExtends().GetSize().x;
	m_SR.y = GRIDSIZE / m_Scene->GetExtends().GetSize().y;
	m_SR.z = GRIDSIZE / m_Scene->GetExtends().GetSize().z;
	// precalculate size of a cell (for x, y, and z)
	m_CW = m_Scene->GetExtends().GetSize() * (1.0f / GRIDSIZE);
}

// -----------------------------------------------------------
// Engine::FindNearest
// Finds the nearest intersection in a regular gird for a ray
// -----------------------------------------------------------
int Engine::FindNearest( Ray& a_Ray, float& a_Dist, Primitive*& a_Prim )
{
	int retval = MISS;
	vector3 raydir, curpos;
	Box e = m_Scene->GetExtends();
	curpos = a_Ray.GetOrigin();
	raydir = a_Ray.GetDirection();
	// setup 3DDDA (double check reusability of primary ray data)
	vector3 cb, tmax, tdelta, cell;
	cell = (curpos - e.GetPos()) * m_SR;
	int stepX, outX, X = (int)cell.x;
	int stepY, outY, Y = (int)cell.y;
	int stepZ, outZ, Z = (int)cell.z;
	if ((X < 0) || (X >= GRIDSIZE) || (Y < 0) || (Y >= GRIDSIZE) || (Z < 0) || (Z >= GRIDSIZE)) return 0;
	if (raydir.x > 0)
	{
		stepX = 1, outX = GRIDSIZE;
		cb.x = e.GetPos().x + (X + 1) * m_CW.x;
	}
	else 
	{
		stepX = -1, outX = -1;
		cb.x = e.GetPos().x + X * m_CW.x;
	}
	if (raydir.y > 0.0f)
	{
		stepY = 1, outY = GRIDSIZE;
		cb.y = e.GetPos().y + (Y + 1) * m_CW.y; 
	}
	else 
	{
		stepY = -1, outY = -1;
		cb.y = e.GetPos().y + Y * m_CW.y;
	}
	if (raydir.z > 0.0f)
	{
		stepZ = 1, outZ = GRIDSIZE;
		cb.z = e.GetPos().z + (Z + 1) * m_CW.z;
	}
	else 
	{
		stepZ = -1, outZ = -1;
		cb.z = e.GetPos().z + Z * m_CW.z;
	}
	float rxr, ryr, rzr;
	if (raydir.x != 0)
	{
		rxr = 1.0f / raydir.x;
		tmax.x = (cb.x - curpos.x) * rxr; 
		tdelta.x = m_CW.x * stepX * rxr;
	}
	else tmax.x = 1000000;
	if (raydir.y != 0)
	{
		ryr = 1.0f / raydir.y;
		tmax.y = (cb.y - curpos.y) * ryr; 
		tdelta.y = m_CW.y * stepY * ryr;
	}
	else tmax.y = 1000000;
	if (raydir.z != 0)
	{
		rzr = 1.0f / raydir.z;
		tmax.z = (cb.z - curpos.z) * rzr; 
		tdelta.z = m_CW.z * stepZ * rzr;
	}
	else tmax.z = 1000000;
	// start stepping
	ObjectList* list = 0;
	ObjectList** grid = m_Scene->GetGrid();
	a_Prim = 0;
	// trace primary ray
	while (1)
	{
		list = grid[X + (Y << GRIDSHFT) + (Z << (GRIDSHFT * 2))];
		while (list)
		{
			Primitive* pr = list->GetPrimitive();
			int result;
			if (pr->GetLastRayID() != a_Ray.GetID()) if (result = pr->Intersect( a_Ray, a_Dist )) 
			{
				retval = result;
				a_Prim = pr;
				goto testloop;
			}
			list = list->GetNext();
		}
		if (tmax.x < tmax.y)
		{
			if (tmax.x < tmax.z)
			{
				X = X + stepX;
				if (X == outX) return MISS;
				tmax.x += tdelta.x;
			}
			else
			{
				Z = Z + stepZ;
				if (Z == outZ) return MISS;
				tmax.z += tdelta.z;
			}
		}
		else
		{
			if (tmax.y < tmax.z)
			{
				Y = Y + stepY;
				if (Y == outY) return MISS;
				tmax.y += tdelta.y;
			}
			else
			{
				Z = Z + stepZ;
				if (Z == outZ) return MISS;
				tmax.z += tdelta.z;
			}
		}
	}
testloop:
	while (1)
	{
		list = grid[X + (Y << GRIDSHFT) + (Z << (GRIDSHFT * 2))];
		while (list)
		{
			Primitive* pr = list->GetPrimitive();
			int result;
			if (pr->GetLastRayID() != a_Ray.GetID()) if (result = pr->Intersect( a_Ray, a_Dist )) 
			{
				a_Prim = pr;
				retval = result;
			}
			list = list->GetNext();
		}
		if (tmax.x < tmax.y)
		{
			if (tmax.x < tmax.z)
			{
				if (a_Dist < tmax.x) break;
				X = X + stepX;
				if (X == outX) break;
				tmax.x += tdelta.x;
			}
			else
			{
				if (a_Dist < tmax.z) break;
				Z = Z + stepZ;
				if (Z == outZ) break;
				tmax.z += tdelta.z;
			}
		}
		else
		{
			if (tmax.y < tmax.z)
			{
				if (a_Dist < tmax.y) break;
				Y = Y + stepY;
				if (Y == outY) break;
				tmax.y += tdelta.y;
			}
			else
			{
				if (a_Dist < tmax.z) break;
				Z = Z + stepZ;
				if (Z == outZ) break;
				tmax.z += tdelta.z;
			}
		}
	}
	return retval;
}

// -----------------------------------------------------------
// Engine::Raytrace
// Naive ray tracing: Intersects the ray with every primitive
// in the scene to determine the closest intersection
// -----------------------------------------------------------
Primitive* Engine::Raytrace( Ray& a_Ray, Color& a_Acc, int a_Depth, float a_RIndex, float& a_Dist, float a_Samples, float a_SScale )
{
	if (a_Depth > TRACEDEPTH) return 0;
	// trace primary ray
	a_Dist = 1000000.0f;
	vector3 pi;
	Primitive* prim = 0;
	int result;
	// find the nearest intersection
	if (!(result = FindNearest( a_Ray, a_Dist, prim ))) return 0;
	// handle intersection
	if (prim->IsLight())
	{
		// we hit a light, stop tracing
		a_Acc = prim->GetMaterial()->GetColor();
	}
	else
	{
		// determine color at point of intersection
		pi = a_Ray.GetOrigin() + a_Ray.GetDirection() * a_Dist;
		Color color = prim->GetColor( pi );
		// trace lights
		for ( int l = 0; l < m_Scene->GetNrLights(); l++ )
		{
			Primitive* light = m_Scene->GetLight( l );
			// handle point light source
			vector3 L;
			float shade = CalcShade( light, pi, L, a_Samples, a_SScale );
			if (shade > 0)
			{
				// calculate diffuse shading
				vector3 N = prim->GetNormal( pi );
				if (prim->GetMaterial()->GetDiffuse() > 0)
				{
					float dot = DOT( L, N );
					if (dot > 0)
					{
						float diff = dot * prim->GetMaterial()->GetDiffuse() * shade;
						// add diffuse component to ray color
						a_Acc += diff * color * light->GetMaterial()->GetColor();
					}
				}
				// determine specular component
				if (prim->GetMaterial()->GetSpecular() > 0)
				{
					// point light source: sample once for specular highlight
					vector3 V = a_Ray.GetDirection();
					vector3 R = L - 2.0f * DOT( L, N ) * N;
					float dot = DOT( V, R );
					if (dot > 0)
					{
						float spec = powf( dot, 20 ) * prim->GetMaterial()->GetSpecular() * shade;
						// add specular component to ray color
						a_Acc += spec * light->GetMaterial()->GetColor();
					}
				}
			}
		}
		// calculate reflection
		float refl = prim->GetMaterial()->GetReflection();
		if ((refl > 0.0f) && (a_Depth < TRACEDEPTH))
		{
			float drefl = prim->GetMaterial()->GetDiffuseRefl();
			if ((drefl > 0) && (a_Depth < 3))
			{
				// calculate diffuse reflection
				vector3 N = prim->GetNormal( pi );
				vector3 RP = a_Ray.GetDirection() - 2.0f * DOT( a_Ray.GetDirection(), N ) * N;
				vector3 RN1 = vector3( RP.z, RP.y, -RP.x );
				vector3 RN2 = RP.Cross( RN1 );
				refl *= a_SScale;
				for ( int i = 0; i < SAMPLES; i++ )
				{
					float xoffs, yoffs;
					do
					{
						xoffs = (m_Twister.Rand() - 0.5f) * drefl;
						yoffs = (m_Twister.Rand() - 0.5f) * drefl;
					}
					while ((xoffs * xoffs + yoffs * yoffs) > (drefl * drefl));
					vector3 R = RP + RN1 * xoffs + RN2 * yoffs * drefl;
					NORMALIZE( R );
					float dist;
					Color rcol( 0, 0, 0 );
				#ifdef IMPORTANCE
					Raytrace( Ray( pi + R * EPSILON, R, ++m_CurID ), rcol, a_Depth + 1, a_RIndex, dist, a_Samples * 0.25f, a_SScale * 4 );
				#else
					Raytrace( Ray( pi + R * EPSILON, R, ++m_CurID ), rcol, a_Depth + 1, a_RIndex, dist, a_Samples, a_SScale );
				#endif
					a_Acc += refl * rcol * color;
				}
			}
			else
			{
				// calculate perfect reflection
				vector3 N = prim->GetNormal( pi );
				vector3 R = a_Ray.GetDirection() - 2.0f * DOT( a_Ray.GetDirection(), N ) * N;
				Color rcol( 0, 0, 0 );
				float dist;
			#ifdef IMPORTANCE
				Raytrace( Ray( pi + R * EPSILON, R, ++m_CurID ), rcol, a_Depth + 1, a_RIndex, dist, a_Samples * 0.5f, a_SScale * 2 );
			#else
				Raytrace( Ray( pi + R * EPSILON, R, ++m_CurID ), rcol, a_Depth + 1, a_RIndex, dist, a_Samples, a_SScale );
			#endif
				a_Acc += refl * rcol * color;
			}
		}
		// calculate refraction
		float refr = prim->GetMaterial()->GetRefraction();
		if ((refr > 0) && (a_Depth < TRACEDEPTH))
		{
			float rindex = prim->GetMaterial()->GetRefrIndex();
			float n = a_RIndex / rindex;
			vector3 N = prim->GetNormal( pi ) * (float)result;
			float cosI = -DOT( N, a_Ray.GetDirection() );
			float cosT2 = 1.0f - n * n * (1.0f - cosI * cosI);
			if (cosT2 > 0.0f)
			{
				vector3 T = (n * a_Ray.GetDirection()) + (n * cosI - sqrtf( cosT2 )) * N;
				Color rcol( 0, 0, 0 );
				float dist;
			#ifdef IMPORTANCE
				Raytrace( Ray( pi + T * EPSILON, T, ++m_CurID ), rcol, a_Depth + 1, rindex, dist, a_Samples * 0.5f, a_SScale * 2 );
			#else
				Raytrace( Ray( pi + T * EPSILON, T, ++m_CurID ), rcol, a_Depth + 1, rindex, dist, a_Samples, a_SScale );
			#endif
				// apply Beer's law
				Color absorbance = prim->GetMaterial()->GetColor() * 0.15f * -dist;
				Color transparency = Color( expf( absorbance.r ), expf( absorbance.g ), expf( absorbance.b ) );
				a_Acc += rcol * transparency;
			}
		}
	}
	// return pointer to primitive hit by primary ray
	return prim;
}

// -----------------------------------------------------------
// Engine::CalcShade
// Determines the light intensity received from a point light
// (in case of a SPHERE primitive) or an area light (in case
// of an AABB primitive)
// -----------------------------------------------------------
float Engine::CalcShade( Primitive* a_Light, vector3 a_IP, vector3& a_Dir, float a_Samples, float a_SScale )
{
	float retval;
	Primitive* prim = 0;
	if (a_Light->GetType() == Primitive::SPHERE)
	{
		// handle point light source
		retval = 1.0f;
		a_Dir = ((Sphere*)a_Light)->GetCentre() - a_IP;
		float tdist = LENGTH( a_Dir );
		a_Dir *= (1.0f / tdist);
		FindNearest( Ray( a_IP + a_Dir * EPSILON, a_Dir, ++m_CurID ), tdist, prim );
		if (prim != a_Light) retval = 0;
	}
	else if (a_Light->GetType() == Primitive::AABB)
	{
		retval = 0;
		Box* b = (Box*)a_Light;
		a_Dir = (b->GetPos() + 0.5f * b->GetSize()) - a_IP;
		NORMALIZE( a_Dir );
		// Monte Carlo rendering
		float deltax = b->GetSize().x * 0.25f, deltay = b->GetSize().z * 0.25f;
		for ( int i = 0; i < a_Samples; i++ )
		{
			vector3 lp( b->GetGridX( i & 15 ) + m_Twister.Rand() * deltax, b->GetPos().y, b->GetGridY( i & 15 ) + m_Twister.Rand() * deltay );
			vector3 dir = lp - a_IP;
			float ldist = (float)LENGTH( dir );
			dir *= 1.0f / ldist;
			if (FindNearest( Ray( a_IP + dir * EPSILON, dir, ++m_CurID ), ldist, prim ))
			{
				if (prim == a_Light) retval += a_SScale;
			}
		}
	}
	return retval;
}

// -----------------------------------------------------------
// Engine::InitRender
// Initializes the renderer, by resetting the line / tile
// counters and precalculating some values
// -----------------------------------------------------------
void Engine::InitRender( vector3& a_Pos, vector3& a_Target )
{
	// set firts line to draw to
	m_CurrLine = 20;
	// set pixel buffer address of first pixel
	m_PPos = m_CurrLine * m_Width;
	// allocate space to store pointers to primitives for previous line
	m_LastRow = new Primitive*[m_Width];
	memset( m_LastRow, 0, m_Width * 4 );
	// reset ray id counter
	m_CurID = 0;
	// set eye and screen plane position
	m_Origin = vector3( 0, 0, -5 );
	m_P1 = vector3( -4,  3, 0 );
	m_P2 = vector3(  4,  3, 0 );
	m_P3 = vector3(  4, -3, 0 );
	m_P4 = vector3( -4, -3, 0 );
	// calculate camera matrix
	vector3 zaxis = a_Target - a_Pos;
	zaxis.Normalize();
	vector3 up( 0, 1, 0 );
	vector3 xaxis = up.Cross( zaxis );
	vector3 yaxis = xaxis.Cross( -zaxis );
	matrix m;
	m.cell[0] = xaxis.x, m.cell[1] = xaxis.y, m.cell[2] = xaxis.z;
	m.cell[4] = yaxis.x, m.cell[5] = yaxis.y, m.cell[6] = yaxis.z;
	m.cell[8] = zaxis.x, m.cell[9] = zaxis.y, m.cell[10] = zaxis.z;
	m.Invert();
	m.cell[3] = a_Pos.x, m.cell[7] = a_Pos.y, m.cell[11] = a_Pos.z;
	// move camera
	m_Origin = m.Transform( m_Origin );
	m_P1 = m.Transform( m_P1 );
	m_P2 = m.Transform( m_P2 );
	m_P3 = m.Transform( m_P3 );
	m_P4 = m.Transform( m_P4 );
	// calculate screen plane interpolation vectors
	m_DX = (m_P2 - m_P1) * (1.0f / m_Width);
	m_DY = (m_P4 - m_P1) * (1.0f / m_Height);
}

// -----------------------------------------------------------
// Engine::RenderRay
// Helper function, fires one ray in the regular grid
// -----------------------------------------------------------
Primitive* Engine::RenderRay( vector3 a_ScreenPos, Color& a_Acc )
{
	Box e = m_Scene->GetExtends();
	vector3 o( 0, 0, -5 );
	vector3 dir = a_ScreenPos - m_Origin;
	NORMALIZE( dir );
	Color acc( 0, 0, 0 );
	Ray r( o, dir, ++m_CurID );
	// advance ray to scene bounding box boundary
	if (!e.Contains( o ))
	{
		float bdist = 10000.0f;
		if (e.Intersect( r, bdist )) r.SetOrigin( o + (bdist + EPSILON) * dir );
	}
	float dist;
	// trace ray
	return Raytrace( r, a_Acc, 1, 1.0f, dist, SAMPLES, 1.0f / SAMPLES );
}

// -----------------------------------------------------------
// Engine::Render
// Fires rays in the scene one scanline at a time, from left
// to right
// -----------------------------------------------------------
bool Engine::Render()
{
	// render scene
	aabb e = m_Scene->GetExtends();
	// initialize timer
	int msecs = GetTickCount();
	// reset last found primitive pointer
	Primitive* lastprim = 0;
	// render remaining lines
	for ( int y = m_CurrLine; y < (m_Height - 20); y++ )
	{
		// render pixels for current line
		vector3 lpos = m_P1 + (float)y * m_DY;
		for ( int x = 0; x < m_Width; x++ )
		{
			// fire primary rays
			Color acc( 0, 0, 0 );
			Primitive* prim = RenderRay( lpos, acc );
			int red, green, blue;
			if (prim != lastprim)
			{
				lastprim = prim;
				RenderRay( lpos - m_DX * 0.5f, acc );
				RenderRay( lpos - m_DY * 0.5f, acc );
				RenderRay( lpos - m_DX * 0.5f - m_DY * 0.5f, acc );
				red = (int)(acc.r * 64);
				green = (int)(acc.g * 64);
				blue = (int)(acc.b * 64);
				if (red > 255) red = 255;
				if (green > 255) green = 255;
				if (blue > 255) blue = 255;
			}
			else
			{
				red = (int)(acc.r * 256);
				green = (int)(acc.g * 256);
				blue = (int)(acc.b * 256);
				if (red > 255) red = 255;
				if (green > 255) green = 255;
				if (blue > 255) blue = 255;
			}
			m_Dest[m_PPos++] = (red << 16) + (green << 8) + blue;
			lpos += m_DX;
		}
		// see if we've been working to long already
		if ((GetTickCount() - msecs) > 100) 
		{
			// return control to windows so the screen gets updated
			m_CurrLine = y + 1;
			return false;
		}
	}
	// all done
	return true;
}

}; // namespace Raytracer