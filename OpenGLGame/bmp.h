#if !defined( BMP_H )
#define BMP_H

#include "common.h"

#define BMP_HEADER_SIZE       14
#define BMP_DIBHEADER_SIZE    40

typedef struct
{
   uint32_t imageOffset;
   int32_t width;
   int32_t height;
   uint16_t bitsPerPixel;
   uint32_t imageBytes;
   // TODO: are we gonna use these?
   int32_t hResolution;
   int32_t vResolution;
   uint32_t numPaletteColors;
   uint32_t importantColors;
}
cBmpData_t;

cBool_t cBmp_LoadFromFile( cBmpData_t* bmpData, const char* filePath );

#endif
