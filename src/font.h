#if !defined( FONT_H )
#define FONT_H

#include "common.h"
#include "pixel_buffer.h"
#include "vector.h"
#include "platform.h"

typedef struct FontGlyph_t
{
   PixelBuffer_t pixelBuffer;
   float leftBearing;
   float baselineOffset;
   float advance;
   uint32_t color;
}
FontGlyph_t;

typedef struct FontGlyphCollection_t
{
   float height;
   float baseline;
   float lineGap;
   FontGlyph_t* glyphs;
}
FontGlyphCollection_t;

typedef struct Font_t
{
   uint32_t codepointOffset;
   uint32_t numGlyphCollections;
   uint32_t numGlyphs;
   FontGlyphCollection_t* glyphCollections;
   FontGlyphCollection_t* curGlyphCollection;
   GLuint textureHandle;
}
Font_t;

Bool_t Font_LoadFromMemory( Font_t* font, uint8_t* memory, uint32_t memSize, uint32_t fontID );
void Font_ClearData( Font_t* font );
Bool_t Font_ContainsChar( Font_t* font, uint32_t codepoint );
void Font_SetCharColor( Font_t* font, uint32_t codepoint, uint32_t color );
void Font_SetColor( Font_t* font, uint32_t color );
void Font_SetGlyphCollectionForHeight( Font_t* font, float height );
Vector2f_t Font_GetTextDimensions( Font_t* font, const char* text );

#endif
