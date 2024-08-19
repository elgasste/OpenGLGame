#if !defined( IMAGE_H )
#define IMAGE_H

#include "common.h"
#include "pixel_buffer.h"
#include "platform.h"

typedef struct Image_t
{
   ImageID_t ID;
   PixelBuffer_t pixelBuffer;
   GLuint textureHandle;
}
Image_t;

Bool_t Image_LoadFromBitmapMemory( Image_t* image, uint8_t* memory, uint32_t memSize );
void Image_ClearData( Image_t* image );

#endif
