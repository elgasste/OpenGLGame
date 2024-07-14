#if !defined( FONT_H )
#define FONT_H

#include "common.h"
#include "pixel_buffer.h"

typedef struct
{
   uint32_t codepointOffset;
   uint32_t numGlyphs;
   PixelBuffer_t* glyphs;
}
Font_t;

Bool_t Font_LoadFromFile( Font_t* font, const char* filePath );

#endif
