#include "sprite.h"
#include "clock.h"

Bool_t Sprite_Init( Sprite_t* sprite, Texture_t* texture, uint32_t frameWidth, uint32_t frameHeight, float frameSeconds )
{
   if ( ( texture->pixelBuffer.dimensions.x % frameWidth != 0 ) ||
        ( texture->pixelBuffer.dimensions.y % frameHeight != 0 ) )
   {
      // TODO: log the texture ID?
      Platform_Log( STR_SPRITEERR_FRAMEDIMENSIONS );
      return False;
   }

   sprite->texture = texture;
   sprite->frameDimensions.x = frameWidth;
   sprite->frameDimensions.y = frameHeight;
   sprite->frameStride = sprite->texture->pixelBuffer.dimensions.x / frameWidth;
   sprite->numFrames = sprite->frameStride * ( sprite->texture->pixelBuffer.dimensions.y / frameHeight );
   sprite->frameSeconds = frameSeconds;

   Sprite_Reset( sprite );

   return True;
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

void Sprite_Tick( Sprite_t* sprite, Clock_t* clock )
{
   sprite->secondsElapsed += clock->frameDeltaSeconds;

   while ( sprite->secondsElapsed > sprite->scaledFrameSeconds )
   {
      sprite->secondsElapsed -= sprite->scaledFrameSeconds;
      sprite->frameIndex++;

      if ( sprite->frameIndex >= sprite->numFrames )
      {
         sprite->frameIndex = 0;
      }
   }
}
