#include "tilemap.h"
#include "tileset.h"
#include "platform.h"

void TileMap_Init( TileMap_t* tileMap, TileSet_t* tileSet, uint32_t numTilesX, uint32_t numTilesY )
{
   uint32_t i;

   tileMap->tileSet = tileSet;
   tileMap->numTiles = numTilesX * numTilesY;
   tileMap->dimensions.x = numTilesX;
   tileMap->dimensions.y = numTilesY;

   tileMap->tileIndexes = (uint32_t*)Platform_MAlloc( (uint64_t)( 4 * tileMap->numTiles ) );

   for ( i = 0; i < tileMap->numTiles; i++ )
   {
      tileMap->tileIndexes[i] = 0;
   }

   PixelBuffer_Init( &( tileMap->buffer ),
                     tileMap->dimensions.x * tileMap->tileSet->dimensions.x,
                     tileMap->dimensions.y * tileMap->tileSet->dimensions.y );

   glGenTextures( 1, &( tileMap->textureHandle ) );
}

void TileMap_ClearData( TileMap_t* tileMap )
{
   PixelBuffer_ClearData( &( tileMap->buffer ) );
   Platform_Free( tileMap->tileIndexes, (uint64_t)( 4 * tileMap->numTiles ) );
   tileMap->tileIndexes = 0;
   tileMap->numTiles = 0;
   tileMap->dimensions.x = 0;
   tileMap->dimensions.y = 0;
}