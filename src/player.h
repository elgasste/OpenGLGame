#if !defined( PLAYER_H )
#define PLAYER_H

#include "common.h"
#include "vector.h"
#include "sprite.h"

typedef struct Clock_t Clock_t;
typedef struct RenderData_t RenderData_t;

typedef struct Player_t
{
   Vector2f_t position;
   PlayerDirection_t facingDirection;
   Vector2f_t velocity;
   Vector2f_t maxVelocity;
   float runAcceleration;
   float gravityDeceleration;
   Bool_t isAirborne;
   float jumpFrameThreshold;
   Bool_t canExtendJump;
   float jumpExtensionSeconds;
   float maxJumpExtensionSeconds;

   Sprite_t* idleSprites[2];
   Sprite_t* runSprites[2];
   Sprite_t* jumpSprites[2];
   Sprite_t* activeSprite;
}
Player_t;

void Player_Init( Player_t* player, RenderData_t* renderData );
void Player_Tick( Player_t* player, Clock_t* clock );
void Player_SetFacingDirection( Player_t* player, PlayerDirection_t direction );
void Player_Accelerate( Player_t* player, Clock_t* clock, PlayerDirection_t direction );
void Player_Decelerate( Player_t* player, Clock_t* clock );
void Player_StartJump( Player_t* player );
void Player_ExtendJump( Player_t* player, Clock_t* clock );

#endif
