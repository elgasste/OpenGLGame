#include "png.h"
#include "platform.h"

#define EXTRACT_CHUNK_DATA_32( x ) 0 | ( x[0] << 24 ) | ( x[1] << 16 ) | ( x[2] << 8 ) | x[3]

internal cBool_t cPng_LoadHeader( uint8_t* filePos, cPngData_t* pngData, const char* filePath );

cBool_t cPng_LoadImageData( cFileData_t* fileData, cPngData_t* pngData )
{
   uint8_t* filePos;
   uint32_t chunkSize, chunkType, bytesRead;
   char errorMsg[STRING_SIZE_DEFAULT];
   cBool_t foundPalette = cFalse;
   cBool_t foundData = cFalse;

   // there should be at least enough space for the file signature and header
   if ( fileData->size < 29 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_FILECORRUPT, fileData->filePath );
      Platform_Log( errorMsg );
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
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_FILECORRUPT, fileData->filePath );
         Platform_Log( errorMsg );
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
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_FILECORRUPT, fileData->filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }

      switch ( chunkType )
      {
         case PNG_CHUNKTYPE_PLTE:
            if ( foundPalette || foundData )
            {
               snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_ROGUEPALETTE, fileData->filePath );
               Platform_Log( errorMsg );
               return cFalse;
            }
            else
            {
               // TODO: make sure we have the correct color type, then load the palette
               foundPalette = cTrue;
            }
            break;
         case PNG_CHUNKTYPE_IDAT:
            // TODO: load the image data (make sure we have a palette if we need one)
            break;
      }

      // every chunk contains a 4-byte CRC after the data
      filePos += ( chunkSize + 4 );
      bytesRead += ( chunkSize + 4 );
   }

   // TODO:
   // 
   // these chunks could appear anywhere at all after IHDR
   //
   // - tIME
   // - iTXt
   // - tEXt
   // - zTXt
   //
   // these chunks must appear before IDAT
   //
   // - pHYs
   // - sPLT
   //
   // these chunks must appear before PLTE
   // 
   // - sRGB
   // - gAMA
   // - cHRM
   // - iCCP
   // - sBIT
   //
   // these chunks must appear between PLTE and IDAT:
   //
   // - tRNS
   // - bKGD
   // - hIST
   //
   // it's also possible for there to be chunk types we don't recognize,
   // we should just be able to ignore them and use their size to skip them.

   if ( bytesRead != fileData->size )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_FILECORRUPT, fileData->filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }
   else
   {
      return cTrue;
   }
}

internal cBool_t cPng_LoadHeader( uint8_t* filePos, cPngData_t* pngData, const char* filePath )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   pngData->width = EXTRACT_CHUNK_DATA_32( filePos );
   filePos += 4;
   pngData->height = EXTRACT_CHUNK_DATA_32( filePos );
   filePos += 4;
   pngData->bitDepth = filePos[0];           // bits per sample, or per palette index (not per pixel)
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
   pngData->colorType = filePos[0];          // sums of the following:
                                             // 1 - palette used
                                             // 2 - color used,
                                             // 4 - alpha channel used
                                             // possible values are 0, 2, 3, 4, and 6 (see above)
   filePos++;
   pngData->compressionMethod = filePos[0];  // only 0 (deflate/inflate compression with a 32K sliding window) is defined
   filePos++;
   pngData->filterMethod = filePos[0];       // only 0 (adaptive filtering with five basic filter types) is defined
   filePos++;
   pngData->interlaceMethod = filePos[0];    // 0 (no interlace) or 1 (Adam7 interlace)
   filePos++;

   if ( pngData->colorType == 0 )
   {
      if ( pngData->bitDepth != 1 && pngData->bitDepth != 2 && pngData->bitDepth != 4 && pngData->bitDepth != 8 && pngData->bitDepth != 16 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_COLORTYPEMISMATCH, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }
   }
   else if ( pngData->colorType == 2 || pngData->colorType == 4 || pngData->colorType == 6 )
   {
      if ( pngData->bitDepth != 8 && pngData->bitDepth != 16 )
      {
         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_COLORTYPEMISMATCH, filePath );
         Platform_Log( errorMsg );
         return cFalse;
      }
   }
   else if ( pngData->colorType == 3 )
   {
      if ( pngData->bitDepth != 1 && pngData->bitDepth != 2 && pngData->bitDepth != 4 && pngData->bitDepth != 8 )
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

   if ( pngData->compressionMethod != 0 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_INVALIDCOMPRESSION, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   if ( pngData->filterMethod != 0 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_INVALIDFILTER, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   if ( pngData->interlaceMethod > 1 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_PNGERROR_INVALIDINTERLACE, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   return cTrue;
}
