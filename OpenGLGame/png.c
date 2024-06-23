#include "png.h"
#include "platform.h"

#define EXTRACT_CHUNK_DATA_32( x ) 0 | ( x[0] << 24 ) | ( x[1] << 16 ) | ( x[2] << 8 ) | x[3]

internal void cPng_LogCorruptFile( const char* filePath );
internal cBool_t cPng_LoadHeader( uint8_t* filePos, cPngData_t* pngData, const char* filePath );
internal cBool_t cPng_LoadPaletteChunk();
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

cBool_t cPng_LoadImageData( cFileData_t* fileData, cPngData_t* pngData )
{
   uint8_t* filePos;
   uint32_t chunkSize, chunkType, bytesRead;
   char errorMsg[STRING_SIZE_DEFAULT];
   cBool_t foundPalette = cFalse;
   cBool_t foundData = cFalse;
   cBool_t stillGood = cTrue;

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
         bytesRead += 4;
         break;
      }

      if ( fileData->size < ( bytesRead + chunkSize ) )
      {
         cPng_LogCorruptFile( fileData->filePath );
         return cFalse;
      }

      switch ( chunkType )
      {
         case PNG_CHUNKTYPE_PLTE:
            if ( foundPalette || foundData )
            {
               snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_ROGUEPALETTE, fileData->filePath );
               Platform_Log( errorMsg );
               stillGood = cFalse;
            }
            else
            {
               foundPalette = cTrue;
               stillGood = cPng_LoadPaletteChunk();
            }
            break;
         case PNG_CHUNKTYPE_IDAT:
            if ( foundData )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               foundData = cTrue;
               stillGood = cPng_LoadImageDataChunk();
            }
            break;
         case PNG_CHUNKTYPE_SRGB:
            if ( foundPalette )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadSRGBChunk();
            }
            break;
         case PNG_CHUNKTYPE_GAMA:
            if ( foundPalette )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadGammaChunk();
            }
            break;
         case PNG_CHUNKTYPE_CHRM:
            if ( foundPalette )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadChromaticitiesChunk();
            }
            break;
         case PNG_CHUNKTYPE_ICCP:
            if ( foundPalette )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadICCProfileChunk();
            }
            break;
         case PNG_CHUNKTYPE_SBIT:
            if ( foundPalette )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadSignificantBitsChunk();
            }
            break;
         case PNG_CHUNKTYPE_TRNS:
            if ( ( pngData->headerData.colorType == 3 && !foundPalette ) || foundData )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadTransparencyChunk();
            }
            break;
         case PNG_CHUNKTYPE_BKGD:
            if ( ( pngData->headerData.colorType == 3 && !foundPalette ) || foundData )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadBackgroundColorChunk();
            }
            break;
         case PNG_CHUNKTYPE_HIST:
            if ( !foundPalette || foundData )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadPaletteHistogramChunk();
            }
            break;
         case PNG_CHUNKTYPE_PHYS:
            if ( foundData )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadPhysicalPixelDimensionsChunk();
            }
            break;
         case PNG_CHUNKTYPE_SPLT:
            if ( foundData )
            {
               cPng_LogCorruptFile( fileData->filePath );
               stillGood = cFalse;
            }
            else
            {
               stillGood = cPng_LoadSuggestedPaletteChunk();
            }
            break;
         // TODO: I don't think we care about these, should we keep them here?
         case PNG_CHUNKTYPE_TIME:
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

   if ( bytesRead != fileData->size || !foundData )
   {
      cPng_LogCorruptFile( fileData->filePath );
      return cFalse;
   }
   else
   {
      return cTrue;
   }
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
   cPngHeaderData_t* header = &( pngData->headerData );

   header->width = EXTRACT_CHUNK_DATA_32( filePos );
   filePos += 4;
   header->height = EXTRACT_CHUNK_DATA_32( filePos );
   filePos += 4;
   header->bitDepth = filePos[0];           // bits per sample, or per palette index (not per pixel)
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
   header->colorType = filePos[0];          // sums of the following:
                                             // 1 - palette used
                                             // 2 - color used,
                                             // 4 - alpha channel used
                                             // possible values are 0, 2, 3, 4, and 6 (see above)
   filePos++;
   header->compressionMethod = filePos[0];  // only 0 (deflate/inflate compression with a 32K sliding window) is defined
   filePos++;
   header->filterMethod = filePos[0];       // only 0 (adaptive filtering with five basic filter types) is defined
   filePos++;
   header->interlaceMethod = filePos[0];    // 0 (no interlace) or 1 (Adam7 interlace)
   filePos++;

   if ( header->colorType == 0 )
   {
      if ( header->bitDepth != 1 && header->bitDepth != 2 && header->bitDepth != 4 && header->bitDepth != 8 && header->bitDepth != 16 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_COLORTYPEMISMATCH, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }
   }
   else if ( header->colorType == 2 || header->colorType == 4 || header->colorType == 6 )
   {
      if ( header->bitDepth != 8 && header->bitDepth != 16 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_COLORTYPEMISMATCH, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }
   }
   else if ( header->colorType == 3 )
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

internal cBool_t cPng_LoadPaletteChunk()
{
   // TODO
   return cTrue;
}

internal cBool_t cPng_LoadImageDataChunk()
{
   // TODO
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
   // TODO
   return cTrue;
}
