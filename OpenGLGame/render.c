#include "render.h"

internal void Render_PrepareTextureForDrawing( Texture_t* texture,
                                               int32_t screenX, int32_t screenY,
                                               uint32_t viewportWidth, uint32_t viewportHeight );

void Render_Clear()
{
   glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
   glClear( GL_COLOR_BUFFER_BIT );
}

void Render_DrawTextureSection( Texture_t* texture, float scale,
                                int32_t screenX, int32_t screenY,
                                int32_t textureX, int32_t textureY,
                                uint32_t sectionWidth, uint32_t sectionHeight )
{
   float fw = (float)sectionWidth;
   float fh = (float)sectionHeight;
   float fnx1 = (float)textureX / texture->pixelBuffer.dimensions.x;
   float fny1 = (float)textureY / texture->pixelBuffer.dimensions.y;
   float fnx2 = fnx1 + ( fw / texture->pixelBuffer.dimensions.x );
   float fny2 = fny1 + ( fh / texture->pixelBuffer.dimensions.y );
   float fsw = fw * scale;
   float fsh = fh * scale;

   Render_PrepareTextureForDrawing( texture,
                                    screenX, screenY,
                                    (uint32_t)( sectionWidth * scale), (uint32_t)( sectionHeight * scale ) );

   glBegin( GL_TRIANGLES );

   // lower triangle
   glTexCoord2f( fnx1, fny1 );
   glVertex2f( 0.0f, 0.0f );
   glTexCoord2f( fnx2, fny1 );
   glVertex2f( fsw, 0.0f );
   glTexCoord2f( fnx2, fny2 );
   glVertex2f( fsw, fsh );

   // upper triangle
   glTexCoord2f( fnx1, fny1 );
   glVertex2f( 0.0f, 0.0f );
   glTexCoord2f( fnx2, fny2 );
   glVertex2f( fsw, fsh );
   glTexCoord2f( fnx1, fny2 );
   glVertex2f( 0.0f, fsh );

   glEnd();
}

void Render_DrawTexture( Texture_t* texture, float scale, int32_t screenX, int32_t screenY )
{
   Render_DrawTextureSection( texture, scale,
                              screenX, screenY,
                              0, 0,
                              texture->pixelBuffer.dimensions.x, texture->pixelBuffer.dimensions.y );
}

void Render_DrawSprite( Sprite_t* sprite, float scale, int32_t screenX, int32_t screenY )
{
   uint32_t rowIndex = ( sprite->frameIndex ) / sprite->frameStride;
   uint32_t colIndex = ( sprite->frameIndex ) % sprite->frameStride;

   Render_DrawTextureSection( sprite->texture, scale,
                              screenX, screenY,
                              sprite->frameDimensions.x * colIndex, sprite->frameDimensions.y * rowIndex,
                              sprite->frameDimensions.x, sprite->frameDimensions.y );
}

internal void Render_PrepareTextureForDrawing( Texture_t* texture,
                                               int32_t screenX, int32_t screenY,
                                               uint32_t viewportWidth, uint32_t viewportHeight )
{
   GLfloat modelMatrix[] = 
   {
      2.0f / viewportWidth, 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / viewportHeight, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      -1.0f, -1.0f, 0.0f, 1.0f
   };

   glViewport( screenX, screenY, viewportWidth, viewportHeight );

   glBindTexture( GL_TEXTURE_2D, texture->textureHandle );
   glTexImage2D( GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 texture->pixelBuffer.dimensions.x,
                 texture->pixelBuffer.dimensions.y,
                 0,
                 GL_BGRA_EXT,
                 GL_UNSIGNED_BYTE,
                 (GLvoid*)( texture->pixelBuffer.buffer ) );

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

   glEnable( GL_TEXTURE_2D );

   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glEnable( GL_BLEND );

   glMatrixMode( GL_TEXTURE );
   glLoadIdentity();

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();

   glMatrixMode( GL_PROJECTION );
   glLoadMatrixf( modelMatrix );
}
