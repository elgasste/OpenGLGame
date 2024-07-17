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

   // first 5 values are codepoint offset, full height, baseline, line gap, and
   // number of glyphs. these are each 4 bytes, and we want to make sure there's
   // at least one glyph, so make sure we can read another 20 bytes after that.
   if ( fileData.fileSize <= 40 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FONTERR_FILECORRUPT, filePath );
      Platform_Log( errorMsg );
      return False;
   }

   filePos32 = (uint32_t*)( fileData.contents );
   font->codepointOffset = filePos32[0];
   font->fullHeight = filePos32[1];
   font->baseline = filePos32[2];
   font->lineGap = filePos32[3];
   font->numGlyphs = filePos32[4];
   filePos32 += 5;
   bytesRead = 20;

   font->glyphs = (FontGlyph_t*)Platform_MemAlloc( sizeof( FontGlyph_t ) * font->numGlyphs );

   glyph = font->glyphs;
   buffer = &( glyph->pixelBuffer );

   for ( i = 0; i < font->numGlyphs; i++ )
   {
      glyph->leftBearing = filePos32[0];
      glyph->baselineOffset = filePos32[1];
      glyph->advance = filePos32[2];
      buffer->dimensions.x = filePos32[3];
      buffer->dimensions.y = filePos32[4];
      filePos32 += 5;
      bytesRead += 20;

      bufferSize = ( buffer->dimensions.x * buffer->dimensions.y * 4 );

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

Bool_t Font_ContainsChar( Font_t* font, uint32_t codepoint )
{
   return ( codepoint < font->codepointOffset ) || ( codepoint > ( font->codepointOffset + font->numGlyphs ) )
      ? False : True;
}

void Font_SetCharColor( Font_t* font, uint32_t codepoint, uint32_t color )
{
   uint32_t i;
   PixelBuffer_t* buffer;
   uint32_t* memory;

   if ( Font_ContainsChar( font, codepoint ) )
   {
      buffer = &( font->glyphs[codepoint - font->codepointOffset].pixelBuffer );
      memory = (uint32_t*)( buffer->memory );

      for ( i = 0; i < ( buffer->dimensions.x * buffer->dimensions.y ); i++ )
      {
         memory[i] = ( memory[i] & 0xFF000000 ) | ( color & 0x00FFFFFF );
      }
   }
}

void Font_SetColor( Font_t* font, uint32_t color )
{
   uint32_t i;

   for ( i = 0; i < font->numGlyphs; i++ )
   {
      Font_SetCharColor( font, i + font->codepointOffset, color );
   }
}
