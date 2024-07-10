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
void Render_DrawTextureSection( Texture_t* texture,
                                int32_t screenX, int32_t screenY,
                                int32_t textureX, int32_t textureY,
                                uint32_t sectionWidth, uint32_t sectionHeight );
void Render_DrawTexture( Texture_t* texture, int32_t screenX, int32_t screenY );
void Render_DrawSprite( Sprite_t* sprite, int32_t screenX, int32_t screenY );

#endif
