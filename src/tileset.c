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
