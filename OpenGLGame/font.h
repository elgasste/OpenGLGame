#if !defined( FONT_H )
#define FONT_H

#include "common.h"
#include "pixel_buffer.h"
#include "platform.h"

typedef struct
{
   PixelBuffer_t pixelBuffer;
   float leftBearing;
   float baselineOffset;
   float advance;
}
FontGlyph_t;

typedef struct
{
   float height;
   float baseline;
   float lineGap;
   FontGlyph_t* glyphs;
}
FontGlyphCollection_t;

typedef struct
{
   uint32_t codepointOffset;
   uint32_t numGlyphCollections;
   uint32_t numGlyphs;
   FontGlyphCollection_t* glyphCollections;
   FontGlyphCollection_t* curGlyphCollection;
   GLuint textureHandle;
}
Font_t;

Bool_t Font_LoadFromFile( Font_t* font, const char* filePath );
Bool_t Font_ContainsChar( Font_t* font, uint32_t codepoint );
void Font_SetCharColor( Font_t* font, uint32_t codepoint, uint32_t color );
void Font_SetColor( Font_t* font, uint32_t color );
void Font_SetGlyphCollectionForHeight( Font_t* font, float height );

#endif
