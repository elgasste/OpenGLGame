#if !defined( TEXTURE_H )
#define TEXTURE_H

#include "common.h"
#include "pixel_buffer.h"
#include "platform.h"

typedef struct Texture_t
{
   PixelBuffer_t pixelBuffer;
   GLuint textureHandle;
}
Texture_t;

Bool_t Texture_LoadFromFile( Texture_t* texture, const char* filePath );
void Texture_Cleanup( Texture_t* texture );

#endif
