#include <math.h>

#include "bmp.h"
#include "platform.h"

#define ERROR_RETURN_FALSE( s ) \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, s, fileData->filePath ); \
   Platform_Log( errorMsg ); \
   return cFalse

internal cBool_t cBmp_ReadHeader( cBmpData_t* bmpData, cFileData_t* fileData, uint8_t* filePos );
internal cBool_t cBmp_ReadDIBHeader( cBmpData_t* bmpData, cFileData_t* fileData, uint8_t* filePos );
internal cBool_t cBmp_ReadPalette( cBmpData_t* bmpData, cFileData_t* fileData, uint8_t* filePos );

cBool_t cBmp_LoadFromFile( cBmpData_t* bmpData, const char* filePath )
{
   cFileData_t fileData;
   uint8_t* filePos;

   bmpData->paletteColors = 0;

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      return cFalse;
   }

   filePos = (uint8_t*)fileData.contents;

   if ( !cBmp_ReadHeader( bmpData, &fileData, filePos ) )
   {
      return cFalse;
   }

   filePos += BMP_HEADER_SIZE;

   if ( !cBmp_ReadDIBHeader( bmpData, &fileData, filePos ) )
   {
      return cFalse;
   }

   filePos += bmpData->dibHeaderSize;

   if ( !cBmp_ReadPalette( bmpData, &fileData, filePos ) )
   {
      cBmp_Cleanup( bmpData );
      return cFalse;
   }

   // TODO:
   //
   // - calculate the stride and padding bytes
   // - verify the file size matches what we'd expect from the DIB header
   // - read the image data into a pixel buffer in our 0xAARRGGBB format

   Platform_ClearFileData( &fileData );
   return cTrue;
}

void cBmp_Cleanup( cBmpData_t* bmpData )
{
   if ( bmpData->paletteColors )
   {
      Platform_MemFree( bmpData->paletteColors );
      bmpData->paletteColors = 0;
   }
}

internal cBool_t cBmp_ReadHeader( cBmpData_t* bmpData, cFileData_t* fileData, uint8_t* filePos )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( fileData->fileSize < BMP_HEADER_SIZE )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   // first 2 bytes are the type, we currenly only support "BM"
   if ( ( (uint16_t*)filePos )[0] != BMP_HEADER_TYPE )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDHEADERTYPE );
   }

   filePos += 2;

   // next 4 bytes are the file size
   if ( ( (uint32_t*)filePos )[0] != fileData->fileSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_HEADERCORRUPT );
   }

   filePos += 8;

   // the next 4 bytes after the file size are reserved, and the last
   // 4 bytes are the image data offset
   bmpData->imageOffset = ( (uint32_t*)filePos )[0];

   if ( bmpData->imageOffset >= fileData->fileSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   return cTrue;
}

internal cBool_t cBmp_ReadDIBHeader( cBmpData_t* bmpData, cFileData_t* fileData, uint8_t* filePos )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( fileData->fileSize < ( BMP_HEADER_SIZE + 4 ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   // first 4 bytes are the header size
   // TODO: 32-bit headers are coming in at 124 bytes, which is a BITMAPV5HEADER.
   // we'll want to support alpha values though, so figure that one out.
   bmpData->dibHeaderSize = ( (uint32_t*)filePos )[0];

   if ( bmpData->dibHeaderSize != BMP_BITMAPINFOHEADER_SIZE )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDDIBHEADERTYPE );
   }

   if ( fileData->fileSize < BMP_HEADER_SIZE + bmpData->dibHeaderSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   filePos += 4;

   // next 8 bytes are the width and height, respectively (height can be negative)
   bmpData->width = ( (int32_t*)filePos )[0];
   bmpData->height = ( (int32_t*)filePos )[1];

   if ( bmpData->width <= 0 || bmpData->height == 0 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDIMAGESIZE );
   }

   filePos += 8;

   // next 2 bytes are the number of color planes (must be 1)
   if ( ( (uint16_t*)filePos )[0] != 1 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDCOLORPLANES );
   }

   filePos += 2;

   // next 2 bytes are the number of bits per pixel
   bmpData->bitsPerPixel = ( (uint16_t*) filePos )[0];

   if ( bmpData->bitsPerPixel != 1 && bmpData->bitsPerPixel != 4 && bmpData->bitsPerPixel != 8 && bmpData->bitsPerPixel != 24 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDBPP );
   }

   filePos += 2;

   // next 4 bytes are the compression method, we only support uncompressed images for now
   if ( ( (uint32_t*)filePos )[0] != 0 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_ISCOMPRESSED );
   }

   filePos += 4;

   // next 4 bytes are the size of the raw image data (should be 0 for BI_RGB bitmaps)
   bmpData->imageBytes = ( (uint32_t*)filePos )[0];

   if ( bmpData->imageBytes > 0 && ( bmpData->imageOffset + bmpData->imageBytes ) > fileData->fileSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   filePos += 12;

   // next 8 bytes after the image size are the horizontal and vertical resolution,
   // which we don't care about. next 4 bytes after that are the number of palette
   // colors. for BPP values less than 24, this should be 2^BPP.
   bmpData->numPaletteColors = ( (uint32_t*)filePos )[0];

   if ( ( bmpData->bitsPerPixel == 24 && bmpData->numPaletteColors != 0 ) ||
        bmpData->numPaletteColors > (uint32_t)pow( 2, (double)( bmpData->bitsPerPixel ) ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDPALETTECOUNT );
   }

   return cTrue;
}

internal cBool_t cBmp_ReadPalette( cBmpData_t* bmpData, cFileData_t* fileData, uint8_t* filePos )
{
   uint32_t i;
   uint32_t paletteSize;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( bmpData->numPaletteColors == 0 )
   {
      return cTrue;
   }

   paletteSize = bmpData->imageOffset - ( BMP_HEADER_SIZE + bmpData->dibHeaderSize );
   
   if ( paletteSize != ( (uint32_t)( bmpData->numPaletteColors * 4 ) ) )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_PALETTECORRUPT );
   }
   else if ( ( BMP_HEADER_SIZE + bmpData->dibHeaderSize + paletteSize ) > fileData->fileSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   // colors are 4 bytes in RGBA format
   bmpData->paletteColors = (uint32_t*)Platform_MemAlloc( bmpData->numPaletteColors * 4 );

   for ( i = 0; i < bmpData->numPaletteColors; i++ )
   {
      // convert RGBA to our ARGB format
      bmpData->paletteColors[i] = 0 |
                                  ( (uint32_t)( filePos[3] ) << 24 ) |
                                  ( (uint32_t)( filePos[0] ) << 16 ) |
                                  ( (uint32_t)( filePos[1] ) << 8 ) |
                                  (uint32_t)( filePos[2] );
      filePos += 4;
   }

   return cTrue;
}
