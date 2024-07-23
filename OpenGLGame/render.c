#include <math.h>

#include "render.h"

internal void Render_PrepareTextureForDrawing( GLuint textureHandle, PixelBuffer_t* pixelBuffer, float scale,
                                               float screenX, float screenY,
                                               float width, float height );

void Render_Clear()
{
   glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
   glClear( GL_COLOR_BUFFER_BIT );
}

void Render_DrawRect( float screenX, float screenY, float width, float height, uint32_t color )
{
   GLint r = ( color >> 16 ) & 0xFF;
   GLint g = ( color >> 8 ) & 0xFF;
   GLint b = color & 0xFF;
   GLint a = ( color >> 24 ) & 0xFF;

   glViewport( (GLint)screenX, (GLint)screenY, (GLsizei)width, (GLsizei)height );

   glDisable( GL_TEXTURE_2D );

   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glEnable( GL_BLEND );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();

   glColor4f( r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f );

   glBegin( GL_QUADS );

   glVertex2f( -1.0f, -1.0f );
   glVertex2f( 1.0f, -1.0f );
   glVertex2f( 1.0f, 1.0f );
   glVertex2f( -1.0f, 1.0f );

   glEnd();
}

void Render_DrawTextureSection( GLuint textureHandle, PixelBuffer_t* pixelBuffer, float scale,
                                float screenX, float screenY,
                                int32_t textureX, int32_t textureY,
                                uint32_t sectionWidth, uint32_t sectionHeight )
{
   float fw = (float)sectionWidth;
   float fh = (float)sectionHeight;
   float fnx1 = (float)textureX / pixelBuffer->dimensions.x;
   float fny1 = (float)textureY / pixelBuffer->dimensions.y;
   float fnx2 = fnx1 + ( fw / pixelBuffer->dimensions.x );
   float fny2 = fny1 + ( fh / pixelBuffer->dimensions.y );

   Render_PrepareTextureForDrawing( textureHandle, pixelBuffer, scale,
                                    screenX, screenY,
                                    (float)sectionWidth, (float)sectionHeight );

   glBegin( GL_TRIANGLES );

   // lower triangle
   glTexCoord2f( fnx1, fny1 );
   glVertex2f( 0.0f, 0.0f );
   glTexCoord2f( fnx2, fny1 );
   glVertex2f( fw, 0.0f );
   glTexCoord2f( fnx2, fny2 );
   glVertex2f( fw, fh );

   // upper triangle
   glTexCoord2f( fnx1, fny1 );
   glVertex2f( 0.0f, 0.0f );
   glTexCoord2f( fnx2, fny2 );
   glVertex2f( fw, fh );
   glTexCoord2f( fnx1, fny2 );
   glVertex2f( 0.0f, fh );

   glEnd();
}

void Render_DrawTexture( Texture_t* texture, float scale, float screenX, float screenY )
{
   Render_DrawTextureSection( texture->textureHandle, &( texture->pixelBuffer ), scale,
                              screenX, screenY,
                              0, 0,
                              texture->pixelBuffer.dimensions.x, texture->pixelBuffer.dimensions.y );
}

void Render_DrawSprite( Sprite_t* sprite, float scale, float screenX, float screenY )
{
   uint32_t rowIndex = ( sprite->frameIndex ) / sprite->frameStride;
   uint32_t colIndex = ( sprite->frameIndex ) % sprite->frameStride;

   Render_DrawTextureSection( sprite->texture->textureHandle, &( sprite->texture->pixelBuffer ), scale,
                              screenX, screenY,
                              sprite->frameDimensions.x * colIndex, sprite->frameDimensions.y * rowIndex,
                              sprite->frameDimensions.x, sprite->frameDimensions.y );
}

void Render_DrawChar( uint32_t codepoint, float scale, float screenX, float screenY, Font_t* font )
{
   FontGlyph_t* glyph;
   PixelBuffer_t* buffer;
   float x, y;

   if ( !Font_ContainsChar( font, codepoint ) )
   {
      return;
   }

   glyph = &( font->curGlyphCollection->glyphs[codepoint - font->codepointOffset] );
   buffer = &( glyph->pixelBuffer );
   x = screenX + ( glyph->leftBearing * scale );
   y = screenY + ceilf( ( ( font->curGlyphCollection->baseline + glyph->baselineOffset ) * scale ) );

   Render_DrawTextureSection( font->textureHandle, buffer, scale,
                              x, y,
                              0, 0,
                              buffer->dimensions.x, buffer->dimensions.y );
}

void Render_DrawTextLine( const char* text, float scale, float screenX, float screenY, Font_t* font )
{
   uint32_t i;
   float x = screenX;
   FontGlyph_t* glyph;

   for ( i = 0; i < strlen( text ); i++ )
   {
      if ( Font_ContainsChar( font, text[i] ) )
      {
         glyph = font->curGlyphCollection->glyphs + ( (uint32_t)text[i] - font->codepointOffset );
         Render_DrawChar( (uint32_t)( text[i] ), scale, x, screenY, font );
         x += ceilf( ( glyph->advance * scale ) );
      }
   }
}

internal void Render_PrepareTextureForDrawing( GLuint textureHandle, PixelBuffer_t* pixelBuffer, float scale,
                                               float screenX, float screenY,
                                               float width, float height )
{
   GLfloat modelMatrix[] = 
   {
      2.0f / ( width / scale ), 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / ( height / scale ), 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      -1.0f, -1.0f, 0.0f, 1.0f
   };

   glViewport( (GLint)screenX, (GLint)screenY, (GLsizei)( width * scale ), (GLsizei)( height * scale ) );

   glBindTexture( GL_TEXTURE_2D, textureHandle );
   glTexImage2D( GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 pixelBuffer->dimensions.x,
                 pixelBuffer->dimensions.y,
                 0,
                 GL_BGRA_EXT,
                 GL_UNSIGNED_BYTE,
                 (GLvoid*)( pixelBuffer->memory ) );

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

   glEnable( GL_TEXTURE_2D );

   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glEnable( GL_BLEND );

   glMatrixMode( GL_TEXTURE );
   glLoadIdentity();

   glMatrixMode( GL_MODELVIEW );
   glLoadMatrixf( modelMatrix );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
}
