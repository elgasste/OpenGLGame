#if !defined( RENDER_H )
#define RENDER_H

#include "common.h"
#include "sprite.h"
#include "font.h"

typedef struct RenderData_t
{
   Texture_t textures[TextureID_Count];
   Font_t fonts[FontID_Count];
}
RenderData_t;

void Render_Clear();
void Render_DrawTextureSection( GLuint textureHandle, PixelBuffer_t* pixelBuffer, float scale,
                                int32_t screenX, int32_t screenY,
                                int32_t textureX, int32_t textureY,
                                uint32_t sectionWidth, uint32_t sectionHeight );
void Render_DrawTexture( Texture_t* texture, float scale, int32_t screenX, int32_t screenY );
void Render_DrawSprite( Sprite_t* sprite, float scale, int32_t screenX, int32_t screenY );
void Render_DrawChar( uint32_t codepoint, float scale, int32_t screenX, int32_t screenY, Font_t* font );
void Render_DrawTextLine( const char* text, float scale, int32_t screenX, int32_t screenY, Font_t* font );

#endif
