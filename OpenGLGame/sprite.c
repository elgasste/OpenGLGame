#include "sprite.h"
#include "clock.h"

Bool_t Sprite_LoadFromFile( Sprite_t* sprite, const char* filePath, uint32_t frameWidth, uint32_t frameHeight, float frameSeconds )
{
   char errorMsg[STRING_SIZE_DEFAULT];

   if ( !Texture_LoadFromFile( &( sprite->texture ), filePath ) )
   {
      return False;
   }
   else if ( ( sprite->texture.pixelBuffer.dimensions.x % frameWidth != 0 ) ||
             ( sprite->texture.pixelBuffer.dimensions.y % frameHeight != 0 ) )
   {
      snprintf( errorMsg, STRING_SIZE_DEFAULT, STR_SPRITEERR_FRAMEDIMENSIONS, filePath );
      Platform_Log( errorMsg );
      return False;
   }

   sprite->frameDimensions.x = frameWidth;
   sprite->frameDimensions.y = frameHeight;
   sprite->frameStride = sprite->texture.pixelBuffer.dimensions.x / frameWidth;
   sprite->numFrames = sprite->frameStride * ( sprite->texture.pixelBuffer.dimensions.y / frameHeight );
   sprite->frameSeconds = frameSeconds;

   Sprite_Reset( sprite );

   return True;
}

void Sprite_Reset( Sprite_t* sprite )
{
   sprite->frameIndex = 0;
   sprite->secondsElapsed = 0.0f;
   Sprite_ScaleSpeed( sprite, 1.0f );
}

void Sprite_ScaleSpeed( Sprite_t* sprite, float scalar )
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
