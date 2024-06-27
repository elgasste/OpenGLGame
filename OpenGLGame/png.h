#if !defined( PNG_H )
#define PNG_H

#include "common.h"
#include "file_data.h"

#define PNG_SIGNATURE                        0x0a1a0a0d474e5089

#define PNG_COLORTYPE_GRAYSCALE              0
#define PNG_COLORTYPE_TRUECOLOR              2
#define PNG_COLORTYPE_INDEXED                3
#define PNG_COLORTYPE_GRAYSCALEALPHA         4
#define PNG_COLORTYPE_TRUECOLORALPHA         6

#define PNG_SRGB_PERCEPTUAL                  0
#define PNG_SRGB_RELATIVECOLORIMETRIC        1
#define PNG_SRGB_SATURATION                  2
#define PNG_SRGB_ABSOLUTECOLORIMETRIC        3

#define PNG_MAX_NAMELENGTH                   80

#define PNG_PHYSSPECIFIER_UNKNOWN            0
#define PNG_PHYSSPECIFIER_METER              1

#define PNG_FLAG_HASPALETTE                  0x0001
#define PNG_FLAG_ALLOCATEDPALETTE            0x0002
#define PNG_FLAG_HASTRNSGRAYLEVEL            0x0004
#define PNG_FLAG_HASTRNSCOLOR                0x0008
#define PNG_FLAG_HASGAMMACORRECTION          0x0010
#define PNG_FLAG_HASSIGNIFICANTBITS          0x0020
#define PNG_FLAG_HASSRGB                     0x0040
#define PNG_FLAG_HASCHROMATICITY             0x0080
#define PNG_FLAG_HASICCPROFILE               0x0100
#define PNG_FLAG_ALLOCATEDICCPROFILE         0x0200
#define PNG_FLAG_HASBACKGROUNDCOLOR          0x0400
#define PNG_FLAG_HASPHYSPIXELDIMENSIONS      0x0800
#define PNG_FLAG_HASSUGGESTEDPALETTE         0x1000
#define PNG_FLAG_ALLOCATEDSUGGESTEDPALETTE   0x2000
#define PNG_FLAG_ALLOCATEDDATASECTIONS       0x4000
#define PNG_FLAG_ALLOCATEDIMAGEDATA          0x8000

#define PNG_CHUNKTYPE_IHDR                   0x49484452  // IHDR
#define PNG_CHUNKTYPE_PLTE                   0x504C5445  // PLTE
#define PNG_CHUNKTYPE_IDAT                   0x49444154  // IDAT
#define PNG_CHUNKTYPE_IEND                   0x49454E44  // IEND

#define PNG_CHUNKTYPE_SRGB                   0x73524742  // sRGB
#define PNG_CHUNKTYPE_GAMA                   0x67414D41  // gAMA
#define PNG_CHUNKTYPE_CHRM                   0x6348524D  // cHRM
#define PNG_CHUNKTYPE_ICCP                   0x69434350  // iCCP
#define PNG_CHUNKTYPE_SBIT                   0x73424954  // sBIT

#define PNG_CHUNKTYPE_TRNS                   0x74524E53  // tRNS
#define PNG_CHUNKTYPE_BKGD                   0x624B4744  // bKGD
#define PNG_CHUNKTYPE_HIST                   0x68495354  // hIST

#define PNG_CHUNKTYPE_PHYS                   0x70485973  // pHYS
#define PNG_CHUNKTYPE_SPLT                   0x73504C54  // sPLT

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
cPngHeader_t;

typedef struct
{
   uint16_t numColors;
   uint32_t* colors;
}
cPngPalette_t;

typedef struct
{
   float whitePointX;
   float whitePointY;
   float redX;
   float redY;
   float greenX;
   float greenY;
   float blueX;
   float blueY;
}
cPngChromaticity_t;

typedef struct
{
   char name[PNG_MAX_NAMELENGTH];
   uint8_t compressionMethod;
   uint32_t compressedProfileSize;
   uint8_t* compressedProfile;
}
cPngICCProfile_t;

typedef struct
{
   uint32_t ppuX;
   uint32_t ppuY;
   uint8_t unitSpecifier;
}
cPngPhysPixelDimensions_t;

typedef struct
{
   uint32_t color;
   uint16_t frequency;
}
cPngSuggestedPaletteColor_t;

typedef struct
{
   char name[PNG_MAX_NAMELENGTH];
   uint8_t sampleDepth;
   uint16_t numColors;
   cPngSuggestedPaletteColor_t* colors;
}
cPngSuggestedPalette_t;

typedef struct
{
   uint32_t size;
   uint8_t* data;
}
cPngImageData_t;

typedef struct
{
   cPngHeader_t header;
   cPngPalette_t palette;
   uint16_t trnsGrayLevel;
   uint32_t trnsColor;
   float gammaCorrection;
   uint32_t significantBits;
   uint8_t sRGB;
   cPngChromaticity_t chromaticity;
   cPngICCProfile_t ICCProfile;
   uint32_t backgroundColor;
   cPngPhysPixelDimensions_t physPixelDimensions;
   cPngSuggestedPalette_t suggestedPalette;
   cPngImageData_t imageData;
   uint16_t flags;
}
cPngData_t;

cBool_t cPng_LoadPngData( cFileData_t* fileData, cPngData_t* pngData );
void cPng_ClearPngData( cPngData_t* pngData );

#endif
