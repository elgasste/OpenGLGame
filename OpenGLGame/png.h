#if !defined( PNG_H )
#define PNG_H

#include "common.h"
#include "file_data.h"

#define PNG_SIGNATURE            0x0a1a0a0d474e5089

#define PNG_COLORTYPE_GRAYSCALE        0
#define PNG_COLORTYPE_TRUECOLOR        2
#define PNG_COLORTYPE_INDEXED          3
#define PNG_COLORTYPE_GRAYSCALEALPHA   4
#define PNG_COLORTYPE_TRUECOLORALPHA   6

#define PNG_SRGB_PERCEPTUAL            0
#define PNG_SRGB_RELATIVECOLORIMETRIC  1
#define PNG_SRGB_SATURATION            2
#define PNG_SRGB_ABSOLUTECOLORIMETRIC  3

#define PNG_ICC_MAXNAMELENGTH          80

// basic chunk types
#define PNG_CHUNKTYPE_IHDR             0x49484452
#define PNG_CHUNKTYPE_PLTE             0x504C5445
#define PNG_CHUNKTYPE_IDAT             0x49444154
#define PNG_CHUNKTYPE_IEND             0x49454E44

// ancillary chunk types
#define PNG_CHUNKTYPE_SRGB             0x73524742
#define PNG_CHUNKTYPE_GAMA             0x67414D41
#define PNG_CHUNKTYPE_CHRM             0x6348524D
#define PNG_CHUNKTYPE_ICCP             0x69434350
#define PNG_CHUNKTYPE_SBIT             0x73424954

#define PNG_CHUNKTYPE_TRNS             0x74524E53
#define PNG_CHUNKTYPE_BKGD             0x624B4744
#define PNG_CHUNKTYPE_HIST             0x68495354

#define PNG_CHUNKTYPE_PHYS             0x70485973
#define PNG_CHUNKTYPE_SPLT             0x73504C54

#define PNG_CHUNKTYPE_TIME             0x74494D45
#define PNG_CHUNKTYPE_ITXT             0x69545874
#define PNG_CHUNKTYPE_TEXT             0x74455874
#define PNG_CHUNKTYPE_ZTXT             0x7A545874

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
   char name[PNG_ICC_MAXNAMELENGTH];
   uint8_t compressionMethod;
   uint32_t compressedProfileSize;
   uint8_t* compressedProfile;
}
cPngICCProfile_t;

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

   // TODO: make these bitwise flags?
   cBool_t hasPalette;
   cBool_t hasTrnsGrayLevel;
   cBool_t hasTrnsColor;
   cBool_t hasGammaCorrection;
   cBool_t hasSignificantBits;
   cBool_t hasSRGB;
   cBool_t hasChromaticity;
   cBool_t hasICCProfile;
   cBool_t hasBackgroundColor;
}
cPngData_t;

cBool_t cPng_LoadPngData( cFileData_t* fileData, cPngData_t* pngData );
void cPng_DeletePngData( cPngData_t* pngData );

#endif
