#if !defined( SPRITE_H )
#define SPRITE_H

#include "common.h"
#include "texture.h"

typedef struct Clock_t Clock_t;

typedef struct Sprite_t
{
   Texture_t texture;
   Vector2ui32_t frameDimensions;
   uint32_t numFrames;
   uint32_t frameStride;

   uint32_t frameIndex;
   float frameSeconds;
   float secondsElapsed;
   float speedScalar;
   float scaledFrameSeconds;
}
Sprite_t;

Bool_t Sprite_LoadFromFile( Sprite_t* sprite, const char* filePath, uint32_t frameWidth, uint32_t frameHeight, float frameSeconds );
void Sprite_Reset( Sprite_t* sprite );
void Sprite_ScaleSpeed( Sprite_t* sprite, float scalar );
void Sprite_Tick( Sprite_t* sprite, Clock_t* clock );

#endif
