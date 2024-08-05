#if !defined( TILESET_H )
#define TILESET_H

#include "common.h"
#include "vector.h"

typedef struct Image_t Image_t;
typedef struct PixelBuffer_t PixelBuffer_t;

typedef struct TileSet_t
{
   Image_t* image;
   Vector2ui32_t dimensions;
   uint32_t numTiles;
   uint32_t stride;
}
TileSet_t;

Bool_t TileSet_Init( TileSet_t* tileSet, Image_t* image, ImageID_t imageID,
                     uint32_t tileSizeX, uint32_t tileSizeY );
void TileSet_CopyTileToBuffer( TileSet_t* tileSet, uint32_t tileIndex,
                               PixelBuffer_t* buffer, uint32_t* bufferPos32 );

#endif
