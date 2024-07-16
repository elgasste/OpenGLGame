#if !defined( FONT_H )
#define FONT_H

#include "common.h"
#include "pixel_buffer.h"
#include "platform.h"

typedef struct
{
   PixelBuffer_t pixelBuffer;
   Vector2i32_t offset;
   int32_t advance;
}
FontGlyph_t;

typedef struct
{
   uint32_t codepointOffset;
   int32_t descent;  // negative means below baseline
   int32_t lineGap;
   uint32_t numGlyphs;
   FontGlyph_t* glyphs;

   GLuint textureHandle;
}
Font_t;

Bool_t Font_LoadFromFile( Font_t* font, const char* filePath );

#endif
