#if !defined( BLIT_H )
#define BLIT_H

#include "common.h"
#include "platform.h"

typedef struct PixelBuffer_t PixelBuffer_t;
typedef struct Image_t Image_t;
typedef struct Sprite_t Sprite_t;
typedef struct Font_t Font_t;

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
void Blit_ColoredTexture( GLuint textureHandle, PixelBuffer_t* pixelBuffer,
                          float screenX, float screenY,
                          float scale, uint32_t color );
void Blit_Texture( GLuint textureHandle, PixelBuffer_t* pixelBuffer, float screenX, float screenY, float scale );
void Blit_ColoredImageSection( Image_t* image,
                               float screenX, float screenY,
                               uint32_t imageX, uint32_t imageY,
                               uint32_t sectionWidth, uint32_t sectionHeight,
                               float scale, uint32_t color );
void Blit_ImageSection( Image_t* image,
                        float screenX, float screenY,
                        uint32_t imageX, uint32_t imageY,
                        uint32_t sectionWidth, uint32_t sectionHeight,
                        float scale );
void Blit_ColoredImage( Image_t* image, float screenX, float screenY, float scale, uint32_t color );
void Blit_Image( Image_t* image, float screenX, float screenY, float scale );
void Blit_ColoredSprite( Sprite_t* sprite, float scale, float screenX, float screenY, uint32_t color );
void Blit_Sprite( Sprite_t* sprite, float scale, float screenX, float screenY );
void Blit_Char( uint32_t codepoint, float scale, float screenX, float screenY, Font_t* font );
void Blit_TextLine( const char* text, float scale, float screenX, float screenY, Font_t* font, FontJustify_t justify );

#endif
