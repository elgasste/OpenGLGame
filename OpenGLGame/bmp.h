#if !defined( BMP_H )
#define BMP_H

#include "common.h"
#include "pixel_buffer.h"

cBool_t cBmp_LoadFromFile( const char* filePath, cPixelBuffer_t* pixelBuffer );

#endif
