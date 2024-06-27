#include <math.h>

#include "png.h"
#include "platform.h"

#define EXTRACT_CHUNK_DATA_32( x ) 0 | ( x[0] << 24 ) | ( x[1] << 16 ) | ( x[2] << 8 ) | x[3]

#define HAS_FLAG( f ) ( pngData->flags & f )
#define TURN_ON_FLAG( f ) pngData->flags |= f
#define TURN_OFF_FLAG( f ) pngData->flags ^= f

#define ERROR_RETURN_FALSE( m ) \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, m, filePath ); \
   Platform_Log( errorMsg ); \
   return cFalse
#define ERROR_NOT_GOOD( m ) \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, m, filePath ); \
   Platform_Log( errorMsg ); \
   stillGood = cFalse

typedef struct
{
   uint32_t size;
   uint8_t* data;
   void* nextSection;
}
cPngImageDataSection_t;

internal void cPng_InitData( cPngData_t* pngData );
internal cBool_t cPng_LoadHeader( uint8_t* filePos, cPngData_t* pngData, const char* filePath );
internal cBool_t cPng_LoadPaletteChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal void cPng_LoadImageDataChunk( uint8_t* filePos, uint32_t chunkSize, cPngImageDataSection_t* imageDataSections );
internal cBool_t cPng_LoadSRGBChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadGammaChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadChromaticitiesChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadICCProfileChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadSignificantBitsChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadTransparencyChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadBackgroundColorChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadPhysicalPixelDimensionsChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadSuggestedPaletteChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_ConsolidateImageData( const char* filePath, cPngData_t* pngData, uint32_t numImageDataSections, cPngImageDataSection_t* imageDataSections );

cBool_t cPng_LoadPngData( cFileData_t* fileData, cPngData_t* pngData )
{
   uint8_t* filePos;
   uint32_t chunkSize, chunkType, bytesRead;
   char errorMsg[STRING_SIZE_DEFAULT];
   const char* filePath = fileData->filePath;
   uint32_t numImageDataSections = 0;
   cPngImageDataSection_t* imageDataSections = 0;
   cBool_t foundImageData = cFalse;
   cBool_t readingImageData = cFalse;
   cBool_t finishedReadingImageData = cFalse;
   cBool_t foundSRGB = cFalse;
   cBool_t foundChromaticities = cFalse;
   cBool_t foundGamma = cFalse;
   cBool_t foundICCProfile = cFalse;
   cBool_t foundSignificantBits = cFalse;
   cBool_t foundBackgroundColor = cFalse;
   cBool_t foundTransparency = cFalse;
   cBool_t foundPhysicalPixelDimensions = cFalse;
   cBool_t stillGood = cTrue;

   cPng_InitData( pngData );

   // the file signature plus header data is 29 bytes
   if ( fileData->size < 29 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_FILECORRUPT );
   }

   if ( ( (uint64_t*)( fileData->contents ) )[0] != PNG_SIGNATURE )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_NOTAPNGFILE );
   }

   filePos = (uint8_t*)( fileData->contents ) + 8;

   chunkSize = EXTRACT_CHUNK_DATA_32( filePos );
   filePos += 4;
   chunkType = EXTRACT_CHUNK_DATA_32( filePos );
   filePos += 4;

   if ( chunkSize != 13 || chunkType != PNG_CHUNKTYPE_IHDR )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_HEADERNOTFOUND );
   }

   if ( !cPng_LoadHeader( filePos, pngData, filePath ) )
   {
      return cFalse;
   }

   filePos += 17;
   bytesRead = 33;

   while ( 1 )
   {
      if ( fileData->size < ( bytesRead + 8 ) )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_FILECORRUPT );
      }

      chunkSize = EXTRACT_CHUNK_DATA_32( filePos );
      filePos += 4;
      chunkType = EXTRACT_CHUNK_DATA_32( filePos );
      filePos += 4;
      bytesRead += 8;

      if ( chunkType == PNG_CHUNKTYPE_IEND )
      {
         readingImageData = cFalse;
         finishedReadingImageData = cTrue;
         bytesRead += 4;
         break;
      }
      else if ( chunkType != PNG_CHUNKTYPE_IDAT && readingImageData )
      {
         readingImageData = cFalse;
         finishedReadingImageData = cTrue;
      }

      if ( fileData->size < ( bytesRead + chunkSize ) )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_FILECORRUPT );
      }

      switch ( chunkType )
      {
         case PNG_CHUNKTYPE_IHDR:
            ERROR_NOT_GOOD( STR_PNGERROR_FILECORRUPT );
            break;
         case PNG_CHUNKTYPE_PLTE:
            if ( HAS_FLAG( PNG_FLAG_HASPALETTE ) || foundImageData )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_ROGUEPALETTE );
            }
            else
            {
               stillGood = cPng_LoadPaletteChunk( filePos, chunkSize, filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_IDAT:
            if ( finishedReadingImageData )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_ROGUEIDAT );
            }
            else
            {
               foundImageData = cTrue;
               readingImageData = cTrue;
               cPng_LoadImageDataChunk( filePos, chunkSize, imageDataSections );
               numImageDataSections++;
            }
            break;
         case PNG_CHUNKTYPE_SRGB:
            if ( HAS_FLAG( PNG_FLAG_HASPALETTE ) || foundSRGB )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_FILECORRUPT );
            }
            else
            {
               foundSRGB = cTrue;
               stillGood = cPng_LoadSRGBChunk( filePos, chunkSize, filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_GAMA:
            if ( HAS_FLAG( PNG_FLAG_HASPALETTE ) || foundGamma )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_FILECORRUPT );
            }
            else
            {
               foundGamma = cTrue;
               stillGood = cPng_LoadGammaChunk( filePos, chunkSize, filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_CHRM:
            if ( HAS_FLAG( PNG_FLAG_HASPALETTE ) || foundChromaticities )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_FILECORRUPT );
            }
            else
            {
               foundChromaticities = cTrue;
               stillGood = cPng_LoadChromaticitiesChunk( filePos, chunkSize, filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_ICCP:
            if ( HAS_FLAG( PNG_FLAG_HASPALETTE ) || foundICCProfile )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_FILECORRUPT );
            }
            else
            {
               foundICCProfile = cTrue;
               stillGood = cPng_LoadICCProfileChunk( filePos, chunkSize, filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_SBIT:
            if ( HAS_FLAG( PNG_FLAG_HASPALETTE ) || foundSignificantBits )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_FILECORRUPT );
            }
            else
            {
               foundSignificantBits = cTrue;
               stillGood = cPng_LoadSignificantBitsChunk( filePos, chunkSize, filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_TRNS:
            if ( ( pngData->header.colorType == PNG_COLORTYPE_INDEXED && !HAS_FLAG( PNG_FLAG_HASPALETTE ) ) ||
                 ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALEALPHA ) ||
                 ( pngData->header.colorType == PNG_COLORTYPE_TRUECOLORALPHA ) ||
                 foundImageData ||
                 foundTransparency )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_FILECORRUPT );
            }
            else
            {
               foundTransparency = cTrue;
               stillGood = cPng_LoadTransparencyChunk( filePos, chunkSize, filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_BKGD:
            if ( ( pngData->header.colorType == 3 && !HAS_FLAG( PNG_FLAG_HASPALETTE ) ) || foundImageData || foundBackgroundColor )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_FILECORRUPT );
            }
            else
            {
               foundBackgroundColor = cTrue;
               stillGood = cPng_LoadBackgroundColorChunk( filePos, chunkSize, filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_PHYS:
            if ( foundImageData || foundPhysicalPixelDimensions )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_FILECORRUPT );
            }
            else
            {
               foundPhysicalPixelDimensions = cTrue;
               stillGood = cPng_LoadPhysicalPixelDimensionsChunk( filePos, chunkSize, filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_SPLT:
            if ( foundImageData )
            {
               ERROR_NOT_GOOD( STR_PNGERROR_FILECORRUPT );
            }
            else
            {
               stillGood = cPng_LoadSuggestedPaletteChunk( filePos, chunkSize, filePath, pngData );
            }
            break;
      }

      if ( !stillGood )
      {
         return cFalse;
      }

      // every chunk contains a 4-byte CRC after the data
      filePos += ( chunkSize + 4 );
      bytesRead += ( chunkSize + 4 );
   }

   if ( bytesRead != fileData->size || !foundImageData )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_FILECORRUPT );
   }

   return cPng_ConsolidateImageData( filePath, pngData, numImageDataSections, imageDataSections );
}

void cPng_ClearPngData( cPngData_t* pngData )
{
   if ( HAS_FLAG( PNG_FLAG_ALLOCATEDPALETTE ) )
   {
      Platform_MemFree( pngData->palette.colors );
      TURN_OFF_FLAG( PNG_FLAG_ALLOCATEDPALETTE );
   }

   if ( HAS_FLAG( PNG_FLAG_ALLOCATEDICCPROFILE ) )
   {
      Platform_MemFree( pngData->ICCProfile.compressedProfile );
      TURN_OFF_FLAG( PNG_FLAG_ALLOCATEDICCPROFILE );
   }

   if ( HAS_FLAG( PNG_FLAG_ALLOCATEDSUGGESTEDPALETTE ) )
   {
      Platform_MemFree( pngData->suggestedPalette.colors );
      TURN_OFF_FLAG( PNG_FLAG_ALLOCATEDSUGGESTEDPALETTE );
   }

   if ( HAS_FLAG( PNG_FLAG_ALLOCATEDIMAGEDATA ) )
   {
      Platform_MemFree( pngData->imageData.data );
      TURN_OFF_FLAG( PNG_FLAG_ALLOCATEDIMAGEDATA );
   }

   // MUFFINS: it's technically possible that some image data sections have been allocated here,
   // we should definitely account for that.
}

internal void cPng_InitData( cPngData_t* pngData )
{
   pngData->header.width = 0;
   pngData->header.height = 0;
   pngData->header.bitDepth = 0;
   pngData->header.colorType = 0;
   pngData->header.compressionMethod = 0;
   pngData->header.filterMethod = 0;
   pngData->header.interlaceMethod = 0;
   pngData->palette.numColors = 0;
   pngData->palette.colors = 0;
   pngData->trnsGrayLevel = 0;
   pngData->trnsColor = 0;
   pngData->gammaCorrection = 0.0f;
   pngData->significantBits = 0;
   pngData->sRGB = 0;
   pngData->chromaticity.whitePointX = 0.0f;
   pngData->chromaticity.whitePointY = 0.0f;
   pngData->chromaticity.redX = 0.0f;
   pngData->chromaticity.redY = 0.0f;
   pngData->chromaticity.greenX = 0.0f;
   pngData->chromaticity.greenY = 0.0f;
   pngData->chromaticity.blueX = 0.0f;
   pngData->chromaticity.blueY = 0.0f;
   pngData->ICCProfile.name[0] = '\0';
   pngData->ICCProfile.compressionMethod = 0;
   pngData->ICCProfile.compressedProfileSize = 0;
   pngData->ICCProfile.compressedProfile = 0;
   pngData->backgroundColor = 0;
   pngData->physPixelDimensions.ppuX = 0;
   pngData->physPixelDimensions.ppuY = 0;
   pngData->physPixelDimensions.unitSpecifier = 0;
   pngData->suggestedPalette.name[0] = '\0';
   pngData->suggestedPalette.numColors = 0;
   pngData->suggestedPalette.colors = 0;
   pngData->imageData.size = 0;
   pngData->imageData.data = 0;
   pngData->flags = 0;
}

internal cBool_t cPng_LoadHeader( uint8_t* filePos, cPngData_t* pngData, const char* filePath )
{
   char errorMsg[STRING_SIZE_DEFAULT];
   cPngHeader_t* header = &( pngData->header );

   header->width = EXTRACT_CHUNK_DATA_32( filePos );
   filePos += 4;
   header->height = EXTRACT_CHUNK_DATA_32( filePos );
   filePos += 4;
   header->bitDepth = filePos[0];            // bits per sample, or per palette index (not per pixel)
                                             // allowed combinations of bit depths with color types:
                                             // - color type 0: bit depths 1, 2, 4, 8, 16 (each pixel is a grayscale sample)
                                             // - color type 2: bit depths 8, 16 (each pixel is an RGB triple)
                                             // - color type 3: bit depths 1, 2, 4, 8 (each pixel is a palette index, and a
                                             //                                        PLTE chunk must be present)
                                             // - color type 4: bit depths 8, 16 (each pixel is a grayscale sample,
                                             //                                   followed by an alpha sample)
                                             // - color type 6: bit depths 8, 16 (each pixel is an RGB triple,
                                             //                                   followed by an alpha sample)
   filePos++;
   header->colorType = filePos[0];           // sums of the following:
                                             // 1 - palette used
                                             // 2 - color used,
                                             // 4 - alpha channel used
                                             // possible values are 0, 2, 3, 4, and 6 (see above)
   filePos++;
   header->compressionMethod = filePos[0];   // only 0 (deflate/inflate compression with a 32K sliding window) is defined
   filePos++;
   header->filterMethod = filePos[0];        // only 0 (adaptive filtering with five basic filter types) is defined
   filePos++;
   header->interlaceMethod = filePos[0];     // 0 (no interlace) or 1 (Adam7 interlace)
   filePos++;

   if ( header->colorType == PNG_COLORTYPE_GRAYSCALE )
   {
      if ( header->bitDepth != 1 && header->bitDepth != 2 && header->bitDepth != 4 && header->bitDepth != 8 && header->bitDepth != 16 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_COLORTYPEMISMATCH );
      }
   }
   else if ( header->colorType == PNG_COLORTYPE_TRUECOLOR ||
             header->colorType == PNG_COLORTYPE_GRAYSCALEALPHA ||
             header->colorType == PNG_COLORTYPE_TRUECOLORALPHA )
   {
      if ( header->bitDepth != 8 && header->bitDepth != 16 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_COLORTYPEMISMATCH );
      }
   }
   else if ( header->colorType == PNG_COLORTYPE_INDEXED )
   {
      if ( header->bitDepth != 1 && header->bitDepth != 2 && header->bitDepth != 4 && header->bitDepth != 8 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_COLORTYPEMISMATCH );
      }
   }
   else
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_INVALIDCOLORTYPE );
   }

   if ( header->compressionMethod != 0 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_INVALIDCOMPRESSION );
   }

   if ( header->filterMethod != 0 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_INVALIDFILTER );
   }

   if ( header->interlaceMethod > 1 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_INVALIDINTERLACE );
   }

   return cTrue;
}

internal cBool_t cPng_LoadPaletteChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   uint8_t r, g, b;
   uint16_t maxColors, i;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( ( chunkSize % 3 ) != 0 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_INVALIDPALETTESIZE );
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALE ||
             pngData->header.colorType == PNG_COLORTYPE_GRAYSCALEALPHA )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_ROGUEPALETTE );
   }

   if ( pngData->header.bitDepth > 8 )
   {
      maxColors = 256;
   }
   else
   {
      maxColors = (uint16_t)pow( (double)2, (double)( pngData->header.bitDepth ) );
   }

   TURN_ON_FLAG( PNG_FLAG_HASPALETTE );
   pngData->palette.numColors = (uint16_t)( ( chunkSize / 3 ) / pngData->header.bitDepth );

   if ( pngData->palette.numColors > maxColors )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_PALETTEENTRYOVERRUN );
   }

   if ( HAS_FLAG( PNG_FLAG_ALLOCATEDPALETTE ) )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_PNGDATAREUSE );
   }

   pngData->palette.colors = (uint32_t*)Platform_MemAlloc( sizeof( uint32_t ) * pngData->palette.numColors );
   TURN_ON_FLAG( PNG_FLAG_ALLOCATEDPALETTE );

   for ( i = 0; i < pngData->palette.numColors; i++ )
   {
      r = filePos[0];
      g = filePos[1];
      b = filePos[2];
      pngData->palette.colors[i] = 0xFF000000 | ( (uint32_t)r << 16 ) | ( (uint32_t)g << 8 ) | (uint32_t)b;
      filePos += 3;
   }

   return cTrue;
}

internal void cPng_LoadImageDataChunk( uint8_t* filePos, uint32_t chunkSize, cPngImageDataSection_t* imageDataSections )
{
   uint32_t i;
   cPngImageDataSection_t* section;

   if ( !imageDataSections )
   {
      imageDataSections = (cPngImageDataSection_t*)Platform_MemAlloc( sizeof( cPngImageDataSection_t ) );
      section = imageDataSections;
   }
   else
   {
      section = imageDataSections;

      while ( section->nextSection != 0 )
      {
         section = section->nextSection;
      }

      section->nextSection = (cPngImageDataSection_t*)Platform_MemAlloc( sizeof( cPngImageDataSection_t ) );
      section = section->nextSection;
   }

   section->size = chunkSize;
   section->data = (uint8_t*)Platform_MemAlloc( chunkSize );
   section->nextSection = 0;

   for ( i = 0; i < chunkSize; i++ )
   {
      section->data[i] = filePos[i];
   }
}

internal cBool_t cPng_LoadSRGBChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( chunkSize != 1 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_SRGBCORRUPT );
   }
   else if ( filePos[0] > PNG_SRGB_ABSOLUTECOLORIMETRIC )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_SRGBINVALID );
   }

   TURN_ON_FLAG( PNG_FLAG_HASSRGB );
   pngData->sRGB = filePos[0];

   return cTrue;
}

internal cBool_t cPng_LoadGammaChunk(  uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( chunkSize != 4 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_GAMMACORRUPT );
   }

   TURN_ON_FLAG( PNG_FLAG_HASGAMMACORRECTION );
   pngData->gammaCorrection = (float)( ( (uint32_t*)filePos )[0] ) / 100000;

   return cTrue;
}

internal cBool_t cPng_LoadChromaticitiesChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   uint32_t* filePos32 = (uint32_t*)filePos;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( chunkSize != sizeof( cPngChromaticity_t ) )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_CHRMCORRUPT );
   }

   TURN_ON_FLAG( PNG_FLAG_HASCHROMATICITY );
   pngData->chromaticity.whitePointX = (float)( filePos32[0] ) / 100000;
   pngData->chromaticity.whitePointY = (float)( filePos32[1] ) / 100000;
   pngData->chromaticity.redX = (float)( filePos32[2] ) / 100000;
   pngData->chromaticity.redY = (float)( filePos32[3] ) / 100000;
   pngData->chromaticity.greenX = (float)( filePos32[4] ) / 100000;
   pngData->chromaticity.greenY = (float)( filePos32[5] ) / 100000;
   pngData->chromaticity.blueX = (float)( filePos32[6] ) / 100000;
   pngData->chromaticity.blueY = (float)( filePos32[7] ) / 100000;

   return cTrue;
}

internal cBool_t cPng_LoadICCProfileChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   uint32_t i;
   cBool_t foundName = cFalse;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( chunkSize < 4 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_ICCCORRUPT );
   }

   TURN_ON_FLAG( PNG_FLAG_HASICCPROFILE );

   for ( i = 0; i < PNG_MAX_NAMELENGTH; i++ )
   {
      if ( i == chunkSize - 1 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_ICCCORRUPT );
      }

      pngData->ICCProfile.name[i] = filePos[0];
      filePos++;

      if ( pngData->ICCProfile.name[i] == '\0' )
      {
         foundName = cTrue;
         break;
      }
   }

   if ( i >= chunkSize - 3 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_ICCCORRUPT );
   }
   else if ( filePos[0] != 0 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_ICCCOMPRESSIONINVALID );
   }

   filePos++;
   pngData->ICCProfile.compressedProfileSize = chunkSize - ( i + 2 );
   pngData->ICCProfile.compressedProfile = (uint8_t*)Platform_MemAlloc( pngData->ICCProfile.compressedProfileSize );
   TURN_ON_FLAG( PNG_FLAG_ALLOCATEDICCPROFILE );

   for ( i = 0; i < pngData->ICCProfile.compressedProfileSize; i++ )
   {
      pngData->ICCProfile.compressedProfile[i] = filePos[0];
      filePos++;
   }

   return cTrue;
}

internal cBool_t cPng_LoadSignificantBitsChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   TURN_ON_FLAG( PNG_FLAG_HASSIGNIFICANTBITS );

   if ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALE )
   {
      if ( chunkSize != 1 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_SBITCORRUPT );
      }

      pngData->significantBits = (uint32_t)( filePos[0] );
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_TRUECOLOR || pngData->header.colorType == PNG_COLORTYPE_INDEXED )
   {
      if ( chunkSize != 3 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_SBITCORRUPT );
      }

      pngData->significantBits = 0 | ( (uint32_t)( filePos[0] ) << 16 ) | ( (uint32_t)( filePos[0] ) << 8 ) | (uint32_t)( filePos[0] );
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALEALPHA )
   {
      if ( chunkSize != 2 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_SBITCORRUPT );
      }

      pngData->significantBits = (uint32_t)( ( (uint16_t*)filePos )[0] );
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_TRUECOLORALPHA )
   {
      if ( chunkSize != 4 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_SBITCORRUPT );
      }

      pngData->significantBits = ( (uint32_t*)filePos )[0];
   }

   return cTrue;
}

internal cBool_t cPng_LoadTransparencyChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   uint16_t numTransparentColors, i;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( pngData->header.colorType == PNG_COLORTYPE_INDEXED )
   {
      // there should be a stream of 1-byte alpha values, each one corresponding
      // to a palette index. there can be fewer alpha values than palette colors.
      numTransparentColors = (uint16_t)chunkSize;

      if ( numTransparentColors > pngData->palette.numColors )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_TRNSOVERRUN );
      }

      for ( i = 0; i < numTransparentColors; i++ )
      {
         pngData->palette.colors[i] = ( (uint32_t)( filePos[0] ) << 24 ) | ( pngData->palette.colors[i] & 0x00FFFFFF );
         filePos++;
      }
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALE )
   {
      // there should be 2 bytes that contain the gray level value, and if the
      // bit depth is less than 16, the least significant bits are used and the
      // others are presumed to be zero. any pixels at this gray level are to be
      // considered transparent, and all others are to be considered opaque.
      if ( chunkSize != 2 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_TRNSCORRUPT );
      }

      TURN_ON_FLAG( PNG_FLAG_HASTRNSGRAYLEVEL );
      pngData->trnsGrayLevel = ( (uint16_t*)filePos )[0];
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_TRUECOLOR )
   {
      // there should be 6 bytes that contain a single RGB color value, in the order
      // of R, G, B. if the bit depth is less than 16, the least significant bits are
      // used and the others are presumed to be zero. any pixels of this color are
      // to be considered transparent, and all others are to be considered opaque.
      if ( chunkSize != 6 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_TRNSCORRUPT );
      }

      TURN_ON_FLAG( PNG_FLAG_HASTRNSCOLOR );
      pngData->trnsColor = 0xFF000000;
      pngData->trnsColor |= (uint32_t)( (uint16_t*)filePos )[0] << 16;
      pngData->trnsColor |= (uint32_t)( (uint16_t*)filePos )[2] << 8;
      pngData->trnsColor |= (uint32_t)( (uint16_t*)filePos )[4];
   }

   return cTrue;
}

internal cBool_t cPng_LoadBackgroundColorChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   TURN_ON_FLAG( PNG_FLAG_HASBACKGROUNDCOLOR );

   if ( pngData->header.colorType == PNG_COLORTYPE_INDEXED )
   {
      if ( chunkSize != 1 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_BKGDCORRUPT );
      }

      pngData->backgroundColor = (uint16_t)( filePos[0] );
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALE ||
             pngData->header.colorType == PNG_COLORTYPE_GRAYSCALEALPHA )
   {
      if ( chunkSize != 2 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_BKGDCORRUPT );
      }

      pngData->backgroundColor = 0xFF000000 | (uint32_t)( filePos[0] );
   }
   else
   {
      if ( chunkSize != 6 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_BKGDCORRUPT );
      }

      pngData->backgroundColor = 0xFF000000 |
                                 ( (uint32_t)( ( (uint16_t*)filePos )[0] ) << 16 ) |
                                 ( (uint32_t)( ( (uint16_t*)filePos )[2] ) << 8 ) |
                                 (uint32_t)( ( (uint16_t*)filePos )[4] );
   }

   return cTrue;
}

internal cBool_t cPng_LoadPhysicalPixelDimensionsChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( chunkSize != 9 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_PHYSCORRUPT );
   }

   TURN_ON_FLAG( PNG_FLAG_HASPHYSPIXELDIMENSIONS );
   pngData->physPixelDimensions.ppuX = ( (uint32_t*)filePos )[0];
   pngData->physPixelDimensions.ppuY = ( (uint32_t*)filePos )[1];
   pngData->physPixelDimensions.unitSpecifier = filePos[8];

   if ( pngData->physPixelDimensions.unitSpecifier > PNG_PHYSSPECIFIER_METER )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_INVALIDPHYSSPECIFIER );
   }

   return cTrue;
}

internal cBool_t cPng_LoadSuggestedPaletteChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   uint16_t i;
   uint16_t* filePos16;
   cBool_t foundName = cFalse;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( chunkSize < 9 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_SPLTCORRUPT );
   }

   TURN_ON_FLAG( PNG_FLAG_HASSUGGESTEDPALETTE );

   for ( i = 0; i < PNG_MAX_NAMELENGTH; i++ )
   {
      if ( i == chunkSize - 1 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_SPLTCORRUPT );
      }

      pngData->suggestedPalette.name[i] = filePos[0];
      filePos++;

      if ( pngData->suggestedPalette.name[i] == '\0' )
      {
         foundName = cTrue;
         break;
      }
   }

   if ( i >= chunkSize - 8 )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_SPLTCORRUPT );
   }

   pngData->suggestedPalette.sampleDepth = filePos[0];
   filePos++;
   i += 2;

   if ( pngData->suggestedPalette.sampleDepth == 8 )
   {
      if ( ( ( chunkSize - i ) % 6 ) != 0 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_SPLTCORRUPT );
      }

      pngData->suggestedPalette.numColors = (uint16_t)( ( chunkSize - i ) / 6 );

      if ( pngData->suggestedPalette.numColors > 256 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_SPLTTOOMANYCOLORS );
      }

      pngData->suggestedPalette.colors = (cPngSuggestedPaletteColor_t*)Platform_MemAlloc( sizeof( cPngSuggestedPaletteColor_t ) * pngData->suggestedPalette.numColors );
      TURN_ON_FLAG( PNG_FLAG_ALLOCATEDSUGGESTEDPALETTE );

      for ( i = 0; i < pngData->suggestedPalette.numColors; i++ )
      {
         pngData->suggestedPalette.colors[i].color = ( (uint32_t)( filePos[3] ) << 24 ) |
                                                     ( (uint32_t)( filePos[0] ) << 16 ) |
                                                     ( (uint32_t)( filePos[1] ) << 8 ) |
                                                     (uint32_t)( filePos[2] );
         filePos += 4;
         pngData->suggestedPalette.colors[i].frequency = ( (uint16_t*)filePos )[0];
         filePos += 2;
      }
   }
   else if ( pngData->suggestedPalette.sampleDepth == 16 )
   {
      if ( ( ( chunkSize - i ) % 10 ) != 0 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_SPLTCORRUPT );
      }

      pngData->suggestedPalette.numColors = (uint16_t)( ( chunkSize - i ) / 10 );

      if ( pngData->suggestedPalette.numColors > 256 )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_SPLTTOOMANYCOLORS );
      }

      pngData->suggestedPalette.colors = (cPngSuggestedPaletteColor_t*)Platform_MemAlloc( sizeof( cPngSuggestedPaletteColor_t ) * pngData->suggestedPalette.numColors );
      TURN_ON_FLAG( PNG_FLAG_ALLOCATEDSUGGESTEDPALETTE );
      filePos16 = (uint16_t*)filePos;

      for ( i = 0; i < pngData->suggestedPalette.numColors; i++ )
      {
         pngData->suggestedPalette.colors[i].color = ( (uint32_t)( filePos16[3] ) << 24 ) |
                                                     ( (uint32_t)( filePos16[0] ) << 16 ) |
                                                     ( (uint32_t)( filePos16[1] ) << 8 ) |
                                                     (uint32_t)( filePos16[2] );
         filePos16 += 4;
         pngData->suggestedPalette.colors[i].frequency = filePos16[0];
         filePos16++;
      }
   }
   else
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_SPLTCORRUPT );
   }

   return cTrue;
}

internal cBool_t cPng_ConsolidateImageData( const char* filePath, cPngData_t* pngData, uint32_t numImageDataSections, cPngImageDataSection_t* imageDataSections )
{
   uint32_t i = 0, j;
   cPngImageDataSection_t* section;
   cPngImageDataSection_t* nextSection;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( !HAS_FLAG( PNG_FLAG_ALLOCATEDDATASECTIONS ) )
   {
      ERROR_RETURN_FALSE( STR_PNGERROR_FILECORRUPT );
   }

   section = imageDataSections;

   do
   {
      pngData->imageData.size += section->size;
      section = section->nextSection;
   }
   while( section );

   pngData->imageData.data = (uint8_t*)Platform_MemAlloc( pngData->imageData.size );
   TURN_ON_FLAG( PNG_FLAG_ALLOCATEDIMAGEDATA );
   section = imageDataSections;

   do
   {
      for ( j = 0; j < section->size; i++, j++ )
      {
         pngData->imageData.data[i] = section->data[j];
      }

      section = section->nextSection;
   }
   while( section );

   section = imageDataSections;

   for ( j = 0; j < numImageDataSections; j++ )
   {
      // I hate this check, but Visual Studio whines if we don't do it.
      if ( !section || !section->data )
      {
         ERROR_RETURN_FALSE( STR_PNGERROR_FILECORRUPT );
      }

      Platform_MemFree( section->data );
      nextSection = section->nextSection;
      Platform_MemFree( section );
      section = nextSection;
   }

   TURN_OFF_FLAG( PNG_FLAG_ALLOCATEDDATASECTIONS );

   return cTrue;
}
