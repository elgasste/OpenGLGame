#include <math.h>

#include "png.h"
#include "platform.h"

#define EXTRACT_CHUNK_DATA_32( x ) 0 | ( x[0] << 24 ) | ( x[1] << 16 ) | ( x[2] << 8 ) | x[3]

internal void cPng_InitData( cPngData_t* pngData );
internal void cPng_LogCorruptFile( const char* filePath );
internal cBool_t cPng_LoadHeader( uint8_t* filePos, cPngData_t* pngData, const char* filePath );
internal cBool_t cPng_LoadPaletteChunk( uint8_t* filePos, uint32_t chunkSize, const char* filePath, cPngData_t* pngData );
internal cBool_t cPng_LoadImageDataChunk();
internal cBool_t cPng_LoadSRGBChunk();
internal cBool_t cPng_LoadGammaChunk();
internal cBool_t cPng_LoadChromaticitiesChunk();
internal cBool_t cPng_LoadICCProfileChunk();
internal cBool_t cPng_LoadSignificantBitsChunk();
internal cBool_t cPng_LoadTransparencyChunk();
internal cBool_t cPng_LoadBackgroundColorChunk();
internal cBool_t cPng_LoadPaletteHistogramChunk();
internal cBool_t cPng_LoadPhysicalPixelDimensionsChunk();
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
   cBool_t foundPaletteHistogram = cFalse;
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
               foundSRGB = cTrue;
               stillGood = cPng_LoadSRGBChunk();
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
               stillGood = cPng_LoadGammaChunk();
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
               stillGood = cPng_LoadChromaticitiesChunk();
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
               stillGood = cPng_LoadICCProfileChunk();
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
               stillGood = cPng_LoadSignificantBitsChunk();
            }
            break;
         case PNG_CHUNKTYPE_TRNS:
            if ( ( pngData->header.colorType == 3 && !pngData->hasPalette ) || foundImageData || foundTransparency )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundTransparency = cTrue;
               stillGood = cPng_LoadTransparencyChunk();
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
               stillGood = cPng_LoadBackgroundColorChunk();
            }
            break;
         case PNG_CHUNKTYPE_HIST:
            if ( !pngData->hasPalette || foundImageData || foundPaletteHistogram )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundPaletteHistogram = cTrue;
               stillGood = cPng_LoadPaletteHistogramChunk();
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
               stillGood = cPng_LoadPhysicalPixelDimensionsChunk();
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

   pngData->hasPalette = cFalse;
   pngData->palette.numColors = 0;
   pngData->palette.colors = 0;
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
   uint16_t maxColors;
   uint32_t r, g, b;
   uint16_t i;
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

   maxColors = (uint16_t)pow( (double)2, (double)( pngData->header.bitDepth ) );
   pngData->palette.numColors = (uint16_t)( chunkSize / 3 ) / pngData->header.bitDepth;

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

internal cBool_t cPng_LoadSRGBChunk()
{
   // TODO
   return cTrue;
}

internal cBool_t cPng_LoadGammaChunk()
{
   // TODO
   return cTrue;
}

internal cBool_t cPng_LoadChromaticitiesChunk()
{
   // TODO
   return cTrue;
}

internal cBool_t cPng_LoadICCProfileChunk()
{
   // TODO
   return cTrue;
}

internal cBool_t cPng_LoadSignificantBitsChunk()
{
   // TODO
   return cTrue;
}

internal cBool_t cPng_LoadTransparencyChunk()
{
   // TODO
   return cTrue;
}

internal cBool_t cPng_LoadBackgroundColorChunk()
{
   // TODO
   return cTrue;
}

internal cBool_t cPng_LoadPaletteHistogramChunk()
{
   // TODO
   return cTrue;
}

internal cBool_t cPng_LoadPhysicalPixelDimensionsChunk()
{
   // TODO
   return cTrue;
}

internal cBool_t cPng_LoadSuggestedPaletteChunk()
{
   // TODO (there could be multiple of these)
   return cTrue;
}
