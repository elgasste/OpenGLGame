#include "render_data.h"

void RenderData_Init( RenderData_t* renderData )
{
   uint32_t i;

   for ( i = 0; i < (uint32_t)ImageID_Count; i++ )
   {
      renderData->images[i].ID = (ImageID_t)i;
      VECTOR2I_RESET( renderData->images[i].pixelBuffer.dimensions );
      renderData->images[i].pixelBuffer.memory = 0;
      renderData->images[i].textureHandle = 0;
   }

   for ( i = 0; i < (uint32_t)FontID_Count; i++ )
   {
      renderData->fonts[i].ID = (FontID_t)i;
   }

   for ( i = 0; i < (uint32_t)SpriteBaseID_Count; i++ )
   {
      renderData->spriteBases[i].ID = (SpriteBaseID_t)i;
   }

   for ( i = 0; i < (uint32_t)SpriteID_Count; i++ )
   {
      renderData->sprites[i].ID = (SpriteID_t)i;
   }
}
