#include <float.h>

#include "font.h"

#define ERROR_RETURN_FALSE() \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FONTERR_FILECORRUPT, filePath ); \
   Platform_Log( errorMsg ); \
   Font_ClearData( font ); \
   return False

Bool_t Font_LoadFromFile( Font_t* font, const char* filePath )
{
   FileData_t fileData;
   uint32_t* filePos32;
   uint32_t bufferSize, bytesRead, i, j, k;
   FontGlyphCollection_t* glyphCollection;
   FontGlyph_t* glyph;
   PixelBuffer_t* buffer;
   char errorMsg[STRING_SIZE_DEFAULT];

   font->glyphCollections = 0;

   if ( !Platform_ReadFileData( filePath, &fileData ) )
   {
      return False;
   }

   // first 3 values are codepoint offset, number of glyph collections, and
   // number of glyphs. these are each 4 bytes, and we want to make sure there's
   // at least one glyph collection header (12 bytes)
   if ( fileData.fileSize < 24 )
   {
      ERROR_RETURN_FALSE();
   }

   filePos32 = (uint32_t*)( fileData.contents );
   font->codepointOffset = filePos32[0];
   font->numGlyphCollections = filePos32[1];
   font->numGlyphs = filePos32[2];
   filePos32 += 3;
   bytesRead = 12;

   font->glyphCollections = (FontGlyphCollection_t*)Platform_MemAlloc( sizeof( FontGlyphCollection_t ) * font->numGlyphCollections );
   for ( i = 0; i < font->numGlyphCollections; i++ )
   {
      font->glyphCollections[i].glyphs = 0;
   }

   glyphCollection = font->glyphCollections;

   for ( i = 0; i < font->numGlyphCollections; i++ )
   {
      // first 3 values are height, baseline, and line gap, which are 4 bytes each
      glyphCollection->height = ( (float*)filePos32 )[0];
      glyphCollection->baseline = ( (float*)filePos32 )[1];
      glyphCollection->lineGap = ( (float*)filePos32 )[2];
      filePos32 += 3;
      bytesRead += 12;

      // make sure there's enough content to read at least one glyph header
      if ( ( fileData.fileSize - bytesRead ) < 20 )
      {
         ERROR_RETURN_FALSE();
      }

      glyphCollection->glyphs = (FontGlyph_t*)Platform_MemAlloc( sizeof( FontGlyph_t ) * font->numGlyphs );
      for ( j = 0; j < font->numGlyphs; j++ )
      {
         glyphCollection->glyphs[j].pixelBuffer.memory = 0;
      }

      glyph = font->glyphCollections[i].glyphs;

      for ( j = 0; j < font->numGlyphs; j++ )
      {
         buffer = &( glyph->pixelBuffer );
         glyph->leftBearing = ( (float*)filePos32 )[0];
         glyph->baselineOffset = ( (float*)filePos32 )[1];
         glyph->advance = ( (float*)filePos32 )[2];
         buffer->dimensions.x = filePos32[3];
         buffer->dimensions.y = filePos32[4];
         filePos32 += 5;
         bytesRead += 20;
         bufferSize = ( buffer->dimensions.x * buffer->dimensions.y * 4 );

         if ( ( fileData.fileSize - bytesRead ) < bufferSize )
         {
            ERROR_RETURN_FALSE();
         }

         buffer->memory = (uint8_t*)Platform_MemAlloc( bufferSize );
         for ( k = 0; k < bufferSize; k++ )
         {
            buffer->memory[k] = ( (uint8_t*)filePos32 )[k];
         }

         filePos32 += ( buffer->dimensions.x * buffer->dimensions.y );
         bytesRead += bufferSize;

         glyph->color = 0xFFFFFFFF;
         glyph++;
      }

      glyphCollection++;
   }

   if ( bytesRead != fileData.fileSize )
   {
      ERROR_RETURN_FALSE();
   }

   Font_SetGlyphCollectionForHeight( font, FONT_DEFAULT_HEIGHT );

   glGenTextures( 1, &( font->textureHandle ) );

   return True;
}

void Font_ClearData( Font_t* font )
{
   uint32_t i, j;
   PixelBuffer_t* buffer;

   if ( font->glyphCollections )
   {
      for ( i = 0; i < font->numGlyphCollections; i++ )
      {
         if ( font->glyphCollections[i].glyphs )
         {
            for ( j = 0; j < font->numGlyphs; j++ )
            {
               buffer = &( font->glyphCollections[i].glyphs[j].pixelBuffer );

               if ( buffer->memory )
               {
                  Platform_MemFree( buffer->memory, (uint64_t)( buffer->dimensions.x * buffer->dimensions.y * 4 ) );
               }
            }
            Platform_MemFree( font->glyphCollections[i].glyphs, sizeof( FontGlyph_t ) * font->numGlyphs );
         }
      }

      Platform_MemFree( font->glyphCollections, sizeof( FontGlyphCollection_t ) * font->numGlyphCollections );
      font->glyphCollections = 0;
      font->curGlyphCollection = 0;
   }
}

Bool_t Font_ContainsChar( Font_t* font, uint32_t codepoint )
{
   return ( codepoint < font->codepointOffset ) || ( codepoint > ( font->codepointOffset + font->numGlyphs ) )
      ? False : True;
}

void Font_SetCharColor( Font_t* font, uint32_t codepoint, uint32_t color )
{
   uint32_t i;

   if ( Font_ContainsChar( font, codepoint ) )
   {
      for ( i = 0; i < font->numGlyphCollections; i++ )
      {
         font->glyphCollections[i].glyphs[codepoint - font->codepointOffset].color = color;
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

void Font_SetGlyphCollectionForHeight( Font_t* font, float height )
{
   uint32_t i, tallestIndex = 0, bestIndex = 0;
   float heightDiff, lowestHeightDiff = FLT_MAX, highestHeight = 0.0f;
   FontGlyphCollection_t* collection = font->glyphCollections;

   for ( i = 0; i < font->numGlyphCollections; i++ )
   {
      if ( collection->height == height )
      {
         font->curGlyphCollection = collection;
         return;
      }
      else if ( collection->height > height )
      {
         heightDiff = collection->height - height;
         if ( heightDiff < lowestHeightDiff )
         {
            lowestHeightDiff = heightDiff;
            bestIndex = i;
         }
      }

      if( collection->height > highestHeight )
      {
         highestHeight = collection->height;
         tallestIndex = i;
      }

      collection++;
   }

   if ( bestIndex == 0 && font->numGlyphCollections > 0 )
   {
      bestIndex = tallestIndex;
   }

   font->curGlyphCollection = &( font->glyphCollections[bestIndex] );
}
