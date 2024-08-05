#include "pixel_buffer.h"
#include "platform.h"

void PixelBuffer_Init( PixelBuffer_t* pixelBuffer, uint32_t width, uint32_t height )
{
   pixelBuffer->dimensions.x = width;
   pixelBuffer->dimensions.y = height;
   pixelBuffer->memory = (uint8_t*)Platform_CAlloc( 1, (uint64_t)( 4 * width * height ) );
}

void PixelBuffer_ClearData( PixelBuffer_t* pixelBuffer )
{
   Platform_Free( pixelBuffer->memory, (uint64_t)( pixelBuffer->dimensions.x * pixelBuffer->dimensions.y * 4 ) );
   pixelBuffer->memory = 0;
   pixelBuffer->dimensions.x = 0;
   pixelBuffer->dimensions.y = 0;
}
