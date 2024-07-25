#include <math.h>

#include "blit.h"
#include "pixel_buffer.h"
#include "image.h"
#include "sprite.h"
#include "font.h"

internal void Blit_PrepareTextureForDrawing( GLuint textureHandle, PixelBuffer_t* pixelBuffer,
                                             float screenX, float screenY,
                                             float width, float height,
                                             float scale, uint32_t color );

void Blit_ClearScreen()
{
   glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
   glClear( GL_COLOR_BUFFER_BIT );
}

void Blit_Rect( float screenX, float screenY, float width, float height, uint32_t color )
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

void Blit_ColoredTextureSection( GLuint textureHandle, PixelBuffer_t* pixelBuffer,
                                 float screenX, float screenY,
                                 int32_t textureX, int32_t textureY,
                                 uint32_t sectionWidth, uint32_t sectionHeight,
                                 float scale, uint32_t color )
{
   float fw = (float)sectionWidth;
   float fh = (float)sectionHeight;
   float fnx1 = (float)textureX / pixelBuffer->dimensions.x;
   float fny1 = (float)textureY / pixelBuffer->dimensions.y;
   float fnx2 = fnx1 + ( fw / pixelBuffer->dimensions.x );
   float fny2 = fny1 + ( fh / pixelBuffer->dimensions.y );

   Blit_PrepareTextureForDrawing( textureHandle, pixelBuffer,
                                  screenX, screenY,
                                  (float)sectionWidth, (float)sectionHeight,
                                  scale, color );

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

void Blit_TextureSection( GLuint textureHandle, PixelBuffer_t* pixelBuffer,
                          float screenX, float screenY,
                          int32_t textureX, int32_t textureY,
                          uint32_t sectionWidth, uint32_t sectionHeight,
                          float scale )
{
   Blit_ColoredTextureSection( textureHandle, pixelBuffer,
                               screenX, screenY,
                               textureX, textureY,
                               sectionWidth, sectionHeight,
                               scale, 0xFFFFFFFF );
}

void Blit_ColoredImage( Image_t* image, float screenX, float screenY, float scale, uint32_t color )
{
   Blit_ColoredTextureSection( image->textureHandle, &( image->pixelBuffer ),
                               screenX, screenY,
                               0, 0,
                               image->pixelBuffer.dimensions.x, image->pixelBuffer.dimensions.y,
                               scale, color );
}

void Blit_Image( Image_t* image, float screenX, float screenY, float scale )
{
   Blit_ColoredImage( image, screenX, screenY, scale, 0xFFFFFFFF );
}

void Blit_ColoredSprite( Sprite_t* sprite, float screenX, float screenY, float scale, uint32_t color )
{
   uint32_t rowIndex = ( sprite->frameIndex ) / sprite->frameStride;
   uint32_t colIndex = ( sprite->frameIndex ) % sprite->frameStride;

   Blit_ColoredTextureSection( sprite->image->textureHandle, &( sprite->image->pixelBuffer ),
                               screenX, screenY,
                               sprite->frameDimensions.x * colIndex, sprite->frameDimensions.y * rowIndex,
                               sprite->frameDimensions.x, sprite->frameDimensions.y,
                               scale, color );
}

void Blit_Sprite( Sprite_t* sprite, float screenX, float screenY, float scale )
{
   Blit_ColoredSprite( sprite, screenX, screenY, scale, 0xFFFFFFFF );
}

void Blit_Char( uint32_t codepoint, float scale, float screenX, float screenY, Font_t* font )
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

   Blit_ColoredTextureSection( font->textureHandle, buffer,
                               x, y,
                               0, 0,
                               buffer->dimensions.x, buffer->dimensions.y,
                               scale, glyph->color );
}

void Blit_TextLine( const char* text, float scale, float screenX, float screenY, Font_t* font )
{
   uint32_t i;
   float x = screenX;
   FontGlyph_t* glyph;

   for ( i = 0; i < strlen( text ); i++ )
   {
      if ( Font_ContainsChar( font, text[i] ) )
      {
         glyph = font->curGlyphCollection->glyphs + ( (uint32_t)text[i] - font->codepointOffset );
         Blit_Char( (uint32_t)( text[i] ), scale, x, screenY, font );
         x += ceilf( ( glyph->advance * scale ) );
      }
   }
}

internal void Blit_PrepareTextureForDrawing( GLuint textureHandle, PixelBuffer_t* pixelBuffer,
                                             float screenX, float screenY,
                                             float width, float height,
                                             float scale, uint32_t color )
{
   GLint r = ( color >> 16 ) & 0xFF;
   GLint g = ( color >> 8 ) & 0xFF;
   GLint b = color & 0xFF;
   GLint a = ( color >> 24 ) & 0xFF;
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

   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

   glEnable( GL_TEXTURE_2D );

   glColor4f( r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glEnable( GL_BLEND );

   glMatrixMode( GL_TEXTURE );
   glLoadIdentity();

   glMatrixMode( GL_MODELVIEW );
   glLoadMatrixf( modelMatrix );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
}
