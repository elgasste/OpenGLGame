#if !defined( BMP_H )
#define BMP_H

#include "common.h"
#include "pixel_buffer.h"

Bool_t Bmp_LoadFromFile( const char* filePath, PixelBuffer_t* pixelBuffer );

#endif
