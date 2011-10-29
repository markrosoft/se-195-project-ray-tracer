#ifndef I_SURFACE_H
#define I_SURFACE_H

#include <math.h>
#include "string.h"
#include "common.h"

struct Surface{
	// Attributes
	Pixel * m_Buffer;
	int m_Width;
	int m_Height;
	
	// Static attributes for the buildin font
	char s_Font[51][5][5];	
	int s_Transl[256];		
};

Surface * Surface_Create(int a_Width, int a_Height );

void Surface_SetFromFile(Surface * s, char* a_File );

// member data access
inline Pixel* Surface_GetBuffer(Surface * s) { return s->m_Buffer; }
inline int Surface_GetWidth(Surface * s) { return s->m_Width; }
inline int Surface_GetHeight(Surface * s) { return s->m_Height; }

// Special operations
void Surface_InitCharset(Surface * s);
void Surface_SetChar( Surface * s, int c, char* c1, char* c2, char* c3, char* c4, char* c5 );
void Surface_Print( Surface * s, char* a_String, int x1, int y1, Pixel color );
void Surface_Clear( Surface * s, Pixel a_Color );


#endif