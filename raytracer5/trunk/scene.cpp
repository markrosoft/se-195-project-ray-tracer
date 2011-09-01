// -----------------------------------------------------------
// scene.cpp
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#define _CRT_SECURE_NO_WARNINGS 1

#include "common.h"
#include "string.h"
#include "scene.h"
#include "raytracer.h"
#include "stdio.h"

namespace Raytracer {

// -----------------------------------------------------------
// Primitive class implementation
// -----------------------------------------------------------

void Primitive::SetName( char* a_Name )
{
	delete m_Name; 
	m_Name = new char[strlen( a_Name ) + 1]; 
	strcpy( m_Name, a_Name ); 
}

// -----------------------------------------------------------
// Texture class implementation
// -----------------------------------------------------------

Texture::Texture( Color* a_Bitmap, int a_Width, int a_Height ) :
	m_Bitmap( a_Bitmap ),
	m_Width( a_Width ), m_Height( a_Height )
{
}

Texture::Texture( char* a_File )
{
	FILE* f = fopen( a_File, "rb" );
	if (f)
	{
		// extract width and height from file header
		unsigned char buffer[20];
		fread( buffer, 1, 20, f );
		m_Width = *(buffer + 12) + 256 * *(buffer + 13);
		m_Height = *(buffer + 14) + 256 * *(buffer + 15);
		fclose( f );
		// read pixel data
		f = fopen( a_File, "rb" );
		unsigned char* t = new unsigned char[m_Width * m_Height * 3 + 1024];
		fread( t, 1, m_Width * m_Height * 3 + 1024, f );
		fclose( f );
		// convert RGB 8:8:8 pixel data to floating point RGB
		m_Bitmap = new Color[m_Width * m_Height];
		float rec = 1.0f / 256;
		for ( int size = m_Width * m_Height, i = 0; i < size; i++ )
			m_Bitmap[i] = Color( t[i * 3 + 20] * rec, t[i * 3 + 19] * rec, t[i * 3 + 18] * rec );
		delete t;
	}
}

Color Texture::GetTexel( float a_U, float a_V )
{
	// fetch a bilinearly filtered texel
	float fu = (a_U + 1000.5f) * m_Width;
	float fv = (a_V + 1000.0f) * m_Width;
	int u1 = ((int)fu) % m_Width;
	int v1 = ((int)fv) % m_Height;
	int u2 = (u1 + 1) % m_Width;
	int v2 = (v1 + 1) % m_Height;
	// calculate fractional parts of u and v
	float fracu = fu - floorf( fu );
	float fracv = fv - floorf( fv );
	// calculate weight factors
	float w1 = (1 - fracu) * (1 - fracv);
	float w2 = fracu * (1 - fracv);
	float w3 = (1 - fracu) * fracv;
	float w4 = fracu *  fracv;
	// fetch four texels
	Color c1 = m_Bitmap[u1 + v1 * m_Width];
	Color c2 = m_Bitmap[u2 + v1 * m_Width];
	Color c3 = m_Bitmap[u1 + v2 * m_Width];
	Color c4 = m_Bitmap[u2 + v2 * m_Width];
	// scale and sum the four colors
	return c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
}

// -----------------------------------------------------------
// Material class implementation
// -----------------------------------------------------------

Material::Material() :
	m_Color( Color( 0.2f, 0.2f, 0.2f ) ),
	m_Refl( 0 ), m_Diff( 0.2f ), m_Spec( 0.8f ), 
	m_RIndex( 1.5f ), m_DRefl( 0 ), m_Texture( 0 ),
	m_UScale( 1.0f ), m_VScale( 1.0f )
{
}

void Material::SetUVScale( float a_UScale, float a_VScale )
{ 
	m_UScale = a_UScale; 
	m_VScale = a_VScale; 
	m_RUScale = 1.0f / a_UScale;
	m_RVScale = 1.0f / a_VScale;
}

// -----------------------------------------------------------
// Sphere primitive methods
// -----------------------------------------------------------

Sphere::Sphere( vector3& a_Centre, float a_Radius ) : 
	m_Centre( a_Centre ), m_SqRadius( a_Radius * a_Radius ), 
	m_Radius( a_Radius ), m_RRadius( 1.0f / a_Radius )
{
	m_Material = new Material();
	// set vectors for texture mapping
	m_Vn = vector3( 0, 1, 0 );
	m_Ve = vector3( 1, 0, 0 );
	m_Vc = m_Vn.Cross( m_Ve );
}

Sphere::~Sphere()
{
	delete m_Material;
}

int Sphere::Intersect( Ray& a_Ray, float& a_Dist )
{
	vector3 v = a_Ray.GetOrigin() - m_Centre;
	float b = -DOT( v, a_Ray.GetDirection() );
	float det = (b * b) - DOT( v, v ) + m_SqRadius;
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
				if (i2 < a_Dist) 
				{
					a_Dist = i2;
					retval = INPRIM;
				}
			}
			else
			{
				if (i1 < a_Dist)
				{
					a_Dist = i1;
					retval = HIT;
				}
			}
		}
	}
	return retval;
}

Color Sphere::GetColor( vector3& a_Pos )
{
	Color retval;
	if (!m_Material->GetTexture()) retval = m_Material->GetColor(); else
	{
		vector3 vp = (a_Pos - m_Centre) * m_RRadius;
		float phi = acosf( -DOT( vp, m_Vn ) );
		float u, v = phi * m_Material->GetVScaleReci() * (1.0f / PI);
		float theta = (acosf( DOT( m_Ve, vp ) / sinf( phi ))) * (2.0f / PI);
		if (DOT( m_Vc, vp ) >= 0) u = (1.0f - theta) * m_Material->GetUScaleReci();
						     else u = theta * m_Material->GetUScaleReci();
		retval = m_Material->GetTexture()->GetTexel( u, v ) * m_Material->GetColor();
	}
	return retval;
}

bool Sphere::IntersectBox( aabb& a_Box )
{
	float dmin = 0;
	vector3 v1 = a_Box.GetPos(), v2 = a_Box.GetPos() + a_Box.GetSize();
	if (m_Centre.x < v1.x) 
	{
		dmin = dmin + (m_Centre.x - v1.x) * (m_Centre.x - v1.x);
	}
	else if (m_Centre.x > v2.x)
	{
		dmin = dmin + (m_Centre.x - v2.x) * (m_Centre.x - v2.x);
	}
	if (m_Centre.y < v1.y)
	{
		dmin = dmin + (m_Centre.y - v1.y) * (m_Centre.y - v1.y);
	}
	else if (m_Centre.y > v2.y)
	{
		dmin = dmin + (m_Centre.y - v2.y) * (m_Centre.y - v2.y);
	}
	if (m_Centre.z < v1.z)
	{
		dmin = dmin + (m_Centre.z - v1.z) * (m_Centre.z - v1.z);
	}
	else if (m_Centre.z > v2.z)
	{
		dmin = dmin + (m_Centre.z - v2.z) * (m_Centre.z - v2.z);
	}
	return (dmin <= m_SqRadius);
}

aabb Sphere::GetAABB()
{ 
	vector3 size( m_Radius, m_Radius, m_Radius );
	return aabb( m_Centre - size, size * 2 );
}

// -----------------------------------------------------------
// Plane primitive class implementation
// -----------------------------------------------------------

PlanePrim::PlanePrim( vector3& a_Normal, float a_D ) : 
	m_Plane( plane( a_Normal, a_D ) )
{
	m_Material = new Material();
	m_UAxis = vector3( m_Plane.N.y, m_Plane.N.z, -m_Plane.N.x );
	m_VAxis = m_UAxis.Cross( m_Plane.N );
}

PlanePrim::~PlanePrim()
{
	delete m_Material;
}

int PlanePrim::Intersect( Ray& a_Ray, float& a_Dist )
{
	float d = DOT( m_Plane.N, a_Ray.GetDirection() );
	if (d < 0)
	{
		float dist = -(DOT( m_Plane.N, a_Ray.GetOrigin() ) + m_Plane.D) / d;
		if (dist < a_Dist) 
		{
			a_Dist = dist;
			return HIT;
		}
	}
	return MISS;
}

bool PlanePrim::IntersectBox( aabb& a_Box )
{
	vector3 v[2];
	v[0] = a_Box.GetPos(), v[1] = a_Box.GetPos() + a_Box.GetSize();
	int side1 = 0;
	int side2 = 0;
	for (int i = 0; i < 8; i++ )
	{
		vector3 p( v[i & 1].x, v[(i >> 1) & 1].y, v[(i >> 2) & 1].z );
		if ((DOT( p, m_Plane.N ) + m_Plane.D) < 0) side1++; else side2++;
	}
	if ((side1 == 0) || (side2 == 0)) return false; else return true;
}

Color PlanePrim::GetColor( vector3& a_Pos )
{
	Color retval;
	if (m_Material->GetTexture())
	{
		Texture* t = m_Material->GetTexture();
		float u = DOT( a_Pos, m_UAxis ) * m_Material->GetUScale();
		float v = DOT( a_Pos, m_VAxis ) * m_Material->GetVScale();
		retval = t->GetTexel( u, v ) * m_Material->GetColor();
	}
	else
	{
		retval = m_Material->GetColor();
	}
	return retval;
}

vector3 PlanePrim::GetNormal( vector3& a_Pos )
{
	return m_Plane.N;
}

// -----------------------------------------------------------
// Axis aligned box primitive class implementation
// -----------------------------------------------------------

Box::Box() : 
	m_Box( vector3( 0, 0, 0 ), vector3( 0, 0, 0 ) ), 
	m_Grid( 0 ) 
{
	m_Material = new Material();	
}

Box::Box( aabb& a_Box ) : 
	m_Box( a_Box ), 
	m_Grid( 0 ) 
{
	m_Material = new Material();	
}

Box::~Box()
{
	delete m_Material;
}

int Box::Intersect( Ray& a_Ray, float& a_Dist )
{
	m_RayID = a_Ray.GetID();
	float dist[6];
	vector3 ip[6], d = a_Ray.GetDirection(), o = a_Ray.GetOrigin();
	bool retval = MISS;
	for ( int i = 0; i < 6; i++ ) dist[i] = -1;
	vector3 v1 = m_Box.GetPos(), v2 = m_Box.GetPos() + GetSize();
	if (d.x) 
	{
		float rc = 1.0f / d.x;
		dist[0] = (v1.x - o.x) * rc;
		dist[3] = (v2.x - o.x) * rc;
	}
	if (d.y) 
	{
		float rc = 1.0f / d.y;
		dist[1] = (v1.y - o.y) * rc;
		dist[4] = (v2.y - o.y) * rc;
	}
	if (d.z) 
	{
		float rc = 1.0f / d.z;
		dist[2] = (v1.z - o.z) * rc;
		dist[5] = (v2.z - o.z) * rc;
	}
	for ( int i = 0; i < 6; i++ ) if (dist[i] > 0)
	{
		//ip[i] = o + dist[i] * d;  // Uses vector addition and multiplication 
		ip[i].x = o.x + dist[i] * d.x;
		ip[i].y = o.y + dist[i] * d.y;
		ip[i].z = o.z + dist[i] * d.z;
		if ((ip[i].x > (v1.x - EPSILON)) && (ip[i].x < (v2.x + EPSILON)) && 
			(ip[i].y > (v1.y - EPSILON)) && (ip[i].y < (v2.y + EPSILON)) &&
			(ip[i].z > (v1.z - EPSILON)) && (ip[i].z < (v2.z + EPSILON)))
		{
			if (dist[i] < a_Dist) 
			{
				a_Dist = dist[i];
				retval = HIT;
			}
		}
	}
	return retval;
}

vector3 Box::GetNormal( vector3& a_Pos )
{
	float dist[6];
	dist[0] = (float)fabs( m_Box.GetSize().x - m_Box.GetPos().x );
	dist[1] = (float)fabs( m_Box.GetSize().x + m_Box.GetSize().x - m_Box.GetPos().x );
	dist[2] = (float)fabs( m_Box.GetSize().y - m_Box.GetPos().y );
	dist[3] = (float)fabs( m_Box.GetSize().y + m_Box.GetSize().y - m_Box.GetPos().y );
	dist[4] = (float)fabs( m_Box.GetSize().z - m_Box.GetPos().z );
	dist[5] = (float)fabs( m_Box.GetSize().z + m_Box.GetSize().z - m_Box.GetPos().z );
	int best = 0;
	float bdist = dist[0];
	for ( int i = 1 ; i < 6; i++ ) if (dist[i] < bdist) 
	{ 
		bdist = dist[i]; 
		best = i;
	}
	if (best == 0) return vector3( -1, 0, 0 );
	else if (best == 1) return vector3( 1, 0, 0 );
	else if (best == 2) return vector3( 0, -1, 0 );
	else if (best == 3)  return vector3( 0, 1, 0 );
	else if (best == 4) return vector3( 0, 0, -1 );
	else return vector3( 0, 0, 1 );
}

void Box::Light( bool a_Light )
{
	m_Light = a_Light;
	if (!m_Grid)
	{
		m_Grid = new float[32];
		m_Grid[ 0] = 1, m_Grid[ 1] = 2;
		m_Grid[ 2] = 3, m_Grid[ 3] = 3;
		m_Grid[ 4] = 2, m_Grid[ 5] = 0;
		m_Grid[ 6] = 0, m_Grid[ 7] = 1;
		m_Grid[ 8] = 2, m_Grid[ 9] = 3;
		m_Grid[10] = 0, m_Grid[11] = 3;
		m_Grid[12] = 0, m_Grid[13] = 0;
		m_Grid[14] = 2, m_Grid[15] = 2;
		m_Grid[16] = 3, m_Grid[17] = 1;
		m_Grid[18] = 1, m_Grid[19] = 3;
		m_Grid[20] = 1, m_Grid[21] = 0;
		m_Grid[22] = 3, m_Grid[23] = 2;
		m_Grid[24] = 2, m_Grid[25] = 1;
		m_Grid[26] = 3, m_Grid[27] = 0;
		m_Grid[28] = 1, m_Grid[29] = 1;
		m_Grid[30] = 0, m_Grid[31] = 2;
		for ( int i = 0; i < 16; i++ )
		{
			m_Grid[i * 2] = m_Grid[i * 2] * m_Box.GetSize().x / 4 + m_Box.GetPos().x;
			m_Grid[i * 2 + 1] = m_Grid[i * 2 + 1] * m_Box.GetSize().z / 4 + m_Box.GetPos().z;
		}
	}
}

// -----------------------------------------------------------
// Scene class implementation
// -----------------------------------------------------------

Scene::~Scene()
{
	delete m_Primitive;
}

void Scene::InitScene()
{
	m_Primitive = new Primitive*[500];
	int pc = 0;

	// ground plane
	m_Primitive[pc] = new PlanePrim( vector3( 0, 1, 0 ), 4.4f );
	m_Primitive[pc]->SetName( "plane" );
	m_Primitive[pc]->GetMaterial()->SetReflection( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetDiffuse( 1.0f );
	m_Primitive[pc]->GetMaterial()->SetSpecular( 0.5f );
	m_Primitive[pc]->GetMaterial()->SetColor( Color( 0.7f, 0.7f, 0.7f ) );
	//m_Primitive[pc]->GetMaterial()->SetColor( Color( 0.4f, 0.3f, 0.3f ) );
	m_Primitive[pc]->GetMaterial()->SetTexture( new Texture( "textures/checker.tga" ) );
	//m_Primitive[pc]->GetMaterial()->SetUVScale( 0.05f, 0.05f );
	m_Primitive[pc]->GetMaterial()->SetUVScale( 0.45f, 0.45f );
	
	// big sphere
	pc++;
	//m_Primitive[pc] = new Sphere( vector3( 0, -0.8f, 7 ), 2 );
	m_Primitive[pc] = new Sphere( vector3( 0, -0.8f, 7 ), 2 );
	m_Primitive[pc]->SetName( "big sphere" );
	//m_Primitive[pc]->GetMaterial()->SetReflection( 0.2f );
	m_Primitive[pc]->GetMaterial()->SetReflection( 0.05f );
	//m_Primitive[pc]->GetMaterial()->SetRefraction( 0.8f );
	m_Primitive[pc]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetRefrIndex( 1.3f );
	m_Primitive[pc]->GetMaterial()->SetDiffuse( 0.5f );
	m_Primitive[pc]->GetMaterial()->SetSpecular( 0.8f );
	m_Primitive[pc]->GetMaterial()->SetColor( Color( 2.7f, 2.7f, 3.0f ) );
	//m_Primitive[pc]->GetMaterial()->SetColor( Color( 4.0f, 3.7f, 3.7f ) );
	//m_Primitive[pc]->GetMaterial()->SetDiffuseRefl( 0.3f );
	m_Primitive[1]->GetMaterial()->SetTexture( new Texture( "textures/marble.tga" ) );
	m_Primitive[1]->GetMaterial()->SetUVScale( 0.8f, 0.8f );

	// small sphere
	pc++;
	//m_Primitive[pc] = new Sphere( vector3( -5, -0.8f, 7 ), 2 );
	m_Primitive[pc] = new Sphere( vector3( -5, -0.8f, 7 ), 2 );
	m_Primitive[pc]->SetName( "small sphere" );
	m_Primitive[pc]->GetMaterial()->SetReflection( 0.5f );
	m_Primitive[pc]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetRefrIndex( 1.3f );
	m_Primitive[pc]->GetMaterial()->SetDiffuse( 0.1f );
	m_Primitive[pc]->GetMaterial()->SetColor( Color( 0.7f, 0.7f, 1.0f ) );
	m_Primitive[pc]->GetMaterial()->SetDiffuseRefl( 0.6f );
	
	// third sphere
	pc++;
	//m_Primitive[pc] = new Sphere( vector3( 5, -0.8f, 7 ), 2 );
	m_Primitive[pc] = new Sphere( vector3( 5, -0.8f, 7 ), 0 );
	m_Primitive[pc]->SetName( "small sphere" );
	m_Primitive[pc]->GetMaterial()->SetReflection( 0.5f );
	m_Primitive[pc]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetRefrIndex( 1.3f );
	m_Primitive[pc]->GetMaterial()->SetDiffuse( 0.1f );
	m_Primitive[pc]->GetMaterial()->SetColor( Color( 0.7f, 0.7f, 1.0f ) );
		
#if 1
	// area light 1
	Color lightColor = Color( 0.5f, 0.5f, 0.5f );
	float h = 5.0f;
	pc++;
	m_Primitive[pc] = new Box( aabb( vector3( -1, h, 4 ), vector3( 2, 0.1f, 2 ) ) );
	m_Primitive[pc]->Light( true );
	m_Primitive[pc]->GetMaterial()->SetColor( lightColor );

	// area light 2	
	pc++;
	m_Primitive[pc] = new Box( aabb( vector3( -6, h, 4 ), vector3( 2, 0.1f, 2 ) ) );
	m_Primitive[pc]->Light( true );
	m_Primitive[pc]->GetMaterial()->SetColor( lightColor );

	// area light 3	
	pc++;
	m_Primitive[pc] = new Box( aabb( vector3( 4, h, 4 ), vector3( 2, 0.1f, 2 ) ) );
	m_Primitive[pc]->Light( true );
	m_Primitive[pc]->GetMaterial()->SetColor( lightColor );

#else
	// light source 1
	m_Primitive[pc] = new Sphere( vector3( 0, 5, 5 ), 0.1f );
	m_Primitive[pc]->Light( true );
	m_Primitive[pc]->GetMaterial()->SetColor( Color( 1, 1, 1 ) );

#endif

	// back plane	
	pc++;
	m_Primitive[pc] = new PlanePrim( vector3( 0, 0, -1 ), 10.4f );
	m_Primitive[pc]->SetName( "backplane" );
	m_Primitive[pc]->GetMaterial()->SetReflection( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetDiffuse( 1.7f );
	//m_Primitive[pc]->GetMaterial()->SetColor( Color( 0.4f, 0.3f, 0.3f ) );
	m_Primitive[pc]->GetMaterial()->SetColor( Color( 0.4f, 0.3f, 0.3f ) );

	// Box
	pc++;
	m_Primitive[pc] = new Box( aabb( vector3( 4, -2, 0.5f ), vector3( 0.25f, 3.5f, 12 ) ) );
	m_Primitive[pc]->GetMaterial()->SetReflection( 0.2f );
	m_Primitive[pc]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetDiffuse( 0.1f );
	m_Primitive[pc]->GetMaterial()->SetColor( Color( 2.7f, 2.7f, 3.0f ) );

	// Box ceiling
	pc++;
	m_Primitive[pc] = new Box( aabb( vector3( -15.0f, h+0.1f, 2.5f ), vector3( 30.0f, 0.25f, 30.0f ) ) );
	m_Primitive[pc]->SetName( "Box ceiling" );
	m_Primitive[pc]->GetMaterial()->SetReflection( 0.00f );
	m_Primitive[pc]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetDiffuse( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetColor( Color( 12.7f, 12.7f, 13.0f ) );

	// ceiling
	pc++;
	m_Primitive[pc] = new PlanePrim( vector3( 0, 1, 0 ), 5.0f );
	m_Primitive[pc]->SetName( "ceiling" );
	m_Primitive[pc]->GetMaterial()->SetReflection( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetRefraction( 0.5f );
	m_Primitive[pc]->GetMaterial()->SetDiffuse( 0.0f );
	m_Primitive[pc]->GetMaterial()->SetColor( Color( 0.4f, 0.3f, 0.3f ) );

	m_Primitives = pc+1;
	// build the regular gird
	BuildGrid();
}

void Scene::BuildGrid()
{
	// initialize regular grid
	m_Grid = new ObjectList*[GRIDSIZE * GRIDSIZE * GRIDSIZE];
	memset( m_Grid, 0, GRIDSIZE * GRIDSIZE * GRIDSIZE * 4 );
	vector3 p1(-14, -5, -6), p2( 14, 8, 30 );
	// calculate cell width, height and depth
	float dx = (p2.x - p1.x) / GRIDSIZE, dx_reci = 1.0f / dx;
	float dy = (p2.y - p1.y) / GRIDSIZE, dy_reci = 1.0f / dy;
	float dz = (p2.z - p1.z) / GRIDSIZE, dz_reci = 1.0f / dz;
	m_Extends = aabb( p1, p2 - p1 );
	m_Light = new Primitive*[MAXLIGHTS];
	m_Lights = 0;
	// store primitives in the grid cells
	for ( int p = 0; p < m_Primitives; p++ )
	{
		if (m_Primitive[p]->IsLight()) m_Light[m_Lights++] = m_Primitive[p];
		aabb bound = m_Primitive[p]->GetAABB();
		vector3 bv1 = bound.GetPos(), bv2 = bound.GetPos() + bound.GetSize();
		// find out which cells could contain the primitive (based on aabb)
		int x1 = (int)((bv1.x - p1.x) * dx_reci), x2 = (int)((bv2.x - p1.x) * dx_reci) + 1;
		x1 = (x1 < 0)?0:x1, x2 = (x2 > (GRIDSIZE - 1))?GRIDSIZE - 1:x2;
		int y1 = (int)((bv1.y - p1.y) * dy_reci), y2 = (int)((bv2.y - p1.y) * dy_reci) + 1;
		y1 = (y1 < 0)?0:y1, y2 = (y2 > (GRIDSIZE - 1))?GRIDSIZE - 1:y2;
		int z1 = (int)((bv1.z - p1.z) * dz_reci), z2 = (int)((bv2.z - p1.z) * dz_reci) + 1;
		z1 = (z1 < 0)?0:z1, z2 = (z2 > (GRIDSIZE - 1))?GRIDSIZE - 1:z2;
		// loop over candidate cells
		for ( int x = x1; x < x2; x++ ) for ( int y = y1; y < y2; y++ ) for ( int z = z1; z < z2; z++ )
		{
			// construct aabb for current cell
			int idx = x + y * GRIDSIZE + z * GRIDSIZE * GRIDSIZE;
			vector3 pos( p1.x + x * dx, p1.y + y * dy, p1.z + z * dz );
			aabb cell( pos, vector3( dx, dy, dz ) );
			// do an accurate aabb / primitive intersection test
			if (m_Primitive[p]->IntersectBox( cell ))
			{
				// object intersects cell; add to object list
				ObjectList* l = new ObjectList();
				l->SetPrimitive( m_Primitive[p] );
				l->SetNext( m_Grid[idx] );
				m_Grid[idx] = l;
			}
		}
	}
}

}; // namespace Raytracer
