#include <math.h>

#include "png.h"
#include "platform.h"

#define EXTRACT_CHUNK_DATA_32( x ) 0 | ( x[0] << 24 ) | ( x[1] << 16 ) | ( x[2] << 8 ) | x[3]

internal void cPng_InitData( cPngData_t* pngData );
internal void cPng_LogCorruptFile( const char* filePath );
internal cBool_t cPng_LoadHeader( uint8_t* filePos, cPngData_t* pngData, const char* filePath );
internal cBool_t cPng_LoadPaletteChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadImageDataChunk();
internal cBool_t cPng_LoadSRGBChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadGammaChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadChromaticitiesChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadICCProfileChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadSignificantBitsChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadTransparencyChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadBackgroundColorChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadPhysicalPixelDimensionsChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadSuggestedPaletteChunk();

cBool_t cPng_LoadPngData( cFileData_t* fileData, cPngData_t* pngData )
{
   uint8_t* filePos;
   uint32_t chunkSize, chunkType, bytesRead;
   char errorMsg[STRING_SIZE_DEFAULT];
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
   cBool_t foundTime = cFalse;
   cBool_t stillGood = cTrue;

   cPng_InitData( pngData );

   // the file signature plus header data is 29 bytes
   if ( fileData->size < 29 )
   {
      cPng_LogCorruptFile( fileData->filePath );
      return cFalse;
   }

   if ( ( (uint64_t*)( fileData->contents ) )[0] != PNG_SIGNATURE )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_NOTAPNGFILE, fileData->filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   filePos = (uint8_t*)( fileData->contents ) + 8;

   chunkSize = EXTRACT_CHUNK_DATA_32( filePos );
   filePos += 4;
   chunkType = EXTRACT_CHUNK_DATA_32( filePos );
   filePos += 4;

   if ( chunkSize != 13 || chunkType != PNG_CHUNKTYPE_IHDR )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_HEADERNOTFOUND, fileData->filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   if ( !cPng_LoadHeader( filePos, pngData, fileData->filePath ) )
   {
      return cFalse;
   }

   filePos += 17;
   bytesRead = 33;

   while ( 1 )
   {
      if ( fileData->size < ( bytesRead + 8 ) )
      {
         cPng_LogCorruptFile( fileData->filePath );
         return cFalse;
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
         cPng_LogCorruptFile( fileData->filePath );
         return cFalse;
      }

      switch ( chunkType )
      {
         case PNG_CHUNKTYPE_IHDR:
            cPng_LogCorruptFile( fileData->filePath );
            stillGood = cFalse;
            break;
         case PNG_CHUNKTYPE_PLTE:
            if ( pngData->hasPalette || foundImageData )
            {
               snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_ROGUEPALETTE, fileData->filePath );
               Platform_Log( errorMsg );
               stillGood = cFalse;
            }
            else
            {
               pngData->hasPalette = cTrue;
               stillGood = cPng_LoadPaletteChunk( filePos, chunkSize, fileData->filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_IDAT:
            if ( finishedReadingImageData )
            {
               snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_ROGUEIDAT, fileData->filePath );
               Platform_Log( errorMsg );
               stillGood = cFalse;
            }
            else
            {
               foundImageData = cTrue;
               readingImageData = cTrue;
               stillGood = cPng_LoadImageDataChunk();
            }
            break;
         case PNG_CHUNKTYPE_SRGB:
            if ( pngData->hasPalette || foundSRGB )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               // NOTE: if sRGB is present, it should override cHRM
               foundSRGB = cTrue;
               stillGood = cPng_LoadSRGBChunk( filePos, chunkSize, fileData->filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_GAMA:
            if ( pngData->hasPalette || foundGamma )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundGamma = cTrue;
               stillGood = cPng_LoadGammaChunk( filePos, chunkSize, fileData->filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_CHRM:
            if ( pngData->hasPalette || foundChromaticities )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundChromaticities = cTrue;
               stillGood = cPng_LoadChromaticitiesChunk( filePos, chunkSize, fileData->filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_ICCP:
            if ( pngData->hasPalette || foundICCProfile )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundICCProfile = cTrue;
               stillGood = cPng_LoadICCProfileChunk( filePos, chunkSize, fileData->filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_SBIT:
            if ( pngData->hasPalette || foundSignificantBits )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundSignificantBits = cTrue;
               stillGood = cPng_LoadSignificantBitsChunk( filePos, chunkSize, fileData->filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_TRNS:
            if ( ( pngData->header.colorType == PNG_COLORTYPE_INDEXED && !pngData->hasPalette ) ||
                 ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALEALPHA ) ||
                 ( pngData->header.colorType == PNG_COLORTYPE_TRUECOLORALPHA ) ||
                 foundImageData ||
                 foundTransparency )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundTransparency = cTrue;
               stillGood = cPng_LoadTransparencyChunk( filePos, chunkSize, fileData->filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_BKGD:
            if ( ( pngData->header.colorType == 3 && !pngData->hasPalette ) || foundImageData || foundBackgroundColor )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundBackgroundColor = cTrue;
               stillGood = cPng_LoadBackgroundColorChunk( filePos, chunkSize, fileData->filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_PHYS:
            if ( foundImageData || foundPhysicalPixelDimensions )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundPhysicalPixelDimensions = cTrue;
               stillGood = cPng_LoadPhysicalPixelDimensionsChunk( filePos, chunkSize, fileData->filePath, pngData );
            }
            break;
         case PNG_CHUNKTYPE_SPLT:
            if ( foundImageData )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadSuggestedPaletteChunk();
            }
            break;
         case PNG_CHUNKTYPE_TIME:
            if ( foundTime )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundTime = cTrue;
            }
            break;
         // TODO: I don't think we care about these, should we keep them here?
         case PNG_CHUNKTYPE_ITXT:
         case PNG_CHUNKTYPE_TEXT:
         case PNG_CHUNKTYPE_ZTXT:
         case PNG_CHUNKTYPE_HIST:
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
      cPng_LogCorruptFile( fileData->filePath );
      return cFalse;
   }
   else
   {
      return cTrue;
   }
}

void cPng_DeletePngData( cPngData_t* pngData )
{
   if ( pngData->hasPalette )
   {
      Platform_MemFree( pngData->palette.colors );
   }
   if ( pngData->hasICCProfile )
   {
      Platform_MemFree( pngData->ICCProfile.compressedProfile );
   }
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

   pngData->hasPalette = cFalse;
   pngData->hasTrnsGrayLevel = cFalse;
   pngData->hasTrnsColor = cFalse;
   pngData->hasGammaCorrection = cFalse;
   pngData->hasSignificantBits = cFalse;
   pngData->hasSRGB = cFalse;
   pngData->hasChromaticity = cFalse;
   pngData->hasICCProfile = cFalse;
   pngData->hasBackgroundColor = cFalse;
   pngData->hasPhysPixelDimensions = cFalse;
}

internal void cPng_LogCorruptFile( const char* filePath )
{
   char errorMsg[STRING_SIZE_DEFAULT];
   snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_FILECORRUPT, filePath );
   Platform_Log( errorMsg );
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
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_COLORTYPEMISMATCH, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }
   }
   else if ( header->colorType == PNG_COLORTYPE_TRUECOLOR ||
             header->colorType == PNG_COLORTYPE_GRAYSCALEALPHA ||
             header->colorType == PNG_COLORTYPE_TRUECOLORALPHA )
   {
      if ( header->bitDepth != 8 && header->bitDepth != 16 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_COLORTYPEMISMATCH, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }
   }
   else if ( header->colorType == PNG_COLORTYPE_INDEXED )
   {
      if ( header->bitDepth != 1 && header->bitDepth != 2 && header->bitDepth != 4 && header->bitDepth != 8 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_COLORTYPEMISMATCH, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }
   }
   else
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_INVALIDCOLORTYPE, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   if ( header->compressionMethod != 0 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_INVALIDCOMPRESSION, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   if ( header->filterMethod != 0 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_INVALIDFILTER, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   if ( header->interlaceMethod > 1 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_INVALIDINTERLACE, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   return cTrue;
}

internal cBool_t cPng_LoadPaletteChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   uint32_t maxColors, r, g, b, i;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( ( chunkSize % 3 ) != 0 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_INVALIDPALETTESIZE, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALE ||
             pngData->header.colorType == PNG_COLORTYPE_GRAYSCALEALPHA )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_ROGUEPALETTE, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   if ( pngData->header.bitDepth > 8 )
   {
      maxColors = 256;
   }
   else
   {
      maxColors = (uint32_t)pow( (double)2, (double)( pngData->header.bitDepth ) );
   }

   pngData->palette.numColors = ( chunkSize / 3 ) / pngData->header.bitDepth;

   if ( pngData->palette.numColors > maxColors )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_PALETTEENTRYOVERRUN, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   pngData->palette.colors = (uint32_t*)Platform_MemAlloc( sizeof( uint32_t ) * pngData->palette.numColors );

   for ( i = 0; i < pngData->palette.numColors; i++ )
   {
      r = filePos[0];
      g = filePos[1];
      b = filePos[2];
      pngData->palette.colors[i] = 0xFF000000 | ( r << 16 ) | ( g << 8 ) | b;
      filePos += 3;
   }

   return cTrue;
}

internal cBool_t cPng_LoadImageDataChunk()
{
   // TODO (there could be multiple of these)
   return cTrue;
}

internal cBool_t cPng_LoadSRGBChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( chunkSize != 1 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_SRGBCORRUPT, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }
   else if ( filePos[0] > PNG_SRGB_ABSOLUTECOLORIMETRIC )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_SRGBINVALID, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   pngData->hasSRGB = cTrue;
   pngData->sRGB = filePos[0];

   return cTrue;
}

internal cBool_t cPng_LoadGammaChunk(  uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( chunkSize != 4 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_GAMMACORRUPT, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   pngData->hasGammaCorrection = cTrue;
   pngData->gammaCorrection = (float)( ( (uint32_t*)filePos )[0] ) / 100000;

   return cTrue;
}

internal cBool_t cPng_LoadChromaticitiesChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData )
{
   uint32_t* filePos32 = (uint32_t*)filePos;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( chunkSize != sizeof( cPngChromaticity_t ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_CHRMCORRUPT, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   pngData->hasChromaticity = cTrue;
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
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_ICCCORRUPT, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   pngData->hasICCProfile = cTrue;

   for ( i = 0; i < PNG_ICC_MAXNAMELENGTH; i++ )
   {
      if ( i == chunkSize - 1 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_ICCCORRUPT, filePath );
         Platform_Log( errorMsg );
         return cFalse;
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
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_ICCCORRUPT, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }
   else if ( filePos[0] != 0 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_ICCCOMPRESSIONINVALID, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   filePos++;
   pngData->ICCProfile.compressedProfileSize = chunkSize - ( i + 2 );
   pngData->ICCProfile.compressedProfile = (uint8_t*)Platform_MemAlloc( pngData->ICCProfile.compressedProfileSize );

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

   pngData->hasSignificantBits = cTrue;

   if ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALE )
   {
      if ( chunkSize != 1 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_SBITCORRUPT, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }

      pngData->significantBits = (uint32_t)( filePos[0] );
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_TRUECOLOR || pngData->header.colorType == PNG_COLORTYPE_INDEXED )
   {
      if ( chunkSize != 3 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_SBITCORRUPT, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }

      pngData->significantBits = 0 | ( (uint32_t)( filePos[0] ) << 16 ) | ( (uint32_t)( filePos[0] ) << 8 ) | (uint32_t)( filePos[0] );
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALEALPHA )
   {
      if ( chunkSize != 2 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_SBITCORRUPT, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }

      pngData->significantBits = (uint32_t)( ( (uint16_t*)filePos )[0] );
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_TRUECOLORALPHA )
   {
      if ( chunkSize != 4 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_SBITCORRUPT, filePath );
         Platform_Log( errorMsg );
         return cFalse;
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
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_TRNSOVERRUN, filePath );
         Platform_Log( errorMsg );
         return cFalse;
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
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_TRNSCORRUPT, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }

      pngData->hasTrnsGrayLevel = cTrue;
      pngData->trnsGrayLevel = ( (uint16_t*)filePos )[0] & ( 0xFFFF >> ( 16 - pngData->header.bitDepth ) );
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_TRUECOLOR )
   {
      // there should be 6 bytes that contain a single RGB color value, in the order
      // of R, G, B. if the bit depth is less than 16, the least significant bits are
      // used and the others are presumed to be zero. any pixels of this color are
      // to be considered transparent, and all others are to be considered opaque.
      if ( chunkSize != 6 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_TRNSCORRUPT, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }

      // TODO: truncate these for bit depths less than 16?
      pngData->hasTrnsColor = cTrue;
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

   pngData->hasBackgroundColor = cTrue;

   if ( pngData->header.colorType == PNG_COLORTYPE_INDEXED )
   {
      if ( chunkSize != 1 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_BKGDCORRUPT, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }

      pngData->backgroundColor = (uint16_t)( filePos[0] );
   }
   else if ( pngData->header.colorType == PNG_COLORTYPE_GRAYSCALE ||
             pngData->header.colorType == PNG_COLORTYPE_GRAYSCALEALPHA )
   {
      if ( chunkSize != 2 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_BKGDCORRUPT, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }

      pngData->backgroundColor = 0xFF000000 | (uint32_t)( filePos[0] );
   }
   else
   {
      if ( chunkSize != 6 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_BKGDCORRUPT, filePath );
         Platform_Log( errorMsg );
         return cFalse;
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

   UNUSED_PARAM( filePos );
   UNUSED_PARAM( pngData );

   if ( chunkSize != 9 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_PHYSCORRUPT, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   pngData->hasPhysPixelDimensions = cTrue;
   pngData->physPixelDimensions.ppuX = ( (uint32_t*)filePos )[0];
   pngData->physPixelDimensions.ppuY = ( (uint32_t*)filePos )[1];
   pngData->physPixelDimensions.unitSpecifier = filePos[8];

   if ( pngData->physPixelDimensions.unitSpecifier > PNG_PHYSSPECIFIER_METER )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_INVALIDPHYSSPECIFIER, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   return cTrue;
}

internal cBool_t cPng_LoadSuggestedPaletteChunk()
{
   // TODO (there could be multiple of these)
   return cTrue;
}
