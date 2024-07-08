#if !defined( RENDER_H )
#define RENDER_H

#include "common.h"
#include "texture.h"

typedef struct RenderData_t
{
   Texture_t backgroundTexture;
   Texture_t starTexture;
}
RenderData_t;

void Render_Clear();
void Render_DrawTexture( Texture_t* texture, int32_t x, int32_t y );

#endif
