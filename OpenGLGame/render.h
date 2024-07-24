#if !defined( RENDER_H )
#define RENDER_H

#include "common.h"
#include "sprite.h"
#include "font.h"

typedef struct
{
   Texture_t textures[TextureID_Count];
   Font_t fonts[FontID_Count];
}
RenderData_t;

void Render_ClearData( RenderData_t* renderData );
void Render_ClearScreen();
void Render_DrawRect( float screenX, float screenY, float width, float height, uint32_t color );
void Render_DrawTextureSection( GLuint textureHandle, PixelBuffer_t* pixelBuffer, float scale,
                                float screenX, float screenY,
                                int32_t textureX, int32_t textureY,
                                uint32_t sectionWidth, uint32_t sectionHeight );
void Render_DrawTexture( Texture_t* texture, float scale, float screenX, float screenY );
void Render_DrawSprite( Sprite_t* sprite, float scale, float screenX, float screenY );
void Render_DrawChar( uint32_t codepoint, float scale, float screenX, float screenY, Font_t* font );
void Render_DrawTextLine( const char* text, float scale, float screenX, float screenY, Font_t* font );

#endif
