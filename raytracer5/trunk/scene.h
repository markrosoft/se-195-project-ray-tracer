// -----------------------------------------------------------
// scene.h
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#ifndef I_SCENE_H
#define I_SCENE_H

#include "raytracer.h"

namespace Raytracer {

// Intersection method return values
#define HIT		 1		// Ray hit primitive
#define MISS	 0		// Ray missed primitive
#define INPRIM	-1		// Ray started inside primitive

#define MAXLIGHTS	10

// -----------------------------------------------------------
// Texture class definition
// -----------------------------------------------------------

class Texture
{
public:
	Texture( Color* a_Bitmap, int a_Width, int a_Height );
	Texture( char* a_File );
	Color* GetBitmap() { return m_Bitmap; }
	Color GetTexel( float a_U, float a_V );
	int GetWidth() { return m_Width; }
	int GetHeight() { return m_Height; }
private:
	Color* m_Bitmap;
	int m_Width, m_Height;
};

// -----------------------------------------------------------
// Material class definition
// -----------------------------------------------------------

class Material
{
public:
	Material();
	void SetColor( Color& a_Color ) { m_Color = a_Color; }
	Color GetColor() { return m_Color; }
	void SetDiffuse( float a_Diff ) { m_Diff = a_Diff; }
	void SetSpecular( float a_Spec ) { m_Spec = a_Spec; }
	void SetReflection( float a_Refl ) { m_Refl = a_Refl; }
	void SetRefraction( float a_Refr ) { m_Refr = a_Refr; }
	float GetSpecular() { return m_Spec; }
	float GetDiffuse() { return m_Diff; }
	float GetReflection() { return m_Refl; }
	float GetRefraction() { return m_Refr; }
	void SetRefrIndex( float a_Refr ) { m_RIndex = a_Refr; }
	float GetRefrIndex() { return m_RIndex; }
	void SetDiffuseRefl( float a_DRefl ) { m_DRefl = a_DRefl; }
	float GetDiffuseRefl() { return m_DRefl; }
	void SetTexture( Texture* a_Texture ) { m_Texture = a_Texture; }
	Texture* GetTexture() { return m_Texture; }
	void SetUVScale( float a_UScale, float a_VScale );
	float GetUScale() { return m_UScale; }
	float GetVScale() { return m_VScale; }
	float GetUScaleReci() { return m_RUScale; }
	float GetVScaleReci() { return m_RVScale; }
private:
	Color m_Color;
	float m_Refl, m_Refr;
	float m_Diff, m_Spec;
	float m_DRefl;
	float m_RIndex;
	Texture* m_Texture;
	float m_UScale, m_VScale, m_RUScale, m_RVScale;
};

// -----------------------------------------------------------
// Primitive class definition
// -----------------------------------------------------------

class Primitive
{
public:
	enum
	{
		SPHERE = 1,
		PLANE,
		AABB,
		TRIANGLE
	};
	Primitive() : m_Name( 0 ), m_Light( false ), m_RayID( -1 ) {};
	Material* GetMaterial() { return m_Material; }
	void SetMaterial( Material* a_Mat ) { m_Material = a_Mat; }
	virtual int GetType() = 0;
	virtual int Intersect( Ray& a_Ray, float& a_Dist ) = 0;
	virtual bool IntersectBox( aabb& a_Box ) = 0;
	virtual vector3 GetNormal( vector3& a_Pos ) = 0;
	virtual Color GetColor( vector3& ) { return m_Material->GetColor(); }
	virtual void Light( bool a_Light ) { m_Light = a_Light; }
	virtual aabb GetAABB() = 0;
	bool IsLight() { return m_Light; }
	int GetLastRayID() { return m_RayID; }
	void SetName( char* a_Name );
	char* GetName() { return m_Name; }
protected:
	Material* m_Material;
	char* m_Name;
	bool m_Light;
	int m_RayID;
};

// -----------------------------------------------------------
// Sphere primitive class definition
// -----------------------------------------------------------

class Sphere : public Primitive
{
public:
	int GetType() { return SPHERE; }
	Sphere( vector3& a_Centre, float a_Radius );
	~Sphere();
	vector3& GetCentre() { return m_Centre; }
	float GetSqRadius() { return m_SqRadius; }
	int Intersect( Ray& a_Ray, float& a_Dist );
	bool IntersectBox( aabb& a_Box );
	vector3 GetNormal( vector3& a_Pos ) { return (a_Pos - m_Centre) * m_RRadius; }
	Color GetColor( vector3& a_Pos );
	aabb GetAABB();
private:
	vector3 m_Centre;
	float m_SqRadius, m_Radius, m_RRadius;
	vector3 m_Ve, m_Vn, m_Vc;
};

// -----------------------------------------------------------
// PlanePrim primitive class definition
// -----------------------------------------------------------

class PlanePrim : public Primitive
{
public:
	int GetType() { return PLANE; }
	PlanePrim( vector3& a_Normal, float a_D );
	~PlanePrim();
	vector3& GetNormal() { return m_Plane.N; }
	float GetD() { return m_Plane.D; }
	int Intersect( Ray& a_Ray, float& a_Dist );
	bool IntersectBox( aabb& a_Box );
	vector3 GetNormal( vector3& a_Pos );
	Color GetColor( vector3& a_Pos );
	aabb GetAABB() { return aabb( vector3( -10000, -10000, -10000 ), vector3( 20000, 20000, 20000 ) ); }
private:
	plane m_Plane;
	vector3 m_UAxis, m_VAxis;
};

// -----------------------------------------------------------
// Box primitive class definition
// -----------------------------------------------------------

class Box : public Primitive
{
public:
	int GetType() { return AABB; }
	Box();
	Box( aabb& a_Box );
	~Box();
	int Intersect( Ray& a_Ray, float& a_Dist );
	bool IntersectBox( aabb& a_Box ) { return m_Box.Intersect( a_Box ); }
	vector3 GetNormal( vector3& );
	bool Contains( vector3& a_Pos ) { return m_Box.Contains( a_Pos ); }
	vector3& GetPos() { return m_Box.GetPos(); }
	vector3& GetSize() { return m_Box.GetSize(); }
	float GetGridX( int a_Idx ) { return m_Grid[a_Idx << 1]; }
	float GetGridY( int a_Idx ) { return m_Grid[(a_Idx << 1) + 1]; }
	void Light( bool a_Light );
	aabb GetAABB() { return m_Box; }
protected:
	aabb m_Box;
	float* m_Grid;
};

// -----------------------------------------------------------
// Object list helper class
// -----------------------------------------------------------

class ObjectList
{
public:
	ObjectList() : m_Primitive( 0 ), m_Next( 0 ) {}
	~ObjectList() { delete m_Next; }
	void SetPrimitive( Primitive* a_Prim ) { m_Primitive = a_Prim; }
	Primitive* GetPrimitive() { return m_Primitive; }
	void SetNext( ObjectList* a_Next ) { m_Next = a_Next; }
	ObjectList* GetNext() { return m_Next; }
private:
	Primitive* m_Primitive;
	ObjectList* m_Next;
};

// -----------------------------------------------------------
// Scene class definition
// -----------------------------------------------------------

class Scene
{
public:
	Scene() : m_Primitives( 0 ), m_Primitive( 0 ), m_Extends( vector3( 0, 0, 0 ), vector3( 0, 0, 0 ) ) {};
	~Scene();
	void InitScene();
	void BuildGrid();
	ObjectList** GetGrid() { return m_Grid; }
	int GetNrPrimitives() { return m_Primitives; }
	Primitive* GetPrimitive( int a_Idx ) { return m_Primitive[a_Idx]; }
	int GetNrLights() { return m_Lights; }
	Primitive* GetLight( int a_Idx ) { return m_Light[a_Idx]; }
	aabb& GetExtends() { return m_Extends; }
private:
	int m_Primitives, m_Lights;
	Primitive** m_Primitive;
	Primitive** m_Light;
	ObjectList** m_Grid;
	aabb m_Extends;
};

}; // namespace Raytracer

#endif