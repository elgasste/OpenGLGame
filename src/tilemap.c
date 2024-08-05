#include "tilemap.h"
#include "platform.h"

void TileMap_Init( TileMap_t* tileMap, TileSet_t* tileSet, uint32_t numTilesX, uint32_t numTilesY )
{
   uint32_t i;

   tileMap->tileSet = tileSet;
   tileMap->numTiles = numTilesX * numTilesY;
   tileMap->dimensions.x = numTilesX;
   tileMap->dimensions.y = numTilesY;

   tileMap->tileIDs = (uint32_t*)Platform_MAlloc( (uint64_t)( 4 * tileMap->numTiles ) );

   for ( i = 0; i < tileMap->numTiles; i++ )
   {
      tileMap->tileIDs[i] = 0;
   }
}

void TileMap_ClearData( TileMap_t* tileMap )
{
   Platform_Free( tileMap->tileIDs, (uint64_t)( 4 * tileMap->numTiles ) );
   tileMap->tileIDs = 0;
   tileMap->numTiles = 0;
   tileMap->dimensions.x = 0;
   tileMap->dimensions.y = 0;
}