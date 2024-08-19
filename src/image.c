#include "image.h"
#include "bmp.h"

Bool_t Image_LoadFromBitmapMemory( Image_t* image, uint8_t* memory, uint32_t memSize )
{
   image->pixelBuffer.memory = 0;
   image->pixelBuffer.dimensions.x = 0;
   image->pixelBuffer.dimensions.y = 0;
   image->textureHandle = 0;

   if ( !Bmp_LoadFromMemory( memory, memSize, &( image->pixelBuffer ), image->ID ) )
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
      PixelBuffer_ClearData( &( image->pixelBuffer ) );
      image->textureHandle = 0;
   }
}
