#if !defined( ASSETS_H )
#define ASSETS_H

#include "common.h"
#include "image.h"
#include "font.h"
#include "text_map.h"
#include "sprite.h"

typedef struct Assets_t
{
   Image_t images[ImageID_Count];
   Font_t fonts[FontID_Count];
   TextMap_t textMap;
   SpriteBase_t spriteBases[SpriteBaseID_Count];
   Sprite_t sprites[SpriteID_Count];
}
Assets_t;

void Assets_Init( Assets_t* assets );

#endif
