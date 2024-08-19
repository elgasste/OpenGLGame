#if !defined( BMP_H )
#define BMP_H

#include "common.h"

typedef struct PixelBuffer_t PixelBuffer_t;

Bool_t Bmp_LoadFromMemory( uint8_t* memory, uint32_t memSize, PixelBuffer_t* pixelBuffer, ImageID_t imageID );

#endif
