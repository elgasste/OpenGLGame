#if !defined( BLIT_H )
#define BLIT_H

#include "common.h"
#include "sprite.h"
#include "font.h"

void Blit_ClearScreen();
void Blit_Rect( float screenX, float screenY, float width, float height, uint32_t color );
void Blit_ColoredTextureSection( GLuint textureHandle, PixelBuffer_t* pixelBuffer,
                                 float screenX, float screenY,
                                 int32_t textureX, int32_t textureY,
                                 uint32_t sectionWidth, uint32_t sectionHeight,
                                 float scale, uint32_t color );
void Blit_TextureSection( GLuint textureHandle, PixelBuffer_t* pixelBuffer,
                          float screenX, float screenY,
                          int32_t textureX, int32_t textureY,
                          uint32_t sectionWidth, uint32_t sectionHeight,
                          float scale );
void Blit_ColoredImage( Image_t* texture, float screenX, float screenY, float scale, uint32_t color );
void Blit_Image( Image_t* texture, float screenX, float screenY, float scale );
void Blit_ColoredSprite( Sprite_t* sprite, float scale, float screenX, float screenY, uint32_t color );
void Blit_Sprite( Sprite_t* sprite, float scale, float screenX, float screenY );
void Blit_Char( uint32_t codepoint, float scale, float screenX, float screenY, Font_t* font );
void Blit_TextLine( const char* text, float scale, float screenX, float screenY, Font_t* font );

#endif
