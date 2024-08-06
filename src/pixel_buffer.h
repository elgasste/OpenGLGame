#if !defined( PIXEL_BUFFER_H )
#define PIXEL_BUFFER_H

#include "common.h"
#include "vector.h"

typedef struct PixelBuffer_t
{
   uint8_t* memory;
   Vector2ui32_t dimensions;
}
PixelBuffer_t;

void PixelBuffer_Init( PixelBuffer_t* pixelBuffer, uint32_t width, uint32_t height );
void PixelBuffer_ClearData( PixelBuffer_t* pixelBuffer );

#endif
