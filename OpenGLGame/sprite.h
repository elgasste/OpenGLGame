#if !defined( SPRITE_H )
#define SPRITE_H

#include "common.h"
#include "image.h"

typedef struct Clock_t Clock_t;

typedef struct Sprite_t
{
   Image_t* image;
   Vector2ui32_t frameDimensions;
   uint32_t numFrames;
   uint32_t frameStride;

   uint32_t frameIndex;
   float frameSeconds;
   float secondsElapsed;
   float scaledFrameSeconds;
}
Sprite_t;

Bool_t Sprite_Init( Sprite_t* sprite, Image_t* image, uint32_t frameWidth, uint32_t frameHeight, float frameSeconds );
void Sprite_Reset( Sprite_t* sprite );
void Sprite_ScaleFrameTime( Sprite_t* sprite, float scalar );
void Sprite_Tick( Sprite_t* sprite, Clock_t* clock );

#endif
