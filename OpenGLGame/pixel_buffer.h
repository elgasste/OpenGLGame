#if !defined( IMAGE_BUFFER_H )
#define IMAGE_BUFFER_H

#include "common.h"

typedef struct
{
   uint8_t* buffer;
   uint32_t width;
   uint32_t height;
   uint32_t pitch;
   uint32_t stride;
}
cPixelBuffer_t;

#endif
