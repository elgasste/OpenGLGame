#if !defined( PNG_H )
#define PNG_H

#include "common.h"
#include "file_data.h"

#define PNG_SIGNATURE         0x0a1a0a0d474e5089
#define PNG_CHUNKTYPE_IHDR    0x49484452

typedef struct
{
   uint32_t width;
   uint32_t height;
   uint8_t bitDepth;
   uint8_t colorType;
   uint8_t compressionMethod;
   uint8_t filterMethod;
   uint8_t interlaceMethod;
}
cPngData_t;

cBool_t cPng_LoadImageData( cFileData_t* fileData, cPngData_t* pngData );

#endif
