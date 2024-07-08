#if !defined( RENDER_H )
#define RENDER_H

#include "common.h"
#include "pixel_buffer.h"
#include "platform.h"

typedef struct
{
   PixelBuffer_t pixelBuffer;
   GLuint textureHandle;
}
Texture_t;

typedef struct
{
   Texture_t backgroundTexture;
}
RenderData_t;

void Render_Init( RenderData_t* renderData );
void Render_Clear();
void Render_DrawTexture( int32_t x, int32_t y, Texture_t* texture );

#endif
