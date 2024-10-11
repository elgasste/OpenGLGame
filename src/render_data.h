#if !defined( RENDER_DATA_H )
#define RENDER_DATA_H

#include "common.h"
#include "image.h"
#include "font.h"
#include "text_map.h"
#include "sprite.h"

typedef struct RenderData_t
{
   Image_t images[ImageID_Count];
   Font_t fonts[FontID_Count];
   TextMap_t textMap;
   SpriteBase_t spriteBases[SpriteBaseID_Count];
   Sprite_t sprites[SpriteID_Count];
}
RenderData_t;

void RenderData_Init( RenderData_t* renderData );

#endif
