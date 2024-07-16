#include "font.h"
#include "platform.h"

Bool_t Font_LoadFromFile( Font_t* font, const char* filePath )
{
   FileData_t fileData;
   uint32_t* filePos32;
   uint32_t bufferSize, bytesRead, i, j;
   int32_t k;
   FontGlyph_t* glyph;
   PixelBuffer_t* buffer;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      return False;
   }

   // first 4 values are codepoint offset, descent, line gap, and number of glyphs.
   // these are each 4 bytes, and we want to make sure there's at least one glyph,
   // so make sure we can read another 20 bytes after that.
   if ( fileData.fileSize <= 36 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FONTERR_FILECORRUPT, filePath );
      Platform_Log( errorMsg );
      return False;
   }

   filePos32 = (uint32_t*)( fileData.contents );
   font->codepointOffset = filePos32[0];
   font->descent = filePos32[1];
   font->lineGap = filePos32[2];
   font->numGlyphs = filePos32[3];
   filePos32 += 4;
   bytesRead = 16;

   font->glyphs = (FontGlyph_t*)Platform_MemAlloc( sizeof( FontGlyph_t ) * font->numGlyphs );

   glyph = font->glyphs;
   buffer = &( glyph->pixelBuffer );

   for ( i = 0; i < font->numGlyphs; i++ )
   {
      glyph->offset.x = filePos32[0];
      glyph->offset.y = filePos32[1];
      glyph->advance = filePos32[2];
      buffer->dimensions.x = filePos32[3];
      buffer->dimensions.y = filePos32[4];
      filePos32 += 5;
      bytesRead += 20;

      bufferSize = ( ( buffer->dimensions.x * buffer->dimensions.y ) * ( GRAPHICS_BPP / 8 ) );

      if ( ( fileData.fileSize - bytesRead ) < bufferSize )
      {
         for ( k = (int32_t)i - 1; k >= 0; k-- )
         {
            Platform_MemFree( buffer->memory );
         }
         Platform_MemFree( font->glyphs );

         snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FONTERR_FILECORRUPT, filePath );
         Platform_Log( errorMsg );
         return False;
      }

      buffer->memory = (uint8_t*)Platform_MemAlloc( bufferSize );

      for ( j = 0; j < bufferSize; j++ )
      {
         buffer->memory[j] = ( (uint8_t*)filePos32 )[j];
      }

      filePos32 += ( buffer->dimensions.x * buffer->dimensions.y );
      bytesRead += bufferSize;

      glyph++;
      buffer = &( glyph->pixelBuffer );
   }

   if ( bytesRead != fileData.fileSize )
   {
      glyph = font->glyphs;
      for ( i = 0; i < font->numGlyphs; i++ )
      {
         Platform_MemFree( glyph->pixelBuffer.memory );
         glyph++;
      }
      Platform_MemFree( font->glyphs );

      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FONTERR_FILECORRUPT, filePath );
      Platform_Log( errorMsg );
      return False;
   }

   glGenTextures( 1, &( font->textureHandle ) );

   return True;
}
