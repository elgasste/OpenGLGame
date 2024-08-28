#include <math.h>

#include "player.h"
#include "clock.h"
#include "render_data.h"

void Player_Init( Player_t* player, RenderData_t* renderData )
{
   player->idleSprites[0] = &( renderData->sprites[SpriteID_PlayerIdleLeft] );
   player->idleSprites[1] = &( renderData->sprites[SpriteID_PlayerIdleRight] );
   player->runSprites[0] = &( renderData->sprites[SpriteID_PlayerRunLeft] );
   player->runSprites[1] = &( renderData->sprites[SpriteID_PlayerRunRight] );
   player->jumpSprites[0] = &( renderData->sprites[SpriteID_PlayerJumpLeft] );
   player->jumpSprites[1] = &( renderData->sprites[SpriteID_PlayerJumpRight] );

   player->velocity.x = 0.0f;
   player->velocity.y = 0.0f;
   player->isAirborne = False;
   player->canExtendJump = False;
}

void Player_Tick( Player_t* player, Clock_t* clock )
{
   uint32_t index;

   if ( player->isAirborne )
   {
      player->velocity.y -= ( player->gravityDeceleration * clock->frameDeltaSeconds );
      player->position.y += ( player->velocity.y * clock->frameDeltaSeconds );

      index = ( player->velocity.y > player->jumpFrameThreshold ) ? 0
         : ( player->velocity.y < -( player->jumpFrameThreshold ) ) ? 2 : 1;
      Sprite_SetFrameIndex( player->activeSprite, index );

      if ( player->position.y <= 0.0f )
      {
         player->position.y = 0.0f;
         player->isAirborne = False;
         player->activeSprite = player->velocity.x == 0.0f
            ? player->idleSprites[(uint64_t)( player->facingDirection )]
            : player->runSprites[(uint64_t)( player->facingDirection )];
      }
   }
   else
   {
      Sprite_Tick( player->activeSprite, clock );
   }

   player->position.x += ( player->velocity.x * clock->frameDeltaSeconds );

   if ( player->position.x < 0.0f )
   {
      player->position.x = 0.0f;
   }
   // TODO: in a real game we wouldn't be scaling these values here
   else if ( ( ( player->position.x + ( player->activeSprite->base->frameDimensions.x * 2.0f ) ) ) >= SCREEN_WIDTH )
   {
      player->position.x = (float)( SCREEN_WIDTH - ( player->activeSprite->base->frameDimensions.x * 2.0f ) );
   }
}

void Player_SetFacingDirection( Player_t* player, PlayerDirection_t direction )
{
   if ( player->facingDirection != direction )
   {
      player->facingDirection = direction;

      if ( player->isAirborne )
      {
         player->activeSprite = player->jumpSprites[(uint32_t)( player->facingDirection )];
      }
      else
      {
         player->activeSprite = player->velocity.x == 0.0f
            ? player->idleSprites[(uint64_t)direction]
            : player->runSprites[(uint64_t)direction];
         Sprite_Reset( player->activeSprite );
      }
   }
}

void Player_Accelerate( Player_t* player, Clock_t* clock, PlayerDirection_t direction )
{
   float velocityDelta, newVelocity;

   velocityDelta = player->runAcceleration * clock->frameDeltaSeconds;
   newVelocity = ( direction == PlayerDirection_Left )
      ? ( player->velocity.x - velocityDelta )
      : ( player->velocity.x + velocityDelta );
   newVelocity = ( direction == PlayerDirection_Left )
      ? fmaxf( newVelocity, -( player->maxVelocity.x ) )
      : fminf( newVelocity, player->maxVelocity.x );

   if ( !player->isAirborne )
   {
      if ( player->velocity.x == 0.0f && newVelocity != 0.0f )
      {
         player->activeSprite = player->runSprites[(uint32_t)player->facingDirection];
      }
      else if ( newVelocity == 0.0f && player->velocity.x != 0.0f )
      {
         player->activeSprite = player->idleSprites[(uint32_t)player->facingDirection];
      }
   }

   player->velocity.x = newVelocity;
}

void Player_Decelerate( Player_t* player, Clock_t* clock )
{
   float decayer;

   if ( player->velocity.x != 0.0f )
   {
      decayer = ( player->velocity.x > 0.0f ) ? -1.0f : 1.0f;
      player->velocity.x += ( ( player->runAcceleration * clock->frameDeltaSeconds ) * decayer );

      if ( ( decayer > 0.0f && player->velocity.x > 0.0f ) || ( decayer < 0.0f && player->velocity.x < 0.0f ) )
      {
         player->velocity.x = 0.0f;

         if ( !player->isAirborne )
         {
            player->activeSprite = player->idleSprites[(uint32_t)player->facingDirection];
         }
      }
   }
   else if ( !player->isAirborne )
   {
      player->activeSprite = player->idleSprites[(uint32_t)player->facingDirection];
   }
}

void Player_StartJump( Player_t* player )
{
   if ( !player->isAirborne )
   {
      player->isAirborne = True;
      player->velocity.y = player->maxVelocity.y;
      player->canExtendJump = True;
      player->jumpExtensionSeconds = 0.0f;
      player->activeSprite = player->jumpSprites[(uint32_t)( player->facingDirection )];
      Sprite_SetFrameIndex( player->activeSprite, 0 );
   }
}

void Player_ExtendJump( Player_t* player, Clock_t* clock )
{
   if ( player->isAirborne && player->canExtendJump )
   {
      player->velocity.y = player->maxVelocity.y;
      player->jumpExtensionSeconds += clock->frameDeltaSeconds;

      if ( player->jumpExtensionSeconds > player->maxJumpExtensionSeconds )
      {
         player->canExtendJump = False;
      }
   }
}
