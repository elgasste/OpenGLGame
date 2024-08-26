#include "player.h"
#include "clock.h"
#include "image.h"

void Player_Init( Player_t* player )
{
   player->runVelocity = 0.0f;
   player->jumpVelocity = 0.0f;
   player->isAirborne = False;
   player->canExtendJump = False;
}

void Player_Tick( Player_t* player, Clock_t* clock )
{
   uint32_t index;

   if ( player->isAirborne )
   {
      player->jumpVelocity -= ( player->gravityDeceleration * clock->frameDeltaSeconds );
      player->position.y += ( player->jumpVelocity * clock->frameDeltaSeconds );

      index = ( player->jumpVelocity > player->jumpFrameThreshold ) ? 0
         : ( player->jumpVelocity < -( player->jumpFrameThreshold ) ) ? 2 : 1;
      Sprite_SetFrameIndex( player->activeSprite, index );

      if ( player->position.y <= 0.0f )
      {
         player->position.y = 0.0f;
         player->isAirborne = False;
         player->activeSprite = player->runVelocity == 0.0f
            ? &( player->idleSprites[(uint64_t)( player->facingDirection )] )
            : &( player->runSprites[(uint64_t)( player->facingDirection )] );
      }
   }
   else
   {
      Sprite_Tick( player->activeSprite, clock );
   }

   if ( player->facingDirection == PlayerDirection_Left )
   {
      player->position.x -= ( player->runVelocity * clock->frameDeltaSeconds );

      if ( player->position.x < 0.0f )
      {
         player->position.x = 0.0f;
      }
   }
   else
   {
      player->position.x += ( player->runVelocity * clock->frameDeltaSeconds );

      // TODO: in a real game we wouldn't be scaling these values here
      if ( ( ( player->position.x + ( player->activeSprite->base->frameDimensions.x * 2.0f ) ) ) >= SCREEN_WIDTH )
      {
         player->position.x = (float)( SCREEN_WIDTH - ( player->activeSprite->base->frameDimensions.x * 2.0f ) );
      }
   }
}

void Player_SetFacingDirection( Player_t* player, PlayerDirection_t direction )
{
   if ( player->facingDirection != direction && !player->isAirborne )
   {
      player->facingDirection = direction;
      player->activeSprite = player->runVelocity == 0.0f
         ? &( player->idleSprites[(uint64_t)direction] )
         : &( player->runSprites[(uint64_t)direction] );
      Sprite_Reset( player->activeSprite );
   }
}

void Player_SetRunVelocity( Player_t* player, float velocity )
{
   if ( !player->isAirborne )
   {
      if ( player->runVelocity == 0.0f && velocity > 0.0f )
      {
         player->activeSprite = &( player->runSprites[(uint32_t)player->facingDirection] );
      }
      else if ( velocity == 0.0f && player->runVelocity > 0.0f )
      {
         player->activeSprite = &( player->idleSprites[(uint32_t)player->facingDirection] );
      }

      player->runVelocity = ( velocity > player->maxRunVelocity ) ? player->maxRunVelocity : velocity;
   }
}

void Player_StartJump( Player_t* player )
{
   if ( !player->isAirborne )
   {
      player->isAirborne = True;
      player->jumpVelocity = player->maxJumpVelocity;
      player->canExtendJump = True;
      player->jumpExtensionSeconds = 0.0f;
      player->activeSprite = &( player->jumpSprites[(uint32_t)( player->facingDirection )] );
      Sprite_SetFrameIndex( player->activeSprite, 0 );
   }
}

void Player_ExtendJump( Player_t* player, Clock_t* clock )
{
   if ( player->isAirborne && player->canExtendJump )
   {
      player->jumpVelocity = player->maxJumpVelocity;
      player->jumpExtensionSeconds += clock->frameDeltaSeconds;

      if ( player->jumpExtensionSeconds > player->maxJumpExtensionSeconds )
      {
         player->canExtendJump = False;
      }
   }
}
