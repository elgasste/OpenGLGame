#include "tileset.h"
#include "image.h"
#include "platform.h"

Bool_t TileSet_Init( TileSet_t* tileSet, Image_t* image, ImageID_t imageID,
                     uint32_t tileSizeX, uint32_t tileSizeY )
{
   tileSet->image = image;
   tileSet->dimensions.x = tileSizeX;
   tileSet->dimensions.y = tileSizeY;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( ( image->pixelBuffer.dimensions.x % tileSizeX != 0 ) ||
        ( image->pixelBuffer.dimensions.y % tileSizeY != 0 ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_TILESETERR_TILEDIMENSIONS, (uint32_t)imageID );
      Platform_Log( errorMsg );
      return False;
   }

   tileSet->stride = image->pixelBuffer.dimensions.x / tileSizeX;
   tileSet->numTiles = tileSet->stride * ( image->pixelBuffer.dimensions.y / tileSizeY );

   return True;
}

void TileSet_CopyTileToBuffer( TileSet_t* tileSet, uint32_t tileIndex,
                               PixelBuffer_t* buffer, uint32_t* bufferPos32 )
{
   uint32_t i, j;
   uint32_t tileX = tileIndex % tileSet->stride;
   uint32_t tileY = tileIndex / tileSet->stride;
   uint32_t* tileSetPos32 = (uint32_t*)( tileSet->image->pixelBuffer.memory )
      + ( tileY * tileSet->image->pixelBuffer.dimensions.x * tileSet->dimensions.y )
      + ( tileX * tileSet->dimensions.x );

   for ( i = 0; i < tileSet->dimensions.y; i++ )
   {
      for ( j = 0; j < tileSet->dimensions.x; j++ )
      {
         bufferPos32[j] = tileSetPos32[j];
      }

      tileSetPos32 += tileSet->image->pixelBuffer.dimensions.x;
      bufferPos32 += buffer->dimensions.x;
   }
}
