#include "texture.h"
#include "bmp.h"

Bool_t Texture_LoadFromFile( Texture_t* texture, const char* filePath )
{
   texture->pixelBuffer.memory = 0;
   texture->pixelBuffer.dimensions.x = 0;
   texture->pixelBuffer.dimensions.y = 0;
   texture->textureHandle = 0;

   if ( !Bmp_LoadFromFile( filePath, &( texture->pixelBuffer ) ) )
   {
      return False;
   }

   glGenTextures( 1, &( texture->textureHandle ) );
   return True;
}

void Texture_ClearData( Texture_t* texture )
{
   if ( texture->pixelBuffer.memory )
   {
      Platform_MemFree( texture->pixelBuffer.memory );

      texture->pixelBuffer.memory = 0;
      texture->pixelBuffer.dimensions.x = 0;
      texture->pixelBuffer.dimensions.y = 0;
      texture->textureHandle = 0;
   }
}
