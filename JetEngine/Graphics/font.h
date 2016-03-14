#ifndef FONT_H
#define FONT_H

#ifdef ANDROID
#include <GLES2/gl2.h>
#endif
#include "string.h"
#include "../defines.h"
#include "CIndexBuffer.h"
#include "CVertexBuffer.h"

class CTexture;

struct Glyph
{
	float u, v, umin, vmin;
};

#undef DrawText
class Font
{
	CVertexBuffer buffer;
	CIndexBuffer ib;

	int spacing[256][8];
public:
	int height;

	int fonttexid;
	unsigned int shader;

	Glyph glyphs[256];

	CTexture* texture;

#ifndef _WIN32
	char* charactersL;
	char* charactersU;
	char* numbers;
	char* symbols;

	Font()
	{
		charactersL = "abcdefghijklmnopqrstuvwxyz";
		charactersU = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		numbers = "1234567890";
		symbols = "!@£$%^&*()-+='.,:;";
};
#endif

	int posAttrib;
	int texUnif;
	int colorUnif;

	unsigned int vb;
	bool Load(int texid, int texw, int texh, int charw, int charh);

	int TextSize(const char* txt, int size = 0);

	struct Vert
	{
		float x, y;
		float u, v;
	};
	void DrawText(const char* txt, float x, float y, float sx, float sy, unsigned int color);
};
#endif
