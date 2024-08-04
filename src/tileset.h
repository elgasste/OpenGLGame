#if !defined( TILESET_H )
#define TILESET_H

#include "common.h"
#include "vector.h"

typedef struct Image_t Image_t;
typedef struct PixelBuffer_t PixelBuffer_t;

typedef struct Tileset_t
{
   Image_t* image;
   uint32_t numTiles;
   uint32_t stride;
   Vector2ui32_t tileDimensions;
}
Tileset_t;

Bool_t Tileset_Init( Tileset_t* tileset, Image_t* image, ImageID_t imageID,
                     uint32_t tileWidth, uint32_t tileHeight );
void Tileset_CopyTileToBuffer( Tileset_t* tileset, PixelBuffer_t* buffer, uint32_t bufferOffset32, uint32_t tileIndex );

#endif
