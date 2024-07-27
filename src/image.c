#include "image.h"
#include "bmp.h"

Bool_t Image_LoadFromBitmapMemory( Image_t* image, uint8_t* memory, uint32_t memSize, uint32_t imageID )
{
   image->pixelBuffer.memory = 0;
   image->pixelBuffer.dimensions.x = 0;
   image->pixelBuffer.dimensions.y = 0;
   image->textureHandle = 0;

   if ( !Bmp_LoadFromMemory( memory, memSize, &( image->pixelBuffer ), imageID ) )
   {
      return False;
   }

   glGenTextures( 1, &( image->textureHandle ) );
   return True;
}

void Image_ClearData( Image_t* image )
{
   if ( image->pixelBuffer.memory )
   {
      Platform_Free( image->pixelBuffer.memory,
                     (uint64_t)( image->pixelBuffer.dimensions.x * image->pixelBuffer.dimensions.y * 4 ) );

      image->pixelBuffer.memory = 0;
      image->pixelBuffer.dimensions.x = 0;
      image->pixelBuffer.dimensions.y = 0;
      image->textureHandle = 0;
   }
}
