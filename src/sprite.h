#if !defined( SPRITE_H )
#define SPRITE_H

#include "common.h"
#include "vector.h"

typedef struct Image_t Image_t;
typedef struct Clock_t Clock_t;

typedef struct SpriteBase_t
{
   Image_t* image;
   Vector2ui32_t frameDimensions;
   uint32_t numFrames;
   uint32_t frameStride;
}
SpriteBase_t;

typedef struct Sprite_t
{
   SpriteBase_t* base;
   uint32_t frameIndex;
   float frameSeconds;
   float secondsElapsed;
   float scaledFrameSeconds;
}
Sprite_t;

Bool_t Sprite_LoadBaseFromMemory( SpriteBase_t* base,
                                  SpriteBaseID_t baseID,
                                  Image_t* image,
                                  ImageID_t imageID,
                                  uint8_t* memory,
                                  uint32_t memSize );
Bool_t Sprite_LoadFromBase( Sprite_t* sprite, SpriteBase_t* base, float frameSeconds );
void Sprite_Reset( Sprite_t* sprite );
void Sprite_ScaleFrameTime( Sprite_t* sprite, float scalar );
void Sprite_Tick( Sprite_t* sprite, Clock_t* clock );

#endif
