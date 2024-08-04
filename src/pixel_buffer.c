#include "pixel_buffer.h"
#include "platform.h"

void PixelBuffer_ClearData( PixelBuffer_t* pixelBuffer )
{
   Platform_Free( pixelBuffer->memory, (uint64_t)( pixelBuffer->dimensions.x * pixelBuffer->dimensions.y * 4 ) );

   pixelBuffer->memory = 0;
   pixelBuffer->dimensions.x = 0;
   pixelBuffer->dimensions.y = 0;
}
