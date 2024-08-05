#if !defined( TILEMAP_H )
#define TILEMAP_H

#include "common.h"
#include "vector.h"

typedef struct TileSet_t TileSet_t;

typedef struct TileMap_t
{
   TileSet_t* tileSet;
   uint32_t* tileIndexes;
   uint32_t numTiles;
   Vector2ui32_t dimensions;
}
TileMap_t;

void TileMap_Init( TileMap_t* tileMap, TileSet_t* tileSet, uint32_t numTilesX, uint32_t numTilesY );
void TileMap_ClearData( TileMap_t* tileMap );

#endif
