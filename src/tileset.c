#include "tileset.h"
#include "image.h"
#include "platform.h"

Bool_t Tileset_Init( Tileset_t* tileset, Image_t* image, ImageID_t imageID,
                     uint32_t tileWidth, uint32_t tileHeight )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   tileset->image = image;
   tileset->tileDimensions.x = tileWidth;
   tileset->tileDimensions.y = tileHeight;

   if ( image->pixelBuffer.dimensions.x % tileWidth != 0 ||
        image->pixelBuffer.dimensions.y % tileHeight != 0 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_TILESETERR_TILESIZE, (uint32_t)imageID );
      Platform_Log( errorMsg );
      return False;
   }

   tileset->stride = image->pixelBuffer.dimensions.x / tileWidth;
   tileset->numTiles = tileset->stride * ( image->pixelBuffer.dimensions.y / tileHeight );

   return True;
}

void Tileset_CopyTileToBuffer( Tileset_t* tileset, PixelBuffer_t* buffer, uint32_t bufferOffset32, uint32_t tileIndex )
{
   uint32_t i, j;
   uint32_t tileOffsetX = tileIndex % tileset->stride;
   uint32_t tileOffsetY = tileIndex / tileset->stride;
   uint32_t* tilesetPos32 = (uint32_t*)( tileset->image->pixelBuffer.memory ) +
      ( tileOffsetY * ( tileset->tileDimensions.x * tileset->tileDimensions.y * tileset->stride ) ) +
      ( tileOffsetX * tileset->tileDimensions.x );
   uint32_t* bufferPos32 = (uint32_t*)( buffer->memory ) + bufferOffset32;

   for ( i = 0; i < tileset->tileDimensions.y; i++ )
   {
      for ( j = 0; j < tileset->tileDimensions.x; j++ )
      {
         bufferPos32[j] = tilesetPos32[j];
      }

      bufferPos32 += buffer->dimensions.x;
      tilesetPos32 += tileset->image->pixelBuffer.dimensions.x;
   }
}
