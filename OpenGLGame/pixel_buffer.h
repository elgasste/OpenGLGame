#if !defined( PIXEL_BUFFER_H )
#define PIXEL_BUFFER_H

#include "common.h"

typedef struct
{
   uint8_t* buffer;
   uint32_t width;
   uint32_t height;
}
PixelBuffer_t;

#endif
