#include "render.h"

internal void Render_PrepareTextureForDrawing( Texture_t* texture,
                                               uint32_t x, uint32_t y,
                                               uint32_t viewportWidth, uint32_t viewportHeight );

void Render_Clear()
{
   glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
   glClear( GL_COLOR_BUFFER_BIT );
}

void Render_DrawTexture( RenderData_t* renderData, TextureID_t textureID, int32_t x, int32_t y )
{
   Texture_t* texture = &( renderData->textures[textureID] );
   uint32_t w = texture->pixelBuffer.dimensions.x;
   uint32_t h = texture->pixelBuffer.dimensions.y;
   float fw = (float)w;
   float fh = (float)h;
   
   Render_PrepareTextureForDrawing( texture, x, y, w, h );

   glBegin( GL_TRIANGLES );

   // lower triangle
   glTexCoord2f( 0.0f, 0.0f );
   glVertex2f( 0.0f, 0.0f );
   glTexCoord2f( 1.0f, 0.0f );
   glVertex2f( fw, 0.0f );
   glTexCoord2f( 1.0f, 1.0f );
   glVertex2f( fw, fh );

   // upper triangle
   glTexCoord2f( 0.0f, 0.0f );
   glVertex2f( 0.0f, 0.0f );
   glTexCoord2f( 1.0f, 1.0f );
   glVertex2f( fw, fh );
   glTexCoord2f( 0.0f, 1.0f );
   glVertex2f( 0.0f, fh );

   glEnd();
}

void Render_DrawSprite( Sprite_t* sprite, int32_t x, int32_t y )
{
   float frameW = (float)( sprite->frameDimensions.x );
   float frameH = (float)( sprite->frameDimensions.y );
   uint32_t numRows = sprite->texture->pixelBuffer.dimensions.y / sprite->frameDimensions.y;
   uint32_t rowIndex = ( sprite->frameIndex ) / sprite->frameStride;
   uint32_t colIndex = ( sprite->frameIndex ) % sprite->frameStride;
   float frameSizeX = 1.0f / sprite->frameStride;
   float frameSizeY = 1.0f / numRows;
   float textureX = frameSizeX * colIndex;
   float textureY = frameSizeY * rowIndex;
   float textureW = textureX + frameSizeX;
   float textureH = textureY + frameSizeY;

   Render_PrepareTextureForDrawing( sprite->texture, x, y, sprite->frameDimensions.x, sprite->frameDimensions.y );

   glBegin( GL_TRIANGLES );

   // lower triangle
   glTexCoord2f( textureX, textureY );
   glVertex2f( 0.0f, 0.0f );
   glTexCoord2f( textureW, textureY );
   glVertex2f( frameW, 0.0f );
   glTexCoord2f( textureW, textureH );
   glVertex2f( frameW, frameH );

   // upper triangle
   glTexCoord2f( textureX, textureY );
   glVertex2f( 0.0f, 0.0f );
   glTexCoord2f( textureW, textureH );
   glVertex2f( frameW, frameH );
   glTexCoord2f( textureX, textureH );
   glVertex2f( 0.0f, frameH );

   glEnd();
}

internal void Render_PrepareTextureForDrawing( Texture_t* texture,
                                               uint32_t x, uint32_t y,
                                               uint32_t viewportWidth, uint32_t viewportHeight )
{
   GLfloat modelMatrix[] = 
   {
      2.0f / viewportWidth, 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / viewportHeight, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      -1.0f, -1.0f, 0.0f, 1.0f
   };

   glViewport( x, y, viewportWidth, viewportHeight );

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
