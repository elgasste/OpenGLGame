#if !defined( BMP_H )
#define BMP_H

#include "common.h"
#include "image_buffer.h"

#define BMP_HEADER_SIZE                14
#define BMP_BITMAPINFOHEADER_SIZE      40
#define BMP_HEADER_TYPE                0x4D42

typedef struct
{
   uint32_t dibHeaderSize;
   uint32_t imageOffset;
   int32_t imageWidth;
   int32_t imageHeight;
   uint16_t bitsPerPixel;
   uint32_t strideBits;
   uint8_t paddingBits;
   uint32_t scanlineSize;
   uint32_t imageBytes;
   uint32_t numPaletteColors;
   uint32_t* paletteColors;

   cImageBuffer_t imageBuffer;
}
cBmpData_t;

cBool_t cBmp_LoadFromFile( cBmpData_t* bmpData, const char* filePath );
void cBmp_Cleanup( cBmpData_t* bmpData );

#endif
