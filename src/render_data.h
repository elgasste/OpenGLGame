#if !defined( RENDER_DATA_H )
#define RENDER_DATA_H

#include "common.h"
#include "image.h"
#include "font.h"
#include "sprite.h"

typedef struct RenderData_t
{
   Image_t images[ImageID_Count];
   Font_t fonts[FontID_Count];
   SpriteBase_t spriteBases[SpriteBaseID_Count];
   Sprite_t sprites[SpriteID_Count];
}
RenderData_t;

#endif
