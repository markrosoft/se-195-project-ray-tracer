#define _CRT_SECURE_NO_WARNINGS 1

#include "common.h"
#include "surface.h"
#include "stdio.h"
#include "string.h"


// -----------------------------------------------------------
// Hicolor surface class implementation
// -----------------------------------------------------------

Surface * Surface_Create( int a_Width, int a_Height ) {
	Surface *s = (Surface *)malloc(sizeof(Surface));
	s->m_Width = a_Width;
	s->m_Height = a_Height;
	s->m_Buffer = (Pixel *)malloc(a_Width*a_Height*sizeof(Pixel));
	return s;
}

void Surface_Clear( Surface * s, Pixel a_Color )
{
	int z = s->m_Width * s->m_Height;
	for ( int i = 0; i < z; i++ ) s->m_Buffer[i] = a_Color;
}

void Surface_Print( Surface * s, char* a_String, int x1, int y1, Pixel color )
{
	Pixel* t = s->m_Buffer + x1 + y1 * s->m_Width;
	int i;
	for ( i = 0; i < (int)(strlen( a_String )); i++ )
	{	
		long pos = 0;
		if ((a_String[i] >= 'A') && (a_String[i] <= 'Z')) pos = s->s_Transl[a_String[i] - ('A' - 'a')];
		else pos = s->s_Transl[a_String[i]];

		Pixel* a = t;
		char* c = (char*)s->s_Font[pos];
		int h, v;
		for ( v = 0; v < 5; v++ ) 
		{
			for ( h = 0; h < 5; h++ ) if (*c++ == 'o') *(a + h) = color;
			a += s->m_Width;
		}
		t += 6;
	}
}

void Surface_SetChar( Surface * s, int c, char* c1, char* c2, char* c3, char* c4, char* c5 )
{
	strcpy( s->s_Font[c][0], c1 );
	strcpy( s->s_Font[c][1], c2 );
	strcpy( s->s_Font[c][2], c3 );
	strcpy( s->s_Font[c][3], c4 );
	strcpy( s->s_Font[c][4], c5 );
}

void Surface_InitCharset(Surface * s)
{
	Surface_SetChar(s,  0, ":ooo:", "o:::o", "ooooo", "o:::o", "o:::o" );
	Surface_SetChar(s,  1, "oooo:", "o:::o", "oooo:", "o:::o", "oooo:" );
	Surface_SetChar(s,  2, ":oooo", "o::::", "o::::", "o::::", ":oooo" );
	Surface_SetChar(s,  3, "oooo:", "o:::o", "o:::o", "o:::o", "oooo:" );
	Surface_SetChar(s,  4, "ooooo", "o::::", "oooo:", "o::::", "ooooo" );
	Surface_SetChar(s,  5, "ooooo", "o::::", "ooo::", "o::::", "o::::" );
	Surface_SetChar(s,  6, ":oooo", "o::::", "o:ooo", "o:::o", ":ooo:" );
	Surface_SetChar(s,  7, "o:::o", "o:::o", "ooooo", "o:::o", "o:::o" );
	Surface_SetChar(s,  8, "::o::", "::o::", "::o::", "::o::", "::o::" );
	Surface_SetChar(s,  9, ":::o:", ":::o:", ":::o:", ":::o:", "ooo::" );
	Surface_SetChar(s, 10, "o::o:", "o:o::", "oo:::", "o:o::", "o::o:" );
	Surface_SetChar(s, 11, "o::::", "o::::", "o::::", "o::::", "ooooo" );
	Surface_SetChar(s, 12, "oo:o:", "o:o:o", "o:o:o", "o:::o", "o:::o" );
	Surface_SetChar(s, 13, "o:::o", "oo::o", "o:o:o", "o::oo", "o:::o" );
	Surface_SetChar(s, 14, ":ooo:", "o:::o", "o:::o", "o:::o", ":ooo:" );
	Surface_SetChar(s, 15, "oooo:", "o:::o", "oooo:", "o::::", "o::::" );
	Surface_SetChar(s, 16, ":ooo:", "o:::o", "o:::o", "o::oo", ":oooo" );
	Surface_SetChar(s, 17, "oooo:", "o:::o", "oooo:", "o:o::", "o::o:" );
	Surface_SetChar(s, 18, ":oooo", "o::::", ":ooo:", "::::o", "oooo:" );
	Surface_SetChar(s, 19, "ooooo", "::o::", "::o::", "::o::", "::o::" );
	Surface_SetChar(s, 20, "o:::o", "o:::o", "o:::o", "o:::o", ":oooo" );
	Surface_SetChar(s, 21, "o:::o", "o:::o", ":o:o:", ":o:o:", "::o::" );
	Surface_SetChar(s, 22, "o:::o", "o:::o", "o:o:o", "o:o:o", ":o:o:" );
	Surface_SetChar(s, 23, "o:::o", ":o:o:", "::o::", ":o:o:", "o:::o" );
	Surface_SetChar(s, 24, "o:::o", "o:::o", ":oooo", "::::o", ":ooo:" );
	Surface_SetChar(s, 25, "ooooo", ":::o:", "::o::", ":o:::", "ooooo" );
	Surface_SetChar(s, 26, ":ooo:", "o::oo", "o:o:o", "oo::o", ":ooo:" );
	Surface_SetChar(s, 27, "::o::", ":oo::", "::o::", "::o::", ":ooo:" );
	Surface_SetChar(s, 28, ":ooo:", "o:::o", "::oo:", ":o:::", "ooooo" );
	Surface_SetChar(s, 29, "oooo:", "::::o", "::oo:", "::::o", "oooo:" );
	Surface_SetChar(s, 30, "o::::", "o::o:", "ooooo", ":::o:", ":::o:" );
	Surface_SetChar(s, 31, "ooooo", "o::::", "oooo:", "::::o", "oooo:" );
	Surface_SetChar(s, 32, ":oooo", "o::::", "oooo:", "o:::o", ":ooo:" );
	Surface_SetChar(s, 33, "ooooo", "::::o", ":::o:", "::o::", "::o::" );
	Surface_SetChar(s, 34, ":ooo:", "o:::o", ":ooo:", "o:::o", ":ooo:" );
	Surface_SetChar(s, 35, ":ooo:", "o:::o", ":oooo", "::::o", ":ooo:" );
	Surface_SetChar(s, 36, "::o::", "::o::", "::o::", ":::::", "::o::" );
	Surface_SetChar(s, 37, ":ooo:", "::::o", ":::o:", ":::::", "::o::" );
	Surface_SetChar(s, 38, ":::::", ":::::", "::o::", ":::::", "::o::" );
	Surface_SetChar(s, 39, ":::::", ":::::", ":ooo:", ":::::", ":ooo:" );
	Surface_SetChar(s, 40, ":::::", ":::::", ":::::", ":::o:", "::o::" );
	Surface_SetChar(s, 41, ":::::", ":::::", ":::::", ":::::", "::o::" );
	Surface_SetChar(s, 42, ":::::", ":::::", ":ooo:", ":::::", ":::::" );
	Surface_SetChar(s, 43, ":::o:", "::o::", "::o::", "::o::", ":::o:" );
	Surface_SetChar(s, 44, "::o::", ":::o:", ":::o:", ":::o:", "::o::" );
	Surface_SetChar(s, 45, ":::::", ":::::", ":::::", ":::::", ":::::" );
	Surface_SetChar(s, 46, "ooooo", "ooooo", "ooooo", "ooooo", "ooooo" );
	Surface_SetChar(s, 47, "::o::", "::o::", ":::::", ":::::", ":::::" ); // Tnx Ferry
	Surface_SetChar(s, 48, "o:o:o", ":ooo:", "ooooo", ":ooo:", "o:o:o" );
	Surface_SetChar(s, 49, "::::o", ":::o:", "::o::", ":o:::", "o::::" );
	char c[] = "abcdefghijklmnopqrstuvwxyz0123456789!?:=,.-() #'*/";
	int i;
	for ( i = 0; i < 256; i++ ) s->s_Transl[i] = 45;
	for ( i = 0; i < 50; i++ ) s->s_Transl[(unsigned char)c[i]] = i;
}
