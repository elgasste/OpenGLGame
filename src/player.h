#if !defined( PLAYER_H )
#define PLAYER_H

#include "common.h"
#include "vector.h"
#include "sprite.h"

typedef struct Clock_t Clock_t;

typedef struct Player_t
{
   Vector2f_t position;
   PlayerDirection_t facingDirection;
   float runVelocity;
   float maxRunVelocity;
   float jumpVelocity;
   float maxJumpVelocity;
   Bool_t isAirborne;
   float gravityDeceleration;
   float jumpFrameThreshold;
   Bool_t canExtendJump;
   float jumpExtensionSeconds;
   float maxJumpExtensionSeconds;

   Sprite_t idleSprites[2];
   Sprite_t runSprites[2];
   Sprite_t jumpSprites[2];
   Sprite_t* activeSprite;
}
Player_t;

void Player_Init( Player_t* player );
void Player_Tick( Player_t* player, Clock_t* clock );
void Player_SetFacingDirection( Player_t* player, PlayerDirection_t direction );
void Player_SetRunVelocity( Player_t* player, float velocity );
void Player_StartJump( Player_t* player );
void Player_ExtendJump( Player_t* player, Clock_t* clock );

#endif
