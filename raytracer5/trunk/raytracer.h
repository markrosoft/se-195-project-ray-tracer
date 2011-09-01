// -----------------------------------------------------------
// raytracer.h
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#ifndef I_RAYTRACER_H
#define I_RAYTRACER_H

#include "common.h"
#include "twister.h"

namespace Raytracer {

// -----------------------------------------------------------
// Ray class definition
// -----------------------------------------------------------
class Ray
{
public:
	Ray() : m_Origin( vector3( 0, 0, 0 ) ), m_Direction( vector3( 0, 0, 0 ) ), m_ID( 0 ) {};
	Ray( vector3& a_Origin, vector3& a_Dir, int a_ID );
	void SetOrigin( vector3& a_Origin ) { m_Origin = a_Origin; }
	void SetDirection( vector3& a_Direction ) { m_Direction = a_Direction; }
	vector3& GetOrigin() { return m_Origin; }
	vector3& GetDirection() { return m_Direction; }
	void SetID( int a_ID ) { m_ID = a_ID; }
	int GetID() { return m_ID; }
private:
	vector3 m_Origin;
	vector3 m_Direction;
	int m_ID;
};

// -----------------------------------------------------------
// Engine class definition
// Raytracer core
// -----------------------------------------------------------
class Scene;
class Primitive;
class Engine
{
public:
	Engine();
	~Engine();
	void SetTarget( Pixel* a_Dest, int a_Width, int a_Height );
	Scene* GetScene() { return m_Scene; }
	int FindNearest( Ray& a_Ray, float& a_Dist, Primitive*& a_Prim );
	Primitive* Raytrace( Ray& a_Ray, Color& a_Acc, int a_Depth, float a_RIndex, float& a_Dist, float a_Samples, float a_SScale );
	float CalcShade( Primitive* a_Light, vector3 a_IP, vector3& a_Dir, float a_Samples, float a_SScale );
	void InitRender( vector3& a_Pos, vector3& a_Target );
	Primitive* RenderRay( vector3 a_ScreenPos, Color& a_Acc );
	bool Render();
protected:
	// renderer data
	Scene* m_Scene;
	Pixel* m_Dest;
	int m_Width, m_Height, m_CurrLine, m_PPos;
	Primitive** m_LastRow;
	Twister m_Twister;
	vector3 m_Origin, m_P1, m_P2, m_P3, m_P4, m_DX, m_DY;
	// data for regular grid stepping
	vector3 m_SR, m_CW;
	int m_CurID;
};

}; // namespace Raytracer

#endif
