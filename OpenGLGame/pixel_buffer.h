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

#endif
