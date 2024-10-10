#if !defined( WAD_H )
#define WAD_H

#include "common.h"
#include "pixel_buffer.h"

typedef struct Wad_t
{
   uint32_t textureCount;
   PixelBuffer_t* textureBuffers;
}
Wad_t;

Bool_t Wad_LoadFromFile( Wad_t* wad, const char* filePath );
void Wad_Clear( Wad_t* wad );

#endif
