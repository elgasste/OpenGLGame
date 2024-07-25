#if !defined( BMP_H )
#define BMP_H

#include "common.h"

typedef struct PixelBuffer_t PixelBuffer_t;

Bool_t Bmp_LoadFromFile( const char* filePath, PixelBuffer_t* pixelBuffer );

#endif
