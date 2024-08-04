#if !defined( TILEMAP_H )
#define TILEMAP_H

#include "common.h"
#include "pixel_buffer.h"
#include "platform.h"

typedef struct Tileset_t Tileset_t;

typedef struct Tilemap_t
{
   Tileset_t* tileset;
   Vector2ui32_t dimensions;
   uint32_t* tiles;
   PixelBuffer_t buffer;
   int32_t tileYOffset;
   GLuint textureHandle;
}
Tilemap_t;

void Tilemap_Init( Tilemap_t* tilemap, Tileset_t* tileset, uint32_t numTilesX, uint32_t numTilesY );
void Tilemap_ClearData( Tilemap_t* tilemap );
void Tilemap_UpdateBuffer( Tilemap_t* tilemap );

#endif
