#include "bmp.h"
#include "platform.h"

#define ERROR_RETURN_FALSE( s ) \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, s, fileData->filePath ); \
   Platform_Log( errorMsg ); \
   return cFalse

internal cBool_t cBmp_ReadHeader( cBmpData_t* bmpData, cFileData_t* fileData, uint8_t* filePos );
internal cBool_t cBmp_ReadDIBHeader( cBmpData_t* bmpData, cFileData_t* fileData, uint8_t* filePos );

cBool_t cBmp_LoadFromFile( cBmpData_t* bmpData, const char* filePath )
{
   cFileData_t fileData;
   uint8_t* filePos;

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

   // TODO: read the rest of the data

   Platform_ClearFileData( &fileData );
   return cTrue;
}

internal cBool_t cBmp_ReadHeader( cBmpData_t* bmpData, cFileData_t* fileData, uint8_t* filePos )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( fileData->fileSize < BMP_HEADER_SIZE )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   // first 2 bytes are the type, we currenly only support "BM"
   if ( ( (uint16_t*)filePos )[0] != 0x4D42 )
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
   if ( ( (uint32_t*)filePos )[0] != BMP_DIBHEADER_SIZE )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDDIBHEADERTYPE );
   }

   if ( fileData->fileSize < BMP_HEADER_SIZE + BMP_DIBHEADER_SIZE )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   filePos += 4;

   // next 8 bytes are the width and height, respectively
   bmpData->width = ( (int32_t*)filePos )[0];
   bmpData->height = ( (int32_t*)filePos )[1];
   filePos += 8;

   // next 2 bytes are the number of color planes (must be 1)
   if ( ( (uint16_t*)filePos )[0] != 1 )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDCOLORPLANES );
   }

   filePos += 2;

   // next 2 bytes are the number of bits per pixel
   bmpData->bitsPerPixel = ( (uint16_t*) filePos )[0];

   if ( bmpData->bitsPerPixel != 1 && bmpData->bitsPerPixel != 4 && bmpData->bitsPerPixel != 8 &&
        bmpData->bitsPerPixel != 16 && bmpData->bitsPerPixel != 24 && bmpData->bitsPerPixel != 32 )
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

   filePos += 4;

   // next 8 bytes are the horizontal and vertical resolution, respectively (pixels per metre)
   bmpData->hResolution = ( (int32_t*)filePos )[0];
   bmpData->vResolution = ( (int32_t*)filePos )[1];
   filePos += 8;

   // next 4 bytes are the number of colors in the palette (can be zero)
   bmpData->numPaletteColors = ( (uint32_t*)filePos )[0];
   filePos += 4;

   // last 4 bytes are the number of important colors (0 if every color is important)
   bmpData->importantColors = ( (uint32_t*)filePos )[0];

   return cTrue;
}
