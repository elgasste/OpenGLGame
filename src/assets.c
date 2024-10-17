#include "assets.h"

void Assets_Init( Assets_t* assets )
{
   uint32_t i;

   for ( i = 0; i < (uint32_t)ImageID_Count; i++ )
   {
      assets->images[i].ID = (ImageID_t)i;
      VECTOR2I_RESET( assets->images[i].pixelBuffer.dimensions );
      assets->images[i].pixelBuffer.memory = 0;
      assets->images[i].textureHandle = 0;
   }

   for ( i = 0; i < (uint32_t)FontID_Count; i++ )
   {
      assets->fonts[i].ID = (FontID_t)i;
   }

   for ( i = 0; i < (uint32_t)SpriteBaseID_Count; i++ )
   {
      assets->spriteBases[i].ID = (SpriteBaseID_t)i;
   }

   for ( i = 0; i < (uint32_t)SpriteID_Count; i++ )
   {
      assets->sprites[i].ID = (SpriteID_t)i;
   }
}
