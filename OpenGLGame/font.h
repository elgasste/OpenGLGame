#if !defined( FONT_H )
#define FONT_H

#include "common.h"
#include "pixel_buffer.h"
#include "platform.h"

typedef struct
{
   PixelBuffer_t pixelBuffer;
   int32_t leftBearing;
   int32_t baselineOffset;
   int32_t advance;
}
FontGlyph_t;

typedef struct
{
   uint32_t codepointOffset;
   uint32_t height;
   int32_t baseline;
   int32_t lineGap;
   uint32_t numGlyphs;
   FontGlyph_t* glyphs;
   GLuint textureHandle;
}
Font_t;

Bool_t Font_LoadFromFile( Font_t* font, const char* filePath );
Bool_t Font_ContainsChar( Font_t* font, uint32_t codepoint );
void Font_SetCharColor( Font_t* font, uint32_t codepoint, uint32_t color );
void Font_SetColor( Font_t* font, uint32_t color );

#endif
