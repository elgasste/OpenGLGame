#include "bmp.h"
#include "platform.h"

#define ERROR_RETURN_FALSE( s ) \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, s, fileData->filePath ); \
   Platform_Log( errorMsg ); \
   return cFalse

internal cBool_t cBmp_ReadHeader( cBmpData_t* bmpData, cFileData_t* fileData, uint8_t* filePos );

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
      ERROR_RETURN_FALSE( STR_BMPERR_INVALIDTYPE );
   }

   filePos += 2;

   // next 4 bytes are the file size
   if ( ( (uint32_t*)filePos )[0] != fileData->fileSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_HEADERCORRUPT );
   }

   // the next 4 bytes after the file size are reserved, and the last
   // 4 bytes are the image data offset
   filePos += 8;
   bmpData->imageOffset = ( (uint32_t*)filePos )[0];

   if ( bmpData->imageOffset >= fileData->fileSize )
   {
      ERROR_RETURN_FALSE( STR_BMPERR_FILECORRUPT );
   }

   return cTrue;
}
