#if !defined( IMAGE_H )
#define IMAGE_H

#include "common.h"
#include "pixel_buffer.h"
#include "platform.h"

typedef struct
{
   PixelBuffer_t pixelBuffer;
   GLuint textureHandle;
}
Image_t;

Bool_t Image_LoadFromBitmapFile( Image_t* image, const char* filePath );
void Image_ClearData( Image_t* image );

#endif
