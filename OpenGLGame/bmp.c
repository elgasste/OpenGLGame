#include "bmp.h"
#include "platform.h"

cBool_t cBmp_LoadFromFile( cBmpData_t* bmpData, const char* filePath )
{
   cFileData_t fileData;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_BMPERR_OPENFILEFAILED, filePath );
      Platform_Log( errorMsg );
      return cFalse;
   }

   // TODO: fill out the real data
   bmpData->width = 0;
   bmpData->height = 0;

   Platform_ClearFileData( &fileData );
   return cTrue;
}
