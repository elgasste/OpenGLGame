#include "font.h"
#include "platform.h"

Bool_t Font_LoadFromFile( Font_t* font, const char* filePath )
{
   FileData_t fileData;
   uint32_t* filePos32;
   uint32_t bufferSize, bytesRead, i, j;
   int32_t k;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      return False;
   }

   // first 8 bytes are the codepoint offset and number of glyphs, and we want to
   // make sure there's at least one glyph, so make sure we can read another 8 bytes
   if ( fileData.fileSize <= 16 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FONTERR_FILECORRUPT, filePath );
      Platform_Log( errorMsg );
      return False;
   }

   filePos32 = (uint32_t*)( fileData.contents );
   font->codepointOffset = filePos32[0];
   font->numGlyphs = filePos32[1];
   filePos32 += 2;
   bytesRead = 8;

   font->glyphs = (PixelBuffer_t*)Platform_MemAlloc( sizeof( PixelBuffer_t ) * font->numGlyphs );

   for ( i = 0; i < font->numGlyphs; i++ )
   {
      font->glyphs[i].dimensions.x = filePos32[0];
      font->glyphs[i].dimensions.y = filePos32[1];
      filePos32 += 2;
      bytesRead += 8;

      bufferSize = ( ( font->glyphs[i].dimensions.x * font->glyphs[i].dimensions.y ) * ( GRAPHICS_BPP / 8 ) );

      if ( ( fileData.fileSize - bytesRead ) < bufferSize )
      {
         for ( k = (int32_t)i - 1; k >= 0; k-- )
         {
            Platform_MemFree( font->glyphs[k].buffer );
         }
         Platform_MemFree( font->glyphs );

         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FONTERR_FILECORRUPT, filePath );
         Platform_Log( errorMsg );
         return False;
      }

      font->glyphs[i].buffer = (uint8_t*)Platform_MemAlloc( bufferSize );

      for ( j = 0; j < bufferSize; j++ )
      {
         font->glyphs[i].buffer[j] = ( (uint8_t*)filePos32 )[j];
      }

      filePos32 += ( font->glyphs[i].dimensions.x * font->glyphs[i].dimensions.y );
      bytesRead += bufferSize;
   }

   if ( bytesRead != fileData.fileSize )
   {
      for ( i = 0; i < font->numGlyphs; i++ )
      {
         Platform_MemFree( font->glyphs[i].buffer );
      }
      Platform_MemFree( font->glyphs );

      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FONTERR_FILECORRUPT, filePath );
      Platform_Log( errorMsg );
      return False;
   }

   return True;
}
