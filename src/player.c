#include "player.h"
#include "clock.h"

void Player_Tick( Player_t* player, Clock_t* clock )
{
   Sprite_Tick( player->activeSprite, clock );
}

void Player_SetDirection( Player_t* player, PlayerDirection_t direction )
{
   player->activeSprite = &( player->idleSprites[(uint64_t)direction] );
   Sprite_Reset( player->activeSprite );
}
