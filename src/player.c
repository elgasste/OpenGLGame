#include <math.h>

#include "player.h"
#include "clock.h"
#include "render_data.h"

internal void Player_CoolDownAttack( Player_t* player, Clock_t* clock );
internal void Player_ApplyGravity( Player_t* player, Clock_t* clock );
internal void Player_ClipToBounds( Player_t* player );

void Player_Init( Player_t* player, RenderData_t* renderData )
{
   player->idleSprites[0] = &( renderData->sprites[SpriteID_PlayerIdleLeft] );
   player->idleSprites[1] = &( renderData->sprites[SpriteID_PlayerIdleRight] );
   player->runSprites[0] = &( renderData->sprites[SpriteID_PlayerRunLeft] );
   player->runSprites[1] = &( renderData->sprites[SpriteID_PlayerRunRight] );
   player->jumpSprites[0] = &( renderData->sprites[SpriteID_PlayerJumpLeft] );
   player->jumpSprites[1] = &( renderData->sprites[SpriteID_PlayerJumpRight] );
   player->attackSprites[0] = &( renderData->sprites[SpriteID_PlayerAttackLeft1] );
   player->attackSprites[1] = &( renderData->sprites[SpriteID_PlayerAttackRight1] );
   player->attackSprites[2] = &( renderData->sprites[SpriteID_PlayerAttackLeft2] );
   player->attackSprites[3] = &( renderData->sprites[SpriteID_PlayerAttackRight2] );

   player->entity.velocity.x = 0.0f;
   player->entity.velocity.y = 0.0f;
   player->entity.isAirborne = False;
   player->canExtendJump = False;
   player->isAttacking = False;
   player->attackSpriteOffset = 0;
}

void Player_Tick( Player_t* player, Clock_t* clock )
{
   Player_CoolDownAttack( player, clock );
   Player_ApplyGravity( player, clock );

   if ( !player->entity.isAirborne || player->isAttacking )
   {
      Sprite_Tick( player->activeSprite, clock );
   }

   player->entity.position.x += ( player->entity.velocity.x * clock->frameDeltaSeconds );
   Player_ClipToBounds( player );
}

void Player_SetFacingDirection( Player_t* player, PlayerDirection_t direction )
{
   if ( player->facingDirection != direction && !player->isAttacking )
   {
      player->facingDirection = direction;

      if ( player->entity.isAirborne )
      {
         player->activeSprite = player->jumpSprites[(uint32_t)( player->facingDirection )];
      }
      else
      {
         player->activeSprite = player->entity.velocity.x == 0.0f
            ? player->idleSprites[(uint32_t)direction]
            : player->runSprites[(uint32_t)direction];
         Sprite_Reset( player->activeSprite );
      }
   }
}

void Player_AccelerateRun( Player_t* player, Clock_t* clock, PlayerDirection_t direction )
{
   float velocityDelta, newVelocity;
   Entity_t* entity = &( player->entity );

   if ( player->isAttacking )
   {
      return;
   }

   velocityDelta = player->runAcceleration * clock->frameDeltaSeconds;
   newVelocity = ( direction == PlayerDirection_Left )
      ? ( entity->velocity.x - velocityDelta )
      : ( entity->velocity.x + velocityDelta );
   newVelocity = ( direction == PlayerDirection_Left )
      ? fmaxf( newVelocity, -( entity->maxVelocity.x ) )
      : fminf( newVelocity, entity->maxVelocity.x );

   if ( !entity->isAirborne && !player->isAttacking )
   {
      if ( entity->velocity.x == 0.0f && newVelocity != 0.0f )
      {
         player->activeSprite = player->runSprites[(uint32_t)player->facingDirection];
         Sprite_Reset( player->activeSprite );
      }
      else if ( newVelocity == 0.0f && entity->velocity.x != 0.0f )
      {
         player->activeSprite = player->idleSprites[(uint32_t)player->facingDirection];
         Sprite_Reset( player->activeSprite );
      }
   }

   entity->velocity.x = newVelocity;
}

void Player_DecelerateRun( Player_t* player, Clock_t* clock )
{
   float decayer;
   Entity_t* entity = &( player->entity );

   if ( entity->velocity.x != 0.0f )
   {
      decayer = ( entity->velocity.x > 0.0f ) ? -1.0f : 1.0f;
      entity->velocity.x += ( ( player->runAcceleration * clock->frameDeltaSeconds ) * decayer );

      if ( ( decayer > 0.0f && entity->velocity.x > 0.0f ) || ( decayer < 0.0f && entity->velocity.x < 0.0f ) )
      {
         entity->velocity.x = 0.0f;

         if ( !entity->isAirborne && !player->isAttacking )
         {
            player->activeSprite = player->idleSprites[(uint32_t)player->facingDirection];
         }
      }
   }
   else if ( !entity->isAirborne && !player->isAttacking )
   {
      player->activeSprite = player->idleSprites[(uint32_t)player->facingDirection];
   }
}

void Player_StartJump( Player_t* player )
{
   if ( !player->entity.isAirborne && !player->isAttacking )
   {
      player->entity.isAirborne = True;
      player->entity.velocity.y = player->entity.maxVelocity.y;
      player->canExtendJump = True;
      player->jumpExtensionSeconds = 0.0f;
      player->activeSprite = player->jumpSprites[(uint32_t)( player->facingDirection )];
      Sprite_SetFrameIndex( player->activeSprite, 0 );
   }
}

void Player_ExtendJump( Player_t* player, Clock_t* clock )
{
   if ( player->entity.isAirborne && player->canExtendJump )
   {
      player->entity.velocity.y = player->entity.maxVelocity.y;
      player->jumpExtensionSeconds += clock->frameDeltaSeconds;

      if ( player->jumpExtensionSeconds > player->maxJumpExtensionSeconds )
      {
         player->canExtendJump = False;
      }
   }
}

void Player_Attack( Player_t* player )
{
   if ( !player->isAttacking )
   {
      player->isAttacking = True;
      player->attackSeconds = 0.0f;
      player->activeSprite = player->attackSprites[(uint32_t)(player->facingDirection) + player->attackSpriteOffset];
      Sprite_Reset( player->activeSprite );
      player->attackSpriteOffset = player->attackSpriteOffset == 0 ? 2 : 0;

      if ( !player->entity.isAirborne )
      {
         player->entity.velocity.x = 0.0f;
      }
   }
}

internal void Player_CoolDownAttack( Player_t* player, Clock_t* clock )
{
   uint32_t index;

   if ( player->isAttacking )
   {
      player->attackSeconds += clock->frameDeltaSeconds;

      if ( player->attackSeconds > ( player->activeSprite->frameSeconds * player->activeSprite->base->numFrames ) )
      {
         player->isAttacking = False;

         if ( player->entity.isAirborne )
         {
            player->activeSprite = player->jumpSprites[(uint32_t)( player->facingDirection )];
            index = ( player->entity.velocity.y > player->jumpFrameThreshold ) ? 0
               : ( player->entity.velocity.y < -( player->jumpFrameThreshold ) ) ? 2 : 1;
            Sprite_SetFrameIndex( player->activeSprite, index );
         }
         else
         {
            player->activeSprite = player->entity.velocity.x == 0.0f
               ? player->idleSprites[(uint32_t)( player->facingDirection )]
               : player->runSprites[(uint32_t)( player->facingDirection )];
         }
      }
   }
}

internal void Player_ApplyGravity( Player_t* player, Clock_t* clock )
{
   uint32_t index;
   Entity_t* entity = &( player->entity );

   // TODO: terminal velocity?
   entity->velocity.y -= ( entity->gravityDeceleration * clock->frameDeltaSeconds );
   entity->position.y += ( entity->velocity.y * clock->frameDeltaSeconds );

   if ( ( entity->position.y + player->activeSprite->hitBox.y ) <= 0.0f )
   {
      // we've hit the ground
      entity->position.y = -( player->activeSprite->hitBox.y );
      entity->velocity.y = 0.0f;

      if ( entity->isAirborne )
      {
         entity->isAirborne = False;

         if ( !player->isAttacking )
         {
            player->activeSprite = entity->velocity.x == 0.0f
               ? player->idleSprites[(uint32_t)( player->facingDirection )]
               : player->runSprites[(uint32_t)( player->facingDirection )];
         }
      }
   }
   else
   {
      // we're in the air
      if ( !entity->isAirborne && !player->isAttacking )
      {
         player->activeSprite = player->jumpSprites[(uint32_t)( player->facingDirection )];
      }

      entity->isAirborne = True;

      if ( !player->isAttacking )
      {
         index = ( entity->velocity.y > player->jumpFrameThreshold ) ? 0
            : ( entity->velocity.y < -( player->jumpFrameThreshold ) ) ? 2 : 1;
         Sprite_SetFrameIndex( player->activeSprite, index );
      }
   }
}

internal void Player_ClipToBounds( Player_t* player )
{
   // TODO: in a real game we wouldn't be scaling these values here
   if ( ( player->entity.position.x + ( player->activeSprite->hitBox.x * 2.0f ) ) < 0.0f )
   {
      player->entity.position.x = -( player->activeSprite->hitBox.x * 2.0f );
   }
   else if ( ( player->entity.position.x + ( ( player->activeSprite->hitBox.x + player->activeSprite->hitBox.w ) * 2.0f ) ) >= SCREEN_WIDTH )
   {
      player->entity.position.x = (float)( SCREEN_WIDTH - ( player->activeSprite->hitBox.w * 2.0f ) - ( player->activeSprite->hitBox.x * 2.0f ) );
   }
}
