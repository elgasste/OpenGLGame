#if !defined( RENDER_H )
#define RENDER_H

#include "common.h"
#include "sprite.h"

typedef struct RenderData_t
{
   Texture_t textures[TextureID_Count];
}
RenderData_t;

void Render_Clear();
void Render_DrawTexture( RenderData_t* renderData, TextureID_t, int32_t x, int32_t y );
void Render_DrawSprite( Sprite_t* sprite, int32_t x, int32_t y );

#endif
