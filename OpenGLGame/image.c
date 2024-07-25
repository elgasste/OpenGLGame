#include "image.h"
#include "bmp.h"

Bool_t Image_LoadFromBitmapFile( Image_t* image, const char* filePath )
{
   image->pixelBuffer.memory = 0;
   image->pixelBuffer.dimensions.x = 0;
   image->pixelBuffer.dimensions.y = 0;
   image->textureHandle = 0;

   if ( !Bmp_LoadFromFile( filePath, &( image->pixelBuffer ) ) )
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
      Platform_MemFree( image->pixelBuffer.memory );

      image->pixelBuffer.memory = 0;
      image->pixelBuffer.dimensions.x = 0;
      image->pixelBuffer.dimensions.y = 0;
      image->textureHandle = 0;
   }
}
