// -----------------------------------------------------------
// common.h
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#ifndef I_COMMON_H
#define I_COMMON_H

#include "math.h"
#include "stdlib.h"

typedef unsigned int Pixel;

inline float Rand( float a_Range ) { return ((float)rand() / RAND_MAX) * a_Range; }

namespace Raytracer {

#define DOT(A,B)		(A.x*B.x+A.y*B.y+A.z*B.z)
#define NORMALIZE(A)	{float l=1/sqrtf(A.x*A.x+A.y*A.y+A.z*A.z);A.x*=l;A.y*=l;A.z*=l;}
#define LENGTH(A)		(sqrtf(A.x*A.x+A.y*A.y+A.z*A.z))
#define SQRLENGTH(A)	(A.x*A.x+A.y*A.y+A.z*A.z)
#define SQRDISTANCE(A,B) ((A.x-B.x)*(A.x-B.x)+(A.y-B.y)*(A.y-B.y)+(A.z-B.z)*(A.z-B.z))

#define SAMPLES			1
#define GRIDSIZE		8
#define GRIDSHFT		3
#define EPSILON			0.001f
#define TRACEDEPTH		4
#define IMPORTANCE

#define PI				3.141592653589793238462f

class vector3
{
public:
	vector3() : x( 0.0f ), y( 0.0f ), z( 0.0f ) {};
	vector3( float a_X, float a_Y, float a_Z ) : x( a_X ), y( a_Y ), z( a_Z ) {};
	void Set( float a_X, float a_Y, float a_Z ) { x = a_X; y = a_Y; z = a_Z; }
	void Normalize() { float l = 1.0f / Length(); x *= l; y *= l; z *= l; }
	float Length() { return (float)sqrt( x * x + y * y + z * z ); }
	float SqrLength() { return x * x + y * y + z * z; }
	float Dot( vector3 a_V ) { return x * a_V.x + y * a_V.y + z * a_V.z; }
	vector3 Cross( vector3 b ) { return vector3( y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x ); }
	void operator += ( vector3& a_V ) { x += a_V.x; y += a_V.y; z += a_V.z; }
	void operator += ( vector3* a_V ) { x += a_V->x; y += a_V->y; z += a_V->z; }
	void operator -= ( vector3& a_V ) { x -= a_V.x; y -= a_V.y; z -= a_V.z; }
	void operator -= ( vector3* a_V ) { x -= a_V->x; y -= a_V->y; z -= a_V->z; }
	void operator *= ( float f ) { x *= f; y *= f; z *= f; }
	void operator *= ( vector3& a_V ) { x *= a_V.x; y *= a_V.y; z *= a_V.z; }
	void operator *= ( vector3* a_V ) { x *= a_V->x; y *= a_V->y; z *= a_V->z; }
	vector3 operator- () const { return vector3( -x, -y, -z ); }
	friend vector3 operator + ( const vector3& v1, const vector3& v2 ) { return vector3( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z ); }
	friend vector3 operator - ( const vector3& v1, const vector3& v2 ) { return vector3( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z ); }
	friend vector3 operator + ( const vector3& v1, vector3* v2 ) { return vector3( v1.x + v2->x, v1.y + v2->y, v1.z + v2->z ); }
	friend vector3 operator - ( const vector3& v1, vector3* v2 ) { return vector3( v1.x - v2->x, v1.y - v2->y, v1.z - v2->z ); }
	friend vector3 operator * ( const vector3& v, float f ) { return vector3( v.x * f, v.y * f, v.z * f ); }
	friend vector3 operator * ( const vector3& v1, vector3& v2 ) { return vector3( v1.x * v2.x, v1.y * v2.y, v1.z * v2.z ); }
	friend vector3 operator * ( float f, const vector3& v ) { return vector3( v.x * f, v.y * f, v.z * f ); }
	union
	{
		struct { float x, y, z; };
		struct { float r, g, b; };
		struct { float cell[3]; };
	};
};

class matrix
{
public:
	enum 
	{ 
		TX=3, 
		TY=7, 
		TZ=11, 
		D0=0, D1=5, D2=10, D3=15, 
		SX=D0, SY=D1, SZ=D2, 
		W=D3 
	};
	matrix() { Identity(); }
	void Identity()
	{
		cell[1] = cell[2] = cell[TX] = cell[4] = cell[6] = cell[TY] =
		cell[8] = cell[9] = cell[TZ] = cell[12] = cell[13] = cell[14] = 0;
		cell[D0] = cell[D1] = cell[D2] = cell[W] = 1;
	}
	void Rotate( vector3 a_Pos, float a_RX, float a_RY, float a_RZ )
	{
		matrix t;
		t.RotateX( a_RZ );
		RotateY( a_RY );
		Concatenate( t );
		t.RotateZ( a_RX );
		Concatenate( t );
		Translate( a_Pos );
	}
	void RotateX( float a_RX )
	{
		float sx = (float)sin( a_RX * PI / 180 );
		float cx = (float)cos( a_RX * PI / 180 );
		Identity();
		cell[5] = cx, cell[6] = sx, cell[9] = -sx, cell[10] = cx;
	}
	void RotateY( float a_RY )
	{
		float sy = (float)sin( a_RY * PI / 180 );
		float cy = (float)cos( a_RY * PI / 180 );
		Identity ();
		cell[0] = cy, cell[2] = -sy, cell[8] = sy, cell[10] = cy;
	}
	void RotateZ( float a_RZ )
	{
		float sz = (float)sin( a_RZ * PI / 180 );
		float cz = (float)cos( a_RZ * PI / 180 );
		Identity ();
		cell[0] = cz, cell[1] = sz, cell[4] = -sz, cell[5] = cz;
	}
	void Translate( vector3 a_Pos ) { cell[TX] += a_Pos.x; cell[TY] += a_Pos.y; cell[TZ] += a_Pos.z; }
	void Concatenate( matrix& m2 )
	{
		matrix res;
		for ( int c = 0; c < 4; c++ ) for ( int r = 0; r < 4; r++ )
			res.cell[r * 4 + c] = cell[r * 4] * m2.cell[c] +
				  				  cell[r * 4 + 1] * m2.cell[c + 4] +
								  cell[r * 4 + 2] * m2.cell[c + 8] +
								  cell[r * 4 + 3] * m2.cell[c + 12];
		for ( int c = 0; c < 16; c++ ) cell[c] = res.cell[c];
	}
	vector3 Transform( vector3& v )
	{
		float x  = cell[0] * v.x + cell[1] * v.y + cell[2] * v.z + cell[3];
		float y  = cell[4] * v.x + cell[5] * v.y + cell[6] * v.z + cell[7];
		float z  = cell[8] * v.x + cell[9] * v.y + cell[10] * v.z + cell[11];
		return vector3( x, y, z );
	}
	void Invert()
	{
		matrix t;
		float tx = -cell[3], ty = -cell[7], tz = -cell[11];
		for ( int h = 0; h < 3; h++ ) for ( int v = 0; v < 3; v++ ) t.cell[h + v * 4] = cell[v + h * 4];
		for ( int i = 0; i < 11; i++ ) cell[i] = t.cell[i];
		cell[3] = tx * cell[0] + ty * cell[1] + tz * cell[2];
		cell[7] = tx * cell[4] + ty * cell[5] + tz * cell[6];
		cell[11] = tx * cell[8] + ty * cell[9] + tz * cell[10];
	}
	float cell[16];
};

class plane
{
public:
	plane() : N( 0, 0, 0 ), D( 0 ) {};
	plane( vector3 a_Normal, float a_D ) : N( a_Normal ), D( a_D ) {};
	union
	{
		struct
		{
			vector3 N;
			float D;
		};
		float cell[4];
	};
};

class aabb
{
public:
	aabb() : m_Pos( vector3( 0, 0, 0 ) ), m_Size( vector3( 0, 0, 0 ) ) {};
	aabb( vector3& a_Pos, vector3& a_Size ) : m_Pos( a_Pos ), m_Size( a_Size ) {};
	vector3& GetPos() { return m_Pos; }
	vector3& GetSize() { return m_Size; }
	bool Intersect( aabb& b2 )
	{
		vector3 v1 = b2.GetPos(), v2 = b2.GetPos() + b2.GetSize();
		vector3 v3 = m_Pos, v4 = m_Pos + m_Size;
		return ((v4.x > v1.x) && (v3.x < v2.x) && // x-axis overlap
				(v4.y > v1.y) && (v3.y < v2.y) && // y-axis overlap
				(v4.z > v1.z) && (v3.z < v2.z));   // z-axis overlap
	}
	bool Contains( vector3 a_Pos )
	{
		vector3 v1 = m_Pos, v2 = m_Pos + m_Size;
		return ((a_Pos.x > (v1.x - EPSILON)) && (a_Pos.x < (v2.x + EPSILON)) &&
				(a_Pos.y > (v1.y - EPSILON)) && (a_Pos.y < (v2.y + EPSILON)) &&
				(a_Pos.z > (v1.z - EPSILON)) && (a_Pos.z < (v2.z + EPSILON)));
	}
private:
	vector3 m_Pos, m_Size;
};

typedef vector3 Color;

}; // namespace Raytracer

#endif