#include "render.h"
#include "platform.h"
#include "bmp.h"

Bool_t Render_LoadTextureFromFile( Texture_t* texture, const char* filePath )
{
   if ( !Bmp_LoadFromFile( filePath, &( texture->pixelBuffer ) ) )
   {
      return False;
   }

   glGenTextures( 1, &( texture->textureHandle ) );
   return True;
}

void Render_Clear()
{
   glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
   glClear( GL_COLOR_BUFFER_BIT );
}

void Render_DrawTexture( int32_t x, int32_t y, Texture_t* texture )
{
   uint32_t w = texture->pixelBuffer.width;
   uint32_t h = texture->pixelBuffer.height;
   float fw = (float)w;
   float fh = (float)h;
   GLfloat modelMatrix[] = 
   {
      2.0f / texture->pixelBuffer.width, 0.0f, 0.0f, 0.0f,
      0.0f, 2.0f / texture->pixelBuffer.height, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      -1.0f, -1.0f, 0.0f, 1.0f
   };

   glViewport( x, y, w, h );

   glBindTexture( GL_TEXTURE_2D, texture->textureHandle );
   glTexImage2D( GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 w,
                 h,
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
