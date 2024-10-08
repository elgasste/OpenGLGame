#include "sprite.h"
#include "image.h"
#include "clock.h"

Bool_t Sprite_LoadBaseFromMemory( SpriteBase_t* base, Image_t* image, uint8_t* memory, uint32_t memSize )
{
   Vector2ui32_t* imageDim = &( image->pixelBuffer.dimensions );
   Vector2ui32_t* frameDim;
   uint32_t* memPos32 = (uint32_t*)memory;
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( memSize != 12 )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_SPRITEERR_MEMORYCORRUPT, (uint32_t)( base->ID ) );
      Platform_Log( errorMsg );
      return False;
   }

   base->image = image; // image ID is in position 0
   base->frameDimensions.x = memPos32[1];
   base->frameDimensions.y = memPos32[2];

   frameDim = &( base->frameDimensions );

   if ( ( imageDim->x % frameDim->x != 0 ) || ( imageDim->y % frameDim->y != 0 ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_SPRITEERR_FRAMEDIMENSIONS, (uint32_t)( image->ID ) );
      Platform_Log( errorMsg );
      return False;
   }

   base->frameStride = imageDim->x / frameDim->x;
   base->numFrames = base->frameStride * ( imageDim->y / frameDim->y );

   return True;
}

void Sprite_LoadFromBase( Sprite_t* sprite, SpriteBase_t* base, float frameSeconds )
{
   sprite->base = base;
   sprite->frameSeconds = frameSeconds;
   Sprite_Reset( sprite );
}

void Sprite_Reset( Sprite_t* sprite )
{
   sprite->frameIndex = 0;
   sprite->secondsElapsed = 0.0f;
   Sprite_ScaleFrameTime( sprite, 1.0f );
}

void Sprite_ScaleFrameTime( Sprite_t* sprite, float scalar )
{
   sprite->scaledFrameSeconds = sprite->frameSeconds * scalar;
}

void Sprite_SetFrameIndex( Sprite_t* sprite, uint32_t index )
{
   if ( index < sprite->base->numFrames )
   {
      sprite->frameIndex = index;
   }
}

void Sprite_Tick( Sprite_t* sprite, Clock_t* clock )
{
   if ( sprite->scaledFrameSeconds == 0.0f )
   {
      return;
   }

   sprite->secondsElapsed += clock->frameDeltaSeconds;

   while ( sprite->secondsElapsed > sprite->scaledFrameSeconds )
   {
      sprite->secondsElapsed -= sprite->scaledFrameSeconds;
      sprite->frameIndex++;

      if ( sprite->frameIndex >= sprite->base->numFrames )
      {
         sprite->frameIndex = 0;
      }
   }
}
