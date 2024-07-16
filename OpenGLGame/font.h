#if !defined( FONT_H )
#define FONT_H

#include "common.h"
#include "pixel_buffer.h"
#include "platform.h"

typedef struct
{
   PixelBuffer_t pixelBuffer;
   uint32_t baseline;
}
FontGlyph_t;

typedef struct
{
   uint32_t codepointOffset;
   uint32_t numGlyphs;
   FontGlyph_t* glyphs;
   GLuint textureHandle;
}
Font_t;

Bool_t Font_LoadFromFile( Font_t* font, const char* filePath );

#endif
