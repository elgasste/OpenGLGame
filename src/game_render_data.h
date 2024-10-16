#if !defined( GAME_RENDER_DATA_H )
#define GAME_RENDER_DATA_H

#include "common.h"
#include "image.h"
#include "font.h"
#include "text_map.h"
#include "sprite.h"

typedef struct GameRenderData_t
{
   Image_t images[ImageID_Count];
   Font_t fonts[FontID_Count];
   TextMap_t textMap;
   SpriteBase_t spriteBases[SpriteBaseID_Count];
   Sprite_t sprites[SpriteID_Count];
}
GameRenderData_t;

void GameRenderData_Init( GameRenderData_t* renderData );

#endif
