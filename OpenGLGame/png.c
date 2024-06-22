#include "png.h"

#define EXTRACT_CHUNK_DATA_32( x ) 0 | ( x[0] << 24 ) | ( x[1] << 16 ) | ( x[2] << 8 ) | x[3]

internal void cPng_LoadHeader( uint8_t* chunkPosition, cPngData_t* pngData );

cBool_t cPng_LoadImageData( cFileData_t* fileData, cPngData_t* pngData )
{
   uint8_t* chunkPosition;
   uint32_t chunkSize, chunkType;

   if ( fileData->size < 8 )
   {
      // TODO: logging, the file is too small
      return cFalse;
   }

   if ( ( (uint64_t*)( fileData->contents ) )[0] != PNG_SIGNATURE )
   {
      // TODO: logging, this isn't a PNG file
      return cFalse;
   }

   chunkPosition = (uint8_t*)( fileData->contents ) + 8;
   chunkSize = EXTRACT_CHUNK_DATA_32( chunkPosition );
   chunkPosition += 4;
   chunkType = EXTRACT_CHUNK_DATA_32( chunkPosition );
   chunkPosition += 4;

   if ( chunkSize != 13 || chunkType != PNG_CHUNKTYPE_IHDR )
   {
      // TODO: logging, this means the IHDR chunk is missing or incorrect
      return cFalse;
   }

   cPng_LoadHeader( chunkPosition, pngData );

   return cTrue;
}

internal void cPng_LoadHeader( uint8_t* chunkPosition, cPngData_t* pngData )
{
   pngData->width = EXTRACT_CHUNK_DATA_32( chunkPosition );
   chunkPosition += 4;
   pngData->height = EXTRACT_CHUNK_DATA_32( chunkPosition );
   chunkPosition += 4;
   pngData->bitDepth = chunkPosition[0];
   chunkPosition++;
   pngData->colorType = chunkPosition[0];
   chunkPosition++;
   pngData->compressionMethod = chunkPosition[0];
   chunkPosition++;
   pngData->filterMethod = chunkPosition[0];
   chunkPosition++;
   pngData->interlaceMethod = chunkPosition[0];
   chunkPosition++;
}
