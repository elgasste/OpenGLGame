#include "player.h"
#include "clock.h"

void Player_Tick( Player_t* player, Clock_t* clock )
{
   Sprite_Tick( player->activeSprite, clock );
}

void Player_SetFacingDirection( Player_t* player, PlayerDirection_t direction )
{
   if ( player->facingDirection != direction )
   {
      player->facingDirection = direction;
      player->activeSprite = &( player->idleSprites[(uint64_t)direction] );
      Sprite_Reset( player->activeSprite );
   }
}
