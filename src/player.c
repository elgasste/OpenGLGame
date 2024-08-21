#include "player.h"
#include "clock.h"

void Player_Tick( Player_t* player, Clock_t* clock )
{
   Sprite_Tick( player->activeSprite, clock );

   if ( player->facingDirection == PlayerDirection_Left )
   {
      player->position.x -= ( player->velocity * clock->frameDeltaSeconds );
   }
   else
   {
      player->position.x += ( player->velocity * clock->frameDeltaSeconds );
   }
}

void Player_SetFacingDirection( Player_t* player, PlayerDirection_t direction )
{
   if ( player->facingDirection != direction )
   {
      player->facingDirection = direction;
      player->activeSprite = player->velocity == 0.0f
         ? &( player->idleSprites[(uint64_t)direction] )
         : &( player->moveSprites[(uint64_t)direction] );
      Sprite_Reset( player->activeSprite );
   }
}

void Player_SetVelocity( Player_t* player, float velocity )
{
   if ( player->velocity == 0.0f && velocity > 0.0f )
   {
      player->activeSprite = &( player->moveSprites[(uint32_t)player->facingDirection] );
   }
   else if ( velocity == 0.0f && player->velocity > 0.0f )
   {
      player->activeSprite = &( player->idleSprites[(uint32_t)player->facingDirection] );
   }

   player->velocity = ( velocity > PLAYER_MAX_VELOCITY ) ? PLAYER_MAX_VELOCITY : velocity;
}
