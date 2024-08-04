#include "tilemap.h"
#include "tileset.h"
#include "image.h"

void Tilemap_Init( Tilemap_t* tilemap, Tileset_t* tileset, uint32_t numTilesX, uint32_t numTilesY )
{
   uint32_t i;

   tilemap->tileset = tileset;
   tilemap->dimensions.x = numTilesX;
   tilemap->dimensions.y = numTilesY;
   tilemap->tileYOffset = 6;

   tilemap->buffer.dimensions.x = numTilesX * tileset->tileDimensions.x;
   if ( numTilesY > 1 )
   {
      tilemap->buffer.dimensions.x += ( tileset->tileDimensions.x / 2 );
   }

   tilemap->buffer.dimensions.y = tileset->tileDimensions.y;
   if ( numTilesY > 1 )
   {
      tilemap->buffer.dimensions.y += ( ( numTilesY - 1 ) * ( ( tileset->tileDimensions.y / 2 ) - tilemap->tileYOffset ) );
   }

   tilemap->buffer.memory = (uint8_t*)Platform_MAlloc( (uint64_t)( 4 * tilemap->buffer.dimensions.x * tilemap->buffer.dimensions.y ) );

   for ( i = 0; i < ( tilemap->buffer.dimensions.x * tilemap->buffer.dimensions.y ); i++ )
   {
      tilemap->buffer.memory[i] = 0;
   }

   tilemap->tiles = (uint32_t*)Platform_MAlloc( (uint64_t)( 4 * numTilesX * numTilesY ) );

   for ( i = 0; i < ( numTilesX * numTilesY ); i++ )
   {
      tilemap->tiles[i] = 0;
   }

   glGenTextures( 1, &( tilemap->textureHandle ) );
}

void Tilemap_ClearData( Tilemap_t* tilemap )
{
   Platform_Free( tilemap->buffer.memory, (uint64_t)( 4 * tilemap->buffer.dimensions.x * tilemap->buffer.dimensions.y ) );
   Platform_Free( tilemap->tiles, (uint64_t)( 4 * tilemap->dimensions.x * tilemap->dimensions.y ) );
}

void Tilemap_UpdateBuffer( Tilemap_t* tilemap )
{
   // MUFFINS: this is too hard for right now. also, an actual tilemap buffer won't have alternating rows.
   // maybe isometric wasn't the thing to go for right now? why don't we try non-isometric?

   uint32_t i, j;
   Vector2ui32_t* tileDimensions = &( tilemap->tileset->tileDimensions );
   Bool_t offsetRow = False;
   uint32_t *startBufferPos32, *bufferPos32;

   for ( i = tilemap->dimensions.y - 1; i >= 0; i-- )
   {


      for ( j = 0; j < tilemap->dimensions.x; j++ )
      {

      }
   }
}
