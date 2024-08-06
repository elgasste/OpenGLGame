#include <float.h>

#include "font.h"

#define ERROR_RETURN_FALSE() \
   snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_FONTERR_MEMORYCORRUPT, fontID ); \
   Platform_Log( errorMsg ); \
   Font_ClearData( font ); \
   return False

Bool_t Font_LoadFromMemory( Font_t* font, uint8_t* memory, uint32_t memSize, uint32_t fontID )
{
   uint32_t* memPos32;
   uint32_t bufferSize, bytesRead, i, j, k;
   FontGlyphCollection_t* glyphCollection;
   FontGlyph_t* glyph;
   PixelBuffer_t* buffer;
   char errorMsg[STRING_SIZE_DEFAULT];

   font->glyphCollections = 0;

   // first 3 values are codepoint offset, number of glyph collections, and
   // number of glyphs. these are each 4 bytes, and we want to make sure there's
   // at least one glyph collection header (12 bytes)
   if ( memSize < 24 )
   {
      ERROR_RETURN_FALSE();
   }

   memPos32 = (uint32_t*)( memory );
   font->codepointOffset = memPos32[0];
   font->numGlyphCollections = memPos32[1];
   font->numGlyphs = memPos32[2];
   memPos32 += 3;
   bytesRead = 12;

   font->glyphCollections = (FontGlyphCollection_t*)Platform_MAlloc( sizeof( FontGlyphCollection_t ) * font->numGlyphCollections );
   for ( i = 0; i < font->numGlyphCollections; i++ )
   {
      font->glyphCollections[i].glyphs = 0;
   }

   glyphCollection = font->glyphCollections;

   for ( i = 0; i < font->numGlyphCollections; i++ )
   {
      // first 3 values are height, baseline, and line gap, which are 4 bytes each
      glyphCollection->height = ( (float*)memPos32 )[0];
      glyphCollection->baseline = ( (float*)memPos32 )[1];
      glyphCollection->lineGap = ( (float*)memPos32 )[2];
      memPos32 += 3;
      bytesRead += 12;

      // make sure there's enough content to read at least one glyph header
      if ( ( memSize - bytesRead ) < 20 )
      {
         ERROR_RETURN_FALSE();
      }

      glyphCollection->glyphs = (FontGlyph_t*)Platform_MAlloc( sizeof( FontGlyph_t ) * font->numGlyphs );
      for ( j = 0; j < font->numGlyphs; j++ )
      {
         glyphCollection->glyphs[j].pixelBuffer.memory = 0;
      }

      glyph = font->glyphCollections[i].glyphs;

      for ( j = 0; j < font->numGlyphs; j++ )
      {
         buffer = &( glyph->pixelBuffer );
         glyph->leftBearing = ( (float*)memPos32 )[0];
         glyph->baselineOffset = ( (float*)memPos32 )[1];
         glyph->advance = ( (float*)memPos32 )[2];
         PixelBuffer_Init( buffer, memPos32[3], memPos32[4] );
         memPos32 += 5;
         bytesRead += 20;
         bufferSize = ( buffer->dimensions.x * buffer->dimensions.y * 4 );

         if ( ( memSize - bytesRead ) < bufferSize )
         {
            ERROR_RETURN_FALSE();
         }

         for ( k = 0; k < bufferSize; k++ )
         {
            if ( buffer->memory )
            {
               buffer->memory[k] = ( (uint8_t*)memPos32 )[k];
            }
         }

         memPos32 += ( buffer->dimensions.x * buffer->dimensions.y );
         bytesRead += bufferSize;

         glyph->color = 0xFFFFFFFF;
         glyph++;
      }

      glyphCollection++;
   }

   if ( bytesRead != memSize )
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
                  PixelBuffer_ClearData( buffer );
               }
            }

            Platform_Free( font->glyphCollections[i].glyphs, sizeof( FontGlyph_t ) * font->numGlyphs );
         }
      }

      Platform_Free( font->glyphCollections, sizeof( FontGlyphCollection_t ) * font->numGlyphCollections );
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

Vector2f_t Font_GetTextDimensions( Font_t* font, const char* text )
{
   uint32_t i, codepoint;
   Vector2f_t dimensions = { 0 };
   FontGlyphCollection_t* collection = font->curGlyphCollection;
   FontGlyph_t* glyph;

   dimensions.y = collection->height;

   for ( i = 0; i < (uint32_t)( strlen( text ) ); i++ )
   {
      codepoint = (uint32_t)( text[i] );

      if ( Font_ContainsChar( font, codepoint ) )
      {
         glyph = &( collection->glyphs[codepoint - font->codepointOffset] );
         dimensions.x += glyph->advance;
      }
   }

   return dimensions;
}
