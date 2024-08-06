#include "tilemap.h"
#include "tileset.h"
#include "platform.h"

typedef struct
{
   TileMap_t* tileMap;
   uint32_t tileIndex;
}
TileUpdateInfo_t;

internal void UpdateTileAsync( TileUpdateInfo_t* info );

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

void TileMap_UpdateBuffer( TileMap_t* tileMap )
{
   uint32_t i, threadQueueEntryCounter = 0;
   TileUpdateInfo_t updateInfos[MAX_THREADQUEUE_SIZE];

   for ( i = 0; i < tileMap->numTiles; i++ )
   {
      updateInfos[threadQueueEntryCounter].tileMap = tileMap;
      updateInfos[threadQueueEntryCounter].tileIndex = i;
      Platform_AddThreadQueueEntry( UpdateTileAsync, (void*)( &(updateInfos[threadQueueEntryCounter] ) ) );
      threadQueueEntryCounter++;

      if ( threadQueueEntryCounter == ( MAX_THREADQUEUE_SIZE - 1 ) || i == ( tileMap->numTiles - 1 ) )
      {
         Platform_RunThreadQueue();
         threadQueueEntryCounter = 0;
      }
   }
}

internal void UpdateTileAsync( TileUpdateInfo_t* info )
{
   uint32_t tileX, tileY;
   uint32_t* bufferPos32;

   tileX = info->tileIndex % info->tileMap->dimensions.x;
   tileY = info->tileIndex / info->tileMap->dimensions.x;
   bufferPos32 = (uint32_t*)( info->tileMap->buffer.memory )
      + ( tileX * info->tileMap->tileSet->dimensions.x )
      + ( tileY * info->tileMap->buffer.dimensions.x * info->tileMap->tileSet->dimensions.y );

   TileSet_CopyTileToBuffer( info->tileMap->tileSet, info->tileMap->tileIndexes[info->tileIndex], &( info->tileMap->buffer ), bufferPos32 );
}
